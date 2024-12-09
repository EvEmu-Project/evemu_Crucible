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
    Updates:    Allan
*/

#ifndef __NPCAI_H_INCL__
#define __NPCAI_H_INCL__

#include "ship/Missile.h"
#include "ship/modules/TurretFormulas.h"

namespace NPCAI {
    namespace State {
        enum {
            Invalid     = -1,
            Idle        = 1,  // not doing anything....idle.
            Chasing     = 2,  // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
            Following   = 3,  // too close to chase, but to far to engage
            Engaged     = 4,  // actively fighting
            Fleeing     = 5,  // running away
            Signaling   = 6,  // calling for help
            WarpOut     = 7,  // leaving bubble
            WarpFollow  = 8   // will follow warping ship to their destination (adv)
        };
    }
}

class NPC;
class SystemEntity;
class Timer;
class EvilNumber;

class NPCAIMgr {
protected:
public:
    NPCAIMgr(NPC *who);
    ~NPCAIMgr()                                         { /* do nothing here */ }

    // this is called from NPC::Process() which is called from SystemManager::Process()
    void Process();

    void Target(SystemEntity* pTargSE);
    void Targeted(SystemEntity* pTargSE);
    void TargetLost(SystemEntity* pTargSE);

    void DisableRepTimers(bool shield=true, bool armor=true);

    // public methods to enable calls from other classes (namely, TurretFormulas.cpp)
    bool IsIdle()                                       { return (m_state == NPCAI::State::Idle); }
    bool IsFighting();
    uint16 GetOptimalRange()                            { return m_optimalRange; }
    uint16 GetSigRes()                                  { return m_sigResolution; }
    uint32 GetFalloff()                                 { return m_falloff; }
    uint32 GetAttackRange()                             { return m_maxAttackRange; }
    double GetTrackingSpeed()                           { return m_trackingSpeed; }

    // npcAI methods
    void DisableWarpOutTimer()                          { m_warpOutTimer.Disable(); }
    void WarpOutComplete()                              { m_warpOutTimer.Disable(); m_state = NPCAI::State::Idle; }

    void LaunchMissile(uint16 typeID, SystemEntity* pTargSE);   // us to them
    void MissileLaunched(Missile* pMissile); // them to us

    void SetWarpFollow(SystemEntity* pSE);

protected:
    void Attack(SystemEntity* pTargSE);
    void SetIdle();
    void WarpOut();
    void SetWander();
    void SetChasing(SystemEntity* pTargSE);
    void SetEngaged(SystemEntity* pTargSE);
    void SetFleeing(SystemEntity* pTargSE);
    void ClearTarget(SystemEntity* pTargSE);
    void SetFollowing(SystemEntity* pTargSE);
    void SetSignaling(SystemEntity* pTargSE);
    void AttackTarget(SystemEntity* pTargSE);
    void CheckDistance(SystemEntity* pTargSE);

    float GetTargetTime();

    int8 m_state;

    std::string GetStateName(int8 stateID);

private:
    bool m_webber           :1;
    bool m_warpScram        :1;
    bool m_isWandering      :1;
    bool m_useSigRadius     :1;
    bool m_useTargSwitching :1;
    bool m_useSecondTarget  :1;

    float m_switchTargChance;   //fuzzy logic
    uint16 m_preferedSigRadius;

    //these attributes are cached to reduce access times. (much faster but uses more memory)
    uint8 m_maxAttackTargets;
    uint8 m_maxLockedTargets;
    uint16 m_maxSpeed;
    uint16 m_attackSpeed;
    uint16 m_missileTypeID;
    uint16 m_launcherCycleTime;
    uint16 m_sigResolution;
    uint16 m_orbitSpeed;
    uint16 m_targetRange;   // max targeting range  default: m_maxAttackRange (unused)
    uint16 m_optimalRange;
    uint16 m_boostRange;    // distance for Speed Boost activation   default:2500
    uint16 m_armorRepairDuration;
    uint16 m_shieldBoosterDuration;

    uint32 m_sigRadius;
    uint32 m_falloff;// distance past maximum range at which accuracy has fallen by half
    uint32 m_flyRange;  // npc tries to stay at this distance from active target    default:500
    uint32 m_sightRange;
    uint32 m_maxAttackRange;// max firing range   default:15000
    uint32 m_warpScramRange;

    float m_warpScramChance;
    float m_armorRepairDelayChance;
    float m_shieldBoosterDelayChance;

    double m_trackingSpeed;
    float m_damageMultiplier;

    NPC* m_npc;
    DestinyManager* m_destiny;
    InventoryItemRef m_self;

    TurretFormulas m_formula;

    Timer m_processTimer;
    Timer m_mainAttackTimer;
    Timer m_missileTimer;
    Timer m_shieldBoosterTimer;
    Timer m_armorRepairTimer;
    Timer m_beginFindTarget;
    Timer m_warpOutTimer;
    Timer m_warpScramblerTimer;
    Timer m_webifierTimer;

    float m_fleeHealthThreshold;     // Health threshold to trigger fleeing
    float m_signalHealthThreshold;   // Health threshold to trigger signaling
    uint32 m_signalRange;           // Range for signaling other NPCs
    uint32 m_maxChaseTime;          // Maximum time to chase target
    Timer m_chaseTimer;             // Timer for chase duration
    Timer m_signalTimer;            // Timer for signaling cooldown
    bool m_hasSignaled;             // Flag to track if already signaled
    
    void CheckForReinforcements(SystemEntity* pSE);
    bool ShouldFlee();
    void SignalNearbyNPCs();

    // Add warp follow related members
    Timer m_warpFollowTimer;         // Timer to check warp follow status
    uint32 m_warpFollowTimeout;      // Maximum time to wait for warp follow
    uint32 m_warpFollowDistance;     // Distance to maintain during warp follow
    bool m_isWarpFollowing;          // Flag to track warp follow state
    SystemEntity* m_warpFollowTarget; // Target being followed in warp

    // Add NPC class and behavior related members
    uint8 m_npcClass;        // NPC class from npcClassGroup
    uint8 m_npcSubClass;     // NPC sub-class for specific behaviors
    uint8 m_spawnClass;      // Spawn class from npcSpawnClass
    bool m_isCommander;      // Flag for commander NPCs
    bool m_isOfficer;        // Flag for officer NPCs

    // Add behavior control methods
    void InitializeNPCClass();
    void ProcessClassBehavior();
    bool ShouldUseECM();      // Electronic Counter Measures
    bool ShouldUseEwar();     // Electronic Warfare
    bool ShouldUseDamps();    // Sensor Dampeners
    bool ShouldUseNeut();     // Energy Neutralizers
    bool ShouldUseNos();      // Energy Vampires

    // Add module use methods
    void UseEwarModule();
    void UseECMModule();
    void UseDampsModule(); 
    void UseNeutModule();
    void UseNosModule();

    // Effect IDs from dgmEffects.sql
    static const uint32 EFFECT_ECM = 38;              // ECM effect
    static const uint32 EFFECT_TARGET_JAM = 39;       // Target jam
    static const uint32 EFFECT_ENERGY_NEUT = 40;      // Energy neutralizer
    static const uint32 EFFECT_ENERGY_VAMP = 41;      // Energy vampire
    static const uint32 EFFECT_SENSOR_DAMP = 42;      // Sensor dampener
};

#endif