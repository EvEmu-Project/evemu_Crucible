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
#include <cmath>
#include <chrono>
#include <cstdio>

// ===== КОНСТАНТЫ ФИЗИКИ =====
#define _HALFPI_  1.57079632679489655
#define _PI_      3.1415926535897931
#define _2PI_     6.2831853071795862

// Коэффициент трения (аналог mPark->mFriction из официального кода)
#define PHYSICS_FRICTION 1.0
// Минимальная скорость для вычисления heading
#define MIN_VELOCITY 0.001
// Дистанция торможения по умолчанию
#define DEFAULT_STOPPING_DISTANCE 100.0
// Соотношение орбитальной скорости к максимальной
#define ORBITAL_SPEED_RATIO 0.7
// Коэффициент коррекции радиуса орбиты
#define ORBIT_RADIUS_CORRECTION 0.01
// Фактор времени
#define PHYSICS_TIME_FACTOR 1.0
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
  // ===== НОВЫЕ ПЕРЕМЕННЫЕ ДЛЯ ФИЗИКИ =====
  m_velocity(GVector(0, 0, 0)),
  m_oldVelocity(GVector(0, 0, 0)),
  m_heading(GVector(0, 0, 1)),
  m_targetVelocity(GVector(0, 0, 0)),
  m_oldHeading(GVector(0, 0, 1)),
  m_physicsFriction(PHYSICS_FRICTION),
  m_stoppingDistance(DEFAULT_STOPPING_DISTANCE),
  m_thrustPower(0.0),
  m_agility(1.0),
  // ===== ПАРАМЕТРЫ ОРБИТЫ =====
  m_orbitAngle(0.0),
  m_orbitRadius(0.0),
  m_orbitalSpeedRatio(ORBITAL_SPEED_RATIO),
  m_angularVelocity(0.0),
  m_orbitCenter(GPoint(NULL_ORIGIN)),
  // ===== ПАРАМЕТРЫ ROLL =====
  m_roll(0.0),
  m_rollSpeed(0.0),
  // ===== АЛИАС ДЛЯ ОБРАТНОЙ СОВМЕСТИМОСТИ =====
  m_shipHeading(m_heading),
  // ===== УСТАРЕВШИЕ ПЕРЕМЕННЫЕ ДЛЯ СОВМЕСТИМОСТИ =====
  m_targetHeading(GVector(0, 0, 1)),
  m_yaw(0.0),
  m_pitch(0.0),
  m_yawSpeed(0.0),
  m_pitchSpeed(0.0),
  m_oldRollSpeed(0.0),
  m_yawDelta(0.0),
  m_massModifier(100.0),
  m_timeFactor(PHYSICS_TIME_FACTOR),
  m_prevSpeed(0.0),
  m_timeFraction(0.0),
  m_prevSpeedFraction(0.0),
  m_maxOrbitSpeedFraction(1.0),
  m_degPerTic(0.0),
  m_radians(0.0),
  m_turnFraction(0.0),
  m_orbitTime(0.0),
  m_orbitRadTic(0.0),
  m_speedChangeTime(0.0),
  m_speedAtChange(0.0),
  m_targetSpeed(0.0),
  m_timeSinceChange(0.0),
  m_activeSpeedFraction(0.0),
  m_userSpeedFraction(0.0),
  m_accel(false),
  m_decel(false),
  m_turning(false),
  m_turnTic(0),
  m_bump(false),
  m_stop(false),
  m_cloaked(false),
  m_tractored(false),
  m_changeDelay(false),
  m_tractorPause(false),
  m_hasSentShipUpdates(false),
  m_capNeeded(0.0),
  m_orbiting(0),
  m_stateStamp(0),
  m_lastDeltaTime(std::chrono::steady_clock::now())
{
    m_radius = mySE->GetRadius();
    m_position = mySE->GetPosition();

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;
    m_targetPoint = GPoint(NULL_ORIGIN);

    UpdateShipVariables();
    m_thrustPower = m_maxShipSpeed * m_mass / 10.0;

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "DestinyManager::DestinyManager() - %s(%u) initialized with new physics. mass=%.2f, agility=%.3f, maxSpeed=%.2f",
                mySE->GetName(), mySE->GetID(), m_mass, m_agility, m_maxShipSpeed);
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
    
    // Проверяем, кто мы: NPC или дрон
    bool isNPC = mySE->IsNPCSE() || mySE->IsDroneSE();
    
    switch(m_ballMode) {
        case Ball::Mode::STOP: {
            if (isNPC) {
                // Простое торможение для NPC/дронов
                if (IsMoving()) {
                    ApplyBrakingNPC(GetDeltaTime());
                    return;
                }
                Stop();
            } else {
                // Сложное торможение для кораблей
                ApplyBraking(GetDeltaTime());
            }
            break;
        }
        case Ball::Mode::GOTO: {
            if (isNPC) {
                // Простое движение для NPC/дронов
                MoveObjectNPC();
            } else {
                // Сложная физика для кораблей
                UpdatePhysics(GetDeltaTime());
            }
            break;
        }
        case Ball::Mode::MISSILE: {
            GVector moveVector(m_position, m_targetPoint);
            moveVector.normalize();
            m_heading = moveVector;
            m_velocity = (moveVector * m_maxSpeed);
            SetPosition(m_position + m_velocity);
        } break;
        case Ball::Mode::ORBIT: {
            if (IsTargetInvalid())
                return;
            if (isNPC) {
                // Простая орбита для NPC/дронов
                UpdateOrbitNPC(GetDeltaTime());
            } else {
                // Сложная орбита для кораблей
                UpdateOrbit(GetDeltaTime());
            }
            break;
        }
        case Ball::Mode::FOLLOW: {
            if (IsTargetInvalid())
                return;
            m_targetPoint = m_targetEntity.second->GetPosition();
            if (isNPC) {
                // Простое следование для NPC/дронов
                MoveObjectNPC();
            } else {
                // Сложная физика для кораблей
                UpdatePhysics(GetDeltaTime());
            }
            break;
        }
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
                        _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  Ship %s(%u) for Player %s(%u) Has WarpState but checks are false.",
                                    mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());
                        mySE->GetPilot()->SendErrorMsg("Internal Server Error. Ref: ServerError 35928.   Please Dock or Relog to reset your ship.");
                    } else {
                        _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  NPC %s(%u) Has WarpState but checks are false.",
                                    mySE->GetName(), mySE->GetID());
                    }
                }
                return;
            }

            GVector toVec(m_position, m_targetPoint);
            toVec.normalize();
            double dot = toVec.dotProduct(m_heading);
            double degrees = EvE::Trig::Rad2Deg(std::acos(dot));

            double currentSpeed = m_velocity.length();
            double speedRatio = currentSpeed / m_maxShipSpeed;

            if ((degrees < WARP_ALIGNMENT) and (speedRatio > 0.749)) {
                m_heading = toVec;
                InitWarp();
                return;
            } else if (speedRatio < 0.749 && m_userSpeedFraction < 0.7499) {
                SetSpeedFraction(1.0, true);
            } else if ((sEntityList.GetStamp() - m_stateStamp) > m_timeToEnterWarp + 0.3) {
                if (mySE->HasPilot()) {
                    _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  Ship %s(%u) for Player %s(%u) - warp align/speed is incorrect, but time > shipTimeToWarp.",
                                mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());
                } else {
                    _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  NPC %s(%u) - warp align/speed is incorrect, but time > shipTimeToWarp.",
                            mySE->GetName(), mySE->GetID());
                }
                m_heading = toVec;
                InitWarp();
                return;
            }

            if (isNPC) {
                MoveObjectNPC();
            } else {
                UpdatePhysics(GetDeltaTime());
            }
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

// ===== НОВАЯ ФИЗИКА: ОСНОВНОЙ МЕТОД ДЛЯ КОРАБЛЕЙ =====
void DestinyManager::UpdatePhysics(double dt) {
    if (mySE->SysBubble() == nullptr)
        mySE->SystemMgr()->AddEntity(mySE);

    // Защита от слишком больших/маленьких значений
    if (dt > 1.1) dt = 1.1;
    if (dt < 0.0001) dt = 0.0001;

    GVector toTarget = m_targetPoint - m_position;
    double distance = toTarget.length();

    if (distance < 0.001) {
        ApplyBraking(dt);
        return;
    }

    GVector direction = toTarget / distance;

    double targetSpeed = m_maxSpeed * m_userSpeedFraction;
    if (distance < m_stoppingDistance) {
        double brakeFactor = distance / m_stoppingDistance;
        targetSpeed = m_maxSpeed * m_userSpeedFraction * brakeFactor;
        if (targetSpeed < 0.01) targetSpeed = 0.01;
    }

    if (m_orbiting && m_orbiting < Destiny::Ball::Orbit::TooClose) {
        targetSpeed *= m_orbitalSpeedRatio;
    }

    m_targetVelocity = direction * targetSpeed;

    double agility = m_agility;
    if (agility < 0.001) agility = 1.0;

    double factor = dt / agility;
    if (factor > 1.0) factor = 1.0;

    m_oldVelocity = m_velocity;

    m_velocity.x += (m_targetVelocity.x - m_velocity.x) * factor;
    m_velocity.y += (m_targetVelocity.y - m_velocity.y) * factor;
    m_velocity.z += (m_targetVelocity.z - m_velocity.z) * factor;

    double speed = m_velocity.length();
    double maxAllowedSpeed = m_maxSpeed * m_userSpeedFraction;
    if (speed > maxAllowedSpeed && maxAllowedSpeed > 0.001) {
        m_velocity = (m_velocity / speed) * maxAllowedSpeed;
    }

    if (speed < MIN_VELOCITY) {
        m_velocity = GVector(0, 0, 0);
    }

    // ===== ИСПРАВЛЕНИЕ: ИСПОЛЬЗУЕМ SetPosition ДЛЯ ЗАПИСИ В СЕРВЕР =====
    // Вместо: m_position += m_velocity * dt;
    GPoint newPos = m_position + m_velocity * dt;
    SetPosition(newPos, true);  // ← Записывает в сервер (mySE->SetPosition) И отправляет клиенту

    CalculateHeading();
    CalculateRoll(dt);
    // SendPhysicsUpdate() уже вызывается внутри SetPosition() если update = true
    // Если нет - раскомментируйте строку ниже:
     SendPhysicsUpdate();

    if (mySE->HasPilot())
        CheckBump();

    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        _log(DESTINY__MOVE_TRACE, "Destiny::UpdatePhysics() - %s(%u) speed: %.2f m/s (%.1f%%) heading: %.3f,%.3f,%.3f dist:%.2f dt:%.3f agility:%.3f",
            mySE->GetName(), mySE->GetID(), m_velocity.length(), m_userSpeedFraction*100,
            m_heading.x, m_heading.y, m_heading.z, distance, dt, m_agility);
    }
}

// ===== ТОРМОЖЕНИЕ ДЛЯ КОРАБЛЕЙ =====
void DestinyManager::ApplyBraking(double dt) {
    if (dt > 1.1) dt = 1.1;
    if (dt < 0.0001) dt = 0.0001;

    double agility = m_agility;
    if (agility < 0.001) agility = 1.0;

    double factor = dt / agility;
    if (factor > 1.0) factor = 1.0;

    m_oldVelocity = m_velocity;
    m_velocity *= (1.0 - factor);

    if (m_velocity.length() < MIN_VELOCITY) {
        m_velocity = GVector(0, 0, 0);
    }

    m_position += m_velocity * dt;
    mySE->SetPosition(m_position);

    if (m_velocity.length() > MIN_VELOCITY) {
        CalculateHeading();
    }

    SendPhysicsUpdate();
}

// ===== ВЫЧИСЛЕНИЕ HEADING ИЗ СКОРОСТИ =====
void DestinyManager::CalculateHeading() {
    double speed = m_velocity.length();

    if (speed > MIN_VELOCITY) {
        m_heading = m_velocity / speed;
    } else {
        GVector toTarget = m_targetPoint - m_position;
        if (toTarget.length() > 0.001) {
            m_heading = toTarget / toTarget.length();
        }
    }

    if (m_heading.length() > 0.001) {
        m_heading = m_heading / m_heading.length();
    }
}

// ===== ВЫЧИСЛЕНИЕ ROLL =====
void DestinyManager::CalculateRoll(double dt) {
    static GVector oldHeading = m_heading;

    double dot = oldHeading.dotProduct(m_heading);
    if (dot < -1.0) dot = -1.0;
    if (dot > 1.0) dot = 1.0;

    double headingDelta = std::acos(dot);

    m_rollSpeed += headingDelta * 0.5;
    m_rollSpeed *= 0.9;

    if (m_rollSpeed > _HALFPI_) m_rollSpeed = _HALFPI_;
    if (m_rollSpeed < -_HALFPI_) m_rollSpeed = -_HALFPI_;

    m_roll += m_rollSpeed * dt * 0.5;
    m_roll *= 0.9;

    if (m_roll > _HALFPI_) m_roll = _HALFPI_;
    if (m_roll < -_HALFPI_) m_roll = -_HALFPI_;

    oldHeading = m_heading;
}

// ===== ОРБИТА ДЛЯ КОРАБЛЕЙ =====
void DestinyManager::UpdateOrbit(double dt) {
    if (!m_targetEntity.second) return;

    m_orbitCenter = m_targetEntity.second->GetPosition();

    double orbitalSpeed = m_maxSpeed * m_orbitalSpeedRatio * m_userSpeedFraction;
    if (m_orbitRadius > 0.001) {
        m_angularVelocity = orbitalSpeed / m_orbitRadius;
    } else {
        m_angularVelocity = 0.01;
    }

    m_orbitAngle += m_angularVelocity * dt;
    if (m_orbitAngle > _2PI_) m_orbitAngle -= _2PI_;
    if (m_orbitAngle < 0) m_orbitAngle += _2PI_;

    GVector orbitPoint;
    orbitPoint.x = m_orbitCenter.x + m_orbitRadius * cos(m_orbitAngle);
    orbitPoint.y = m_orbitCenter.y;
    orbitPoint.z = m_orbitCenter.z + m_orbitRadius * sin(m_orbitAngle);

    m_targetPoint = orbitPoint;
    UpdatePhysics(dt);

    GVector toCenter = m_orbitCenter - m_position;
    double currentRadius = toCenter.length();
    double radiusError = currentRadius - m_orbitRadius;

    if (fabs(radiusError) > 1.0) {
        GVector correction = toCenter / currentRadius * radiusError * ORBIT_RADIUS_CORRECTION;
        m_position += correction;
        mySE->SetPosition(m_position);
    }

    if (is_log_enabled(DESTINY__ORBIT_TRACE)) {
        _log(DESTINY__ORBIT_TRACE, "Destiny::UpdateOrbit() - %s(%u) radius:%.2f, angle:%.3f, speed:%.2f, error:%.2f",
            mySE->GetName(), mySE->GetID(), currentRadius, m_orbitAngle, m_velocity.length(), radiusError);
    }
}

// ===== УПРОЩЁННАЯ ФИЗИКА ДЛЯ NPC И ДРОНОВ =====

void DestinyManager::MoveObjectNPC() {
    if (mySE->SysBubble() == nullptr)
        mySE->SystemMgr()->AddEntity(mySE);

    // Простое движение как в старом коде
    if (m_velocity.length() < 0.001) {
        return;
    }

    SetPosition(m_position + m_velocity);
    
    if (m_velocity.length() > 0.001) {
        m_heading = m_velocity / m_velocity.length();
    }
    
    SendPhysicsUpdate();
}

void DestinyManager::ApplyBrakingNPC(double dt) {
    if (dt > 1.1) dt = 1.1;
    if (dt < 0.0001) dt = 0.0001;

    double speed = m_velocity.length();
    if (speed < 0.01) {
        m_velocity = GVector(0, 0, 0);
        return;
    }

    // Простое линейное торможение
    double newSpeed = speed * (1.0 - dt * 0.5);
    if (newSpeed < 0.01) newSpeed = 0;

    if (newSpeed > 0) {
        m_velocity = (m_velocity / speed) * newSpeed;
    } else {
        m_velocity = GVector(0, 0, 0);
    }

    SetPosition(m_position + m_velocity);
    SendPhysicsUpdate();
}

void DestinyManager::UpdateOrbitNPC(double dt) {
    if (!m_targetEntity.second) return;

    GPoint targetPos = m_targetEntity.second->GetPosition();
    GVector toTarget = targetPos - m_position;
    double distance = toTarget.length();

    if (distance < 0.001) {
        ApplyBrakingNPC(dt);
        return;
    }

    GVector direction = toTarget / distance;
    
    // Поворачиваем направление на 90 градусов для орбиты
    GVector orbitDir(-direction.z, 0, direction.x);
    orbitDir.normalize();

    // Скорость орбиты - 70% от максимальной
    double speed = m_maxSpeed * m_userSpeedFraction * 0.7;
    if (speed < 0.01) speed = 0.01;

    m_velocity = orbitDir * speed;
    SetPosition(m_position + m_velocity * dt);
    m_heading = orbitDir;
    SendPhysicsUpdate();
}

void DestinyManager::SetSpeedFractionNPC(double fraction) {
    // Простая установка скорости для NPC/дронов
    m_userSpeedFraction = fraction;
    m_maxSpeed = m_maxShipSpeed * fraction;
    m_activeSpeedFraction = fraction;
    m_timeFraction = 1.0;
    
    if (fraction < 0.01) {
        m_velocity = GVector(0, 0, 0);
        m_stop = true;
    }
}

// ===== ОТПРАВКА ОБНОВЛЕНИЙ КЛИЕНТУ =====
void DestinyManager::SendPhysicsUpdate() {
    std::vector<PyTuple*> updates;

    SetBallPosition posPacket;
    posPacket.entityID = mySE->GetID();
    posPacket.x = m_position.x;
    posPacket.y = m_position.y;
    posPacket.z = m_position.z;
    updates.push_back(posPacket.Encode());

    SetBallVelocity velPacket;
    velPacket.entityID = mySE->GetID();
    velPacket.x = m_velocity.x;
    velPacket.y = m_velocity.y;
    velPacket.z = m_velocity.z;
    updates.push_back(velPacket.Encode());

    SendDestinyUpdate(updates);
}

// ===== SET SPEED FRACTION =====
void DestinyManager::SetSpeedFraction(double fraction, bool startMovement) {
    if (m_orbiting != 0)
        Orbit(m_targetEntity.second, m_targetDistance);

    if ((fraction == m_userSpeedFraction) and (!startMovement)) {
        return;
    }

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::SetSpeedFraction() - %s(%u): fraction: %.2f, start: %s, stop: %s",
             mySE->GetName(), mySE->GetID(), fraction, startMovement ? "true" : "false", m_stop ? "true" : "false");

    if (startMovement) {
        m_stop = false;
        if (m_ballMode == Destiny::Ball::Mode::STOP)
            m_ballMode = Destiny::Ball::Mode::GOTO;
    }

    if (m_stop)
        return;

    m_userSpeedFraction = fraction;

    if (m_ballMode == Destiny::Ball::Mode::WARP) {
        m_ballMode = Destiny::Ball::Mode::GOTO;
        return;
    }

    std::vector<PyTuple*> updates;
    if (fraction > 0.001) {
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

// ===== STOP =====
void DestinyManager::Stop() {
    if (AbortIfLoginWarping(false)) {
        return;
    }

    if (mySE->HasPilot()) {
        mySE->GetPilot()->SetAutoPilot(false);
    }

    if (m_userSpeedFraction == 0.0) {
        m_stop = true;
    } else if ((m_ballMode == Destiny::Ball::Mode::WARP) and (!IsWarping())) {
        m_ballMode = Destiny::Ball::Mode::STOP;
    } else if (IsMoving()) {
        m_ballMode = Destiny::Ball::Mode::STOP;
    }

    m_userSpeedFraction = 0.0;
    m_stop = true;
    m_stateStamp = sEntityList.GetStamp();

    ClearOrbit();

    CmdStop du;
    du.entityID = mySE->GetID();
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

// ===== HALT =====
void DestinyManager::Halt() {
    SafeDelete(m_warpState);

    m_ballMode = Destiny::Ball::Mode::STOP;
    m_stop = true;
    m_maxSpeed = 0.0;
    m_velocity = GVector(0, 0, 0);
    m_heading = GVector(0, 0, 1);
    m_targetVelocity = GVector(0, 0, 0);
    m_oldVelocity = GVector(0, 0, 0);
    m_moveTime = 0.0;
    m_prevSpeed = 0.0;
    m_stateStamp = 0;
    m_targetPoint = GPoint(NULL_ORIGIN);
    m_stopDistance = 0;
    m_targetDistance = 0;
    m_followDistance = 0;
    m_userSpeedFraction = 0.0;
    m_activeSpeedFraction = 0.0;
    m_orbitAngle = 0.0;
    m_orbitRadius = 0.0;
    m_angularVelocity = 0.0;
    m_roll = 0.0;
    m_rollSpeed = 0.0;

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::Halt() - %s(%u): velocity: %.3f,%.3f,%.3f heading: %.3f,%.3f,%.3f",
                mySE->GetName(), mySE->GetID(), m_velocity.x, m_velocity.y, m_velocity.z,
                m_heading.x, m_heading.y, m_heading.z);
}

void DestinyManager::Eject() {
    Stop();
    UpdateOldShip(mySE->GetShipSE());
    SendJettisonPacket();
}

void DestinyManager::CheckBump() {
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

void DestinyManager::Bump(SystemEntity* pSE) {
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
    m_userSpeedFraction = 1.0;
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = direction * speed;

    std::vector<PyTuple*> updates;
    SetBallVelocity bv;
    bv.entityID = mySE->GetID();
    bv.x = m_velocity.x;
    bv.y = m_velocity.y;
    bv.z = m_velocity.z;
    updates.push_back(bv.Encode());
    CmdGotoDirection du;
    du.entityID = mySE->GetID();
    du.x = direction.x;
    du.y = direction.y;
    du.z = direction.z;
    updates.push_back(du.Encode());
    SendDestinyUpdate(updates);
    Stop();
}

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

// ===== ORBIT =====
void DestinyManager::Orbit(SystemEntity *pSE, uint32 distance) {
    if ((m_ballMode == Destiny::Ball::Mode::ORBIT)
    and (m_targetEntity.second == pSE)
    and (m_targetDistance == static_cast<double>(distance)))
        return;

    m_ballMode = Destiny::Ball::Mode::ORBIT;
    m_orbiting = Destiny::Ball::Orbit::Orbiting;
    m_targetEntity.first = pSE->GetID();
    m_targetEntity.second = pSE;
    m_orbitCenter = pSE->GetPosition();
    m_orbitRadius = static_cast<double>(distance);
    m_orbitAngle = 0.0;
    m_orbitalSpeedRatio = ORBITAL_SPEED_RATIO;
    m_targetDistance = static_cast<double>(distance);

    double orbitalSpeed = m_maxSpeed * m_orbitalSpeedRatio;
    if (m_orbitRadius > 0.001) {
        m_angularVelocity = orbitalSpeed / m_orbitRadius;
    } else {
        m_angularVelocity = 0.01;
    }

    BeginMovement();

    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "%s(%u) - Orbit started - radius:%.2f, angularVel:%.5f",
            mySE->GetName(), mySE->GetID(), m_orbitRadius, m_angularVelocity);

    CmdOrbit du;
    du.entityID = mySE->GetID();
    du.orbitEntityID = pSE->GetID();
    du.distance = (int32)m_targetDistance;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
}

// ===== ORBIT (ЗАГЛУШКА ДЛЯ СОВМЕСТИМОСТИ) =====
void DestinyManager::Orbit() {
    if (m_targetEntity.second) {
        Orbit(m_targetEntity.second, static_cast<uint32>(m_targetDistance));
    }
}

// ===== FOLLOW =====
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
    m_stoppingDistance = distance;
    m_ticAlign = true;
    BeginMovement();

    CmdFollowBall du;
    du.entityID = mySE->GetID();
    du.targetID = pSE->GetID();
    du.range = (int32)distance;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
}

// ===== FOLLOW (ЗАГЛУШКА ДЛЯ СОВМЕСТИМОСТИ) =====
void DestinyManager::Follow() {
    if (m_targetEntity.second) {
        Follow(m_targetEntity.second, static_cast<uint32>(m_followDistance));
    } else {
        Stop();
    }
}

void DestinyManager::ClearOrbit() {
    m_orbiting = Destiny::Ball::Orbit::None;
    m_orbitAngle = 0.0;
    m_orbitRadius = 0.0;
    m_angularVelocity = 0.0;
    m_targetDistance = 0;
    m_followDistance = 0;
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

    m_stop = false;

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

    if (IsCloaked())
        UnCloak();

    // Для NPC и дронов используем простую установку скорости
    if (mySE->IsNPCSE() || mySE->IsDroneSE()) {
        if (m_userSpeedFraction < 0.02) {
            SetSpeedFractionNPC(1.0);
        } else {
            SetSpeedFractionNPC(m_userSpeedFraction);
        }
    } else {
        // Для кораблей используем сложную установку скорости
        if (m_userSpeedFraction < 0.02) {
            SetSpeedFraction(1.0, true);
        } else {
            SetSpeedFraction(m_userSpeedFraction, true);
        }
    }

    SetPosition(m_position, sConfig.debug.PositionHack);
}

void DestinyManager::AlignTo(SystemEntity* ent) {
    Follow(ent, 0);
}

void DestinyManager::GotoDirection(const GPoint& direction) {
    if (m_orbiting)
        ClearOrbit();

    m_ballMode = Destiny::Ball::Mode::GOTO;
    m_targetPoint = direction * 1.0e16;
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

// ===== ИНИЦИАЛИЗАЦИЯ ВАРПА =====
void DestinyManager::InitWarp() {
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
            "longer warp distance dictates that warp cruise time is warranted"
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

    WarpUpdate(currentShipSpeed);
}

void DestinyManager::WarpCruise(uint16 sec_into_warp) {
    m_targetDistance -= m_warpState->warpSpeed;

    if ((m_targetDistance - m_warpState->warpSpeed) < m_warpState->decelDist) {
        m_warpState->cruise = false;
        m_warpState->decel = true;
    }

    WarpUpdate(m_warpState->warpSpeed);
}

void DestinyManager::WarpDecel(uint16 sec_into_warp) {
    uint8 decelTime = (sec_into_warp - m_warpDecelTime);
    double currentDistance = (m_warpState->total_distance - (exp(-decelTime) * m_warpState->decelDist));
    m_targetDistance = static_cast<double>(m_warpState->total_distance - currentDistance);
    double currentShipSpeed = (m_warpState->warpSpeed * exp(-decelTime));

    WarpUpdate(currentShipSpeed);
    if (currentShipSpeed <= m_speedToLeaveWarp)
        WarpStop(currentShipSpeed);
}

void DestinyManager::WarpUpdate(double currentShipSpeed) {
    m_velocity = (m_warpState->warp_vector * currentShipSpeed);
    SetPosition(m_targetPoint - (m_warpState->warp_vector * m_targetDistance));

    if (m_targBubble->InBubble(m_position, true)) {
        m_targBubble->Add(mySE);
        SetPosition(m_position, true);
    } else {
        SystemBubble* midWarpSystemBubble(sBubbleMgr.GetBubble(mySE->SystemMgr(), m_position));
        midWarpSystemBubble->Add(mySE);
    }
}

void DestinyManager::WarpStop(double currentShipSpeed) {
    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__WARP_TRACE, 
            "Destiny::WarpStop(): %s(%u) - Warp complete. Exit velocity %.4f m/s with %.2f m left to go.",
            mySE->GetName(), mySE->GetID(), currentShipSpeed, m_targetDistance
        );
    }
    
    // ===== 1. ОБНОВЛЯЕМ ПАРАМЕТРЫ КОРАБЛЯ =====
    UpdateShipVariables();
    
    // ===== 2. СОХРАНЯЕМ ВАРП-ВЕКТОР ДЛЯ ФИНАЛЬНОЙ КОРРЕКЦИИ =====
    GVector warpVector = m_warpState->warp_vector;
    
    // ===== 3. СБРАСЫВАЕМ ВСЕ СКОРОСТИ =====
    m_velocity = GVector(0, 0, 0);
    m_oldVelocity = GVector(0, 0, 0);
    m_targetVelocity = GVector(0, 0, 0);
    
    // ===== 4. ВОССТАНАВЛИВАЕМ НОРМАЛЬНЫЕ ПАРАМЕТРЫ =====
    m_maxSpeed = m_maxShipSpeed;
    m_userSpeedFraction = 0.0;
    m_activeSpeedFraction = 0.0;
    m_stop = true;
    m_ballMode = Destiny::Ball::Mode::STOP;
    m_stateStamp = sEntityList.GetStamp();
    
    // ===== 5. КОРРЕКТИРУЕМ ФИНАЛЬНУЮ ПОЗИЦИЮ =====
    m_targetPoint += (warpVector * 10000);
    
    // ===== 6. ОБНОВЛЯЕМ ПОЗИЦИЮ (ТОЛЬКО СЕРВЕР, БЕЗ ОТПРАВКИ КЛИЕНТУ) =====
    m_position = m_targetPoint;
    mySE->SetPosition(m_position);
    
    // ===== 7. ВЫЧИСЛЯЕМ HEADING =====
    CalculateHeading();
    
    // ===== 8. ВЫЧИСЛЯЕМ ROLL =====
    CalculateRoll(GetDeltaTime());
    
    // ===== 9. ОТПРАВЛЯЕМ ОДИН ПАКЕТ С ПОЗИЦИЕЙ И СКОРОСТЬЮ =====
    std::vector<PyTuple*> updates;
    
    // Позиция
    SetBallPosition posPacket;
    posPacket.entityID = mySE->GetID();
    posPacket.x = m_position.x;
    posPacket.y = m_position.y;
    posPacket.z = m_position.z;
    updates.push_back(posPacket.Encode());
    
    // Скорость (ноль)
    SetBallVelocity velPacket;
    velPacket.entityID = mySE->GetID();
    velPacket.x = 0;
    velPacket.y = 0;
    velPacket.z = 0;
    updates.push_back(velPacket.Encode());
    
    // Команда STOP для отключения клиентской физики
    CmdStop stopPacket;
    stopPacket.entityID = mySE->GetID();
    updates.push_back(stopPacket.Encode());
    
    // Отправляем одним пакетом
    SendDestinyUpdate(updates);
    
    // ===== 10. ПРОВЕРЯЕМ КОЛЛИЗИИ =====
    if (mySE->HasPilot())
        CheckBump();
    
    // ===== 11. ОЧИЩАЕМ СОСТОЯНИЕ ВАРПА =====
    SafeDelete(m_warpState);
    m_targBubble = nullptr;
    
    // ===== 12. ДЛЯ КОРАБЛЕЙ ИГРОКОВ =====
    if (mySE->IsShipSE()) {
        _log(AUTOPILOT__MESSAGE, 
            "Destiny::WarpStop(): %s(%u) - Warp complete.", 
            mySE->GetName(), mySE->GetID()
        );
        if (mySE->HasPilot()) {
            mySE->GetPilot()->SetLoginWarpComplete();
        }
    }
    
    // ===== 13. ДЛЯ NPC =====
    if ((mySE->IsNPCSE()) and (mySE->GetNPCSE()->GetAIMgr() != nullptr)) {
        mySE->GetNPCSE()->GetAIMgr()->WarpOutComplete();
    }
    
    // ===== 14. ЛОГИРУЕМ РЕЗУЛЬТАТ =====
    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        _log(DESTINY__MOVE_TRACE, 
            "WarpStop complete: %s(%u) pos=(%.2f,%.2f,%.2f) maxSpeed=%.2f, agility=%.3f, mass=%.2f, velocity=(%.2f,%.2f,%.2f)",
            mySE->GetName(), mySE->GetID(),
            m_position.x, m_position.y, m_position.z,
            m_maxShipSpeed, m_agility, m_mass,
            m_velocity.x, m_velocity.y, m_velocity.z
        );
    }
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

    // ===== РАСЧЕТ ДИСТАНЦИИ С УЧЕТОМ РАДИУСА =====
    double stopDistance = static_cast<double>(distance);
    double targetRadius = 0.0;
    double myRadius = mySE->GetRadius();
    double minDistance = 1000.0;  // Минимальная дистанция по умолчанию
    
    if (pSE) {
        targetRadius = pSE->GetRadius();
        
        // Минимальная дистанция = радиус цели + радиус корабля + запас
        minDistance = targetRadius + myRadius + 1000.0;  // 1км запас
        
        // Проверяем тип цели через существующие методы
        if (pSE->IsStationSE()) {
            // Для станций - паркуемся рядом
            minDistance = targetRadius + myRadius + 500.0;
            
            // Используем существующий метод GetDockPosY
            double dockY = stDataMgr.GetDockPosY(pSE->GetID());
            if (dockY > 0) {
                // Если есть точка дока - используем её Y координату
                // Но не меняем where, так как это const
                _log(DESTINY__WARP_TRACE, 
                    "WarpTo: Station %s(%u) dockY=%.2f, radius=%.2f",
                    pSE->GetName(), pSE->GetID(), dockY, targetRadius);
            }
        }
        else if (pSE->IsShipSE()) {
            // Для кораблей - держимся на расстоянии
            minDistance = targetRadius + myRadius + 2000.0;  // 2км до корабля
        }
        // Убираем IsStructureSE - его нет
    }
    
    // Используем переданную дистанцию или минимальную
    if (stopDistance < minDistance) {
        _log(DESTINY__WARP_TRACE, 
            "WarpTo: Adjusted stopDistance from %.2f to %.2f (minDistance)",
            stopDistance, minDistance);
        stopDistance = minDistance;
    }
    
    m_stopDistance = static_cast<int32>(stopDistance);
    
    // ===== РАСЧЕТ КОНЕЧНОЙ ТОЧКИ =====
    GVector warp_distance(m_position, where);
    double totalDistance = warp_distance.length();
    m_targetDistance = totalDistance - stopDistance;
    
    // Защита от отрицательной дистанции
    if (m_targetDistance < 100.0) {
        _log(DESTINY__WARNING, 
            "WarpTo: Target too close! totalDist=%.2f, stopDist=%.2f, targetDist=%.2f",
            totalDistance, stopDistance, m_targetDistance);
        m_targetDistance = 100.0;
        stopDistance = totalDistance - 100.0;
        m_stopDistance = static_cast<int32>(stopDistance);
    }
    
    // Вычисляем точку назначения
    warp_distance.normalize();
    warp_distance *= stopDistance;
    m_targetPoint = where - warp_distance;
    
    // Проверяем, что точка не внутри цели
    if (pSE && pSE->GetRadius() > 0) {
        double distToTarget = m_position.distance(m_targetPoint);
        if (distToTarget < pSE->GetRadius()) {
            // Если точка внутри цели - корректируем
            GVector correction(m_targetPoint, where);
            correction.normalize();
            correction *= (pSE->GetRadius() + myRadius + 500.0);
            m_targetPoint = where - correction;
            
            _log(DESTINY__WARNING, 
                "WarpTo: Corrected target point (was inside target radius)");
        }
    }

    // ===== ПОЛУЧАЕМ БАБЛ ДЛЯ ТОЧКИ НАЗНАЧЕНИЯ =====
    m_targBubble = sBubbleMgr.GetBubble(mySE->SystemMgr(), m_targetPoint);
    
    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__TRACE, 
            "Destiny::WarpTo() - %s(%u) target bubble: %u  stopDistance: %.2f  targetDistance: %.2f  targetRadius: %.2f",
            mySE->GetName(), mySE->GetID(), 
            m_targBubble ? m_targBubble->GetID() : 0,
            stopDistance, m_targetDistance, targetRadius);
    }

    // ===== ПРОВЕРКА КАПА =====
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
        double capNeeded = m_mass * m_warpCapacitorNeed * (m_targetDistance / static_cast<double>(ONE_AU_IN_METERS));
        capNeeded *= (1.0 - (0.1 * pClient->GetChar()->GetSkillLevel(EvESkill::WarpDriveOperation)));

        if (capNeeded > currentShipCap) {
            capNeeded = (currentShipCap / m_warpCapacitorNeed) / m_mass;
            if (capNeeded > 1) {
                m_targetDistance = capNeeded * static_cast<double>(ONE_AU_IN_METERS);
                GVector warp_direction(m_position, where);
                GPoint newTarget(m_position + (warp_direction * m_targetDistance));
                m_targBubble = sBubbleMgr.GetBubble(mySE->SystemMgr(), newTarget);
            } else {
                pClient->SendErrorMsg("You don't have enough capacitor charge to warp.");
                _log(DESTINY__WARNING, 
                    "Destiny::WarpTo() - %s(%u): Capacitor needed vs current  %.3f / %.3f",
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

    // ===== ОТПРАВКА ПАКЕТОВ =====
    if (m_targBubble && m_targBubble->HasWarpBubble()) {
        if (!mySE->GetSelf()->HasAttribute(AttrWarpBubbleImmune)) {
            // Попали в варп-бабл
            // ...
        }
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

    SetBallMassive bm;
    bm.entityID = mySE->GetID();
    bm.is_massive = false;
    PyTuple *up = bm.Encode();
    SendSingleDestinyUpdate(&up, true);

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__WARP_TRACE,
            "Destiny::WarpTo() toBubble:%u from:%u, targetPoint: (%.2f,%.2f,%.2f) stopDistance: %i targetDistance: %.2f",
            m_targBubble ? m_targBubble->GetID() : 0,
            mySE->SysBubble() ? mySE->SysBubble()->GetID() : 0,
            m_targetPoint.x, m_targetPoint.y, m_targetPoint.z,
            m_stopDistance, m_targetDistance
        );
    }
}

// ===== ОСТАЛЬНЫЕ МЕТОДЫ =====

bool DestinyManager::IsAligned(GPoint& targetPoint) {
    GVector toVec(m_position, targetPoint);
    toVec.normalize();
    double dot = toVec.dotProduct(m_heading);
    double degrees = EvE::Trig::Rad2Deg(std::acos(dot));
    if (degrees < TURN_ALIGNMENT)
        return true;
    return false;
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

bool DestinyManager::IsTargetInvalid() {
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

void DestinyManager::Undock(GPoint dir) {
    m_targetPoint = dir * 1.0e16;
    m_heading = GVector(dir);
    SetUndockSpeed();
    if (mySE->IsShipSE())
        mySE->GetShipSE()->GetShipItemRef()->SetUndocking(false);
}

void DestinyManager::SetUndockSpeed() {
    m_stop = false;
    m_orbiting = 0;
    m_stateStamp = sEntityList.GetStamp();
    m_changeDelay = true;
    m_userSpeedFraction = 1.1;
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = m_heading * m_maxSpeed;
    m_activeSpeedFraction = 1.1;

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
    du.x = m_heading.x;
    du.y = m_heading.y;
    du.z = m_heading.z;
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
        AlignTo(station);
        if (mySE->HasPilot() and mySE->GetPilot()->CanThrow())
            throw UserError ("DockingApproach");
    }

    pClient->SetStateTimer(Player::State::Dock, sConfig.world.StationDockDelay *1000);
    pClient->SetAutoPilot(false);

    return new PyLong(GetFileTimeNow());
}

void DestinyManager::DockingAccepted() {
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
            _log(DESTINY__TRACE, "Destiny::SetMaxVelocity() - Ship:%s(%u) Pilot:%s(%u) - AttrMaxDirectionalVelocity is %.1f, maxSpeed is %.1f, update is %.1f",
                    mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID(),
                    mySE->GetSelf()->GetAttribute(AttrMaxDirectionalVelocity).get_float(), maxSpeed, maxVelocity);

    if (maxVelocity > maxSpeed) {
        m_maxShipSpeed = maxSpeed;
    } else {
        m_maxShipSpeed = maxVelocity;
    }
}

void DestinyManager::SpeedBoost(bool deactivate) {
    m_mass = mySE->GetSelf()->GetAttribute(AttrMass).get_float();
    m_massMKg = m_mass / 1000000;
    m_shipAgility = m_massMKg * m_shipInertia;
    m_agility = m_shipAgility;

    m_maxShipSpeed = mySE->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();
    m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;

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
        _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost() - mass: %.5f, agility: %.5f, maxSpeed: %.2f, thrustPower: %.2f",
                m_mass, m_agility, m_maxShipSpeed, m_thrustPower);
    }

    SetSpeedFraction(m_userSpeedFraction, true);
}

void DestinyManager::WebbedMe(InventoryItemRef modRef, bool apply) {
    if (apply) {
        m_maxShipSpeed *= (1 + (modRef->GetAttribute(AttrSpeedFactor).get_float() / 100.0));
    } else {
        m_maxShipSpeed /= (1 + (modRef->GetAttribute(AttrSpeedFactor).get_float() / 100.0));
    }
    std::vector<PyTuple*> updates;
    SetBallSpeed sbms;
    sbms.entityID = mySE->GetID();
    sbms.speed = m_maxShipSpeed;
    updates.push_back(sbms.Encode());
    SendDestinyUpdate(updates);
    m_hasSentShipUpdates = true;

    SetSpeedFraction(m_userSpeedFraction, true);
}

// ===== ОБНОВЛЕНИЕ ПАРАМЕТРОВ КОРАБЛЯ =====
void DestinyManager::UpdateShipVariables() {
    InventoryItemRef sRef = mySE->GetSelf();
    m_mass = sRef->GetAttribute(AttrMass).get_float();
    m_massMKg = m_mass / 1000000;

    if (sRef->HasAttribute(AttrWarpSpeedMultiplier))
        m_shipWarpSpeed = sRef->GetAttribute(AttrWarpSpeedMultiplier).get_float() * 3.0; // стандартную скорсть 3 а.е. домножаем на модификатор
    if (sRef->HasAttribute(AttrInetia))
        m_shipInertia = sRef->GetAttribute(AttrInetia).get_float();
    if (sRef->HasAttribute(AttrMaxVelocity))
        m_maxShipSpeed = sRef->GetAttribute(AttrMaxVelocity).get_float();
    if (sRef->HasAttribute(AttrWarpCapacitorNeed))
        m_warpCapacitorNeed = sRef->GetAttribute(AttrWarpCapacitorNeed).get_float() * 2;

    if (mySE->IsNPCSE() or mySE->IsDroneSE())
        m_maxShipSpeed = sRef->GetAttribute(AttrEntityCruiseSpeed).get_float();

    if (m_maxShipSpeed < 0.1) {
        _log(DESTINY__WARNING, "UpdateShipVariables: maxShipSpeed is 0 for %s(%u)! Setting default 100 m/s",
             mySE->GetName(), mySE->GetID());
        m_maxShipSpeed = 100.0;
    }

    m_speedToLeaveWarp = m_maxShipSpeed * 0.75;
    if ((m_speedToLeaveWarp < 100) and (m_maxShipSpeed > 135))
        m_speedToLeaveWarp = 100;

    m_shipAgility = m_massMKg * m_shipInertia / m_physicsFriction;
    m_agility = m_shipAgility;

    if (m_agility < 0.001) {
        _log(DESTINY__WARNING, "UpdateShipVariables: agility too small (%.6f), setting to 1.0", m_agility);
        m_agility = 1.0;
    }

    m_shipMaxAccelTime = -log(0.01) * m_agility;
    m_alignTime = -log(0.25) * m_agility;
    m_timeToEnterWarp = m_alignTime;

    m_thrustPower = m_maxShipSpeed * m_mass / 10.0;

    _log(DESTINY__MOVE_TRACE,
        "=== UpdateShipVariables: %s(%u) mass=%.2f Mkg, inertia=%.3f, agility=%.3fs, maxSpeed=%.2f m/s, thrustPower=%.2f ===",
        mySE->GetName(), mySE->GetID(), m_massMKg, m_shipInertia, m_agility, m_maxShipSpeed, m_thrustPower);

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

void DestinyManager::MakeMissile(Missile* pMissile) {
    SetMaxVelocity(pMissile->GetSpeed());
    SetPosition(pMissile->GetSelf()->position());
    m_mass = pMissile->GetSelf()->type().mass();
    m_massMKg = m_mass / 1000000;
    m_shipInertia = pMissile->GetSelf()->GetAttribute(AttrInetia).get_float();
    m_shipAgility = m_massMKg * m_shipInertia / m_physicsFriction;
    m_agility = m_shipAgility;

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
    m_heading = moveVector;

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

// ===== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ =====

double DestinyManager::GetDeltaTime() {
    auto currentTime = std::chrono::steady_clock::now();
    
    auto dt_us = std::chrono::duration_cast<std::chrono::microseconds>(
        currentTime - m_lastDeltaTime).count();
    
    double dt = dt_us / 1000000.0;
    m_lastDeltaTime = currentTime;
    
    static int count = 0;
    count++;
    if (count % 60 == 0) {
        printf("[GetDeltaTime] dt = %.6f seconds (%.3f ms), dt_us = %lld\n", 
               dt, dt * 1000, (long long)dt_us);
        fflush(stdout);
    }
    
    return dt;
}

void DestinyManager::UpdateNewShip(const ShipItemRef newShipRef) {
    if (m_hasSentShipUpdates)
        return;

    Client* pClient = mySE->GetPilot();
    if (pClient == nullptr)
        return;
    PyDict* slim = new PyDict();
    slim->SetItemString("name", new PyString(newShipRef->itemName()));
    slim->SetItemString("itemID", new PyInt(newShipRef->itemID()));
    slim->SetItemString("typeID", new PyInt(newShipRef->typeID()));
    slim->SetItemString("ownerID", new PyInt(mySE->GetOwnerID()));
    slim->SetItemString("charID", new PyInt(pClient->GetCharacterID()));
    slim->SetItemString("corpID", IsCorp(mySE->GetCorporationID()) ? new PyInt(mySE->GetCorporationID()) : PyStatic.NewNone());
    slim->SetItemString("allianceID", IsAlliance(mySE->GetAllianceID()) ? new PyInt(mySE->GetAllianceID()) : PyStatic.NewNone());
    slim->SetItemString("warFactionID", IsFaction(mySE->GetWarFactionID()) ? new PyInt(mySE->GetWarFactionID()) : PyStatic.NewNone());
    slim->SetItemString("bounty", new PyFloat(pClient->GetBounty()));
    slim->SetItemString("securityStatus", new PyFloat(pClient->GetSecurityRating()));
    if (newShipRef->typeID() == itemTypeCapsule) {
        slim->SetItemString("launcherID", new PyInt(mySE->GetShipSE()->GetLauncherID()));
        slim->SetItemString("modules", new PyList());
    } else {
        slim->SetItemString("categoryID", new PyInt(newShipRef->categoryID()));
        slim->SetItemString("groupID", new PyInt(newShipRef->groupID()));
        slim->SetItemString("modules", newShipRef->ShipGetModuleList());
    }

    std::vector<PyTuple*> updates;
    PyTuple* shipData = new PyTuple(2);
    shipData->SetItem(0, new PyLong(newShipRef->itemID()));
    shipData->SetItem(1, new PyObject("foo.SlimItem", slim));
    PyTuple* shipItem = new PyTuple(2);
    shipItem->SetItem(0, new PyString("OnSlimItemChange"));
    shipItem->SetItem(1, shipData);
    updates.push_back(shipItem);
    SendDestinyUpdate(updates);

    UpdateShipVariables();
    SendBallInteractive(newShipRef, true);
}

void DestinyManager::UpdateOldShip(ShipSE* pShipSE) {
    if (pShipSE->IsDead())
        return;
    PyDict* slimPod = new PyDict();
    slimPod->SetItemString("itemID", new PyInt(pShipSE->GetID()));
    slimPod->SetItemString("typeID", new PyInt(pShipSE->GetTypeID()));
    slimPod->SetItemString("categoryID", new PyInt(pShipSE->GetCategoryID()));
    slimPod->SetItemString("ownerID", new PyInt(pShipSE->GetOwnerID()));
    slimPod->SetItemString("charID", PyStatic.NewNone());
    slimPod->SetItemString("corpID", new PyInt(pShipSE->GetCorporationID()));
    slimPod->SetItemString("allianceID", new PyInt(pShipSE->GetAllianceID()));
    slimPod->SetItemString("warFactionID", new PyInt(pShipSE->GetWarFactionID()));
    slimPod->SetItemString("bounty", PyStatic.NewNone());
    slimPod->SetItemString("securityStatus", PyStatic.NewNone());
    PyTuple* shipData = new PyTuple(2);
    shipData->SetItem(0, new PyLong(pShipSE->GetID()));
    shipData->SetItem(1, new PyObject("foo.SlimItem", slimPod));
    PyTuple* shipItem = new PyTuple(2);
    shipItem->SetItem(0, new PyString("OnSlimItemChange"));
    shipItem->SetItem(1, shipData);
    SendSingleDestinyUpdate(&shipItem);

    SendBallInteractive(pShipSE->GetShipItemRef(), false);
    m_hasSentShipUpdates = false;
}

void DestinyManager::Jump(bool showCloak) {
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

void DestinyManager::TractorBeamStart(SystemEntity* pShipSE, EvilNumber speed) {
    m_ballMode = Destiny::Ball::Mode::FOLLOW;

    m_stop = false;
    m_tractored = true;
    m_moveTime = GetTimeMSeconds();
    m_stateStamp = sEntityList.GetStamp();

    m_targetPoint = pShipSE->GetPosition();
    GVector moveVector(m_position, m_targetPoint);
    m_targetDistance = moveVector.length();
    moveVector.normalize();
    m_heading = moveVector;

    m_maxShipSpeed = speed.get_float();
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = m_heading * m_maxSpeed;

    m_followDistance = 500 + pShipSE->GetRadius();

    m_userSpeedFraction = 1.0;

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

void DestinyManager::TractorBeamStop() {
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

// ===== МЕТОДЫ ДЛЯ ОТПРАВКИ ПАКЕТОВ =====

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

void DestinyManager::SendSpecialEffect10(uint32 entityID, uint32 targetID, std::string guid, bool isOffensive, bool start, bool isActive) const {
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
                                       bool isActive, int32 duration, uint32 repeat, int32 graphicInfo) const {
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

void DestinyManager::SendMovementPacket() {
    SendSingleDestinyUpdate(&mvPacket);
    PySafeDecRef(mvPacket);
}

void DestinyManager::SendSingleDestinyEvent(PyTuple** ev, bool self_only) const {
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

void DestinyManager::SendDestinyUpdate(std::vector<PyTuple*>& updates, std::vector<PyTuple*>& events, bool self_only) const {
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
                mySE->SysBubble()->BubblecastDestiny(updates, events, "destiny");
            }
            return;
        }

        if (is_log_enabled(PLAYER__MESSAGE))
            _log(PLAYER__MESSAGE, "[%u] DestinyManager::SendDestinyUpdate() (u:%lu, e:%lu) called as 'self_only' for %s(%i)",
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

        mySE->SysBubble()->BubblecastDestiny(updates, events, "destiny");
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