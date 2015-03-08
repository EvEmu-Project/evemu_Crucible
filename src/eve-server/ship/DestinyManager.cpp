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
    Author:        Zhur
    Updated:        Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "PyServiceMgr.h"
#include "npc/NPC.h"
#include "ship/DestinyManager.h"
#include "station/Station.h"
#include "system/BubbleManager.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

using namespace Destiny;

const float SPACE_FRICTION = 1.0e+6;        //straight from client. Do not change.
const double SPACE_FRICTION_SQUARED = SPACE_FRICTION*SPACE_FRICTION;
const float TIC_DURATION_IN_SECONDS = 1.0f;    //straight from client. Do not change.
static const float DESTINY_UPDATE_RANGE = 1.0e8;    //totally made up. a more complex spatial partitioning system is needed.
static const float FOLLOW_BAND_WIDTH = 100.0f;    //totally made up
static const uint32 minWarpDistance = 100000;

uint32 DestinyManager::m_stamp(1000);    //used for warping and client login timer
Timer DestinyManager::m_stampTimer(static_cast<int32>(TIC_DURATION_IN_SECONDS * 1000), true);

DestinyManager::DestinyManager(SystemEntity *self, SystemManager *system)
: m_self(self),
m_system(system),
//  m_lastDestinyTime(Timer::GetTimeSeconds()),
m_position( NULL_ORIGIN ),    // right in the middle of the star!
m_maxVelocity(1.0),
m_shipMaxAccelTime(0.0),
State(DSTBALL_STOP),
m_moveTimer(0.0),
m_userSpeedFraction(0.0f),
m_activeSpeedFraction(0.0f),
m_targetDistance(0.0),
m_stopDistance(0),
m_radius(1.0),
m_mass(1.0f),
m_massMKg(1.0f),
m_shipWarpSpeed(0.0f),
m_maxShipVelocity(1.0),
m_shipAgility(1.0),
m_shipInertiaModifier(1.0),
m_warpStrength(1),
m_warpCapacitorNeed(1.0),
m_warpAccelTime(1),
m_warpDecelTime(1),
m_warpState(NULL)
{
    //do not touch m_self here, it may not be fully constructed.
    m_targetEntity.first = 0;
    m_targetEntity.second = NULL;

    m_velocity = GVector( NULL_ORIGIN );
    m_targetPoint = GPoint( NULL_ORIGIN );
    m_moveDirection = GPoint( NULL_ORIGIN );

    m_inBubble = true;

    m_cloaked = false;
    m_stop = false;

    m_capCharge = 0;
}

DestinyManager::~DestinyManager() {
    delete m_warpState;
}

void DestinyManager::Process() {
    //check for and process Destiny State changes.
    ProcessState();
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

void DestinyManager::SendDestinyUpdate( std::vector<PyTuple*>& updates, std::vector<PyTuple*>& events, bool self_only ) const {
    if( self_only ) {
        _log( DESTINY__TRACE, "[%u] Sending destiny update (%lu, %lu) to self (%u).", GetStamp(), updates.size(), events.size(), m_self->GetID() );

        std::vector<PyTuple*>::iterator cur = updates.begin();
        for(; cur != updates.end(); cur++) {
            PyTuple* t = *cur;
            m_self->QueueDestinyUpdate( &t );
            PySafeDecRef( t ); //they are not required to consume it.
        }
        updates.clear();

        cur = events.begin();
        for(; cur != events.end(); cur++) {
            PyTuple* t = *cur;
            m_self->QueueDestinyEvent( &t );
            PySafeDecRef( t ); //they are not required to consume it.
        }
        events.clear();
    } else if( m_self->Bubble() ) {
        _log( DESTINY__TRACE, "[%u] Broadcasting destiny update (%lu, %lu)", GetStamp(), updates.size(), events.size() );

        m_self->Bubble()->BubblecastDestiny( updates, events, "destiny" );
    } else {
        _log( DESTINY__ERROR, "[%u] Cannot broadcast destiny update (%lu, %lu); entity (%u) is not in any bubble.", GetStamp(), updates.size(), events.size(), m_self->GetID() );
    }
}

void DestinyManager::ProcessState() {
    switch(State) {
        case DSTBALL_STOP:
        case DSTBALL_MUSHROOM:
        case DSTBALL_BOID:
        case DSTBALL_TROLL:
        case DSTBALL_MINIBALL:      // used for missiles?
        case DSTBALL_MISSILE:
        case DSTBALL_FIELD:
        case DSTBALL_FORMATION:
        case DSTBALL_RIGID:         // item that never moves
            //no default on purpose
            break;

        case DSTBALL_GOTO: {
            _Move();
        } break;

        case DSTBALL_FOLLOW: {
            if (m_targetEntity.second) {
                _Follow();
            } else {
                //nobody to follow?
                _log(PHYSICS__TRACE, "DestinyManager::ProcessTic - Entity %u has nothing to follow. Stopping.",
                     m_self->GetID() );
                Stop();
            }
        } break;

        case DSTBALL_WARP: {
            /*
             * There are three stages of warp, which are functions of time, speed and distance:
             *
             *  1) Acceleration.
             *      this is a fixed attribute, which is roughly 9s to full warp speed for all ships
             *  2) Cruising.
             *      traveling at maximum warp speed
             *  3) Deceleration.
             *      this also is a fixed attribute, which is roughly 22s from full warp speed for all ships
             *
             *  Acceleration and Deceleration are logrithmic with finite caps (instead of infinity) at the ends.
             *      see also:  my notes in _InitWarp()
             */

            if (m_warpState) {
                //warp is in progress
                uint16 sec_into_warp = (GetStamp() - m_stateStamp);
                //  speed and distance formulas based on current warp distance
                if (m_warpState->accel)
                    _WarpAccel(sec_into_warp);
                else if (m_warpState->cruise)
                    _WarpCruise(sec_into_warp);
                else if (m_warpState->decel)
                    _WarpDecel(sec_into_warp);
                else // houston, we have a problem...
                    sLog.Error("Destiny::_UpdateVelocity", "Error!  Client %s (%u) Has WarpState but checks are false.", m_self->GetName(), m_self->GetID());
                break;
            }

            //first check the angle between our warp vector and our velocity vector.
            GVector warp_vector(m_position, m_targetPoint);
            warp_vector.normalize();
            GVector vel_normalized(m_velocity);
            vel_normalized.normalize();
            float dot = warp_vector.dotProduct(vel_normalized);    //cos(angle) between vectors.
            if (dot < (1.0 - 0.01)) {    //this is about 8 degrees
                _Move();
                break;
            }

            //ok, so they are heading in the right direction now, see about
            //velocity. We need to be going 75% of max velocity.
            double needed_velocity2 = m_maxShipVelocity*0.75;   // this is a TOTAL, MODIFIED ship speed...and affected by web.
            needed_velocity2 *= needed_velocity2;
            if (m_velocity.lengthSquared() < needed_velocity2) {
                _Move();
                break;
            }

            _InitWarp();
        } break;

        case DSTBALL_ORBIT: {
            _Orbit();
        } break;
    }
}

//Velocity setting methods
void DestinyManager::SetSpeedFraction(float fraction, bool startMovement, bool undock) {
    // this is to start movement when setting fractional speeds from speedo in client and for undocking.
    if (startMovement) {
        State = DSTBALL_GOTO;
        m_stop = false;
    }

    // prevent multiple client calls to Stop() from resetting ship speed. (bug to be fixed)
    if (m_stop) return;

    /* movement is set according to speed fraction.         -allan 8Oct14
     *  m_userSpeedFraction is user-set speed control (fractional from speedo or full from goto, warp, align, follow, and stop).
     *   -> sets m_maxShipVelocity
     *  m_activeSpeedFraction holds current e^() value for time vs accel/decel
     *   -> sets m_velocity
     *  m_moveDirection holds current direction
     *   -> sets m_velocity
     *  m_targetPoint holds current target coords
     *   -> sets m_velocity
     */

    if (m_moveDirection.isZero()) {
        GVector moveVector(m_position, m_targetPoint);
        moveVector.normalize();
        m_moveDirection = moveVector;
    }

    m_userSpeedFraction = fraction;
    bool isMoving = false;
    if (m_activeSpeedFraction) isMoving = true;
    _UpdateVelocity(isMoving, undock);

    if (State == DSTBALL_WARP) {
        // set state to DSTBALL_GOTO so warp completion will decel properly
        State = DSTBALL_GOTO;
        return;
    }

    std::vector<PyTuple *> updates;

    DoDestiny_CmdSetSpeedFraction du;
    du.entityID = m_self->GetID();
    du.fraction = fraction;
    updates.push_back(du.Encode());

    DoDestiny_SetBallVelocity bv;
    bv.entityID = m_self->GetID();
    bv.x = m_velocity.x;
    bv.y = m_velocity.y;
    bv.z = m_velocity.z;
    updates.push_back(bv.Encode());

    if (m_self->IsNPC()) {
        DoDestiny_SetMaxSpeed ms;
        ms.entityID = m_self->GetID();
        ms.speedValue = m_maxVelocity;
        updates.push_back(ms.Encode());
    }

    //ensure that our bubble is correct.
    //m_system->bubbles.CheckBubble(m_self);

    SendDestinyUpdate(updates);
}

void DestinyManager::_UpdateVelocity(bool isMoving, bool undock) {

    m_moveTimer = GetTimeMSeconds();

    //start ship movement @ 1.5x max velocity for undocking from stations.
    // this simulates being forcefully "ejected" from station
    if (undock) {
        m_shipMaxAccelTime = 1.5f;
        m_activeSpeedFraction = 1.0;
        m_maxVelocity = m_maxShipVelocity * m_userSpeedFraction;
        m_velocity = (m_moveDirection * m_maxVelocity * m_activeSpeedFraction);
        return;
    }

    // use this to set a maximum acceleration time (based on ship variables),
    //  after which the server will set speed to min or max, depending on other variables.
    m_shipMaxAccelTime = (m_shipInertiaModifier * m_massMKg * -log(0.0015));
    /* if ((isMoving) && (m_warpState == NULL)) {
     *        double speed = m_maxVelocity * m_activeSpeedFraction;
     *        float fraction = (speed / m_maxShipVelocity);
     *        m_shipMaxAccelTime = (m_shipMaxAccelTime * fraction);
     *        m_velocity = (m_moveDirection * speed);
     *        m_maxVelocity = speed;
    } else */
    if (m_userSpeedFraction) {
        m_shipMaxAccelTime *= m_userSpeedFraction;      // for accel with user speeds <= 1.0
        m_maxVelocity = m_maxShipVelocity * m_userSpeedFraction;
    } else {
        m_shipMaxAccelTime = ((m_shipMaxAccelTime * m_activeSpeedFraction) +7);        // for decel with user speeds <= 1.0
        m_maxVelocity = m_maxShipVelocity * m_activeSpeedFraction;
    }

    if ((State == DSTBALL_WARP) && (m_warpState != NULL)) {
        //sLog.Warning("Destiny::_UpdateVelocity", "state = warp.  --Leaving _Warp");
        /*  comming out of warp, so decel from speed ship actually dropped out of warp at (set to m_speedToLeaveWarp),
         *  reset m_shipMaxAccelTime as a fraction of m_speedToLeaveWarp/m_maxShipVelocity
         *  to set decel from warp correctly, as m_speedToLeaveWarp varies with ship and/or distance.
         */
        float fraction = (m_speedToLeaveWarp / m_maxShipVelocity);
        m_shipMaxAccelTime = (m_shipMaxAccelTime *fraction);
        m_velocity = (m_moveDirection * m_speedToLeaveWarp);
        m_maxVelocity = m_speedToLeaveWarp;     //FIXME  this still isnt completely right.
    } else if (m_userSpeedFraction) {
        //sLog.Warning("Destiny::_UpdateVelocity", "user speed fraction != 0.  --Accel or change direction");
        //  see notes in _Move() for information relating to accel equations
        m_activeSpeedFraction = (1 - exp(-1000000 / (m_shipInertiaModifier * m_mass)));
        m_velocity = (m_moveDirection * (m_maxVelocity * m_activeSpeedFraction));
    } else if ((m_userSpeedFraction == 0) && (m_activeSpeedFraction == 0)) {
        //sLog.Warning("Destiny::_UpdateVelocity", "both speed fractions = 0.  --Halt");
        // ship Halt()ed (for whatever reason) so delete all velocity variables
        Halt();
    } else if (m_userSpeedFraction == 0) {
        //sLog.Warning("Destiny::_UpdateVelocity", "user speed fraction = 0.  --Stop");
        if (IsMoving()) {
            //sLog.Warning("Destiny::_UpdateVelocity", "ship is moving when stop called.");
            //  see notes in _Move() for information relating to decel equations
            m_velocity = (m_moveDirection * (m_maxShipVelocity * m_activeSpeedFraction));
        } else {
            //sLog.Warning("Destiny::_UpdateVelocity", "ship is not moving when stop called.");
            Halt();
        }
    } else if (m_activeSpeedFraction != 0) {
        //WARNING  conditional should very rarely arrive here.
        /* Stop() has been called with active movement not caught in previous conditional */
        sLog.Warning("Destiny::_UpdateVelocity", "active speed fraction != 0 and not caught in previous m_userSpeedFraction.");
    } else {
        //WARNING conditional should never arrive here.
        sLog.Error("Destiny::_UpdateVelocity", "Error!  Client %s (%u) Has No WarpState or Speed Fraction.", m_self->GetName(), m_self->GetID());
        assert(0);  // change this to a correct error-handling implementation
    }
}

//Global Actions:
void DestinyManager::Stop() {
    /*  not implemented yet in this version  -allan 4Mar15
    //Clear autopilot
    if( m_self->IsClient() )
        m_self->CastToClient()->SetAutoPilot(false);
    */

    if (State == DSTBALL_STOP) {
        //state is already at stop.
        // move along, nothing do to here.
        return;
    } else if  ((State == DSTBALL_WARP) && (!IsWarping()))  {
        //warp aborted before initalized.  standard Stop() applies.
        State = DSTBALL_GOTO;
    } else if (IsMoving()) {
        //stop called while moving
        // set state to GOTO so _UpdateVelocity() will let us decel correctly
        State = DSTBALL_GOTO;
    }

    SetSpeedFraction(0.0f);
    m_stop = true;

    DoDestiny_CmdStop du;
    du.entityID = m_self->GetID();

    PyTuple *tmp = du.Encode();
    SendSingleDestinyUpdate(&tmp);    //consumed
}

void DestinyManager::Halt() {
    /*  not implemented yet in this version  -allan 4Mar15
     / /*Clear autopilot
     if( m_self->IsClient() )
         m_self->CastToClient()->SetAutoPilot(false);
     */

    //  reset ALL movement variables and states.  calling this will set object to a COMPLETE and IMMEDIATE stop.
    State = DSTBALL_STOP;
    m_stop = true;

    GPoint position = m_targetPoint;
    position.normalize();
    GVector moveVector(m_position, (position * 1.0e16));
    moveVector.normalize();
    m_velocity = GVector(NULL_ORIGIN);
    m_moveTimer = 0.0;
    m_targetPoint = GPoint(NULL_ORIGIN);
    m_stopDistance = 0;
    m_targetEntity.first = 0;
    m_targetEntity.second = NULL;
    m_moveDirection = moveVector;
    m_targetDistance = 0.0;
    m_shipMaxAccelTime = 0;
    m_userSpeedFraction = 0;
    m_activeSpeedFraction = 0;
}

void DestinyManager::_Follow() {
    //  Follow is also used by client as AlignTo.

    // First check to see if our target has somehow been removed from space
    // _OR_ the player has left the target ship..
    // if so, then we need to call DestinyManager::Stop() to stop the ship from
    // following a non-existent space object or a pilot-less ship,
    // otherwise, proceed to continue following:

    if ( !m_system->get( m_targetEntity.first ) ) {
        // Our target was removed, so STOP
        Stop();
        return;
    }

    Client *targetClient = m_targetEntity.second->CastToClient();
    // Our target still exists, so do more checking:
    if (targetClient) {
        // We are orbiting a Client object, i.e. a player ship
        if (m_targetEntity.first != targetClient->GetShipID()) {
            // The client is no longer in the ship we were targeting, so STOP
            Stop();
            return;
        } else if (targetClient->Destiny()->IsWarping()) {
            // The target is warping.  stop following
            Stop();
            return;
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
    m_moveDirection = them_to_us;

    _Move();
}

void DestinyManager::_Move() {
    //apply our velocity to our position for 1 unit of time (a second)

    /* acceleration and deceleration are both logarithmic, and the server needs to keep up with client position.
     * this is another step towards getting ready for collision detection.
     *
     * formula for time taken to accelerate from v to V, from https://wiki.eveonline.com/en/wiki/Acceleration
     *
     *   t=IM(10^-6) * -ln(1-(v/V))
     *
     * as this uses the natural log, the higher the speed, the slower the acceleration, to the limits of ln(0)
     * since lim ln(x) = -INFINITY where x->0+. and ln(0) is undefined, we will use
     *
     *   m_shipMaxAccelTime = (m_shipInertiaModifier * m_massMKg * -log(0.001));
     *
     * to define the time it will take a given ship to reach 99.9% of m_maxShipVelocity, at which point,
     * the server will set m_velocity to m_maxShipVelocity (or 100%).
     *
     * to define speed at X time, we will use the following equation.
     *
     *   Vt = Vm * (1 - e(-t * 10^6 / IM))
     *
     * where
     * Vt = ships velocity at t
     * Vm = ships maximum velocity
     *  t = time
     *  I = ships inertia in s/kg
     *  M = ships mass in kg
     *  e = base of natural logarithms
     */

    //set current direction based on position and targetPoint.  this will keep ship aligned properly
    GVector moveVector(m_position, m_targetPoint);
    moveVector.normalize();

    //  this is for changing direction..
    // FIXME  not working right.
    //GVector vel_normalized = m_velocity;
    //vel_normalized.normalize();
    //GPoint direction = m_position + (vel_normalized  * 1.0e16);
    //GVector curDirection(m_velocity);
    //curDirection.normalize();
    //float dot = moveVector.dotProduct(curDirection);    //cos(angle) between vectors.

    double timeStamp = (GetTimeMSeconds() - m_moveTimer);
    double speed = 0.0;
    float speedFraction = 0.0f;
    std::string move = "";

    if (timeStamp > m_shipMaxAccelTime) {
        if (m_userSpeedFraction) {
            // ship has reached full speed
            m_activeSpeedFraction = 1.0f;
            speedFraction = m_activeSpeedFraction;
            move = "at max speed, going";
            speed = m_maxVelocity;
        } else {
            //ship has reached full stop
            // update position one final time and exit function
            m_position += m_velocity;
            //bring ship to complete stop
            Halt();
            return;
        }
        /*
    } else if(dot < (1.0 - 0.01)) {    //this is about 8 degrees
        //just changed direction.  reset velocity  FIXME needs work.....
        //  stilleto drops to 25% on 180* turn
        //  fernir drops to ~10% on 180* turn
        move = "changing direction";
        e = (1 - exp(-1000000 / (m_shipInertiaModifier * m_mass)));
        speed = (m_maxVelocity * e);
        m_velocity = (moveVector * speed);
    } else if(curDistance < m_stopDistance){  // this is for following
        //too close   FIXME needs work.....
        move = "too close";
        //m_velocity /= 2;
        */
    } else {
        /* accel in eve is logarithmic, following a modified bell curve, but capped to a specific time,
         *      based on ship class and maxSubWarpSpeed.
         * this function is defined above, and implemented here.
         */
        m_activeSpeedFraction = (1 - exp(-timeStamp * 1000000 / (m_shipInertiaModifier * m_mass)));
        speed = (m_maxVelocity * m_activeSpeedFraction);
        move = "accelerating";
        speedFraction = m_activeSpeedFraction;
        if (!m_userSpeedFraction) {
            //just decelerating
            /* decel is opposite of accel and uses inverse of same formula.
             * see notes above
             */
            move = "decelerating";
            speed = (m_maxVelocity - speed);
            speedFraction = (1 - m_activeSpeedFraction);
        }
    }

    //set postion and direction for this round of movement
    m_velocity = (moveVector * speed);
    m_position += m_velocity;
    m_moveDirection = moveVector;

    _log(PHYSICS__TRACE, "DestinyManager::_Move() - Entity %s(%u) is %s at %.4f m/s (%.4f) at %.3f seconds.", \
            m_self->GetName(), m_self->GetID(), move.c_str(), \
            speed, speedFraction, timeStamp);
    _log(PHYSICS__TRACE, "DestinyManager::_Move() - Entity %s(%u) Position: %.2f,%.2f,%.2f  velocity: %.3f,%.3f,%.3f", \
            m_self->GetName(), m_self->GetID(), \
            m_position.x, m_position.y, m_position.z, \
            m_velocity.x, m_velocity.y, m_velocity.z);
    _log(PHYSICS__TRACE, "DestinyManager::_Move() - Entity %s(%u) moveVector: %.3f,%.3f,%.3f  m_moveDirection: %.3f,%.3f,%.3f", \
            m_self->GetName(), m_self->GetID(), \
            moveVector.x, moveVector.y, moveVector.z, \
            m_moveDirection.x, m_moveDirection.y, m_moveDirection.z);
}

void DestinyManager::_InitWarp() {
    //init warp:

    // warp time and distance math
    //   allan 1Nov14 - 14Nov14
    //  rewrite 3jan15  to use distance instead of time for warping.  more accurate now, and covers ALL distances.
    //  calculation and implementation update   9Jan15      accuracy is within 1000m

    /*  my research into warp formulas, i have used these sites, with a few exerpts and ideas from each...
     *     https://wiki.eveonline.com/en/wiki/Acceleration
     *     http://oldforums.eveonline.com/?a=topic&threadID=1251486
     *     http://gaming.stackexchange.com/questions/115271/how-does-one-calculate-a-ships-agility
     *     http://eve-search.com/thread/1514884-0
     *     http://www.eve-search.com/thread/478431-0/page/1
     */

    /* this is my version of how warp should be timed and followed by the server.
     * checks here for distance < warp speed and distance < 2AU, (with all distances in meters)
     *  and adjusts accel/decel times accordingly
     *
     *   accel/decel are logrithmic per ccp (see above).
     *
     * times are as follows, per this table.  http://cdn1.eveonline.com/www/newssystem/media/65418/1/numbers_table.png
     *
     * all warps are in same time groups for all ships, except freighters and caps.
     * distance checks are seperated into 3 time groups, with subgroups for freighters and caps.
     *
     * the client seems to accept and agree with the math here.
     */
    bool cruise = false;
    double warpSpeedInMeters = (m_shipWarpSpeed * ONE_AU_IN_METERS);
    //TODO  ALL of these will need checks for ship speed and distance.  current categories are not enough play.
    if (m_targetDistance < 1000000001) {
        // < 1MKm, total time for inty is 15s.
        m_warpAccelTime = 6;
        m_warpDecelTime = 13;
    } else if (m_targetDistance < ONE_AU_IN_METERS) {
        // this covers 1MKm to 1AU
        // at 1MKm, total time for all ships except freighters is 23s.
        // freighters base time is 29s, and all other freighter warp distances are covered here.
        if (m_targetDistance > warpSpeedInMeters) {
            m_warpAccelTime = 8;
            m_warpDecelTime = 20; //21
        } else {
            m_warpAccelTime = 7;
            m_warpDecelTime = 16;
        }
    } else if (m_targetDistance < (ONE_AU_IN_METERS * 2)) {
        // this covers between 1AU and 2AU, and excludes freighters.
        // at 2AU, total time for all other ships except caps is 29s.
        // capitals base time is 30s, and all other capital warp distances are covered here
        if (m_targetDistance > warpSpeedInMeters) {
            m_warpAccelTime = 9;
            m_warpDecelTime = 22;  //21
        } else {
            m_warpAccelTime = 8;
            m_warpDecelTime = 20;  //21
        }
    } else {
        // this covers all other ships (except freighters and capitals) at all distances > 2AU.
        // for all distances > 2AU, 30s is base time.
        m_warpAccelTime = 9;
        m_warpDecelTime = 23;  //21
    }

    if (m_targetDistance > warpSpeedInMeters) cruise = true;

    /*  this is from http://community.eveonline.com/news/dev-blogs/warp-drive-active/
     * For the acceleration phase, k is 3.
     * For deceleration, k is 1.
     * x = e^(k*t)
     * x = distance in meters
     * t = time in seconds
     *
     * this gives distances as functions of time.
     *  the client seems to agree with this reasoning, and follows the same idea.
     *
     * short warps are different.  they do not agree with the time function, so i have
     * to do them different, and go by distance.
     */
    double accelDistance = 0.0, decelDistance = 0.0, cruiseDistance = 0.0;
    float cruiseTime = 0.0f, warpTime = 0.0f;

    //  set distances
    if (cruise) {                                   //  short         long
        accelDistance = exp(m_warpDecelTime);       //  2980.957    1.3188e9
        decelDistance = exp(3 * m_warpAccelTime);   //  8103.083    5.3204e11
        cruiseDistance = (m_targetDistance - accelDistance - decelDistance);
        cruiseTime = (cruiseDistance / warpSpeedInMeters);
    } else {
        //  short warp....no cruise
        accelDistance = (m_targetDistance /3);
        decelDistance = (m_targetDistance - accelDistance);
        warpSpeedInMeters = accelDistance;
    }

    //  set total warp time based on above math.
    warpTime =  (m_warpAccelTime + m_warpDecelTime + floor(cruiseTime));

    GVector warp_vector(m_position, m_targetPoint);
    warp_vector.normalize();

    _log(PHYSICS__TRACE, "DestinyManager::_InitWarp():Calculate - Entity %s(%u): Warp will accelerate for %us, cruise for %.3f, then decelerate for %us, with total time of %.3fs, and warp speed of %.4f m/s.",
            m_self->GetName(), m_self->GetID(), m_warpAccelTime, cruiseTime, m_warpDecelTime, warpTime, warpSpeedInMeters);

    _log(PHYSICS__TRACE, "DestinyManager::_InitWarp():Calculate - Entity %s(%u): Accel distance is %.4f. Cruise distance is %.4f.  Decel distance is %.4f.  Direction is %.3f,%.3f,%.3f.",
            m_self->GetName(), m_self->GetID(), accelDistance, cruiseDistance, decelDistance, warp_vector.x, warp_vector.y, warp_vector.z);

    _log(PHYSICS__TRACE, "DestinyManager::_InitWarp():Calculate - Entity %s(%u): We will exit warp at %.2f,%.2f,%.2f at a distance of %.4f AU (%.4fm).",
            m_self->GetName(), m_self->GetID(), m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_targetDistance/ONE_AU_IN_METERS, m_targetDistance);

    GPoint destination = m_position + (warp_vector * m_targetDistance);
    _log(PHYSICS__TRACE, "DestinyManager::_InitWarp():Calculate - Entity %s(%u): calculated exit is %.2f,%.2f,%.2f and vector is %.4f,%.4f,%.4f.",
            m_self->GetName(), m_self->GetID(), destination.x, destination.y, destination.z, warp_vector.x, warp_vector.y, warp_vector.z);

    //  reset deceltime for time check in _WarpDecel()
    m_warpDecelTime = m_warpAccelTime + floor(cruiseTime);

    m_stateStamp = GetStamp();

    delete m_warpState;
    m_warpState = new WarpState(
                        GetStamp(),
                        m_targetDistance,
                        warpSpeedInMeters,
                        accelDistance,
                        cruiseDistance,
                        decelDistance,
                        warpTime,
                        true,
                        false,
                        false,
                        warp_vector );

    //drain cap
    if (m_self->IsClient())
        m_self->CastToClient()->GetShip()->SetAttribute(AttrCharge, m_capCharge);

    //clear targets
    m_self->CastToClient()->targets.ClearAllTargets();
}

void DestinyManager::_WarpAccel(uint16 sec_into_warp) {
    /* For acceleration, k = 3.
     * distance = e^(k*s)
     * speed = k*e^(k*s)
     */
    double currentDistance = exp(3 * sec_into_warp);
    double currentShipSpeed = (3 * exp(3 * sec_into_warp));

    if (currentDistance > m_warpState->accelDist) {
        currentDistance = m_warpState->accelDist;
        m_warpState->accel = false;
        if (m_warpState->cruiseDist > 0)
            m_warpState->cruise = true;
        else
            m_warpState->decel = true;
    }
    m_targetDistance -= currentDistance;

    _WarpUpdate(currentShipSpeed);

    _log(PHYSICS__TRACE, "DestinyManager::_WarpAccel(): Entity %s(%u) - Warp Accelerating(%us): velocity %.4f m/s with %.4f m left to go.  Current distance %.4f.",
            m_self->GetName(), m_self->GetID(), sec_into_warp, currentShipSpeed, m_targetDistance, currentDistance);
}

void DestinyManager::_WarpCruise(uint16 sec_into_warp) {
    /* in cruise....calculate distance to update postion */
    m_targetDistance -= m_warpState->warpSpeed;

    if ((m_targetDistance - m_warpState->warpSpeed) < m_warpState->decelDist) {
        m_warpState->cruise = false;
        m_warpState->decel = true;
    }

    _WarpUpdate(m_warpState->warpSpeed);

    _log(PHYSICS__TRACE, "DestinyManager::_WarpCruise(): Entity %s(%u) - Warp Crusing(%us): velocity %.4f m/s. with %.4f m left to go.",
             m_self->GetName(), m_self->GetID(), sec_into_warp, m_warpState->warpSpeed, m_targetDistance);
}

void DestinyManager::_WarpDecel(uint16 sec_into_warp) {
    /* For deceleration, k = -1.
     * distance = e^(k*s)
     * speed = -k*e^(k*s)
     */
    uint8 decelTime = (sec_into_warp - m_warpDecelTime);
    double currentDistance = (m_warpState->total_distance - (exp(-decelTime) * m_warpState->decelDist));
    m_targetDistance = (m_warpState->total_distance - currentDistance);
    double currentShipSpeed = (m_warpState->warpSpeed * exp(-decelTime));

    if (currentShipSpeed <= m_speedToLeaveWarp) {
        // stop warp, and return to normal(stop) mode
        m_speedToLeaveWarp = currentShipSpeed;
        m_stop = true;
        _WarpUpdate(currentShipSpeed);
        _WarpStop(sec_into_warp, currentShipSpeed);
        return;
    }
    _WarpUpdate(currentShipSpeed);

    _log(PHYSICS__TRACE, "DestinyManager::_WarpDecel(): Entity %s(%u) - Warp Decelerating(%us/%us): velocity %.4f m/s with %.4f m left to go.",
             m_self->GetName(), m_self->GetID(), decelTime, sec_into_warp, currentShipSpeed, m_targetDistance);
}

void DestinyManager::_WarpUpdate(double currentShipSpeed) {
    //  update position and velocity for all stages.
    //  this method is ~1000m off actual.  could be due to rounding.   -allan 9Jan15
    m_position = (m_targetPoint - (m_warpState->warp_vector * m_targetDistance));
    m_velocity = (m_warpState->warp_vector * currentShipSpeed);

    if ((m_inBubble) && (m_warpState->accel)) {
        if (!m_self->Bubble()->InBubble(m_position)) {
            m_system->bubbles.Remove(m_self);
            SetBubble(false);
        }
    } else if (!m_inBubble && m_warpState->decel) {
        if (m_targetDistance < (BUBBLE_DIAMETER_METERS /2)) {
            _log(PHYSICS__TRACE, "DestinyManager::_WarpUpdate() - Entity %s(%u): Ship at %.2f,%.2f,%.2f is calling Add() .",
                m_self->GetName(), m_self->GetID(), m_position.x, m_position.y, m_position.z);
            m_system->bubbles.Add(m_self, true, true);
            SetBubble(true);
        }
    } else
        ;
}

void DestinyManager::_WarpStop(uint16 sec_into_warp, double currentShipSpeed) {
    _log(PHYSICS__TRACE, "DestinyManager::_WarpStop(): Entity %s(%u) - Warp complete. Exit velocity %.4f m/s with %.4f m left to go.",
             m_self->GetName(), m_self->GetID(), currentShipSpeed, m_targetDistance);

    // this should be a self-only update (do NOT bubblecast it).  server is tracking movement correctly, so entities in bubble
    //  have the correct position of entering client, but client <-> server math is a lil bit off....
    // so client will neeed to be updated to where server says the ship position is.  (no desync on existing clients)
    SetPosition(m_position, true, true);

    // this function checks for State = Warp and warpstate != null to set decel variables correctly with warp decel.
    //   have to call this BEFORE deleting or resetting State or WarpState.
    SetSpeedFraction(0.0f);

    m_speedToLeaveWarp = m_maxShipVelocity *0.75f;

    delete m_warpState;
    m_warpState = NULL;
}

//FIXME  this is not right.
void DestinyManager::_Orbit() {
    #define PVN(v) _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - Entity %u: " #v ": (%.15e, %.15e, %.15e)   len=%.15e", \
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
        Stop();
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
                Stop();
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
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - desired_distance = %.15e", desired_distance);
    if(desired_distance != 0) {
        something = (m_maxVelocity * TIC_DURATION_IN_SECONDS * .01) / desired_distance;
    }
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - something = %.15e", something);

    //this seems to be correct, without rounding error.
    //  use m_moveTimer here.....
    double v488 = double(GetStamp()-m_stateStamp) * something;
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - v488 = %.15e", v488);

    //this is not quite right... some sort of rounding I think.
    double coef = (v488 * 0.7) + 130001409/*entityID*/;
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - coef = %.15e", coef);

    //all of these are wrong due to rounding
    double cos_coef = cos( coef );
    double sin_coef = sin( coef );
    double cos_v488 = cos( v488 );
    double sin_v488 = sin( v488 );
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - coef(sin=%.15e, cos=%.15e) v488(sin=%.15e, cos=%.15e)", sin_coef, cos_coef, sin_v488, cos_v488);
    double v438 = cos_coef * sin_v488;
    double v3C0 = cos_v488 * cos_coef;

    GPoint pt( v3C0, sin_coef, v438 );    //this is a unit vector naturally
    PVN(pt);

    GVector tan_vector = pt.crossProduct(delta);
    tan_vector.normalize();
    PVN(tan_vector);

    double delta_d2 = current_distance*current_distance - desired_distance*desired_distance;
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - delta_d2 = %.15e", delta_d2);
    if(delta_d2 >= 0) {
        double mag = sqrt(delta_d2) * desired_distance / current_distance;
        _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - mag = %.15e", mag);
        GVector s = tan_vector * mag;
        PVN(s);
        GVector d = delta * (delta_d2/current_distance);
        PVN(d);
        delta = s + d;
        delta.normalize();
        PVN(delta);
    }

    double d = desired_distance - current_distance;
    d = exp(d*d / (-40000.0));
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - d = %.15e", d);

    GVector negative_delta = delta * -1;
    PVN(negative_delta);

    double tdn = tan_vector.dotProduct(negative_delta);
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - tdn = %.15e", tdn);
    double jjj = ((tdn*tdn - 1.0) * d * d) + 1;
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - jjj = %.15e", jjj);
    double iii;
    if(jjj < 0) {    //not sure on this condition at all.
        iii = d * tdn;    //not positive
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
    _log(PHYSICS__TRACEPOS, "DestinyManager::_Orbit() - m_accelerationFactor = %.15e", m_shipMaxAccelTime);
    accel_vector *=  m_shipMaxAccelTime;
    PVN(accel_vector);

    //copy delta_position into acceleration for input into movement.

    static const double ten_au = 1.495978707e12;
    GVector big_delta_position = accel_vector * ten_au;
    PVN(big_delta_position);

    m_targetPoint = orbit_point + big_delta_position;
    PVN(m_targetPoint);
    #undef PVN

    //_MoveAccel(accel_vector);
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

//Local Movement:
void DestinyManager::Follow(SystemEntity *who, double distance, bool update) {
    //called from client as 'CmdFollowBall'
    //  also used by 'Approach'

    if (State == DSTBALL_FOLLOW && m_targetEntity.second == who && m_targetDistance == distance)
        return;

    State = DSTBALL_FOLLOW;
    m_targetPoint = GPoint(who->GetPosition());
    m_targetEntity.first = who->GetID();
    m_targetEntity.second = who;
    m_targetDistance = distance;

    GVector moveVector(m_position, m_targetPoint);  //which way do we want to go?
    moveVector.normalize();     //change vector to direction
    m_moveDirection = moveVector;

    m_stop = false;
    SetSpeedFraction(1.0f);

    if (update) {
        DoDestiny_CmdFollowBall du;
        du.entityID = m_self->GetID();
        du.ballID = who->GetID();
        du.range = int32(distance);

        PyTuple *up = du.Encode();
        SendSingleDestinyUpdate(&up);    //consumed
        PySafeDecRef(up);
    }
}

void DestinyManager::TractorBeamFollow(SystemEntity *who, double mass, double maxVelocity, double distance, bool update) {
    if (State == DSTBALL_FOLLOW && m_targetEntity.second == who && m_targetDistance == distance)
        return;

    State = DSTBALL_FOLLOW;
    m_targetEntity.first = who->GetID();
    m_targetEntity.second = who;
    m_targetDistance = distance;

    // FIXME  need to set a target point here
    m_targetPoint = GPoint(0,0,0);

    GVector moveVector(m_position, m_targetPoint);  //which way do we want to go?
    moveVector.normalize();     //change vector to direction
    m_moveDirection = moveVector;

    m_stop = false;
    SetSpeedFraction(1.0f);

    //sLog.Debug("DestinyManager::TractorBeamFollow()", "SystemEntity '%s' following SystemEntity '%s' at velocity %f", \
    m_self->GetName(), who->GetName(), m_maxVelocity);

    //ensure that our bubble is correct.
    //m_system->bubbles.CheckBubble(m_self);

    if (update) {
        std::vector<PyTuple *> updates;

        DoDestiny_SetMaxSpeed maxspeed1;
        maxspeed1.entityID = m_self->GetID();
        maxspeed1.speedValue = maxVelocity;
        updates.push_back(maxspeed1.Encode());

        DoDestiny_SetBallFree ballfree;
        ballfree.entityID = m_self->GetID();
        ballfree.is_free = 1;
        updates.push_back(ballfree.Encode());

        DoDestiny_SetBallMass ballmass;
        ballmass.entityID = m_self->GetID();
        ballmass.mass = mass;
        updates.push_back(ballmass.Encode());

        DoDestiny_SetMaxSpeed maxspeed2;
        maxspeed2.entityID = m_self->GetID();
        maxspeed2.speedValue = maxVelocity;
        updates.push_back(maxspeed2.Encode());

        DoDestiny_CmdSetSpeedFraction speedfrac;
        speedfrac.entityID = m_self->GetID();
        speedfrac.fraction = 1.0;
        updates.push_back(speedfrac.Encode());

        DoDestiny_CmdFollowBall followball;
        followball.entityID = m_self->GetID();
        followball.ballID = who->GetID();
        followball.range = static_cast<int32>(m_targetDistance);
        updates.push_back(followball.Encode());

        SendDestinyUpdate(updates, false); //consumed
    }
}

void DestinyManager::TractorBeamHalt(bool update)
{
    m_targetEntity.first = 0;
    m_targetEntity.second = NULL;
    m_velocity = GVector( NULL_ORIGIN );
    //SetSpeedFraction(0.0f);
    Stop();

    State = DSTBALL_STOP;

    //ensure that our bubble is correct.
    //m_system->bubbles.CheckBubble(m_self);

    if (update) {
        std::vector<PyTuple *> updates;

        DoDestiny_SetMaxSpeed maxspeed1;
        maxspeed1.entityID = m_self->GetID();
        maxspeed1.speedValue = 0;
        updates.push_back(maxspeed1.Encode());

        DoDestiny_SetBallFree ballfree;
        ballfree.entityID = m_self->GetID();
        ballfree.is_free = 0;
        updates.push_back(ballfree.Encode());

        DoDestiny_SetBallMass ballmass;
        ballmass.entityID = m_self->GetID();
        ballmass.mass = 10000000000;
        updates.push_back(ballmass.Encode());

        DoDestiny_SetMaxSpeed maxspeed2;
        maxspeed2.entityID = m_self->GetID();
        maxspeed2.speedValue = 1000000;
        updates.push_back(maxspeed2.Encode());

        SendDestinyUpdate(updates, false); //consumed
    }
}

void DestinyManager::Orbit(SystemEntity *who, double distance, bool update) {
    /*
     *    def Orbit(self, id, range = None):
     *        if id == session.shipid:
     *            return
     *        if range is None:
     *            range = self.GetDefaultDist('Orbit')
     *        bp = sm.StartService('michelle').GetRemotePark()
     *        if bp is not None and range is not None:
     *            name = sm.GetService('space').GetWarpDestinationName(id)
     *            range = float(range) if range < 10.0 else int(range)
     *            eve.Message('CustomNotify', {'notify': localization.GetByLabel('UI/Inflight/Orbiting', name=name, range=range)})
     *            bp.CmdOrbit(id, range)
     */
    if (State == DSTBALL_ORBIT && m_targetEntity.second == who && m_targetDistance == distance)
        return;

    State = DSTBALL_ORBIT;
    m_stateStamp = GetStamp();

    m_targetEntity.first = who->GetID();
    m_targetEntity.second = who;
    m_targetPoint = who->GetPosition();
    m_targetDistance = distance;

    GVector moveVector(m_position, m_targetPoint);  //which way do we want to go?
    moveVector.normalize();     //change vector to direction
    m_moveDirection = moveVector;

    m_stop = false;
    SetSpeedFraction(1.0f);

    if (update) {
        DoDestiny_CmdOrbit du;
        du.entityID = m_self->GetID();
        du.orbitEntityID = who->GetID();
        du.distance = uint32(distance);

        PyTuple *up = du.Encode();
        SendSingleDestinyUpdate(&up);    //consumed
        PySafeDecRef(up);
    }
}

void DestinyManager::OrbitingCruise(SystemEntity *who, double distance, bool update, double cruiseSpeed) {
    if (State == DSTBALL_ORBIT && m_targetEntity.second == who && m_targetDistance == distance)
        return;

    if ( cruiseSpeed > 0 ) {
        m_maxShipVelocity = cruiseSpeed;
        _UpdateVelocity();
    }

    State = DSTBALL_ORBIT;
    m_stop = false;
    m_stateStamp = GetStamp();

    m_targetEntity.first = who->GetID();
    m_targetEntity.second = who;
    m_targetDistance = distance;
    SetSpeedFraction(1.0f);

    if(update) {
        DoDestiny_CmdOrbit du;
        du.entityID = m_self->GetID();
        du.orbitEntityID = who->GetID();
        du.distance = uint32(distance);

        PyTuple *up = du.Encode();
        SendSingleDestinyUpdate(&up);    //consumed
        PySafeDecRef(up);
    }
}

// settings for ship attributes
void DestinyManager::SetShipVariables(InventoryItemRef ship)
{
    double radius = ship->GetAttribute(AttrRadius).get_float();

    //check for rigs and modules that affect radius here
    m_radius = radius;

    /* AttrMass = 4,    (largest mass = Leviathan(3764) @ 2,430,000,000kg)
     * AttrMassLimit = 622,
     * AttrMassAddition = 796,
     * AttrMassMultiplier = 1471,
     */
    double shipMass = ship->GetAttribute(AttrMass).get_float();

    //  check for rigs and modules that modify mass here
    m_mass = shipMass;
    m_massMKg = m_mass / 1000000; //changes mass from Kg to MillionKg (10^-6)

    //  check for warp strength modifiers
    m_warpStrength = 1;

    /*  capacitor shit
     *    AttrCharge = 18,
     *     AttrRechargeRate = 55,       // time to charge from 0 to full, in milliSeconds
     *    AttrCapacitorBonus = 67,
     *    AttrCapacitorRechargeRateMultiplier = 144,
     *    AttrCapacitorCapacityMultiplier = 147,
     *    AttrCapRechargeBonus = 314,
     *     AttrRechargeratebonus = 338,
     *    AttrCapacitorCapacity = 482,
     *    AttrCapacitorCapacityBonus = 1079,
     *    AttrDisplayCapacitorCapacityBonus = 1094,
     *    AttrCapacitorCapacityMultiplierSystem = 1499,
     *   AttrRechargeRateMultiplier = 1500,
     */

    double capCapacity = ship->GetDefaultAttribute(AttrCapacitorCapacity).get_float();  // default value from db
    double capChargeRate = ship->GetDefaultAttribute(AttrRechargeRate).get_float(); // default value from db

    if ( m_self->IsClient() ) {
        Character *pChar = m_self->CastToClient()->GetChar().get();
        capCapacity *=  1 + ( 0.05 * (pChar->GetSkillLevel(skillEnergyManagement, true)));      // 5% increase
        capChargeRate *=  1 - ( 0.05 * (pChar->GetSkillLevel(skillEnergySystemsOperation, true)));      //5% decrease
    }

    // add checks for modules, rigs and implants that affect cap capacity here

    // add checks for modules, rigs and implants that affect cap charge rate here

    // set capacitor to full, and apply recharge rate.
    ship->SetAttribute(AttrCharge, capCapacity, true);
    ship->SetAttribute(AttrRechargeRate, capChargeRate, true);
    Halt();
}

//  called from Client::BoardShip(), Client::UpdateLocation(), Undock(), NPC::NPC()
void DestinyManager::SetShipCapabilities(InventoryItemRef ship, bool undock)
{
    /* this now sets variables needed for correct warp math.
     * noted modifiers to look into later, after everything is working
     * skill bonuses to agility, velocity, and cap are now implemented.
     */

    SetShipVariables(ship);

    double warpCapNeed = ship->GetDefaultAttribute(AttrWarpCapacitorNeed).get_float();

    double adjInertiaModifier = ship->GetDefaultAttribute(AttrAgility).get_float();
    float adjShipMaxVelocity = ship->GetDefaultAttribute(AttrMaxVelocity).get_float();
    float warpSpeedMultiplier = 1.0f;
    float shipBaseWarpSpeed = 3.0f;

    // skill bonuses to agility and velocity and warpCapacitorNeed
    /*
     *    Advanced Spaceship Command: 5% agility bonus s per level on ships requiring this skill
     *    Capital Ships: 5% agility bonus s per level on ships requiring this skill
     *    Spaceship Command: 2% agility for all ships per level
     *    Evasive Maneuvering: 5% agility bonus for all ships per level
     *    Skirmish Warfare: 2% agility to fleet per skill level
     *    Skirmish warfare Mindlink (implant): 15% agility to fleet, replaces Skirmish warfare skill
     *    Warp Drive Operation (skill) (only listed because it affects how far you can warp) :–)
     */
    if (m_self->IsClient()) {
        Character *p_Char = m_self->CastToClient()->GetChar().get();
        bool IsCapShip = false;
        // add check here for capital ships
        adjInertiaModifier -= ( adjInertiaModifier * p_Char->GetAgilitySkills(IsCapShip) );
        adjShipMaxVelocity += ( adjShipMaxVelocity * ( 5 * (p_Char->GetSkillLevel(skillNavigation, true)))/100);
        shipBaseWarpSpeed = ship->GetAttribute(AttrBaseWarpSpeed).get_float();
        warpSpeedMultiplier = ship->GetAttribute(AttrWarpSpeedMultiplier).get_float();
        warpCapNeed *=  1 + ( 0.1 * (p_Char->GetSkillLevel(skillWarpDriveOperation, true)));
        // TODO check for implants  AttrWarpCapacitorNeedBonus(319)
    } else {
        warpCapNeed = 0.0001f;
        adjInertiaModifier = 1.0f;
    }

    // TODO add module and rig bounses to inertia, agility, velocity here

    m_maxShipVelocity = adjShipMaxVelocity;
    m_shipInertiaModifier = adjInertiaModifier;
    ship->SetAttribute(AttrAgility, adjInertiaModifier);
    ship->SetAttribute(AttrMaxVelocity, adjShipMaxVelocity);

    /*  per https://forums.eveonline.com/default.aspx?g=posts&m=3912843   post#103
     *
     * Ships will exit warp mode when their warping speed drops below
     * 50% of sub-warp max speed, or 100m/s, whichever is the lower.
     */
    // half isnt right.  change back to 3/4 maxSubWarpSpeed
    m_speedToLeaveWarp = m_maxShipVelocity *0.75;

    // TODO add module and rig bonuses to warp cap here

    m_warpCapacitorNeed = warpCapNeed;
    ship->SetAttribute(AttrWarpCapacitorNeed, warpCapNeed);

    /* The product of Mass and the Inertia Modifier gives the ship's agility
     * Agility = Mass x Inertia Modifier
     *   NOTE agility is an internal-use variable, and is NOT sent to the client.
     */
    m_shipAgility = m_massMKg * m_shipInertiaModifier;

    /*   look into these, too...
     * AttrWarpSBonus(624) [rigs and implants]
     * AttrWarpFactor(21) [all are 0]
     * AttrWarpInhibitor(29) [default is null]
     */

    //TODO  add module and rig modifiers to warp speed here
    m_shipWarpSpeed = ( warpSpeedMultiplier * shipBaseWarpSpeed );
}

void DestinyManager::SetPosition(const GPoint &pt, bool update, bool selfOnly) {
    m_position = pt;
    if (selfOnly) {
        DoDestiny_SetBallPosition du;
        du.entityID = m_self->GetID();
        du.x = m_position.x;
        du.y = m_position.y;
        du.z = m_position.z;

        PyTuple *up = du.Encode();
        SendSingleDestinyUpdate(&up, true);    //consumed
        PySafeDecRef(up);
    } else {
        if (update) {
            DoDestiny_SetBallPosition du;
            du.entityID = m_self->GetID();
            du.x = m_position.x;
            du.y = m_position.y;
            du.z = m_position.z;

            PyTuple *up = du.Encode();
            SendSingleDestinyUpdate(&up);    //consumed
            PySafeDecRef(up);
        }
    }
}

void DestinyManager::AlignTo(SystemEntity *ent) {
    GPoint position = ent->GetPosition();
    State = DSTBALL_GOTO;
    m_targetPoint = position;

    DoDestiny_GotoPoint gtpoint;
    gtpoint.entityID = m_self->GetID();
    gtpoint.x = position.x;
    gtpoint.y = position.y;
    gtpoint.z = position.z;
    PyTuple *up = gtpoint.Encode();
    SendSingleDestinyUpdate(&up);    //consumed

    m_stop = false;
    SetSpeedFraction(1.0f);
}

void DestinyManager::GotoDirection(const GPoint &direction, float speed) {
    State = DSTBALL_GOTO;
    m_targetPoint = (direction * 1.0e16);

    DoDestiny_CmdGotoDirection du;
    du.entityID = m_self->GetID();
    du.x = direction.x;
    du.y = direction.y;
    du.z = direction.z;

    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);    //consumed

    m_stop = false;
    SetSpeedFraction(speed);
}

void DestinyManager::GotoPoint(const GPoint &point) {
    State = DSTBALL_GOTO;
    m_targetPoint = point;

    DoDestiny_GotoPoint gtp;
    gtp.entityID = m_self->GetID();
    gtp.x = point.x;
    gtp.y = point.y;
    gtp.z = point.z;

    PyTuple *up = gtp.Encode();
    SendSingleDestinyUpdate(&up);    //consumed

    m_stop = false;
    SetSpeedFraction(1.0f);
}
/*
void DestinyManager::Undock(GPoint dockPosition, GPoint direction) {
    //set position at undock point of station
    SetPosition(dockPosition);
    //ship should never be NULL.
    SetShipCapabilities(m_self->CastToClient()->GetShip(), true);
    //set direction and ship movement
    m_targetPoint = (direction *1.0e16);
    m_moveDirection = direction;
    SetSpeedFraction(1.5, true, true);
    //broadcast this client's data
    SendBallInfoOnUndock(direction);
}
*/
void DestinyManager::SendBallInfoOnUndock() const {
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

    SendDestinyUpdate(updates, true);    //consumed
}

PyResult DestinyManager::AttemptDockOperation() {
    Client *who = m_self->CastToClient();
    SystemManager *sm = m_self->System();
    uint32 stationID = who->GetDockStationID();
    SystemEntity *station = sm->get(stationID);

    if (station == NULL) {
        codelog(CLIENT__ERROR, "%s: Station %u not found.", who->GetName(), stationID);
        who->SendErrorMsg("Station Not Found, Docking Aborted.");
        return NULL;
    }

    //get the TRUE station Docking Perimiter   FIXME  this still needs work.
    GPoint stationDockPoint = station->GetPosition();
    GVector directionShipToStation(m_position, stationDockPoint);
    directionShipToStation.normalize();
    float radius = station->GetRadius();
    GPoint stationRadius = (directionShipToStation * radius);
    stationDockPoint -= stationRadius;
    GVector stationPerimiter(m_position, stationDockPoint);
    double rangeToStationPerimiter = stationPerimiter.length();

    // Verify range to station is within docking perimeter of 1500 meters:
    sLog.Warning("DestinyManager::AttemptDockOperation()", "rangeToStationPerimiter is %.2f", rangeToStationPerimiter);
    if (rangeToStationPerimiter > 2000.0) {
        who->SendErrorMsg("Outside Docking Perimiter.  Please Move Closer.");
        // This packet has to be returned to the client when outside the docking perimeter
        //   TODO:  make a packet response for this, named:  OnDockingApproach
        // Packet::ErrorResponse
        /* -- In the marshalstream:
         *    ObjectEx:
         *    Header:
         *      Tuple: 3 elements
         *        [ 0] Token: 'ccp_exceptions.UserError'
         *        [ 1] Tuple: 1 elements
         *        [ 1]   [ 0] String: 'DockingApproach'
         *        [ 2] Dictionary: 2 entries
         *        [ 2]   [ 0] Key: String: 'msg'
         *        [ 2]   [ 0] Value: String: 'DockingApproach'
         *        [ 2]   [ 1] Key: String: 'dict'
         *        [ 2]   [ 1] Value: (None)
         */
        // TODO: On docking failure: Send DoDestinyUpdates for GotoPoint for Station Dockpoint, SetBallMassive, SetBallMass
        /*
         *        [PySubStream 74 bytes]
         *          [PyObjectEx Normal]
         *            [PyTuple 3 items]
         *              [PyToken ccp_exceptions.UserError]
         *              [PyTuple 1 items]
         *                [PyString "DockingApproach"]
         *              [PyDict 2 kvp]
         *                [PyString "msg"]
         *                [PyString "DockingApproach"]
         *                [PyString "dict"]
         *                [PyNone]
         */
        //AlignTo( station );   // Turn ship and move toward docking point - client will call Dock() automatically when close enough.
        //return NULL;
    }
    // Docking was accepted, so send the OnDockingAccepted packet:
    std::vector<PyTuple *> updates;
    DoDestiny_OnDockingAccepted oda;
    oda.ship_x = m_position.x;
    oda.ship_y = m_position.y;
    oda.ship_z = m_position.z;
    oda.station_x = stationDockPoint.x;
    oda.station_y = stationDockPoint.y;
    oda.station_z = stationDockPoint.z;
    oda.stationID = stationID;
    updates.push_back(oda.Encode());

    // per client packet sniff
    DoDestiny_SetBallMassive bm;
    bm.entityID = m_self->GetID();
    bm.is_massive = false;
    updates.push_back(bm.Encode());

    SendDestinyUpdate(updates, true);

    //clear all targets
    who->targets.ClearAllTargets();

    //clear AutoPilot
    who->SetAutoPilot(false);

    // per client packet sniff
    Stop();

    // When docking, Set X,Y,Z to system origin so that when changing ships in stations, they don't appear outside
    who->MoveToLocation(stationID, NULL_ORIGIN);

    //tell ship it's docking.  this offlines modules and sets shield and cap to full.
    who->GetShip()->Dock();

    //Check if player is in pod, in which case they get a rookie ship for free  (with 2 civilian modules and 1 trit)
    //  on live, SCC sends mail about the loss of the players ship, and offers a new, fully-fitted ship as replacement.  we dont.
    if (who->GetShip()->typeID() == itemTypeCapsule)
        who->SpawnNewRookieShip();

    return NULL;
}

void DestinyManager::Cloak()
{
	sLog.Warning("DestinyManager::Cloak()", "TODO - check for warp-safe-ness, and turn on any cloaking device module fitted");
	m_cloaked = true;
	SendCloakShip(true);
	m_self->Bubble()->RemoveExclusive(m_self,true);
}

void DestinyManager::UnCloak()
{
	sLog.Warning("DestinyManager::UnCloak()", "TODO - check for warp-safe-ness, and turn off any cloaking device module fitted");
	m_cloaked = false;
	SendUncloakShip();
	m_self->Bubble()->AddExclusive(m_self,true);
}

void DestinyManager::WarpTo(const GPoint where, int32 distance) {
    /* warp order..
     * pick destination -> align -> aura "warp drive active" -> cap drain -> accel
     *      -> enter warp -> warp -> decel -> leave warp -> coast -> stop
     */

    if (m_warpState != NULL) {
        delete m_warpState;
        m_warpState = NULL;
    }

    // check for scramble
    //if(m_warpStrength < m_warpScrambleSrength) { ** note to client about being scrambled ** }

    GVector warp_distance(m_position, where);
    m_targetDistance = warp_distance.length();

    /*supercap warp modifiers
     * these will go here, and modify distance, target, and range accordingly
     *
     * AttrWarpAccuracyMaxRange = 1021,
     * AttrWarpAccuracyFactor = 1022,
     * AttrWarpAccuracyFactorMultiplier = 1023,
     * AttrWarpAccuracyMaxRangeMultiplier = 1024,
     * AttrWarpAccuracyFactorPercentage = 1025,
     * AttrWarpAccuracyMaxRangePercentage = 1026,
     */

    if (m_targetDistance < minWarpDistance) {
        // warp distance too close.  cancel warp and return
        if(m_self->IsClient())
            m_self->CastToClient()->SendErrorMsg("That is too close for your Warp Drive.");

        State = DSTBALL_STOP;
        return;
    }

    m_targetEntity.first = 0;
    m_targetEntity.second = NULL;
    m_targetPoint = where;
    m_stopDistance = distance;

    if (m_self->IsClient()) {   //FIXME  warp cap need is WRONG for short warps........MUST FIX
        Client *pClient = m_self->CastToClient();
        InventoryItemRef pShip = pClient->GetShip();

        // adjust warp distance based on cap left.   NPC's are not restricted by this. (or wont be in EvEmu)
        double currentShipCap = pShip->GetAttribute(AttrCharge).get_float();

        /*  capacitor for warp forumlas from https://oldforums.eveonline.com/?a=topic&threadID=332116
         *  Energy to warp = warpCapacitorNeed * mass * au * (1 - warp_drive_operation_skill_level * 0.10)
         */
        double adjWarpCapNeed = m_warpCapacitorNeed * (1 - (pClient->GetChar()->GetSkillLevel(skillWarpDriveOperation, true) * 0.1));
        double adjDistance = m_targetDistance / ONE_AU_IN_METERS; // change distance in meters to AU.
        double capNeeded = m_mass * adjWarpCapNeed * adjDistance;

        //  check if ship has enough capacitor to warp full distance
        if (capNeeded > currentShipCap) {
            /*        //  nope...enough cap to min warp?
             *        double cap_check = ((150000 /ONE_AU_IN_METERS) * m_mass * adjWarpCapNeed);
             *        if (cap_check > 0) {
             *          //reset distance based on avalible capacitor
             *            // FIXME this is wrong...
             *              m_stopDistance = currentShipCap / (m_mass * adjWarpCapNeed);
             *              m_targetPoint = NULL_ORIGIN;
             *            capNeeded = currentShipCap;
        } else {*/
            pClient->SendErrorMsg("You don't have enough capacitor charge to warp.");
            sLog.Warning("DestinyManager::WarpTo()", "Client %s(%u): Capacitor needed vs current  %.3f / %.3f",
                         m_self->GetName(), m_self->GetID(), capNeeded, currentShipCap);

            State = DSTBALL_STOP;
            delete m_warpState;
            return;
            //}
        } else {
            capNeeded = currentShipCap - capNeeded;
        }

        m_capCharge = capNeeded;
    }

    /*  TODO PUT CHECK HERE FOR WARP BUBBLES
     *     and other things that affect warp-in point.....when we get to there.
     * AttrWarpBubbleImmune = 1538,
     * AttrWarpBubbleImmuneModifier = 1539,
     *   NOTE:  warp bubble in path (or within 100km of m_targetPoint) will change m_targetDistance and m_targetPoint
     */

    sLog.Warning("DestinyManager::WarpTo()", "m_targetPoint: %.2f,%.2f,%.2f  m_stopDistance: %i  m_targetDistance: %.4f", \
    m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_stopDistance, m_targetDistance);

    if (m_moveDirection == NULL_ORIGIN) {
        GPoint position = m_position;
        position.normalize();
        GVector moveVector((position * 1.0e16), m_position); //check this per Vector(from, to)
        moveVector.normalize();
        m_moveDirection = moveVector;
    }

    const GPoint &point = where;
    GotoPoint(point);
    State = DSTBALL_WARP;

    // send client updates
    std::vector<PyTuple *> updates;

    // acknowledge client's warpto request
    DoDestiny_CmdWarpTo wt;
    wt.entityID = m_self->GetID();
    wt.dest_x = m_targetPoint.x;
    wt.dest_y = m_targetPoint.y;
    wt.dest_z = m_targetPoint.z;
    wt.distance = m_stopDistance;
    // client expects speed x 10.  dumb ccp shit again
    wt.warpSpeed = GetWarpSpeed();
    updates.push_back(wt.Encode());

    //send a warp effect...
    DoDestiny_OnSpecialFX10 sfx;
    sfx.effect_type = "effects.Warping";
    sfx.entityID = m_self->GetID();
    sfx.isOffensive = false;
    sfx.start = true;
    sfx.active = false;
    updates.push_back(sfx.Encode());
    /*
     *    //set massive for warp, per client
     *    DoDestiny_SetBallMassive bm;
     *    bm.entityID = m_self->GetID();
     *    bm.is_massive = true;
     *    PyTuple *up = bm.Encode();
     *    SendDestinyUpdate(up, true);
     *    PySafeDecRef(up);
     */

    SendDestinyUpdate(updates);
    updates.clear();
}

void DestinyManager::SendJumpOut(uint32 stargateID) const {
    //send a warping special effects update...
    DoDestiny_OnSpecialFX10 effect;
    effect.entityID = m_self->GetID();
    effect.targetID = stargateID;
    effect.effect_type = "effects.JumpOut";
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    PyTuple *up = effect.Encode();

    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendJumpIn() const {
    //hacked for simplicity... I dont like jumping in until we have
    //jumping in general much better quantified.

    DoDestiny_OnSpecialFX10 effect;
    effect.effect_type = "effects.JumpDriveIn";
    effect.entityID = m_self->GetID();
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    PyTuple *up = effect.Encode();

    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendJumpOutEffect(std::string JumpEffect, uint32 locationID) const {
    std::vector<PyTuple *> updates;

    DoDestiny_CmdStop du;
    du.entityID = m_self->GetID();
    updates.push_back(du.Encode());

    //send a warping special effects update...
    DoDestiny_OnSpecialFX10 effect;
    effect.entityID = m_self->GetID();
    effect.targetID = locationID;
	effect.effect_type = "effects.JumpDriveOut";
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    SendDestinyUpdate(updates, false);
}

void DestinyManager::SendJumpInEffect(std::string JumpEffect) const {
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

    DoDestiny_CmdSetSpeedFraction ssf;
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

void DestinyManager::SendTerminalExplosion() const {
    //send a warping special effects update...
    DoDestiny_TerminalExplosion du;
    du.entityID = m_self->GetID();
    du.unknownInt = 1206;    // this seems to be different every so often in the logs, no idea what it means
    du.unknownBool = false; // this always seems to be false, no idea what it means
    PyTuple *up = du.Encode();

    SendSingleDestinyUpdate(&up);
}

//this should only be available on gates.
//   not working correctly for gateActivate   -allan 6Aug14
void DestinyManager::SendGateActivity(uint32 stargateID) const {

    DoDestiny_OnSpecialFX10 du;
    du.entityID = stargateID;
    du.effect_type = "effects.GateActivity";
    du.isOffensive = 0;
    du.start = 1;
    du.active = 0;

    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);    //consumed
    PySafeDecRef(up);
}

void DestinyManager::SendSetState(const SystemBubble *b) const {
    DoDestiny_SetState ss;

    ss.stamp = GetStamp();
    ss.ego = m_self->GetID();
    m_system->MakeSetState(b, ss);

    PyTuple *tmp = ss.Encode();
    SendSingleDestinyUpdate(&tmp, true);    //consumed
}

void DestinyManager::SendBoardShip(const ShipRef boardShipRef) const {

    DoDestiny_SetBallInteractive du_setBallInteractiveCapsule;
    du_setBallInteractiveCapsule.entityID = boardShipRef->itemID();
    du_setBallInteractiveCapsule.interactive = 1;
    PyTuple *up = du_setBallInteractiveCapsule.Encode();

    SendSingleDestinyUpdate(&up);
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
    //DoDestiny_DamageState du_damageStateItemCapsule;
    /*
     *                                    [PyTuple 2 items]
     *                                      [PyDict 10 kvp]
     *                                        [PyString "itemID"]
     *                                        [PyIntegerVar 1002539931335]
     *                                        [PyString "typeID"]
     *                                        [PyInt 670]        <<< capsule
     *                                        [PyString "allianceID"]
     *                                        [PyInt 99000003]
     *                                        [PyString "corpID"]
     *                                        [PyInt 98006392]
     *                                        [PyString "modules"]
     *                                        [PyList 0 items]
     *                                        [PyString "warFactionID"]
     *                                        [PyNone]
     *                                        [PyString "securityStatus"]
     *                                        [PyFloat 4.58482992247122]
     *                                        [PyString "bounty"]
     *                                        [PyInt 0]
     *                                        [PyString "ownerID"]
     *                                        [PyInt 312857992]
     *                                        [PyString "charID"]
     *                                        [PyInt 312857992]
     *                                      [PyList 3 items]
     *                                        [PyTuple 3 items]
     *                                          [PyFloat 0.838750637528315]
     *                                          [PyFloat 100000]
     *                                          [PyIntegerVar 129511505719634791]
     *                                        [PyFloat 1]
     *                                        [PyFloat 1]
     */

    DoDestiny_SetBallInteractive du_setBallInteractiveOldShip;
    du_setBallInteractiveOldShip.entityID = oldShipRef->itemID();
    du_setBallInteractiveOldShip.interactive = 0;
    updates.push_back(du_setBallInteractiveOldShip.Encode());

    DoDestiny_SlimItem du_slimItemOldShip;
    //du_slimItemOldShip.

    SendDestinyUpdate(updates, false);
}

void DestinyManager::SendJettisonCargo(const InventoryItemRef itemRef) const {
/*    std::vector<PyTuple *> updates;

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

void DestinyManager::SendCloakShip(const bool IsWarpSafe) const {
    std::vector<PyTuple *> updates;

      DoDestiny_OnSpecialFX10 effect;
    effect.effect_type = "effects.Cloak";
	effect.entityID = m_self->GetID();
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    SendDestinyUpdate(updates, false);
}

void DestinyManager::SendUncloakShip() const {
    std::vector<PyTuple *> updates;

    DoDestiny_OnSpecialFX10 effect;
    effect.effect_type = "effects.Uncloak";
	effect.entityID = m_self->GetID();
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    SendDestinyUpdate(updates);
}

void DestinyManager::SendSpecialEffect10(uint32 entityID, const ShipRef shipRef, uint32 targetID, std::string effectString, bool isOffensive, bool start, bool isActive) const
{
    std::vector<PyTuple *> update;
	std::vector<int32, std::allocator<int32> > area;

    DoDestiny_OnSpecialFX10 effect;
    effect.entityID = entityID;
	effect.targetID = targetID;
    effect.effect_type = effectString;
    effect.area = area;
	effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    update.push_back(effect.Encode());

    SendDestinyUpdate(update);
}

void DestinyManager::SendSpecialEffect(const ShipRef shipRef, uint32 moduleID, uint32 moduleTypeID,
    uint32 targetID, uint32 chargeTypeID, std::string effectString, bool isOffensive, bool start, bool isActive, double duration, uint32 repeat) const
{
    std::vector<PyTuple *> update;
	std::vector<int32, std::allocator<int32> > area;

    DoDestiny_OnSpecialFX13 effect;
    effect.entityID = shipRef->itemID();
    effect.moduleID = moduleID;
    effect.moduleTypeID = moduleTypeID;
    effect.targetID = targetID;
    effect.otherTypeID = chargeTypeID;
    effect.area = area;
    effect.effect_type = effectString;
    effect.isOffensive = (isOffensive) ? 1 : 0;
    effect.start = start;
    effect.active = isActive;
    effect.duration_ms = duration;
    effect.repeat = new PyInt(repeat);
    effect.startTime = Win32TimeNow() + ((duration * Win32Time_Second) / 1000);

    update.push_back(effect.Encode());

    SendDestinyUpdate(update);
}
