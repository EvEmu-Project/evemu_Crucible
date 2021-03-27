/**
 * @name SentryAI.h
 *   Sentry AI class
 * @Author:     Allan
 * @date:       23 April 2017
 * @version     0.10
 */


#include "eve-server.h"

#include "Client.h"
#include "inventory/AttributeEnum.h"
#include "npc/Sentry.h"
#include "npc/SentryAI.h"
#include "system/DestinyManager.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"

SentryAI::SentryAI(Sentry* who)
: m_state(State::Idle),
m_npc(who),
m_mainAttackTimer(1000),
m_processTimer(5000),         //arbitrary.
m_beginFindTarget(5000),      //arbitrary.
m_warpScramblerTimer(5000),   //arbitrary.
m_webifierTimer(5000)         //arbitrary.
{
    m_webifierTimer.Disable();      //not implemented yet
    m_beginFindTarget.Disable();    //arbitrary.
    m_mainAttackTimer.Disable();    //waiting till engaged
    m_warpScramblerTimer.Disable(); //not implemented yet

    m_webber = false;
    m_warpScram = false;

    m_damageMultiplier = who->GetSelf()->GetAttribute(AttrDamageMultiplier).get_int();

    /* set npc ship speeds and distances */
    m_sigRadius = who->GetSelf()->GetAttribute(AttrSignatureRadius).get_int();
    m_attackSpeed = who->GetSelf()->GetAttribute(AttrSpeed).get_int();
    m_processTimer.Start(m_attackSpeed);

    // Optimal Range
    m_optimalRange = who->GetSelf()->GetAttribute(AttrMaxRange).get_int();
    // Accuracy falloff  (distance past maximum range at which accuracy has fallen by half)
    m_falloff = who->GetSelf()->GetAttribute(AttrFalloff).get_int();
    m_trackingSpeed = who->GetSelf()->GetAttribute(AttrTrackingSpeed).get_float();
    // max firing range   default:10000
    m_maxAttackRange = who->GetSelf()->GetAttribute(AttrEntityAttackRange).get_int();
    if (!m_maxAttackRange)
        m_maxAttackRange = 10000;
    // 'sight' range
    m_sightRange = 20000;
    if (m_maxAttackRange > m_sightRange)
        m_sightRange = m_maxAttackRange *2;

    // advanced AI variables  only used by sleepers for now.  will update advanced npcs to use these also
    if (who->GetSelf()->HasAttribute(AttrAI_ShouldUseTargetSwitching)) {
        m_useTargSwitching = true;
    } else {
        m_useTargSwitching = false;
    }
    if (who->GetSelf()->HasAttribute(AttrAI_ShouldUseSecondaryTarget)) {
        m_useSecondTarget = true;
    } else {
            m_useSecondTarget = false;
    }
    if (who->GetSelf()->HasAttribute(AttrAI_ShouldUseSignatureRadius)) {
        m_useSigRadius = true;
        m_preferedSigRadius = who->GetSelf()->GetAttribute(AttrAI_PreferredSignatureRadius).get_int();
    } else {
        m_useSigRadius = false;
        m_preferedSigRadius = 0;
    }
    if (who->GetSelf()->HasAttribute(AttrAI_ChanceToNotTargetSwitch)) {
        m_switchTargChance = 1.0 - who->GetSelf()->GetAttribute(AttrAI_ChanceToNotTargetSwitch).get_float();
    } else {
        m_switchTargChance = 0.0f;
    }
}

void SentryAI::Process() {
    if ((!m_processTimer.Check()) or (!m_npc->SysBubble()->HasPlayers()))
        return;

    /* NPC::State definitions   -allan 25July15  (UD 1June16)
    *   Idle,       // not doing anything, nothing in sight....idle.
    *   Engaged,    // actively fighting.
    *   Signaling   // calling for help
    */
    switch(m_state) {
        case State::Idle: {
            // The parameter proximityRange (154) tells us how far we "see" (npc's dont have this, but drones do)
            if (m_beginFindTarget.Check()) {
                std::vector<Client*> clientVec;
                clientVec.clear();
                DestinyManager* pDestiny(nullptr);
                m_npc->SysBubble()->GetPlayers(clientVec); // what about player drones?  yes...later
                for (auto cur : clientVec) {
                    /** @todo  this needs work
                    if (cur->IsLogin() or cur->IsInvul() or cur->InPod())
                        continue;
                    if (!cur->GetShipTarget())
                        continue;
                    if ((!cur->GetShipTarget()->DestinyMgr()) or (!cur->GetShipTarget()->SysBubble()))    // this shouldnt be needed, but whatever...
                        continue;
                    pDestiny = cur->GetShipTarget()->DestinyMgr();
                    if (pDestiny->IsCloaked() or pDestiny->IsWarping())
                        continue;
                    if (m_npc->GetPosition().distance(cur->GetShipTarget()->GetPosition()) > m_sightRange)
                        continue;
                    Target(cur->GetShipTarget());
                    */
                    return;
                }
            } else {
                if (!m_beginFindTarget.Enabled())
                    m_beginFindTarget.Start(m_attackSpeed);  //find target is based on npc attack speed.
            }
        } break;
        case State::Engaged: {
            if (m_npc->TargetMgr()->HasNoTargets()) {
                _log(NPC__AI_TRACE, "%s(%u): Stopped %s, HasNoTargets = true.", m_npc->GetName(), m_npc->GetID(), GetStateName(m_state).c_str());
                SetIdle();
                return;
            }
            SystemEntity* pTarget = m_npc->TargetMgr()->GetFirstTarget(false);
            if (!pTarget) {
                _log(NPC__AI_TRACE, "%s(%u): Stopped %s, GetFirstTarget() returned NULL.", m_npc->GetName(), m_npc->GetID(), GetStateName(m_state).c_str());
                SetIdle();
                return;
            } else if (!pTarget->SysBubble()) {
                ClearTarget(pTarget);
                return;
            }
            CheckDistance(pTarget);
        } break;
        case State::Signaling: {
            _log(NPC__AI_TRACE, "%s(%u): Called %s, needs to be completed.", m_npc->GetName(), m_npc->GetID(), GetStateName(m_state).c_str());
            // not sure how im gonna do this
        } break;
    }
}

void SentryAI::SetIdle() {
    if (m_state == State::Idle) return;
    // not doing anything....idle.
    _log(NPC__AI_TRACE, "%s(%u): Idle: returning to idle.", \
    m_npc->GetName(), m_npc->GetID());
    m_state = State::Idle;

    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_mainAttackTimer.Disable();
    m_warpScramblerTimer.Disable();
}

void SentryAI::SetEngaged(SystemEntity* pTarget) {
    if (m_state == State::Engaged)
        return;
    _log(NPC__AI_TRACE, "%s(%u): Engaged: Begin engaging.  Target is %s(%u).", \
            m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively fighting
    m_state = State::Engaged;
}

void SentryAI::SetSignaling(SystemEntity* pTarget) {
    if (m_state == State::Signaling)
        return;
    _log(NPC__AI_TRACE, "%s(%u): Signaling: Begin signaling.  Target is %s(%u).", \
            m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively signaling
    m_state = State::Signaling;
}

void SentryAI::CheckDistance(SystemEntity* pTarget)
{
    double dist = m_npc->GetPosition().distance(pTarget->GetPosition());
    if ((dist > m_sightRange) and (!m_npc->TargetMgr()->IsTargetedBy(pTarget))) {
        _log(NPC__AI_TRACE, "%s(%u): CheckDistance: %s(%u) is too far away (%u).  Return to Idle.", \
                m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID(), dist);
        if (m_state != State::Idle) {
            // target is no longer in npc's "sight range" and is NOT targeting this npc.  unlock target and return to idle.
            //   should we do anything else here?  search for another target?  wander around?  yes..later
            // if npc is targeted greater than this distance, it will chase
            ClearTarget(pTarget);
        }
        return;
    }

    SetEngaged(pTarget);

    if (!m_mainAttackTimer.Enabled())
        m_mainAttackTimer.Start(m_attackSpeed);

    Attack(pTarget);
}

void SentryAI::ClearTarget(SystemEntity* pTarget) {
    m_npc->TargetMgr()->ClearTarget(pTarget);
    //m_npc->TargetMgr()->OnTarget(pTarget, TargMgr::Mode::Lost);

    if (m_npc->TargetMgr()->HasNoTargets())
        SetIdle();
}

void SentryAI::ClearAllTargets() {
    m_npc->TargetMgr()->ClearAllTargets();
    //m_npc->TargetMgr()->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::ClientReq);
}

void SentryAI::Target(SystemEntity* pTarget) {
    double targetTime = GetTargetTime();
    bool chase = false;

    if (!m_npc->TargetMgr()->StartTargeting(pTarget, targetTime, (uint8)m_npc->GetSelf()->GetAttribute(AttrMaxAttackTargets).get_int(), m_sightRange, chase)) {
        if (!chase) {
            _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.", \
                    m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
            SetIdle();
        }
        return;
    }
    m_beginFindTarget.Disable();
    CheckDistance(pTarget);
}

void SentryAI::Targeted(SystemEntity* pAgressor) {
    double targetTime = GetTargetTime();
    switch(m_state) {
        case State::Idle: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) in Idle. Begin Targeting sequence.", \
                    m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());

            bool chase = false;
            if (!m_npc->TargetMgr()->StartTargeting( pAgressor, targetTime, (uint8)m_npc->GetSelf()->GetAttribute(AttrMaxAttackTargets).get_int(), m_sightRange, chase)) {
                if (!chase) {
                    _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.", \
                            m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
                    SetIdle();
                }
            }
            m_beginFindTarget.Disable();
        } break;
        /** @todo  determine if new targetedby entity is weaker than current target. use optimalSigRadius to test for 'optimal' target */
        case State::Engaged: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while engaged.", \
                    m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case State::Signaling: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while signaling.", \
                    m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
    }
}

void SentryAI::TargetLost(SystemEntity* pTarget) {
    if (m_state == State::Engaged) {
        if (m_npc->TargetMgr()->HasNoTargets()) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) lost. No targets remain.  Return to Idle.", \
                    m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
            SetIdle();
        } else {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) lost, but more targets remain.", \
                    m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
            /** @todo engage weakest target in current list */
        }
    }
}

void SentryAI::Attack(SystemEntity* pTarget)
{
    if (m_mainAttackTimer.Check()) {
        if (!pTarget) return;
        // Check to see if the target still in the bubble (Client warped out)
        if (!m_npc->SysBubble()->InBubble(pTarget->GetPosition())) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) no longer in bubble.  Clear target and move on",
                 m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
            ClearTarget(pTarget);
            return;
        }
        if (!pTarget->DestinyMgr()) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) has no destiny manager.  Clear target and move on",
                 m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
            ClearTarget(pTarget);
            return;
        }
        // Check to see if the target is not cloaked:
        if (pTarget->DestinyMgr()->IsCloaked()) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) is cloaked.  Clear target and move on",
                 m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
            ClearTarget(pTarget);
            return;
        }
        if (m_npc->TargetMgr()->CanAttack())
            AttackTarget(pTarget);
    }
}

//also check for special effects and write code to implement them
//modifyTargetSpeedRange, modifyTargetSpeedChance
//entityWarpScrambleChance
void SentryAI::AttackTarget(SystemEntity* pTarget) {
    // some npcs use missiles.....write code for using missiles   -- entityMissileTypeID
    std::string guid = "effects.Laser";
    // sentry does NOT have a destiny manager...use target's destiny manager for sending fx
    pTarget->DestinyMgr()->SendSpecialEffect(m_npc->GetSelf()->itemID(),
                                             m_npc->GetSelf()->itemID(),
                                             m_npc->GetSelf()->typeID(), //m_npc->GetSelf()->GetAttribute(AttrGfxTurretID).get_int(),
                                             pTarget->GetID(),
                                             0,guid,1,1,1,m_attackSpeed,0);

    Damage d(m_npc,
             m_npc->GetSelf(),
             m_npc->GetKinetic(),
             m_npc->GetThermal(),
             m_npc->GetEM(),
             m_npc->GetExplosive(),
             m_formula.GetSentryToHit(m_npc, pTarget),
             EVEEffectID::targetAttack
    );

    d *= m_damageMultiplier;
    pTarget->ApplyDamage(d);
}

double SentryAI::GetTargetTime()
{
    double targetTime = (m_npc->GetSelf()->GetAttribute(AttrScanSpeed).get_int());
    float radius = m_npc->GetSelf()->GetAttribute(AttrRadius).get_float();
    if (!targetTime) {
        if (radius < 30) {
            targetTime = 1500;
        } else if (radius < 60) {
            targetTime = 2500;
        } else if (radius < 150) {
            targetTime = 4000;
        } else if (radius < 280) {
            targetTime = 6000;
        } else if (radius < 550) {
            targetTime = 8000;
        } else {
            targetTime = 13000;
        }
    }
    return targetTime;
}

std::string SentryAI::GetStateName(SentryAI::State name)
{
    switch (name) {
        case State::Idle:      return "Idle";
        case State::Engaged:   return "Engaged";
        case State::Signaling: return "Signaling";
    }
    return "Undefined";
}
