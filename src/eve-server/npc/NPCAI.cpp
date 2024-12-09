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
    AI Version: 0.57
*/

/** @todo  ai update ideas
 *   bubble call *SomeFunction* to tell ai of new ship arriving in bubble
 *   method to use npc's preferred sig radius for targets
 *   finish flee and signal action methods (and determine who can use them and when)
 *      - this should take system sov, npc anomalies, destruction speed, and pirate faction
 *   add methods to check target/targeter warping out and chance of npc following (and possibly calling backup)
 *
 *  have data...needs coding...
 *   chase duration/distance timers
 *   ewar shit, including point/tackle
 */

#include "eve-server.h"

#include "Client.h"
#include "inventory/AttributeEnum.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "ship/Missile.h"
#include "system/DestinyManager.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"

NPCAIMgr::NPCAIMgr(NPC* who)
: m_state(NPCAI::State::Idle),
  m_npc(who),
  m_destiny(who->DestinyMgr()),
  m_self(who->GetSelf()),
  m_processTimer(0),
  m_mainAttackTimer(0),
  m_missileTimer(0),
  m_warpOutTimer(0),
  m_shieldBoosterTimer(0),
  m_armorRepairTimer(0),
  m_beginFindTarget(0),
  m_warpScramblerTimer(0),
  m_webifierTimer(0),
  m_missileTypeID(0),
  m_webber(false),
  m_warpScram(false),
  m_isWandering(false)
{
    assert(m_self.get() != nullptr);
    m_damageMultiplier = m_self->GetAttribute(AttrDamageMultiplier).get_float();

    /* set npc ship data */
    m_sigResolution = m_self->GetAttribute(AttrOptimalSigRadius).get_uint32();
    m_attackSpeed = m_self->GetAttribute(AttrSpeed).get_uint32();
    m_sigRadius = m_self->GetAttribute(AttrSignatureRadius).get_uint32();
    m_launcherCycleTime = m_self->GetAttribute(AttrMissileLaunchDuration).get_uint32();
    if (m_launcherCycleTime > 100)
        m_missileTypeID = m_self->GetAttribute(AttrEntityMissileTypeID).get_uint32();

    //  AttrEntityDefenderChance = 497,  <<< for defender missiles

    /** @todo  all of these need to be verified and/or updated */

    // ship speeds
    // absolute (boosted) Max Ship Speed
    m_maxSpeed = m_self->GetAttribute(AttrMaxVelocity).get_uint32();
    // Orbit Velocity
    m_orbitSpeed = m_self->GetAttribute(AttrEntityCruiseSpeed).get_uint32();   // ship speed when not chasing target
    //AttrEntityChaseMaxDelay  - time before 'chase speed' kicks in
    //AttrEntityChaseMaxDelayChance  - chance npc will wait AttrEntityChaseMaxDelay before chasing
    //AttrEntityChaseMaxDuration  - max time a chase will last (unless weapons fired)
    //AttrEntityChaseMaxDurationChance  - chance that any chase will last for AttrEntityChaseMaxDuration

    // ship distances
    //AttrEntityMaxWanderRange
    // Optimal Range  - TODO: test for 0
    m_optimalRange = m_self->GetAttribute(AttrMaxRange).get_uint32();  // distance which npc starts using weapons
    // Accuracy falloff  (distance past optimal range at which accuracy has fallen by half) - TODO: test for 0
    m_falloff = m_self->GetAttribute(AttrFalloff).get_uint32();
    m_trackingSpeed = m_self->GetAttribute(AttrTrackingSpeed).get_double();  //rad/sec
    // Orbit Range, Follow Range  - npc tries to stay at this distance from active target
    m_flyRange = m_self->GetAttribute(AttrEntityFlyRange).get_uint32();    //AttrOrbitRange is 0 for npc
    if (!m_flyRange)
        m_flyRange = 0;
    // distance for Speed Boost activation  (this needs to be revisited)
    m_boostRange = m_self->GetAttribute(AttrEntityChaseMaxDistance).get_uint32();
    if (!m_boostRange)
        m_boostRange = 0;
    // some npcs have flyRange > boostRange.  this corrects it. (extends boost range)
    if (m_flyRange > m_boostRange)
        m_boostRange += m_boostRange + m_flyRange;
    // max firing range   default:10000  (lowest in db is 1000)
    m_maxAttackRange = m_self->GetAttribute(AttrEntityAttackRange).get_uint32();
    // this should be set according to npc size.
    if (m_maxAttackRange < 1000)
        m_maxAttackRange = 10000;

    // 'sight' range (undefined in db)
    float radius = m_self->GetAttribute(AttrRadius).get_float();
    if (radius < 30) {
        m_sightRange = 2500;
    } else if (radius < 60) {
        m_sightRange = 5000;
    } else if (radius < 150) {
        m_sightRange = 8000;
    } else if (radius < 280) {
        m_sightRange = 12000;
    } else if (radius < 550) {
        m_sightRange = 15000;
    } else {
        m_sightRange = 20000;
    }
    if (m_maxAttackRange > m_sightRange)
        m_sightRange = m_maxAttackRange *2;

    // ship targets
    m_maxAttackTargets = m_self->GetAttribute(AttrMaxAttackTargets).get_uint32();
    if (m_maxAttackTargets < 1)
        m_maxAttackTargets = 1;
    m_maxLockedTargets = m_self->GetAttribute(AttrMaxLockedTargets).get_uint32();
    if (m_maxLockedTargets < 1) {
        if (m_maxAttackTargets > 1) {
            m_maxLockedTargets = m_maxAttackTargets;
        } else {
            m_maxLockedTargets = 1;
        }
    }

    /** @todo change these next 2 (rep and boost) to boolean to avoid timer creation/checks */

    // this is chance an npc has of delaying it's rep (if applicable)
    if (m_self->HasAttribute(AttrEntityArmorRepairDelayChance)) {
        m_armorRepairDelayChance = m_self->GetAttribute(AttrEntityArmorRepairDelayChance).get_float();
    } else if (m_self->HasAttribute(AttrEntityArmorRepairDelayChanceSmall)) {
        m_armorRepairDelayChance = m_self->GetAttribute(AttrEntityArmorRepairDelayChanceSmall).get_float();
    } else if (m_self->HasAttribute(AttrEntityArmorRepairDelayChanceMedium)) {
        m_armorRepairDelayChance = m_self->GetAttribute(AttrEntityArmorRepairDelayChanceMedium).get_float();
    } else if (m_self->HasAttribute(AttrEntityArmorRepairDelayChanceLarge)) {
        m_armorRepairDelayChance = m_self->GetAttribute(AttrEntityArmorRepairDelayChanceLarge).get_float();
    } else {
        m_armorRepairDuration = 0;
        m_armorRepairDelayChance = 0;
    }
    if (m_armorRepairDelayChance)
        m_armorRepairDuration = m_self->GetAttribute(AttrEntityArmorRepairDuration).get_uint32();

    // this is chance an npc has of delaying it's sebo (if applicable)
    if (m_self->HasAttribute(AttrEntityShieldBoostDelayChance)) {
        m_shieldBoosterDelayChance = m_self->GetAttribute(AttrEntityShieldBoostDelayChance).get_float();
    } else if (m_self->HasAttribute(AttrEntityShieldBoostDelayChanceSmall)) {
        m_shieldBoosterDelayChance = m_self->GetAttribute(AttrEntityShieldBoostDelayChanceSmall).get_float();
    } else if (m_self->HasAttribute(AttrEntityShieldBoostDelayChanceMedium)) {
        m_shieldBoosterDelayChance = m_self->GetAttribute(AttrEntityShieldBoostDelayChanceMedium).get_float();
    } else if (m_self->HasAttribute(AttrEntityShieldBoostDelayChanceLarge)) {
        m_shieldBoosterDelayChance = m_self->GetAttribute(AttrEntityShieldBoostDelayChanceLarge).get_float();
    } else {
        m_shieldBoosterDuration = 0;
        m_shieldBoosterDelayChance = 0;
    }
    if (m_shieldBoosterDelayChance)
        m_shieldBoosterDuration = m_self->GetAttribute(AttrEntityShieldBoostDuration).get_uint32();

    // advanced AI variables  only used by sleepers for now (and on live).  will update advanced npcs to use these also
    if (m_self->HasAttribute(AttrAI_ShouldUseTargetSwitching)) {
        m_useTargSwitching = true;
    } else {
        m_useTargSwitching = false;
    }
    if (m_self->HasAttribute(AttrAI_ShouldUseSecondaryTarget)) {
        m_useSecondTarget = true;
    } else {
        m_useSecondTarget = false;
    }
    if (m_self->HasAttribute(AttrAI_ShouldUseSignatureRadius)) {
        m_useSigRadius = true;
        m_preferedSigRadius = m_self->GetAttribute(AttrAI_PreferredSignatureRadius).get_uint32();
    } else {
        m_useSigRadius = false;
        m_preferedSigRadius = 0;
    }
    if (m_self->HasAttribute(AttrAI_ChanceToNotTargetSwitch)) {
        m_switchTargChance = 1.0 - m_self->GetAttribute(AttrAI_ChanceToNotTargetSwitch).get_float();
    } else {
        m_switchTargChance = 0;
    }

    if (m_self->HasAttribute(AttrWarpScrambleRange)) {
        m_warpScramRange = m_self->GetAttribute(AttrWarpScrambleRange).get_float();
    } else {
        m_warpScramRange = 0;
    }
    if (m_self->HasAttribute(AttrEntityWarpScrambleChance)) {
        m_warpScramChance = 1.0 - m_self->GetAttribute(AttrEntityWarpScrambleChance).get_float();
    } else {
        m_warpScramChance = 0;
    }

    /*
    AttrWarpScrambleRange = 103,
    AttrWarpScrambleStrength = 105,
    AttrEntityWarpScrambleChance = 504,
    AttrWarpScrambleDuration = 505,
    AttrModifyTargetSpeedRange = 514
    */

    /*
    AttrEntityEquipmentMin = 456,
    AttrEntityEquipmentMax = 457,
    */

    /*
    AttrEntityTargetJam = 928,
    AttrEntityTargetJamDuration = 929,
    AttrEntityTargetJamDurationChance = 930,    // npcActivationChanceAttributeID in dgmEffects
    AttrEntityCapacitorDrainDurationChance = 931,   // npcActivationChanceAttributeID in dgmEffects
    AttrEntitySensorDampenDurationChance = 932,   // npcActivationChanceAttributeID in dgmEffects
    AttrEntityTrackingDisruptDurationChance = 933,   // npcActivationChanceAttributeID in dgmEffects
    AttrEntityTargetPaintDurationChance = 935,   // npcActivationChanceAttributeID in dgmEffects
    AttrEntityTargetJamMaxRange = 936,
    AttrEntityCapacitorDrainMaxRange = 937,
    AttrEntitySensorDampenMaxRange = 938,
    AttrEntityTrackingDisruptMaxRange = 940,
    AttrEntityTargetPaintMaxRange = 941,
    AttrEntityCapacitorDrainDuration = 942,
    AttrEntitySensorDampenDuration = 943,
    AttrEntityTrackingDisruptDuration = 944,
    AttrEntityTargetPaintDuration = 945,
    AttrEntityCapacitorDrainAmount = 946,
    AttrEntitySensorDampenMultiplier = 947,
    AttrEntityTrackingDisruptMultiplier = 948,
    AttrEntityTargetPaintMultiplier = 949,
    AttrEntitySensorDampenFallOff = 950,
    AttrEntityTrackingDisruptFallOff = 951,
    AttrEntityCapacitorFallOff = 952,
    AttrEntityTargetJamFallOff = 953,
    AttrEntityTargetPaintFallOff = 954,
    */

    // does this need to be running if there are no players in bubble?
    //  yes...npcs will warp out when no targets in sight range, but need a process tic to do that.
   // m_processTimer.Start(m_attackSpeed);

    // maybe this can be used to tell spawnMgr to respawn this npc as required....
    //    AttrEntityGroupRespawnChance = 640,
}

void NPCAIMgr::Process() {
    if (m_destiny->IsWarping())
        return;

    if (m_warpOutTimer.Check(false)) {
        // disallow warpout if spawn has active respawn timer (spawn is being chained)
        if (m_npc->GetSpawnMgr() && m_npc->GetSpawnMgr()->IsChaining(m_npc->SysBubble()->GetID())) {
            m_state = NPCAI::State::Idle;
            m_warpOutTimer.Disable();
            // Add destiny stop to ensure movement is halted when canceling warp
            m_destiny->Stop();
        }
    }

    /* NPCAI::State definitions   -allan 25July15  (UD 1June16)
     *   Idle,       // not doing anything, nothing in sight....idle.  call Wander() to loosely orbit random object in bubble ~10-20k at 1/2 orbit speed
     *   Chasing,    // target within npc sight range.  attacking begins here.  use m_maxSpeed to get within falloff
     *   Following,  // between optimal and falloff.  try to get closer, but still orbiting and attacking
     *   Engaged,    // actively fighting (in orbit).  use m_orbitSpeed.
     *   Fleeing,    // running away....use m_maxSpeed then warp away when out of range	(does this make sense??)
     *   Signaling   // calling for help..use m_orbitSpeed *2 to speed tank while calling for reinforcements
     */
    // Add state validation while keeping original comments
    if (m_state < NPCAI::State::Idle || m_state > NPCAI::State::WarpFollow) {
        _log(NPC__ERROR, "Invalid state %d for NPC %s(%u). Resetting to Idle.", 
             m_state, m_npc->GetName(), m_npc->GetID());
        SetIdle();
        return;
    }

    switch(m_state) {
        case NPCAI::State::Idle: {
            if (m_beginFindTarget.Check()) {
                std::vector<Client*> clientVec;
                clientVec.clear();
                DestinyManager* pDestiny(nullptr);
                m_npc->SysBubble()->GetPlayers(clientVec); // what about player drones?  yes...later
                for (auto cur : clientVec) {
                    if (cur->IsInvul())
                        continue;
                    if (cur->GetShipSE() == nullptr)
                        continue;
                    if (cur->InPod()) {
                        if (sConfig.npc.TargetPod) {
                            if (m_npc->SystemMgr()->GetSystemSecurityRating() > sConfig.npc.TargetPodSec)
                                continue;
                        } else {
                            continue;
                        }
                    }
                    pDestiny = cur->GetShipSE()->DestinyMgr();
                    if (pDestiny == nullptr)
                        continue;
                    if (pDestiny->IsCloaked() or pDestiny->IsWarping())
                        continue;
                    if (m_npc->GetPosition().distance(cur->GetShipSE()->GetPosition()) > m_sightRange)
                        continue;

                    Target(cur->GetShipSE());
                    return;
                }
                // Combine conditions to reduce nesting while maintaining functionality
                if (sConfig.npc.IdleWander && !m_isWandering)
                    SetWander();
            } else {
                if (!m_beginFindTarget.Enabled())
                    m_beginFindTarget.Start(m_attackSpeed);  //find target is based on npc attack speed.
            }
        } break;
        case NPCAI::State::Chasing:
        case NPCAI::State::Following:
        case NPCAI::State::Engaged: {
            // Add null check for TargetMgr while keeping existing logic
            if (!m_npc->TargetMgr() || m_npc->TargetMgr()->HasNoTargets()) {
                _log(NPC__AI_TRACE, "%s(%u): Stopped %s - HasNoTargets = true.", 
                     m_npc->GetName(), m_npc->GetID(), GetStateName(m_state).c_str());
                SetIdle();
                return;
            }
            SystemEntity* pSE = m_npc->TargetMgr()->GetFirstTarget(false);
            // Add position validation while maintaining existing checks
            if (pSE == nullptr || pSE->SysBubble() == nullptr) {
                ClearTarget(pSE);
                return;
            }
            // Add position sanity check
            if (pSE->GetPosition().isNaN()) {
                _log(NPC__ERROR, "%s(%u): Target position is invalid. Clearing target.", 
                     m_npc->GetName(), m_npc->GetID());
                ClearTarget(pSE);
                return;
            }
            CheckDistance(pSE);
            if (m_missileTimer.Check())
                LaunchMissile(m_missileTypeID, pSE);
        } break;
        // Keep these states for future implementation
        case NPCAI::State::WarpOut:
        case NPCAI::State::WarpFollow:
        case NPCAI::State::Fleeing:
        case NPCAI::State::Signaling:{
            _log(NPC__AI_TRACE, "%s(%u): Called %s - needs to be completed.", 
                 m_npc->GetName(), m_npc->GetID(), GetStateName(m_state).c_str());
            m_state = NPCAI::State::Idle;
            // not sure how im gonna do these
        } break;
    }

    if (m_shieldBoosterTimer.Enabled())
        if (m_shieldBoosterTimer.Check())
            m_npc->UseShieldRecharge();

    if (m_armorRepairTimer.Enabled())
        if (m_armorRepairTimer.Check())
            m_npc->UseArmorRepairer();
}

bool NPCAIMgr::IsFighting() {
    // more to this here....
    return (m_state != NPCAI::State::Idle);
}

void NPCAIMgr::WarpOut()
{
    m_warpOutTimer.Disable();

    if (m_state == NPCAI::State::WarpOut) {
        m_state = NPCAI::State::Idle;
        return;
    }

    m_state = NPCAI::State::WarpOut;
    SystemManager* pSys = m_npc->SystemMgr();

    /** @todo  eventually, this will check with anomaly mgr for possible npc hideouts in system
     * based on npc faction, system, players in system, players in bubble, and *more later*
     * to determine a warpto target for this npc, or this group
     *
     * for now, if there are players in system, just warp to another belt.
     * if there are no players in this system, avoid using proc tics on npcs
     */

    if (pSys->PlayerCount()) {
        // pSys->GetAnomMgr();
        uint32 newBeltID = pSys->GetRandBeltID();
        if (newBeltID == sBubbleMgr.GetBeltID(m_npc->SysBubble()->GetID()))
            newBeltID = pSys->GetRandBeltID();

        SystemEntity* newSE = pSys->GetSE(newBeltID);
        m_destiny->WarpTo(newSE->GetPosition());
        m_npc->GetSpawnMgr()->MoveSpawn(m_npc, sBubbleMgr.FindBubble(newSE));
    }
}

void NPCAIMgr::SetWander()
{
    if (m_npc->GetSpawnMgr() == nullptr)
        return;
    if (!m_isWandering) {
        _log(NPC__AI_TRACE, "%s(%u): Wandering:  No Targets within my sight range of %um", \
                m_npc->GetName(), m_npc->GetID(), m_sightRange);
        m_isWandering = true;
    }

    SystemBubble* pBubble = m_npc->SysBubble();

    // wandering.  nothing to shoot.  look for target.
    if (pBubble->IsAnomaly() or pBubble->IsIncursion() or pBubble->IsMission()) {
        return;
    } else if (pBubble->HasDynamics() and pBubble->IsBelt()) {
        // pick random entity and loosely orbit it.  if no entity found, orbit center of belt
        SystemEntity* pSE = pBubble->GetRandomEntity();
        if (pSE == nullptr)
            pSE = m_npc->SystemMgr()->GetSE(sBubbleMgr.GetBeltID(pBubble->GetID()));
        if (pSE == nullptr) {
            _log(NPC__ERROR, "%s(%u): Wandering:  No Target or beltSE found.", m_npc->GetName(), m_npc->GetID());
            // nothing here...leave bubble
            WarpOut();
            return;
        }
        m_destiny->SetMaxVelocity(m_orbitSpeed);
        uint16 orbitDistance = MakeRandomInt(10000, 20000);
        m_destiny->Orbit(pSE, orbitDistance);
        _log(NPC__AI_TRACE, "%s(%u):  Just for shits-n-giggles, I\'m gonna orbit %s(%u) at %um.", \
                m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID(), orbitDistance);
    } else {
        /** @todo  figure out a way for npc to wander 'aimlessly' around their bubble */
        m_destiny->Stop();
    }
}

void NPCAIMgr::SetIdle() {
    if (m_state == NPCAI::State::Idle)
        return;
    // not doing anything....idle.

    /** @todo need to clear out targets here */

    _log(NPC__AI_TRACE, "%s(%u): Idle: returning to idle.", m_npc->GetName(), m_npc->GetID());
    m_state = NPCAI::State::Idle;
    m_destiny->Stop();
    m_destiny->SetMaxVelocity(m_orbitSpeed);
    // Add speed fraction reset to ensure complete stop
    m_destiny->SetSpeedFraction(0.0f);

    // Keep all timer disables - each has specific functionality
    m_missileTimer.Disable();        // Stops missile launches
    m_webifierTimer.Disable();       // Stops webifier effects
    m_beginFindTarget.Disable();     // Stops target searching
    m_mainAttackTimer.Disable();     // Stops main weapon attacks  
    m_armorRepairTimer.Disable();    // Stops armor repair cycles
    m_warpScramblerTimer.Disable();  // Stops warp scrambling effects
    m_shieldBoosterTimer.Disable();  // Stops shield boost cycles

    m_isWandering = false;  // Reset wandering state

    SystemBubble* pBubble = m_npc->SysBubble();
    //disallow warpout if anomaly, incursion or mission rat
    if (pBubble && (pBubble->IsAnomaly() || pBubble->IsIncursion() || pBubble->IsMission()))
        return;

    //disallow warpout by NOT setting timer.
    if (sConfig.npc.WarpOut > 0)
        if (m_npc->GetSpawnMgr() != nullptr)
            m_warpOutTimer.Start(sConfig.npc.WarpOut * 1000); // s to ms
}

void NPCAIMgr::SetChasing(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    /** @todo implement chase timer using entityChaseMaxDuration to limit chase time. */
    if ((m_state == NPCAI::State::Chasing) and (m_destiny->IsGoto() or m_destiny->IsFollowing()))
        return;
    _log(NPC__AI_TRACE, "%s(%u): Begin chasing.  Target is %s(%u).", \
         m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
    // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
    m_destiny->SetMaxVelocity(m_maxSpeed);
    m_destiny->GotoPoint(pSE->GetPosition());  //head towards target
    m_state = NPCAI::State::Chasing;
    m_warpOutTimer.Disable();
}

void NPCAIMgr::SetFollowing(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    if ((m_state == NPCAI::State::Following) and (m_destiny->IsGoto() or m_destiny->IsFollowing()))
        return;
    _log(NPC__AI_TRACE, "%s(%u): Begin following.  Target is %s(%u).", \
         m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
    // too close to chase, but to far to engage
    m_destiny->SetMaxVelocity(m_orbitSpeed *2);
    m_destiny->Follow(pSE, m_falloff);  //try to get inside falloff range
    m_state = NPCAI::State::Following;
    m_warpOutTimer.Disable();
}

void NPCAIMgr::SetEngaged(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    if ((m_state == NPCAI::State::Engaged) and m_destiny->IsOrbiting())
        return;
    _log(NPC__AI_TRACE, "%s(%u): Begin engaging.  Target is %s(%u).", \
         m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
    // actively fighting
    m_destiny->SetMaxVelocity(m_orbitSpeed);
    m_destiny->Orbit(pSE, m_optimalRange);  //try to get inside orbit range
    m_state = NPCAI::State::Engaged;
    m_warpOutTimer.Disable();
}

// not used yet
void NPCAIMgr::SetFleeing(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    if ((m_state == NPCAI::State::Fleeing) and m_destiny->IsMoving())
        return;
    _log(NPC__AI_TRACE, "%s(%u): Begin fleeing.  Target is %s(%u).", \
         m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
    // actively fleeing
    //  use superspeed to disengage, then warp.  << both these will need to be written.
    //  this state is only usable by higher-class npcs.
    m_destiny->SetMaxVelocity(m_maxSpeed);
    m_state = NPCAI::State::Fleeing;
    m_warpOutTimer.Disable();
}

// not used yet
void NPCAIMgr::SetSignaling(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    if ((m_state == NPCAI::State::Signaling) and m_destiny->IsOrbiting())
        return;
    _log(NPC__AI_TRACE, "%s(%u): Begin signaling.  Target is %s(%u).", \
         m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
    // actively signaling
    //  start speedtanking while signaling.  (im sure this is cheating, but fuckem.)
    //  this state is only usable by higher-class npcs.
    m_destiny->SetMaxVelocity(m_orbitSpeed * 2);
    m_destiny->Orbit(pSE, m_falloff);  //try to get outside orbit range
    m_state = NPCAI::State::Signaling;
    m_warpOutTimer.Disable();
}

void NPCAIMgr::CheckDistance(SystemEntity* pSE)
{
    if (pSE == nullptr)
        return;
    double dist = m_npc->GetPosition().distance(pSE->GetPosition());
    if ((dist > m_sightRange) and (!m_npc->TargetMgr()->IsTargetedBy(pSE))) {
        _log(NPC__AI_TRACE, "%s(%u): CheckDistance: %s(%u) is too far away (%.0fm).  Return to Idle.", \
             m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID(), dist);
        if (m_state != NPCAI::State::Idle) {
            // target is no longer in npc's "sight range" and is NOT targeting this npc.  unlock target and return to idle.
            //   should we do anything else here?  search for another target?  wander around?  yes..later
            // if npc is targeted greater than this distance, it will chase
            ClearTarget(pSE);
        }
        return;
    }

    m_isWandering = false;

    if (dist < m_flyRange) {
        SetEngaged(pSE);
    } else if (dist < m_boostRange) {
        SetFollowing(pSE);
    } else {
        SetChasing(pSE);
    }

    _log(NPC__AI_TRACE, "%s(%u): CheckDistance:  target: %s(%u), state: %s, dist: %.0fm, flyRange: %u, boostRange: %u.", \
            m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID(), GetStateName(m_state).c_str(), dist, m_flyRange, m_boostRange);

    Attack(pSE);
}

void NPCAIMgr::Target(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    float targetTime = GetTargetTime();
    bool chase = false;

    if (!m_npc->TargetMgr()->StartTargeting(pSE, targetTime, m_maxLockedTargets, m_sightRange, chase)) {
        if (chase) {
            _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Begin Chasing.", \
                        m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
            SetChasing(pSE);
        } else {
            _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.", \
                        m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
            SetIdle();
        }
        return;
    }
    m_beginFindTarget.Disable();
    CheckDistance(pSE);

    if (!m_mainAttackTimer.Enabled())
        m_mainAttackTimer.Start(m_attackSpeed);

    if (!m_missileTimer.Enabled() and (m_launcherCycleTime > 100))
        m_missileTimer.Start(m_launcherCycleTime);
}

void NPCAIMgr::Targeted(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    double targetTime = GetTargetTime();

    _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while %s.", \
            m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID(), GetStateName(m_state).c_str());

    switch(m_state) {
        case NPCAI::State::Idle: {
            _log(NPC__AI_TRACE, "%s(%u): Begin Approaching and start Targeting sequence.", \
                    m_npc->GetName(), m_npc->GetID());
            SetChasing(pSE);

            bool chase = false;
            if (!m_npc->TargetMgr()->StartTargeting( pSE, targetTime, m_maxLockedTargets, m_sightRange, chase)) {
                if (chase) {
                    _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Begin Chasing.", \
                            m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
                    SetChasing(pSE);
                } else {
                    _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.", \
                            m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
                    SetIdle();
                }
            }
            m_beginFindTarget.Disable();
            //CheckDistance(pAgressor);
        } break;

        /** @todo  determine if new targetedby entity is weaker than current target. use optimalSigRadius to test for 'optimal' target */
        case NPCAI::State::Chasing: {
        } break;
        case NPCAI::State::Following: {
        } break;
        case NPCAI::State::Engaged: {
        } break;
        case NPCAI::State::Fleeing: {
        } break;
        case NPCAI::State::Signaling: {
        } break;
    }
    if (!m_shieldBoosterTimer.Enabled())
        if (MakeRandomFloat() > m_shieldBoosterDelayChance)
            m_shieldBoosterTimer.Start(m_shieldBoosterDuration);
    if (!m_armorRepairTimer.Enabled())
        if (MakeRandomFloat() > m_armorRepairDelayChance)
            m_armorRepairTimer.Start(m_armorRepairDuration);
}

void NPCAIMgr::TargetLost(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    switch(m_state) {
        case NPCAI::State::Chasing:
        case NPCAI::State::Following:
        case NPCAI::State::Engaged: {
            // implement chance for npc to follow warping player
            // sConfig.npc.WarpFollowChance;
            // NPCAI::State::WarpFollow
            if (m_npc->TargetMgr()->HasNoTargets()) {
                _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) lost. No targets remain.  Return to Idle.", \
                        m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
                SetIdle();
            } else {
                _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) lost, but more targets remain.", \
                        m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
                /** @todo engage weakest target in current list */
                Attack(m_npc->TargetMgr()->GetFirstTarget(true));
            }
        }
    }
}

void NPCAIMgr::Attack(SystemEntity* pSE)
{
    if (pSE == nullptr)
        return;
    if (m_mainAttackTimer.Check()) {
        // Check to see if the target still in the bubble (Client warped out)
        if (!m_npc->SysBubble()->InBubble(pSE->GetPosition())) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) no longer in bubble.  Clear target and move on",
                    m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
            m_missileTimer.Disable();
            ClearTarget(pSE);
            return;
        }
        if (pSE->DestinyMgr() == nullptr) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) has no destiny manager.  Clear target and move on",
                    m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
            m_missileTimer.Disable();
            ClearTarget(pSE);
            return;
        }
        // Check to see if the target is not cloaked:
        if (pSE->DestinyMgr()->IsCloaked()) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) is cloaked.  Clear target and move on",
                    m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
            m_missileTimer.Disable();
            ClearTarget(pSE);
            return;
        }
        if (m_npc->TargetMgr()->CanAttack())
            AttackTarget(pSE);
    }
}

void NPCAIMgr::ClearTarget(SystemEntity* pSE) {
    m_npc->TargetMgr()->ClearTarget(pSE);
    //m_npc->TargetMgr()->OnTarget(pSE, TargMgr::Mode::Lost);

    if (m_npc->TargetMgr()->HasNoTargets())
        SetIdle();
}

//also check for special effects and write code to implement them
//modifyTargetSpeedRange, modifyTargetSpeedChance
//entityWarpScrambleChance
void NPCAIMgr::AttackTarget(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    // put checks here for point/tackle

    // effects are listed in EVE_Effects.h
    std::string guid = "effects.Laser"; // client looks for 'turret' in ship.ball.modules for 'effects.laser'
    uint32 gfxID = 0;
    if (m_self->HasAttribute(AttrGfxTurretID))// graphicID for turret for drone type ships
        gfxID = m_self->GetAttribute(AttrGfxTurretID).get_uint32();
    m_destiny->SendSpecialEffect(m_self->itemID(), m_self->itemID(), m_self->typeID(),
                                 pSE->GetID(),0,guid,1,1,
                                 1,m_attackSpeed,0,gfxID);

    Damage d(m_npc,
             m_self,
             m_npc->GetKinetic(),
             m_npc->GetThermal(),
             m_npc->GetEM(),
             m_npc->GetExplosive(),
             m_formula.GetNPCToHit(m_npc, pSE),
             EVEEffectID::targetAttack
            );

    if (sConfig.npc.UseDamageMultiplier)
        if (m_damageMultiplier > 0)
            d *= m_damageMultiplier;

    pSE->ApplyDamage(d);
}

/* missile shit..
 * //AttrEntityDefenderChance  - chance to shoot defender missile at incomming missile
 * //AttrMissileLaunchDuration  - missile cycle time
 * //AttrEntityMissileTypeID
 * //AttrMissileEntityVelocityMultiplier
 * //AttrMissileEntityFlightTimeMultiplier
 * //AttrMissileEntityAoeCloudSizeMultiplier
 * //AttrMissileEntityAoeVelocityMultiplier
 * //AttrMissileEntityAoeFalloffMultiplier
 */

void NPCAIMgr::LaunchMissile(uint16 typeID, SystemEntity* pSE)
{
    if (typeID == 0)
        return;
    // Actually Launch a missile, creating a new Destiny object for it
    // ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, const char *_name = "", 
    //          const GPoint &_position = NULL_ORIGIN, const char *_customInfo = "", bool _contraband = false);
    ItemData idata(typeID, m_npc->GetID(), m_npc->GetLocationID(), flagMissile, "NPC Missile", m_npc->GetPosition());
    InventoryItemRef missileRef = sItemFactory.SpawnItem(idata);
    if (missileRef.get() == nullptr)
        return;  // make error here

    // modify missile based on npc attribs
    if (m_self->HasAttribute(AttrMissileEntityVelocityMultiplier))
        missileRef->MultiplyAttribute(AttrMaxVelocity, m_self->GetAttribute(AttrMissileEntityVelocityMultiplier));
    if (m_self->HasAttribute(AttrMissileEntityFlightTimeMultiplier))    // this may be wrong
        missileRef->MultiplyAttribute(AttrExplosionDelay, m_self->GetAttribute(AttrMissileEntityFlightTimeMultiplier));
    if (m_self->HasAttribute(AttrMissileEntityAoeVelocityMultiplier))
        missileRef->MultiplyAttribute(AttrAoeVelocity, m_self->GetAttribute(AttrMissileEntityAoeVelocityMultiplier));
    if (m_self->HasAttribute(AttrMissileEntityAoeCloudSizeMultiplier))
        missileRef->MultiplyAttribute(AttrAoeCloudSize, m_self->GetAttribute(AttrMissileEntityAoeCloudSizeMultiplier));
    if (m_self->HasAttribute(AttrMissileEntityAoeFalloffMultiplier))
        missileRef->MultiplyAttribute(AttrAoeFalloff, m_self->GetAttribute(AttrMissileEntityAoeFalloffMultiplier));

    SystemManager* pSystem = m_npc->SystemMgr();
    // Missile(InventoryItemRef self, PyServiceMgr &services, SystemManager* system, InventoryItemRef module, SystemEntity* target, ShipItem* ship);
    Missile* pMissile = new Missile(missileRef, pSystem->GetServiceMgr(),  pSystem, m_self, pSE, m_npc);
    if (pMissile == nullptr)
        return; // make error here
    double distance = pMissile->GetPosition().distance(pSE->GetPosition());
    double missileSpeed = missileRef->GetAttribute(AttrMaxVelocity).get_float();
    double travelTime = (distance/missileSpeed);
    if (travelTime < 1)
        travelTime = 1;
    pMissile->SetSpeed(missileSpeed);
    pMissile->SetHitTimer(travelTime *1000);
    pMissile->DestinyMgr()->MakeMissile(pMissile);

    // tell target a missile has been launched at them.. (defender missile trigger for ship, tower, pos, npc, others?)
    if (typeID != 265)  // but only if it's NOT a defender missile
        pSE->MissileLaunched(pMissile);
}

void NPCAIMgr::MissileLaunched(Missile* pMissile)
{
    float chance = m_self->GetAttribute(AttrEntityDefenderChance).get_float();
    if (sConfig.npc.DefenderMissileChance)
        chance = sConfig.npc.DefenderMissileChance;
    // check chance to shoot defender missile at incomming missile (working, ??/??/??)
    if (MakeRandomFloat() < chance)
        LaunchMissile(265, pMissile); // defender missile
}

float NPCAIMgr::GetTargetTime()
{
    float targetTime = (m_self->GetAttribute(AttrScanSpeed).get_float());
    float radius = m_self->GetAttribute(AttrRadius).get_float();
    if (targetTime < 1) {
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

void NPCAIMgr::DisableRepTimers(bool shield/*true*/, bool armor/*true*/)
{
    if (armor)
        m_armorRepairTimer.Disable();
    if (shield)
        m_shieldBoosterTimer.Disable();
}

std::string NPCAIMgr::GetStateName(int8 stateID)
{
    switch (stateID) {
        case NPCAI::State::Idle:           return "Idle";
        case NPCAI::State::Chasing:        return "Chasing";
        case NPCAI::State::Engaged:        return "Engaged";
        case NPCAI::State::Fleeing:        return "Fleeing";
        case NPCAI::State::Following:      return "Following";
        case NPCAI::State::Signaling:      return "Signaling";
        case NPCAI::State::WarpOut:        return "Warping Out";
        case NPCAI::State::WarpFollow:     return "Following Warp";
        default:                           return "Invalid";
    }
}
