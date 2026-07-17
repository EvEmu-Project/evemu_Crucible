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
    Physics:    Updated to match official EVE Online client physics
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
#include <cstdlib>

// ===== КОНСТАНТЫ ФИЗИКИ ИЗ ОФИЦИАЛЬНОГО КОДА =====
#define _HALFPI_  1.57079632679489655
#define _PI_      3.1415926535897931
#define _2PI_     6.2831853071795862

// Коэффициент трения (аналог mPark->mFriction из официального кода)
#define PHYSICS_FRICTION 0.5
// Фактор времени (обычно 1.0)
#define PHYSICS_TIME_FACTOR 1.0
// Максимальный угол поворота от удара
#define MAX_IMPACT_ANGLE 0.25f
// Минимальная угловая скорость
#define MIN_ANGULAR_VELOCITY 0.01f
// ===== КОНЕЦ КОНСТАНТ =====

DestinyManager::DestinyManager(SystemEntity *self)
: mySE(self),
m_maxSpeed(1.0),
m_shipAccelTime(0.0),
m_shipMaxAccelTime(0.0),
m_ballMode(Destiny::Ball::Mode::STOP),
m_warpTimer(0),
m_moveTime(0.0),
m_targetDistance(0),
m_followDistance(0),
m_stopDistance(0),
m_mass(1.0),
m_turnTic(1),
m_massMKg(1.0),
m_alignTime(1.0),
m_timeToEnterWarp(10.0),
m_shipWarpSpeed(1.0),
m_maxShipSpeed(100.0),
m_shipAgility(1.0),
m_shipInertia(1.0),
m_warpAccelTime(1),
m_warpDecelTime(1),
m_warpState(nullptr),
m_targBubble(nullptr),
m_warpCapacitorNeed(0.00001),
m_frozen(false),
m_ticAlign(false),
mvPacket(nullptr),
m_speedChangeTime(0.0),
m_speedAtChange(0.0),
m_targetSpeed(0.0),
m_timeSinceChange(0.0),
// ===== НОВЫЕ ПЕРЕМЕННЫЕ ДЛЯ ФИЗИКИ =====
m_yaw(0.0),
m_pitch(0.0),
m_roll(0.0),
m_oldYaw(0.0),
m_oldPitch(0.0),
m_oldRoll(0.0),
m_yawSpeed(0.0),
m_pitchSpeed(0.0),
m_rollSpeed(0.0),
m_oldRollSpeed(0.0),
m_yawDelta(0.0),
m_massModifier(100.0),
m_physicsFriction(PHYSICS_FRICTION),
m_timeFactor(PHYSICS_TIME_FACTOR)
// ===== КОНЕЦ НОВЫХ ПЕРЕМЕННЫХ =====
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
    m_prevSpeed = 0.0;
    m_degPerTic = 0.0;
    m_orbitTime = 0.0;
    m_orbitRadTic = 0.0;
    m_timeFraction = 0.0;
    m_prevSpeedFraction = 0.0;
    m_userSpeedFraction = 0.0;
    m_activeSpeedFraction = 0.0;
    m_maxOrbitSpeedFraction = 1.0;

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

void DestinyManager::Process() {
    double profileStartTime(GetTimeUSeconds());

    if (mySE->IsFrozen()) {
        Halt();
        return;
    }

    if (m_ticAlign) {
        m_ticAlign = false;
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
            GVector moveVector(m_position, m_targetPoint);
            moveVector.normalize();
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
            if (m_warpState != nullptr) {
                uint16 sec_into_warp = (sEntityList.GetStamp() - m_stateStamp);
                if (m_warpState->accel) {
                    WarpAccel(sec_into_warp);
                } else if (m_warpState->cruise) {
                    WarpCruise(sec_into_warp);
                } else if (m_warpState->decel) {
                    WarpDecel(sec_into_warp);
                } else {
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

            GVector toVec(m_position, m_targetPoint);
            toVec.normalize();
            double dot = toVec.dotProduct(m_shipHeading);
            double degrees = EvE::Trig::Rad2Deg(std::acos(dot));

            if ((degrees < WARP_ALIGNMENT) and (m_timeFraction > 0.749)) {
                m_shipHeading = toVec;
                InitWarp();
                return;
            } else if (m_timeFraction < 0.749 && m_userSpeedFraction < 0.7499) {
                SetSpeedFraction(1.0, true);
            } else if ((sEntityList.GetStamp() - m_stateStamp) > m_timeToEnterWarp + 0.3) {
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
        case Ball::Mode::MUSHROOM:
        case Ball::Mode::BOID:
        case Ball::Mode::TROLL:
        case Ball::Mode::MINIBALL:
        case Ball::Mode::FIELD:
        case Ball::Mode::FORMATION:
        case Ball::Mode::RIGID:
            break;
    }
}

// ===== НОВАЯ ФУНКЦИЯ: Расчет ориентации по официальной формуле =====
void DestinyManager::CalculateYawPitchRoll(bool snap) {
    // Переносим старые значения
    m_oldPitch = m_pitch;
    m_oldYaw = m_yaw;
    m_oldRoll = m_roll;
    m_oldRollSpeed = m_rollSpeed;

    // Массовый модификатор для поворота
    if (m_mass != 0.0) {
        m_massModifier = 100.0 / (m_mass / m_physicsFriction);
    } else {
        m_massModifier = 100.0;
    }

    // Направление движения - ИСПРАВЛЕНО: используем length() вместо LengthSq()
    GVector direction;
    double vel2 = m_velocity.length() * m_velocity.length(); // квадрат длины
    
    if (vel2 == 0.0) {
        direction = m_targetPoint - m_position;
    } else {
        direction = m_velocity;
        // Добавляем небольшое смещение к точке назначения (как в официальном коде)
        GVector offset = m_targetPoint - m_position;
        offset.normalize();
        direction = direction + sqrt(vel2) * 0.3 * offset;
    }

    // Расчет yaw и pitch - ИСПРАВЛЕНО: используем length()
    if (direction.length() > 0.0) {
        m_yaw = atan2(direction.x, direction.z);
        double r = direction.length();
        if (r > 0.0) {
            double pitchVal = -std::asin(std::min(1.0, std::max(-1.0, direction.y / r)));
            m_pitch = pitchVal;
        } else {
            m_pitch = 0.0;
        }
    } else {
        m_yaw = 0.0;
        m_pitch = 0.0;
    }

    // Модуль для углов (приведение к диапазону)
    Modulo2pi(m_oldYaw, m_yaw);
    Modulo2pi(m_oldPitch, m_pitch);

    if (snap) {
        m_roll = 0.0;
        m_oldRoll = 0.0;
        m_rollSpeed = 0.0;
        m_oldRollSpeed = 0.0;
        m_yawDelta = 0.0;
        return;
    }

    // Демпфирование yaw и pitch с учетом массы
    double tmp = 1.0 / (1.0 + m_massModifier * 0.5);
    m_yaw = (m_oldYaw + m_massModifier * 0.5 * m_yaw) * tmp;
    m_pitch = (m_oldPitch + m_massModifier * 0.5 * m_pitch) * tmp;

    // Расчет изменения yaw
    m_yawDelta = asin(sin(m_yaw - m_oldYaw));

    // Расчет roll (вращение вокруг оси)
    // Roll speed увеличивается от изменений yaw
    m_rollSpeed += m_yawDelta * 0.5; // mPark->mRollSpeedAcceleration

    // Roll speed затухает со временем
    m_rollSpeed += m_oldRollSpeed * 0.9 * 0.1; // mPark->mRollSpeedDecay * dt

    // Clamp roll speed
    m_rollSpeed = std::max(-_HALFPI_, std::min(_HALFPI_, m_rollSpeed));

    // Roll увеличивается от roll speed
    m_roll += m_rollSpeed * 0.5 * 0.1; // mPark->mRollAcceleration * dt

    // Roll затухает со временем
    m_roll += m_oldRoll * 0.9 * 0.1; // mPark->mRollDecay * dt

    // Clamp roll
    m_roll = std::max(-_HALFPI_, std::min(_HALFPI_, m_roll));
}

// ===== ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ: Модуль углов =====
void DestinyManager::Modulo2pi(double& a, double& b) {
    b = fmod(b, _2PI_);
    a = fmod(a, _2PI_);

    if (fabs(b - a) < fabs(b + _2PI_ - a)) {
        if (fabs(b - a) < fabs(b - _2PI_ - a)) {
            return;
        } else {
            b = b - _2PI_;
            return;
        }
    } else {
        if (fabs(b + _2PI_ - a) < fabs(b - _2PI_ - a)) {
            b = b + _2PI_;
            return;
        } else {
            b = b - _2PI_;
            return;
        }
    }
}
// ===== КОНЕЦ НОВЫХ ФУНКЦИЙ =====

void DestinyManager::SetSpeedFraction(double fraction, bool startMovement) {
    if (m_orbiting != 0)
        Orbit(m_targetEntity.second, m_targetDistance);

    if ((fraction == m_userSpeedFraction) and (!startMovement)) {
        return;
    }

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::SetSpeedFraction() - %s(%u):   prevSpeed:%.2f, fraction: %.2f, start: %s, stop: %s,accel: %s, decel: %s",
             mySE->GetName(), mySE->GetID(), m_prevSpeed, fraction, startMovement ? "true" : "false", m_stop ? "true" : "false", \
             m_accel ? "true" : "false", m_decel ? "true": "false");

    if (startMovement) {
        m_stop = false;
        if (m_ballMode == Destiny::Ball::Mode::STOP)
            m_ballMode = Destiny::Ball::Mode::GOTO;
    }

    if (m_stop)
        return;

    m_prevSpeedFraction = 0.0;

    // Устанавливаем целевую скорость
    m_targetSpeed = fraction * m_maxShipSpeed;
    
    // Если корабль стоит и мы даём команду на движение
    if (m_activeSpeedFraction < 0.001 && fraction > 0.001) {
        m_speedAtChange = 0.0;
        m_speedChangeTime = GetTimeMSeconds();
        m_timeSinceChange = 0.0;
        m_activeSpeedFraction = fraction;
    }

    if (m_activeSpeedFraction > 0.01) {
        m_userSpeedFraction = fraction;
        m_prevSpeedFraction = m_activeSpeedFraction;
        UpdateVelocity(true);
    } else {
        m_userSpeedFraction = fraction;
        UpdateVelocity(false);
    }

    if (m_ballMode == Destiny::Ball::Mode::WARP) {
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
        SetBallSpeed ms;
            ms.entityID = mySE->GetID();
            ms.speed = m_maxSpeed;
        updates.push_back(ms.Encode());
        m_hasSentShipUpdates = true;
    }

    if (!updates.empty())
        SendDestinyUpdate(updates);
}

// ===== НОВАЯ ВЕРСИЯ UpdateVelocity с официальной физикой =====
void DestinyManager::UpdateVelocity(bool isMoving) {
    if ((m_ballMode == Destiny::Ball::Mode::WARP) and (m_warpState != nullptr)) {
        m_accel = false;
        m_decel = true;
        m_targBubble = nullptr;
        m_maxSpeed = m_speedToLeaveWarp;
        m_prevSpeed = m_speedToLeaveWarp;
        m_velocity = m_shipHeading * m_maxSpeed;
        m_prevSpeedFraction = m_maxSpeed / m_maxShipSpeed;
        
        // Время ускорения по официальной формуле
        double targetFraction = m_prevSpeedFraction;
        if (targetFraction > 0.99) targetFraction = 0.99;
        m_shipAccelTime = -log(1.0 - targetFraction) * m_shipAgility;
        
        m_speedAtChange = m_prevSpeed;
        m_targetSpeed = 0.0;
        m_speedChangeTime = GetTimeMSeconds();
        m_timeSinceChange = 0.0;
        return;
    }
    
    double currentSpeed = m_activeSpeedFraction * m_maxShipSpeed;
    double targetSpeed = m_userSpeedFraction * m_maxShipSpeed;
    
    double speedDiff = fabs(targetSpeed - currentSpeed);
    if (speedDiff < 0.001) {
        m_accel = false;
        m_decel = false;
        m_timeFraction = 1.0;
        return;
    }
    
    // Запоминаем состояние
    m_speedAtChange = currentSpeed;
    m_targetSpeed = targetSpeed;
    m_speedChangeTime = GetTimeMSeconds();
    m_timeSinceChange = 0.0;
    
    if (targetSpeed > currentSpeed) {
        m_accel = true;
        m_decel = false;
    } else {
        m_accel = false;
        m_decel = true;
    }
    
    // Защита от деления на ноль
    double agility = m_shipAgility;
    if (agility < 0.001) {
        agility = 1.0;
        _log(DESTINY__WARNING, "UpdateVelocity: agility too small, using 1.0");
    }
    
    // Время ускорения по официальной формуле: t = -τ * ln(1 - v/V_max)
    double speedRatio = speedDiff / m_maxShipSpeed;
    if (speedRatio > 0.99) speedRatio = 0.99;
    m_shipAccelTime = -log(1.0 - speedRatio) * agility;
    
    m_prevSpeed = currentSpeed;
    m_prevSpeedFraction = m_activeSpeedFraction;
    m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::UpdateVelocity - %s(%u): Speed Change - USF: %.2f, ASF: %.2f, pSpeed: %.2f, mSpeed: %.2f, accel: %s, accelTime: %.3f", \
            mySE->GetName(), mySE->GetID(), m_userSpeedFraction, m_activeSpeedFraction, m_prevSpeed, m_maxSpeed, \
            m_accel ? "true" : "false", m_shipAccelTime);
}
// ===== КОНЕЦ НОВОЙ ВЕРСИИ =====

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
    if (AbortIfLoginWarping(false)) {
        return;
    }

    if (mySE->HasPilot()) {
        mySE->GetPilot()->SetAutoPilot(false);
    }

    if (m_userSpeedFraction == 0.0) {
        m_stop = true;
    } else if  ((m_ballMode == Destiny::Ball::Mode::WARP) and (!IsWarping()))  {
        m_ballMode = Destiny::Ball::Mode::STOP;
    } else if (IsMoving()) {
        m_ballMode = Destiny::Ball::Mode::STOP;
    }

    m_accel = false;
    m_decel = false;
    m_prevSpeed = 0.0;
    m_prevSpeedFraction = 0.0;

    ClearTurn();
    ClearOrbit();

    m_stateStamp = sEntityList.GetStamp();

    SetSpeedFraction(0.0);
    m_stop = true;

    CmdStop du;
        du.entityID = mySE->GetID();
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void DestinyManager::Halt() {
    SafeDelete(m_warpState);

    m_ballMode = Destiny::Ball::Mode::STOP;
    m_stop = true;
    m_accel = false;
    m_decel = false;
    m_turning = false;
    m_maxSpeed = 0.0;
    m_velocity = GVector(NULL_ORIGIN);
    m_moveTime = 0.0;
    m_prevSpeed = 0.0;
    m_stateStamp = 0;
    m_targetPoint = GPoint(NULL_ORIGIN);
    m_stopDistance = 0;
    m_targetDistance = 0;
    m_followDistance = 0;
    m_prevSpeedFraction = 0.0;
    m_userSpeedFraction = 0.0;
    m_activeSpeedFraction = 0.0;
    m_timeFraction = 0.0;
    m_maxOrbitSpeedFraction = 1.0;
    m_targetSpeed = 0.0;
    m_speedAtChange = 0.0;
    m_timeSinceChange = 0.0;
    m_speedChangeTime = 0.0;

    // Сброс углов
    m_yaw = 0.0;
    m_pitch = 0.0;
    m_roll = 0.0;
    m_oldYaw = 0.0;
    m_oldPitch = 0.0;
    m_oldRoll = 0.0;
    m_yawSpeed = 0.0;
    m_pitchSpeed = 0.0;
    m_rollSpeed = 0.0;
    m_oldRollSpeed = 0.0;
    m_yawDelta = 0.0;

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;

    ClearTurn();

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::Halt() - %s(%u): m_shipHeading: %.3f,%.3f,%.3f", \
                mySE->GetName(), mySE->GetID(), m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
}

void DestinyManager::Eject()
{
    Stop();
    UpdateOldShip(mySE->GetShipSE());
    SendJettisonPacket();
}

void DestinyManager::CheckBump()
{
    double profileStartTime(GetTimeUSeconds());

    std::vector<Client*> vPlayers;
    mySE->SysBubble()->GetPlayers(vPlayers);
    Client* pClient = mySE->GetPilot();
    GPoint pos(GetPosition());
    double distance = 0.0;
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

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::collision, GetTimeUSeconds() - profileStartTime);
}

void DestinyManager::Bump(SystemEntity* pSE)
{
    if (m_bump)
        return;
    std::string msg1 = "You have bumped ";
    msg1 += pSE->GetPilot()->GetName();
    mySE->GetPilot()->SendNotifyMsg(msg1.c_str());
    if (pSE->HasPilot()) {
        std::string msg2 = "You have been bumped by ";
        msg2 += mySE->GetPilot()->GetName();
        pSE->GetPilot()->SendNotifyMsg(msg2.c_str());
    }
}

void DestinyManager::Bounce(GVector direction, double speed) {
    m_ballMode = Destiny::Ball::Mode::GOTO;
    m_stop = false;
    m_stateStamp = sEntityList.GetStamp();
    m_moveTime = GetTimeMSeconds();
    m_shipAccelTime = 0.1;
    m_userSpeedFraction = 1.0;
    m_timeFraction = 1.0;
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

// ===== НОВАЯ ВЕРСИЯ MoveObject с официальной физикой =====
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
        m_moveTime = GetTimeMSeconds();
        _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - ChangeDelay - %s(%u): stateStamp: %u", \
                    mySE->GetName(), mySE->GetID(), m_stateStamp);
        return;
    }

    // Поворот с использованием официальной физики
    if (m_userSpeedFraction > 0.001) {
        if (!m_orbiting or (m_orbiting > Destiny::Ball::Orbit::Far))
            Turn();
    }

    // ===== ФИЗИКА ДВИЖЕНИЯ ПО ОФИЦИАЛЬНОЙ ФОРМУЛЕ =====
    double currentTime = GetTimeMSeconds();
    
    if (m_speedChangeTime == 0.0 && m_userSpeedFraction > 0.001) {
        m_speedChangeTime = currentTime;
        m_speedAtChange = m_activeSpeedFraction * m_maxShipSpeed;
        m_targetSpeed = m_userSpeedFraction * m_maxShipSpeed;
        m_timeSinceChange = 0.0;
    }
    
    double dt = (currentTime - m_speedChangeTime) * 0.001;
    if (dt < 0) dt = 0;
    
    m_timeSinceChange += dt;
    if (m_timeSinceChange > 60.0) m_timeSinceChange = 60.0;
    
    double currentSpeed = 0.0;
    
    if (m_userSpeedFraction > 0.001) {
        // Постоянная времени = масса * инерция (как в официальном коде)
        double tau = m_shipAgility;
        if (tau < 0.001) tau = 1.0;
        
        // Официальная формула: v(t) = V_target + (V_start - V_target) * exp(-t/τ)
        double factor = exp(-m_timeSinceChange / tau);
        currentSpeed = m_targetSpeed + (m_speedAtChange - m_targetSpeed) * factor;
        
        // Проверка на достижение целевой скорости
        double speedDiff = fabs(currentSpeed - m_targetSpeed);
        if (speedDiff < 0.001 || m_timeSinceChange > 60.0) {
            currentSpeed = m_targetSpeed;
            m_activeSpeedFraction = m_userSpeedFraction;
            m_accel = false;
            m_decel = false;
            m_timeFraction = 1.0;
        } else {
            if (m_maxShipSpeed > 0.001) {
                m_activeSpeedFraction = currentSpeed / m_maxShipSpeed;
            }
            m_timeFraction = 1.0 - factor;
        }
    } else {
        // Остановка: экспоненциальное замедление
        double tau = m_shipAgility;
        if (tau < 0.001) tau = 1.0;
        
        double v0 = m_speedAtChange;
        if (v0 < 0.01) v0 = m_activeSpeedFraction * m_maxShipSpeed;
        
        currentSpeed = v0 * exp(-m_timeSinceChange / tau);
        if (m_maxShipSpeed > 0.001) {
            m_activeSpeedFraction = currentSpeed / m_maxShipSpeed;
        }
        m_timeFraction = 1.0 - exp(-m_timeSinceChange / tau);
        
        if (currentSpeed < 0.001) {
            currentSpeed = 0.0;
            m_activeSpeedFraction = 0.0;
            m_timeFraction = 0.0;
        }
    }
    // ===== КОНЕЦ ФИЗИКИ =====

    // Коррекция для орбиты
    if (m_orbiting && m_orbiting < Destiny::Ball::Orbit::TooClose) {
        currentSpeed *= m_maxOrbitSpeedFraction;
    }

    // Отладка
    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) speed: %.2f m/s (%.1f%%) tf:%.4f asf:%.4f dt:%.3f agility:%.3f", \
            mySE->GetName(), mySE->GetID(), currentSpeed, m_activeSpeedFraction*100, m_timeFraction, m_activeSpeedFraction, dt, m_shipAgility);
    }

    // Нормализация heading
    if (mySE->HasPilot()) {
        double len = sqrt(m_shipHeading.x*m_shipHeading.x + 
                          m_shipHeading.y*m_shipHeading.y + 
                          m_shipHeading.z*m_shipHeading.z);
        if (len < 0.99 || len > 1.01) {
            if (len > 1e-6) {
                m_shipHeading.x /= len;
                m_shipHeading.y /= len;
                m_shipHeading.z /= len;
            }
        }
    }

    // Обновление позиции
    m_velocity = m_shipHeading * currentSpeed;
    SetPosition(m_position + m_velocity, sConfig.debug.PositionHack);

    // Отправка обновлений скорости - ИСПРАВЛЕНО: используем length()
    if ((m_stateStamp > 0) and (m_velocity.length() > 0.0)) {
        if (mvPacket != nullptr) {
            PyDecRef(mvPacket);
            mvPacket = nullptr;
        }
        SetBallVelocity sbv;
            sbv.entityID = mySE->GetID();
            sbv.x = m_velocity.x;
            sbv.y = m_velocity.y;
            sbv.z = m_velocity.z;
        mvPacket = sbv.Encode();
    }

    // Вращение при рывке
    if (m_bump) {
        if (!is_log_enabled(DESTINY__MOVE_TRACE)) {
            _log(DESTINY__MOVE_TRACE, "Bump at %.2f", m_position.distance(m_targetEntity.second->GetPosition()));
        }
        m_bump = false;
    }

    // Проверка коллизий с другими сущностями
    if (mySE->HasPilot())
        CheckBump();
}
// ===== КОНЕЦ НОВОЙ ВЕРСИИ =====

bool DestinyManager::IsTurn() {
    if (m_targetPoint.isZero()) {
        _log(DESTINY__ERROR, "Destiny::IsTurn() - %s(%u): TargetPoint is null.", mySE->GetName(), mySE->GetID());
        if (mySE->HasPilot())
            mySE->GetPilot()->SendNotifyMsg("There was an error in your ship's navigation computer.  Ref: ServerError 35221");
        ClearTurn();
        Halt();
        return false;
    }
    if ((m_timeFraction < 0.1) and (m_activeSpeedFraction < 0.1)) {
        GVector toVec(m_position, m_targetPoint);
        toVec.normalize();
        m_shipHeading = toVec;
        return false;
    }

    GVector toVec(m_position, m_targetPoint);
    toVec.normalize();
    double dot(toVec.dotProduct(m_shipHeading));
    if ((dot > 1.0) or (dot < -1.0)) {
        sLog.Error("Destiny::IsTurn()", "%s(%u) - shipHeading has screwed up.  dot is %.5f", mySE->GetName(), mySE->GetID(), dot);
        _log(DESTINY__ERROR, "Destiny::IsTurn() m_shipHeading: %.3f,%.3f,%.3f.  m_targetHeading: %.3f,%.3f,%.3f, toVec:%.3f,%.3f,%.3f", \
                m_shipHeading.x, m_shipHeading.y, m_shipHeading.z, m_targetHeading.x, m_targetHeading.y, m_targetHeading.z, toVec.x, toVec.y, toVec.z);
             if (m_shipHeading.x > 1.0)  { m_shipHeading.x -= 1; }
        else if (m_shipHeading.x < 1.0)  { m_shipHeading.x += 1; }
             if (m_shipHeading.y > 1.0)  { m_shipHeading.y -= 1; }
        else if (m_shipHeading.y < 1.0)  { m_shipHeading.y += 1; }
             if (m_shipHeading.z > 1.0)  { m_shipHeading.z -= 1; }
        else if (m_shipHeading.z < 1.0)  { m_shipHeading.z += 1; }
        dot = toVec.dotProduct(m_shipHeading);
        if ((dot > 1.0) or (dot < -1.0)) {
            sLog.Error("Destiny::IsTurn()", "%s(%u) - shipHeading has screwed up AGAIN.  dot is %.5f", mySE->GetName(), mySE->GetID(), dot);
            return false;
        }
    }
    m_radians = std::acos(dot);
    double degrees(EvE::Trig::Rad2Deg(m_radians));
    if (degrees < TURN_ALIGNMENT) {
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

// ===== НОВАЯ ВЕРСИЯ Turn с официальной физикой поворота =====
void DestinyManager::Turn() {
    if (mySE->HasPilot())
        if (mySE->GetPilot()->IsUndock())
            return;

    ++m_turnTic;

    if (!IsTurn()) {
        if (m_turning)
            ClearTurn();
        return;
    }

    // Расчет массового модификатора для поворота (как в официальном коде)
    double massModifier = m_mass / m_physicsFriction;
    if (massModifier != 0.0)
        massModifier = 100.0 / massModifier;
    else
        massModifier = 100.0;

    // Скорость поворота зависит от массы
    double rotationSpeed = m_degPerTic * (1.0 / (1.0 + massModifier * 0.5));

    double turnTime(m_shipAgility / 2.2);
    if (!m_turning) {
        m_turning = true;
        m_turnFraction = std::sqrt((std::cos(m_radians) + 1) / 2);
        m_alignTime = (EvE::Trig::Rad2Deg(m_radians) / m_degPerTic);
        if (is_log_enabled(DESTINY__TURN_TRACE))
            _log(DESTINY__TURN_TRACE, "Destiny::Turn() - %s(%u): Agility:%.3f, Inertia:%.3f, alignTime:%.3f, turnTime:%.3f, turnFraction:%.3f, m_degPerTic:%.3f, rotationSpeed:%.3f", \
                mySE->GetName(), mySE->GetID(), m_shipAgility, m_shipInertia, m_alignTime, turnTime, m_turnFraction, m_degPerTic, rotationSpeed);
    }

    if (m_turnTic == 1)
        if (m_turnFraction < m_timeFraction)
            UpdateVelocity(true);

    GVector deltaHeading(m_shipHeading, m_targetHeading);
    
    // Используем rotationSpeed вместо m_degPerTic
    double turnPercent(0.1);
    double degrees(EvE::Trig::Rad2Deg(m_radians));
    if (degrees > 100) {
        if (m_decel and (m_turnTic > turnTime)) {
            turnPercent = 0.3;
        } else {
            turnPercent = rotationSpeed / (degrees - 100);
        }
    } else if (degrees > rotationSpeed) {
        turnPercent = rotationSpeed / (degrees * 0.5);
    } else {
        if (m_decel)
            UpdateVelocity(true);
    }

    if (turnPercent > 0.9) {
        _log(DESTINY__ERROR, "Destiny::Turn() - turnTic:%u, degRemain:%.3f, turnPercent:%.2f", m_turnTic, degrees, turnPercent);
        turnPercent = 0.9;
    }
    deltaHeading *= turnPercent;
    m_shipHeading += deltaHeading;
    m_shipHeading.normalize();
    if (is_log_enabled(DESTINY__TURN_TRACE))
        _log(DESTINY__TURN_TRACE, "Destiny::Turn() - tf:%.3f, turnTic:%u, degRemain:%.3f  (deltaHeading:%.5f, %.5f, %.5f * turnPercent:%.2f) = shipHeading:%.3f, %.3f, %.3f", \
            m_timeFraction, m_turnTic, degrees, deltaHeading.x, deltaHeading.y, deltaHeading.z, turnPercent, m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
}
// ===== КОНЕЦ НОВОЙ ВЕРСИИ =====

void DestinyManager::ClearTurn() {
    SetPosition(m_position, sConfig.debug.PositionHack);
    m_turnTic = 0;
    m_turning = false;
    m_radians = 0.0;
    m_turnFraction = 0.0;
    m_alignTime = m_timeToEnterWarp;
}

void DestinyManager::Follow() {
    const GPoint& target_point = m_targetEntity.second->GetPosition();
    GVector heading(m_position, target_point);
    m_targetDistance = heading.length() - m_radius;

    if (m_targetDistance < m_followDistance) {
        if (mySE->HasPilot())
            if (mySE->GetPilot()->IsAutoPilot()) {
                SetSpeedFraction(0.1);
                _log(AUTOPILOT__TRACE, "DestinyManager::Follow() - Target within FollowDistance.  SpeedFraction = 0.1.");
                return;
            }
        if (m_tractored) {
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
            m_activeSpeedFraction = m_userSpeedFraction = m_timeFraction = m_prevSpeedFraction = 0.0;
            return;
        } else {
            if ((m_targetEntity.second->IsDynamicEntity()) and (m_targetEntity.second->DestinyMgr()->IsMoving())) {
                SetSpeedFraction(0.2);
            } else {
                Stop();
            }
        }
    } else {
        if (m_tractored and m_tractorPause) {
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
            m_prevSpeedFraction = 0.0;
            m_activeSpeedFraction = m_userSpeedFraction = m_timeFraction = 1;
        } else if (m_userSpeedFraction != 0.0) {
            SetSpeedFraction(1.0);
        }
    }

    heading.normalize();
    m_targetPoint = target_point + (heading * m_targetDistance);

    MoveObject();
}

void DestinyManager::Orbit() {
    if ((m_targetDistance > BUBBLE_RADIUS_METERS) or (m_followDistance > BUBBLE_RADIUS_METERS)) {
        if (mySE->HasPilot())
            mySE->GetPilot()->SendErrorMsg("Internal Server Error.  Ref: ServerError 35412");
        sLog.Error("Destiny::Orbit()", "%s(%u) - Distance check OOB. ", mySE->GetName(), mySE->GetID());
        Stop();
        return;
    }

    uint32 timeStamp = sEntityList.GetStamp() - m_stateStamp;
    double Tr = m_targetEntity.second->GetRadius();
    GPoint Tp(m_targetEntity.second->GetPosition());

    double centers(m_position.distance(Tp));
    double edges(centers - m_radius - Tr);
    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "1 - %s(%u): time:%u, centers:%.2f, edges:%.2f, target:%u, follow:%u", \
            mySE->GetName(), mySE->GetID(), timeStamp, centers, edges, m_targetDistance, m_followDistance);

    GPoint mPos(NULL_ORIGIN);
    double mPosAdj(0.0);
    if ((edges / 2) > m_followDistance) {
        if (m_orbiting == Destiny::Ball::Orbit::TooFar) {
            MoveObject();
            return;
        }
        m_orbiting = Destiny::Ball::Orbit::TooFar;
        double radTarg = atan2(Tp.z - m_position.z, Tp.x - m_position.x);
        radTarg += atan2(m_followDistance, edges);
        mPos.x = m_followDistance * cos(radTarg);
        mPos.z = m_followDistance * sin(radTarg);
        if (Tp.y > m_position.y) {
            mPos.y = Tp.y - m_position.y;
        } else {
            mPos.y = m_position.y - Tp.y;
        }
        m_targetPoint = Tp + mPos;
        GVector heading(m_position, m_targetPoint);
        heading.normalize();
        m_shipHeading = heading;
        _log(DESTINY__ORBIT_TRACE, "2 - way too far - rads:%.3f, heading: %.3f, %.3f, %.3f", \
                radTarg, m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
        MoveObject();
        return;
    } else if ( (centers + m_targetDistance / 3) < m_followDistance) {
        if (m_orbiting == Destiny::Ball::Orbit::TooClose) {
            MoveObject();
            return;
        }
        m_orbiting = Destiny::Ball::Orbit::TooClose;
        double radTarg = atan2(Tp.z - m_position.z, Tp.x - m_position.x);
        mPos.x = m_followDistance * cos(radTarg);
        mPos.z = m_followDistance * sin(radTarg);
        if (Tp.y > m_position.y) {
            mPos.y = Tp.y - m_position.y;
        } else {
            mPos.y = m_position.y - Tp.y;
        }
        m_targetPoint = Tp + mPos;
        GVector heading(m_position, m_targetPoint);
        heading.normalize();
        m_shipHeading = heading;
        _log(DESTINY__ORBIT_TRACE, "2 - way too close - rads:%.3f, heading: %.3f, %.3f, %.3f", \
                radTarg, m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
        MoveObject();
        return;
    } else if ((edges - m_targetDistance / 4) > m_followDistance) {
        m_orbiting = Destiny::Ball::Orbit::Far;
        mPosAdj = -m_followDistance / 25;
        _log(DESTINY__ORBIT_TRACE, "2 - too far");
    } else if (centers < m_followDistance) {
        m_orbiting = Destiny::Ball::Orbit::Close;
        mPosAdj = m_followDistance / 25;
        _log(DESTINY__ORBIT_TRACE, "2 - too close");
    } else {
        m_orbiting = Destiny::Ball::Orbit::Orbiting;
        _log(DESTINY__ORBIT_TRACE, "2 - within tolerance");
    }

    #define LogMacro(v) _log(DESTINY__ORBIT_TRACE, "m - " #v ": (%.3f, %.3f, %.3f)   len=%.3f", v.x, v.y, v.z, v.length())

    double radius = m_followDistance + mPosAdj;
    double theta = EvE::Trig::Pi2 - EvE::Trig::Deg2Rad(360) - (m_orbitRadTic * timeStamp);
    double inclination = 45;
    double period = fmod(timeStamp, m_orbitTime) / m_orbitTime;
    double c = cos(EvE::Trig::Deg2Rad(360 * period));
    double phi = EvE::Trig::Deg2Rad(inclination * c);
    double s = sin(EvE::Trig::Deg2Rad(360 * period));
    double mu = EvE::Trig::Deg2Rad(inclination * s);
    mPos.x = radius * cos( theta );
    mPos.z = radius * sin( theta );
    mPos.y = radius * phi;
    _log(DESTINY__ORBIT_TRACE, "4 - theta:%.5f, phi:%.3f, mu:%.2f period:%.5f, radius:%.3f, inc:%.5f", theta,phi,mu,period,radius,inclination);
    LogMacro(mPos);
    mPos += Tp;
    m_position = mPos;

    GPoint mPosNext(NULL_ORIGIN);
    theta += m_orbitRadTic;
    period = fmod(timeStamp + 1, m_orbitTime) / m_orbitTime;
    c = cos(EvE::Trig::Deg2Rad(360 * period));
    phi = EvE::Trig::Deg2Rad(inclination * c);
    mPosNext.x = radius * cos( theta );
    mPosNext.z = radius * sin( theta );
    mPosNext.y = radius * phi;
    LogMacro(mPosNext);
    double Tv = (m_targetEntity.second->DestinyMgr() != nullptr ? m_targetEntity.second->DestinyMgr()->GetSpeed() : 0);
    GVector Th(m_targetEntity.second->DestinyMgr() != nullptr ? m_targetEntity.second->DestinyMgr()->GetHeading() : NULL_ORIGIN_V);
    Tp += (Tv*Th);
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
    GPoint Tp(m_targetEntity.second->GetPosition());
    double adj = sqrt(pow(m_position.x - Tp.x, 2) * pow(m_position.z - Tp.z, 2));
    double opp = m_position.y - Tp.y;
    double i = atan2(opp, adj);

    GPoint mPos(NULL_ORIGIN);
    double radius = m_targetDistance + (m_radius *2);
    double theta = m_orbitRadTic * 0;
    GVector target(m_position, Tp);
    LogMacro(target);
    double hyp = sqrt(pow(target.z, 2) + pow(target.x, 2));
    double inclination = 45;
    double period = fmod(0, m_orbitTime) /m_orbitTime;
    double c = cos(EvE::Trig::Deg2Rad(360 * period));
    double phi = EvE::Trig::Deg2Rad(inclination * c);
    mPos.x = radius * cos( theta );
    mPos.z = radius * sin( theta );
    mPos.y = radius * phi;

    if (mPos.isNaN()) {
        _log(DESTINY__ERROR, "mPos calculated as NaN.  Stopping Orbit.");
        Stop();
        return NULL_ORIGIN;
    }
    return mPos;
}

void DestinyManager::ClearOrbit() {
    m_orbiting = Destiny::Ball::Orbit::None;
    m_orbitTime = 0.0;
    m_orbitRadTic = 0.0;
    m_targetDistance = 0;
    m_followDistance = 0;
    m_maxOrbitSpeedFraction = 1.0;
}

void DestinyManager::InitWarp() {
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

    bool cruise(true);
    double cruiseTime(0.0);
    double accelDistance(0.0), decelDistance(0.0), cruiseDistance(0.0);
    if (abs(static_cast<double>(m_targetDistance)) < warpSpeedInMeters) {
        _log(
            DESTINY__WARP_TRACE,
            "short warp distance dictates that warp cruise time is unnecessary"
        );
        cruise = false;
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
        m_warpAccelTime = 7;
        m_warpDecelTime = 21;
        decelDistance = exp(static_cast<double>(m_warpDecelTime));
        accelDistance = exp(static_cast<double>(3) * static_cast<double>(m_warpAccelTime));
        cruiseDistance = (static_cast<double>(m_targetDistance) - accelDistance - decelDistance);
        cruiseTime = static_cast<double>(cruiseDistance / warpSpeedInMeters);
    }

    double warpTime(static_cast<double>(m_warpAccelTime) + static_cast<double>(m_warpDecelTime) + std::floor(cruiseTime));

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

    if (mySE->HasPilot()) {
        mySE->GetSelf()->SetAttribute(AttrCapacitorCharge, m_capNeeded);
        mySE->GetShipSE()->Warp();
        m_capNeeded = 0;
    }

    mySE->TargetMgr()->ClearAllTargets();

    WarpAccel(0);
}

void DestinyManager::WarpAccel(uint16 sec_into_warp) {
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
    SetSpeedFraction(0.0);
    m_stop = true;
    SafeDelete(m_warpState);
    m_targBubble = nullptr;
    if ((mySE->IsNPCSE()) and (mySE->GetNPCSE()->GetAIMgr() != nullptr)) {
        mySE->GetNPCSE()->GetAIMgr()->WarpOutComplete();
    }
    Halt();
}

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
    if (mySE->SystemMgr()->GetSE(m_targetEntity.first) == nullptr) {
        Stop();
        return true;
    }
    if (!m_targetEntity.second->IsDynamicEntity())
        return false;
    if (m_targetEntity.second->HasPilot()) {
        if (m_targetEntity.second->GetPilot()->IsDocked()) {
            Stop();
            return true;
        }
    }
    if (m_targetEntity.second->DestinyMgr()->IsWarping()) {
        Stop();
        return true;
    }
    return false;
}

void DestinyManager::BeginMovement() {
    if (!m_hasSentShipUpdates) {
        std::vector<PyTuple*> updates;
        SetBallAgility sbagility;
            sbagility.entityID =  mySE->GetID();
            sbagility.agility = m_shipInertia;
        updates.push_back(sbagility.Encode());
        SetBallMassive sbmassive;
            sbmassive.entityID = mySE->GetID();
            sbmassive.is_massive = false;
        updates.push_back(sbmassive.Encode());
        SetBallMass sbmass;
            sbmass.entityID = mySE->GetID();
            sbmass.mass = m_mass;
        updates.push_back(sbmass.Encode());
        SendDestinyUpdate(updates);
        m_hasSentShipUpdates = true;
    }

    m_turnTic = 0;
    m_stop = m_accel = m_decel = m_turning = false;

    if (!mySE->IsNPCSE() or (mySE->IsNPCSE() and mySE->GetNPCSE()->GetAIMgr()->IsIdle()))
        m_stateStamp = sEntityList.GetStamp();

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
        GVector shipVector(m_position, m_targetPoint);
        m_targetDistance = shipVector.length();
        m_orbitRadTic = 0.0;
        m_maxOrbitSpeedFraction = 1;
    }

    if (IsCloaked())
        UnCloak();

    if ((m_userSpeedFraction < 0.02) and (m_timeFraction < 0.02)) {
        SetSpeedFraction(1.0, true);
    } else {
        SetSpeedFraction(m_userSpeedFraction, true);
    }

    SetPosition(m_position, sConfig.debug.PositionHack);
}

void DestinyManager::Follow(SystemEntity* pSE, uint32 distance) {
    if ((m_ballMode == Destiny::Ball::Mode::FOLLOW)
    and (m_targetEntity.second == pSE)
    and (m_followDistance == distance)
    and (m_userSpeedFraction))
        return;

    if (m_orbiting)
        ClearOrbit();

    m_ballMode = Destiny::Ball::Mode::FOLLOW;
    m_targetPoint = pSE->GetPosition();

    if (pSE->IsStationSE()) {
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
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::AlignTo(SystemEntity* ent) {
    Follow(ent, 0);
}

void DestinyManager::GotoDirection(const GPoint& direction) {
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
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::GotoPoint(const GPoint& point) {
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
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::WarpTo(const GPoint& where, int32 distance, bool autoPilot, SystemEntity* pSE) {
    SafeDelete(m_warpState);

    if (autoPilot) {
        Follow(pSE, distance);
    } else {
        GotoPoint(where);
    }

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;

    m_stopDistance = distance;
    GVector warp_distance(m_position, where);
    m_targetDistance = warp_distance.length();
    m_targetDistance -= static_cast<double>(m_stopDistance);
    warp_distance.normalize();
    warp_distance *= m_stopDistance;
    m_targetPoint -= warp_distance;

    m_targBubble = sBubbleMgr.GetBubble(mySE->SystemMgr(), m_targetPoint);
    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__TRACE, "Destiny::WarpTo() - %s(%u) target bubble: %u  m_stopDistance: %i  m_targetDistance: %.2f",
            mySE->GetName(), mySE->GetID(), m_targBubble->GetID(), m_stopDistance, m_targetDistance);

    if (mySE->IsNPCSE() or mySE->IsDroneSE()) {
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

    if (mySE->HasPilot()) {
        if (m_targetDistance < static_cast<double>(minWarpDistance)) {
            mySE->GetPilot()->SendErrorMsg("That is too close for your Warp Drive.");
            if (sConfig.debug.PositionHack)
                SetPosition(mySE->GetPosition(), true);
            m_ballMode = Destiny::Ball::Mode::STOP;
            SafeDelete(m_warpState);
            return;
        }

        Client *pClient = mySE->GetPilot();

        double currentShipCap = pClient->GetShip()->GetAttribute(AttrCapacitorCharge).get_float();
        double capNeeded = m_mass * m_warpCapacitorNeed * (static_cast<double>(m_targetDistance) / static_cast<double>(ONE_AU_IN_METERS));
        capNeeded *= (1.0 - (0.1 *pClient->GetChar()->GetSkillLevel(EvESkill::WarpDriveOperation)));

        _log(DESTINY__WARNING, "Warp Cap need for %s(%u) is %.4f", mySE->GetName(), mySE->GetID(), capNeeded);

        if (capNeeded > currentShipCap) {
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

    if (m_targBubble->HasWarpBubble()) {
        if (!mySE->GetSelf()->HasAttribute(AttrWarpBubbleImmune))
            ;
    }

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
    updates.clear();

    SetBallMassive bm;
    bm.entityID = mySE->GetID();
    bm.is_massive = false;
    PyTuple *up = bm.Encode();
    SendSingleDestinyUpdate(&up, true);

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

void DestinyManager::Orbit(SystemEntity *pSE, uint32 distance) {
    if ((m_ballMode == Destiny::Ball::Mode::ORBIT)
    and (m_targetEntity.second == pSE)
    and (m_targetDistance == static_cast<double>(distance)))
        return;

    if (m_orbiting)
        m_shipHeading = NULL_ORIGIN_V;

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

    double Tr = pSE->GetRadius();
    double Tm = pSE->GetSelf()->GetAttribute(AttrMass).get_float();
    if (Tm != 0.0)
        Tm = pSE->GetSelf()->type().mass();

    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "%s(%u) - Target Data - mass:%.3f, speed:%.2f, radius:%.2f", \
            mySE->GetName(), mySE->GetID(), Tm, (pSE->DestinyMgr() ? pSE->DestinyMgr()->GetSpeed() : 0 ), Tr);

    double Rc = ((distance + 150 + m_radius - (pSE->GetRadius() /12)) * 1.2);
    double Rc2 =  std::pow(Rc,2);
    double Vm2 =  std::pow(m_maxShipSpeed,2);
    double t2 =  std::pow(m_shipAgility,2);

    double one = (108 * t2 * Vm2 * Rc2);
    double two = (12 * t2 * Vm2 *  std::pow(Rc,10));
    double three = (12 * std::sqrt(81 *  std::pow(m_shipAgility,4) *  std::pow(m_maxShipSpeed,4) + two));
    double four = (6 *  std::cbrt(one + 8 *  std::pow(Rc,6) + three));
    double five =  std::cbrt( std::sqrt(three *  std::pow(Rc,8) + two));
    double six = (one + (8 * Rc2) + (12 * five));
    m_followDistance =  std::sqrt(four + (24 *  std::pow(Rc, 4) / six) + 12 * Rc2) / 6;

    double velocity = m_maxShipSpeed * ((distance / m_followDistance) + 0.065);
    m_maxOrbitSpeedFraction = velocity / m_maxShipSpeed;

    double circ = EvE::Trig::Pi2 * m_followDistance;
    m_orbitTime = circ / velocity;
    m_orbitRadTic = EvE::Trig::Pi2 / m_orbitTime;

    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "%s(%u) - Orbit Data - Rc:%.3f, velocity:%.2f, osf:%.2f, targetDistance:%.2f, followDistance:%u, orbitTime:%.1f, radTic:%.5f", \
                mySE->GetName(), mySE->GetID(), Rc, velocity, m_maxOrbitSpeedFraction, \
                m_targetDistance, m_followDistance, m_orbitTime, m_orbitRadTic);

    if (m_followDistance == 0) {
        _log(DESTINY__ERROR, "%s(%u) - FollowDistance is 0.", mySE->GetName(), mySE->GetID());
        m_followDistance = (m_targetDistance + Tr + m_radius);
    }

    CmdOrbit du;
        du.entityID = mySE->GetID();
        du.orbitEntityID = pSE->GetID();
        du.distance = (int32)m_targetDistance;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
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
    double dot = toVec.dotProduct(m_shipHeading);
    double degrees = EvE::Trig::Rad2Deg(std::acos(dot));
    if (degrees < TURN_ALIGNMENT)
        return true;
    return false;
}

void DestinyManager::Undock(GPoint dir) {
    m_targetPoint = dir *1.0e16;
    m_shipHeading = GVector(dir);
    SetUndockSpeed();
    if (mySE->IsShipSE())
        mySE->GetShipSE()->GetShipItemRef()->SetUndocking(false);
}

void DestinyManager::SetUndockSpeed() {
    m_stop = false;
    m_orbiting = 0;
    m_stateStamp = sEntityList.GetStamp();
    m_changeDelay = true;
    m_shipAccelTime = 0.5;
    m_prevSpeedFraction = 0.0;
    m_userSpeedFraction = 1.1;
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = m_shipHeading * m_maxSpeed;
    m_activeSpeedFraction = 1.1;
    m_timeFraction = 1.1;

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

    const GPoint stationPos = station->GetPosition();
    double rangeToStationPerimiter = m_position.distance(stationPos);
    rangeToStationPerimiter -= mySE->GetRadius();
    rangeToStationPerimiter -= station->GetRadius();

    _log(DESTINY__TRACE, "Destiny::AttemptDockOperation() rangeToStationPerimiter is %.2fm", rangeToStationPerimiter);
    if (rangeToStationPerimiter > 2500.0) {
        AlignTo( station );
        if (mySE->HasPilot() and mySE->GetPilot()->CanThrow())
            throw UserError ("DockingApproach");
    }

    pClient->SetStateTimer(Player::State::Dock, sConfig.world.StationDockDelay *1000);
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

void DestinyManager::SetPosition(const GPoint &pt, bool update) {
    _log(DESTINY__TRACE, "Destiny::SetPosition() called by %s(%u)", mySE->GetName(), mySE->GetID());

    if (pt.isZero()) {
        _log(DESTINY__TRACE, "Destiny::SetPosition() - %s(%u) point is zero", mySE->GetName(), mySE->GetID());
        EvE::traceStack();
        m_position = sMapData.GetRandPointOnPlanet(mySE->GetLocationID());
    } else {
        m_position = pt;
    }

    mySE->SetPosition(m_position);

    if (mySE->IsPOSSE() or mySE->IsProbeSE() or update) {
        SetBallPosition du;
            du.entityID = mySE->GetID();
            du.x = m_position.x;
            du.y = m_position.y;
            du.z = m_position.z;
        PyTuple* up = du.Encode();
        SendSingleDestinyUpdate(&up);
    }
}

void DestinyManager::SetRadius(double radius, bool update) {
    _log(DESTINY__TRACE, "Destiny::SetPosition() called by %s(%u)", mySE->GetName(), mySE->GetID());

    m_radius = radius;
    mySE->SetRadius(m_radius);

    if (update) {
        SetBallRadius du;
            du.entityID = mySE->GetID();
            du.radius = m_radius;
        PyTuple* up = du.Encode();
        SendSingleDestinyUpdate(&up);
    }
}

void DestinyManager::SetMaxVelocity(double maxVelocity) {
    double maxSpeed = mySE->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();
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

void DestinyManager::SpeedBoost(bool deactivate)
{
    m_prevSpeed = m_maxSpeed * m_activeSpeedFraction;

    m_mass = mySE->GetSelf()->GetAttribute(AttrMass).get_float();
    m_massMKg = m_mass / 1000000;
    m_shipAgility = m_massMKg * m_shipInertia;
    
    // Время достижения 75% скорости по официальной формуле
    m_alignTime = (-log(0.25) * m_shipAgility);
    m_shipMaxAccelTime = (-log(0.0001) * m_shipAgility);
    
    m_degPerTic = (60.0 - m_shipAgility) / 10;
    m_maxShipSpeed = mySE->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();
    m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;
    m_activeSpeedFraction = m_prevSpeed / m_maxShipSpeed;

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
    m_hasSentShipUpdates = true;

    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        if (deactivate) {
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::Deactivate");
        } else if (m_activeSpeedFraction < m_userSpeedFraction) {
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(psf!=0&tf>usf) - decelerating.");
        } else if (m_activeSpeedFraction) {
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(0<asf<=usf)");
        } else {
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

    SetSpeedFraction(m_userSpeedFraction, true);
}

void DestinyManager::WebbedMe(InventoryItemRef modRef, bool apply)
{
    if (apply) {
        m_maxShipSpeed *= (1 + (modRef->GetAttribute(AttrSpeedFactor).get_float() / 100.0));
    } else {
        m_maxShipSpeed /= (1 + (modRef->GetAttribute(AttrSpeedFactor).get_float() / 100.0));
    }
    m_activeSpeedFraction = m_activeSpeedFraction * 0.999;
    std::vector<PyTuple*> updates;
    SetBallSpeed sbms;
        sbms.entityID = mySE->GetID();
        sbms.speed = m_maxShipSpeed;
        updates.push_back(sbms.Encode());
    SendDestinyUpdate(updates);
    m_hasSentShipUpdates = true;

    SetSpeedFraction(m_userSpeedFraction, true);
}

// ===== НОВАЯ ВЕРСИЯ UpdateShipVariables с официальной физикой =====
void DestinyManager::UpdateShipVariables()
{
    InventoryItemRef sRef = mySE->GetSelf();
    m_mass = sRef->GetAttribute(AttrMass).get_float();
    m_massMKg = m_mass / 1000000;

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

    // Защита от нулевой скорости
    if (m_maxShipSpeed < 0.1) {
        _log(DESTINY__WARNING, "UpdateShipVariables: maxShipSpeed is 0 for %s(%u)! Setting default 100 m/s", 
             mySE->GetName(), mySE->GetID());
        m_maxShipSpeed = 100.0;
    }

    m_speedToLeaveWarp = m_maxShipSpeed * 0.75;
    if ((m_speedToLeaveWarp < 100) and (m_maxShipSpeed > 135))
        m_speedToLeaveWarp = 100;

    // Расчет agility по официальной формуле: agility = mass * inertia (mass в Mkg)
    m_shipAgility = m_massMKg * m_shipInertia;
    
    if (m_shipAgility < 0.001) {
        _log(DESTINY__WARNING, "UpdateShipVariables: agility too small (%.6f), setting to 1.0", m_shipAgility);
        m_shipAgility = 1.0;
    }

    // Расчет времени ускорения по официальной формуле
    // Время достижения 99% скорости: t = -τ * ln(0.01)
    // где τ = (Mass * Agility) / Friction
    m_shipMaxAccelTime = -log(0.01) * m_shipAgility;
    m_alignTime = -log(0.25) * m_shipAgility;  // Время достижения 75% скорости
    m_timeToEnterWarp = m_alignTime;

    // Угловая скорость поворота (градусов за тик)
    // В официальном коде это зависит от agility и массы
    m_degPerTic = (60.0 - m_shipAgility) / 10;
    if (m_degPerTic < 0.1) m_degPerTic = 0.1;

    // Инициализация переменных физики
    m_speedChangeTime = GetTimeMSeconds();
    m_speedAtChange = m_activeSpeedFraction * m_maxShipSpeed;
    m_targetSpeed = m_userSpeedFraction * m_maxShipSpeed;
    m_timeSinceChange = 0.0;

    // Расчет массового модификатора для поворота
    if (m_mass != 0.0) {
        m_massModifier = 100.0 / (m_mass / m_physicsFriction);
    } else {
        m_massModifier = 100.0;
    }

    // Отладка
    _log(DESTINY__MOVE_TRACE, 
        "=== UpdateShipVariables: %s(%u) mass=%.2f Mkg, inertia=%.3f, agility=%.3fs, maxSpeed=%.2f m/s, massModifier=%.3f ===", 
        mySE->GetName(), mySE->GetID(), m_massMKg, m_shipInertia, m_shipAgility, m_maxShipSpeed, m_massModifier);

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
            sbmassive.is_massive = false;
        updates.push_back(sbmassive.Encode());
        SetBallMass sbmass;
            sbmass.entityID = mySE->GetID();
            sbmass.mass = m_mass;
        updates.push_back(sbmass.Encode());
        SetBallSpeed sbspeed;
            sbspeed.entityID = mySE->GetID();
            sbspeed.speed = m_maxShipSpeed;
        updates.push_back(sbspeed.Encode());
        SendDestinyUpdate(updates);
    }
}
// ===== КОНЕЦ НОВОЙ ВЕРСИИ =====

void DestinyManager::MakeMissile(Missile* pMissile) {
    SetMaxVelocity(pMissile->GetSpeed());
    SetPosition(pMissile->GetSelf()->position());
    m_mass = pMissile->GetSelf()->type().mass();
    m_massMKg = m_mass / 1000000;
    m_shipInertia = pMissile->GetSelf()->GetAttribute(AttrInetia).get_float();
    m_shipAgility = m_massMKg * m_shipInertia;

    m_stop = false;
    m_ballMode = Destiny::Ball::Mode::MISSILE;
    m_stateStamp = sEntityList.GetStamp();

    SystemEntity* pTarget = pMissile->GetTargetSE();
    m_targetPoint = GPoint(pTarget->GetPosition());
    m_targetEntity.first = pTarget->GetID();
    m_targetEntity.second = pTarget;
    m_targetDistance = m_position.distance(m_targetPoint);

    GVector moveVector(m_position, m_targetPoint);
    moveVector.normalize();
    m_shipHeading = moveVector;

    SetUndockSpeed();
    mySE->SystemMgr()->AddEntity(pMissile, false);

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
    SendDestinyUpdate(updates);
}

void DestinyManager::UpdateNewShip(const ShipItemRef newShipRef) {
    if (m_hasSentShipUpdates)
        return;

    Client* pClient = mySE->GetPilot();
    if (pClient == nullptr)
        return;
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
    SendSingleDestinyUpdate(&shipItem);

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
    m_targetDistance = moveVector.length();
    moveVector.normalize();
    m_shipHeading = moveVector;

    m_maxShipSpeed = speed.get_float();
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = m_shipHeading * m_maxSpeed;

    m_followDistance = 500 + pShipSE->GetRadius();
    m_shipAccelTime = 0.1;

    m_activeSpeedFraction = m_userSpeedFraction = m_timeFraction = 1.0;

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

void DestinyManager::SendJettisonPacket() const {
    OnSpecialFX10 effect;
        effect.entityID = mySE->GetID();
        effect.guid = "effects.Jettison";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple* up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

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
    SendSingleDestinyUpdate(&up);
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
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendCloakFx(bool apply, bool module) const {
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
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendSpecialEffect10(uint32 entityID, uint32 targetID, std::string guid, bool isOffensive, bool start, bool isActive) const
{
    OnSpecialFX10 effect;
        effect.entityID = entityID;
        effect.targetID = targetID;
        effect.guid = guid;
        effect.area = new PyList();
        effect.isOffensive = isOffensive;
        effect.start = start;
        effect.active = isActive;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendSpecialEffect(uint32 entityID, uint32 moduleID, uint32 moduleTypeID, uint32 targetID,
                                       uint32 chargeTypeID, std::string guid, bool isOffensive, bool start,
                                       bool isActive, int32 duration, uint32 repeat, int32 graphicInfo) const
{
    OnSpecialFX14 effect;
        effect.entityID = entityID;
        effect.moduleID = moduleID;
        effect.moduleTypeID = moduleTypeID;
        effect.targetID = (targetID == 0 ? PyStatic.NewNone() : new PyInt(targetID));
        effect.chargeTypeID = (chargeTypeID == 0 ? PyStatic.NewNone() : new PyInt(chargeTypeID));
        effect.guid = guid;
        effect.isOffensive = isOffensive;
        effect.start = start;
        effect.active = isActive;
        effect.duration = duration;
        effect.repeat = repeat;
        effect.startTime = GetFileTimeNow();
        effect.graphicInfo = (graphicInfo == 0 ? PyStatic.NewNone() : new PyInt(graphicInfo));
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendJumpOut(uint32 gateID) const {
    OnSpecialFX10 effect;
        effect.entityID = mySE->GetID();
        effect.targetID = gateID;
        effect.guid = "effects.JumpOut";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
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
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendGateActivity(uint32 gateID) const {
    OnSpecialFX10 du;
        du.entityID = gateID;
        du.guid = "effects.GateActivity";
        du.isOffensive = 0;
        du.start = 1;
        du.active = 0;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendWormholeActivity(uint32 wormholeID) const {
    OnSpecialFX10 du;
        du.entityID = wormholeID;
        du.guid = "effects.WormholeActivity";
        du.isOffensive = 0;
        du.start = 1;
        du.active = 0;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendBallInteractive(const ShipItemRef shipRef, bool set) const {
    SetBallInteractive sbi;
        sbi.entityID = shipRef->itemID();
        sbi.interactive = set;
    PyTuple* up = sbi.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendJumpOutEffect(std::string JumpEffect, uint32 shipID) const {
    std::vector<PyTuple*> updates;
    CmdStop du;
        du.entityID = mySE->GetID();
    updates.push_back(du.Encode());
    OnSpecialFX14 effect;
        effect.entityID = mySE->GetID();
        effect.targetID = new PyInt(shipID);
        effect.guid = "effects.JumpDriveOut";
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

void DestinyManager::SendTerminalExplosion(uint32 shipID, uint32 bubbleID, bool isGlobal) const {
     TerminalExplosion du;
        du.shipID = shipID;
        du.bubbleID = bubbleID;
        du.ballIsGlobal = isGlobal;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);
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

    std::vector<PyTuple*> updates;
    OnSpecialFX10 sfx;
    sfx.guid = "effects.Warping";
    sfx.entityID = mySE->GetID();
    sfx.isOffensive = false;
    sfx.start = false;
    sfx.active = false;
    if (m_ballMode == Destiny::Ball::Mode::WARP) {
        sfx.start = true;
        sfx.active = true;
    }

    updates.push_back(sfx.Encode());
    SendDestinyUpdate(updates);
    updates.clear();

    SetState ss;

    ss.stamp = sEntityList.GetStamp();
    ss.ego = mySE->GetID();

    if (mySE->SysBubble() == nullptr) {
        sLog.Error(
            "DestinyManager::SendSetState()",
            "Destiny::SendSetState() the player isn't in a system bubble! Aborting attempt to send state."
        );
        return;
    }

    mySE->SystemMgr()->MakeSetState(mySE->SysBubble(), ss);

    PyTuple* tmp(ss.Encode());

    mySE->GetPilot()->QueueDestinyUpdate(&tmp, true, true);

    mySE->GetPilot()->SetStateSent(true);
}

void DestinyManager::SendMovementPacket()
{
    SendSingleDestinyUpdate(&mvPacket);
    PySafeDecRef(mvPacket);
}

void DestinyManager::SendSingleDestinyEvent(PyTuple** ev, bool self_only) const
{
    std::vector<PyTuple*> updates;
    std::vector<PyTuple*> events(1, *ev);
    SendDestinyUpdate(updates, events, self_only);
}

void DestinyManager::SendSingleDestinyUpdate(PyTuple **up, bool self_only) const {
    std::vector<PyTuple*> updates(1, *up);
    std::vector<PyTuple*> events;
    SendDestinyUpdate(updates, events, self_only);
}

void DestinyManager::SendDestinyUpdate(std::vector<PyTuple*> &updates, bool self_only) const {
    std::vector<PyTuple*> events;
    SendDestinyUpdate(updates, events, self_only);
}

void DestinyManager::SendDestinyUpdate( std::vector<PyTuple*>& updates, std::vector<PyTuple*>& events, bool self_only) const {
    if (!mySE->SystemMgr()->IsLoaded()) {
        return;
    }

    if (self_only) {
        if (!mySE->HasPilot()) {
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
    } else if (mySE->IsOperSE()) {
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
    }
}