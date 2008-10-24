/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "EvemuPCH.h"

using namespace Destiny;

static const double DESTINY_UPDATE_RANGE = 1.0e8;	//totally made up. a more complex spatial partitioning system is needed.
static const double SPACE_FRICTION = 1.0e6;			//straight from client. Do not change.
static const double SPACE_FRICTION_SQUARED = SPACE_FRICTION*SPACE_FRICTION;
static const double TIC_DURATION_IN_SECONDS = 1.0;	//straight from client. Do not change.
static const double FOLLOW_BAND_WIDTH = 100.0f;	//totally made up

uint32 DestinyManager::m_stamp(40000);	//completely arbitrary starting point.
Timer DestinyManager::m_stampTimer(TIC_DURATION_IN_SECONDS * 1000, true);	//accurate timing is essential.

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
  m_targetEntity(NULL),
  m_radius(1.0),
  m_mass(1.0),
  m_maxShipVelocity(1.0),
  m_shipAgility(1.0),
  m_shipInertia(1.0),
  m_warpState(NULL)
{
	//do not touch m_self here, it may not be fully constructed.
}

DestinyManager::~DestinyManager() {
	delete m_warpState;
}

void DestinyManager::Process() {
	//process 1 second worth of destiny activities
	ProcessTic();
}

void DestinyManager::SendSingleDestinyUpdate(PyRepTuple **up, bool self_only) const {
	std::vector<PyRepTuple *> updates(1, *up);
	*up = NULL;
	std::vector<PyRepTuple *> events;
	//consumes updates and events
	SendDestinyUpdate(updates, events, self_only);
}

void DestinyManager::SendDestinyUpdate(std::vector<PyRepTuple *> &updates, bool self_only) const {
	std::vector<PyRepTuple *> events;
	//consumes updates and events
	SendDestinyUpdate(updates, events, self_only);
}

void DestinyManager::SendDestinyUpdate(std::vector<PyRepTuple *> &updates, std::vector<PyRepTuple *> &events, bool self_only) const {
	if(self_only) {
		_log(DESTINY__TRACE, "[%lu] Sending destiny update to self.", GetStamp());
		std::vector<PyRepTuple *>::iterator cur, end;
		
		cur = updates.begin();
		end = updates.end();
		for(; cur != end; cur++) {
			PyRepTuple *t = *cur;
			m_self->QueueDestinyUpdate(&t);
			delete t;	//they are not required to consume it.
		}
		updates.clear();
		
		cur = events.begin();
		end = events.end();
		for(; cur != end; cur++) {
			PyRepTuple *t = *cur;
			m_self->QueueDestinyEvent(&t);
			delete t;	//they are not required to consume it.
		}
		events.clear();
	} else {
		_log(DESTINY__TRACE, "[%lu] Broadcasting destiny update (%d, %d)", GetStamp(), updates.size(), events.size());
		//this should really be something like "bubblecast"
		m_system->RangecastDestiny(m_position, DESTINY_UPDATE_RANGE, updates, events);
	}
}

void DestinyManager::_UpdateDerrived() {
	m_maxVelocity = m_maxShipVelocity * m_activeSpeedFraction;
	m_velocityAdjuster = exp(- (SPACE_FRICTION * TIC_DURATION_IN_SECONDS) / (m_mass * m_shipAgility));
	m_accelerationFactor = (SPACE_FRICTION*m_maxVelocity) / (m_mass * m_shipAgility);
	
	_log(PHYSICS__TRACE, "Entity %lu has derrived: maxVelocity=%f, velocityAdjuster=%f, accelerationFactor=%f",
		m_self->GetID(), m_maxVelocity, m_velocityAdjuster, m_accelerationFactor);
}

void DestinyManager::ProcessTic() {
	
	_log(PHYSICS__TRACEPOS, "[%d] Entity %lu starts at (%.3f, %.3f, %.3f) with velocity (%f, %f, %f)=%.1f in mode %s",
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
		if(m_targetEntity != NULL) {
			_Follow();
		} else {
			//nobody to follow?
			_log(PHYSICS__TRACE, "Entity %lu has nothing to follow. Stopping.",
				m_self->GetID() );
			Stop();
		}
		break;
		
	case DSTBALL_STOP:
		if(m_velocity.isNotZero()) {
			if(m_velocity.y > 0) {
				//no freakin clue what this is all about, maybe to dampen vertical movement.
				m_velocity.y = (m_velocity.y-(m_velocity.y*.07))/1.07;
			}
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
    this was my idea of how it should be done, which dosent agree with
    the client.

	GVector targetDirection = GVector(m_position, goal);
	double distance = targetDirection.normalize();
	_log(PHYSICS__TRACEPOS, "Entity %lu: Moving towards (%.1f, %.1f, %.1f) which is %f away.",
		m_self->GetID(),
		goal.x, goal.y, goal.z,
		distance);
	distance -= FOLLOW_BAND_WIDTH;
	
	if(distance <= m_targetDistance) {
		//there has to be some band of distance that causes us not to move.
		if((m_targetDistance-distance) < (FOLLOW_BAND_WIDTH+FOLLOW_BAND_WIDTH)) {
			//close enough.
			_log(PHYSICS__TRACEPOS, " Entity %lu is %f/%f away from followed entity. Close enough. Not moving.", m_self->GetID());
			m_activeSpeedFraction = 0.0f;
			_UpdateDerrived();
			return;
		}
		//swap around the direction, see what that does...
		targetDirection *= -1.0f;
		
		_log(PHYSICS__TRACEPOS, " Entity %lu is %f/%f away from followed entity. Should prolly back off...", m_self->GetID());
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

	const GPoint &target_point = m_targetEntity->GetPosition();
	
	GVector them_to_us(target_point, m_position);
	them_to_us.normalize();

	double desired_distance = 
		m_radius + 
		m_targetEntity->GetRadius() + 
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
	
	_MoveAccel(calc_acceleration);
}

void DestinyManager::_MoveAccel(const GVector &calc_acceleration) {
	_log(PHYSICS__TRACEPOS, "Entity %lu: Goal Point (%.1f, %.1f, %.1f) with accel (%f, %f, %f)",
		m_self->GetID(),
		m_targetPoint.x, m_targetPoint.y, m_targetPoint.z,
		calc_acceleration.x, calc_acceleration.y, calc_acceleration.z);

	double mass_times_agility = m_mass * m_shipAgility;
	
	GVector velocity_change_and_adjust = 
		(calc_acceleration * TIC_DURATION_IN_SECONDS) + m_velocity*(1 - m_velocityAdjuster);
	
	GVector max_velocity_times_friction = calc_acceleration * mass_times_agility;	//m*kg*agi/s^2 = m*fric/s
	
	GVector friction_application = max_velocity_times_friction * (m_velocityAdjuster - 1);	//m*kg*agi/s^2 = m*fric/s
	
	GVector position_change_sum = 
		friction_application/SPACE_FRICTION + velocity_change_and_adjust;
	position_change_sum *= mass_times_agility/SPACE_FRICTION;	//yeilds m

	GVector position_adjusted = position_change_sum + m_position;	//m
	m_position = position_adjusted;
	
	
	//now on to velocity:

	

	//velocity_to_m_us:
	GVector maxVelocityVector = max_velocity_times_friction / SPACE_FRICTION;	//m/s
	
	GVector allowed_acceleration = maxVelocityVector - m_velocity;	//m/s
	
	GVector allowed_acceleration_adjusted = allowed_acceleration * m_velocityAdjuster;	//m/s
	
	GVector velocity_adjusted = maxVelocityVector - allowed_acceleration_adjusted;  //m/s
	m_velocity = velocity_adjusted;


	
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
	
	_log(PHYSICS__TRACE, " Entity %lu starting warp to (%f, %f, %f) at distance %.2f",
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
	_log(PHYSICS__TRACE, " Entity %lu has new warp target of (%f, %f, %f) which is at distance %f",
		m_self->GetID(),
		m_targetPoint.x, m_targetPoint.y, m_targetPoint.z,
		warp_distance);

	//the client clears massive during warp, presumably so you
	//do not run into things while warping.
#ifndef WIN32
#warning TODO: set massive to false!
#endif


	double warp_speed = m_system->GetWarpSpeed();
	if(1.5*warp_distance < warp_speed) {//not positive on this conditional
		warp_speed = 1.5*warp_distance;
		_log(PHYSICS__TRACEPOS, "Adjusting warp speed to %f for short warp.", warp_speed);
	} else {
		_log(PHYSICS__TRACEPOS, "Warp speed in system %lu is %f", m_system->GetID(), warp_speed);
	}
	
	const double log_warp_speed_over_three__ = log(warp_speed / 3.0f);
	
	const double warp_acceleration_time = log_warp_speed_over_three__ / 3.0f;	//name guessed.
	
	double warp_slow_time = log_warp_speed_over_three__ - 2;
	warp_slow_time *= warp_speed;
	warp_slow_time += warp_distance * 3.0f;
	warp_slow_time /= warp_speed * 3.0f;	//v40 ~9.1105
	
	_log(PHYSICS__TRACEPOS, "Warp will accelerate for %f s, then slow down at %f s", warp_acceleration_time, warp_slow_time);
	
	_log(PHYSICS__TRACEPOS, "Opposite warp direction is (%.13f, %.13f, %.13f)",
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
	m_self->Bubble()->Remove(m_self);
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
		
		_log(PHYSICS__TRACEPOS, "Entity %lu: Warp Accelerating: velocity %f m/s with %f m left to go.", 
			m_self->GetID(),
			velocity_magnitude, dist_remaining);
		
	} else if(seconds_into_warp < m_warpState->slow_time) {
		printf("Warp Peak Speed:\n");
		//dont simplify this, the roundoff error caused by all the extra
		// multiply/divide crap they is major enough as to cause problems
		double delta_t = (m_warpState->acceleration_time * 3.0f) - ((seconds_into_warp * 3.0f)+1.0f);
		double delta_s = (m_warpState->speed * delta_t)/(-3.0f);
		dist_remaining = m_warpState->total_distance - delta_s;

		velocity_magnitude = m_warpState->speed;
		
		_log(PHYSICS__TRACEPOS, "Entity %lu: Warp Cruising: velocity %f m/s with %f m left to go.", 
			m_self->GetID(),
			velocity_magnitude, dist_remaining);
	} else {
		//warp_completely_done
		double v58 = (((m_warpState->acceleration_time + (m_warpState->total_distance/m_warpState->speed) - seconds_into_warp) )) - (2.0f/3.0f);
		
		velocity_magnitude = exp(v58) * m_warpState->speed / 3.0f;
		
		dist_remaining = velocity_magnitude;

		if(velocity_magnitude < 0)
			velocity_magnitude = -velocity_magnitude;
		
		_log(PHYSICS__TRACEPOS, "Entity %lu: Warp Slowing: velocity %f m/s with %f m left to go.", 
			m_self->GetID(),
			velocity_magnitude, dist_remaining);

		//note, this should actually be checked AFTER we change new_velocity.
		//but hey, it dosent get copied into ball.velocity until later anyhow.
		if(velocity_magnitude < m_maxShipVelocity) {
			stop = true;
		}
	}
	
	GVector vector_to_us = m_warpState->normvec_them_to_us * dist_remaining;
	
	m_position = m_targetPoint + vector_to_us;
	m_velocity = m_warpState->normvec_them_to_us * (-velocity_magnitude);
	
	if(stop) {
		_log(PHYSICS__TRACEPOS, "Entity %lu: Warp completed. Exit velocity %f m/s with %f m left to go.", 
			m_self->GetID(),
			velocity_magnitude, dist_remaining);
		//they re-calculated the velocity, but it was exactly the same..
		// our calc may only be accurate to 6 decimals for some reason..
		delete m_warpState;
		m_warpState = NULL;
		//put ourself back into a bubble.
		m_system->bubbles.UpdateBubble(m_self);
		Stop(false);	//no updates, client is doing this too.
	}
}

//this is still under construction. Its not working well right now.
void DestinyManager::_Orbit() {
#define PVN(v) _log(PHYSICS__TRACEPOS, "Entity %lu: " #v ": (%.15e, %.15e, %.15e)   len=%.15e", \
			m_self->GetID(), \
			v.x, v.y, v.z, v.length() )
		
	const GPoint &orbit_point = m_targetEntity->GetPosition();
	
	GVector delta(m_position, orbit_point);
	PVN(delta);
	double current_distance = delta.normalize();
	PVN(delta);
	
	double something = 0;
	double desired_distance = 
		m_radius + 
		m_targetEntity->GetRadius() + 
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
	if(m_targetEntity == who) {
		m_targetEntity = NULL;
		
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
			_log(DESTINY__DEBUG, "%lu: Our target entity has gone away. Stopping.", m_self->GetID());
			Stop();
			break;
			
		//no default on purpose
		}
	}
}
	
//Global Actions:
void DestinyManager::Stop(bool update) {
	if(State == DSTBALL_STOP)
		return;

	if(State == DSTBALL_WARP && m_warpState != NULL) {
		//warp aborted!
		delete m_warpState;
		m_warpState = NULL;
		//put ourself back into a bubble.
		m_system->bubbles.UpdateBubble(m_self);
	}
	
	m_targetEntity = NULL;
	
	//redude velocity to 0, applying reverse thrust until we get there.
//	m_targetPoint = m_position - (m_velocity * 1.0e6);	//opposite direction
	m_activeSpeedFraction = 0.0f;
	_UpdateDerrived();
	
	State = DSTBALL_STOP;

	if(update) {
		DoDestiny_Stop du;
		du.entityID = m_self->GetID();
		
		PyRepTuple *tmp = du.FastEncode();
		SendSingleDestinyUpdate(&tmp);	//consumed
	}
}

void DestinyManager::Halt(bool update) {
	m_targetEntity = NULL;
	m_velocity = GVector(0, 0, 0);
	m_activeSpeedFraction = 0.0f;
	_UpdateDerrived();
	
	State = DSTBALL_STOP;

	//ensure that our bubble is correct.
	m_system->bubbles.UpdateBubble(m_self);
	
	if(update) {
		std::vector<PyRepTuple *> updates;
		
		{
		DoDestiny_SetSpeedFraction du;
		du.entityID = m_self->GetID();
		du.fraction = 0.0;
	
		updates.push_back(du.FastEncode());
		}
		{
		DoDestiny_SetBallVelocity du;
		du.entityID = m_self->GetID();
		du.x = 0.0;
		du.y = 0.0;
		du.z = 0.0;
	
		updates.push_back(du.FastEncode());
		}
		
		SendDestinyUpdate(updates, false);
	}
}
	
//Local Movement:
void DestinyManager::Follow(SystemEntity *who, double distance, bool update) {
	if(State == DSTBALL_FOLLOW && m_targetEntity == who && m_targetDistance == distance)
		return;
	
	State = DSTBALL_FOLLOW;
	m_targetEntity = who;
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
		
		PyRepTuple *tmp = du.FastEncode();
		SendSingleDestinyUpdate(&tmp);	//consumed
	}
}

void DestinyManager::Orbit(SystemEntity *who, double distance, bool update) {
	if(State == DSTBALL_ORBIT && m_targetEntity == who && m_targetDistance == distance)
		return;
	
	State = DSTBALL_ORBIT;
	m_stateStamp = GetStamp()+1;
	
	m_targetEntity = who;
	m_targetDistance = distance;
    /*if(m_userSpeedFraction == 0.0f)
        m_userSpeedFraction = 1.0f;*/	//dosent seem to do this.
	if(m_activeSpeedFraction != m_userSpeedFraction) {
		m_activeSpeedFraction = m_userSpeedFraction;
		_UpdateDerrived();
	}
	
	if(update) {
		DoDestiny_Orbit du;
		du.entityID = m_self->GetID();
		du.orbitEntityID = who->GetID();
		du.distance = uint32(distance);
		
		PyRepTuple *tmp = du.FastEncode();
		SendSingleDestinyUpdate(&tmp);	//consumed
	}
}

void DestinyManager::SetShipCapabilities(const InventoryItem *ship)
{
	double mass = ship->mass();
	double radius = ship->radius();
	double Inertia = ship->Inertia();
	double agility = ship->agility();
	int maxVelocity = ship->maxVelocity();

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
	
	_log(PHYSICS__TRACE, "Entity %lu is using ship attributes: Radius=%f, Mass=%f, maxVelocity=%f, agility=%f, inertia=%f",
		m_self->GetID(), m_radius, m_mass, m_maxShipVelocity, m_shipAgility, m_shipInertia);
	
	_UpdateDerrived();
}

void DestinyManager::SetPosition(const GPoint &pt, bool update) {
	//m_body->setPosition( pt );
	m_position = pt;
	_log(PHYSICS__TRACE, "Entity %lu set its position to (%.1f, %.1f, %.1f)",
		m_position.x, m_position.y, m_position.z );
	
	if(update) {
		DoDestiny_SetBallPosition du;
		du.entityID = m_self->GetID();
		du.x = pt.x;
		du.y = pt.y;
		du.z = pt.z;
		
		PyRepTuple *tmp = du.FastEncode();
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
		
		PyRepTuple *tmp = du.FastEncode();
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
		
		PyRepTuple *tmp = du.FastEncode();
		SendSingleDestinyUpdate(&tmp);	//consumed
	}
}

void DestinyManager::WarpTo(const GPoint &where, double distance, bool update) {
	SetSpeedFraction(1.0, update);

	if(m_warpState != NULL) {
		delete m_warpState;
		m_warpState = NULL;
	}
	
	State = DSTBALL_WARP;
	m_targetEntity = NULL;
	m_targetPoint = where;
	m_targetDistance = distance;
	
	if(update) {
		std::vector<PyRepTuple *> updates;
		
		{
		DoDestiny_WarpTo du;
		du.entityID = m_self->GetID();
		du.dest_x = where.x;
		du.dest_y = where.y;
		du.dest_z = where.z;
		du.distance = 15000;
		du.u5 = 30;
		
		updates.push_back(du.FastEncode());
		}
	
		{
		//send a warping special effects update...
		DoDestiny_OnSpecialFX10 du;
		du.effect_type = "effects.Warping";
		du.entityID = m_self->GetID();
		du.isOffensive = 0;
		du.start = 1;
		du.active = 0;
		
		updates.push_back(du.FastEncode());
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
			return(false);
		}
		
		//else, we need some turning action... apply our "turning" forces
#ifndef WIN32
#warning TODO: apply turning force (torque)
#endif
		m_body->applyTorque(Ga::GaVec3(0, 0, 0));
		
		if(direction_cross < 0.5) {
			//we have a lot of turning to do, let the caller know in case they want to delay acceleration
			return(true);
		}
	}
#endif
	return(false);
}

void DestinyManager::SendJumpOut(uint32 stargateID) const {
	std::vector<PyRepTuple *> updates;
	
	{
	DoDestiny_Stop du;
	du.entityID = m_self->GetID();
	updates.push_back(du.FastEncode());
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
	std::vector<PyRepTuple *> updates;
	
	{
		//send a warping special effects update...
		DoDestiny_TerminalExplosion du;
		du.entityID = m_self->GetID();
		du.unknown = 1206;	//no idead right now.
		updates.push_back(du.FastEncode());
	}
	
	SendDestinyUpdate(updates, false);
}

void DestinyManager::SendJumpIn() const {
	//hacked for simplicity... I dont like jumping in until we have
	//jumping in general much better quantified.
	
	std::vector<PyRepTuple *> updates;
	
	DoDestiny_OnSpecialFX10 effect;
	effect.effect_type = "effects.JumpDriveIn";
	effect.entityID = m_self->GetID();
	effect.isOffensive = 0;
	effect.start = 1;
	effect.active = 0;
	updates.push_back(effect.FastEncode());

	DoDestiny_SetSpeedFraction ssf;
	ssf.entityID = m_self->GetID();
	ssf.fraction = 0.0;
	updates.push_back(ssf.FastEncode());

	DoDestiny_SetBallVelocity sbv;
	sbv.entityID = m_self->GetID();
	sbv.x = 0.0;
	sbv.y = 0.0;
	sbv.z = 0.0;
	updates.push_back(sbv.FastEncode());

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
	
	PyRepTuple *tmp = du.FastEncode();
	SendSingleDestinyUpdate(&tmp);	//consumed
}

void DestinyManager::SendAddBall() const {
	DoDestiny_AddBall addball;
	m_self->MakeAddBall(addball, GetStamp());

	PyRepTuple *tmp = addball.FastEncode();
	SendSingleDestinyUpdate(&tmp);	//consumed
}

void DestinyManager::SendSetState(const SystemBubble *b) const {
	DoDestiny_SetState ss;
	
	ss.stamp = GetStamp();
	ss.ego = m_self->GetID();
	m_system->MakeSetState(b, ss);

	PyRepTuple *tmp = ss.FastEncode();
	SendSingleDestinyUpdate(&tmp, true);	//consumed
}

void DestinyManager::SendRemoveBall() const {
	DoDestiny_RemoveBalls du;
	du.balls.push_back(m_self->GetID());
	
	PyRepTuple *tmp = du.FastEncode();
	SendSingleDestinyUpdate(&tmp);	//consumed
}

void DestinyManager::DoTargetAdded(SystemEntity *who) const {
	DoDestiny_OnDamageStateChange odsc;
	odsc.entityID = who->GetID();
	odsc.state = who->MakeDamageState();
	PyRepTuple *tmp = odsc.FastEncode();
	SendSingleDestinyUpdate(&tmp);	//consumed
	
    Notify_OnTarget te;
    te.mode = "add";
	te.targetID = who->GetID();
	PyRepTuple *tmp2 = te.FastEncode();
	m_self->QueueDestinyEvent(&tmp2);
	delete tmp2;
}












