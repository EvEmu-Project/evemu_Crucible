/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Rewrite:    Allan
*/

// this class is for objects that move

#include "EVEServerConfig.h"

#include "Client.h"
#include "EntityList.h"

#include "StaticDataMgr.h"
#include "log/logsys.h"
#include "map/MapData.h"
#include "math/Trig.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "packets/Missile.h"
#include "ship/Missile.h"
#include "ship/Ship.h"
#include "station/Station.h"
#include "station/StationDataMgr.h"
#include "system/BubbleManager.h"
#include "system/Container.h"
#include "system/DestinyManager.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include <cstdlib>


DestinyManager::DestinyManager(SystemEntity *self)
: mySE(self),
m_maxSpeed(1.0f),
m_shipAccelTime(0.0f),
m_shipMaxAccelTime(0.0f),
m_ballMode(Destiny::Ball::Mode::STOP),
m_warpTimer(0),
m_moveTime(0.0),
m_targetDistance(0),
m_followDistance(0),
m_stopDistance(0),
m_mass(1.0f),
m_turnTic(1),
m_massMKg(1.0f),
m_alignTime(1.0f),
m_timeToEnterWarp(10.0f),
m_shipWarpSpeed(1.0f),
m_maxShipSpeed(100.0f),
m_shipAgility(1.0),
m_shipInertia(1.0),
m_warpAccelTime(1),
m_warpDecelTime(1),
m_warpState(nullptr),
m_targBubble(nullptr),
m_warpCapacitorNeed(0.00001),
m_frozen(false),
m_ticAlign(false),
mvPacket(nullptr)
{
    m_bump = false;
    m_stop = false;
    m_accel = false;
    m_decel = false;
    m_cloaked = false;
    m_turning = false;
    m_orbiting = 0;
    m_tractored = false;
    m_changeDelay = false;
    m_tractorPause = false;
    m_hasSentShipUpdates = false;

    m_capNeeded = 0.0;
    m_prevSpeed = 0.0f;
    m_degPerTic = 0.0f;
    m_orbitTime = 0.0f;
    m_orbitRadTic = 0.0f;
    m_timeFraction = 0.0f;
    m_prevSpeedFraction = 0.0f;
    m_userSpeedFraction = 0.0f;
    m_activeSpeedFraction = 0.0f;
    m_maxOrbitSpeedFraction = 1.0f;

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;
    m_velocity = GVector( NULL_ORIGIN );
    m_targetPoint = GPoint( NULL_ORIGIN );
    m_shipHeading = GVector( NULL_ORIGIN );
    m_targetHeading = GVector( NULL_ORIGIN );

    m_radius = mySE->GetRadius();
    m_position = mySE->GetPosition();

    m_turnTic = 0;
    m_turning = false;
    m_turnFraction = 0;

    m_inclination = 0;
    m_longAscNode = 0;

    m_stateStamp = 0;
}

DestinyManager::~DestinyManager() {
    m_warpTimer.Disable();
    SafeDelete(m_warpState);
}

// this is called once per tic by SystemEntity::Process()
void DestinyManager::Process() {
    double profileStartTime(GetTimeUSeconds());

    if (mySE->IsFrozen()) {
        Halt();
        return;
    }

    //check for and process Destiny::Ball::State changes.
    if (m_ticAlign) {
        m_ticAlign = false;
        // send movement packet this tic and begin movement
        //SendMovementPacket();
    }

    ProcessState();

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::destiny, GetTimeUSeconds() - profileStartTime);
}

void DestinyManager::ProcessState() {
    using namespace Destiny;
    switch(m_ballMode) {
        case Ball::Mode::STOP: {
            if (IsMoving()) {
                MoveObject();
                return;
            }
            Stop();
        } break;
        case Ball::Mode::GOTO: {
            MoveObject();
        } break;
        case Ball::Mode::MISSILE: {
            // if target was removed, continue movement and wait for Missile::EndOfLife() call to do cleanup
            //set current direction based on position and targetPoint.  this will keep missile aligned properly
            GVector moveVector(m_position, m_targetPoint);
            moveVector.normalize();
            //set position and direction for this round of movement
            m_shipHeading = moveVector;
            m_velocity = (moveVector * m_maxSpeed);
            SetPosition(m_position + m_velocity);
        } break;
        case Ball::Mode::ORBIT: {
            if (IsTargetInvalid())
                return;
            Orbit();
        } break;
        case Ball::Mode::FOLLOW: {
            if (IsTargetInvalid())
                return;
            Follow();
        } break;
        case Ball::Mode::WARP: {
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
             *  Acceleration and Deceleration are logarithmic with finite caps (instead of infinity) at the ends.
             *      see also:  my notes in InitWarp()
             */
            if (m_warpState != nullptr) {
                //warp is in progress
                uint16 sec_into_warp = (sEntityList.GetStamp() - m_stateStamp);
                //  speed and distance formulas based on current warp distance
                if (m_warpState->accel) {
                    WarpAccel(sec_into_warp);
                } else if (m_warpState->cruise) {
                    WarpCruise(sec_into_warp);
                } else if (m_warpState->decel) {
                    WarpDecel(sec_into_warp);
                } else {// uh, Houston...we have a problem...
                    if (mySE->HasPilot()) {
                        _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  Ship %s(%u) for Player %s(%u) Has WarpState but checks are false.",  \
                                    mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());
                        mySE->GetPilot()->SendErrorMsg("Internal Server Error. Ref: ServerError 35928.   Please Dock or Relog to reset your ship.");
                    } else {
                        _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  NPC %s(%u) Has WarpState but checks are false.",  \
                                    mySE->GetName(), mySE->GetID());
                    }
                }
                return;
            }

            // Updated warp alignment and speed check.  -allan  17nov15
            GVector toVec(m_position, m_targetPoint);
            toVec.normalize();
            float dot = toVec.dotProduct(m_shipHeading);
            float degrees = EvE::Trig::Rad2Deg(std::acos(dot));

            if ((degrees < WARP_ALIGNMENT) and (m_timeFraction > 0.749)) {
                m_shipHeading = toVec;
                InitWarp();
                return;
            } else if (m_timeFraction < 0.749 && m_userSpeedFraction < 0.7499) {
                SetSpeedFraction(1.0f, true);
            } else if ((sEntityList.GetStamp() - m_stateStamp) > m_timeToEnterWarp + 0.3) {
                // catchall for turn checks messed up, and m_moveTime > ship align time
                if (mySE->HasPilot()) {
                    _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  Ship %s(%u) for Player %s(%u) - warp align/speed is incorrect, but time > shipTimeToWarp.",  \
                                mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());
                } else {
                    _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  NPC %s(%u) - warp align/speed is incorrect, but time > shipTimeToWarp.",  \
                            mySE->GetName(), mySE->GetID());
                }
                m_shipHeading = toVec;
                InitWarp();
                return;
            }

            MoveObject();
        } break;
        case Ball::Mode::MUSHROOM:      // aoe?
        case Ball::Mode::BOID:          // this will turn RIGID after a set time
        case Ball::Mode::TROLL:         // seen for wrecks
        case Ball::Mode::MINIBALL:      // used for sentrys
        case Ball::Mode::FIELD:         // dunno
        case Ball::Mode::FORMATION:     // dunno
        case Ball::Mode::RIGID:         // item that never moves
            //no default on purpose
            break;
    }
}
/* acceleration forumula
 * V(t) = Vmax*(1-e^-(t/a))
 * V(t) = velocity at time t
 * a = agility
 */
 //Velocity setting methods
void DestinyManager::SetSpeedFraction(float fraction/*1.0*/, bool startMovement/*false*/) {
    // this sets current speed fraction for object.

    // if orbiting, call Orbit() and let code reset the variables
    if (m_orbiting != 0)
        Orbit(m_targetEntity.second, m_targetDistance);

    if ((fraction == m_userSpeedFraction) and (!startMovement)) {
        // no change.
        return;
    }

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::SetSpeedFraction() - %s(%u):   prevSpeed:%.2f, fraction: %.2f, start: %s, stop: %s,accel: %s, decel: %s",
             mySE->GetName(), mySE->GetID(), m_prevSpeed, fraction, startMovement ? "true" : "false", m_stop ? "true" : "false", \
             m_accel ? "true" : "false", m_decel ? "true": "false");

    // this is to start movement when setting fractional speeds from speedo in client.
    //  also a hack to circumvent above check when called again by goto, warp, align, follow for changing direction.
    if (startMovement) {
        m_stop = false;
        if (m_ballMode == Destiny::Ball::Mode::STOP)
            m_ballMode = Destiny::Ball::Mode::GOTO;
    }

    // prevent multiple client calls to Stop() from resetting ship speed.
    if (m_stop)
        return;

    /* movement is set according to time, speed fraction, and objects' maximum configured speed.
     * all *Fraction variables use fuzzy logic
     *  -allan 8Oct14  -major update 20Nov15  -added prop mod code 29Mar17
     *  -base movement rewrite/update 18Oct21
     *
     *  speed is the actual distance an object travels over a given time
     *   -> time is measured in seconds
     *   -> distance measured in meters
     *  m_maxSpeed is ship maximum speed based on user input and current configuration.
     *   -> set in UpdateVelocity()
     *   -> only used for logging
     *  m_maxShipSpeed (MSS) is the maximum speed an object can travel in a given time
     *   -> measured in m/s
     *   -> initially set by UpdateShipVariables() based on ship configuration
     *   -> reset by SpeedBoost() for prop mods
     *  m_userSpeedFraction (USF) is user-set fraction of max ship speed (fractional from speedo or full from command).
     *   -> sets m_maxSpeed
     *   -> used with ASF and MSS to set speed
     *   -> range is 0.0 for stop and 1.0 for full
     *  m_prevSpeedFraction (PSF) is previous speed fraction
     *   -> reset on every speed change
     *   -> used to calculate estimated time to change speed
     *   -> used to set speed when speed changes via user input or prop mod
     *   -> can be > 1.0 based on other factors (deactivating prop mod at full speed will give PSF > 1.0)
     *  m_activeSpeedFraction (ASF) this is the percentage of ship max speed for the given tic.
     *   -> set in MoveObject()
     *   -> uses USF and MSS to set speed
     *  m_timeFraction (TF) holds current euler value for time.
     *   -> this is the current percent of change between previous speed and requested speed.
     *   -> range is 0.0 to 1.0
     *   -> set in MoveObject()
     *   -> sets ASF (directly)
     *   -> sets velocity (indirectly)
     *  m_maxOrbitSpeedFraction (OSF) is ship's max speed based on orbit data
     *   -> may not be used after update
     *  m_moveTime is timestamp when move change started
     *   -> data type is (double) FileTime
     *   -> set in BeginMovement()
     *   -> reset on every speed change
     *   -> used to calculate TF
     *  m_targetPoint holds current target coords.
     *   -> set by goto, warp, align, follow, orbit
     *   -> sets m_shipHeading
     *  m_shipHeading holds current direction and is set in _Turn()
     *   -> used with speed to set m_velocity
     *  m_velocity is current ship velocity.  set in MoveObject()
     *   -> m_velocity = m_shipHeading * (ASF * MSS)
     *   -> this is the variable used for position tracking.
     *  m_shipAccelTime is calculated time to complete requested speed change
     *   -> measured in seconds
     *   -> set on all speed changes
     *  m_shipMaxAccelTime is calculated time for ship to accelerate from stop to full
     *   -> measured in seconds
     *   -> initially set by UpdateShipVariables() based on ship configuration
     *   -> reset by SpeedBoost() for prop mods
     */

    m_prevSpeedFraction = 0.0f;

    if (m_activeSpeedFraction > 0.01f) {
        m_userSpeedFraction = fraction;
        m_prevSpeedFraction = m_activeSpeedFraction;
        UpdateVelocity(true);
    } else {
        m_userSpeedFraction = fraction;
        UpdateVelocity(false);
    }

    if (m_ballMode == Destiny::Ball::Mode::WARP) {
        // set state to Ball::Mode::GOTO after setting warp decel variables, so warp completion will decel properly
        m_ballMode = Destiny::Ball::Mode::GOTO;
        return;
    }

    std::vector<PyTuple*> updates;
    updates.clear();
    if (fraction) {
         CmdSetSpeedFraction du;
            du.entityID = mySE->GetID();
            du.fraction = fraction;
        updates.push_back(du.Encode());
    }
    if (((mySE->IsNPCSE() or mySE->IsDroneSE()) and !m_hasSentShipUpdates)
    or mySE->IsMissileSE() or mySE->IsContainerSE() or mySE->IsWreckSE()) {
        SetBallSpeed ms;   //NPCs and Missiles only.
            ms.entityID = mySE->GetID();
            ms.speed = m_maxSpeed;
        updates.push_back(ms.Encode());
        m_hasSentShipUpdates = true;
    }

    if (!updates.empty())
        SendDestinyUpdate(updates);
}

void DestinyManager::UpdateVelocity(bool isMoving) {
    uint8 logType(0);
    if ((m_ballMode == Destiny::Ball::Mode::WARP) and (m_warpState != nullptr)) {
        /*  Warp() finished, and ship dropped out of warp at m_speedToLeaveWarp,
         * set variables for decel from this speed.
         */
        logType = 1;
        m_accel = false;
        m_decel = true;
        m_targBubble = nullptr;
        m_maxSpeed = m_speedToLeaveWarp;
        m_prevSpeed = m_speedToLeaveWarp;
        m_velocity = m_shipHeading * m_maxSpeed;
        m_prevSpeedFraction = m_maxSpeed / m_maxShipSpeed;
        m_shipAccelTime = m_shipAgility * -log(1-(m_prevSpeedFraction));
    } else if (m_userSpeedFraction) {
        // commanded speed fraction > 0 and ...
        float delta(1.0f);
        if (isMoving) {
            //  ... ship is moving and ...
            if ((m_activeSpeedFraction == m_userSpeedFraction) and (!m_prevSpeed)) {
                // ... nothing has changed.
                return;
            }

            if (m_activeSpeedFraction > m_userSpeedFraction) {
                // ... request is lower than current speed - begin decel
                logType = 4;
                m_accel = false;
                m_decel = true;
                delta = m_activeSpeedFraction - m_userSpeedFraction;
            } else if (m_userSpeedFraction > m_activeSpeedFraction) {
                // ... request is higher than current speed - begin accel
                logType = 3;
                m_accel = true;
                m_decel = false;
                delta = m_userSpeedFraction - m_activeSpeedFraction;
            }

            m_shipAccelTime = m_shipMaxAccelTime * delta;
            m_prevSpeed = m_maxShipSpeed * m_activeSpeedFraction;
        } else {
            // ... ship is not moving. begin movement
            logType = 2;
            m_accel = true;
            m_decel = false;
            // this isnt spot on, but pretty damn close
            m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;
            m_shipAccelTime = m_shipMaxAccelTime * m_userSpeedFraction;      // for accel with user speeds <= 1.0
        }
        if (is_log_enabled(DESTINY__MOVE_TRACE))
            _log(DESTINY__MOVE_TRACE, "Destiny::UpdateVelocity - %s(%u): Speed Change - USF: %.2f, ASF: %.2f, TF: %.2f, PSF: %.2f, pSpeed: %.2f, mSpeed: %.2f, accel: %s, decel: %s, delta: %.3f", \
            mySE->GetName(), mySE->GetID(), m_userSpeedFraction, m_activeSpeedFraction, m_timeFraction, m_prevSpeedFraction, m_prevSpeed, m_maxSpeed, \
                 m_accel ? "true" : "false", m_decel ? "true": "false", delta);
    } else if (m_activeSpeedFraction) {
        //  commanded to stop while ship is moving.  begin decelerating
        logType = 5;
        m_accel = false;
        m_decel = true;
        m_maxSpeed = 0;
        m_prevSpeed = m_maxShipSpeed * m_activeSpeedFraction;
        m_shipAccelTime = m_shipMaxAccelTime * m_activeSpeedFraction;
    } else {
        // ... ship is not moving.  reset all move vars by calling Halt()
        logType = 6;
        Halt();
    }

    // reset moveTime to now
    // this is time call came from client, NOT on destiny tic.
    m_moveTime = GetTimeMSeconds();

    if (m_shipAccelTime < 0)
        sLog.Error("Destiny::UpdateVelocity()", "Accel Time is negative: %.2f", m_shipAccelTime);

    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        std::string msg = "";
        switch (logType) {
            case 1: { msg = "ship dropped out of warp.      --Begin Decel"; }       break;
            case 2: { msg = "USF != 0 and ship is Stopped.  --Begin Accel"; }       break;
            case 3: { msg = "USF != 0 and ship is Moving.   --Begin Accel"; }       break;
            case 4: { msg = "USF != 0 and ship is Moving.   --Begin Decel"; }       break;
            case 5: { msg = "USF == 0 and ship is Moving.   --Decel for Stop"; }    break;
            case 6: { msg = "USF == 0 and ship is Stopped.  --Halt"; }              break;
        }
        _log(DESTINY__MOVE_TRACE, "Destiny::UpdateVelocity - %s(%u):  %s  AccelTime: %.2f, USF: %.2f, ASF: %.2f, TF: %.2f, PSF: %.2f", \
                mySE->GetName(), mySE->GetID(), msg.c_str(), m_shipAccelTime, m_userSpeedFraction, \
                m_activeSpeedFraction, m_timeFraction, m_prevSpeedFraction);
    }
}

// `AbortIfLoginWarping` prevents the player from moving, warping, stopping,
// etc. if the player is logging in and attempting to warp back to their
// previous position.
//
// If `showMsg` is `true`, the player will be notified of their inability to
// perform the action they requested.
//
// TODO: This should be called more often, such as when the player is trying to
// do most Destiny actions - eject, orbit, follow, etc. These were implemented
// in BeyonceService, but some other edge cases may remain.
bool DestinyManager::AbortIfLoginWarping(bool showMsg) {
    Client* pClient = mySE->GetPilot();
    if (pClient != nullptr && pClient->IsLoginWarping()) {
        if (showMsg) {
            pClient->SendNotifyMsg("You cannot perform this action right now.");
        }

        return true;
    }

    return false;
}

void DestinyManager::Stop() {
    // Usually there's no need to show a message for this because it gets
    // triggered unnecessarily a few times upon login. Commands that should
    // show a notification are handled in BeyonceService.
    if (AbortIfLoginWarping(false)) {
        return;
    }

    // AP not implemented yet in this version  -allan 4Mar15
    // Clear autopilot
    if (mySE->HasPilot()) {
        mySE->GetPilot()->SetAutoPilot(false);
    }

    if (m_userSpeedFraction == 0.0f) {
        m_stop = true;
    } else if  ((m_ballMode == Destiny::Ball::Mode::WARP) and (!IsWarping()))  {
        //warp aborted before initialized.  standard Stop() applies.
        m_ballMode = Destiny::Ball::Mode::STOP;
    } else if (IsMoving()) {
        //stop called while moving
        m_ballMode = Destiny::Ball::Mode::STOP;
    }

    m_accel = false;
    m_decel = false;
    m_prevSpeed = 0.0f;
    m_prevSpeedFraction = 0.0f;

    ClearTurn();
    ClearOrbit();

    m_stateStamp = sEntityList.GetStamp();

    SetSpeedFraction(0.0f);
    m_stop = true;

    CmdStop du;
        du.entityID = mySE->GetID();
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void DestinyManager::Halt() {
    SafeDelete(m_warpState);

    //  reset ALL movement variables and states.  calling this will set object to a COMPLETE and IMMEDIATE stop.
    m_ballMode = Destiny::Ball::Mode::STOP;
    m_stop = true;
    m_accel = false;
    m_decel = false;
    m_turning = false;
    m_maxSpeed = 0.0f;
    m_velocity = GVector(NULL_ORIGIN);
    m_moveTime = 0.0;
    m_prevSpeed = 0.0f;
    m_stateStamp = 0;
    m_targetPoint = GPoint(NULL_ORIGIN);
    m_stopDistance = 0;
    m_targetDistance = 0;
    m_followDistance = 0;
    m_prevSpeedFraction = 0.0f;
    m_userSpeedFraction = 0.0f;
    m_activeSpeedFraction = 0.0f;
    m_timeFraction = 0.0f;
    m_maxOrbitSpeedFraction = 1.0f;

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;

    ClearTurn();

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::Halt() - %s(%u): m_shipHeading: %.3f,%.3f,%.3f", \
                mySE->GetName(), mySE->GetID(), m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
}

void DestinyManager::Eject()
{
    // basic updates for ejecting from ship
    Stop();
    UpdateOldShip(mySE->GetShipSE());
    SendJettisonPacket();
}

// Global collision methods
//  check for collision.  called by Move()
void DestinyManager::CheckBump()
{
    double profileStartTime(GetTimeUSeconds());

    //  collision detection code here
    /*  in this case, we are ONLY interested in objects
     *   that have drifted within each others radius (for whatever reason)
     *  this only checks for ships running sub-warp speeds
     *   in relation to other objects in bubble.
     */

    // NOTE:  object's "massive = true" means it can bump/collide  (massive = solid)

    // initial implementation will ONLY check player ships for bumping.
    std::vector<Client*> vPlayers;
    mySE->SysBubble()->GetPlayers(vPlayers);
    Client* pClient = mySE->GetPilot();
    GPoint pos(GetPosition());
    float distance = 0.0f;
    for (auto cur : vPlayers) {
        if (cur == pClient)
            continue;
        distance = pos.distance(cur->GetShipSE()->GetPosition());
        distance -= (mySE->GetRadius() - cur->GetShipSE()->GetRadius());
        if (distance < BUMP_DISTANCE) {
            Bump(cur->GetShipSE());
            m_bump = true;
        } else {
            m_bump = false;
        }
    }
    /** @todo  add data and checks for each ship bumped
     * to give single bump msg for each ship combo
     * without spamming their overview
     */

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::collision, GetTimeUSeconds() - profileStartTime);
}

void DestinyManager::Bump(SystemEntity* pSE)
{
    if (m_bump)
        return;
    // bump code here
    /*  determine most massive object...maybe not.  use percentiles here (becham math)
     *  determine direction(s) involved
     *  determine speed(s) involved
     *  determine new headings based on above
     *  determine new speed based on above
     *
     * NOTE static or large objects wont move, but will apply equal and opposite force.
     * un-anchored objects WILL move (jetcans, wrecks, portable hangers)
     */

    /* bump math, by Scheulagh Santorine, Ph.D.
     *  velocity of bumped object immediately after bump
     * v2(t=0+) = 2v1*m1/m1+m2
     */

    /*  run-time options for bumping jetcans, biomass, and other space objects
     *   bump drones??  prolly not, for simplicity
     */
    std::string msg1 = "You have bumped ";
    msg1 += pSE->GetPilot()->GetName();
    mySE->GetPilot()->SendNotifyMsg(msg1.c_str());
    // this test isnt needed right now, as it's ONLY checking against players and will always return true.
    //  will keep it in here for later expansion.
    if (pSE->HasPilot()) {
        std::string msg2 = "You have been bumped by ";
        msg2 += mySE->GetPilot()->GetName();
        pSE->GetPilot()->SendNotifyMsg(msg2.c_str());
    }
}

void DestinyManager::Bounce(GVector direction, float speed)
{
    // bounce code here (not used yet)
    /*  this code will update ship movement after being bumped
     *  all items will drift to a complete stop, unless other movement is called.
     */
    m_ballMode = Destiny::Ball::Mode::GOTO;
    m_stop = false;
    m_stateStamp = sEntityList.GetStamp();
    m_moveTime = GetTimeMSeconds();
    m_shipAccelTime = 0.1f;
    m_userSpeedFraction = 1.0f;
    m_timeFraction = 1.0f;
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = m_shipHeading * m_maxSpeed;

    std::vector<PyTuple*> updates;
    SetBallVelocity bv;
        bv.entityID = mySE->GetID();
        bv.x = m_velocity.x;
        bv.y = m_velocity.y;
        bv.z = m_velocity.z;
    updates.push_back(bv.Encode());
    CmdGotoDirection du;
        du.entityID = mySE->GetID();
        du.x = m_shipHeading.x;
        du.y = m_shipHeading.y;
        du.z = m_shipHeading.z;
    updates.push_back(du.Encode());
    SendDestinyUpdate(updates);
    Stop();
}

// main movement method
void DestinyManager::MoveObject() {
    if (mySE->SysBubble() == nullptr)
        mySE->SystemMgr()->AddEntity(mySE);

    if (m_stateStamp > sEntityList.GetStamp()) {
        if (is_log_enabled(DESTINY__MOVE_TRACE))
            _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u): stateStamp (%u) > GetStamp (%u).", \
            mySE->GetName(), mySE->GetID(), m_stateStamp, sEntityList.GetStamp());
        return;
    }

    if (m_changeDelay) {
        m_changeDelay = false;
        m_moveTime = GetTimeMSeconds(); // reset m_moveTime to now
        _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - ChangeDelay - %s(%u): stateStamp: %u", \
                    mySE->GetName(), mySE->GetID(), m_stateStamp);
        return;
    }

    /* acceleration and deceleration are both logarithmic, and the server needs to keep up with client position.
     * formula for time taken to accelerate from v to V, from https://wiki.eveonline.com/en/wiki/Acceleration
     *
     *   t=IM(10^-6) * -ln(1-(v/V))
     *   m_shipAccelTime = m_shipAgility * -ln(1-(v/V))
     *
     * as this uses the natural log, the higher the speed, the slower the acceleration, to the limits of ln(0)
     * since lim ln(x) = -INFINITY where x->0+. and ln(0) is undefined, we will use
     *
     *   m_shipMaxAccelTime = (-log(0.0001) * m_shipAgility);
     *
     * to define the time it will take a given ship to reach 99.9999% of m_maxShipSpeed, at which point,
     * the server will set m_velocity = (m_maxShipSpeed * direction) (or 100% ship speed).
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

    /* **UPDATE**  this now uses time AND (m_timeFraction > 0.9999f) for min/max speeds.  -allan 6Aug14
     * **UPDATE**  this is now tracking ALL speed changes correctly.  -allan 21Nov15
     * **UPDATE**  initial orbit implementation.  -allan 13July16
     * **UPDATE**  removed speed fraction checks for min/max speeds.  -allan 02Jul17
     * **UPDATE**  reworked entire basic movement checks and formulas -allan 17Oct21
     */

    float timeStamp(0);
    // check for moving ship changing heading
    if (m_userSpeedFraction) {
        if (!m_orbiting or (m_orbiting > Destiny::Ball::Orbit::Far))
            Turn();
    }

    float speed(0.0f);
    std::string move = "";

    // keep timer in seconds.
    timeStamp = (GetTimeMSeconds() - m_moveTime) * 0.001f;

    if ((timeStamp > m_shipAccelTime) and (m_timeFraction > 0.9998f)) {
        m_activeSpeedFraction = m_userSpeedFraction;
        speed = m_maxShipSpeed * m_activeSpeedFraction;

        if (m_decel) {
            if (is_log_enabled(DESTINY__MOVE_TRACE))
                _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) has decel'd from %.2fm/s to %.2fm/s in %.3fs.", \
                    mySE->GetName(), mySE->GetID(), m_prevSpeed, speed, timeStamp);
        } else if (m_accel) {
            if (is_log_enabled(DESTINY__MOVE_TRACE))
                _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) has accel'd from %.2fm/s to %.2fm/s in %.3fs.", \
                mySE->GetName(), mySE->GetID(), m_prevSpeed, speed, timeStamp);
        }

        m_accel = false;
        m_decel = false;
        m_prevSpeed = 0.0f;
        m_prevSpeedFraction = 0.0f;

        if (m_userSpeedFraction) {
            // ship has reached full commanded speed
            move = "at constant speed, going";
        } else {
            //ship has reached full stop
            if (is_log_enabled(DESTINY__MOVE_TRACE))
                _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) is at full stop after %.3f seconds.", \
                    mySE->GetName(), mySE->GetID(), timeStamp);
            Halt();
            return;
        }
    } else {
        // changed speed and asf != usf
        m_timeFraction = (1 - exp(-timeStamp / m_shipAgility));

        if (m_accel) {
            // object still accelerating.
            move = "accelerating";
            if (m_turning)
                move = "accelerating in turn";

            if (m_prevSpeedFraction) {
                /* accel from previous non-full speed
                 *   take diff of psf and usf then multiply by tf
                 *   add result to psf to get asf
                 *  asf is the fraction of max speed the ship is moving at this tic.
                 */
                m_activeSpeedFraction = m_prevSpeedFraction + (m_userSpeedFraction - m_prevSpeedFraction) * m_timeFraction;
            } else {
                // this is simple acceleration.  asf = usf * tf
                m_activeSpeedFraction = m_userSpeedFraction * m_timeFraction;
            }
        } else if (m_decel) {
            // object still decelerating.
            move = "decelerating";
            if (m_turning) {
                // decel for turn
                move = "decelerating for turn";
            }
            if (m_prevSpeedFraction) {
                // asf = psf - (psf - usf) * tf
                m_activeSpeedFraction = m_prevSpeedFraction - (m_prevSpeedFraction - m_userSpeedFraction) * m_timeFraction;
            } else {
                // this should never hit....should not have decel w/o previous speed
                sLog.Warning("Destiny::MoveObject()", "decel = true, but psf = 0.");
            }
        } else if (m_tractored or m_tractorPause) {
            ;   // do nothing here.  this is to remove error reporting from next line.
        } else {
            sLog.Error("Destiny::MoveObject()", "%s(%u) - move checks are not set right. Acc:%s, Dec:%s, Turn:%s, Tic:%u, Tractored:%s, TractorPause:%s", \
                    mySE->GetName(), mySE->GetID(), (m_accel ? "True" : "False"), (m_decel ? "True" : "False"), (m_turning ? "True" : "False"), \
                    m_turnTic, (m_tractored ? "True" : "False"), (m_tractorPause ? "True" : "False"));
        }

        speed = (m_maxShipSpeed * m_activeSpeedFraction);
    }

    // ships tend to "level out" when stopping.  try to mimic that here (wip)
    // this will also need *something* with ship agility
    if (m_stop and (m_timeFraction > 0.5f)) {
        if (m_shipHeading.y < -0.15f) {
            m_shipHeading.y += 0.05f;
        } else if (m_shipHeading.y > 0.15f) {
            m_shipHeading.y -= 0.05f;
        }
    }

    if (m_orbiting)
        if (m_orbiting < Destiny::Ball::Orbit::TooClose) {
            // object is orbiting...set orbit speed correctly.
            speed *= m_maxOrbitSpeedFraction;
            move += " in orbit";
    }

    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        if (m_prevSpeedFraction) {
            _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) is %s at %.3f m/s (tf:%.4f asf:%.4f ps:%.2f psf:%.4f, sec: %.5f).", \
                mySE->GetName(), mySE->GetID(), move.c_str(), speed, m_timeFraction, m_activeSpeedFraction, m_prevSpeed, m_prevSpeedFraction, timeStamp);
        } else {
            _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) is %s at %.3f m/s (tf:%.4f asf:%.4f sec: %.5f).", \
                mySE->GetName(), mySE->GetID(), move.c_str(), speed, m_timeFraction, m_activeSpeedFraction, timeStamp);
        }
    }

    //set velocity and position for this tic
    m_velocity = m_shipHeading * speed;
    SetPosition(m_position + m_velocity, sConfig.debug.PositionHack);   // (PositionHack == true) here will force position update to client

    if (is_log_enabled(DESTINY__MOVE_DEBUG))
        _log(DESTINY__MOVE_DEBUG, "Destiny::MoveObject() - %s(%u) Pos:%.2f,%.2f,%.2f  Vel:%.3f,%.3f,%.3f  Head:%.3f,%.3f,%.3f", \
            mySE->GetName(), mySE->GetID(), m_position.x, m_position.y, m_position.z, m_velocity.x, m_velocity.y, m_velocity.z,\
            m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);

    if (sEntityList.GetTracking()) {
        // only create can when ship is moving significant amount
        if (m_activeSpeedFraction > 0.002f) {
            // create jetcan to visualize object movement
            std::string str = mySE->GetName();
            str += "  ";
            str += itoa(timeStamp);
            ItemData idata(23, ownerSystem, mySE->GetLocationID(), flagNone, str.c_str(), m_position, "Position Test");
            CargoContainerRef iRef = CargoContainer::SpawnTemp(idata);
            if (iRef.get() != nullptr) {
                // create new container
                FactionData data = FactionData();
                ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
                if (cSE == nullptr)
                    return;
                iRef->SetMySE(cSE);
                mySE->SystemMgr()->AddMarker(cSE);
            }
        }
    }

    if (sConfig.cosmic.BumpEnabled)
        if (mySE->HasPilot() and mySE->SysBubble()->HasPlayers()) // no players in bubble = nothing to check against (for now)
            CheckBump();
}

bool DestinyManager::IsTurn() {    //this is working.  dont change
    if (m_targetPoint.isZero()) {
        _log(DESTINY__ERROR, "Destiny::IsTurn() - %s(%u): TargetPoint is null.", mySE->GetName(), mySE->GetID());
        if (mySE->HasPilot())
            mySE->GetPilot()->SendNotifyMsg("There was an error in your ship's navigation computer.  Ref: ServerError 35221");
        ClearTurn();
        Halt();
        return false;
    }
    // if ship is stopped, there is no turn.  immediately begin movement in desired direction
    if ((m_timeFraction < 0.1) and (m_activeSpeedFraction < 0.1)) {
        GVector toVec(m_position, m_targetPoint);
        toVec.normalize();
        m_shipHeading = toVec;
        return false;
    }

    // check for turning angle.  returns true if angle is enough to change movement variables
    // create isosceles triangle where legs are current direction and destination, then find angle between legs
    //  it will set m_radians in the range of [-pi,pi].
    /** @todo revisit this to verify angle calcs */
    GVector toVec(m_position, m_targetPoint);
    toVec.normalize();
    float dot(toVec.dotProduct(m_shipHeading));
    if ((dot > 1.0f) or (dot < -1.0f)) {
        sLog.Error("Destiny::IsTurn()", "%s(%u) - shipHeading has screwed up.  dot is %.5f", mySE->GetName(), mySE->GetID(), dot);
        _log(DESTINY__ERROR, "Destiny::IsTurn() m_shipHeading: %.3f,%.3f,%.3f.  m_targetHeading: %.3f,%.3f,%.3f, toVec:%.3f,%.3f,%.3f", \
                m_shipHeading.x, m_shipHeading.y, m_shipHeading.z, m_targetHeading.x, m_targetHeading.y, m_targetHeading.z, toVec.x, toVec.y, toVec.z);
        // try to correct for bad heading vector and retest...
             if (m_shipHeading.x > 1.0f)  { m_shipHeading.x -= 1; }
        else if (m_shipHeading.x < 1.0f)  { m_shipHeading.x += 1; }
             if (m_shipHeading.y > 1.0f)  { m_shipHeading.y -= 1; }
        else if (m_shipHeading.y < 1.0f)  { m_shipHeading.y += 1; }
             if (m_shipHeading.z > 1.0f)  { m_shipHeading.z -= 1; }
        else if (m_shipHeading.z < 1.0f)  { m_shipHeading.z += 1; }
        dot = toVec.dotProduct(m_shipHeading);
        if ((dot > 1.0f) or (dot < -1.0f)) {
            sLog.Error("Destiny::IsTurn()", "%s(%u) - shipHeading has screwed up AGAIN.  dot is %.5f", mySE->GetName(), mySE->GetID(), dot);
            return false;
        }
    }
    m_radians = std::acos(dot);
    float degrees(EvE::Trig::Rad2Deg(m_radians));
    if (degrees < TURN_ALIGNMENT/*4*/) {
        m_shipHeading = toVec;
        return false;
    }
    if (is_log_enabled(DESTINY__TURN_TRACE)) {
        _log(DESTINY__TURN_TRACE, "Destiny::IsTurn() - %s(%u): dot: %.5f, radians:%.5f, degrees:%.3f",\
            mySE->GetName(), mySE->GetID(), dot, m_radians, degrees);
        _log(DESTINY__TURN_TRACE, "Destiny::IsTurn() m_shipHeading: %.3f,%.3f,%.3f.  m_targetHeading: %.3f,%.3f,%.3f", \
            m_shipHeading.x, m_shipHeading.y, m_shipHeading.z, m_targetHeading.x, m_targetHeading.y, m_targetHeading.z);
    }
    return true;
}

/* Quaternion slerp(Quaternion const &v0, Quaternion const &v1, double t) {
 *   // v0 and v1 should be unit length or else something broken will happen.
 *
 *   // Compute the cosine of the angle between the two vectors.
 *   double dot = dot_product(v0, v1);
 *
 *   const double DOT_THRESHOLD = 0.9995;
 *   if (dot > DOT_THRESHOLD) {
 *       // If the inputs are too close for comfort, linearly interpolate
 *       // and normalize the result.
 *
 *       Quaternion result = v0 + t*(v1 – v0);
 *       result.normalize();
 *       return result;
 *   }
 *
 *   Clamp(dot, -1, 1);           // Robustness: Stay within domain of acos()
 *   double theta_0 = acos(dot);  // theta_0 = angle between input vectors
 *   double theta = theta_0*t;    // theta = angle between v0 and result
 *
 *   Quaternion v2 = v1 – v0*dot;
 *   v2.normalize();              // { v0, v2 } is now an orthonormal basis
 *
 *   return v0*cos(theta) + v2*sin(theta);
 * }
 */

//from new source at eve/client/script/ui/services\flightControls.py
//  self.curve = trinity.Tr2QuaternionLerpCurve()
void DestinyManager::Turn() {   // tracking within 900m for Frigates, 1k4m for BS.  05Jun17
    if (mySE->HasPilot())
        if (mySE->GetPilot()->IsUndock())
            return;

    ++m_turnTic;

    if (!IsTurn()) {
        if (m_turning)
            ClearTurn();
        return;
    }
    /*when changing directions....
     *  m_moveTime will have to be reset - handled in UpdateVelocity()
     *  m_shipHeading will have to be reset - reset here and used in MoveObject() (our calling function)
     *  check for decel, then call UpdateVelocity() to set variables as needed.  Move() will handle the rest.
     *
     * this below isnt right, hack is almost close
     *   m_degPerTic = (60.0f - m_shipAgility) / 10;  ([this file]:2317, reset for ab/mwd [this file]:2154)
     */

    // this is off for rookie ship (maybe others)
    float turnTime(m_shipAgility / 2.2);
    if (!m_turning) {
        m_turning = true;
        //m_radians is set in IsTurn() on every tic
        m_turnFraction = std::sqrt((std::cos(m_radians) + 1) / 2);
        //this isnt used yet...used as comparison for testing time calc's
        m_alignTime = (EvE::Trig::Rad2Deg(m_radians) / m_degPerTic);
        if (is_log_enabled(DESTINY__TURN_TRACE))
            _log(DESTINY__TURN_TRACE, "Destiny::Turn() - %s(%u): Agility:%.3f, Inertia:%.3f, alignTime:%.3f, turnTime:%.3f, turnFraction:%.3f, m_degPerTic:%.3f", \
                mySE->GetName(), mySE->GetID(), m_shipAgility, m_shipInertia, m_alignTime, turnTime, m_turnFraction, m_degPerTic);
    }

    // logic to determine speed changes for turning
    if (m_turnTic == 1)
        if (m_turnFraction < m_timeFraction)
            UpdateVelocity(true);

    // need to check turnFraction vs m_timeFraction to hold speed when turning.

    /*  class          agility
     * Capsule          .06
     * Shuttle          1.6
     * Rookie           5
     * Frigates         3 - 6 (adv. 3 - 4)  (2s) 1s         < 0.15 not enough.
     * Destroyers       4 - 5
     * Cruisers         4 - 8
     * T3 Cruiser       2.4 - 2.8
     * HAC              5 - 7
     * Battlecruisers   6 - 9
     * Battleships      8 - 14      (12s) 4s        0.15 works well  0.2 is very well.   > 0.25 is too much.
     * Industrials      8 - 12
     * Marauder         ~12
     * Orca             40          (40s) 18s   0.05 turnPercent seems to work very well.  > 0.1 is wrong.
     * Freighters       ~60
     * Supercarrier     ~60
     * Command          ~9
     * Transport        5 or 19
     * Barges           10 - 18
     * Dreadnought      ~55
     * Zephyr           5
     */
    // set ship turn amount based on position in turn, current speed and ship agility
    GVector deltaHeading(m_shipHeading, m_targetHeading);
    float turnPercent(0.1f);
    float degrees(EvE::Trig::Rad2Deg(m_radians));
    if (degrees > 100) {
        if (m_decel and (m_turnTic > turnTime)) {
            // turn half of remaining turn (simulate greatest turn angle when (turn > 90*) and (speed < time)
            turnPercent = 0.3f;
        } else {
            turnPercent = m_degPerTic / (degrees - 100);
        }
    } else if (degrees > m_degPerTic) {
        turnPercent = m_degPerTic / (degrees * 0.5);
    } else {
        // degrees < m_degPerTic, so complete turn and continue accel
        if (m_decel)
            UpdateVelocity(true);
    }

    if (turnPercent > 0.9f) {
        _log(DESTINY__ERROR, "Destiny::Turn() - turnTic:%u, degRemain:%.3f, turnPercent:%.2f", m_turnTic, degrees, turnPercent);
        turnPercent = 0.9;
    }
    deltaHeading *= turnPercent;
    m_shipHeading += deltaHeading;
    if (is_log_enabled(DESTINY__TURN_TRACE))
        _log(DESTINY__TURN_TRACE, "Destiny::Turn() - tf:%.3f, turnTic:%u, degRemain:%.3f  (deltaHeading:%.5f, %.5f, %.5f * turnPercent:%.2f) = shipHeading:%.3f, %.3f, %.3f", \
            m_timeFraction, m_turnTic, degrees, deltaHeading.x, deltaHeading.y, deltaHeading.z, turnPercent, m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
}

void DestinyManager::ClearTurn() {
    SetPosition(m_position, sConfig.debug.PositionHack);   // (PositionHack == true) here will force position update to client
    m_turnTic = 0;
    m_turning = false;
    m_radians = 0.0f;
    m_turnFraction = 0.0f;
    m_alignTime = m_timeToEnterWarp;
}

void DestinyManager::Follow() {
    //  Follow is also used by client as AlignTo.
    const GPoint& target_point = m_targetEntity.second->GetPosition();
    GVector heading(m_position, target_point);
    m_targetDistance = (uint32)(heading.length() - m_radius);

    if (m_targetDistance < m_followDistance) {
        if (mySE->HasPilot())
            if (mySE->GetPilot()->IsAutoPilot()) {
                SetSpeedFraction(0.1);
                _log(AUTOPILOT__TRACE, "DestinyManager::Follow() - Target within FollowDistance.  SpeedFraction = 0.1.");
                return;
            }
    // this will allow following entities to keep their follow state, yet stop movement if within their follow distance.
    //  by keeping their follow state, once the distance is greater than their follow distance, they will begin movement again.
        if (m_tractored) {
            // specific to tractored entities.  sudden halt to mimic tractor stopping
            if (!m_tractorPause) {
                std::vector<PyTuple*> updates;
                CmdSetSpeedFraction ssf;
                    ssf.entityID = mySE->GetID();
                    ssf.fraction = 0;
                updates.push_back(ssf.Encode());
                SendDestinyUpdate(updates);
            }
            m_velocity = NULL_ORIGIN_V;
            m_tractorPause = true;
            m_activeSpeedFraction = m_userSpeedFraction = m_timeFraction = m_prevSpeedFraction = 0.0f;
            return;
        } else {
            if ((m_targetEntity.second->IsDynamicEntity()) and (m_targetEntity.second->DestinyMgr()->IsMoving())) {
                // this will mimic real movement, where ship will decel instead of a sudden halt
                //  still need to call MoveObject() here
                SetSpeedFraction(0.2);
            } else {
                Stop();
            }
        }
    } else {
        if (m_tractored and m_tractorPause) {
            // tractored object is outside follow distance.  begin movement again
            if (m_tractorPause) {
                std::vector<PyTuple*> updates;
                CmdSetSpeedFraction ssf;
                    ssf.entityID = mySE->GetID();
                    ssf.fraction = 1;
                updates.push_back(ssf.Encode());
                SendDestinyUpdate(updates);
            }
            m_tractorPause = false;
            m_velocity = m_shipHeading * m_maxSpeed;
            m_moveTime = GetTimeMSeconds();
            m_stateStamp = sEntityList.GetStamp();
            m_prevSpeedFraction = 0.0f;
            // there is no accel/decel for tractor'd items
            m_activeSpeedFraction = m_userSpeedFraction = m_timeFraction = 1;
        } else if (m_userSpeedFraction != 0.0f) {
            SetSpeedFraction(1.0f);
        }
    }

    heading.normalize();
    m_targetPoint = target_point + (heading * m_targetDistance);

    MoveObject();
}

/*eve/client/script/ui/services\flightPredictionSvc.py
"""
Prediction service for in-space flight
"""
*/
void DestinyManager::Orbit() {
    // data consistency checks...
    if ((m_targetDistance > BUBBLE_RADIUS_METERS) or (m_followDistance > BUBBLE_RADIUS_METERS)) {
        // well, something fucked up.  stop object and throw error.   player can reset if they want to.
        if (mySE->HasPilot())
            mySE->GetPilot()->SendErrorMsg("Internal Server Error.  Ref: ServerError 35412");
        sLog.Error("Destiny::Orbit()", "%s(%u) - Distance check OOB. ", mySE->GetName(), mySE->GetID());
        Stop();
        return;
    }

    // this will set position of ship relative to target, based on period of orbit.

    /*   destiny variables used here
     * m_position - probably the most important calculated value.
     * m_velocity - 2nd most important calculated value
     * m_targetDistance - commanded orbit distance
     * m_followDistance - calculated orbit distance based on mass, velocity, gravity, and other ship variables
     * m_targetHeading - direction to target from current position
     * m_targetPoint - calculated distant point from above variable
     * m_shipHeading - current direction ship is pointed
     * m_stateStamp - time movement started.  1Hz tic
     * m_orbiting - 0=no orbit, >0=in orbit, 1=at distance, 2=too close , 3=too far, 4=way too close, 5=way too far
     * m_orbitRadTic - rad/sec in current orbit.  set by Orbit() (~2090)
     * m_maxOrbitSpeedFraction - calculated max speed to maintain commanded orbit distance.  set in Orbit() but not used here yet
     *
     *   our target variables
     * Tr = target radius
     * Tp = target position  (updated for movement, if applicable)
     * Tv = target velocity
     * Th = target heading   (updated for movement, if applicable)
     * Tm = target mass
     *
     * centers = distance between object and target centers
     * edges = distance between object and target closest edges (counting for radius)
     *
     */

    /** @todo  will have to set/reset orbit time once actual orbit is started for proper radian setting */
    // get current times
    uint32 timeStamp = sEntityList.GetStamp() - m_stateStamp;
    float Tr = m_targetEntity.second->GetRadius();
    //float Tm = m_targetEntity.second->GetSelf()->GetAttribute(AttrMass).get_float();
    GPoint Tp(m_targetEntity.second->GetPosition());

    // current and edges are used to determine ship's orbit distance, and adjust position accordingly
    double centers(m_position.distance(Tp));
    double edges(centers - m_radius - Tr);
    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "1 - %s(%u): time:%u, centers:%.2f, edges:%.2f, target:%u, follow:%u", \
            mySE->GetName(), mySE->GetID(), timeStamp, centers, edges, m_targetDistance, m_followDistance);

    // distances checks for orbit calculations
    GPoint mPos(NULL_ORIGIN);
    float mPosAdj(0.0f);
    // check distances for this tic
    if ((edges / 2) > m_followDistance) {
        if (m_orbiting == Destiny::Ball::Orbit::TooFar) {
            MoveObject();
            return;
        }
        // too far to realistically orbit.
        m_orbiting = Destiny::Ball::Orbit::TooFar;
        // TODO: update this to determine orbit and set heading/target to smoothly go from turn into orbit trajectory
        // set point to side of target (based on current position), to avoid near-zero angular velocity
        double radTarg = atan2(Tp.z - m_position.z, Tp.x - m_position.x);  // rad from '0' to target
        radTarg += atan2(m_followDistance, edges);  // rad from 'distance line' to target 'offset'
        mPos.x = m_followDistance * cos(radTarg);
        mPos.z = m_followDistance * sin(radTarg);
        if (Tp.y > m_position.y) { // target is above us.  set point below target using calculated distance
            mPos.y = Tp.y - m_position.y;
        } else { // opposite of above
            mPos.y = m_position.y - Tp.y;
        }
        m_targetPoint = Tp + mPos;
        GVector heading(m_position, m_targetPoint);
        heading.normalize();
        m_shipHeading = heading;    // this sets object velocity using speed
        _log(DESTINY__ORBIT_TRACE, "2 - way too far - rads:%.3f, heading: %.3f, %.3f, %.3f", \
                radTarg, m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
        MoveObject();
        return;
    } else if ( (centers + m_targetDistance / 3) < m_followDistance) {
        if (m_orbiting == Destiny::Ball::Orbit::TooClose) {
            MoveObject();
            return;
        }
        // to close to realistically orbit.  move away from target
        m_orbiting = Destiny::Ball::Orbit::TooClose;
        // set point to side of target (based on current position), to avoid near-zero angular velocity
        double radTarg = atan2(Tp.z - m_position.z, Tp.x - m_position.x);  // rad from '0' to target
        //radTarg += atan2(m_followDistance, edges);  // rad from 'distance line' to target 'offset'
        mPos.x = m_followDistance * cos(radTarg);
        mPos.z = m_followDistance * sin(radTarg);
        if (Tp.y > m_position.y) {  // target is above us.  set point below target using calculated distance
            mPos.y = Tp.y - m_position.y;
        } else { // opposite of above
            mPos.y = m_position.y - Tp.y;
        }
        m_targetPoint = Tp + mPos;
        GVector heading(m_position, m_targetPoint);
        heading.normalize();
        m_shipHeading = heading;    // this sets object velocity using speed
        _log(DESTINY__ORBIT_TRACE, "2 - way too close - rads:%.3f, heading: %.3f, %.3f, %.3f", \
                radTarg, m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
        MoveObject();
        return;
    } else if ((edges - m_targetDistance / 4) > m_followDistance) {
        m_orbiting = Destiny::Ball::Orbit::Far;
        // fudge distance for a smaller orbit
        // modify this based on calculated distance
        mPosAdj = static_cast<float>(-static_cast<int64>(m_followDistance)) / 25;
        _log(DESTINY__ORBIT_TRACE, "2 - too far");
    } else if (centers < m_followDistance) {
        m_orbiting = Destiny::Ball::Orbit::Close;
        // fudge distance for larger orbit
        // modify this based on calculated distance
        mPosAdj = m_followDistance / 25;
        _log(DESTINY__ORBIT_TRACE, "2 - too close");
    } else {
        m_orbiting = Destiny::Ball::Orbit::Orbiting;
        _log(DESTINY__ORBIT_TRACE, "2 - within tolerance");
    }

    #define LogMacro(v) _log(DESTINY__ORBIT_TRACE, "m - " #v ": (%.3f, %.3f, %.3f)   len=%.3f", v.x, v.y, v.z, v.length())

    // new orbit code
    float radius = m_followDistance + mPosAdj;// fudge a bit as using targetDistance is a hair too close
    // angle around y axis (from +x) - horizontal movement  - ccw from +x using ships orbit in rad/tic
    float theta = EvE::Trig::Pi2 - EvE::Trig::Deg2Rad(360) - (m_orbitRadTic * timeStamp);
    // angle around xz axis (from 0) - vertical movement
    //GVector target(m_position, Tp);
    //LogMacro(target);
    //float hyp = sqrt(pow(target.z, 2) + pow(target.x, 2));
    float inclination = 45; //atan(hyp / target.y);
    // fractional value of orbit period (0 < x < 1)
    float period = fmod(timeStamp, m_orbitTime) / m_orbitTime;
    // calculate a pendulum value here to adjust elevation (+/-y) where +x is 1, 0x is 0, -x is -1
    float c = cos(EvE::Trig::Deg2Rad(360 * period));
    // get elevation modifier based on orbit period
    float phi = EvE::Trig::Deg2Rad(inclination * c);
    // set xz plane modifier from elevation
    float s = sin(EvE::Trig::Deg2Rad(360 * period));
    float mu = EvE::Trig::Deg2Rad(inclination * s);
    // here we will adjust orbit plane by adding OrbitRotation angle to theta
    // calculate position
    mPos.x = radius /* mu */* cos( theta );
    mPos.z = radius /* mu */* sin( theta );
    mPos.y = radius * phi;
    _log(DESTINY__ORBIT_TRACE, "4 - theta:%.5f, phi:%.3f, mu:%.2f period:%.5f, radius:%.3f, inc:%.5f", theta,phi,mu,period,radius,inclination);
    LogMacro(mPos);
    // apply origin to our calculated position
    mPos += Tp;
    // set position for this tic
    m_position = mPos;

    // set heading for this tic
    GPoint mPosNext(NULL_ORIGIN);
    theta += m_orbitRadTic;
    period = fmod(timeStamp + 1, m_orbitTime) / m_orbitTime;
    c = cos(EvE::Trig::Deg2Rad(360 * period));
    phi = EvE::Trig::Deg2Rad(inclination * c);
    mPosNext.x = radius * cos( theta );
    mPosNext.z = radius * sin( theta );
    mPosNext.y = radius * phi;
    LogMacro(mPosNext);
    // determine where our target should be next tic, and figure that into our heading calculation
    float Tv = (m_targetEntity.second->DestinyMgr() != nullptr ? m_targetEntity.second->DestinyMgr()->GetSpeed() : 0);
    GVector Th(m_targetEntity.second->DestinyMgr() != nullptr ? m_targetEntity.second->DestinyMgr()->GetHeading() : NULL_ORIGIN_V);
    Tp += (Tv*Th); // use Tv*Th and add to position to account for target movement.  Tv for non-moving targets return 0.
    mPosNext += Tp;
    GVector heading(m_position, mPosNext);
    heading.normalize();
    m_shipHeading = heading;
    m_targetPoint = m_position + (m_shipHeading * 1.0e16);
    LogMacro( heading );

    double curSpeed = m_maxSpeed * m_activeSpeedFraction * m_maxOrbitSpeedFraction;
    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "5(%u) - orbiting at %.2f. timestamp:%u, speed:%.2f", \
            m_orbiting, m_position.distance(Tp), timeStamp, curSpeed);

    MoveObject();
}

GPoint DestinyManager::ComputePosition(double curRad) {
    /*
     *   orbital definitions for EVEmu:
     * node line = ascending node
     * ascending node = line of intersection between orbit plane and reference plane, on the 'upward' side
     * periapsis = closest point of orbit to target point
     * apoapsis = farthest point of orbit to target point
     * ecliptic = plane of orbit
     * reference direction = line on reference plane inline with periapsis
     *
     *   primary orbital elements:
     * Y = reference direction (vector on reference plane that lines up with periapsis)
     * i = inclination to the positive ecliptic at node line
     * a = semi-major axis, or mean distance to target.  this will adjust ship's orbit based on distance
     * e = eccentricity (0=circle, 0-1=ellipse, 1=parabola)
     * w = argument of periapsis, angle from the node line to the periapsis
     * N = longitude of the ascending node (from Y, ccw to node line)
     * NOTE: w + N = 360*
     * M = mean anomaly, radians between our current position and periapsis.  increases uniformly with time from 0 to 2pi (360_deg)
     *
     *   calculated orbital elements:
     * L  = M + p   = mean longitude, measure of how far around its orbit a body has progressed since passing the argument of periapsis (w)
     * P  = orbital period,  time in seconds to complete one orbit (assuming all other variables remain constant)
     * T  = Epoch_of_M - (M(deg)/360_deg) / P  = time of periapsis
     * v  = true anomaly, position of the orbiting body along the orbit at a specific time, measured from w
     * E  = eccentric anomaly, angle from target side of center point of line qQ, at which we are located
     *
     * Under ideal conditions of a perfectly spherical central body and zero perturbations,
     *    all orbital elements except the mean anomaly (M) are constants.
     *
     * As we're using circular orbits, the reference direction (Y) will be +x
     *
     */

    GPoint Tp(m_targetEntity.second->GetPosition());
    //i = inclination to the positive ecliptic (plane of our orbit) at node line
    double adj = sqrt(pow(m_position.x - Tp.x, 2) * pow(m_position.z - Tp.z, 2));
    double opp = m_position.y - Tp.y;
    double i = atan2(opp, adj);

    /*  not needed yet, but here just in case...
    // to determine orbiters position relative to target, use RA and dec (from their point of view, with vernal equinox being their heading)
    // calculating right ascension (RA)
    double A = cos(w) * cos(N) - sin(w) * sin(N) * cos(i);
    double B = cos(cos(w) * cos(N) + sin(w) * sin(N) * cos(i)) - sin(sin(w) * sin(i));
    double RA = atan2(B, A);
    // calculating declination (dec)
    double C = sin(cos(w) * cos(N) + sin(w) * sin(N) * cos(i)) + cos(sin(w) * sin(i));
    double dec = asin(C);
    */

    GPoint mPos(NULL_ORIGIN);
    // fig 8 on nw of targ sphere
    float radius = m_targetDistance + (m_radius *2); // fudge a bit as using targetDistance is a hair too close
    // angle around y axis (from +x) - horizontal movement  - cw from +x using ships orbit in rad/tic
    float theta = m_orbitRadTic * 0/*timeStamp*/;
    // angle around xz axis (from 0) - vertical movement
    GVector target(m_position, Tp);
    LogMacro(target);
    float hyp = sqrt(pow(target.z, 2) + pow(target.x, 2));
    float inclination = 45; //atan(hyp / target.y);
    // fractional value of orbit period (0 < x < 1)
    float period = fmod(0/*timeStamp*/, m_orbitTime) /m_orbitTime;
    // calculate a pendulum value here to adjust elevation (+/-y) where +x is 1, 0x is 0, -x is -1
    float c = cos(EvE::Trig::Deg2Rad(360 * period));
    // get elevation modifier based on orbit period
    float phi = EvE::Trig::Deg2Rad(inclination * c);
    // set xz plane modifier from elevation
    //float s = sin(EvE::Trig::Deg2Rad(360 * period));
    //float mu = EvE::Trig::Deg2Rad(inclination * s);
    // here we will adjust orbit plane by adding OrbitRotation angle to theta
    // calculate position using trig.
    mPos.x = radius /* mu */* cos( theta );
    mPos.z = radius /* mu */* sin( theta );
    mPos.y = radius * phi;

    //_log(DESTINY__ORBIT_TRACE, "Destiny::ComputePosition() - a:%.5f, i:%.5f, v:%.5f, N:%.5f, M:%.5f, w:%.5f, e:%.5f, E:%f, P:%f", a,i,v,N,M,w,e,E,P);
    //_log(DESTINY__ORBIT_TRACE, "Destiny::ComputePosition() - mPos(%.3f, %.3f, %.3f) - radius check %.3f, q:%.3f", mPos.x, mPos.y, mPos.z, r,q);

    // position test
    if (mPos.isNaN()) {
        _log(DESTINY__ERROR, "mPos calculated as NaN.  Stopping Orbit.");
        Stop();
        return NULL_ORIGIN;
    }
    // get new position as reference to target
    return mPos;
}

void DestinyManager::ClearOrbit() {
    m_orbiting = Destiny::Ball::Orbit::None;
    m_orbitTime = 0.0f;
    m_orbitRadTic = 0.0f;
    m_targetDistance = 0;
    m_followDistance = 0;
    m_maxOrbitSpeedFraction = 1.0f;
}

void DestinyManager::InitWarp() {
    //init warp:

    // warp time and distance math
    //   allan 1Nov14 - 14Nov14
    //  rewrite 3jan15  to use distance instead of time for warping.  more accurate now, and covers ALL distances.
    //  calculation and implementation update   9Jan15      accuracy is within 1000m

    /*  my research into warp formulas, i have used these sites, with a few excerpts and ideas from each...
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
     *   accel/decel are logarithmic per ccp (see above).
     *
     * times are as follows, per this table.  http://cdn1.eveonline.com/www/newssystem/media/65418/1/numbers_table.png
     *
     * all warps are in same time groups for all ships, except freighters and caps.
     * distance checks are separated into 4 time groups, with subgroups for freighters and caps.
     *
     * the client seems to accept and agree with the math here.
     */
    // reset turn variables for warping
    if (m_turning) {
        ClearTurn();
    }

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(
            DESTINY__WARP_TRACE,
            "Destiny::InitWarp(): %s(%u) has initialized warp.",
            mySE->GetName(),
            mySE->GetID()
        );
    }

    double warpSpeedInMeters(static_cast<double>(m_shipWarpSpeed) * static_cast<double>(ONE_AU_IN_METERS));

    /* this is from http://community.eveonline.com/news/dev-blogs/warp-drive-active/
     * x = e^(k*t)
     * v = k*e^(k*t)
     *
     * x = distance in meters
     * t = time in seconds
     * v = speed in m/s
     * k = 3 for accel, 1 for decel
     *
     * this gives distances as functions of time.
     * the client seems to agree with this reasoning, and follows the same idea.
     */

    bool cruise(true);
    float cruiseTime(0.0f);
    double accelDistance(0.0), decelDistance(0.0), cruiseDistance(0.0);
    // fudge this a bit for accel/decel distances
    if (abs(static_cast<double>(m_targetDistance)) < warpSpeedInMeters) {
        _log(
            DESTINY__WARP_TRACE,
            "short warp distance dictates that warp cruise time is unnecessary"
        );

        // short warp....no cruise
        // this isnt very accurate....times and distances are a bit off....
        cruise = false;
        // accel = 1/3 decel
        accelDistance = (static_cast<double>(m_targetDistance) / static_cast<double>(3));
        decelDistance = (static_cast<double>(m_targetDistance) - accelDistance);
        warpSpeedInMeters = accelDistance;
        m_warpDecelTime = log(decelDistance / static_cast<double>(3));
        m_warpAccelTime = log(accelDistance / static_cast<double>(3)) / static_cast<double>(3);
    } else {
        _log(
            DESTINY__WARP_TRACE,
            "longer warp distance dictates that warp cruise time is is warranted"
        );

        // all ships base time is 29s for distances > ship warp speed
        m_warpAccelTime = 7;
        m_warpDecelTime = 21; // accel *3
        decelDistance = exp(static_cast<double>(m_warpDecelTime));   // ship warp speed in meters * 1.7
        accelDistance = exp(static_cast<double>(3) * static_cast<double>(m_warpAccelTime));       // ship warp speed in meters
        cruiseDistance = (static_cast<double>(m_targetDistance) - accelDistance - decelDistance);
        cruiseTime = static_cast<float>(cruiseDistance / warpSpeedInMeters);
    }

    //  set total warp time based on above math.
    float warpTime(static_cast<float>(m_warpAccelTime) + static_cast<float>(m_warpDecelTime) + std::floor(cruiseTime));

    GVector warp_vector(m_position, m_targetPoint);
    warp_vector.normalize();

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(
            DESTINY__WARP_TRACE,
            "Destiny::InitWarp():Calculate - %s(%u): Warp will accelerate for %us, cruise for %.3f, then decelerate for %us, with total time of %.3fs, and warp speed of %.4f m/s.",
            mySE->GetName(),
            mySE->GetID(),
            m_warpAccelTime,
            cruiseTime,
            m_warpDecelTime,
            warpTime,
            warpSpeedInMeters
        );

        _log(
            DESTINY__WARP_TRACE,
            "Destiny::InitWarp():Calculate - %s(%u): Accel distance is %.4f. Cruise distance is %.4f.  Decel distance is %.4f.  Direction is %.3f,%.3f,%.3f.",
            mySE->GetName(),
            mySE->GetID(),
            accelDistance,
            cruiseDistance,
            decelDistance,
            warp_vector.x,
            warp_vector.y,
            warp_vector.z
        );

        _log(
            DESTINY__WARP_TRACE,
            "Destiny::InitWarp():Calculate - %s(%u): We will exit warp at %.2f,%.2f,%.2f at a distance of %.4f AU (%.2fm).",
            mySE->GetName(),
            mySE->GetID(),
            m_targetPoint.x,
            m_targetPoint.y,
            m_targetPoint.z,
            static_cast<double>(m_targetDistance)/static_cast<double>(ONE_AU_IN_METERS),
            static_cast<double>(m_targetDistance)
        );

        GPoint destination = m_position + (warp_vector * m_targetDistance);
        _log(
            DESTINY__WARP_TRACE,
            "Destiny::InitWarp():Calculate - %s(%u): calculated exit is %.2f,%.2f,%.2f and vector is %.4f,%.4f,%.4f.",
            mySE->GetName(),
            mySE->GetID(),
            destination.x,
            destination.y,
            destination.z,
            warp_vector.x,
            warp_vector.y,
            warp_vector.z
        );

        GVector diff(m_targetPoint, destination);
        _log(
            DESTINY__WARP_TRACE,
            "Destiny::InitWarp():Calculate - target vs calculated is %.2fm, and m_targetDistance is %.2f m.",
            diff.length(),
            m_targetDistance
        );
    }

    // reset deceltime (from duration to time) for time check in WarpDecel()
    m_warpDecelTime = m_warpAccelTime + floor(cruiseTime);
    m_stateStamp = sEntityList.GetStamp();

    SafeDelete(m_warpState);

    m_warpState = new WarpState(
        m_stateStamp,
        m_targetDistance,
        warpSpeedInMeters,
        accelDistance,
        cruiseDistance,
        decelDistance,
        warpTime,
        true,
        false,
        false,
        warp_vector
    );

    //drain cap
    if (mySE->HasPilot()) {
        mySE->GetSelf()->SetAttribute(AttrCapacitorCharge, m_capNeeded);
        mySE->GetShipSE()->Warp();
        m_capNeeded = 0;
    }

    //clear targets
    mySE->TargetMgr()->ClearAllTargets();
    //mySE->TargetMgr()->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::WarpingOut);

    WarpAccel(0);
}

void DestinyManager::WarpAccel(uint16 sec_into_warp) {
    /* For acceleration, k = 3.
     * distance = e^(k*s)
     * speed = k*e^(k*s)
     */
    double currentDistance = exp(3 * sec_into_warp);

    if (mySE->SysBubble() != nullptr && currentDistance > BUBBLE_RADIUS_METERS && mySE->SysBubble() != m_targBubble) {
        if (is_log_enabled(DESTINY__WARP_TRACE)) {
            _log(
                DESTINY__WARP_TRACE,
                "Destiny::WarpAccel(): %s(%u) is being removed from bubble %u.",
                mySE->GetName(),
                mySE->GetID(),
                mySE->SysBubble()->GetID()
            );
        }

        mySE->SysBubble()->Remove(mySE);
    }

    if (currentDistance > m_warpState->accelDist) {
        currentDistance = m_warpState->accelDist;
        m_warpState->accel = false;
        if (m_warpState->cruiseDist > 0) {
            m_warpState->cruise = true;
        } else {
            m_warpState->decel = true;
        }
    }

    m_targetDistance -= currentDistance;
    double currentShipSpeed = (3 * currentDistance);

    if (is_log_enabled(DESTINY__WARP_TRACE) && m_warpState->accel) {
        _log(
            DESTINY__WARP_TRACE,
            "Destiny::WarpAccel(): %s(%u) - Warp Accelerating(%us): velocity %.4f m/s with %.2f m left to go. Current distance %.4f from origin.",
            mySE->GetName(),
            mySE->GetID(),
            sec_into_warp,
            currentShipSpeed,
            m_targetDistance,
            currentDistance
        );
    }

    WarpUpdate(currentShipSpeed);
}

void DestinyManager::WarpCruise(uint16 sec_into_warp) {
    /* in cruise....calculate distance only to update internal position data. */
    m_targetDistance -= m_warpState->warpSpeed;

    if ((m_targetDistance - m_warpState->warpSpeed) < m_warpState->decelDist) {
        m_warpState->cruise = false;
        m_warpState->decel = true;
    }

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(
            DESTINY__WARP_TRACE,
            "Destiny::WarpCruise(): %s(%u) - Warp Cruising(%us): velocity %.4f m/s. with %.2f m left to go.",
            mySE->GetName(),
            mySE->GetID(),
            sec_into_warp,
            m_warpState->warpSpeed,
            m_targetDistance
        );
    }

    WarpUpdate(m_warpState->warpSpeed);
}

void DestinyManager::WarpDecel(uint16 sec_into_warp) {
    /* For deceleration, k = -1.
     * distance = e^(k*s)
     * speed = -k*e^(k*s)
     */
    uint8 decelTime = (sec_into_warp - m_warpDecelTime);
    double currentDistance = (m_warpState->total_distance - (exp(-decelTime) * m_warpState->decelDist));
    m_targetDistance = static_cast<double>(m_warpState->total_distance - currentDistance);
    double currentShipSpeed = (m_warpState->warpSpeed * exp(-decelTime));

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::WarpDecel(): %s(%u) - Warp Decelerating(%us/%us): velocity %.4f m/s with %.2f m left to go.", \
                mySE->GetName(), mySE->GetID(), decelTime, sec_into_warp, currentShipSpeed, m_targetDistance);

    WarpUpdate(currentShipSpeed);
    if (currentShipSpeed <= m_speedToLeaveWarp)
        WarpStop(currentShipSpeed);
}

void DestinyManager::WarpUpdate(double currentShipSpeed) {
    //  update position and velocity for all stages.
    //  this method is ~1000m off actual.  could be due to rounding.   -allan 9Jan15
    m_velocity = (m_warpState->warp_vector * currentShipSpeed);
    SetPosition(m_targetPoint - (m_warpState->warp_vector * m_targetDistance));

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(
            DESTINY__WARP_TRACE,
            "Destiny::WarpUpdate()  %s(%u): Ship is %f from center of target bubble %u.",
            mySE->GetName(),
            mySE->GetID(),
            m_targBubble->GetCenter().distance(m_position),
            m_targBubble->GetID()
        );
    }

    if (m_targBubble->InBubble(m_position, true)) {
        if (is_log_enabled(DESTINY__WARP_TRACE)) {
            _log(
                DESTINY__WARP_TRACE,
                "Destiny::WarpUpdate()  %s(%u): Ship at %.2f,%.2f,%.2f is calling Add() for bubble %u.",
                mySE->GetName(),
                mySE->GetID(),
                m_position.x,
                m_position.y,
                m_position.z,
                m_targBubble->GetID()
            );
        }

        m_targBubble->Add(mySE);

        SetPosition(m_position, true);
    } else {
        _log(
            DESTINY__WARP_TRACE,
            "Destiny::WarpUpdate()  %s(%u): adding to midWarpSystemBubble.",
            mySE->GetName(),
            mySE->GetID()
        );
        SystemBubble* midWarpSystemBubble(sBubbleMgr.GetBubble(mySE->SystemMgr(), m_position));
        midWarpSystemBubble->Add(mySE);
    }
}

void DestinyManager::WarpStop(double currentShipSpeed) {
    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__WARP_TRACE, "Destiny::WarpStop(): %s(%u) - Warp complete. Exit velocity %.4f m/s with %.2f m left to go.", \
                mySE->GetName(), mySE->GetID(), currentShipSpeed, m_targetDistance);
        _log(DESTINY__WARP_TRACE, "Destiny::WarpStop(): %s(%u): Ship currently at %.2f,%.2f,%.2f.", \
                mySE->GetName(), mySE->GetID(), m_position.x, m_position.y, m_position.z);
    }
    if (mySE->IsShipSE()) {
        _log(AUTOPILOT__MESSAGE, "Destiny::WarpStop(): %s(%u) - Warp complete.", mySE->GetName(), mySE->GetID());
        mySE->GetPilot()->SetLoginWarpComplete();
    }
    m_targetPoint += (m_warpState->warp_vector *10000);
    // SetSpeedFraction() checks for m_state = Warp and warpstate != null to set decel variables correctly with warp decel.
    //   have to call this BEFORE deleting or reseting m_state or WarpState.
    SetSpeedFraction(0.0f);
    m_stop = true;
    SafeDelete(m_warpState);
    m_targBubble = nullptr;
    if ((mySE->IsNPCSE()) and (mySE->GetNPCSE()->GetAIMgr() != nullptr)) {
        mySE->GetNPCSE()->GetAIMgr()->WarpOutComplete();
    }

    // TODO: when exiting warp, and attempting to warp again shortly after, the
    // ball mode reaches a weird state where it goes from Warp to a regular
    // move. Halting the ship after warp completes seems to fix this, but it's
    // not a good fix, because the client shows that the ship moves a few meters
    // forward while decelerating - meaning that the client and server are
    // briefly out of sync because the server thinks the ship is halted.
    Halt();
}

//called whenever an entity is going away and can no longer be used as a target
void DestinyManager::EntityRemoved(SystemEntity *pSE) {
    if (m_targetEntity.second == pSE) {
        m_targetEntity.first = 0;
        m_targetEntity.second = nullptr;

        switch(m_ballMode) {
            case Destiny::Ball::Mode::FOLLOW:
            case Destiny::Ball::Mode::ORBIT: {
                _log(DESTINY__DEBUG, "%u: Our target entity has gone away. Stopping.", mySE->GetID());
                Stop();
            } break;
        }
    }
}

bool DestinyManager::IsTargetInvalid()
{
    /** @todo  this needs a good lookover */
    if (mySE->SystemMgr()->GetSE(m_targetEntity.first) == nullptr) {
        // Our target was removed
        Stop();
        return true;
    }
    if (!m_targetEntity.second->IsDynamicEntity())
        return false;
    if (m_targetEntity.second->HasPilot()) {
        if (m_targetEntity.second->GetPilot()->IsDocked()) {  // Our target docked, so STOP
            //mySE->TargetMgr()->ClearTarget(m_targetEntity.second);
            Stop();
            return true;
        }
    }
    if (m_targetEntity.second->DestinyMgr()->IsWarping()) { // The target is warping
        //mySE->TargetMgr()->ClearTarget(m_targetEntity.second);
        Stop();
        return true;
    }
    return false;
}

// Basic Movement Calls:
void DestinyManager::BeginMovement() {
    // common movement for all types
    if (!m_hasSentShipUpdates) {
        // error fix for setting ship movement variables before ship is in bubble (cannot BubbleCast)
        std::vector<PyTuple*> updates;
        SetBallAgility sbagility;
            sbagility.entityID =  mySE->GetID();
            sbagility.agility = m_shipInertia;
        updates.push_back(sbagility.Encode());
        SetBallMassive sbmassive;
            sbmassive.entityID = mySE->GetID();
            sbmassive.is_massive = false;       // disable client-side bump checks
        updates.push_back(sbmassive.Encode());
        SetBallMass sbmass;
            sbmass.entityID = mySE->GetID();
            sbmass.mass = m_mass;
        updates.push_back(sbmass.Encode());
        SendDestinyUpdate(updates); //consumed
        m_hasSentShipUpdates = true;
    }

    // reset turn and movement checks for possible velocity change.
    m_turnTic = 0;
    m_stop = m_accel = m_decel = m_turning = false;

    if (!mySE->IsNPCSE() or (mySE->IsNPCSE() and mySE->GetNPCSE()->GetAIMgr()->IsIdle()))
        m_stateStamp = sEntityList.GetStamp();

    // do any of these next 3 need to be on every movement check??
    if (m_position.isNaN()) {
        _log(DESTINY__ERROR, "%s position is NaN.", mySE->GetName());
    }
    if (m_position.isZero()) {
        _log(DESTINY__ERROR, "%s position is zero.", mySE->GetName());
    }
    if (m_position.isInf()) {
        _log(DESTINY__ERROR, "%s position is inf.", mySE->GetName());
    }

    if (m_targetPoint.isNotZero()) {
        GVector targHeading(m_position, m_targetPoint);
        targHeading.normalize();
        m_targetHeading = targHeading;
        if (m_shipHeading.isZero())
            m_shipHeading = targHeading;
    }

    if (m_shipHeading.isZero() and m_targetHeading.isZero()) {
        GVector point(m_position);
        point.normalize();
        m_targetPoint =  (point * 1.0e16);
        GVector targHeading(m_position, m_targetPoint);
        targHeading.normalize();
        m_targetHeading = m_shipHeading = targHeading;
    }

    if (m_orbiting == Destiny::Ball::Orbit::None) {
        // reset target distance just in case it changed.
        GVector shipVector(m_position, m_targetPoint);
        m_targetDistance = static_cast<double>(shipVector.length());
        m_orbitRadTic = 0.0f;
        m_maxOrbitSpeedFraction = 1;
    }

    // this will have to be adjusted for cloak mod.
    if (IsCloaked())
        UnCloak();

    // if ship is not moving, set initial movement variables
    if ((m_userSpeedFraction < 0.02f) and (m_timeFraction < 0.02f)) {
        SetSpeedFraction(1.0f, true);
    } else {
        SetSpeedFraction(m_userSpeedFraction, true);
    }

    SetPosition(m_position, sConfig.debug.PositionHack);   // (PositionHack == true) here will force position update to client
}

void DestinyManager::Follow(SystemEntity* pSE, uint32 distance) {
    //called from client as 'CmdFollowBall'
    //  also used by 'Approach'
    if ((m_ballMode == Destiny::Ball::Mode::FOLLOW)
    and (m_targetEntity.second == pSE)
    and (m_followDistance == distance)
    and (m_userSpeedFraction))
        return;

    //reset orbit vars in case we were orbiting before
    if (m_orbiting)
        ClearOrbit();

    m_ballMode = Destiny::Ball::Mode::FOLLOW;
    m_targetPoint = pSE->GetPosition();

    if (pSE->IsStationSE()) {
        // this makes ship approach station dock elevation (y), instead of approaching to stations "center point" position (where icon is)
        m_targetPoint.y = stDataMgr.GetDockPosY(pSE->GetID());
    }

    m_targetEntity.first = pSE->GetID();
    m_targetEntity.second = pSE;
    m_followDistance = distance;
    m_ticAlign = true;
    BeginMovement();

    CmdFollowBall du;
        du.entityID = mySE->GetID();
        du.targetID = pSE->GetID();
        du.range = (int32)distance;
    PyTuple *up = du.Encode();
        //mvPacket = du.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

void DestinyManager::AlignTo(SystemEntity* ent) {
    Follow(ent, 0);
}

void DestinyManager::GotoDirection(const GPoint& direction) {
    //reset orbit vars in case we were orbiting before
    if (m_orbiting)
        ClearOrbit();

    m_ballMode = Destiny::Ball::Mode::GOTO;
    m_targetPoint = direction *1.0e16;
    BeginMovement();

    CmdGotoDirection du;
        du.entityID = mySE->GetID();
        du.x = direction.x;
        du.y = direction.y;
        du.z = direction.z;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

void DestinyManager::GotoPoint(const GPoint& point) {
    //reset orbit vars in case we were orbiting before
    if (m_orbiting)
        ClearOrbit();

    m_ballMode = Destiny::Ball::Mode::GOTO;
    m_targetPoint = point;
    BeginMovement();

    CmdGotoPoint gtpoint;
        gtpoint.entityID = mySE->GetID();
        gtpoint.x = m_targetPoint.x;
        gtpoint.y = m_targetPoint.y;
        gtpoint.z = m_targetPoint.z;
    PyTuple* up = gtpoint.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

void DestinyManager::WarpTo(const GPoint& where, int32 distance/*0*/, bool autoPilot/*false*/, SystemEntity* pSE/*nullptr*/) {
    /* warp order..
     * pick destination -> align/accel -> aura "warp drive active" -> cap drain -> accel
     *      -> enter warp -> warp -> decel -> leave warp -> coast -> stop
     */
    SafeDelete(m_warpState);

    // check for autopilot.  it has 'special' checks in client for auto-disable by destiny update
    if (autoPilot) {
        Follow(pSE, distance);
    } else {
        GotoPoint(where);
    }

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;

    m_stopDistance = distance;
    // get warp target point
    GVector warp_distance(m_position, where);
    m_targetDistance = warp_distance.length();
    m_targetDistance -= static_cast<double>(m_stopDistance);
    // change to heading
    warp_distance.normalize();
    // adjust for stop distance from our travel direction
    warp_distance *= m_stopDistance;
    // adjust target point by calculated stopping point
    m_targetPoint -= warp_distance;

    m_targBubble = sBubbleMgr.GetBubble(mySE->SystemMgr(), m_targetPoint);
    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__TRACE, "Destiny::WarpTo() - %s(%u) target bubble: %u  m_stopDistance: %i  m_targetDistance: %.2f",
            mySE->GetName(), mySE->GetID(), m_targBubble->GetID(), m_stopDistance, m_targetDistance);

    // npcs have no warp restrictions (yet)
    if (mySE->IsNPCSE() or mySE->IsDroneSE()) {
        // do drones warp??
        m_ballMode = Destiny::Ball::Mode::WARP;

        std::vector<PyTuple*> updates;
        CmdWarpTo wt;
            wt.entityID = mySE->GetID();
            wt.dest_x = m_targetPoint.x;
            wt.dest_y = m_targetPoint.y;
            wt.dest_z = m_targetPoint.z;
            wt.distance = m_stopDistance;
            wt.warpSpeed = GetWarpSpeed();
        updates.push_back(wt.Encode());
        OnSpecialFX10 sfx;
            sfx.guid = "effects.Warping";
            sfx.entityID = mySE->GetID();
            sfx.isOffensive = false;
            sfx.start = true;
            sfx.active = true;
        updates.push_back(sfx.Encode());
        SendDestinyUpdate(updates);
        if (is_log_enabled(NPC__MESSAGE))
            _log(NPC__MESSAGE, "Destiny::WarpTo() NPC %s(%u) to:%u from:%u, m_targetPoint: %.2f,%.2f,%.2f  m_stopDistance: %i  m_targetDistance: %.2f",\
                    mySE->GetName(), mySE->GetID(), m_targBubble->GetID(), mySE->SysBubble()->GetID(), \
                    m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_stopDistance, m_targetDistance);
        return;
    }

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

    if (mySE->HasPilot()) {
        if (m_targetDistance < static_cast<double>(minWarpDistance)) {
            mySE->GetPilot()->SendErrorMsg("That is too close for your Warp Drive.");
            // warp distance too close.  cancel warp and return
            // we may need to send pos update
            if (sConfig.debug.PositionHack)
                SetPosition(mySE->GetPosition(), true);
            m_ballMode = Destiny::Ball::Mode::STOP;
            SafeDelete(m_warpState);
            return;
        }

        Client *pClient = mySE->GetPilot();

        /*  capacitor for warp formulas from https://oldforums.eveonline.com/?a=topic&threadID=332116
         *  Energy to warp = warpCapacitorNeed * mass * au * (1 - warp_drive_operation_skill_level * 0.10)
         */
        float currentShipCap = pClient->GetShip()->GetAttribute(AttrCapacitorCharge).get_float();
        float capNeeded = m_mass * m_warpCapacitorNeed * (static_cast<double>(m_targetDistance) / static_cast<double>(ONE_AU_IN_METERS));
        capNeeded *= (1.0f - (0.1f *pClient->GetChar()->GetSkillLevel(EvESkill::WarpDriveOperation)));

        _log(DESTINY__WARNING, "Warp Cap need for %s(%u) is %.4f", mySE->GetName(), mySE->GetID(), capNeeded);

        //  check if ship has enough capacitor to warp full distance
        if (capNeeded > currentShipCap) {
            // not enough cap.  reset everything based on available cap
            capNeeded = (currentShipCap /m_warpCapacitorNeed) /m_mass;
            if (capNeeded > 1) {
                m_targetDistance = static_cast<double>(capNeeded) * static_cast<double>(ONE_AU_IN_METERS);
                GVector warp_direction(m_position, where);
                GPoint newTarget(m_position + (warp_direction * m_targetDistance));

                m_targBubble = sBubbleMgr.GetBubble(mySE->SystemMgr(), newTarget);
                if (is_log_enabled(DESTINY__WARP_TRACE))
                    _log(DESTINY__TRACE, "Destiny::WarpTo():Update - %s(%u) target bubble: %u  m_stopDistance: %i  m_targetDistance: %.2f",
                        mySE->GetName(), mySE->GetID(), m_targBubble->GetID(), m_stopDistance, m_targetDistance);
            } else {
                // if not enough cap to do min warp, cancel and return
                pClient->SendErrorMsg("You don't have enough capacitor charge to warp.");
                _log(DESTINY__WARNING, "Destiny::WarpTo() - %s(%u): Capacitor needed vs current  %.3f / %.3f",
                        mySE->GetName(), mySE->GetID(), capNeeded, currentShipCap);

                m_ballMode = Destiny::Ball::Mode::STOP;
                m_targBubble = nullptr;
                SafeDelete(m_warpState);
                return;
            }
        } else {
            capNeeded = currentShipCap - capNeeded;
        }

        m_capNeeded = capNeeded;
    }

    /*  TODO PUT CHECK HERE FOR WARP BUBBLES
     *     and other things that affect warp-in point.....when we get to there.
     * AttrWarpBubbleImmune = 1538,
     * AttrWarpBubbleImmuneModifier = 1539,
     *   NOTE:  warp bubble in path (or within 100km of m_targetPoint) will change m_targetDistance and m_targetPoint
     *   however, this does NOT affect original calculations for energy needed, etc...
     */
    if (m_targBubble->HasWarpBubble()) {
        if (!mySE->GetSelf()->HasAttribute(AttrWarpBubbleImmune))
            ;   // not immune to bubble
    }

    m_ballMode = Destiny::Ball::Mode::WARP;

    // send client updates
    std::vector<PyTuple*> updates;

    // acknowledge client's warpto request
    CmdWarpTo wt;
    wt.entityID = mySE->GetID();
    wt.dest_x = m_targetPoint.x;
    wt.dest_y = m_targetPoint.y;
    wt.dest_z = m_targetPoint.z;
    wt.distance = m_stopDistance;
    wt.warpSpeed = GetWarpSpeed(); // warp speed x10

    updates.push_back(wt.Encode());

    // send a warp effect...
    OnSpecialFX10 sfx;
    sfx.guid = "effects.Warping";
    sfx.entityID = mySE->GetID();
    sfx.isOffensive = false;
    sfx.start = true;
    sfx.active = true;

    updates.push_back(sfx.Encode());
    SendDestinyUpdate(updates);
    updates.clear();

    //set massive for warp, per client, but self-only
    SetBallMassive bm;
    bm.entityID = mySE->GetID();
    bm.is_massive = false;       // disable client-side bump checks
    PyTuple *up = bm.Encode();
    SendSingleDestinyUpdate(&up, true);   // consumed

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(
            DESTINY__WARP_TRACE,
            "Destiny::WarpTo() toBubble:%u from:%u, m_targetPoint: %.2f,%.2f,%.2f  m_stopDistance: %i  m_targetDistance: %.2f",
            m_targBubble->GetID(),
            mySE->SysBubble()->GetID(),
            m_targetPoint.x,
            m_targetPoint.y,
            m_targetPoint.z,
            m_stopDistance,
            m_targetDistance
        );
    }
}

void DestinyManager::Orbit(SystemEntity *pSE, uint32 distance/*0*/) {
    if ((m_ballMode == Destiny::Ball::Mode::ORBIT)
    and (m_targetEntity.second == pSE)
    and (m_targetDistance == static_cast<double>(distance)))
        return;

    if (m_orbiting)
        m_shipHeading = NULL_ORIGIN_V;

    /* this initial Orbit() call will, based on position data, determine the orbit plane, rotation (cw/ccw)
     *  initial heading, actual orbit radius, actual orbit velocity, and some other shit i havent thought about yet.
     *
     * m_targetPoint    - updated in Orbit()
     * m_shipHeading    - updated in Orbit()
     * m_targetEntity   - SE object to orbit
     * m_targetDistance - commanded orbit distance
     * m_followDistance - calculated orbit distance based on mass, velocity, gravity, and other ship variables
     * m_stateStamp (via BeginMovement())
     * speed fractions (usf, tf, asf - via SetSpeedFraction() to begin or alter speed)
     * m_maxOrbitSpeedFraction - calculated max speed to maintain commanded orbit distance.  set in Orbit()
     */
    m_ballMode = Destiny::Ball::Mode::ORBIT;
    m_orbiting = Destiny::Ball::Orbit::Orbiting;
    m_targetEntity.first = pSE->GetID();
    m_targetEntity.second = pSE;
    m_targetPoint = pSE->GetPosition();
    m_targetDistance = static_cast<double>(distance);
    BeginMovement();

    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "%s(%u) - Ship Data - agility:%.3f, inertia:%.3f, massMkg:%.3f, maxSpeed:%.2f, radius:%.2f", \
            mySE->GetName(), mySE->GetID(), m_shipAgility, m_shipInertia, m_massMKg, m_maxShipSpeed, m_radius);
    //EvE::traceStack();

    // Target (orbited object)
    double Tr = pSE->GetRadius();
    double Tm = pSE->GetSelf()->GetAttribute(AttrMass).get_float();
    if (Tm != 0.0)
        Tm = pSE->GetSelf()->type().mass();

    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "%s(%u) - Target Data - mass:%.3f, speed:%.2f, radius:%.2f", \
            mySE->GetName(), mySE->GetID(), Tm, (pSE->DestinyMgr() ? pSE->DestinyMgr()->GetSpeed() : 0 ), Tr);

    // fudge distance to work 'close enough' with all targets...this was trial-n-error
    double Rc = ((distance + 150 + m_radius - (pSE->GetRadius() /12)) * 1.2);
    double Rc2 =  std::pow(Rc,2);
    double Vm2 =  std::pow(m_maxShipSpeed,2);
    double t2 =  std::pow(m_shipAgility,2);

    // the following equation is from "Ship Motion in Eve Online" by Scheulagh Santorine, Ph.D
    // radius needs target mass and grav const factored in....somehow.
    // orbit radius
    /* r = sqrt(6 * cbrt(108t^2*Vm^2 * Rc^2 + 8Rc^6 + 12sqrt(81t^4 *Vm^4 + 12t^2 * Vm^2 * Rc^10))
     * + (24Rc^4 / (108t^2 * Vm^2 * Rc^2 + 8Rc^2 + 12sqrt(81t^4 * Vm^4 * Rc^8 + 12t^2 * Vm^2 * Rc^10)^1/3)) + 12Rc^2) /6
     */
    double one = (108 * t2 * Vm2 * Rc2);
    double two = (12 * t2 * Vm2 *  std::pow(Rc,10));
    double three = (12 * std::sqrt(81 *  std::pow(m_shipAgility,4) *  std::pow(m_maxShipSpeed,4) + two));
    double four = (6 *  std::cbrt(one + 8 *  std::pow(Rc,6) + three));
    double five =  std::cbrt( std::sqrt(three *  std::pow(Rc,8) + two));
    double six = (one + (8 * Rc2) + (12 * five));
    m_followDistance =  std::sqrt(four + (24 *  std::pow(Rc, 4) / six) + 12 * Rc2) / 6;

    double velocity = m_maxShipSpeed * ((distance / m_followDistance) + 0.065); // dunno where i got this from but seems to work very well.
    m_maxOrbitSpeedFraction = velocity / m_maxShipSpeed;

    double circ = EvE::Trig::Pi2 * m_followDistance;
    m_orbitTime = circ / velocity;
    m_orbitRadTic = EvE::Trig::Pi2 / m_orbitTime;

    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "%s(%u) - Orbit Data - Rc:%.3f, velocity:%.2f, osf:%.2f, targetDistance:%.2f, followDistance:%u, orbitTime:%.1f, radTic:%.5f", \
                mySE->GetName(), mySE->GetID(), Rc, velocity, m_maxOrbitSpeedFraction, \
                m_targetDistance, m_followDistance, m_orbitTime, m_orbitRadTic);
/*  dont really need this here yet.....maybe not at all.
    double current = m_position.distance(pSE->GetPosition());
    double actual = (current - m_radius - Tr);
    // m_orbiting: -2=way too close  -1=too close, 0=no orbit, 1=at distance 2=too far, 3=way too far
    if ((actual - m_followDistance) > m_followDistance) {
        // too far to engage target.
        m_orbiting = 3;
    } else if (current > m_followDistance) {
        // too far outside orbit.  move closer
        m_orbiting = 2;
    } else if (actual < m_followDistance) {
        // way too close inside orbit.  move away quickly.
        m_orbiting = -2;
    } else if (current < m_followDistance) {
        // too close inside orbit; move away slowly.
        m_orbiting = -1;
    } else {
        // within orbit distance tolerance
        m_orbiting = 1;
    }

    if (m_orbiting > 1) {
        // outside target distance.  set orbit parameters based on current position.

    }
*/
    if (m_followDistance == 0) {
        _log(DESTINY__ERROR, "%s(%u) - FollowDistance is 0.", mySE->GetName(), mySE->GetID());
        m_followDistance = (uint32)(m_targetDistance + Tr + m_radius); // fudge something here.  will have to fix later, but this is close enough
    }

    CmdOrbit du;
        du.entityID = mySE->GetID();
        du.orbitEntityID = pSE->GetID();
        du.distance = (int32)m_targetDistance;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

bool DestinyManager::IsAligned(GPoint& targetPoint)
{
    if (m_shipHeading.isZero()) {
        GVector moveVector(m_position, targetPoint);
        moveVector.normalize();
        m_shipHeading = moveVector;
    }
    GVector toVec(m_position, targetPoint);
    toVec.normalize();
    float dot = toVec.dotProduct(m_shipHeading);
    float degrees = EvE::Trig::Rad2Deg(std::acos(dot));
    if (degrees < TURN_ALIGNMENT)
        return true;
    return false;
}

void DestinyManager::Undock(GPoint dir) {
    //set movement direction
    m_targetPoint = dir *1.0e16;
    m_shipHeading = GVector(dir);
    SetUndockSpeed();
    if (mySE->IsShipSE())
        mySE->GetShipSE()->GetShipItemRef()->SetUndocking(false);
}

void DestinyManager::SetUndockSpeed() {
    //start ship movement @ max velocity for undocking.
    // this simulates being forcefully "ejected" from station (and is currently ~500m off)
    m_stop = false;
    m_orbiting = 0;
    m_stateStamp = sEntityList.GetStamp();
    m_changeDelay = true;   // skip a single tic before making change
    m_shipAccelTime = 0.5f;
    m_prevSpeedFraction = 0.0f;
    m_userSpeedFraction = 1.1f;
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = m_shipHeading * m_maxSpeed;
    // may need to tweak these for larger ships...
    m_activeSpeedFraction = 1.1f;
    m_timeFraction = 1.1f;

    if (m_ballMode == Destiny::Ball::Mode::MISSILE)
        return;

    m_ballMode = Destiny::Ball::Mode::GOTO;
    std::vector<PyTuple*> updates;
    SetBallVelocity bv;
        bv.entityID = mySE->GetID();
        bv.x = m_velocity.x;
        bv.y = m_velocity.y;
        bv.z = m_velocity.z;
    updates.push_back(bv.Encode());
    CmdGotoDirection du;
        du.entityID = mySE->GetID();
        du.x = m_shipHeading.x;
        du.y = m_shipHeading.y;
        du.z = m_shipHeading.z;
    updates.push_back(du.Encode());
    SendDestinyUpdate(updates);
}

PyResult DestinyManager::AttemptDockOperation() {
    Client *pClient = mySE->GetPilot();
    uint32 stationID = pClient->GetDockStationID();
    SystemEntity *station = mySE->SystemMgr()->GetSE(stationID);

    if (station == nullptr) {
        codelog(CLIENT__ERROR, "%s: Station %u not found.", pClient->GetName(), stationID);
        pClient->SendErrorMsg("Station Not Found, Docking Aborted.");
        return PyStatic.NewNone();
    }

    //get the station Docking Perimiter
    const GPoint stationPos = station->GetPosition();
    double rangeToStationPerimiter = m_position.distance(stationPos);
    rangeToStationPerimiter -= mySE->GetRadius();
    rangeToStationPerimiter -= station->GetRadius();

    // Verify range to station is within docking perimeter of 2500 meters:
    _log(DESTINY__TRACE, "Destiny::AttemptDockOperation() rangeToStationPerimiter is %.2fm", rangeToStationPerimiter);
    if (rangeToStationPerimiter > 2500.0) {
        AlignTo( station );   // Turn ship and move toward docking point - client will usually call Dock() automatically...sometimes
        if (mySE->HasPilot() and mySE->GetPilot()->CanThrow())
            throw UserError ("DockingApproach");
    }

    pClient->SetStateTimer(Player::State::Dock, sConfig.world.StationDockDelay *1000); // default @ 4sec();
    pClient->SetAutoPilot(false);

    return new PyLong(GetFileTimeNow());
}

void DestinyManager::DockingAccepted()
{
    Stop();
    UnCloak();
    Client *pClient = mySE->GetPilot();
    if (pClient == nullptr)
        return;

    SystemEntity *pSE = mySE->SystemMgr()->GetSE(pClient->GetDockStationID());
    if (pSE == nullptr)
        return;

    const GPoint stationPos = pSE->GetPosition();
    OnDockingAccepted oda;
        oda.ship_x = m_position.x;
        oda.ship_y = m_position.y;
        oda.ship_z = m_position.z;
        oda.station_x = stationPos.x;
        oda.station_y = stationPos.y;
        oda.station_z = stationPos.z;
        oda.stationID = pClient->GetDockStationID();
    PyTuple* ev = oda.Encode();
    pClient->SendNotification("OnDockingAccepted", "charid", &ev);
}

void DestinyManager::SetPosition(const GPoint &pt, bool update /*false*/) {
    _log(DESTINY__TRACE, "Destiny::SetPosition() called by %s(%u)", mySE->GetName(), mySE->GetID());

    if (pt.isZero()) {
        _log(DESTINY__TRACE, "Destiny::SetPosition() - %s(%u) point is zero", mySE->GetName(), mySE->GetID());
        EvE::traceStack();
        // this *should* be systemID...
        m_position = sMapData.GetRandPointOnPlanet(mySE->GetLocationID());
    } else {
        m_position = pt;
    }

    // this sets InventoryItemRef.m_position correctly, which is used for all position references
    mySE->SetPosition(m_position);

    //according to packet sniffs, this is only used for 'Structure' and 'Probe" items.  'update' is for syncing client position data with ours
    if (mySE->IsPOSSE() or mySE->IsProbeSE() or update) {
        SetBallPosition du;
            du.entityID = mySE->GetID();
            du.x = m_position.x;
            du.y = m_position.y;
            du.z = m_position.z;
        PyTuple* up = du.Encode();
        SendSingleDestinyUpdate(&up);   // consumed
    }
}

void DestinyManager::SetRadius(double radius, bool update /*false*/) {
    _log(DESTINY__TRACE, "Destiny::SetPosition() called by %s(%u)", mySE->GetName(), mySE->GetID());

    m_radius = radius;

    mySE->SetRadius(m_radius);

    if (update) {
        SetBallRadius du;
        du.entityID = mySE->GetID();
        du.radius = m_radius;
        PyTuple* up = du.Encode();
        SendSingleDestinyUpdate(&up);   // consumed
    }
}

// settings for ship, npc and missile max speeds
void DestinyManager::SetMaxVelocity(float maxVelocity)
{
    float maxSpeed = mySE->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();
    /*
    if (mySE->IsMissileSE() or mySE->IsNPCSE())
        maxSpeed = mySE->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();
    else if (mySE->IsShipSE() or mySE->IsDroneSE())
        maxSpeed = mySE->GetSelf()->GetAttribute(AttrMaxDirectionalVelocity).get_float();   // this is depreciated.  used as an absolute max speed, accounting for ab/mwd
    else
        ; // make error here?
        */
    if (mySE->IsShipSE())
        if (is_log_enabled(DESTINY__TRACE))
            _log(DESTINY__TRACE, "Destiny::SetMaxVelocity() - Ship:%s(%u) Pilot:%s(%u) - AttrMaxDirectionalVelocity is %.1f, maxSpeed is %.1f, update is %.1f", \
                    mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID(), \
                    mySE->GetSelf()->GetAttribute(AttrMaxDirectionalVelocity).get_float(), maxSpeed, maxVelocity);

    if (maxVelocity > maxSpeed) {
        m_maxShipSpeed = maxSpeed;
    } else {
        m_maxShipSpeed = maxVelocity;
    }
}

void DestinyManager::SpeedBoost(bool deactivate/*false*/)
{
    // after UpdateVelocity() rewrite, only thing to do here is reset ship's speed data.
    //  UpdateVelocity will handle the rest.

    m_prevSpeed = m_maxSpeed * m_activeSpeedFraction;  //get current ship speed

    // prop mod state changed.  reset ship movement variables and update current movement, if applicable
    m_mass = mySE->GetSelf()->GetAttribute(AttrMass).get_float();
    m_massMKg = m_mass / 1000000; //changes mass from Kg to MillionKg (10^-6)
    m_shipAgility = m_massMKg * m_shipInertia;
    m_alignTime = (-log(0.25) * m_shipAgility);
    m_shipMaxAccelTime = (-log(0.0001) * m_shipAgility);
    m_degPerTic = (60.0f - m_shipAgility) / 10;  // this isnt right....
    m_maxShipSpeed = mySE->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();
    // reset ship max speed using updated m_maxShipSpeed
    m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;
    // set asf as fraction of current speed over new max speed.
    m_activeSpeedFraction = m_prevSpeed / m_maxShipSpeed;     // this may give >1.0

    // send out updated ship data
    std::vector<PyTuple*> updates;
    SetBallAgility sbagility;
        sbagility.entityID =  mySE->GetID();
        sbagility.agility = m_shipInertia;
        updates.push_back(sbagility.Encode());
    SetBallMass sbmass;
        sbmass.entityID = mySE->GetID();
        sbmass.mass = m_mass;
        updates.push_back(sbmass.Encode());
    SetBallSpeed sbms;
        sbms.entityID = mySE->GetID();
        sbms.speed = m_maxShipSpeed;
        updates.push_back(sbms.Encode());
    SendDestinyUpdate(updates);
    m_hasSentShipUpdates = true;    // just in case, as this is re-sent in BeginMovement()

    // this is just for debug logging
    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        // ship is currently ...
        if (deactivate) {
            // ... deactivating prop mod
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::Deactivate");
        } else if (m_activeSpeedFraction < m_userSpeedFraction) {
            // ....moving and decelerating
            // - this hits when prop mod activated while ship is decel
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(psf!=0&tf>usf) - decelerating.");
        } else if (m_activeSpeedFraction) {
            // ....moving and not decelerating (this includes turning)
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(0<asf<=usf)");
        } else {
            // ....sitting still - do nothing
            if (m_userSpeedFraction) {
                _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(usf>asf=0) -  sitting still.");
            } else {
                _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(usf=asf=0) -  sitting still.");
            }
        }

        _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost() - nMass: %.5f, nAg: %.5f, tf: %.2f, usf: %.2f, asf: %.3f", \
                m_mass, m_shipAgility, m_timeFraction, m_userSpeedFraction, m_activeSpeedFraction);
        _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost() - pSpeed:%.2f, maxSpeed:%.2f, maxShipSpeed:%.2f", \
                m_prevSpeed, m_maxSpeed, m_maxShipSpeed);
    }

    // update ship speed variables based on new data
    SetSpeedFraction(m_userSpeedFraction, true);
}

void DestinyManager::WebbedMe(InventoryItemRef modRef, bool apply/*false*/)
{
    if (apply) {
        m_maxShipSpeed *= (1 + (modRef->GetAttribute(AttrSpeedFactor).get_float() / 100.0f));
    } else {
        m_maxShipSpeed /= (1 + (modRef->GetAttribute(AttrSpeedFactor).get_float() / 100.0f));
    }
    m_activeSpeedFraction = m_activeSpeedFraction * 0.999f;
    std::vector<PyTuple*> updates;
    SetBallSpeed sbms;
        sbms.entityID = mySE->GetID();
        sbms.speed = m_maxShipSpeed;
        updates.push_back(sbms.Encode());
    SendDestinyUpdate(updates);
    m_hasSentShipUpdates = true;    // just in case, as this is re-sent in BeginMovement()

    SetSpeedFraction(m_userSpeedFraction, true);
}

//  called from Client::CreateShipSE(), Client::ResetAfterPodded(), NPC::NPC(), Concord::Concord(), Drone::Drone(), DestinyManager::UpdateNewShip()
void DestinyManager::UpdateShipVariables()
{
    /*
Frigates (incl. CovOps, Inty, AF) have an agility of 3.1
Destroyers 3.5
Industrials 1.0
Cruisers 0.55 (Elite/Faction 0.65)
Battlecruisers 1.1
Battleships 0.155
*/
    /* this sets variables needed for correct movement math.
     *  these attribs are set from ship item when shipSE created.  DO NOT modify anything here
     * this is also called when fleet boosts are updated.
     */
    /** @todo check for movement when fleet boosts are applied and this is called */
    InventoryItemRef sRef = mySE->GetSelf();
    m_mass = sRef->GetAttribute(AttrMass).get_float();
    m_massMKg = m_mass / 1000000; //changes mass from Kg to milliKg (10^-6)

    // this will catch speeds/needs for all ships (player and npc), and is easier to do here.
    if (sRef->HasAttribute(AttrWarpSpeedMultiplier))
        m_shipWarpSpeed = sRef->GetAttribute(AttrWarpSpeedMultiplier).get_float();
    if (sRef->HasAttribute(AttrInetia))
        m_shipInertia = sRef->GetAttribute(AttrInetia).get_float();
    if (sRef->HasAttribute(AttrMaxVelocity))
        m_maxShipSpeed = sRef->GetAttribute(AttrMaxVelocity).get_float();
    if (sRef->HasAttribute(AttrWarpCapacitorNeed))
        m_warpCapacitorNeed = sRef->GetAttribute(AttrWarpCapacitorNeed).get_float() *2;

    if (mySE->IsNPCSE() or mySE->IsDroneSE())
        m_maxShipSpeed = sRef->GetAttribute(AttrEntityCruiseSpeed).get_float();

    /*  per https://forums.eveonline.com/default.aspx?g=posts&m=3912843   post#103
     *
     * Ships will exit warp mode when their warping speed drops below
     * 75% of sub-warp max speed, or 100m/s, whichever is the lower.
     */
    m_speedToLeaveWarp = m_maxShipSpeed * 0.75f;
    if ((m_speedToLeaveWarp < 100) and (m_maxShipSpeed > 135))      // 75% of 135 is 101.25
        m_speedToLeaveWarp = 100;

    /* The product of Mass and the Inertia Modifier gives the ship's agility
     * Agility = Mass x Inertia Modifier
     *  agility is an internal-use variable.
     */
    m_shipAgility = m_massMKg * m_shipInertia;
    m_degPerTic = (60.0f - m_shipAgility) / 10;
    // set a maximum acceleration time (based on ship variables)
    m_shipMaxAccelTime = (-log(0.0001) * m_shipAgility);

    //  both of these formulas have identical products
    //TimeToWarp = -ln(0.25) x Mass Mkg x Inertia Mod
    //float alignTime = ((log(2) * m_shipInertia * m_mass) / 500000);
    m_alignTime = (-log(0.25) * m_shipAgility);
    m_timeToEnterWarp = m_alignTime;

    m_hasSentShipUpdates = true;

    if (!mySE->HasPilot())
        return;
    if (mySE->GetPilot()->IsInSpace() and (mySE->SysBubble() != nullptr)) {
        std::vector<PyTuple*> updates;
        SetBallAgility sbagility;
            sbagility.entityID =  mySE->GetID();
            sbagility.agility = m_shipInertia;
        updates.push_back(sbagility.Encode());
        SetBallMassive sbmassive;
            sbmassive.entityID = mySE->GetID();
            sbmassive.is_massive = false;       // disable client-side bump checks
        updates.push_back(sbmassive.Encode());
        SetBallMass sbmass;
            sbmass.entityID = mySE->GetID();
            sbmass.mass = m_mass;
        updates.push_back(sbmass.Encode());
        SetBallSpeed sbspeed;
            sbspeed.entityID = mySE->GetID();
            sbspeed.speed = m_maxShipSpeed;
        updates.push_back(sbspeed.Encode());
        SendDestinyUpdate(updates); //consumed
    }
}

void DestinyManager::MakeMissile(Missile* pMissile) {
    SetMaxVelocity(pMissile->GetSpeed());
    SetPosition(pMissile->GetSelf()->position());
    m_mass = pMissile->GetSelf()->type().mass();
    m_massMKg = m_mass / 1000000; //changes mass from Kg to MillionKg (10^-6)
    m_shipInertia = pMissile->GetSelf()->GetAttribute(AttrInetia).get_float();
    m_shipAgility = m_massMKg * m_shipInertia;

    m_stop = false;
    m_ballMode = Destiny::Ball::Mode::MISSILE;
    m_stateStamp = sEntityList.GetStamp();

    SystemEntity* pTarget = pMissile->GetTargetSE();
    m_targetPoint = GPoint(pTarget->GetPosition());
    m_targetEntity.first = pTarget->GetID();
    m_targetEntity.second = pTarget;
    m_targetDistance = static_cast<double>(m_position.distance(m_targetPoint));

    GVector moveVector(m_position, m_targetPoint);
    moveVector.normalize();     //change vector to direction
    m_shipHeading = moveVector;

    SetUndockSpeed();   /* sets all needed variables for max velocity */
    mySE->SystemMgr()->AddEntity(pMissile, false); // we are not adding missiles to anomaly map

    std::vector<PyTuple*> updates;
    SetBallSpeed maxspeed;
        maxspeed.entityID = pMissile->GetID();
        maxspeed.speed = m_maxShipSpeed;
    updates.push_back(maxspeed.Encode());
    Rsp_LaunchMissile miss;
        miss.shipID = pMissile->GetLauncherID();
        miss.targetID = pTarget->GetID();
        miss.missileID = pMissile->GetID();
        miss.unk1 = 1;
        miss.unk2 = 1;
    updates.push_back(miss.Encode());
    SendDestinyUpdate(updates); //consumed
}

void DestinyManager::UpdateNewShip(const ShipItemRef newShipRef) {
    if (m_hasSentShipUpdates)
        return;

    Client* pClient = mySE->GetPilot();
    if (pClient == nullptr)
        return;
    // exactly why do we need this here??
    PyDict* slim = new PyDict();
        slim->SetItemString("name",                     new PyString(newShipRef->itemName()));
        slim->SetItemString("itemID",                   new PyInt(newShipRef->itemID()));
        slim->SetItemString("typeID",                   new PyInt(newShipRef->typeID()));
        slim->SetItemString("ownerID",                  new PyInt(mySE->GetOwnerID()));
        slim->SetItemString("charID",                   new PyInt(pClient->GetCharacterID()));
        slim->SetItemString("corpID",                   IsCorp(mySE->GetCorporationID()) ? new PyInt(mySE->GetCorporationID()) : PyStatic.NewNone());
        slim->SetItemString("allianceID",               IsAlliance(mySE->GetAllianceID()) ? new PyInt(mySE->GetAllianceID()) : PyStatic.NewNone());
        slim->SetItemString("warFactionID",             IsFaction(mySE->GetWarFactionID()) ? new PyInt(mySE->GetWarFactionID()) : PyStatic.NewNone());
        slim->SetItemString("bounty",                   new PyFloat(pClient->GetBounty()));
        slim->SetItemString("securityStatus",           new PyFloat(pClient->GetSecurityRating()));
    if (newShipRef->typeID() == itemTypeCapsule) {
        slim->SetItemString("launcherID",               new PyInt(mySE->GetShipSE()->GetLauncherID()));
        slim->SetItemString("modules",                  new PyList());
    } else {
        slim->SetItemString("categoryID",               new PyInt(newShipRef->categoryID()));
        slim->SetItemString("groupID",                  new PyInt(newShipRef->groupID()));
        slim->SetItemString("modules",                  newShipRef->ShipGetModuleList());
    }

    std::vector<PyTuple*> updates;
    PyTuple* shipData = new PyTuple(2);
        shipData->SetItem(0, new PyLong(newShipRef->itemID()));
        shipData->SetItem(1, new PyObject( "foo.SlimItem", slim));
    PyTuple* shipItem = new PyTuple(2);
        shipItem->SetItem(0, new PyString("OnSlimItemChange"));
        shipItem->SetItem(1, shipData);
    updates.push_back(shipItem);
    SendDestinyUpdate(updates);

    UpdateShipVariables();
    SendBallInteractive(newShipRef, true);
}

void DestinyManager::UpdateOldShip(ShipSE* pShipSE)
{
    if (pShipSE->IsDead())
        return;
    PyDict* slimPod = new PyDict();
        slimPod->SetItemString("itemID",                new PyInt(pShipSE->GetID()));
        slimPod->SetItemString("typeID",                new PyInt(pShipSE->GetTypeID()));
        slimPod->SetItemString("categoryID",            new PyInt(pShipSE->GetCategoryID()));
        slimPod->SetItemString("ownerID",               new PyInt(pShipSE->GetOwnerID()));
        slimPod->SetItemString("charID",                PyStatic.NewNone());
        slimPod->SetItemString("corpID",                new PyInt(pShipSE->GetCorporationID()));
        slimPod->SetItemString("allianceID",            new PyInt(pShipSE->GetAllianceID()));
        slimPod->SetItemString("warFactionID",          new PyInt(pShipSE->GetWarFactionID()));
        slimPod->SetItemString("bounty",                PyStatic.NewNone());
        slimPod->SetItemString("securityStatus",        PyStatic.NewNone());
    PyTuple* shipData = new PyTuple(2);
        shipData->SetItem(0, new PyLong(pShipSE->GetID()));
        shipData->SetItem(1, new PyObject( "foo.SlimItem", slimPod));
    PyTuple* shipItem = new PyTuple(2);
        shipItem->SetItem(0, new PyString("OnSlimItemChange"));
        shipItem->SetItem(1, shipData);
    SendSingleDestinyUpdate(&shipItem);   // consumed

    SendBallInteractive(pShipSE->GetShipItemRef(), false);
    m_hasSentShipUpdates = false;
}

void DestinyManager::Jump(bool showCloak)
{
    Halt();
    if (showCloak) {
        m_cloaked = true;
    }
    if (mySE->SysBubble() != nullptr)
        mySE->SysBubble()->RemoveExclusive(mySE);
}

void DestinyManager::Cloak() {
    if (m_cloaked)
        return;
    m_cloaked = true;
    SendCloakFx(true);
    if (mySE->SysBubble() != nullptr)
        mySE->SysBubble()->RemoveExclusive(mySE);
}

void DestinyManager::UnCloak() {
    if (!m_cloaked)
        return;
    m_cloaked = false;
    SendCloakFx();
    if (mySE->SysBubble() != nullptr)
        mySE->SysBubble()->AddBallExclusive(mySE);
}

void DestinyManager::TractorBeamStart(SystemEntity* pShipSE, EvilNumber speed)
{
    /** @todo  need to update this */
    m_ballMode = Destiny::Ball::Mode::FOLLOW;

    m_stop = false;
    m_accel = false;
    m_decel = false;
    m_turning = false;
    m_tractored = true;
    m_moveTime = GetTimeMSeconds();
    m_stateStamp = sEntityList.GetStamp();

    m_targetPoint = pShipSE->GetPosition();
    GVector moveVector(m_position, m_targetPoint);
    m_targetDistance = static_cast<double>(moveVector.length());
    moveVector.normalize();
    m_shipHeading = moveVector;

    m_maxShipSpeed = speed.get_float();   //AttrMaxTractorVelocity
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = m_shipHeading * m_maxSpeed;

    m_followDistance = 500 + pShipSE->GetRadius();
    m_shipAccelTime = 0.1f;

    m_activeSpeedFraction = m_userSpeedFraction = m_timeFraction = 1.0f;

    m_targetEntity.first = pShipSE->GetID();
    m_targetEntity.second = pShipSE;

    std::vector<PyTuple*> updates;
    SetBallSpeed ms;
        ms.entityID = mySE->GetID();
        ms.speed = m_maxShipSpeed;
    updates.push_back(ms.Encode());
    SetBallFree bf;
        bf.entityID = mySE->GetID();
        bf.is_free = 1;
    updates.push_back(bf.Encode());
    SetBallMass sbmass;
        sbmass.entityID = mySE->GetID();
        sbmass.mass = 10000;
    updates.push_back(sbmass.Encode());
    CmdSetSpeedFraction ssf;
        ssf.entityID = mySE->GetID();
        ssf.fraction = 1;
    updates.push_back(ssf.Encode());
    CmdFollowBall fb;
        fb.entityID = mySE->GetID();
        fb.targetID = pShipSE->GetID();
        fb.range = m_followDistance;
    updates.push_back(fb.Encode());
    SendDestinyUpdate(updates);
}

void DestinyManager::TractorBeamStop()
{
    Halt();
    m_tractored = false;
    std::vector<PyTuple*> updates;
    SetBallSpeed ms;
        ms.entityID = mySE->GetID();
        ms.speed = 0;
    updates.push_back(ms.Encode());
    SetBallFree bf;
        bf.entityID = mySE->GetID();
        bf.is_free = 0;
    updates.push_back(bf.Encode());
    SetBallMass sbmass;
        sbmass.entityID = mySE->GetID();
        sbmass.mass = m_mass;
    updates.push_back(sbmass.Encode());
    SendDestinyUpdate(updates);
}

/*
                  [PyTuple 2 items]
                    [PyInt 62696]
                    [PyTuple 2 items]
                      [PyString "OnSpecialFX"]
                      [PyTuple 10 items]
                        [PyIntegerVar 9000000000001190976]
                        [PyNone]
                        [PyNone]
                        [PyNone]
                        [PyNone]
                        [PyList 0 items]
                        [PyString "effects.Jettison"]
                        [PyInt 0]
                        [PyInt 1]
                        [PyInt 0]
                        */
void DestinyManager::SendJettisonPacket() const {
    OnSpecialFX10 effect;
        effect.entityID = mySE->GetID();
        effect.guid = "effects.Jettison";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple* up = effect.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}
/*
 *                  [PyTuple 2 items]
 *                    [PyInt 8087]
 *                    [PyTuple 2 items]
 *                      [PyString "OnSpecialFX"]
 *                      [PyTuple 14 items]
 *                        [PyIntegerVar 1002332856217]
 *                        [PyIntegerVar 1002332856217]
 *                        [PyInt 12235]
 *                        [PyNone]
 *                        [PyNone]
 *                        [PyList 0 items]
 *                        [PyString "effects.AnchorDrop"]
 *                        [PyBool False]
 *                        [PyInt 1]
 *                        [PyInt 1]
 *                        [PyInt -1]
 *                        [PyInt 0]
 *                        [PyIntegerVar 129516974756172792]
 *                        [PyNone]
 */
void DestinyManager::SendAnchorDrop() const {
    OnSpecialFX14 effect;
        effect.entityID = mySE->GetID();
        effect.moduleID = mySE->GetID();
        effect.moduleTypeID = mySE->GetTypeID();
        effect.guid = "effects.AnchorDrop";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 1;
        effect.startTime = GetFileTimeNow();
    PyTuple* up = effect.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

void DestinyManager::SendAnchorLift() const {
    OnSpecialFX14 effect;
        effect.entityID = mySE->GetID();
        effect.moduleID = mySE->GetID();
        effect.moduleTypeID = mySE->GetTypeID();
        effect.guid = "effects.AnchorLift";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.startTime = GetFileTimeNow();
    PyTuple* up = effect.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

/*
                  [PyTuple 2 items]
                    [PyInt 517]
                    [PyTuple 2 items]
                      [PyString "OnSpecialFX"]
                      [PyTuple 10 items]
                        [PyIntegerVar 1002332228246]
                        [PyNone]
                        [PyNone]
                        [PyNone]
                        [PyNone]
                        [PyList 0 items]
                        [PyString "effects.Cloak"]
                        [PyInt 0]
                        [PyInt 1]
                        [PyInt 0]
                  [PyTuple 2 items]
                    [PyInt 517]
                    [PyTuple 2 items]
                      [PyString "OnSpecialFX"]
                      [PyTuple 14 items]
                        [PyIntegerVar 1002332228246]
                        [PyIntegerVar 1002333797260]
                        [PyInt 11578]
                        [PyNone]
                        [PyNone]
                        [PyList 0 items]
                        [PyString "effects.Cloaking"]
                        [PyBool False]
                        [PyInt 1]
                        [PyInt 1]
                        [PyInt -1]
                        [PyInt 0]
                        [PyIntegerVar 129527563080275219]
                        [PyNone]
                [PyBool False]
    */

/** @todo verify 'start' and 'active' here... */
void DestinyManager::SendCloakFx(bool apply/*false*/, bool module/*false*/) const {
    PyTuple *up(nullptr);
    if (module) {
        OnSpecialFX14 effect;
        effect.entityID = mySE->GetID();
        effect.isOffensive = 0;
        if (apply) {
            effect.guid = "effects.Cloaking";
            effect.start = 1;
            effect.active = 1;
        } else {
            effect.guid = "effects.Uncloak";
        }
        up = effect.Encode();
    } else {
        OnSpecialFX10 effect;
        if (apply) {
            effect.guid = "effects.Cloak";
        } else {
            effect.guid = "effects.Uncloak";
        }
        effect.entityID = mySE->GetID();
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
        up = effect.Encode();
    }
    SendSingleDestinyUpdate(&up);   // consumed
}

// def OnSpecialFX(shipID, moduleID, moduleTypeID, targetID, otherTypeID, area, guid, isOffensive, start, active, duration = -1, repeat = None, startTime = None, graphicInfo = None):

void DestinyManager::SendSpecialEffect10(uint32 entityID, uint32 targetID, std::string guid, bool isOffensive, bool start, bool isActive) const
{
    OnSpecialFX10 effect;
        effect.entityID = entityID;
        effect.targetID = targetID;
        effect.guid = guid;
        effect.area = new PyList();     // this is unused variable in client.
        effect.isOffensive = isOffensive;
        effect.start = start;
        effect.active = isActive;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

// def OnSpecialFX(shipID, moduleID, moduleTypeID, targetID, otherTypeID, area, guid, isOffensive, start, active, duration = -1, repeat = None, startTime = None, graphicInfo = None):

void DestinyManager::SendSpecialEffect(uint32 entityID, uint32 moduleID, uint32 moduleTypeID, uint32 targetID,
                                       uint32 chargeTypeID, std::string guid, bool isOffensive, bool start,
                                       bool isActive, int32 duration, uint32 repeat, int32 graphicInfo/*0*/) const
{
    OnSpecialFX14 effect;
        effect.entityID = entityID;
        effect.moduleID = moduleID;
        effect.moduleTypeID = moduleTypeID;     // npc typeID for npc's/drones
        effect.targetID = (targetID == 0 ? PyStatic.NewNone() : new PyInt(targetID));
        effect.chargeTypeID = (chargeTypeID == 0 ? PyStatic.NewNone() : new PyInt(chargeTypeID));
        effect.guid = guid;
        effect.isOffensive = isOffensive;                  // bool
        effect.start = start;                   // bool
        effect.active = isActive;                  // bool
        effect.duration = duration;
        effect.repeat = repeat;
        effect.startTime = GetFileTimeNow();
        effect.graphicInfo = (graphicInfo == 0 ? PyStatic.NewNone() : new PyInt(graphicInfo));
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}
/*
                  [PyTuple 2 items]
                    [PyInt 62565]
                    [PyTuple 2 items]
                      [PyString "OnSpecialFX"]
                      [PyTuple 14 items]
                        [PyIntegerVar 9000000000001190096]
                        [PyIntegerVar 9000000000001190096]
                        [PyInt 11931]
                        [PyNone]
                        [PyNone]
                        [PyList 0 items]
                        [PyString "effects.ShieldBoosting"]
                        [PyBool False]
                        [PyInt 1]
                        [PyInt 1]
                        [PyFloat 5000]
                        [PyInt 1]
                        [PyIntegerVar 129756560173255648]
                        [PyNone]
                */

void DestinyManager::SendJumpOut(uint32 gateID) const {
    OnSpecialFX10 effect;
        effect.entityID = mySE->GetID();
        effect.targetID = gateID;
        effect.guid = "effects.JumpOut";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

void DestinyManager::SendJumpOutWormhole(uint32 wormholeID) const {
    OnSpecialFX10 effect;
        effect.entityID = mySE->GetID();
        effect.targetID = wormholeID;
        effect.guid = "effects.JumpOutWormhole";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

void DestinyManager::SendGateActivity(uint32 gateID) const {
    OnSpecialFX10 du;
        du.entityID = gateID;
        du.guid = "effects.GateActivity";
        du.isOffensive = 0;
        du.start = 1;
        du.active = 0;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

void DestinyManager::SendWormholeActivity(uint32 wormholeID) const {
    OnSpecialFX10 du;
        du.entityID = wormholeID;
        du.guid = "effects.WormholeActivity";
        du.isOffensive = 0;
        du.start = 1;
        du.active = 0;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

void DestinyManager::SendBallInteractive(const ShipItemRef shipRef, bool set/*false*/) const {
    // interactive means "ship has pilot"
    SetBallInteractive sbi;
        sbi.entityID = shipRef->itemID();
        sbi.interactive = set;
    PyTuple* up = sbi.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

void DestinyManager::SendJumpOutEffect(std::string JumpEffect, uint32 shipID) const {
    std::vector<PyTuple*> updates;
    CmdStop du;
        du.entityID = mySE->GetID();
    updates.push_back(du.Encode());
    OnSpecialFX14 effect;
        effect.entityID = mySE->GetID();
        effect.targetID = new PyInt(shipID);
        effect.guid = "effects.JumpDriveOut";   /* JumpDriveInBO */
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 1;
        effect.duration = 5000;
        effect.repeat = 0;
        effect.startTime = GetFileTimeNow();
    updates.push_back(effect.Encode());
    SendDestinyUpdate(updates);
}

void DestinyManager::SendJumpInEffect(std::string JumpEffect) const {
    std::vector<PyTuple*> updates;
    OnSpecialFX14 effect;
        effect.guid = "effects.JumpDriveIn";
        effect.entityID = mySE->GetID();
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 1;
        effect.duration = 2000;
        effect.repeat = 0;
        effect.startTime = GetFileTimeNow();
        effect.targetID = new PyInt(mySE->GetID());
    updates.push_back(effect.Encode());
    CmdSetSpeedFraction ssf;
        ssf.entityID = mySE->GetID();
        ssf.fraction = 0.0;
    updates.push_back(ssf.Encode());
    SetBallVelocity sbv;
        sbv.entityID = mySE->GetID();
        sbv.x = 0.0;
        sbv.y = 0.0;
        sbv.z = 0.0;
    updates.push_back(sbv.Encode());
    SendDestinyUpdate(updates);
}

void DestinyManager::SendTerminalExplosion(uint32 shipID, uint32 bubbleID, bool isGlobal/*false*/) const {
    //exploders = [ x[1][1][0] for x in state if x[1][0] == 'TerminalExplosion' ]
    /*
                  [PyTuple 2 items]                         x
                    [PyInt 62609]                           x[0]
                    [PyTuple 2 items]                       x[1]
                      [PyString "TerminalExplosion"]        x[1][0]
                      [PyTuple 3 items]                     x[1][1]
                        [PyIntegerVar 9000000000001190702]  x[1][1][0]
                        [PyInt 39]
                        [PyBool False]
                */
    //send an explosion special effects update...
     TerminalExplosion du;
        du.shipID = shipID;
        du.bubbleID = bubbleID;
        du.ballIsGlobal = isGlobal;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

void DestinyManager::SendSetState() const {
    if (!mySE->HasPilot()) {
        return;
    }

    if (is_log_enabled(DESTINY__MESSAGE)) {
        _log(
            DESTINY__MESSAGE,
            "Destiny::SendSetState() Called for Ship:%s(%u) Pilot:%s(%u)",
            mySE->GetName(),
            mySE->GetID(),
            mySE->GetPilot()->GetName(),
            mySE->GetPilot()->GetCharacterID()
        );
    }

    // if the player is not warping, tell the client they're not warping.
    // As of 2024-10-03, this doesn't always work and there are still issues
    // with the client sometimes not starting a warp sequence.
    std::vector<PyTuple*> updates;
    OnSpecialFX10 sfx;
    sfx.guid = "effects.Warping";
    sfx.entityID = mySE->GetID();
    sfx.isOffensive = false;
    sfx.start = false;
    sfx.active = false;
    if (m_ballMode == Destiny::Ball::Mode::WARP) {
        sfx.start = true; // TODO: verify if this is necessary
        sfx.active = true;
    }

    updates.push_back(sfx.Encode());
    SendDestinyUpdate(updates);
    updates.clear();

    SetState ss;

    ss.stamp = sEntityList.GetStamp();
    ss.ego = mySE->GetID();

    if (mySE->SysBubble() == nullptr) {
        // returning here preemptively avoids a segfault (good), but isn't a
        // good situation to encounter.
        sLog.Error(
            "DestinyManager::SendSetState()",
            "Destiny::SendSetState() the player isn't in a system bubble! Aborting attempt to send state."
        );

        return;
    }

    mySE->SystemMgr()->MakeSetState(mySE->SysBubble(), ss);

    PyTuple* tmp(ss.Encode());

    // setstate should be alone and immediate. send directly
    mySE->GetPilot()->QueueDestinyUpdate(&tmp, true, true); // consumed

    mySE->GetPilot()->SetStateSent(true);
}

void DestinyManager::SendMovementPacket()
{
    SendSingleDestinyUpdate(&mvPacket);
    PySafeDecRef(mvPacket);
}

void DestinyManager::SendSingleDestinyEvent(PyTuple** ev, bool self_only/*false*/) const
{
    std::vector<PyTuple*> updates;
    std::vector<PyTuple*> events(1, *ev);   // create vector of size "1" and insert "*ev" into it
    SendDestinyUpdate(updates, events, self_only);
}

void DestinyManager::SendSingleDestinyUpdate(PyTuple **up, bool self_only/*false*/) const {
    std::vector<PyTuple*> updates(1, *up);   // create vector of size "1" and insert "*up" into it
    std::vector<PyTuple*> events;
    SendDestinyUpdate(updates, events, self_only);
}

void DestinyManager::SendDestinyUpdate(std::vector<PyTuple*> &updates, bool self_only/*false*/) const {
    std::vector<PyTuple*> events;
    SendDestinyUpdate(updates, events, self_only);
}

void DestinyManager::SendDestinyUpdate( std::vector<PyTuple*>& updates, std::vector<PyTuple*>& events, bool self_only/*false*/) const {
    // this check shouldnt be needed...
    if (!mySE->SystemMgr()->IsLoaded()) {
        return;
    }

    if (self_only) {
        if (!mySE->HasPilot()) {
            // this entity is NOT a player ship...change to BubbleCast (or silently fail)
            if (mySE->SysBubble() != nullptr) {
                if (is_log_enabled(DESTINY__UPDATES))
                    _log(
                        DESTINY__UPDATES,
                        "[%u] BubbleCasting destiny update (u:%lu, e:%lu) for stamp %u to bubbleID %u from %s(%u)",
                        sEntityList.GetStamp(),
                        updates.size(),
                        events.size(),
                        sEntityList.GetStamp(),
                        mySE->SysBubble()->GetID(),
                        mySE->GetName(),
                        mySE->GetID()
                    );
                mySE->SysBubble()->BubblecastDestiny(updates, events, "destiny" );
            }

            return;
        }

        if (is_log_enabled(PLAYER__MESSAGE))
            _log(PLAYER__MESSAGE, "[%u] DestinyManager::SendDestinyUpdate() (u:%lu, e:%lu) called as 'self_only' for %s(%i)", \
                    sEntityList.GetStamp(), updates.size(), events.size(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());

        for (std::vector<PyTuple*>::iterator itr = updates.begin(); itr != updates.end(); ++itr) {
            PyIncRef(*itr);
            mySE->GetPilot()->QueueDestinyUpdate(&(*itr));
        }

        for (std::vector<PyTuple*>::iterator itr = events.begin(); itr != events.end(); ++itr) {
            PyIncRef(*itr);
            mySE->GetPilot()->QueueDestinyEvent(&(*itr));
        }
    } else if (mySE->IsOperSE()) { //These are global entities, so we have to send update to all bubbles in a system
        if (is_log_enabled(DESTINY__UPDATES)) {
            _log(
                DESTINY__UPDATES,
                "[%u] BubbleCasting global structure destiny update (u:%u, e:%u) for stamp %u to all bubbles from %s(%u)",
                sEntityList.GetStamp(),
                updates.size(),
                events.size(),
                sEntityList.GetStamp(),
                (mySE->HasPilot()?mySE->GetPilot()->GetName():mySE->GetName()),
                (mySE->HasPilot()?mySE->GetPilot()->GetCharID():mySE->GetID())
            );
        }

        //Get all clients in the system which the SE is in
        /** @todo  this isnt right....will segfault.  needs to be fixed */
        std::vector<Client*> cv;

        mySE->SystemMgr()->GetClientList(cv);

        for(auto const& value: cv) {
            value->GetShipSE()->SysBubble()->BubblecastDestiny(updates, events, "destiny");
        }
    } else if (mySE->SysBubble() != nullptr) {
        if (is_log_enabled(DESTINY__UPDATES)) {
            _log(
                DESTINY__UPDATES,
                "[%u] BubbleCasting destiny update (u:%u, e:%u) for stamp %u to bubbleID %u from %s(%u)",
                sEntityList.GetStamp(),
                updates.size(),
                events.size(),
                sEntityList.GetStamp(),
                mySE->SysBubble()->GetID(),
                (mySE->HasPilot()?mySE->GetPilot()->GetName():mySE->GetName()),
                (mySE->HasPilot()?mySE->GetPilot()->GetCharID():mySE->GetID())
            );
        }

        mySE->SysBubble()->BubblecastDestiny( updates, events, "destiny" );
    } else {
        _log(
            DESTINY__ERROR,
            "[%u] Cannot BubbleCast destiny update (u:%u, e:%u); entity (%u) is not in any bubble.",
            sEntityList.GetStamp(),
            updates.size(),
            events.size(),
            mySE->GetID()
        );

        // if (sConfig.debug.IsTestServer) {
        //     EvE::traceStack();
        // }

        //sBubbleMgr.Add(mySE);
        //mySE->SysBubble()->BubblecastDestiny( updates, events, "destiny" );
    }
}

void DestinyManager::WarpScrambled(InventoryItemRef moduleRef, bool apply) {
    if (apply) {
        m_isWarpScrambled = true;
        m_scramblerID = moduleRef->itemID();
        m_scrambleStrength = moduleRef->GetAttribute(AttrWarpScrambleStrength).get_float();

        // 发送状态更新到客户端
        PyTuple* tuple = new PyTuple(3);
        tuple->SetItem(0, new PyInt(m_scramblerID));
        tuple->SetItem(1, new PyFloat(m_scrambleStrength));
        tuple->SetItem(2, new PyBool(true));

        // 创建OnModuleWarpScramble消息
        PyTuple* update = new PyTuple(2);
        update->SetItem(0, new PyString("OnModuleWarpScramble"));
        update->SetItem(1, tuple);
        SendSingleDestinyUpdate(&update);  // 传递PyTuple**
    } else {
        m_isWarpScrambled = false;
        m_scramblerID = 0;
        m_scrambleStrength = 0;

        // 发送状态更新到客户端
        PyTuple* tuple = new PyTuple(3);
        tuple->SetItem(0, new PyInt(m_scramblerID));
        tuple->SetItem(1, new PyFloat(0));
        tuple->SetItem(2, new PyBool(false));

        // 创建OnModuleWarpScramble消息
        PyTuple* update = new PyTuple(2);
        update->SetItem(0, new PyString("OnModuleWarpScramble"));
        update->SetItem(1, tuple);
        SendSingleDestinyUpdate(&update);  // 传递PyTuple**
    }
}
