/**
 * DroneAI.cpp
 *      this class is for drone AI
 *
 * @Author:     Allan
 * @Version:    0.15
 * @Date:       27Nov19
*/

#include "eve-server.h"

#include "Client.h"
#include "inventory/AttributeEnum.h"
#include "system/DestinyManager.h"
#include "npc/Drone.h"
#include "npc/DroneAI.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"
#include <iostream>

DroneAIMgr::DroneAIMgr(DroneSE* who)
: m_state(DroneAI::State::Idle),
  m_pDrone(who),
  m_assignedShip(nullptr),
  m_mainAttackTimer(0),
  m_processTimer(0),
  m_beginFindTarget(0),
  m_warpScramblerTimer(0),
  m_webifierTimer(0),
  m_hasAttackOrder(false),
  m_sigRadius(who->GetSelf()->GetAttribute(AttrSignatureRadius).get_float()),
  m_attackSpeed(who->GetSelf()->GetAttribute(AttrSpeed).get_float()),
  m_cruiseSpeed(who->GetSelf()->GetAttribute(AttrEntityCruiseSpeed).get_int()),
  m_chaseSpeed(who->GetSelf()->GetAttribute(AttrMaxVelocity).get_int()),
  m_entityFlyRange(who->GetSelf()->GetAttribute(AttrEntityFlyRange).get_float() + who->GetSelf()->GetAttribute(AttrMaxRange).get_float()),
  m_entityChaseRange(who->GetSelf()->GetAttribute(AttrEntityChaseMaxDistance).get_float() *2),
  m_entityOrbitRange(who->GetSelf()->GetAttribute(AttrMaxRange).get_float()),
  m_entityAttackRange(who->GetSelf()->GetAttribute(AttrEntityAttackRange).get_float() * 5),
  m_shieldBoosterDuration(who->GetSelf()->GetAttribute(AttrEntityShieldBoostDuration).get_int()),
  m_armorRepairDuration(who->GetSelf()->GetAttribute(AttrEntityArmorRepairDuration).get_int())
{
    m_processTimer.Start(5000);

    if (m_entityAttackRange < 15000) {
        m_entityAttackRange = 15000;
    }
    
    double maxRange = who->GetSelf()->GetAttribute(AttrMaxRange).get_float();
    double falloff = who->GetSelf()->GetAttribute(AttrFalloff).get_float();
    if (maxRange > 0) {
        m_entityOrbitRange = maxRange + (falloff * 0.5);
    }
    
    if (m_attackSpeed < 0.5) {
        m_attackSpeed = 1.0;
    }
    
    std::cout << "[DRONE] === DRONE CREATED === " << who->GetName() << "(" << who->GetID() 
              << "), attackSpeed=" << m_attackSpeed 
              << ", attackRange=" << m_entityAttackRange 
              << ", orbitRange=" << m_entityOrbitRange << std::endl;
}

void DroneAIMgr::Process() {
    if (!m_pDrone->IsEnabled()) {
        return;
    }

    switch(m_state) {
        case DroneAI::State::Idle: {
            if (m_hasAttackOrder) {
                SystemEntity* pTarget = m_pDrone->TargetMgr()->GetFirstTarget(true);
                if (!pTarget) {
                    uint32 targetID = m_pDrone->GetTargetID();
                    if (targetID != 0) {
                        SystemManager* pSysMgr = m_pDrone->GetSystemManager();
                        if (pSysMgr) {
                            pTarget = pSysMgr->GetSE(targetID);
                            if (pTarget) {
                                std::cout << "[DRONE] DRONE: IDLE - Restored target from m_targetID: " 
                                          << pTarget->GetName() << "(" << pTarget->GetID() << ")" << std::endl;
                            }
                        }
                    }
                }
                if (pTarget) {
                    std::cout << "[DRONE] DRONE: IDLE but HAS ATTACK ORDER, resuming on " 
                              << pTarget->GetName() << "(" << pTarget->GetID() << ")" << std::endl;
                    SetEngaged(pTarget);
                    return;
                }
            }
            
            if (m_assignedShip && m_state == DroneAI::State::Idle) {
                m_pDrone->IdleOrbit(m_assignedShip);
            }
            break;
        }
        
        case DroneAI::State::Combat:
        case DroneAI::State::Engaged: {
            std::cout << "[DRONE] DRONE " << m_pDrone->GetName() << "(" << m_pDrone->GetID() 
                      << "): PROCESS - ENGAGED STATE, attackOrder=" 
                      << (m_hasAttackOrder ? "TRUE" : "FALSE") << std::endl;
            
            SystemEntity* pTarget = m_pDrone->TargetMgr()->GetFirstTarget(true);
            
            if (!pTarget) {
                uint32 targetID = m_pDrone->GetTargetID();
                if (targetID != 0) {
                    SystemManager* pSysMgr = m_pDrone->GetSystemManager();
                    if (pSysMgr) {
                        pTarget = pSysMgr->GetSE(targetID);
                        if (pTarget) {
                            std::cout << "[DRONE] DRONE: Retrieved target from m_targetID: " 
                                      << pTarget->GetName() << "(" << pTarget->GetID() << ")" << std::endl;
                        }
                    }
                }
            }
            
            if (!pTarget) {
                if (m_hasAttackOrder) {
                    std::cout << "[DRONE] DRONE: No target but has attack order, waiting..." << std::endl;
                    return;
                }
                std::cout << "[DRONE] DRONE: No target and no attack order, SetIdle" << std::endl;
                SetIdle();
                return;
            }
            
            if (pTarget->SysBubble() == nullptr) {
                m_pDrone->TargetMgr()->ClearTarget(pTarget);
                return;
            }
            
            std::cout << "[DRONE] DRONE " << m_pDrone->GetName() << "(" << m_pDrone->GetID() 
                      << "): PROCESS - Calling FightTarget on " << pTarget->GetName() 
                      << "(" << pTarget->GetID() << ")" << std::endl;
            FightTarget(pTarget);
            break;
        }

        case DroneAI::State::Departing: {
            if (m_assignedShip) {
                double dist = m_pDrone->GetPosition().distance(m_assignedShip->GetPosition());
                if (dist < m_entityOrbitRange) {
                    SetIdle();
                } else {
                    m_pDrone->DestinyMgr()->Follow(m_assignedShip, 100.0f);
                }
            }
            break;
        }

        default:
            break;
    }
}

int8 DroneAIMgr::GetState() {
    switch (m_state) {
        case DroneAI::State::Invalid:
        case DroneAI::State::Unknown:
        case DroneAI::State::Incapacitated:
            return DroneAI::State::Idle;
        case DroneAI::State::Guarding:
        case DroneAI::State::Assisting:
            return DroneAI::State::Engaged;
        default:
            return m_state;
    }
}

void DroneAIMgr::Return() {
    std::cout << "[DRONE] DRONE " << m_pDrone->GetName() << "(" << m_pDrone->GetID() 
              << "): RETURN() called" << std::endl;
    
    m_assignedShip = m_pDrone->GetHomeShip();
    if (!m_assignedShip) {
        return;
    }
    
    m_hasAttackOrder = false;
    m_pDrone->DestinyMgr()->SetMaxVelocity(m_chaseSpeed);
    m_pDrone->DestinyMgr()->Follow(m_assignedShip, 100.0f);
    m_state = DroneAI::State::Departing;
    m_pDrone->TargetMgr()->ClearAllTargets();
}

void DroneAIMgr::StopAttack() {
    std::cout << "[DRONE] DRONE " << m_pDrone->GetName() << "(" << m_pDrone->GetID() 
              << "): STOPATTACK() called" << std::endl;
    m_hasAttackOrder = false;
    ClearAllTargets();
    SetIdle();
}

void DroneAIMgr::SetIdle() {
    if (m_state == DroneAI::State::Idle)
        return;
    
    std::cout << "[DRONE] DRONE " << m_pDrone->GetName() << "(" << m_pDrone->GetID() 
              << "): SETIDLE() called, state was " << GetStateName(m_state) << std::endl;
    
    m_state = DroneAI::State::Idle;
    m_hasAttackOrder = false;
    
    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_mainAttackTimer.Disable();
    m_warpScramblerTimer.Disable();
    
    if (m_assignedShip) {
        m_pDrone->IdleOrbit(m_assignedShip);
    }
}

void DroneAIMgr::SetEngaged(SystemEntity* pTarget) {
    std::cout << "[DRONE] DRONE " << m_pDrone->GetName() << "(" << m_pDrone->GetID() 
              << "): === SETENGAGED() CALLED === Target: " 
              << (pTarget ? pTarget->GetName() : "NULL") 
              << "(" << (pTarget ? pTarget->GetID() : 0) << ")" << std::endl;
    
    if (!pTarget) {
        std::cout << "[DRONE] DRONE: SETENGAGED - NULL target!" << std::endl;
        return;
    }
    
    if (m_state == DroneAI::State::Engaged && m_hasAttackOrder) {
        std::cout << "[DRONE] DRONE: Already engaged with attack order, skipping" << std::endl;
        return;
    }
    
    m_hasAttackOrder = true;
    double orbitRange = m_entityOrbitRange;
    if (orbitRange < 500) orbitRange = 500;
    
    m_pDrone->DestinyMgr()->SetMaxVelocity(MakeRandomFloat(m_cruiseSpeed, (m_chaseSpeed / 4)));
    m_pDrone->DestinyMgr()->Orbit(pTarget, orbitRange);
    m_state = DroneAI::State::Engaged;
    
    if (!m_mainAttackTimer.Enabled()) {
        m_mainAttackTimer.Start(m_attackSpeed * 1000);
        std::cout << "[DRONE] DRONE: Attack timer STARTED, interval=" << (m_attackSpeed * 1000) << " ms" << std::endl;
    } else {
        std::cout << "[DRONE] DRONE: Attack timer ALREADY enabled" << std::endl;
    }
    
    std::cout << "[DRONE] DRONE " << m_pDrone->GetName() << "(" << m_pDrone->GetID() 
              << "): SETENGAGED COMPLETE, state=Engaged, attackOrder=TRUE" << std::endl;
}

void DroneAIMgr::FightTarget(SystemEntity* pTarget) {
    std::cout << "[DRONE] DRONE " << m_pDrone->GetName() << "(" << m_pDrone->GetID() 
              << "): === FIGHTTARGET() CALLED === Target: " 
              << (pTarget ? pTarget->GetName() : "NULL") 
              << "(" << (pTarget ? pTarget->GetID() : 0) << ")" << std::endl;
    
    if (!pTarget) {
        std::cout << "[DRONE] DRONE: FIGHTTARGET - NULL target!" << std::endl;
        return;
    }
    
    double orbitRange = m_entityOrbitRange;
    if (orbitRange < 500) orbitRange = 500;
    
    m_pDrone->DestinyMgr()->Orbit(pTarget, orbitRange);
    
    // ПРЯМОЙ ВЫЗОВ АТАКИ (без таймера)
    AttackTarget(pTarget);
}

void DroneAIMgr::CheckDistance(SystemEntity* pSE) {
    FightTarget(pSE);
}

void DroneAIMgr::ClearTargets() {
    m_pDrone->TargetMgr()->ClearTargets();
}

void DroneAIMgr::ClearAllTargets() {
    m_pDrone->TargetMgr()->ClearAllTargets();
}

void DroneAIMgr::Target(SystemEntity* pTarget) {
    bool chase = false;
    if (!m_pDrone->TargetMgr()->StartTargeting(pTarget, m_pDrone->GetSelf()->GetAttribute(AttrScanSpeed).get_uint32(), 
                                               (uint8)m_pDrone->GetSelf()->GetAttribute(AttrMaxAttackTargets).get_int(), 
                                               m_entityAttackRange, chase)) {
        std::cout << "[DRONE] DRONE: Targeting of " << pTarget->GetName() << "(" << pTarget->GetID() << ") FAILED" << std::endl;
        SetIdle();
        return;
    }
    m_beginFindTarget.Disable();
    FightTarget(pTarget);
}

void DroneAIMgr::Targeted(SystemEntity* pAgressor) {
    std::cout << "[DRONE] DRONE " << m_pDrone->GetName() << "(" << m_pDrone->GetID() 
              << "): Targeted by " << pAgressor->GetName() << "(" << pAgressor->GetID() << ")" << std::endl;
}

void DroneAIMgr::TargetLost(SystemEntity* pTarget) {
    std::cout << "[DRONE] DRONE " << m_pDrone->GetName() << "(" << m_pDrone->GetID() 
              << "): TARGETLOST - " << (pTarget ? pTarget->GetName() : "NULL") 
              << "(" << (pTarget ? pTarget->GetID() : 0) << ")" << std::endl;
    
    switch(m_state) {
        case DroneAI::State::Engaged: {
            if (m_pDrone->TargetMgr()->HasNoTargets()) {
                if (m_hasAttackOrder) {
                    std::cout << "[DRONE] DRONE: Target lost but has attack order, waiting" << std::endl;
                } else {
                    SetIdle();
                }
            }
        } break;
        default:
            break;
    }
}

void DroneAIMgr::Attack(SystemEntity* pSE) {
    std::cout << "[DRONE] DRONE " << m_pDrone->GetName() << "(" << m_pDrone->GetID() 
              << "): === ATTACK() CALLED (DEPRECATED) ===" << std::endl;
}

void DroneAIMgr::ClearTarget(SystemEntity* pSE) {
    m_pDrone->TargetMgr()->ClearTarget(pSE);
    if (m_pDrone->TargetMgr()->HasNoTargets()) {
        if (!m_hasAttackOrder) {
            SetIdle();
        }
    }
}

void DroneAIMgr::AttackTarget(SystemEntity* pTarget) {
    std::cout << "[DRONE] DRONE " << m_pDrone->GetName() << "(" << m_pDrone->GetID() 
              << "): === ATTACKTARGET() CALLED === Target: " 
              << (pTarget ? pTarget->GetName() : "NULL") 
              << "(" << (pTarget ? pTarget->GetID() : 0) << ")" << std::endl;
    
    if (!pTarget) {
        std::cout << "[DRONE] DRONE: AttackTarget - NULL target!" << std::endl;
        return;
    }
    
    // Проверка: не врек/контейнер
    InventoryItemRef pTargetItem = pTarget->GetSelf();
    if (pTargetItem) {
        uint32 groupID = pTargetItem->groupID();
        if (groupID == 6 || groupID == 7 || groupID == 365) {
            std::cout << "[DRONE] DRONE: Target is a wreck/container, stopping attack!" << std::endl;
            ClearTarget(pTarget);
            SetIdle();
            return;
        }
        
        float hp = pTargetItem->GetAttribute(AttrHP).get_float();
        if (hp <= 0) {
            std::cout << "[DRONE] DRONE: Target has 0 HP, stopping attack!" << std::endl;
            ClearTarget(pTarget);
            SetIdle();
            return;
        }
    }
    
    std::cout << "[DRONE] DRONE " << m_pDrone->GetName() << "(" << m_pDrone->GetID() 
              << "): === APPLYING DAMAGE to " << pTarget->GetName() 
              << "(" << pTarget->GetID() << ") ===" << std::endl;
    
    std::string guid = "effects.Laser";
    uint32 gfxID = 0;
    if (m_pDrone->GetSelf()->HasAttribute(AttrGfxTurretID))
        gfxID = m_pDrone->GetSelf()->GetAttribute(AttrGfxTurretID).get_uint32();
    
    m_pDrone->DestinyMgr()->SendSpecialEffect(m_pDrone->GetSelf()->itemID(),
                                             m_pDrone->GetSelf()->itemID(),
                                             m_pDrone->GetSelf()->typeID(),
                                             pTarget->GetID(),
                                             0, guid, 1, 1, 1, m_attackSpeed, 0, gfxID);

    Damage d(m_pDrone,
             m_pDrone->GetSelf(),
             m_pDrone->GetKinetic(),
             m_pDrone->GetThermal(),
             m_pDrone->GetEM(),
             m_pDrone->GetExplosive(),
             m_formula.GetDroneToHit(m_pDrone, pTarget),
             EVEEffectID::targetAttack
            );

    d *= m_pDrone->GetSelf()->GetAttribute(AttrDamageMultiplier).get_float();
    d *= sConfig.rates.damageRate;
    
    std::cout << "[DRONE] DRONE: Damage applied to " << pTarget->GetName() 
              << "(" << pTarget->GetID() << ")" << std::endl;
    
    pTarget->ApplyDamage(d);
}

std::string DroneAIMgr::GetStateName(int8 stateID) {
    switch (stateID) {
        case DroneAI::State::Idle:            return "Idle";
        case DroneAI::State::Combat:          return "Combat";
        case DroneAI::State::Mining:          return "Mining";
        case DroneAI::State::Approaching:     return "Approaching";
        case DroneAI::State::Departing:       return "Returning to ship";
        case DroneAI::State::Departing2:      return "Departing2";
        case DroneAI::State::Pursuit:         return "Pursuit";
        case DroneAI::State::Engaged:         return "Engaged";
        case DroneAI::State::Fleeing:         return "Fleeing";
        case DroneAI::State::Unknown:         return "Unknown";
        case DroneAI::State::Operating:       return "Operating";
        case DroneAI::State::Assisting:       return "Assisting";
        case DroneAI::State::Guarding:        return "Guarding";
        case DroneAI::State::Incapacitated:   return "Incapacitated";
        default:                              return "Invalid";
    }
}