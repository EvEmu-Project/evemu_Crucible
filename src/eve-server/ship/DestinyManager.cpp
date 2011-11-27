/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2011 The EVEmu Team
	For the latest information visit http://evemu.org
	------------------------------------------------------------------------------------
	This program is free software; you can redistribute it and/or modify it under
	the terms of the GNU Lesser General Public License as published by the Free Software
	Foundation; either version 2 of the License, or (at your option) any later
	version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place - Suite 330, Boston, MA 02111-1307, USA, or go to
	http://www.gnu.org/copyleft/lesser.txt.
	------------------------------------------------------------------------------------
	Author:		Zhur
*/

#include "EVEServerPCH.h"


using namespace Destiny;

const double SPACE_FRICTION = 1.0e+6;		//straight from client. Do not change.
const double SPACE_FRICTION_SQUARED = SPACE_FRICTION*SPACE_FRICTION;
const double TIC_DURATION_IN_SECONDS = 1.0;	//straight from client. Do not change.

static const double DESTINY_UPDATE_RANGE = 1.0e8;	//totally made up. a more complex spatial partitioning system is needed.
static const double FOLLOW_BAND_WIDTH = 100.0f;	//totally made up

uint32 DestinyManager::m_stamp(40000);	//completely arbitrary starting point.
Timer DestinyManager::m_stampTimer(static_cast<int32>(TIC_DURATION_IN_SECONDS * 1000), true);	//accurate timing is essential.

DestinyManager::DestinyManager(SystemEntity *self, SystemManager *system)
: m_self(self),
  m_system(system),
//  m_body(NULL),
//  m_shape(NULL),
//  m_lastDestinyTime(Timer::GetTimeSeconds()),
  m_position(0, 0, 0),	//generally right in the middle of the star!
//  m_direction(0, 0, 0),
//  m_velocity(0),
  m_velocity(0, 0, 0),
//  m_acceleration(0),
  m_maxVelocity(1.0),
  m_accelerationFactor(0.0),
  m_velocityAdjuster(0.0),
//  m_maxAcceleration(1.0),
  State(DSTBALL_STOP),
  m_userSpeedFraction(0),
  m_activeSpeedFraction(0.0),
  m_targetPoint(0, 0, 0),
//  m_targetEntity(0,NULL),
  m_radius(1.0),
  m_mass(1.0),
  m_maxShipVelocity(1.0),
  m_shipAgility(1.0),
  m_shipInertia(1.0),
  m_warpState(NULL)
{
	//do not touch m_self here, it may not be fully constructed.
    m_targetEntity.first = 0;
    m_targetEntity.second = NULL;

    m_warpDecelerateFactor = 0.75;
}

DestinyManager::~DestinyManager() {
	delete m_warpState;
}

void DestinyManager::Process() {
	//process 1 second worth of destiny activities
	ProcessTic();
}

void DestinyManager::SendSingleDestinyUpdate(PyTuple **up, bool self_only) const {
	std::vector<PyTuple *> updates(1, *up);
	*up = NULL;
	std::vector<PyTuple *> events;
	//consumes updates and events
	SendDestinyUpdate(updates, events, self_only);
}

void DestinyManager::SendDestinyUpdate(std::vector<PyTuple *> &updates, bool self_only) const {
	std::vector<PyTuple *> events;
	//consumes updates and events
	SendDestinyUpdate(updates, events, self_only);
}

void DestinyManager::SendDestinyUpdate( std::vector<PyTuple*>& updates, std::vector<PyTuple*>& events, bool self_only ) const
{
	if( self_only )
    {
		_log( DESTINY__TRACE, "[%u] Sending destiny update (%lu, %lu) to self (%u).", GetStamp(), updates.size(), events.size(), m_self->GetID() );

        std::vector<PyTuple*>::iterator cur, end;
		cur = updates.begin();
		end = updates.end();
		for(; cur != end; cur++)
        {
			PyTuple* t = *cur;
			m_self->QueueDestinyUpdate( &t );
            PySafeDecRef( t ); //they are not required to consume it.
		}
		updates.clear();
		
		cur = events.begin();
		end = events.end();
		for(; cur != end; cur++)
        {
			PyTuple* t = *cur;
			m_self->QueueDestinyEvent( &t );
            PySafeDecRef( t ); //they are not required to consume it.
		}
		events.clear();
	}
    else if( NULL != m_self->Bubble() )
    {
		_log( DESTINY__TRACE, "[%u] Broadcasting destiny update (%lu, %lu)", GetStamp(), updates.size(), events.size() );

		m_self->Bubble()->BubblecastDestiny( updates, events, "destiny" );
	}
    else
    {
		_log( DESTINY__ERROR, "[%u] Cannot broadcast destiny update (%lu, %lu); entity (%u) is not in any bubble.", GetStamp(), updates.size(), events.size(), m_self->GetID() );
	}
}

void DestinyManager::_UpdateDerrived() {
	m_maxVelocity = m_maxShipVelocity * m_activeSpeedFraction;
	m_velocityAdjuster = exp(- (SPACE_FRICTION * TIC_DURATION_IN_SECONDS) / (m_mass * m_shipAgility));
	m_accelerationFactor = (SPACE_FRICTION*m_maxVelocity) / (m_mass * m_shipAgility);
	
	_log(PHYSICS__TRACE, "Entity %u has derrived: maxVelocity=%f, velocityAdjuster=%f, accelerationFactor=%f",
		m_self->GetID(), m_maxVelocity, m_velocityAdjuster, m_accelerationFactor);
}

void DestinyManager::ProcessTic() {
	
	_log(PHYSICS__TRACEPOS, "[%d] Entity %u starts at (%.3f, %.3f, %.3f) with velocity (%f, %f, %f)=%.1f in mode %s",
		GetStamp(),
		m_self->GetID(),
		m_position.x, m_position.y, m_position.z,
		m_velocity.x, m_velocity.y, m_velocity.z,
		m_velocity.length(),
		DSTBALL_modeNames[State]);
	
	switch(State) {
		
	case DSTBALL_GOTO:
		//NOTE: WARP code assumes this is simply a call to _Move()!
		_Move();
		break;
		
	case DSTBALL_FOLLOW:
        if(m_targetEntity.second != NULL) {
			_Follow();
		} else {
			//nobody to follow?
			_log(PHYSICS__TRACE, "Entity %u has nothing to follow. Stopping.",
				m_self->GetID() );
			Stop();
		}
		break;
		
	case DSTBALL_STOP:
		if(m_velocity.isNotZero()) {
			//no freakin clue what this is about, maybe to dampen vertical movement.
			//you can see result of this just by going directly up or down in space and
			//then stopping ... deaccelerate is faster than normal.
			m_velocity.y *= 0.93/1.07;

			_Move();
		}
		break;
		
	case DSTBALL_WARP: {
		//warp is a 3 stage thing... first you get up to speed,
		//then you warp, and then you stop.
		
		if(m_warpState != NULL) {
			//warp is already started!
			_Warp();
			break;
		}

		//first check the angle between our warp vector and our
		//velocity vector.
		GVector warp_vector(m_position, m_targetPoint);
		warp_vector.normalize();
		GVector vel_normalized(m_velocity);
		vel_normalized.normalize();
		double dot = warp_vector.dotProduct(vel_normalized);	//cos(angle) between vectors.
		if(dot < (1.0 - 0.01)) {	//this is about 8 degrees
			//execute normal GOTO code.
			_Move();
			break;
		}

		//ok, so they are heading in the right direction now, see about
		//velocity. We need to be going 75% of max velocity.
		double needed_velocity2 = m_maxVelocity*0.75;
		needed_velocity2 *= needed_velocity2;
		if(m_velocity.lengthSquared() < needed_velocity2) {
			//execute normal GOTO code.
			_Move();
			break;
		}

		//ok, we are starting the warp!
		_InitWarp();
		_Warp();
		
		} break;
		
	case DSTBALL_ORBIT:
		_Orbit();
		break;
		
	case DSTBALL_MISSILE:
	case DSTBALL_MUSHROOM:
	case DSTBALL_BOID:
	case DSTBALL_TROLL:
	case DSTBALL_MINIBALL:
	case DSTBALL_FIELD:
	case DSTBALL_FORMATION:
		break;
	case DSTBALL_RIGID:
		break;
	//no default on purpose
	}
}

void DestinyManager::_Follow() {
	
    /*
    this was my idea of how it should be done, which doesn't agree with
    the client.

	GVector targetDirection = GVector(m_position, goal);
	double distance = targetDirection.normalize();
	_log(PHYSICS__TRACEPOS, "Entity %u: Moving towards (%.1f, %.1f, %.1f) which is %f away.",
		m_self->GetID(),
		goal.x, goal.y, goal.z,
		distance);
	distance -= FOLLOW_BAND_WIDTH;
	
	if(distance <= m_targetDistance) {
		//there has to be some band of distance that causes us not to move.
		if((m_targetDistance-distance) < (FOLLOW_BAND_WIDTH+FOLLOW_BAND_WIDTH)) {
			//close enough.
			_log(PHYSICS__TRACEPOS, " Entity %u is %f/%f away from followed entity. Close enough. Not moving.", m_self->GetID());
			m_activeSpeedFraction = 0.0f;
			_UpdateDerrived();
			return;
		}
		//swap around the direction, see what that does...
		targetDirection *= -1.0f;
		
		_log(PHYSICS__TRACEPOS, " Entity %u is %f/%f away from followed entity. Should probably back off...", m_self->GetID());
		return;
	}
	
	targetDirection /= distance;	//normalize the direction vector.
	
	//set our goal point to be m_targetDistance away from targetPos
	//directly along the straight-line vector between us.
	m_targetPoint = goal - (targetDirection * m_targetDistance);
	
	if(m_activeSpeedFraction != m_userSpeedFraction) {
		//we stopped temporarily, go again.
		m_activeSpeedFraction = m_userSpeedFraction;
		_UpdateDerrived();
	}*/

    // First check to see if our target has somehow been removed from space
    // _OR_ the player has left the target ship..
    // if so, then we need to call DestinyManager::Stop() to stop the ship from
    // following a non-existent space object or a pilot-less ship,
    // otherwise, proceed to continue following:
    Client * targetClient = NULL;
    if( m_system->get( m_targetEntity.first ) == NULL )
    {
        // Our target was removed, so STOP
        SetSpeedFraction( 0.0, true );
        Stop( true );
        return;
    }
    else
    {
        // Our target still exists, so do more checking:
        if( (targetClient = (m_targetEntity.second->CastToClient())) != NULL )
        {
            // We are orbiting a Client object, i.e. a player ship
            if( m_targetEntity.first != targetClient->GetShipID() )
            {
                // The client is no longer in the ship we were targeting, so STOP
                SetSpeedFraction( 0.0, true );
                Stop( true );
                return;
            }
        }
    }

    const GPoint &target_point = m_targetEntity.second->GetPosition();
	
	GVector them_to_us(target_point, m_position);
	them_to_us.normalize();

	double desired_distance = 
		m_radius + 
		m_targetEntity.second->GetRadius() + 
		m_targetDistance;

	m_targetPoint = target_point + (them_to_us * desired_distance);
	
	_Move();
}

void DestinyManager::_Move() {
	
	//CalcAcceleration:
	GVector vector_to_goal(m_position, m_targetPoint); //m
	Ga::GaFloat distance_to_goal2 = vector_to_goal.lengthSquared(); //m^2
//	Ga::GaFloat delta = ((tic_duration_in_seconds * speed_fraction * maxVelocity) + radius); //m
//	Ga::GaFloat accel = (space_friction*speed_fraction*maxVelocity) / (agility * mass); //fric*m/(s*agi*kg)
/*  if(delta*delta >= distance_to_goal2) {
		// we are going to reach our goal
		Ga::GaFloat tmp = distance_to_goal2/(delta*delta);
		accel *= tmp*tmp;
    } else {*/
		vector_to_goal /= sqrt(distance_to_goal2); //normalize  yeilds unitless
	//}
	_log(PHYSICS__TRACEPOS, "Accel Magnitude = %.13f", m_accelerationFactor);
	GVector calc_acceleration = vector_to_goal * m_accelerationFactor;	//fric*m/(s*agi*kg) = m/s^2
	
    // Check to see if we have a pending docking operation and attempt to dock if so:
    if( m_self->CastToClient()->GetPendingDockOperation() )
        AttemptDockOperation();
    else
        _MoveAccel(calc_acceleration);
}

void DestinyManager::_MoveAccel(const GVector &calc_acceleration) {
	_log(PHYSICS__TRACEPOS, "Entity %u: Goal Point (%.1f, %.1f, %.1f) with accel (%f, %f, %f)",
		m_self->GetID(),
		m_targetPoint.x, m_targetPoint.y, m_targetPoint.z,
		calc_acceleration.x, calc_acceleration.y, calc_acceleration.z);

	double mass_agility_friction = m_mass * m_shipAgility / SPACE_FRICTION;
	
	GVector max_velocity = calc_acceleration * mass_agility_friction;
	

	// position:

	m_position +=
		max_velocity * TIC_DURATION_IN_SECONDS
		- (max_velocity - m_velocity) * (1 - m_velocityAdjuster) * mass_agility_friction;
	
	
	//now on to velocity:

	

	//velocity_to_m_us:
	m_velocity =
		max_velocity - (max_velocity - m_velocity) * m_velocityAdjuster;


	
#if 0
	Ga::GaVec3 start_acceleration = CalcAcceleration();
	global Ga::GaFloat space_friction = 1.0e6;
	Ga::GaFloat mass_times_agility = mass * agility;
	Ga::GaFloat tic_duration_in_seconds = 1.0;
	Ga::GaFloat velocity_adjuster = exp(- (space_friction * tic_duration_in_seconds) / mass_times_agility);
	
	Ga::GaVec3 start_position = m_body->getPosition();
	Ga::GaVec3 start_velocity = m_body->getLinearVelocity();
	
	Ga::GaFloat friction_squared = space_friction*space_friction;
	Ga::GaVec3 pos_times_f2 = start_position * friction_squared;

	Ga::GaVec3 velocity_reduced = start_velocity * velocity_adjuster;
	
	Ga::GaVec3 acceleration = start_acceleration * tic_duration_in_seconds;
	acceleration += start_velocity;
	velocity_reduced -= acceleration;
	
	//next_crap:
	acceleration = velocity_reduced * space_friction;
	
	//next_crap2:
	Ga::GaVec3 acceleration_ma = start_acceleration * mass_times_agility;
	acceleration_remainder = acceleration_ma * (velocity_adjuster - 1);
	
	//apply_acceleration:
	Ga::GaVec3 accel_sum = acceleration_remainder + acceleration;
	accel_sum *= mass_times_agility;

	//apply_velocity:
	Ga::GaVec3 position_adjusted = accel_sum + pos_times_f2;
	//reduce_big_to_si:
	position_adjusted /= friction_squared;
	//-- this is the resulting position --

	//velocity_to_m_us:
	Ga::GaVec3 velocity_friction = start_velocity * space_friction;
	
	//acceleration_times_arg_c re-calculates acceleration_ma
	
	//next_crap4:
	Ga::GaVec3 remaining_accel = acceleration_ma - velocity_friction;

	//next_crap5:
	Ga::GaVec3 remaining_accel_adjusted = remaining_accel * velocity_adjuster;
	
	//next_crap6:
	Ga::GaVec3 velocity_adjusted = acceleration_ma - remaining_accel_adjusted;
	//divide_by_bp60:
	velocity_adjusted /= space_friction;
	//-- this is the resulting acceleration --
	
#endif
}

void DestinyManager::_InitWarp() {
	
	_log(PHYSICS__TRACE, " Entity %u starting warp to (%f, %f, %f) at distance %.2f",
		m_self->GetID(),
		m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_targetDistance);
	
	//init warp:
	//destiny_WARP_Math
	GVector vector_from_goal(m_targetPoint, m_position);
	vector_from_goal.normalize();
	
	//the vector indicating how far from the goal we will stop warping.
	GVector warp_to_distance_vector = vector_from_goal * m_targetDistance;
	
	GPoint stop_point = m_targetPoint + warp_to_distance_vector;
	
	GPoint warp_vector = stop_point - m_position;
	double warp_distance = warp_vector.length();

	m_targetPoint = stop_point;
	_log(PHYSICS__TRACE, " Entity %u has new warp target of (%f, %f, %f) which is at distance %f",
		m_self->GetID(),
		m_targetPoint.x, m_targetPoint.y, m_targetPoint.z,
		warp_distance);

	//the client clears massive during warp, presumably so you
	//do not run into things while warping.
#ifndef WIN32
#warning TODO: set massive to false!
#endif


	//double warp_speed = m_system->GetWarpSpeed();
    double warp_speed = 0.0;
    ShipRef shipRef;
    if( m_self->IsClient() )
        shipRef = m_self->CastToClient()->GetShip();
    else if( m_self->IsNPC() )
        sLog.Warning( "DestinyManager::_InitWarp():", "NPC Ship using DestinyManager is NOT supported at this time. NPC class MUST be inherited from ShipEntity" );
        //shipRef = m_self->CastToNPC()->GetShip();
    else
        sLog.Error( "DestinyManager::_InitWarp():", "'m_self' was not found to be either a Client object nor an NPC object." );

    if( shipRef )
    {
        double baseWarpSpeed = m_self->CastToClient()->GetShip()->GetAttribute(AttrBaseWarpSpeed).get_float();
        double warpSpeedMultiplier = m_self->CastToClient()->GetShip()->GetAttribute(AttrWarpSpeedMultiplier).get_float();
        
        //warp_speed = (double)(m_self->CastToClient()->GetShip()->GetAttribute(AttrWarpSpeedMultiplier).get_float()) * ONE_AU_IN_METERS;
        warp_speed = baseWarpSpeed * ((double)BASE_WARP_SPEED) * warpSpeedMultiplier * ((double)ONE_AU_IN_METERS);
    }
    else
        warp_speed = m_system->GetWarpSpeed();

	if(1.5*warp_distance < warp_speed) {//not positive on this conditional
		warp_speed = 1.5*warp_distance;
		_log(PHYSICS__TRACEPOS, "Adjusting warp speed to %f for short warp.", warp_speed);
	} else {
		_log(PHYSICS__TRACEPOS, "Warp speed in system %u is %f", m_system->GetID(), warp_speed);
	}
	
	const double log_warp_speed_over_three__ = log(warp_speed / 3.0f);
	
	const double warp_acceleration_time = log_warp_speed_over_three__ / 3.0f;	//name guessed.
	
	double warp_slow_time = log_warp_speed_over_three__ - 2;
	warp_slow_time *= warp_speed;
	warp_slow_time += warp_distance * 3.0f;
	warp_slow_time /= warp_speed * 3.0f;	//v40 ~9.1105
	
    sLog.Debug( "DestinyManager::_InitWarp():", "Warp will accelerate for %f s, then slow down at %f s", warp_acceleration_time, warp_slow_time);
	
	sLog.Debug( "DestinyManager::_InitWarp():", "Opposite warp direction is (%.13f, %.13f, %.13f)",
		vector_from_goal.x, vector_from_goal.y, vector_from_goal.z);

	delete m_warpState;
	m_warpState = new WarpState(
		GetStamp(),
		warp_distance, 
		warp_speed, 
		warp_acceleration_time, 
		warp_slow_time, 
		vector_from_goal
		);
	//during the warp, we are not in any bubble.
	//m_self->Bubble()->Remove(m_self);
}

void DestinyManager::_Warp() {
	if(m_warpState == NULL) {
		codelog(DESTINY__ERROR, "Error: _Warp() called with no warp state!");
		return;
	}
	const double seconds_into_warp = (GetStamp() - m_warpState->start_stamp) * TIC_DURATION_IN_SECONDS;
	_log(PHYSICS__TRACEPOS, "Seconds into warp %.1f", seconds_into_warp);
	
	
	/*
     * There are three stages of warp, which are related to our speed:
     *
     * stage 1) Accelerating. This is where we are accelerating, and
     *    lasts until we have reached the warp speed for this system.
     * stage 2) Cruising. This is where we have reached warp speed
     *    and are just moving along at warp speed until we need to
     *    start slowing down.
     * stage 3) Slowing. We are approaching our destination, and start
     *    slowing down out of warp speed.
     *
     */
	//variables set by the current stage
	double dist_remaining;
	double velocity_magnitude;
	bool stop = false;
	
	if(seconds_into_warp < m_warpState->acceleration_time) {
		double warp_progress = exp( 3.0f * seconds_into_warp );
		dist_remaining = m_warpState->total_distance - warp_progress;
		
		velocity_magnitude = warp_progress * 3.0;
		
        // Remove ship from bubble only when distance traveled takes the ship beyond the bubble's radius
        m_system->bubbles.UpdateBubble(m_self);

        sLog.Debug( "DestinyManager::_Warp():", "Entity %u: Warp Accelerating: velocity %f m/s with %f m left to go.", 
			m_self->GetID(),
			velocity_magnitude, dist_remaining);
		
	} else if(seconds_into_warp < m_warpState->slow_time) {
		//dont simplify this, the roundoff error caused by all the extra
		// multiply/divide crap they is major enough as to cause problems
		double delta_t = (m_warpState->acceleration_time * 3.0f) - ((seconds_into_warp * 3.0f)+1.0f);
		double delta_s = (m_warpState->speed * delta_t)/(-3.0f);
		dist_remaining = m_warpState->total_distance - delta_s;

		velocity_magnitude = m_warpState->speed;

	//    if( (m_warpState->total_distance - dist_remaining) > 1.0e6f )
	//    {
	//	    // warp is long enough to be in cruise and we're out of the warp-out bubble,
    //        // so remove ourselves from our previous bubble:
	//	    m_self->Bubble()->Remove(m_self);
	//    }

		sLog.Debug( "DestinyManager::_Warp():", "Entity %u: Warp Cruising: velocity %f m/s with %f m left to go.", 
			m_self->GetID(),
			velocity_magnitude, dist_remaining);
	} else {
		//warp_completely_done

        // TODO: Adjust these equations because warp-in final position depends on them
        // and is still wrong.  Also, see how these work when you change the
        // maximum warp speed in AU/s, it is hard-coded to 3.0AU/s in SystemManager.cpp
        // in the SystemManager::GetWarpSpeed() function.
		double v58 = (((m_warpState->acceleration_time + (m_warpState->total_distance/m_warpState->speed) - seconds_into_warp) )) - (2.0f/3.0f);
		
		velocity_magnitude = exp(v58) * m_warpState->speed / 3.0f;
		
		dist_remaining = velocity_magnitude / 1.65;
		//dist_remaining = m_warpDecelerateFactor * velocity_magnitude;

		if(velocity_magnitude < 0)
			velocity_magnitude = -velocity_magnitude;
		
		sLog.Debug( "DestinyManager::_Warp():", "Entity %u: Warp Slowing: velocity %f m/s with %f m left to go.", 
			m_self->GetID(),
			velocity_magnitude, dist_remaining);
		
		// Put ourself back into a bubble once we reach the outer edge of the bubble's radius:
        if( dist_remaining <= m_self->Bubble()->m_radius )
		    m_system->bubbles.UpdateBubble(m_self);
		
		//note, this should actually be checked AFTER we change new_velocity.
		//but hey, it doesn't get copied into ball.velocity until later anyhow.
		if(velocity_magnitude < m_maxShipVelocity) {
			stop = true;
            SetPosition( GetPosition(), true );
		}
	}

//	if( m_warpState->acceleration_time > 1000 )
//	{
//		//during the warp, we are not in any bubble.
//		m_self->Bubble()->Remove(m_self);
//	}
	
	GVector vector_to_us = m_warpState->normvec_them_to_us * dist_remaining;
	
	m_position = m_targetPoint + vector_to_us;
	m_velocity = m_warpState->normvec_them_to_us * (-velocity_magnitude);
	
	if(stop) {
		sLog.Debug( "DestinyManager::_Warp():", "Entity %u: Warp completed. Exit velocity %f m/s with %f m left to go.", 
			m_self->GetID(),
			velocity_magnitude, dist_remaining);
		//they re-calculated the velocity, but it was exactly the same..
		// our calc may only be accurate to 6 decimals for some reason..
		delete m_warpState;
		m_warpState = NULL;
		//put ourself back into a bubble.
		//m_system->bubbles.UpdateBubble(m_self);
		Stop(false);	//no updates, client is doing this too.
	}
}

//this is still under construction. Its not working well right now.
void DestinyManager::_Orbit() {
#define PVN(v) _log(PHYSICS__TRACEPOS, "Entity %u: " #v ": (%.15e, %.15e, %.15e)   len=%.15e", \
			m_self->GetID(), \
			v.x, v.y, v.z, v.length() )
		
    // First check to see if our target has somehow been removed from space
    // _OR_ the player has left the target ship..
    // if so, then we need to call DestinyManager::Stop() to stop the ship from
    // following a non-existent space object or a pilot-less ship,
    // otherwise, proceed to continue orbiting:
    Client * targetClient = NULL;
    if( m_system->get( m_targetEntity.first ) == NULL )
    {
        // Our target was removed, so STOP
        SetSpeedFraction( 0.0, true );
        Stop( true );
        return;
    }
    else
    {
        // Our target still exists, so do more checking:
        if( (targetClient = (m_targetEntity.second->CastToClient())) != NULL )
        {
            // We are orbiting a Client object, i.e. a player ship
            if( m_targetEntity.first != targetClient->GetShipID() )
            {
                // The client is no longer in the ship we were targeting, so STOP
                SetSpeedFraction( 0.0, true );
                Stop( true );
                return;
            }
        }
    }

    const GPoint &orbit_point = m_targetEntity.second->GetPosition();
	
	GVector delta(m_position, orbit_point);
	PVN(delta);
	double current_distance = delta.normalize();
	PVN(delta);
	
	double something = 0;
	double desired_distance = 
		m_radius + 
		m_targetEntity.second->GetRadius() + 
		m_targetDistance;
	_log(PHYSICS__TRACEPOS, "desired_distance = %.15e", desired_distance);
	if(desired_distance != 0) {
		something = (m_maxVelocity * TIC_DURATION_IN_SECONDS * .01) / desired_distance;
	}
	_log(PHYSICS__TRACEPOS, "something = %.15e", something);

//this seems to be correct, without rounding error.
	double v488 = double(GetStamp()-m_stateStamp) * something;
	_log(PHYSICS__TRACEPOS, "v488 = %.15e", v488);
	
//this is not quite right... some sort of rounding I think.
	double coef = (v488 * 0.7f) + 130001409/*entityID*/;
	_log(PHYSICS__TRACEPOS, "coef = %.15e", coef);

//all of these are wrong due to rounding
	double cos_coef = cos( coef );
	double sin_coef = sin( coef );
	double cos_v488 = cos( v488 );
	double sin_v488 = sin( v488 );
	_log(PHYSICS__TRACEPOS, "coef(sin=%.15e, cos=%.15e) v488(sin=%.15e, cos=%.15e)", sin_coef, cos_coef, sin_v488, cos_v488);
	double v438 = cos_coef * sin_v488;
	double v3C0 = cos_v488 * cos_coef;

	GPoint pt( v3C0, sin_coef, v438 );	//this is a unit vector naturally
	PVN(pt);
	
	GVector tan_vector = pt.crossProduct(delta);
	tan_vector.normalize();
	PVN(tan_vector);

	double delta_d2 = current_distance*current_distance - desired_distance*desired_distance;
	_log(PHYSICS__TRACEPOS, "delta_d2 = %.15e", delta_d2);
	if(delta_d2 >= 0) {
		double mag = sqrt(delta_d2) * desired_distance / current_distance;
		_log(PHYSICS__TRACEPOS, "mag = %.15e", mag);
		GVector s = tan_vector * mag;
		PVN(s);
		GVector d = delta * (delta_d2/current_distance);
		PVN(d);
		delta = s + d;
		delta.normalize();
		PVN(delta);
	}
	
	double d = desired_distance - current_distance;
	d = exp(d*d / (-40000.0f));
	_log(PHYSICS__TRACEPOS, "d = %.15e", d);

	GVector negative_delta = delta * -1;
	PVN(negative_delta);
	
	double tdn = tan_vector.dotProduct(negative_delta);
	_log(PHYSICS__TRACEPOS, "tdn = %.15e", tdn);
	double jjj = ((tdn*tdn - 1.0f) * d * d) + 1;
	_log(PHYSICS__TRACEPOS, "jjj = %.15e", jjj);
	double iii;
	if(jjj < 0) {	//not sure on this condition at all.
		iii = d * tdn;	//not positive
	} else {
		iii = sqrt(jjj) + (d * tdn);
	}

	if((current_distance - desired_distance) < 0) {
		iii *= -1;
	}

	GPoint bliii = delta * iii;
	PVN(bliii);
	GPoint vliii = tan_vector * d;
	PVN(vliii);

	GPoint accel_vector = bliii + vliii;
	_log(PHYSICS__TRACEPOS, "m_accelerationFactor = %.15e", m_accelerationFactor);
	accel_vector *=  m_accelerationFactor;
	PVN(accel_vector);

	//copy delta_position into acceleration for input into movement.

	static const double ten_au = 1.495978707e12;
	GVector big_delta_position = accel_vector * ten_au;
	PVN(big_delta_position);

	m_targetPoint = orbit_point + big_delta_position;
	PVN(m_targetPoint);
#undef PVN
	
	_MoveAccel(accel_vector);
}

//called whenever an entity is going away and can no longer be used as a target
void DestinyManager::EntityRemoved(SystemEntity *who) {
	if(m_targetEntity.second == who) {
        m_targetEntity.first = 0;
        m_targetEntity.second = NULL;
		
		switch(State) {
		case DSTBALL_GOTO:
		case DSTBALL_STOP:
		case DSTBALL_MISSILE:
		case DSTBALL_MUSHROOM:
		case DSTBALL_BOID:
		case DSTBALL_TROLL:
		case DSTBALL_MINIBALL:
		case DSTBALL_FIELD:
		case DSTBALL_FORMATION:
		case DSTBALL_RIGID:
		case DSTBALL_WARP:
			//do nothing, they dont use the follow entity.
			break;
			
		case DSTBALL_FOLLOW:
		case DSTBALL_ORBIT:
			_log(DESTINY__DEBUG, "%u: Our target entity has gone away. Stopping.", m_self->GetID());
			Stop();
			break;
			
		//no default on purpose
		}
	}
}
	
//Global Actions:
void DestinyManager::Stop(bool update) {
    // THIS IS A HACK AS WE DONT KNOW WHY THE CLIENT CALLS STOP AT UNDOCK
    if( m_self->CastToClient()->GetJustUndocking() )
    {
        // Client just undocked from a station so DO NOT STOP:
        m_self->CastToClient()->SetJustUndocking( false );
        GPoint dest;
        m_self->CastToClient()->GetUndockAlignToPoint( dest );
        //AlignTo( dest, true );
        GotoDirection( dest.normalize(), true );
        SetSpeedFraction( 1.0, true );
    }
    else
    {
        // vvv ORIGINAL DestinyManager::Stop(bool update) BEGINS HERE vvv
	    if(State == DSTBALL_STOP)
		    return;

        if(State == DSTBALL_WARP && m_warpState != NULL) {
		    //warp aborted!
		    delete m_warpState;
		    m_warpState = NULL;
		    //put ourself back into a bubble.
		    m_system->bubbles.UpdateBubble(m_self);
	    }
	
        m_targetEntity.first = 0;
	    m_targetEntity.second = NULL;
	
	    //redude velocity to 0, applying reverse thrust until we get there.
    //	m_targetPoint = m_position - (m_velocity * 1.0e6);	//opposite direction
	    m_activeSpeedFraction = 0.0f;
	    _UpdateDerrived();
	
	    State = DSTBALL_STOP;

	    if(update) {
		    DoDestiny_Stop du;
		    du.entityID = m_self->GetID();
		
		    PyTuple *tmp = du.Encode();
		    SendSingleDestinyUpdate(&tmp);	//consumed
	    }
    }
}

void DestinyManager::Halt(bool update) {
    m_targetEntity.first = 0;
    m_targetEntity.second = NULL;
	m_velocity = GVector(0, 0, 0);
	m_activeSpeedFraction = 0.0f;
	_UpdateDerrived();
	
	State = DSTBALL_STOP;

	//ensure that our bubble is correct.
	m_system->bubbles.UpdateBubble(m_self);
	
	if(update) {
		std::vector<PyTuple *> updates;
		
		{
		DoDestiny_SetSpeedFraction du;
		du.entityID = m_self->GetID();
		du.fraction = 0.0;
	
		updates.push_back(du.Encode());
		}
		{
		DoDestiny_SetBallVelocity du;
		du.entityID = m_self->GetID();
		du.x = 0.0;
		du.y = 0.0;
		du.z = 0.0;
	
		updates.push_back(du.Encode());
		}
		
		SendDestinyUpdate(updates, false);
	}
}
	
//Local Movement:
void DestinyManager::Follow(SystemEntity *who, double distance, bool update) {
    if(State == DSTBALL_FOLLOW && m_targetEntity.second == who && m_targetDistance == distance)
		return;
	
	State = DSTBALL_FOLLOW;
    m_targetEntity.first = who->GetID();
	m_targetEntity.second = who;
	m_targetDistance = distance;
	if(m_userSpeedFraction == 0.0f)
		m_userSpeedFraction = 1.0f;
	if(m_activeSpeedFraction != m_userSpeedFraction) {
		m_activeSpeedFraction = m_userSpeedFraction;
		_UpdateDerrived();
	}
	
	if(update) {
		DoDestiny_FollowBall du;
		du.entityID = m_self->GetID();
		du.ballID = who->GetID();
		du.unknown = uint32(distance);
		
		PyTuple *tmp = du.Encode();
		SendSingleDestinyUpdate(&tmp);	//consumed
	}

    // Forcibly set Speed since it doesn't get updated when Following upon Undock from stations:
    SetSpeedFraction( m_activeSpeedFraction, true );
}

void DestinyManager::Orbit(SystemEntity *who, double distance, bool update) {
	if(State == DSTBALL_ORBIT && m_targetEntity.second == who && m_targetDistance == distance)
		return;
	
	State = DSTBALL_ORBIT;
	m_stateStamp = GetStamp()+1;
	
    m_targetEntity.first = who->GetID();
	m_targetEntity.second = who;
	m_targetDistance = distance;
    /*if(m_userSpeedFraction == 0.0f)
        m_userSpeedFraction = 1.0f;*/	//doesn't seem to do this.
	if(m_activeSpeedFraction != m_userSpeedFraction) {
		m_activeSpeedFraction = m_userSpeedFraction;
		_UpdateDerrived();
	}
	
	if(update) {
		DoDestiny_Orbit du;
		du.entityID = m_self->GetID();
		du.orbitEntityID = who->GetID();
		du.distance = uint32(distance);
		
		PyTuple *tmp = du.Encode();
		SendSingleDestinyUpdate(&tmp);	//consumed
	}
}

void DestinyManager::SetShipCapabilities(InventoryItemRef ship)
{
	double mass = ship->GetAttribute(AttrMass).get_float();               // Aknor: EVEAttributeMgr cant find this
	double radius = ship->GetAttribute(AttrRadius).get_float();         // Aknor: EVEAttributeMgr cant find this, assertion failed: "mType == evil_number_float", line 189 EvilNumber.h
	double Inertia = ship->GetAttribute(AttrInertia).get_float();       // Aknor: EVEAttributeMgr cant find this, assertion failed: "mType == evil_number_float", line 189 EvilNumber.h
	double agility = ship->GetAttribute(AttrAgility).get_float();
	double maxVelocity = ship->GetAttribute(AttrMaxVelocity).get_float();

	//might need to care about turnAngle: Maximum turn angle of a ship in Radians, 0 to pi (3.14). 
	//might need newAgility: Maximum "Thrust angle" for an object in Radians, 0 to pi (3.14).
	//speedBoostFactor is in newtons...
	//speedBonus affects max velocity.
	//warpSpeedMultiplier
	
/*#ifndef WIN32
#warning need to feed inertia to physics model properly
#endif
	//do we want to use radius here??? or just adjust CoG?
	Ga::GaVec3 center_of_gravity = Ga::Parameter::VEC3_ZERO;
	Ga::GaVec3 inertia_tensor = Ga::Parameter::VEC3_ONE;
    m_body->setMassMatrix(mass, center_of_gravity, inertia_tensor);*/

	m_radius = radius;
	m_mass = mass;
	m_maxShipVelocity = maxVelocity;
	m_shipAgility = agility;
	m_shipInertia = Inertia;
//	m_maxShipAcceleration = maxShipAcceleration;
	
	_log(PHYSICS__TRACE, "Entity %u is using ship attributes: Radius=%f, Mass=%f, maxVelocity=%f, agility=%f, inertia=%f",
		m_self->GetID(), m_radius, m_mass, m_maxShipVelocity, m_shipAgility, m_shipInertia);
	
	_UpdateDerrived();
}

void DestinyManager::SetPosition(const GPoint &pt, bool update) {
	//m_body->setPosition( pt );
	m_position = pt;
	_log(PHYSICS__TRACE, "Entity %u set its position to (%.1f, %.1f, %.1f)",
		m_self->GetID(), m_position.x, m_position.y, m_position.z );
	
	if(update) {
		DoDestiny_SetBallPosition du;
		du.entityID = m_self->GetID();
		du.x = pt.x;
		du.y = pt.y;
		du.z = pt.z;
		
		PyTuple *tmp = du.Encode();
		SendSingleDestinyUpdate(&tmp);	//consumed
	}
	m_system->bubbles.UpdateBubble(m_self, update);
}

void DestinyManager::SetSpeedFraction(double fraction, bool update) {
	m_userSpeedFraction = fraction;
	m_activeSpeedFraction = m_userSpeedFraction;
	_UpdateDerrived();
	
	if(update) {
		DoDestiny_SetSpeedFraction du;
		du.entityID = m_self->GetID();
		du.fraction = fraction;
		
		PyTuple *tmp = du.Encode();
		SendSingleDestinyUpdate(&tmp);	//consumed
	}
}

/* AlignTo
 * align the ship/pod to an entity in the same system.  Respond with 
 * a GotoPoint.
 *
 * @author xanarox
*/
void DestinyManager::AlignTo(const GPoint &direction, bool update) {
	State = DSTBALL_GOTO;
	m_targetPoint = m_position + (direction * 1.0e16);
	bool process = false;
	if(m_userSpeedFraction == 0.0f) {
		m_userSpeedFraction = 1.0f;
		process = true;
	}
	if(m_activeSpeedFraction != m_userSpeedFraction) {
		m_activeSpeedFraction = m_userSpeedFraction;
		_UpdateDerrived();
	}

	if(update) {
		DoDestiny_GotoPoint du;
		du.entityID = m_self->GetID();
		du.x = direction.x;
		du.y = direction.y;
		du.z = direction.z;
		
		PyTuple *tmp = du.Encode();
		SendSingleDestinyUpdate(&tmp);	//consumed
	}
}

void DestinyManager::GotoDirection(const GPoint &direction, bool update) {
	State = DSTBALL_GOTO;
	m_targetPoint = m_position + (direction * 1.0e16);
	bool process = false;
	if(m_userSpeedFraction == 0.0f) {
		m_userSpeedFraction = 1.0f;
		process = true;
	}
	if(m_activeSpeedFraction != m_userSpeedFraction) {
		m_activeSpeedFraction = m_userSpeedFraction;
		_UpdateDerrived();
	}
	
	if(update) {
		DoDestiny_GotoDirection du;
		du.entityID = m_self->GetID();
		du.x = direction.x;
		du.y = direction.y;
		du.z = direction.z;
		
		PyTuple *tmp = du.Encode();
		SendSingleDestinyUpdate(&tmp);	//consumed
	}
}

PyResult DestinyManager::AttemptDockOperation()
{
    Client * who = m_self->CastToClient();
    SystemManager * sm = m_self->System();
    uint32 stationID = who->GetDockStationID();
	SystemEntity *station = sm->get(stationID);

    if(station == NULL) {
		codelog(CLIENT__ERROR, "%s: Station %u not found.", who->GetName(), stationID);
		return NULL;
	}
	
    GPoint stationDockPoint = static_cast< StationEntity* >( station )->GetStationObject()->GetStationType()->dockEntry();     //station->GetPosition();
    GVector stationDockOrientation = static_cast< StationEntity*>( station )->GetStationObject()->GetStationType()->dockOrientation();
    const GPoint &position = who->GetPosition();

	OnDockingAccepted da;
    da.end_x = stationDockPoint.x;
    da.end_y = stationDockPoint.y;
    da.end_z = stationDockPoint.z;
    da.start_x = position.x;
	da.start_y = position.y;
	da.start_z = position.z;
	da.stationID = stationID;

	GPoint start(da.start_x, da.start_y, da.start_z);
	GPoint end(da.end_x, da.end_y, da.end_z);
	GVector direction(start, end);
	direction.normalize();
    double rangeToStation = direction.length();
	
    // Calculate 1000m out from docking bay along dock orientation vector away from station:
    stationDockPoint.x += 1000 * stationDockOrientation.x;
    stationDockPoint.y += 1000 * stationDockOrientation.y;
    stationDockPoint.z += 1000 * stationDockOrientation.z;

    GotoDirection( stationDockPoint, true );   // Turn ship and move toward docking point

    // Verify range to station is within docking perimeter of 500 meters:
    if( (rangeToStation - station->GetRadius()) > 500 )
    {
        who->SetPendingDockOperation( true );   // Set client object into state that a Docking operation is pending
        return NULL;                            // so that DestinyManager can track when it needs to auto-dock
    }
    // This packet has to be returned to the client when outside the docking perimeter
    // Packet::ErrorResponse
    /* -- In the marshalstream:
    ObjectEx:
    Header:
      Tuple: 3 elements
        [ 0] Token: 'ccp_exceptions.UserError'
        [ 1] Tuple: 1 elements
        [ 1]   [ 0] String: 'DockingApproach'
        [ 2] Dictionary: 2 entries
        [ 2]   [ 0] Key: String: 'msg'
        [ 2]   [ 0] Value: String: 'DockingApproach'
        [ 2]   [ 1] Key: String: 'dict'
        [ 2]   [ 1] Value: (None)
    List data:
      Empty
    Dict data:
      Empty
    */
    // TODO: On docking failure: Send DoDestinyUpdates for GotoPoint for Station Dockpoint, SetBallMassive, SetBallMass
    
	// When docking, Set X,Y,Z to origin so that when changing ships in stations, they don't appear outside:
	who->MoveToLocation( stationID, GPoint(0, 0, 0) );

    who->SetPendingDockOperation( false );

    //clear all targets
	who->targets.ClearAllTargets();

	//Check if player is in pod, in which case they get a rookie ship for free
	if( who->GetShip()->typeID() == itemTypeCapsule )
    {
		//set base type for rookie ship
		uint32 typeID = caldariRookie;

		//set spawn location for hangar - not sure if this is correct.  Do you instantly get put in the rookie ship?
		EVEItemFlags flag = (EVEItemFlags)flagHangar;

		//create rookie ship of appropriate type
		if(who->GetChar()->race() == raceAmarr )
			typeID = amarrRookie;
		else if(who->GetChar()->race() == raceCaldari )
			typeID = caldariRookie;
		else if(who->GetChar()->race() == raceGallente )
			typeID = gallenteRookie;
		else if(who->GetChar()->race() == raceMinmatar )
			typeID = minmatarRookie;
		
		//create data for new rookie ship
		ItemData idata(
			typeID,
			who->GetCharacterID(),
			0, //temp location
			flag,
			1
		);
		//spawn rookie
		InventoryItemRef i = who->services().item_factory.SpawnItem( idata );
	
		//move the new rookie ship into the players hanger in station
		if(!i)
			throw PyException( MakeCustomError( "Unable to generate correct rookie ship" ) );

		i->Move( who->GetStationID(), flag, true );

	}

    // Docking was accepted, so send the OnDockingAccepted packet:
    // Packet::Notification
    /* -- In the marshalstream:
    Tuple: 2 elements
      [ 0] Integer field: 0
      [ 1] Tuple: 2 elements
      [ 1]   [ 0] Integer field: 1
      [ 1]   [ 1] Tuple: 3 elements
      [ 1]   [ 1]   [ 0] List: 3 elements                           // Station Dockpoint
      [ 1]   [ 1]   [ 0]   [ 0] Real field: 273625620310.000000     // x
      [ 1]   [ 1]   [ 0]   [ 1] Real field: -12937663343.000000     // y
      [ 1]   [ 1]   [ 0]   [ 2] Real field: 50607993006.845703      // z
      [ 1]   [ 1]   [ 1] List: 3 elements                           // Ship Position
      [ 1]   [ 1]   [ 1]   [ 0] Real field: 273625620310.000000     // x
      [ 1]   [ 1]   [ 1]   [ 1] Real field: -12937663343.000000     // y
      [ 1]   [ 1]   [ 1]   [ 2] Real field: 50607993007.845703      // z
      [ 1]   [ 1]   [ 2] Integer field: 60004450                    // Station ID
    */
	return NULL;
}

void DestinyManager::WarpTo(const GPoint &where, double distance, bool update) {
	SetSpeedFraction(1.0, update);

	if(m_warpState != NULL) {
		delete m_warpState;
		m_warpState = NULL;
	}
	
	State = DSTBALL_WARP;
    m_targetEntity.first = 0;
	m_targetEntity.second = NULL;
	m_targetPoint = where;
	m_targetDistance = distance;
	
	if(update) {
		std::vector<PyTuple *> updates;
		
		{
		DoDestiny_WarpTo du;
		du.entityID = m_self->GetID();
		du.dest_x = where.x;
		du.dest_y = where.y;
		du.dest_z = where.z;
		du.distance = static_cast<int32>(distance);
		du.u5 = 30;
		
		updates.push_back(du.Encode());
		}
	
		{
		//send a warping special effects update...
		DoDestiny_OnSpecialFX10 du;
		du.effect_type = "effects.Warping";
		du.entityID = m_self->GetID();
		du.isOffensive = 0;
		du.start = 1;
		du.active = 0;
		
		updates.push_back(du.Encode());
		}
		
		SendDestinyUpdate(updates, false);
	}
}

bool DestinyManager::_Turn() {
#ifdef OLD_STUFF
	double direction_cross = m_direction.dotProduct(m_targetDirection);	//cos(angle between)
	if(direction_cross != 1) {
		//we are not heading in the exact direction we want to go...
		if(direction_cross > 0.98) {
			//close enough, dont mess with the thrust crap, just put them on course
			m_body->setAngularVelocity(Ga::GaVec3(0, 0, 0));
			Ga::GaVec3 vel = Ga::GaVec3(m_direction);
			vel *= m_velocity;
			m_body->setLinearVelocity(vel);
			return false;
		}
		
		//else, we need some turning action... apply our "turning" forces
#ifndef WIN32
#warning TODO: apply turning force (torque)
#endif
		m_body->applyTorque(Ga::GaVec3(0, 0, 0));
		
		if(direction_cross < 0.5) {
			//we have a lot of turning to do, let the caller know in case they want to delay acceleration
			return true;
		}
	}
#endif
	return false;
}

void DestinyManager::SendJumpOut(uint32 stargateID) const {
	std::vector<PyTuple *> updates;
	
	{
	DoDestiny_Stop du;
	du.entityID = m_self->GetID();
	updates.push_back(du.Encode());
	}

	{
	//send a warping special effects update...
	DoDestiny_OnSpecialFX10 effect;
	effect.entityID = m_self->GetID();
	effect.targetID = stargateID;
	effect.effect_type = "effects.JumpOut";
	effect.isOffensive = 0;
	effect.start = 1;
	effect.active = 0;
	updates.push_back(effect.Encode());
	}
	
	SendDestinyUpdate(updates, false);
}


void DestinyManager::SendTerminalExplosion() const {
	std::vector<PyTuple *> updates;
	
	{
		//send a warping special effects update...
		DoDestiny_TerminalExplosion du;
		du.entityID = m_self->GetID();
		du.unknown = 1206;	//no idead right now.
		updates.push_back(du.Encode());
	}
	
	SendDestinyUpdate(updates, false);
}

void DestinyManager::SendJumpIn() const {
	//hacked for simplicity... I dont like jumping in until we have
	//jumping in general much better quantified.
	
	std::vector<PyTuple *> updates;
	
	DoDestiny_OnSpecialFX10 effect;
	effect.effect_type = "effects.JumpDriveIn";
	effect.entityID = m_self->GetID();
	effect.isOffensive = 0;
	effect.start = 1;
	effect.active = 0;
	updates.push_back(effect.Encode());

	DoDestiny_SetSpeedFraction ssf;
	ssf.entityID = m_self->GetID();
	ssf.fraction = 0.0;
	updates.push_back(ssf.Encode());

	DoDestiny_SetBallVelocity sbv;
	sbv.entityID = m_self->GetID();
	sbv.x = 0.0;
	sbv.y = 0.0;
	sbv.z = 0.0;
	updates.push_back(sbv.Encode());

	SendDestinyUpdate(updates, false);
}

//this should only be available on gates.
void DestinyManager::SendGateActivity() const {
	DoDestiny_OnSpecialFX10 du;
	du.entityID = m_self->GetID();
	du.effect_type = "effects.GateActivity";
	du.isOffensive = 0;
	du.start = 1;
	du.active = 0;
	
	PyTuple *tmp = du.Encode();
	SendSingleDestinyUpdate(&tmp);	//consumed
}

void DestinyManager::SendSetState(const SystemBubble *b) const {
	DoDestiny_SetState ss;
	
	ss.stamp = GetStamp();
	ss.ego = m_self->GetID();
	m_system->MakeSetState(b, ss);

	PyTuple *tmp = ss.Encode();
	SendSingleDestinyUpdate(&tmp, true);	//consumed
}

void DestinyManager::SendBallInfoOnUndock(bool update) const {
	std::vector<PyTuple *> updates;

    DoDestiny_SetBallMassive sbmassive;
    sbmassive.entityID = m_self->GetID();
    sbmassive.is_massive = 0;
    updates.push_back(sbmassive.Encode());

    DoDestiny_SetBallMass sbmass;
    sbmass.entityID = m_self->GetID();
    sbmass.mass = m_self->Item()->GetAttribute(AttrMass).get_float();
    updates.push_back(sbmass.Encode());

    DoDestiny_SetBallVelocity sbvelocity;
    sbvelocity.entityID = m_self->GetID();
    sbvelocity.x = m_velocity.x;
    sbvelocity.y = m_velocity.y;
    sbvelocity.z = m_velocity.z;
    updates.push_back(sbvelocity.Encode());

    SendDestinyUpdate(updates, true);	//consumed
}

void DestinyManager::SendBoardShip(const ShipRef boardShipRef) const {

	std::vector<PyTuple *> updates;

    DoDestiny_SetBallInteractive du_setBallInteractiveCapsule;
    du_setBallInteractiveCapsule.entityID = boardShipRef->itemID();
    du_setBallInteractiveCapsule.interactive = 1;
    updates.push_back(du_setBallInteractiveCapsule.Encode());

	SendDestinyUpdate(updates, false);
}

void DestinyManager::SendEjectShip(const ShipRef capsuleRef, const ShipRef oldShipRef) const {
    //DoDestinyUpdate
        //addball for only the capsule.

    //DoDestinyUpdate
        //AddBalls:
            //old ship (STOP, w/ship and mass)
            //capsule (STOP, w/ship and mass)
            //and our newbie ship (STOP, w/ship and mass)
        //a bunch of OnModuleAttributeChange and OnGodmaShipEffect for old ship and capsule

    //DoDestinyUpdate
        //effects.Jettison
        //SetBallInteractive (old ship, false)
        //OnSlimItemChange (old ship, SlimItem)
        //SetBallInteractive (capsule, true)
        //OnSlimItemChange (capsule, SlimItem)
        //SetMaxSpeed (capsule)

	std::vector<PyTuple *> updates;

    DoDestiny_OnSpecialFX10 du_jettisonEffect;
	du_jettisonEffect.entityID = m_self->GetID();
	du_jettisonEffect.effect_type = "effects.Jettison";
	du_jettisonEffect.isOffensive = 0;
	du_jettisonEffect.start = 1;
	du_jettisonEffect.active = 0;
    updates.push_back(du_jettisonEffect.Encode());

    DoDestiny_SetBallInteractive du_setBallInteractiveCapsule;
    du_setBallInteractiveCapsule.entityID = capsuleRef->itemID();
    du_setBallInteractiveCapsule.interactive = 1;
    updates.push_back(du_setBallInteractiveCapsule.Encode());

    DoDestiny_SlimItem du_slimItemCapsule;
//    du_slimItemCapsule.allianceID = m_self->CastToClient()->GetAllianceID();
//    du_slimItemCapsule.bounty = 0.0;
//    du_slimItemCapsule.charID = 0;
//    du_slimItemCapsule.color = 0;
//    du_slimItemCapsule.corpID = 0;
//    du_slimItemCapsule.groupID = 0;
//    du_slimItemCapsule.itemID = 0;
//    du_slimItemCapsule.modules = ;
//    du_slimItemCapsule.ownerID = 0;
//    du_slimItemCapsule.securityStatus = 0.0;
//    du_slimItemCapsule.typeID = 0;
//    du_slimItemCapsule.warFactionID = 0;

    DoDestiny_SetBallInteractive du_setBallInteractiveOldShip;
    du_setBallInteractiveOldShip.entityID = oldShipRef->itemID();
    du_setBallInteractiveOldShip.interactive = 0;
    updates.push_back(du_setBallInteractiveOldShip.Encode());

    DoDestiny_SlimItem du_slimItemOldShip;
    //du_slimItemOldShip.

    // Set Capsule's max velocity:
    DoDestiny_SetMaxSpeed du_setMaxSpeed;
    du_setMaxSpeed.entityID = capsuleRef->itemID();
    du_setMaxSpeed.speed = capsuleRef->GetAttribute(AttrMaxVelocity).get_float();
    updates.push_back(du_setMaxSpeed.Encode());

	SendDestinyUpdate(updates, false);
}

void DestinyManager::SendJettisonCargo(const InventoryItemRef itemRef) const {
/*	std::vector<PyTuple *> updates;
	
	DoDestiny_OnSpecialFX10 effect;
	effect.effect_type = "effects.JumpDriveIn";
	effect.entityID = m_self->GetID();
	effect.isOffensive = 0;
	effect.start = 1;
	effect.active = 0;
	updates.push_back(effect.Encode());

    DoDestiny

	SendDestinyUpdate(updates, false);
*/}

void DestinyManager::SendAnchorDrop(const InventoryItemRef itemRef) const {
	std::vector<PyTuple *> updates;
	
	DoDestiny_OnSpecialFX10 effect;
	effect.effect_type = "effects.AnchorDrop";
	effect.entityID = itemRef->itemID();
	effect.isOffensive = 0;
	effect.start = 1;
	effect.active = 0;
	updates.push_back(effect.Encode());

	SendDestinyUpdate(updates, false);
}

void DestinyManager::SendAnchorLift(const InventoryItemRef itemRef) const {
	std::vector<PyTuple *> updates;
	
	DoDestiny_OnSpecialFX10 effect;
	effect.effect_type = "effects.AnchorLift";
    effect.entityID = itemRef->itemID();
	effect.isOffensive = 0;
	effect.start = 1;
	effect.active = 0;
	updates.push_back(effect.Encode());

	SendDestinyUpdate(updates, false);
}

void DestinyManager::SendCloakShip(const ShipRef shipRef, const bool IsWarpSafe) const {
	std::vector<PyTuple *> updates;
	
  	DoDestiny_OnSpecialFX10 effect;
	effect.effect_type = "effects.Cloak";
    effect.entityID = shipRef->itemID();
	effect.isOffensive = 0;
	effect.start = 1;
	effect.active = 0;
	updates.push_back(effect.Encode());

	SendDestinyUpdate(updates, false);
}

void DestinyManager::SendUncloakShip(const ShipRef shipRef) const {
	std::vector<PyTuple *> updates;
	
	DoDestiny_OnSpecialFX10 effect;
	effect.effect_type = "effects.Uncloak";
    effect.entityID = shipRef->itemID();
	effect.isOffensive = 0;
	effect.start = 1;
	effect.active = 0;
	updates.push_back(effect.Encode());

	SendDestinyUpdate(updates, false);
}
