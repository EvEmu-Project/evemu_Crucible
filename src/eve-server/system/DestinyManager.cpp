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

/* Movement model
 *
 * The client runs its own copy of the destiny simulation and applies every destiny update at the tick
 * it was stamped with, rewinding or fast-forwarding as needed.  Given the same starting state and the same
 * stamped commands it always ends up in the same place.  The server therefore runs the same simulation:
 *
 *  - every command that changes movement is applied to our state with the client's semantics (the Apply*
 *    methods) at the moment it is sent.  the update is stamped with the current tick, and our state is the
 *    state at that tick, so the client applies it to the same state we did.
 *  - once per tick SystemManager::ProcessDestiny() calls EvolveThrust() on every moving entity, then
 *    EvolveMove() on every moving entity (the client works out every ball's thrust from the same snapshot
 *    before it moves any of them), then PostEvolve() for server-side consequences.
 *
 * The formulas are taken from the Crucible client's destiny.dll.  The open-sourced destiny in __destiny/ is a
 * much later version, and differs in the warp curve, orbit precession and goto homing range.
 * Names in comments like [mGoto] are the matching members of destiny's Ball class.
 */

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
#include "utils/Buffer.h"
#include <cstdlib>
#include <cmath>

namespace {
    // client ballpark constants (Ballpark constructor in destiny.dll)
    const double DST_FRICTION = 1000000.0;
    const double DST_DT = 1.0;                      // 1000ms tick
    const double DST_AU = 149597870700.0;
    const double DST_GOTO_DIRECTION_RANGE = 1.0e17; // GotoDirection aims at a point this far away
    const double DST_MIN_WARP_DISTANCE_SQ = 1.0e10; // client turns warps under 100km into GotoPoint

    // destiny's Normalize() leaves zero-length vectors untouched
    inline void Normalize(GVector& v) {
        double len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        if (len != 0.0) {
            len = 1.0 / len;
            v.x *= len;
            v.y *= len;
            v.z *= len;
        }
    }

    // destiny rounds some orbit terms to 7 decimals so all platforms agree
    inline double Round7(double v) {
        return static_cast<double>(static_cast<int64>(v * 10000000.0)) / 10000000.0;
    }

    // the client keeps these ball properties as floats
    inline double AsFloat(double v) {
        return static_cast<double>(static_cast<float>(v));
    }
}

DestinyManager::DestinyManager(SystemEntity *self)
: mySE(self),
m_targBubble(nullptr),
m_frozen(false)
{
    m_hasSentShipUpdates = false;

    m_mass = 1.0f;
    m_massMKg = 0.000001f;
    m_alignTime = 1.0f;
    m_maxShipSpeed = 100.0f;
    m_shipWarpSpeed = 1.0f;
    m_timeToEnterWarp = 10.0f;
    m_shipMaxAccelTime = 0.0f;
    m_preTractorMass = 1.0f;

    m_radius = mySE->GetRadius();
    m_capNeeded = 0.0;
    m_warpCapacitorNeed = 0.00001;
    m_shipAgility = 1.0;
    m_shipInertia = 1.0;

    m_ballMode = Destiny::Ball::Mode::STOP;
    m_position = mySE->GetPosition();
    m_velocity = GVector(NULL_ORIGIN);
    m_targetPoint = GPoint(NULL_ORIGIN);
    m_thrust = GVector(NULL_ORIGIN);
    m_speedFraction = 0.0f;
    m_followRange = 0.0f;
    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;
    m_effectStamp = 0;
    m_warpDistance = -1.0;
    m_warpMinRange = 0.0;
    m_warpFactor = 0;

    m_shipHeading = GVector(NULL_ORIGIN);
    m_stateStamp = 0;
    m_resetSpeedOnMove = true;

    m_warpActivated = false;
    m_warpExited = false;
    m_targetLost = false;

    m_cloaked = false;
    m_tractored = false;
    m_bump = false;
}

DestinyManager::~DestinyManager() {
}

// called once per tic by SystemEntity::Process(), after this tic's movement has been simulated
void DestinyManager::Process() {
    if (mySE->IsFrozen())
        Halt();
}

/* ======================================================================================
 *  per-tick simulation
 * ====================================================================================== */

// phase 1 of the client's Ballpark::Evolve() - works out this tick's thrust for the ball.
//  'tick' is the client's ballpark time this evolve starts from (the stamp before the current one)
void DestinyManager::EvolveThrust(int32 tick) {
    using namespace Destiny;
    m_thrust = GVector(NULL_ORIGIN);

    switch (m_ballMode) {
        case Ball::Mode::GOTO: {
            m_thrust = GotoThrust(m_targetPoint);
        } break;
        case Ball::Mode::FOLLOW: {
            SystemEntity* pTarget = GetFollowTarget();
            if (pTarget == nullptr) {
                ApplyStop();
                m_targetLost = true;
                break;
            }
            // aim for a point at follow range (plus both radii) from the target, on the line between us
            const GPoint& otherPos = pTarget->GetPosition();
            GVector delta(otherPos, m_position);
            double dist = delta.length();
            double range = static_cast<double>(m_followRange) + AsFloat(m_radius) + AsFloat(pTarget->GetRadius());
            if (dist == 0.0) {
                m_targetPoint = GPoint(otherPos.x + range, otherPos.y + range * 0.0, otherPos.z + range * 0.0);
            } else {
                double invDist = 1.0 / dist;
                m_targetPoint = GPoint(delta.x * range * invDist + otherPos.x,
                                       delta.y * range * invDist + otherPos.y,
                                       delta.z * range * invDist + otherPos.z);
            }
            m_thrust = GotoThrust(m_targetPoint);
        } break;
        case Ball::Mode::STOP: {
            // stopped balls level out their vertical velocity faster than the rest
            m_velocity.y = (m_velocity.y - m_velocity.y * 0.07) * 0.9345794392523364;
        } break;
        case Ball::Mode::WARP: {
            // once in warp proper, position comes straight from WarpDistance() in EvolveMove()
            if (m_effectStamp != -1)
                break;
            if (IsAlignedForWarp()) {
                RealWarp(tick);
                break;
            }
            m_thrust = GotoThrust(m_targetPoint);
        } break;
        case Ball::Mode::ORBIT: {
            SystemEntity* pTarget = GetFollowTarget();
            if (pTarget == nullptr) {
                ApplyStop();
                m_targetLost = true;
                break;
            }
            m_thrust = OrbitThrust(pTarget, tick);
        } break;
        default:
            // MISSILE is moved in EvolveMove().  everything else doesn't move under its own power
            break;
    }
}

// phase 2 of the client's Ballpark::Evolve() - moves the ball
void DestinyManager::EvolveMove(int32 tick) {
    using namespace Destiny;
    if (m_ballMode == Ball::Mode::MISSILE) {
        MoveMissile();
        return;
    }

    GPoint pos(m_position);
    GVector vel(m_velocity);
    if (IsWarping()) {
        if (WarpDistance(pos, vel, static_cast<double>(tick - m_effectStamp) * DST_DT)) {
            // just dropped out of warp.  the client integrates one step from the exit point and coasts from there
            Integrate(pos, vel, m_thrust);
        }
    } else {
        // nothing to do for a ball at rest.  this also leaves fixed balls alone
        if (vel.isZero() and m_thrust.isZero())
            return;
        Integrate(pos, vel, m_thrust);
    }

    m_velocity = vel;
    MoveBallTo(pos);

    if (m_velocity.isNotZero()) {
        m_shipHeading = m_velocity;
        Normalize(m_shipHeading);
    }
}

// server-side consequences of this tick's movement.  this may add/remove entities and bubbles
void DestinyManager::PostEvolve() {
    if (m_targetLost) {
        // our follow/orbit target is gone.  the client should have stopped us already, but make sure
        m_targetLost = false;
        CmdStop du;
            du.entityID = mySE->GetID();
        PyTuple* up = du.Encode();
        SendSingleDestinyUpdate(&up);   // consumed
    }

    if (m_warpActivated) {
        m_warpActivated = false;
        OnWarpActivated();
    }

    if (IsWarping() or m_warpExited)
        UpdateWarpBubble();

    if (m_warpExited) {
        m_warpExited = false;
        OnWarpExited();
    }

    if ((m_ballMode == Destiny::Ball::Mode::FOLLOW) or (m_ballMode == Destiny::Ball::Mode::ORBIT))
        IsTargetInvalid();

    if (!IsMoving())
        return;

    if (sConfig.debug.PositionHack)
        SetPosition(m_position, true);

    if (sConfig.cosmic.BumpEnabled)
        if (mySE->HasPilot() and (mySE->SysBubble() != nullptr) and mySE->SysBubble()->HasPlayers())
            CheckBump();
}

/* ======================================================================================
 *  client physics, from the Crucible client's destiny.dll
 * ====================================================================================== */

// mass used in the client's equations of motion: mass * agility(inertia modifier)
double DestinyManager::BallMass() const {
    return static_cast<double>(m_mass) * AsFloat(m_shipInertia);
}

// Ballpark::GotoThrust.  full thrust towards the target, scaled down once within one tick of travel
GVector DestinyManager::GotoThrust(const GPoint& target) const {
    GVector a(m_position, target);
    double mass = BallMass();
    if (mass <= 0.0)
        return GVector(NULL_ORIGIN);

    double len2 = a.y * a.y + a.x * a.x + a.z * a.z;
    double maxVel = static_cast<double>(m_maxShipSpeed);
    double fraction = static_cast<double>(m_speedFraction);
    // the Crucible client adds the ball radius to the homing distance
    double homing = maxVel * fraction * DST_DT + AsFloat(m_radius);
    double maxThrust = (fraction * DST_FRICTION * maxVel) / mass;

    Normalize(a);
    if (len2 < homing * homing) {
        double coff = len2 / (homing * homing);
        a *= coff * maxThrust * coff;
    } else {
        a *= maxThrust;
    }
    return a;
}

// old-style orbit from the Crucible client (Ballpark::Evolve, ORBIT case).
//  the orbital plane precesses at 1% of the orbit's angular speed, seeded by the ball's id
GVector DestinyManager::OrbitThrust(SystemEntity* pTarget, int32 tick) const {
    double mass = BallMass();
    if (mass <= 0.0)
        return GVector(NULL_ORIGIN);

    GVector toVec(m_position, pTarget->GetPosition());
    double range = static_cast<double>(m_followRange) + AsFloat(m_radius) + AsFloat(pTarget->GetRadius());
    double dist = toVec.length();
    if (dist != 0.0) {
        double invDist = 1.0 / dist;
        toVec.x *= invDist;
        toVec.y *= invDist;
        toVec.z *= invDist;
    }

    double cruise = static_cast<double>(m_speedFraction) * static_cast<double>(m_maxShipSpeed);
    double precession = 0.0;
    if (range != 0.0)
        precession = (DST_DT * 0.01 * cruise) / range;
    double phi1 = static_cast<double>(tick) * precession;
    double phi2 = phi1 * 0.7 + static_cast<double>(mySE->GetID() & 0xffff);
    double cosPhi2 = std::cos(phi2);
    GVector radial(Round7(std::cos(phi1) * cosPhi2), Round7(std::sin(phi2)), Round7(std::sin(phi1) * cosPhi2));

    // radial = radial x toVec, now transverse to the target direction
    radial = GVector(radial.y * toVec.z - radial.z * toVec.y,
                     radial.z * toVec.x - radial.x * toVec.z,
                     radial.x * toVec.y - radial.y * toVec.x);
    Normalize(radial);

    // when outside the orbit, aim along the tangent to it
    double toComp = dist * dist - range * range;
    if (toComp >= 0.0) {
        double radComp = (std::sqrt(toComp) * range) / dist;
        toComp /= dist;
        toVec = GVector(toComp * toVec.x + radComp * radial.x,
                        toComp * toVec.y + radComp * radial.y,
                        toComp * toVec.z + radComp * radial.z);
        Normalize(toVec);
    }

    // share of thrust put into the radial component
    double diff = dist - range;
    double radialFactor = Round7(std::exp((diff * diff) / -40000.0));

    // law of cosines for the other factor
    double cosAngle = -toVec.y * radial.y + -toVec.x * radial.x + -toVec.z * radial.z;
    double transverseFactor = (cosAngle * cosAngle - 1.0) * radialFactor * radialFactor + 1.0;
    if (transverseFactor > 0.0) {
        transverseFactor = std::sqrt(transverseFactor) + radialFactor * cosAngle;
    } else {
        transverseFactor = radialFactor * cosAngle;
    }
    transverseFactor *= ((dist - range < 0.0) ? -1.0 : 1.0);

    double maxThrust = (DST_FRICTION * cruise) / mass;
    return GVector((radialFactor * radial.x + toVec.x * transverseFactor) * maxThrust,
                   (radialFactor * radial.y + toVec.y * transverseFactor) * maxThrust,
                   (radialFactor * radial.z + toVec.z * transverseFactor) * maxThrust);
}

// the client enters warp once it is pointed within ~8 degrees of the destination at over 75% of max speed.
//  there is no time limit on aligning in the Crucible client
bool DestinyManager::IsAlignedForWarp() const {
    GVector dir(m_position, m_targetPoint);
    Normalize(dir);
    GVector velDir(m_velocity);
    Normalize(velDir);
    double misalign = 1.0 - (velDir.x * dir.x + velDir.y * dir.y + velDir.z * dir.z);
    if (misalign < 0.0)
        misalign = -misalign;
    if (misalign >= 0.01)
        return false;
    double maxVel = static_cast<double>(m_maxShipSpeed);
    return (m_velocity.lengthSquared() > maxVel * 0.5625 * maxVel);
}

// Ballpark::RealWarp.  starts warp proper from the current position
void DestinyManager::RealWarp(int32 tick) {
    StopFollowers();

    // the warp-to range is applied here, along the line from the destination back to where we are now
    GVector dir(m_targetPoint, m_position);
    Normalize(dir);
    m_targetPoint = GPoint(dir.x * m_warpMinRange + m_targetPoint.x,
                           dir.y * m_warpMinRange + m_targetPoint.y,
                           dir.z * m_warpMinRange + m_targetPoint.z);
    m_effectStamp = tick;
    m_warpDistance = GVector(m_position, m_targetPoint).length();
    m_warpActivated = true;
}

/* Ballpark::WarpDistance from the Crucible client.  position and velocity are a function of time in warp:
 *   S = warp speed (capped at 1.5x the warp distance D so short warps still have room to slow down)
 *   accelerate:  x = e^(3t) until speed 3e^(3t) reaches S, at ta = ln(S/3)/3, having covered S/3
 *   cruise:      at S until S/3 remains, at tb = ta + D/S - 2/3
 *   decelerate:  remaining distance and speed both (S/3)e^-(t-tb)
 * the ball drops out of warp once speed falls below its max velocity.  returns true when that happens
 */
bool DestinyManager::WarpDistance(GPoint& pos, GVector& vel, double t) {
    double total = m_warpDistance;
    double speed = static_cast<double>(m_warpFactor) * 0.1 * DST_AU;
    if (speed >= total * 1.5)
        speed = total * 1.5;

    double logSpeed = std::log(speed / 3.0);
    double accelTime = logSpeed / 3.0;
    double cruiseEnd = (logSpeed * speed - speed * 2.0 + total * 3.0) / (speed * 3.0);

    GVector dir(m_targetPoint, pos);
    Normalize(dir);

    double remaining(0.0), curSpeed(0.0);
    bool exited(false);
    if (accelTime > t) {
        double travelled = std::exp(3.0 * t);
        remaining = total - travelled;
        curSpeed = travelled * 3.0;
    } else if (cruiseEnd > t) {
        double travelled = ((accelTime * 3.0 - (t * 3.0 + 1.0)) * speed) / -3.0;
        remaining = total - travelled;
        curSpeed = speed;
    } else {
        double decay = std::exp((accelTime * (speed * 3.0) - (t * speed * 3.0 + speed * 2.0) + total * 3.0) / speed / 3.0);
        double third = speed * 0.3333333333333333;
        double full = third * 3.0;
        double travelled = ((total * 3.0 - speed) - decay * full + full) / 3.0;
        remaining = total - travelled;
        curSpeed = decay * third;
        if (curSpeed < 0.0)
            curSpeed = -curSpeed;
        if (curSpeed < static_cast<double>(m_maxShipSpeed)) {
            ApplyStop();
            m_warpExited = true;
            exited = true;
        }
    }

    pos = GPoint(m_targetPoint.x + remaining * dir.x,
                 m_targetPoint.y + remaining * dir.y,
                 m_targetPoint.z + remaining * dir.z);
    vel = GVector(dir.x * -curSpeed, dir.y * -curSpeed, dir.z * -curSpeed);
    return exited;
}

// Ballpark::Integrate.  exact solution for thrust against linear drag over one tick
void DestinyManager::Integrate(GPoint& pos, GVector& vel, const GVector& accel) const {
    double m = BallMass();
    if (m <= 0.0)
        return;

    const double k = DST_FRICTION;
    const double t = DST_DT;
    const double tf = std::exp(-k * t / m);
    const double k2 = k * k;
    const double ook2 = 1.0 / k2;
    const double ook = 1.0 / k;

    pos.x = ((accel.x * m * (tf - 1.0) + ((vel.x + accel.x * t) - vel.x * tf) * k) * m + k2 * pos.x) * ook2;
    pos.y = ((accel.y * m * (tf - 1.0) + ((vel.y + accel.y * t) - vel.y * tf) * k) * m + k2 * pos.y) * ook2;
    pos.z = ((accel.z * m * (tf - 1.0) + ((vel.z + accel.z * t) - vel.z * tf) * k) * m + k2 * pos.z) * ook2;
    vel.x = (accel.x * m - (accel.x * m - vel.x * k) * tf) * ook;
    vel.y = (accel.y * m - (accel.y * m - vel.y * k) * tf) * ook;
    vel.z = (accel.z * m - (accel.z * m - vel.z * k) * tf) * ook;
}

// missiles are still moved in a straight line towards their target point.  their impact is timed by Missile
void DestinyManager::MoveMissile() {
    GVector moveVector(m_position, m_targetPoint);
    moveVector.normalize();
    m_shipHeading = moveVector;
    m_velocity = (moveVector * m_maxShipSpeed);
    MoveBallTo(m_position + m_velocity);
}

/* ======================================================================================
 *  client-semantics state changes.  these mirror what the client does when it receives the matching update
 * ====================================================================================== */

void DestinyManager::MoveBallTo(const GPoint& pt) {
    m_position = pt;
    // this sets InventoryItemRef.m_position correctly, which is used for all position references
    mySE->SetPosition(m_position);
}

// Ballpark::Stop
void DestinyManager::ApplyStop() {
    using namespace Destiny;
    if ((m_ballMode == Ball::Mode::FOLLOW) or (m_ballMode == Ball::Mode::ORBIT)
    or  (m_ballMode == Ball::Mode::MISSILE) or (m_ballMode == Ball::Mode::FORMATION)) {
        m_effectStamp = 0;
        m_targetEntity.first = 0;
        m_targetEntity.second = nullptr;
        m_warpFactor = 0;
        m_followRange = 0.0f;
    }
    m_ballMode = Ball::Mode::STOP;
}

// Ballpark::GotoPoint
void DestinyManager::ApplyGotoPoint(const GPoint& point) {
    if (!std::isfinite(point.x) or !std::isfinite(point.y) or !std::isfinite(point.z))
        return;
    ApplyStop();
    m_targetPoint = point;
    if (m_speedFraction == 0.0f)
        m_speedFraction = 1.0f;
    m_ballMode = Destiny::Ball::Mode::GOTO;
}

// Ballpark::FollowBall and Ballpark::Orbit
void DestinyManager::ApplyFollow(SystemEntity* pSE, float range, uint8 mode) {
    if (pSE == nullptr)
        return;
    ApplyStop();
    // the client stops the ball, then refuses to follow a cloaked target
    if ((pSE->DestinyMgr() != nullptr) and pSE->DestinyMgr()->IsCloaked())
        return;
    m_targetEntity.first = pSE->GetID();
    m_targetEntity.second = pSE;
    m_followRange = range;
    m_ballMode = mode;
}

// Ballpark::WarpTo
void DestinyManager::ApplyWarpTo(const GPoint& where, double minRange, int32 warpFactor) {
    if (!std::isfinite(where.x) or !std::isfinite(where.y) or !std::isfinite(where.z))
        return;
    if (minRange < 0.0)
        minRange = 0.0;
    if (warpFactor < 1)
        warpFactor = 1;

    if (GVector(m_position, where).lengthSquared() < DST_MIN_WARP_DISTANCE_SQ) {
        ApplyGotoPoint(where);
        return;
    }

    ApplyStop();
    m_targetPoint = where;
    m_followRange = 0.0f;
    m_warpMinRange = minRange;
    m_effectStamp = -1;
    m_warpFactor = warpFactor;
    m_warpDistance = -1.0;
    m_ballMode = Destiny::Ball::Mode::WARP;
}

SystemEntity* DestinyManager::GetFollowTarget() {
    if (m_targetEntity.first == 0)
        return nullptr;
    // look the target up every time, as it may have been removed from the system
    m_targetEntity.second = mySE->SystemMgr()->GetSE(m_targetEntity.first);
    return m_targetEntity.second;
}

// Ballpark::StopAllFollowers.  the client stops everything following a ball when it enters warp
void DestinyManager::StopFollowers() {
    using namespace Destiny;
    for (auto cur : mySE->SystemMgr()->GetTicEntities()) {
        if (cur.second == mySE)
            continue;
        DestinyManager* pDestiny = cur.second->DestinyMgr();
        if (pDestiny == nullptr)
            continue;
        if ((pDestiny->m_ballMode != Ball::Mode::FOLLOW) and (pDestiny->m_ballMode != Ball::Mode::ORBIT))
            continue;
        if (pDestiny->m_targetEntity.first == mySE->GetID())
            pDestiny->ApplyStop();
    }
}

/* ======================================================================================
 *  server-side warp handling
 * ====================================================================================== */

// the client has just entered warp proper (it does this on its own once aligned)
void DestinyManager::OnWarpActivated() {
    m_stateStamp = sEntityList.GetStamp();

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::OnWarpActivated(): %s(%u) entered warp at tick %i.  %.4f AU to %.2f,%.2f,%.2f at %.1f AU/s.",
                mySE->GetName(), mySE->GetID(), m_effectStamp, m_warpDistance / DST_AU,
                m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, static_cast<double>(m_warpFactor) * 0.1);

    //drain cap
    if (mySE->HasPilot()) {
        mySE->GetSelf()->SetAttribute(AttrCapacitorCharge, m_capNeeded);
        mySE->GetShipSE()->Warp();
        m_capNeeded = 0;
    }

    //clear targets
    if (mySE->TargetMgr() != nullptr)
        mySE->TargetMgr()->ClearAllTargets();
}

// the client has just dropped out of warp, and is coasting to a stop
void DestinyManager::OnWarpExited() {
    m_stateStamp = sEntityList.GetStamp();

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::OnWarpExited(): %s(%u) - Warp complete at %.2f,%.2f,%.2f, %.2f m from exit point, at %.2f m/s.",
                mySE->GetName(), mySE->GetID(), m_position.x, m_position.y, m_position.z,
                m_position.distance(m_targetPoint), m_velocity.length());

    // the client makes the ball massive when it drops out of warp.  we don't simulate collisions, so undo that
    SetBallMassive bm;
        bm.entityID = mySE->GetID();
        bm.is_massive = false;
    PyTuple* up = bm.Encode();
    SendSingleDestinyUpdate(&up);   // consumed

    m_targBubble = nullptr;

    if (mySE->IsShipSE() and mySE->HasPilot()) {
        _log(AUTOPILOT__MESSAGE, "Destiny::OnWarpExited(): %s(%u) - Warp complete.", mySE->GetName(), mySE->GetID());
        mySE->GetPilot()->SetLoginWarpComplete();
    }

    if ((mySE->IsNPCSE()) and (mySE->GetNPCSE()->GetAIMgr() != nullptr))
        mySE->GetNPCSE()->GetAIMgr()->WarpOutComplete();
}

// keeps a warping entity in the bubble it is passing through, ending in the bubble at the exit point
void DestinyManager::UpdateWarpBubble() {
    SystemBubble* pCurrent(mySE->SysBubble());
    if ((pCurrent != nullptr) and pCurrent->InBubble(m_position, true))
        return;

    SystemBubble* pNext(nullptr);
    if ((m_targBubble != nullptr) and m_targBubble->InBubble(m_position, true)) {
        pNext = m_targBubble;
    } else {
        pNext = sBubbleMgr.GetBubble(mySE->SystemMgr(), m_position);
    }

    if ((pNext == nullptr) or (pNext == pCurrent))
        return;

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::UpdateWarpBubble(): %s(%u) moving from bubble %u to bubble %u.",
                mySE->GetName(), mySE->GetID(), (pCurrent == nullptr ? 0 : pCurrent->GetID()), pNext->GetID());

    if (pCurrent != nullptr)
        pCurrent->Remove(mySE);
    pNext->Add(mySE);
}

/* ======================================================================================
 *  commands.  each applies the client's semantics locally and sends the matching update
 * ====================================================================================== */

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

void DestinyManager::SetSpeedFraction(float fraction/*1.0*/, bool startMovement/*false*/) {
    if (!std::isfinite(fraction))
        return;
    if (fraction < 0.0f) {
        fraction = 0.0f;
    } else if (fraction > 1.0f) {
        fraction = 1.0f;
    }

    // an explicit speed setting replaces the full-speed default after a stop
    m_resetSpeedOnMove = false;

    if (fraction != m_speedFraction) {
        if (is_log_enabled(DESTINY__MOVE_TRACE))
            _log(DESTINY__MOVE_TRACE, "Destiny::SetSpeedFraction() - %s(%u): %.2f -> %.2f", \
                    mySE->GetName(), mySE->GetID(), m_speedFraction, fraction);
        m_speedFraction = fraction;
        CmdSetSpeedFraction du;
            du.entityID = mySE->GetID();
            du.fraction = fraction;
        PyTuple* up = du.Encode();
        SendSingleDestinyUpdate(&up);   // consumed
    }

    // setting a speed while stopped starts the ship moving along its current heading
    if (startMovement and (fraction > 0.0f) and (m_ballMode == Destiny::Ball::Mode::STOP)) {
        GVector heading(m_velocity);
        if (heading.lengthSquared() < 0.01)
            heading = m_shipHeading;
        if (heading.isZero()) {
            heading = GVector(m_position);
            if (heading.isZero())
                heading = GVector(0, 0, 1);
        }
        Normalize(heading);
        GotoDirection(heading);
    }
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

    m_resetSpeedOnMove = true;

    // the client ignores Stop for a ball that is already stopped
    if (m_ballMode == Destiny::Ball::Mode::STOP)
        return;

    if (IsWarping()) {
        // stopping a ball in warp proper would leave it coasting at warp speed
        _log(DESTINY__ERROR, "Destiny::Stop() - %s(%u) ignoring Stop while in warp.", mySE->GetName(), mySE->GetID());
        return;
    }

    ApplyStop();
    m_targBubble = nullptr;
    m_capNeeded = 0;
    m_stateStamp = sEntityList.GetStamp();

    CmdStop du;
        du.entityID = mySE->GetID();
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

void DestinyManager::Halt() {
    //  reset ALL movement variables and states.  calling this will set object to a COMPLETE and IMMEDIATE stop.
    bool wasMoving(IsMoving());

    ApplyStop();
    m_velocity = GVector(NULL_ORIGIN);
    m_thrust = GVector(NULL_ORIGIN);
    m_targetPoint = GPoint(NULL_ORIGIN);
    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;
    m_followRange = 0.0f;
    m_effectStamp = 0;
    m_warpDistance = -1.0;
    m_targBubble = nullptr;
    m_capNeeded = 0;
    m_stateStamp = 0;
    m_resetSpeedOnMove = true;
    m_warpActivated = false;
    m_warpExited = false;
    m_targetLost = false;

    // let the client know, so it stops where we do
    if (wasMoving and (mySE->SysBubble() != nullptr)) {
        std::vector<PyTuple*> updates;
        CmdStop du;
            du.entityID = mySE->GetID();
        updates.push_back(du.Encode());
        SetBallVelocity bv;
            bv.entityID = mySE->GetID();
            bv.x = 0.0;
            bv.y = 0.0;
            bv.z = 0.0;
        updates.push_back(bv.Encode());
        SendDestinyUpdate(updates);
    }

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
//  check for collision.  called by PostEvolve()
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
    Normalize(direction);
    m_velocity = direction * speed;

    SetBallVelocity bv;
        bv.entityID = mySE->GetID();
        bv.x = m_velocity.x;
        bv.y = m_velocity.y;
        bv.z = m_velocity.z;
    PyTuple* up = bv.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
    Stop();
}

//called whenever an entity is going away and can no longer be used as a target
void DestinyManager::EntityRemoved(SystemEntity *pSE) {
    if (m_targetEntity.second == pSE) {
        switch(m_ballMode) {
            case Destiny::Ball::Mode::FOLLOW:
            case Destiny::Ball::Mode::ORBIT: {
                _log(DESTINY__DEBUG, "%u: Our target entity has gone away. Stopping.", mySE->GetID());
                Stop();
            } break;
        }
        m_targetEntity.first = 0;
        m_targetEntity.second = nullptr;
    }
}

bool DestinyManager::IsTargetInvalid()
{
    SystemEntity* pTarget(GetFollowTarget());
    if (pTarget == nullptr) {
        // Our target was removed
        Stop();
        return true;
    }
    if (!pTarget->IsDynamicEntity())
        return false;
    if (pTarget->HasPilot()) {
        if (pTarget->GetPilot()->IsDocked()) {  // Our target docked, so STOP
            Stop();
            return true;
        }
    }
    // the client stops followers itself when the target enters warp (see StopFollowers())
    return false;
}

// common setup for all movement commands
void DestinyManager::BeginMovement() {
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

    if (m_position.isNaN()) {
        _log(DESTINY__ERROR, "%s position is NaN.", mySE->GetName());
    }
    if (m_position.isZero()) {
        _log(DESTINY__ERROR, "%s position is zero.", mySE->GetName());
    }
    if (m_position.isInf()) {
        _log(DESTINY__ERROR, "%s position is inf.", mySE->GetName());
    }

    // this will have to be adjusted for cloak mod.
    if (IsCloaked())
        UnCloak();

    m_stateStamp = sEntityList.GetStamp();

    // after a Stop, the next movement command goes at full speed
    if (m_resetSpeedOnMove or (m_speedFraction <= 0.0f))
        SetSpeedFraction(1.0f);
}

void DestinyManager::Follow(SystemEntity* pSE, uint32 distance) {
    //called from client as 'CmdFollowBall'
    //  also used by 'Approach'
    if (pSE == nullptr)
        return;
    int32 range(static_cast<int32>(distance));
    if ((m_ballMode == Destiny::Ball::Mode::FOLLOW)
    and (m_targetEntity.first == pSE->GetID())
    and (m_followRange == static_cast<float>(range)))
        return;

    BeginMovement();
    ApplyFollow(pSE, static_cast<float>(range), Destiny::Ball::Mode::FOLLOW);

    CmdFollowBall du;
        du.entityID = mySE->GetID();
        du.targetID = pSE->GetID();
        du.range = range;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

void DestinyManager::AlignTo(SystemEntity* ent) {
    Follow(ent, 0);
}

void DestinyManager::GotoDirection(const GPoint& direction) {
    if (!std::isfinite(direction.x) or !std::isfinite(direction.y) or !std::isfinite(direction.z))
        return;

    // the client normalizes the direction and aims at a point very far along it
    GVector dir(direction);
    Normalize(dir);
    GPoint point(m_position.x + dir.x * DST_GOTO_DIRECTION_RANGE,
                 m_position.y + dir.y * DST_GOTO_DIRECTION_RANGE,
                 m_position.z + dir.z * DST_GOTO_DIRECTION_RANGE);

    BeginMovement();
    ApplyGotoPoint(point);

    CmdGotoDirection du;
        du.entityID = mySE->GetID();
        du.x = direction.x;
        du.y = direction.y;
        du.z = direction.z;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

void DestinyManager::GotoPoint(const GPoint& point) {
    BeginMovement();
    ApplyGotoPoint(point);

    CmdGotoPoint gtpoint;
        gtpoint.entityID = mySE->GetID();
        gtpoint.x = point.x;
        gtpoint.y = point.y;
        gtpoint.z = point.z;
    PyTuple* up = gtpoint.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

void DestinyManager::WarpTo(const GPoint& where, int32 distance/*0*/, bool autoPilot/*false*/, SystemEntity* pSE/*nullptr*/) {
    /* warp order..
     * pick destination -> align/accel -> aura "warp drive active" -> cap drain -> accel
     *      -> enter warp -> warp -> decel -> leave warp -> coast -> stop
     *
     * the client does the align, warp and exit itself from the WarpTo update.  we send the destination as-is, with
     *  the warp-to range separately; the client applies the range when it enters warp (see RealWarp())
     */
    if (distance < 0)
        distance = 0;
    GPoint destination(where);
    double minRange(static_cast<double>(distance));

    // check for autopilot.  it has 'special' checks in client for auto-disable by destiny update
    if (autoPilot and (pSE != nullptr))
        Follow(pSE, distance);

    GVector toDestination(m_position, destination);
    double warpDistance(toDestination.length() - minRange);

    if (mySE->HasPilot()) {
        if (warpDistance < static_cast<double>(minWarpDistance)) {
            mySE->GetPilot()->SendErrorMsg("That is too close for your Warp Drive.");
            // warp distance too close.  cancel warp and return
            // we may need to send pos update
            if (sConfig.debug.PositionHack)
                SetPosition(mySE->GetPosition(), true);
            return;
        }

        Client *pClient = mySE->GetPilot();

        /*  capacitor for warp formulas from https://oldforums.eveonline.com/?a=topic&threadID=332116
         *  Energy to warp = warpCapacitorNeed * mass * au * (1 - warp_drive_operation_skill_level * 0.10)
         */
        double currentShipCap = pClient->GetShip()->GetAttribute(AttrCapacitorCharge).get_double();
        double skillMod = (1.0 - (0.1 * pClient->GetChar()->GetSkillLevel(EvESkill::WarpDriveOperation)));
        double capNeeded = m_mass * m_warpCapacitorNeed * (warpDistance / DST_AU) * skillMod;

        _log(DESTINY__WARNING, "Warp Cap need for %s(%u) is %.4f", mySE->GetName(), mySE->GetID(), capNeeded);

        //  check if ship has enough capacitor to warp full distance
        if (capNeeded > currentShipCap) {
            // not enough cap.  warp as far as we can towards the destination
            double possibleAU = currentShipCap / (m_mass * m_warpCapacitorNeed * skillMod);
            if (possibleAU * DST_AU < static_cast<double>(minWarpDistance)) {
                // if not enough cap to do min warp, cancel and return
                pClient->SendErrorMsg("You don't have enough capacitor charge to warp.");
                _log(DESTINY__WARNING, "Destiny::WarpTo() - %s(%u): Capacitor needed vs current  %.3f / %.3f",
                        mySE->GetName(), mySE->GetID(), capNeeded, currentShipCap);
                return;
            }
            toDestination.normalize();
            destination = m_position + (toDestination * (possibleAU * DST_AU));
            minRange = 0.0;
            distance = 0;
            m_capNeeded = 0;
        } else {
            m_capNeeded = currentShipCap - capNeeded;
        }
    }

    /*  TODO PUT CHECK HERE FOR WARP BUBBLES
     *     and other things that affect warp-in point.....when we get to there.
     * AttrWarpBubbleImmune = 1538,
     * AttrWarpBubbleImmuneModifier = 1539,
     *   NOTE:  warp bubble in path (or within 100km of the exit point) will change the warp
     *   however, this does NOT affect original calculations for energy needed, etc...
     */

    BeginMovement();
    // the client needs over 75% of max speed to enter warp
    if (m_speedFraction < 1.0f)
        SetSpeedFraction(1.0f);

    ApplyWarpTo(destination, minRange, GetWarpSpeed());

    // bubble at the exit point.  the exact exit point is fixed when warp proper starts, which is close enough here
    GVector back(destination, m_position);
    back.normalize();
    GPoint exitPoint(destination + (back * minRange));
    m_targBubble = sBubbleMgr.GetBubble(mySE->SystemMgr(), exitPoint);

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::WarpTo() - %s(%u) to %.2f,%.2f,%.2f at range %i (%.4f AU).  target bubble: %u",
            mySE->GetName(), mySE->GetID(), destination.x, destination.y, destination.z, distance,
            warpDistance / DST_AU, (m_targBubble == nullptr ? 0 : m_targBubble->GetID()));

    // send client updates
    std::vector<PyTuple*> updates;
    CmdWarpTo wt;
        wt.entityID = mySE->GetID();
        wt.dest_x = destination.x;
        wt.dest_y = destination.y;
        wt.dest_z = destination.z;
        wt.distance = distance;
        wt.warpSpeed = m_warpFactor; // warp speed x10
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

    if (mySE->IsNPCSE() or mySE->IsDroneSE())
        return;

    //set massive for warp, per client, but self-only
    SetBallMassive bm;
        bm.entityID = mySE->GetID();
        bm.is_massive = false;       // disable client-side bump checks
    PyTuple *up = bm.Encode();
    SendSingleDestinyUpdate(&up, true);   // consumed
}

void DestinyManager::Orbit(SystemEntity *pSE, uint32 distance/*0*/) {
    if (pSE == nullptr)
        return;
    int32 range(static_cast<int32>(distance));
    if ((m_ballMode == Destiny::Ball::Mode::ORBIT)
    and (m_targetEntity.first == pSE->GetID())
    and (m_followRange == static_cast<float>(range)))
        return;

    BeginMovement();
    ApplyFollow(pSE, static_cast<float>(range), Destiny::Ball::Mode::ORBIT);

    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "%s(%u) - orbiting %s(%u) at %i m.  agility:%.3f, inertia:%.3f, maxSpeed:%.2f, radius:%.2f", \
            mySE->GetName(), mySE->GetID(), pSE->GetName(), pSE->GetID(), range, m_shipAgility, m_shipInertia, m_maxShipSpeed, m_radius);

    CmdOrbit du;
        du.entityID = mySE->GetID();
        du.orbitEntityID = pSE->GetID();
        du.distance = range;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
}

bool DestinyManager::IsAligned(GPoint& targetPoint)
{
    GVector heading(m_velocity);
    if (heading.lengthSquared() < 0.01)
        heading = m_shipHeading;
    if (heading.isZero())
        return false;
    Normalize(heading);
    GVector toVec(m_position, targetPoint);
    Normalize(toVec);
    // same tolerance the client uses for warp alignment
    double misalign = 1.0 - toVec.dotProduct(heading);
    return ((misalign < 0.01) and (misalign > -0.01));
}

void DestinyManager::Undock(GPoint dir) {
    //set movement direction
    m_shipHeading = GVector(dir);
    Normalize(m_shipHeading);
    SetUndockSpeed();
    if (mySE->IsShipSE())
        mySE->GetShipSE()->GetShipItemRef()->SetUndocking(false);
}

void DestinyManager::SetUndockSpeed() {
    //start ship movement @ max velocity along m_shipHeading.  this simulates being forcefully "ejected" from station
    m_velocity = m_shipHeading * m_maxShipSpeed;

    if (m_ballMode == Destiny::Ball::Mode::MISSILE) {
        m_speedFraction = 1.0f;
        return;
    }

    SetSpeedFraction(1.0f);
    SetBallVelocity bv;
        bv.entityID = mySE->GetID();
        bv.x = m_velocity.x;
        bv.y = m_velocity.y;
        bv.z = m_velocity.z;
    PyTuple* up = bv.Encode();
    SendSingleDestinyUpdate(&up);   // consumed
    GotoDirection(m_shipHeading);
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

    float oldSpeed(m_maxShipSpeed);
    if (maxVelocity > maxSpeed) {
        m_maxShipSpeed = maxSpeed;
    } else {
        m_maxShipSpeed = maxVelocity;
    }

    // the client simulates with this speed too, so keep it informed once we are in space
    if ((oldSpeed != m_maxShipSpeed) and (mySE->SysBubble() != nullptr)) {
        SetBallSpeed sbms;
            sbms.entityID = mySE->GetID();
            sbms.speed = m_maxShipSpeed;
        PyTuple* up = sbms.Encode();
        SendSingleDestinyUpdate(&up);   // consumed
    }
}

void DestinyManager::SpeedBoost(bool deactivate/*false*/)
{
    // prop mod state changed.  reset ship movement variables.  the client carries on with its current
    //  movement using the new values, as do we.
    m_mass = mySE->GetSelf()->GetAttribute(AttrMass).get_float();
    m_massMKg = m_mass / 1000000; //changes mass from Kg to MillionKg (10^-6)
    m_shipAgility = m_massMKg * m_shipInertia;
    m_alignTime = (-log(0.25) * m_shipAgility);
    m_shipMaxAccelTime = (-log(0.0001) * m_shipAgility);
    m_timeToEnterWarp = m_alignTime;
    m_maxShipSpeed = mySE->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();

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

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost(%s) - mass: %.5f, agility: %.5f, maxSpeed: %.2f, speed: %.2f, sf: %.2f", \
                (deactivate ? "deactivate" : "activate"), m_mass, m_shipAgility, m_maxShipSpeed, m_velocity.length(), m_speedFraction);
}

void DestinyManager::WebbedMe(InventoryItemRef modRef, bool apply/*false*/)
{
    if (apply) {
        m_maxShipSpeed *= (1 + (modRef->GetAttribute(AttrSpeedFactor).get_float() / 100.0f));
    } else {
        m_maxShipSpeed /= (1 + (modRef->GetAttribute(AttrSpeedFactor).get_float() / 100.0f));
    }
    std::vector<PyTuple*> updates;
    SetBallSpeed sbms;
        sbms.entityID = mySE->GetID();
        sbms.speed = m_maxShipSpeed;
        updates.push_back(sbms.Encode());
    SendDestinyUpdate(updates);
    m_hasSentShipUpdates = true;    // just in case, as this is re-sent in BeginMovement()
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

    /* The product of Mass and the Inertia Modifier gives the ship's agility
     * Agility = Mass x Inertia Modifier
     *  agility is an internal-use variable.
     */
    m_shipAgility = m_massMKg * m_shipInertia;
    // set a maximum acceleration time (based on ship variables)
    m_shipMaxAccelTime = (-log(0.0001) * m_shipAgility);

    //  both of these formulas have identical products
    //TimeToWarp = -ln(0.25) x Mass Mkg x Inertia Mod
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

    m_ballMode = Destiny::Ball::Mode::MISSILE;
    m_stateStamp = sEntityList.GetStamp();

    SystemEntity* pTarget = pMissile->GetTargetSE();
    m_targetPoint = GPoint(pTarget->GetPosition());
    m_targetEntity.first = pTarget->GetID();
    m_targetEntity.second = pTarget;
    m_followRange = static_cast<float>(m_position.distance(m_targetPoint));

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
    // the tractored ball is made light and free, then follows the tractoring ship.  the client's follow
    //  physics pull it in and hold it at range, so we mirror the same updates here
    m_tractored = true;
    m_stateStamp = sEntityList.GetStamp();

    m_preTractorMass = m_mass;
    m_mass = 10000;
    m_maxShipSpeed = speed.get_float();   //AttrMaxTractorVelocity
    m_speedFraction = 1.0f;
    m_resetSpeedOnMove = false;
    int32 range(static_cast<int32>(500 + pShipSE->GetRadius()));
    ApplyFollow(pShipSE, static_cast<float>(range), Destiny::Ball::Mode::FOLLOW);

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
        sbmass.mass = m_mass;
    updates.push_back(sbmass.Encode());
    CmdSetSpeedFraction ssf;
        ssf.entityID = mySE->GetID();
        ssf.fraction = m_speedFraction;
    updates.push_back(ssf.Encode());
    CmdFollowBall fb;
        fb.entityID = mySE->GetID();
        fb.targetID = pShipSE->GetID();
        fb.range = range;
    updates.push_back(fb.Encode());
    SendDestinyUpdate(updates);
}

void DestinyManager::TractorBeamStop()
{
    // SetBallFree(false) stops the ball dead on the client
    ApplyStop();
    m_velocity = GVector(NULL_ORIGIN);
    m_thrust = GVector(NULL_ORIGIN);
    m_tractored = false;
    m_mass = m_preTractorMass;
    m_maxShipSpeed = 0;
    m_resetSpeedOnMove = true;

    std::vector<PyTuple*> updates;
    SetBallSpeed ms;
        ms.entityID = mySE->GetID();
        ms.speed = m_maxShipSpeed;
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

void DestinyManager::SendJumpOutEffect(std::string JumpEffect, uint32 shipID) {
    // the CmdStop below applies to our ball too
    ApplyStop();
    m_resetSpeedOnMove = true;
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

void DestinyManager::SendJumpInEffect(std::string JumpEffect) {
    // the speed fraction and velocity below apply to our ball too
    m_speedFraction = 0.0f;
    m_velocity = GVector(NULL_ORIGIN);
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

// writes the DataSector and the mode-specific part of this ball, as the client's ReadBallFromStream expects them.
//  the ball header and MassSector are written by the entity
void DestinyManager::EncodeDestiny(Buffer& into) const {
    using namespace Destiny;
    DataSector data = DataSector();
        data.maxSpeed = m_maxShipSpeed;
        data.velX = m_velocity.x;
        data.velY = m_velocity.y;
        data.velZ = m_velocity.z;
        data.inertia = static_cast<float>(m_shipInertia);
        data.speedfraction = m_speedFraction;
    into.Append( data );

    switch (m_ballMode) {
        case Ball::Mode::WARP: {
            WARP_Struct warp;
                warp.formationID = 0xFF;
                warp.targX = m_targetPoint.x;
                warp.targY = m_targetPoint.y;
                warp.targZ = m_targetPoint.z;
                warp.effectStamp = m_effectStamp;
                warp.totalDistance = m_warpDistance;
                warp.minRange = m_warpMinRange;
                warp.warpFactor = m_warpFactor;
            into.Append( warp );
        }  break;
        case Ball::Mode::FOLLOW: {
            FOLLOW_Struct follow;
                follow.formationID = 0xFF;
                follow.followID = m_targetEntity.first;
                follow.followRange = m_followRange;
            into.Append( follow );
        }  break;
        case Ball::Mode::ORBIT: {
            ORBIT_Struct orbit;
                orbit.formationID = 0xFF;
                orbit.followID = m_targetEntity.first;
                orbit.followRange = m_followRange;
            into.Append( orbit );
        }  break;
        case Ball::Mode::GOTO: {
            GOTO_Struct go;
                go.formationID = 0xFF;
                go.x = m_targetPoint.x;
                go.y = m_targetPoint.y;
                go.z = m_targetPoint.z;
            into.Append( go );
        }  break;
        default: {
            // missiles are encoded by Missile::EncodeDestiny()
            STOP_Struct main;
                main.formationID = 0xFF;
            into.Append( main );
        } break;
    }
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
             if (value->GetShipSE() != nullptr) {
                value->GetShipSE()->SysBubble()->BubblecastDestiny(updates, events, "destiny");
            }
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
