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

DroneAIMgr::DroneAIMgr(DroneSE* who)
: m_state(DroneAI::State::Idle),
  m_pDrone(who),
  m_assignedShip(nullptr),
  m_mainAttackTimer(0),// dont start timer until we have a target
  m_processTimer(0),
  m_beginFindTarget(0),
  m_warpScramblerTimer(0),     //not implemented yet
  m_webifierTimer(0),             //not implemented yet
  m_sigRadius(who->GetSelf()->GetAttribute(AttrSignatureRadius).get_float()),
  m_attackSpeed(who->GetSelf()->GetAttribute(AttrSpeed).get_float()),
  m_cruiseSpeed(who->GetSelf()->GetAttribute(AttrEntityCruiseSpeed).get_int()),
  m_chaseSpeed(who->GetSelf()->GetAttribute(AttrMaxVelocity).get_int()),
  m_entityFlyRange(who->GetSelf()->GetAttribute(AttrEntityFlyRange).get_float() + who->GetSelf()->GetAttribute(AttrMaxRange).get_float()),
  m_entityChaseRange(who->GetSelf()->GetAttribute(AttrEntityChaseMaxDistance).get_float() *2),
  m_entityOrbitRange(who->GetSelf()->GetAttribute(AttrMaxRange).get_float()),
  m_entityAttackRange(who->GetSelf()->GetAttribute(AttrEntityAttackRange).get_float() *2),
  m_shieldBoosterDuration(who->GetSelf()->GetAttribute(AttrEntityShieldBoostDuration).get_int()),
  m_armorRepairDuration(who->GetSelf()->GetAttribute(AttrEntityArmorRepairDuration).get_int())
{
    m_processTimer.Start(5000);     //arbitrary.

    // proximityRange (154) tells us how far we "see"

    if (m_entityAttackRange < 10000)   // most of these are low...under 6k  that sux for targeting
        m_entityAttackRange *= 3;
}

void DroneAIMgr::Process() {
    double profileStartTime(GetTimeUSeconds());

    /* Drone::State definitions   -allan 27Nov19  
     *   Invalid
     *   Idle              = 0,  // not doing anything....idle.
     *   Combat            = 1,  // fighting - needs targetID
     *   Mining            = 2,  // unsure - needs targetID
     *   Approaching       = 3,  // too close to chase, but to far to engage
     *   Departing         = 4,  // return to ship
     *   Departing2        = 5,  // leaving.  different from Departing
     *   Pursuit           = 6,  // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
     *   Fleeing           = 7,  // running away
     *   Operating         = 9,  // whats diff from engaged here?  mining maybe?
     *   Engaged           = 10, // non-combat? - needs targetID
     *   // internal only
     *   Unknown           = 8,  // as stated
     *   Guarding          = 11,
     *   Assisting         = 12,
     *   Incapacitated     = 13  // out of control range, but online
     */

    // test for drone attributes here - aggressive, focus fire, attack/follow
    // test for control distance here also.  offline drones outside this  (AttrDroneControlDistance)
    if (m_destiny->IsWarping())
        return;

    // Check if drone is within control range
    if (!IsInControlRange()) {
        m_state = DroneAI::State::Incapacitated;
        return;
    }

    switch(m_state) {
        case DroneAI::State::Invalid: {
            if (m_assignedShip != nullptr)
                Return();
        } break;
        case DroneAI::State::Idle: {
            if (m_assignedShip != nullptr)
                IdleOrbit();
        } break;
        case DroneAI::State::Engaged: {
            //NOTE: getting our pTarget like this is pretty weak...
            SystemEntity* pTarget = m_pDrone->TargetMgr()->GetFirstTarget(true);
            if (pTarget == nullptr) {
                if (m_pDrone->TargetMgr()->HasNoTargets()) {
                    _log(DRONE__AI_TRACE, "Drone %s(%u): Stopped engagement, GetFirstTarget() returned NULL.", m_pDrone->GetName(), m_pDrone->GetID());
                    SetIdle();
                }
                return;
            } else if (pTarget->SysBubble() == nullptr) {
                m_pDrone->TargetMgr()->ClearTarget(pTarget);
                return;
            }
            CheckDistance(pTarget);
        } break;

        case DroneAI::State::Departing: { // return to ship.  when close enough, set lazy orbit
            if (m_pDrone->GetPosition().distance(m_assignedShip->GetPosition()) < m_entityOrbitRange)
                SetIdle();
        } break;
        case DroneAI::State::Combat: {
            ProcessCombat();
        } break;
        case DroneAI::State::Mining: {
            ProcessMining(); 
        } break;
        case DroneAI::State::Approaching: {
            SystemEntity* pTarget = m_pDrone->TargetMgr()->GetFirstTarget(true);
            if (pTarget == nullptr) {
                SetIdle();
                return;
            }
            CheckDistance(pTarget);
        } break;
        case DroneAI::State::Departing2:
        case DroneAI::State::Pursuit: {
            SystemEntity* pTarget = m_pDrone->TargetMgr()->GetFirstTarget(true);
            if (pTarget == nullptr) {
                SetIdle();
                return;
            }
            CheckDistance(pTarget);
        } break;
        case DroneAI::State::Assisting: {
            // 检查辅助目标是否还存在
            SystemEntity* pTarget = m_pDrone->TargetMgr()->GetFirstTarget(true);
            if (pTarget == nullptr || pTarget->TargetMgr() == nullptr) {
                SetIdle();
                return;
            }
            // 获取新的攻击目标
            SystemEntity* assistTarget = pTarget->TargetMgr()->GetFirstTarget(true);
            if (assistTarget != nullptr && IsHostile(assistTarget)) {
                Target(assistTarget);
            }
        } break;
        case DroneAI::State::Fleeing:
        case DroneAI::State::Operating:
        case DroneAI::State::Unknown:
        case DroneAI::State::Incapacitated:
        case DroneAI::State::Guarding: {
            // 检查守卫目标是否还存在
            SystemEntity* pTarget = m_pDrone->TargetMgr()->GetFirstTarget(true);
            if (pTarget == nullptr) {
                SetIdle();
                return;
            }
            // 检查周围是否有敌对目标
            std::map<uint32, SystemEntity*> entityMap;
            m_pDrone->SysBubble()->GetEntities(entityMap);
            for (auto& entry : entityMap) {
                SystemEntity* entity = entry.second;
                if (entity != nullptr && IsHostile(entity)) {
                    Target(entity);
                    break;
                }
            }
        } break;

    //no default on purpose
    }
    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::drone, GetTimeUSeconds() - profileStartTime);
}

bool DroneAIMgr::IsInControlRange() {
    if (m_assignedShip == nullptr)
        return false;

    float distance = m_pDrone->GetPosition().distance(m_assignedShip->GetPosition());
    return distance <= m_droneControlDistance;
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
    m_assignedShip = m_pDrone->GetHomeShip();
    m_pDrone->DestinyMgr()->SetMaxVelocity(m_chaseSpeed);
    m_pDrone->DestinyMgr()->Follow(m_assignedShip, m_entityOrbitRange);
    m_state = DroneAI::State::Departing;
}

void DroneAIMgr::SetIdle() {
    if (m_state == DroneAI::State::Idle)
        return;
    // not doing anything....idle.
    _log(DRONE__AI_TRACE, "Drone %s(%u): SetIdle: returning to idle.",
         m_pDrone->GetName(), m_pDrone->GetID());
    m_state = DroneAI::State::Idle;

    // disable ewar timers
    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_mainAttackTimer.Disable();
    m_warpScramblerTimer.Disable();

    // orbit assigned ship
    m_pDrone->IdleOrbit(m_assignedShip);
}

void DroneAIMgr::SetEngaged(SystemEntity* pTarget) {
    if (m_state == DroneAI::State::Engaged)
        return;
    _log(DRONE__AI_TRACE, "Drone %s(%u): SetEngaged: %s(%u) begin engaging.",
         m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively fighting
    //   not sure of the actual orbit speed of npc's, but their 'cruise speed' seems a bit slow.
    //   this sets orbit speed between cruise speed and quarter of max speed (whether mwb or ab)
    //   this will also enable this npc to have a variable speed, instead of fixed upon creation.
    m_pDrone->DestinyMgr()->SetMaxVelocity(MakeRandomFloat(m_cruiseSpeed, (m_chaseSpeed /4)));
    m_pDrone->DestinyMgr()->Orbit(pTarget, m_entityOrbitRange);  //try to get inside orbit range
    m_state = DroneAI::State::Engaged;
}

void DroneAIMgr::CheckDistance(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;

    double dist = m_pDrone->GetPosition().distance(pSE->GetPosition());
    if (dist > m_entityAttackRange) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) is too far away (%.0f). Setting pursuit.",
             m_pDrone->GetName(), m_pDrone->GetID(), pSE->GetName(), pSE->GetID(), dist);
        SetPursuit(pSE);
        return;
    }

    if (dist < m_entityFlyRange) {
        // 在武器最大射程内，开始攻击
        SetEngaged(pSE);
    } else if (dist < m_entityChaseRange) {
        // 在追击范围内，接近目标
        SetApproaching(pSE);
    } else if (dist < m_entityAttackRange) {
        // 在视野范围内，追击目标
        SetPursuit(pSE);
    }

    // 检查是否需要激活特殊效果(webifier, scrambler等)
    if (m_warpScramblerTimer.Enabled() && m_warpScramblerTimer.Check()) {
        if (dist <= m_entityAttackRange) {
            // TODO: 实现warp scrambler效果
        }
    }

    if (m_webifierTimer.Enabled() && m_webifierTimer.Check()) {
        if (dist <= m_entityAttackRange) {
            // TODO: 实现webifier效果
        }
    }
}

void DroneAIMgr::ClearTargets() {
    m_pDrone->TargetMgr()->ClearTargets();
}

void DroneAIMgr::ClearAllTargets() {
    m_pDrone->TargetMgr()->ClearAllTargets();
    //m_pDrone->TargetMgr()->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::ClientReq);
}

void DroneAIMgr::Target(SystemEntity* pTarget) {
    if (pTarget == nullptr)
        return;

    bool chase = false;
    if (!m_pDrone->TargetMgr()->StartTargeting(pTarget, 
        m_pDrone->GetSelf()->GetAttribute(AttrScanSpeed).get_uint32(),
        (uint8)m_pDrone->GetSelf()->GetAttribute(AttrMaxAttackTargets).get_int(),
        m_entityAttackRange, chase))
    {
        _log(DRONE__AI_TRACE, "Drone %s(%u): Targeting of %s(%u) failed. Clear Target and Return to Idle.",
             m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());
        SetIdle();
        return;
    }

    m_beginFindTarget.Disable();
    CheckDistance(pTarget);
}

void DroneAIMgr::Targeted(SystemEntity* pAgressor) {
    _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while %s.",
                m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID(), GetStateName(m_state).c_str());
    switch(m_state) {
        case DroneAI::State::Idle: {
            // 如果是敌对目标，则进入战斗状态
            if (IsHostile(pAgressor)) {
                Target(pAgressor);
                m_state = DroneAI::State::Combat;
            }
        } break;
        case DroneAI::State::Operating: {
            // 如果是敌对目标，则中断当前操作进入战斗状态
            if (IsHostile(pAgressor)) {
                ClearAllTargets();
                Target(pAgressor);
                m_state = DroneAI::State::Combat;
            }
        } break;
        case DroneAI::State::Engaged:{
        } break;
        case DroneAI::State::Combat: {
            // 已经在战斗中，检查是否需要切换目标
            if (IsHostile(pAgressor)) {
                SystemEntity* currentTarget = m_pDrone->TargetMgr()->GetFirstTarget(true);
                if (currentTarget == nullptr || 
                    pAgressor->GetSelf()->GetAttribute(AttrMass).get_float() > currentTarget->GetSelf()->GetAttribute(AttrMass).get_float()) {
                    Target(pAgressor);
                }
            }
        } break;
        case DroneAI::State::Mining: {
            // 如果是敌对目标，则中断采矿进入战斗状态
            if (IsHostile(pAgressor)) {
                ClearAllTargets();
                Target(pAgressor);
                m_state = DroneAI::State::Combat;
            }
        } break;
        case DroneAI::State::Guarding: {
            // 如果是敌对目标，则攻击
            if (IsHostile(pAgressor)) {
                Target(pAgressor);
            }
        } break;
        case DroneAI::State::Assisting: {
            // 如果辅助目标被攻击且攻击者是敌对的，则攻击
            SystemEntity* assistTarget = m_pDrone->TargetMgr()->GetFirstTarget(true);
            if (assistTarget != nullptr && pAgressor->GetID() == assistTarget->GetID() && IsHostile(pAgressor)) {
                Target(pAgressor);
            }
        } break;
        // 其他状态保持不变
        default: break;
    }
}

void DroneAIMgr::TargetLost(SystemEntity* pTarget) {
    switch(m_state) {
        case DroneAI::State::Engaged:
        case DroneAI::State::Combat: {
            if (m_pDrone->TargetMgr()->HasNoTargets()) {
                _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) lost. No targets remain. Return to Idle.",
                     m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());
                SetIdle();
            } else {
                _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) lost, but more targets remain.",
                     m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());
            }
        } break;
        case DroneAI::State::Mining: {
            if (m_pDrone->TargetMgr()->HasNoTargets()) {
                _log(DRONE__AI_TRACE, "Drone %s(%u): Mining target %s(%u) lost. Return to Idle.",
                     m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());
                SetIdle();
            }
        } break;
        case DroneAI::State::Guarding: {
            // 检查是否还有其他敌对目标
            std::map<uint32, SystemEntity*> entityMap;
            m_pDrone->SysBubble()->GetEntities(entityMap);
            for (auto& entry : entityMap) {
                SystemEntity* entity = entry.second;
                if (entity != nullptr && IsHostile(entity)) {
                    Target(entity);
                    return;
                }
            }
            // 没有其���敌对目标，继续守卫
        } break;
        case DroneAI::State::Assisting: {
            // 检查辅助目标是否还存在
            SystemEntity* assistTarget = m_pDrone->TargetMgr()->GetFirstTarget(true);
            if (assistTarget == nullptr || assistTarget->TargetMgr() == nullptr) {
                SetIdle();
                return;
            }
            // 获取新的攻击目标
            SystemEntity* newTarget = assistTarget->TargetMgr()->GetFirstTarget(true);
            if (newTarget != nullptr && IsHostile(newTarget)) {
                Target(newTarget);
            }
        } break;
        default:
            break;
    }
}

void DroneAIMgr::Attack(SystemEntity* pSE) {
    if (m_mainAttackTimer.Check()) {
        if (pSE == nullptr)
            return;
            
        // Check to see if the target still in the bubble (Client warped out)
        // fighters/bombers are able to follow.
        if (!m_pDrone->SysBubble()->InBubble(pSE->GetPosition())) {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) no longer in bubble.  Clear target and move on",
                 m_pDrone->GetName(), m_pDrone->GetID(), pSE->GetName(), pSE->GetID());
            ClearTarget(pSE);
            return;
        }
        DestinyManager* pDestiny = pSE->DestinyMgr();
        if (pDestiny == nullptr) {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) has no destiny manager.  Clear target and move on",
                 m_pDrone->GetName(), m_pDrone->GetID(), pSE->GetName(), pSE->GetID());
            ClearTarget(pSE);
            return;
        }
        // Check to see if the target is not cloaked:
        if (pDestiny->IsCloaked()) {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) is cloaked.  Clear target and move on",
                 m_pDrone->GetName(), m_pDrone->GetID(), pSE->GetName(), pSE->GetID());
            ClearTarget(pSE);
            return;
        }

        // 检查并应用特殊效果
        double dist = m_pDrone->GetPosition().distance(pSE->GetPosition());
        if (dist <= m_entityAttackRange) {
            // 检查并应用webifier效果
            if (m_webifierTimer.Enabled() && m_webifierTimer.Check()) {
                UseWebifier(pSE);
            }

            // 检查并应用warp scrambler效果
            if (m_warpScramblerTimer.Enabled() && m_warpScramblerTimer.Check()) {
                UseWarpScrambler(pSE);
            }
        }

        if (m_pDrone->TargetMgr()->CanAttack())
            AttackTarget(pSE);
    }
}

void DroneAIMgr::ClearTarget(SystemEntity* pSE) {
    m_pDrone->TargetMgr()->ClearTarget(pSE);
    //m_pDrone->TargetMgr()->OnTarget(pSE, TargMgr::Mode::Lost);

    if (m_pDrone->TargetMgr()->HasNoTargets())
        SetIdle();
}

//also check for special effects and write code to implement them
//modifyTargetSpeedRange, modifyTargetSpeedChance
//entityWarpScrambleChance

void DroneAIMgr::AttackTarget(SystemEntity* pTarget) {
    /** @todo  not all drones use lazors...fix this */
    //  woot!! --> group:1010        cat:8       Compact Citadel Torpedo         Citadel torpedoes for fighter-bombers

    // effects are listed in EVE_Effects.h
    //  NOTE: drones are called 'entities' in client; EVE_Effects has 'entityxxx' for gfx
    std::string guid = "effects.Laser"; // client looks for 'turret' in ship.ball.modules for 'effects.laser'
    uint32 gfxID = 0;
    if (m_pDrone->GetSelf()->HasAttribute(AttrGfxTurretID))
        gfxID = m_pDrone->GetSelf()->GetAttribute(AttrGfxTurretID).get_uint32();
    m_pDrone->DestinyMgr()->SendSpecialEffect(m_pDrone->GetSelf()->itemID(),
                                             m_pDrone->GetSelf()->itemID(),
                                             m_pDrone->GetSelf()->typeID(),
                                             pTarget->GetID(),
                                             0,guid,1,1,1,m_attackSpeed,0,gfxID);

    Damage d(m_pDrone,                                  // attacker
             m_pDrone->GetSelf(),                       // weapon
             m_pDrone->GetKinetic(),                    // kinetic damage
             m_pDrone->GetThermal(),                    // thermal damage
             m_pDrone->GetEM(),                         // em damage
             m_pDrone->GetExplosive(),                  // explosive damage
             m_formula.GetDroneToHit(m_pDrone, pTarget),// chance to hit
             EVEEffectID::targetAttack);                // effect ID

    d *= m_pDrone->GetSelf()->GetAttribute(AttrDamageMultiplier).get_float();
    d *= sConfig.rates.damageRate;
    pTarget->ApplyDamage(d);
}


std::string DroneAIMgr::GetStateName(int8 stateID)
{
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

void DroneAIMgr::ProcessCombat() {
    // 获取当前目标
    SystemEntity* currentTarget = m_pDrone->TargetMgr()->GetFirstTarget(true);
    
    // 获取所有可能的目标
    std::vector<SystemEntity*> potentialTargets;
    std::map<uint32, SystemEntity*> entityMap;
    m_pDrone->SysBubble()->GetEntities(entityMap);
    
    float bestScore = -1.0f;
    SystemEntity* bestTarget = nullptr;

    // 评估所有目标
    for (auto& entry : entityMap) {
        SystemEntity* entity = entry.second;
        if (entity == nullptr || !IsHostile(entity))
            continue;

        // 检查目标是否在攻击范围内
        if (m_pDrone->GetPosition().distance(entity->GetPosition()) > m_entityAttackRange)
            continue;

        float score = GetTargetScore(entity);
        if (score > bestScore) {
            bestScore = score;
            bestTarget = entity;
        }
    }

    // 如果找到更好的目标，切���目标
    if (bestTarget != nullptr && bestTarget != currentTarget) {
        if (currentTarget == nullptr || GetTargetScore(bestTarget) > GetTargetScore(currentTarget) * 1.2f) {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Switching target from %s to %s (score: %.1f)",
                 m_pDrone->GetName(), m_pDrone->GetID(),
                 (currentTarget ? currentTarget->GetName() : "none"),
                 bestTarget->GetName(), bestScore);
            
            Target(bestTarget);
        }
    }

    // 继续现有的战斗逻辑
    if (currentTarget != nullptr) {
        // 检查目标是否在bubble中
        if (!m_pDrone->SysBubble()->InBubble(currentTarget->GetPosition())) {
            ClearTarget(currentTarget);
            return;
        }

        // 检查目标是否隐形
        if (currentTarget->DestinyMgr() && currentTarget->DestinyMgr()->IsCloaked()) {
            ClearTarget(currentTarget);
            return;
        }

        // 检查距离并攻击
        CheckDistance(currentTarget);
        if (m_mainAttackTimer.Check()) {
            Attack(currentTarget);
        }
    }
}

void DroneAIMgr::ProcessMining() {
    // 获取当前目标
    SystemEntity* currentTarget = m_pDrone->TargetMgr()->GetFirstTarget(true);
    
    // 如果没有目标，寻找最佳采矿目���
    if (currentTarget == nullptr) {
        std::map<uint32, SystemEntity*> entityMap;
        m_pDrone->SysBubble()->GetEntities(entityMap);
        
        float bestScore = -1.0f;
        SystemEntity* bestTarget = nullptr;

        // 评估所有可能的采矿目标
        for (auto& entry : entityMap) {
            SystemEntity* entity = entry.second;
            if (entity == nullptr || !entity->IsAsteroidSE())
                continue;

            // 检查距离
            float distance = m_pDrone->GetPosition().distance(entity->GetPosition());
            if (distance > m_entityAttackRange)
                continue;

            // 计算目标分数
            float score = GetMiningTargetScore(entity);
            if (score > bestScore) {
                bestScore = score;
                bestTarget = entity;
            }
        }

        // 如果找到合适的目标，开始采矿
        if (bestTarget != nullptr) {
            Target(bestTarget);
            currentTarget = bestTarget;
        } else {
            return;  // 没有找到合适的目标
        }
    }

    // 检查目标是否是小行星
    if (!currentTarget->IsAsteroidSE()) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) is not an asteroid. Clearing target.", 
             m_pDrone->GetName(), m_pDrone->GetID(), currentTarget->GetName(), currentTarget->GetID());
        ClearTarget(currentTarget);
        return;
    }

    // 检查货舱容量
    if (IsCargoFull()) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): Cargo hold is full. Returning to ship.", 
             m_pDrone->GetName(), m_pDrone->GetID());
        Return();
        return;
    }

    // 检查距离并调整位置
    CheckMiningDistance(currentTarget);

    // 执行采矿操作
    if (m_mainAttackTimer.Check()) {
        // 发送采矿特效
        SendMiningEffect(currentTarget);

        // 计算采矿量
        float miningAmount = CalculateMiningAmount();
        float oreVolume = currentTarget->GetSelf()->GetAttribute(AttrQuantity).get_float();
        float actualAmount = std::min(miningAmount, oreVolume);
        
        if (actualAmount > 0) {
            // 减少小行星的矿石量
            currentTarget->GetSelf()->SetAttribute(AttrQuantity, oreVolume - actualAmount);

            // 更新小行星半径
            UpdateAsteroidRadius(currentTarget, oreVolume - actualAmount);

            // 将矿石转移到无人机货舱
            TransferOreToCargoHold(currentTarget, actualAmount);

            // 检查小行星是否已经采空
            if ((oreVolume - actualAmount) <= 0) {
                _log(DRONE__AI_TRACE, "Drone %s(%u): Asteroid %s(%u) depleted. Finding new target.", 
                     m_pDrone->GetName(), m_pDrone->GetID(), currentTarget->GetName(), currentTarget->GetID());
                ClearTarget(currentTarget);
            }
        }
    }
}

// 计算采矿目标分数
float DroneAIMgr::GetMiningTargetScore(SystemEntity* pTarget) {
    if (pTarget == nullptr)
        return 0.0f;

    float score = 0.0f;

    // 矿石量评分
    float oreVolume = pTarget->GetSelf()->GetAttribute(AttrQuantity).get_float();
    score += oreVolume * 0.5f;

    // 距离评分 (越近分数越高)
    float distance = m_pDrone->GetPosition().distance(pTarget->GetPosition());
    score += (m_entityAttackRange - distance) / m_entityAttackRange * 100.0f;

    // 矿石类型评分 (根据矿石价值)
    score += GetOreValueScore(pTarget->GetSelf()->typeID());

    return score;
}

// 检查货舱是否已满
bool DroneAIMgr::IsCargoFull() {
    if (!m_pDrone->GetSelf()->HasAttribute(AttrCapacity))
        return true;

    float cargoCapacity = m_pDrone->GetSelf()->GetAttribute(AttrCapacity).get_float();
    float currentVolume = m_pDrone->GetSelf()->GetAttribute(AttrVolume).get_float();
    
    return (currentVolume >= cargoCapacity * 0.95f);  // 留5%余量
}

// 检查采矿距离
void DroneAIMgr::CheckMiningDistance(SystemEntity* pTarget) {
    double dist = m_pDrone->GetPosition().distance(pTarget->GetPosition());
    
    // 优化采矿位置
    if (dist > m_entityFlyRange * 0.8) {  // 保持在最佳采矿范围内
        SetApproaching(pTarget);
    } else if (dist < m_entityFlyRange * 0.2) {  // 避免太靠近
        GVector direction = m_pDrone->GetPosition() - pTarget->GetPosition();
        direction.normalize();
        m_pDrone->DestinyMgr()->GotoDirection(direction);
    } else {
        // 在最佳范围内，开始环绕采矿
        m_pDrone->DestinyMgr()->Orbit(pTarget, m_entityFlyRange * 0.5);
    }
}

// 获取矿石价值评分
float DroneAIMgr::GetOreValueScore(uint16 typeID) {
    // 根据矿石类型返回基础价值分数
    switch (typeID) {
        case 1230: return 100.0f;  // Veldspar
        case 1228: return 150.0f;  // Scordite
        case 1224: return 200.0f;  // Pyroxeres
        // ... 添加更多矿石类型
        default: return 50.0f;
    }
}

void DroneAIMgr::IdleOrbit() {
    if (m_assignedShip == nullptr)
        return;
        
    m_pDrone->DestinyMgr()->SetMaxVelocity(m_cruiseSpeed);
    m_pDrone->DestinyMgr()->Orbit(m_assignedShip, m_entityOrbitRange);
}

void DroneAIMgr::SetFleeing(SystemEntity* pTarget) {
    if (m_state == DroneAI::State::Fleeing)
        return;

    _log(DRONE__AI_TRACE, "Drone %s(%u): Begin fleeing from %s(%u)",
         m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());

    m_state = DroneAI::State::Fleeing;
    
    // 计算逃离方向
    GVector direction = m_pDrone->GetPosition() - pTarget->GetPosition();
    direction.normalize();
    
    // 设置最大速度并逃离
    m_pDrone->DestinyMgr()->SetMaxVelocity(m_chaseSpeed);
    m_pDrone->DestinyMgr()->GotoDirection(direction);
}

void DroneAIMgr::SetGuarding(SystemEntity* pTarget) {
    if (m_state == DroneAI::State::Guarding)
        return;

    _log(DRONE__AI_TRACE, "Drone %s(%u): Begin guarding %s(%u)",
         m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());

    m_state = DroneAI::State::Guarding;
    
    // 设置守卫轨道
    m_pDrone->DestinyMgr()->SetMaxVelocity(m_cruiseSpeed);
    m_pDrone->DestinyMgr()->Orbit(pTarget, m_entityOrbitRange);

    // 如果设置为主动守卫，检查周围的敌对目标
    if (m_aggressiveGuard) {
        std::map<uint32, SystemEntity*> entityMap;
        m_pDrone->SysBubble()->GetEntities(entityMap);
        
        float bestThreat = -1.0f;
        SystemEntity* highestThreat = nullptr;

        // 评估有潜在威胁
        for (auto& entry : entityMap) {
            SystemEntity* entity = entry.second;
            if (entity == nullptr || !IsHostile(entity))
                continue;

            // 检查是否在警戒范围内
            float distance = entity->GetPosition().distance(pTarget->GetPosition());
            if (distance > m_guardRange)
                continue;

            // 计算威胁度
            float threat = CalculateThreatLevel(entity);
            if (threat > bestThreat) {
                bestThreat = threat;
                highestThreat = entity;
            }
        }

        // 如果发现威胁，进行攻击
        if (highestThreat != nullptr) {
            Target(highestThreat);
            _log(DRONE__AI_TRACE, "Drone %s(%u): Detected threat %s(%u) while guarding %s(%u). Engaging.",
                 m_pDrone->GetName(), m_pDrone->GetID(), 
                 highestThreat->GetName(), highestThreat->GetID(),
                 pTarget->GetName(), pTarget->GetID());
        }
    }
}

bool DroneAIMgr::IsHostile(SystemEntity* pTarget) {
    if (pTarget == nullptr)
        return false;

    // 检查是否是友方
    if (pTarget->GetWarFactionID() == m_pDrone->GetWarFactionID())
        return false;

    // 检查是否是敌对势力
    if (pTarget->GetWarFactionID() > 0 && m_pDrone->GetWarFactionID() > 0)
        if (pTarget->GetWarFactionID() != m_pDrone->GetWarFactionID())
            return true;

    // 检查是否是敌对玩家
    if (pTarget->HasPilot()) {
        Client* pClient = pTarget->GetPilot();
        if (pClient == nullptr)
            return false;

        // 检查是否在同一个军团
        if (pClient->GetCorporationID() == m_pDrone->GetCorporationID())
            return false;

        // 检查是否在同一个联盟
        if (pClient->GetAllianceID() == m_pDrone->GetAllianceID())
            return false;

        // 检查是否在战争中
        if (pClient->GetWarFactionID() > 0 && m_pDrone->GetWarFactionID() > 0)
            if (pClient->GetWarFactionID() != m_pDrone->GetWarFactionID())
                return true;

        // 检查安全等级
        if (pClient->GetSecurityRating() < -5.0)
            return true;

        // 检查是否有悬赏
        if (pClient->GetBounty() > 0)
            return true;
    }

    return false;
}

void DroneAIMgr::SetApproaching(SystemEntity* pTarget) {
    if (m_state == DroneAI::State::Approaching)
        return;

    _log(DRONE__AI_TRACE, "Drone %s(%u): Begin approaching %s(%u)",
         m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());

    m_state = DroneAI::State::Approaching;
    
    // 设置接近速度和距离
    m_pDrone->DestinyMgr()->SetMaxVelocity(m_cruiseSpeed);
    m_pDrone->DestinyMgr()->Follow(pTarget, m_entityFlyRange);
}

void DroneAIMgr::SetPursuit(SystemEntity* pTarget) {
    if (m_state == DroneAI::State::Pursuit)
        return;

    _log(DRONE__AI_TRACE, "Drone %s(%u): Begin pursuit of %s(%u)",
         m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());

    m_state = DroneAI::State::Pursuit;
    
    // 追击使用大速度
    m_pDrone->DestinyMgr()->SetMaxVelocity(m_chaseSpeed);
    m_pDrone->DestinyMgr()->Follow(pTarget, m_entityChaseRange);
}

void DroneAIMgr::SetAssisting(SystemEntity* pTarget) {
    if (m_state == DroneAI::State::Assisting)
        return;

    _log(DRONE__AI_TRACE, "Drone %s(%u): Begin assisting %s(%u)",
         m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());

    m_state = DroneAI::State::Assisting;

    // 获取辅助目标的所有目标
    if (pTarget->TargetMgr() != nullptr) {
        PyList* targets = pTarget->TargetMgr()->GetTargets();
        if (targets == nullptr)
            return;

        float bestScore = -1.0f;
        SystemEntity* bestTarget = nullptr;

        // 评估所有目标
        for (size_t i = 0; i < targets->size(); ++i) {
            PyTuple* targetTuple = targets->GetItem(i)->AsTuple();
            if (targetTuple == nullptr)
                continue;
            
            SystemEntity* target = pTarget->TargetMgr()->GetTarget(targetTuple->GetItem(0)->AsInt()->value());
            if (target == nullptr || !IsHostile(target))
                continue;

            // 检查是否在协助范围内
            float distance = target->GetPosition().distance(pTarget->GetPosition());
            if (distance > m_assistRange)
                continue;

            // 计算目标优先级
            float score = GetAssistTargetScore(target, pTarget);
            if (score > bestScore) {
                bestScore = score;
                bestTarget = target;
            }
        }

        // 选择最优目标进行攻击
        if (bestTarget != nullptr) {
            Target(bestTarget);
            _log(DRONE__AI_TRACE, "Drone %s(%u): Assisting %s(%u) by attacking %s(%u)",
                 m_pDrone->GetName(), m_pDrone->GetID(),
                 pTarget->GetName(), pTarget->GetID(),
                 bestTarget->GetName(), bestTarget->GetID());
        }
    }
    
    // 跟随辅助目标
    m_pDrone->DestinyMgr()->SetMaxVelocity(m_cruiseSpeed);
    m_pDrone->DestinyMgr()->Follow(pTarget, m_entityOrbitRange);
}

void DroneAIMgr::UseWebifier(SystemEntity* pTarget) {
    if (pTarget == nullptr)
        return;

    // 检查是否有webifier属性
    if (!m_pDrone->GetSelf()->HasAttribute(AttrEntityWebifierDuration))
        return;

    float duration = m_pDrone->GetSelf()->GetAttribute(AttrEntityWebifierDuration).get_float();
    float strength = m_pDrone->GetSelf()->GetAttribute(AttrEntityWebifierStrength).get_float();

    // 发送webifier特效
    m_pDrone->DestinyMgr()->SendSpecialEffect(
        m_pDrone->GetSelf()->itemID(),
        m_pDrone->GetSelf()->itemID(),
        m_pDrone->GetSelf()->typeID(),
        pTarget->GetID(),
        0,
        "effects.ModifyTargetSpeed",
        true,
        true,
        true,
        duration,
        1,
        0);

    // 应用减速效果
    if (pTarget->DestinyMgr()) {
        pTarget->DestinyMgr()->WebbedMe(m_pDrone->GetSelf(), true);
    }
}

void DroneAIMgr::UseWarpScrambler(SystemEntity* pTarget) {
    if (pTarget == nullptr)
        return;

    // 检查是否有warp scrambler属性
    if (!m_pDrone->GetSelf()->HasAttribute(AttrEntityWarpScrambleChance))
        return;

    // 获取warp scramble属性
    float duration = m_pDrone->GetSelf()->GetAttribute(AttrWarpScrambleDuration).get_float();
    float strength = m_pDrone->GetSelf()->GetAttribute(AttrWarpScrambleStrength).get_float();
    float chance = m_pDrone->GetSelf()->GetAttribute(AttrEntityWarpScrambleChance).get_float();

    // 检查是否触发warp scramble效果
    if (MakeRandomFloat(0, 1.0) > chance)
        return;

    // 发送warp scrambler特效
    m_pDrone->DestinyMgr()->SendSpecialEffect(
        m_pDrone->GetSelf()->itemID(),
        m_pDrone->GetSelf()->itemID(),
        m_pDrone->GetSelf()->typeID(),
        pTarget->GetID(),
        0,
        "effects.WarpScramble",
        true,
        true,
        true,
        duration,
        1,
        0);

    // 应用warp scramble效果
    if (pTarget->DestinyMgr()) {
        // 禁用目标的跃迁引擎
        if (pTarget->DestinyMgr())
            pTarget->DestinyMgr()->WarpScrambled(m_pDrone->GetSelf(), true);

        // 设置warp scramble计时器
        m_warpScramblerTimer.Start(duration * 1000);  // 转为毫秒

        _log(DRONE__AI_TRACE, "Drone %s(%u): Applied warp scramble to %s(%u) for %.1f seconds with strength %.1f",
             m_pDrone->GetName(), m_pDrone->GetID(), 
             pTarget->GetName(), pTarget->GetID(),
             duration, strength);
    }
}

float DroneAIMgr::GetTargetScore(SystemEntity* pTarget) {
    if (pTarget == nullptr)
        return 0.0f;

    float score = 0.0f;

    // 基础分数
    if (pTarget->IsNPCSE())
        score += 10.0f;
    else if (pTarget->HasPilot())
        score += 20.0f;

    // 距离评分 (越近分数越高)
    float distance = m_pDrone->GetPosition().distance(pTarget->GetPosition());
    score += (m_entityAttackRange - distance) / m_entityAttackRange * 10.0f;

    // 目标血量评分 (血量越低分数越高)
    if (pTarget->GetSelf()->HasAttribute(AttrHP)) {
        float hp = pTarget->GetSelf()->GetAttribute(AttrHP).get_float();
        float maxHP = pTarget->GetSelf()->GetAttribute(AttrMaxHP).get_float();
        score += (1.0f - hp/maxHP) * 15.0f;
    }

    // 目标威胁评分
    if (pTarget->GetSelf()->HasAttribute(AttrDamageMultiplier)) {
        float dmgMult = pTarget->GetSelf()->GetAttribute(AttrDamageMultiplier).get_float();
        score += dmgMult * 5.0f;
    }

    // 目标价值评分
    if (pTarget->GetSelf()->HasAttribute(AttrMass)) {
        float mass = pTarget->GetSelf()->GetAttribute(AttrMass).get_float();
        score += mass * 0.001f;  // 质量越大，价值可能越高
    }

    return score;
}

void DroneAIMgr::SendMiningEffect(SystemEntity* pTarget) {
    std::string guid = "effects.Mining";
    uint32 gfxID = 0;
    if (m_pDrone->GetSelf()->HasAttribute(AttrGfxTurretID))
        gfxID = m_pDrone->GetSelf()->GetAttribute(AttrGfxTurretID).get_uint32();

    m_pDrone->DestinyMgr()->SendSpecialEffect(
        m_pDrone->GetSelf()->itemID(),
        m_pDrone->GetSelf()->itemID(), 
        m_pDrone->GetSelf()->typeID(),
        pTarget->GetID(),
        0, guid, 1, 1, 1, m_attackSpeed, 0, gfxID);
}

float DroneAIMgr::CalculateMiningAmount() {
    float miningAmount = m_pDrone->GetSelf()->GetAttribute(AttrMiningAmount).get_float();
    return miningAmount * m_pDrone->GetSelf()->GetAttribute(AttrMiningAmountMultiplier).get_float();
}

void DroneAIMgr::UpdateAsteroidRadius(SystemEntity* pTarget, float newVolume) {
    float newRadius = 89.675f * exp(4e-05f * newVolume);
    pTarget->GetSelf()->SetAttribute(AttrRadius, newRadius);
}

void DroneAIMgr::TransferOreToCargoHold(SystemEntity* pTarget, float amount) {
    if (!m_pDrone->GetSelf()->HasAttribute(AttrCapacity))
        return;

    float cargoCapacity = m_pDrone->GetSelf()->GetAttribute(AttrCapacity).get_float();
    float currentVolume = m_pDrone->GetSelf()->GetAttribute(AttrVolume).get_float();
    
    if ((currentVolume + amount) <= cargoCapacity) {
        // 创建矿石物品
        ItemData idata(
            pTarget->GetSelf()->typeID(),
            m_pDrone->GetOwnerID(),
            m_pDrone->GetID(),
            flagCargoHold,
            amount
        );
        
        InventoryItemRef newItem = sItemFactory.SpawnItem(idata);
        if (newItem) {
            m_pDrone->GetSelf()->AddItem(newItem);
            _log(DRONE__AI_TRACE, "Drone %s(%u): Mined %.2f units of ore from %s(%u)",
                 m_pDrone->GetName(), m_pDrone->GetID(), 
                 amount, pTarget->GetName(), pTarget->GetID());
        }
    }
}

float DroneAIMgr::CalculateThreatLevel(SystemEntity* pTarget) {
    if (pTarget == nullptr)
        return 0.0f;

    float threat = 0.0f;

    // 基础威胁值
    if (pTarget->IsNPCSE())
        threat += 10.0f;
    else if (pTarget->HasPilot())
        threat += 20.0f;

    // 装备威胁
    if (pTarget->GetSelf()->HasAttribute(AttrDamageMultiplier)) {
        float dmgMult = pTarget->GetSelf()->GetAttribute(AttrDamageMultiplier).get_float();
        threat += dmgMult * 5.0f;
    }

    // 电子战威胁
    if (pTarget->GetSelf()->HasAttribute(AttrScanResolutionBonus))
        threat += 15.0f;
    if (pTarget->GetSelf()->HasAttribute(AttrTrackingSpeedBonus))
        threat += 15.0f;

    // 目标锁定数量威胁
    if (pTarget->TargetMgr()) {
        PyList* targets = pTarget->TargetMgr()->GetTargets();
        uint32 targetCount = (targets != nullptr) ? static_cast<uint32>(targets->size()) : 0;
        threat += targetCount * 5.0f;
    }

    return threat;
}

float DroneAIMgr::GetAssistTargetScore(SystemEntity* pTarget, SystemEntity* pAssistTarget) {
    if (pTarget == nullptr || pAssistTarget == nullptr)
        return 0.0f;

    float score = 0.0f;

    // 基础分数
    score += GetTargetScore(pTarget);

    // 距离辅助目标的距离评分
    float distance = pTarget->GetPosition().distance(pAssistTarget->GetPosition());
    score += (m_assistRange - distance) / m_assistRange * 20.0f;

    // 目标正在攻击辅助目标的加成
    if (pTarget->TargetMgr()) {
        // 检查目标是否在目标列表中
        if (pTarget->TargetMgr()->GetTarget(pAssistTarget->GetID()) != nullptr)
            score *= 1.5f;
    }

    return score;
}
