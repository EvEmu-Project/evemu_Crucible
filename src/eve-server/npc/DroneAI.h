/**
 * DroneAI.h
 *      this class is for drone AI
 *
 * @Author:     Allan
 * @Version:    0.15
 * @Date:       27Nov19
 */

#ifndef __EVEMU_SHIP_DRONEAI_H__
#define __EVEMU_SHIP_DRONEAI_H__

#include "ship/modules/TurretFormulas.h"
#include "system/DestinyManager.h"

// only for drones
namespace DroneAI {
    namespace State {
        enum {
            Invalid           = -1,
            // defined in client
            Idle              = 0,  // not doing anything....idle.
            Combat            = 1,  // fighting - needs targetID
            Mining            = 2,  // unsure - needs targetID
            Approaching       = 3,  // too close to chase, but to far to engage
            Departing         = 4,  // return to ship
            Departing2        = 5,  // leaving.  different from Departing
            Pursuit           = 6,  // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
            Fleeing           = 7,  // running away
            Operating         = 9,  // whats diff from engaged here?
            Engaged           = 10, // non-combat? - needs targetID
            // internal only
            Unknown           = 8,  // as stated
            Guarding          = 11,
            Assisting         = 12,
            Incapacitated     = 13  //
        };
    }
}

class DroneSE;
class SystemEntity;
class Timer;
class EvilNumber;

class DroneAIMgr {
public:
    DroneAIMgr(DroneSE *who);

    void Process();
    bool IsInControlRange();
    void ProcessCombat();
    void ProcessMining();
    void IdleOrbit();
    void SetFleeing(SystemEntity* pTarget);
    void SetGuarding(SystemEntity* pTarget);
    bool IsHostile(SystemEntity* pTarget);
    void Target(SystemEntity* pTarget);

    // 采矿相关方法
    float GetMiningTargetScore(SystemEntity* pTarget);
    bool IsCargoFull();
    void CheckMiningDistance(SystemEntity* pTarget);
    void SendMiningEffect(SystemEntity* pTarget);
    float CalculateMiningAmount();
    void UpdateAsteroidRadius(SystemEntity* pTarget, float newVolume);
    void TransferOreToCargoHold(SystemEntity* pTarget, float amount);
    float GetOreValueScore(uint16 typeID);

    // 战斗相关方法
    float GetTargetScore(SystemEntity* pTarget);
    float CalculateThreatLevel(SystemEntity* pTarget);
    float GetAssistTargetScore(SystemEntity* pTarget, SystemEntity* pAssistTarget);

    void Targeted(SystemEntity *by_who);
    void TargetLost(SystemEntity *by_who);

    void ClearTargets();
    void ClearAllTargets();

    int8 GetState();

    void SetIdle();
    void Return();
    void AssignShip(ShipSE* pSE)                        { m_assignedShip = pSE; }

    void SetApproaching(SystemEntity* pTarget);
    void SetPursuit(SystemEntity* pTarget);
    void SetAssisting(SystemEntity* pTarget);

    void UseWebifier(SystemEntity* pTarget);
    void UseWarpScrambler(SystemEntity* pTarget);

protected:
    void Attack(SystemEntity* pTarget);
    void SetEngaged(SystemEntity* pTarget);
    void ClearTarget(SystemEntity* pTarget);
    void AttackTarget(SystemEntity* pTarget);
    void CheckDistance(SystemEntity* pTarget);

    int8 m_state;
    std::string GetStateName(int8 stateID);

    DroneSE* m_pDrone;
    DestinyManager* m_destiny;
    ShipSE* m_assignedShip;
    TurretFormulas m_formula;

    // 无人机属性
    float m_emDamage;
    float m_expDamage; 
    float m_kinDamage;
    float m_therDamage;
    float m_orbitRange;
    float m_maxSpeed;
    float m_optimalRange;
    float m_falloff;

    // 无人机控制
    uint32 m_controllerID;
    uint32 m_controllerOwnerID;
    uint32 m_droneControlDistance;

    // 添加警戒范围相关属性
    float m_guardRange;         // 警戒范围
    float m_assistRange;        // 协助范围
    bool m_aggressiveGuard;     // 是否主动攻击进入警戒范围的敌人

private:
    //cached to reduce access times. (faster but uses more memory)
    double m_sigRadius;
    double m_attackSpeed;
    double m_entityFlyRange;
    double m_entityOrbitRange;
    double m_entityChaseRange;
    double m_entityAttackRange;

    uint32 m_chaseSpeed;
    uint32 m_cruiseSpeed;
    uint32 m_targetRange;
    uint32 m_armorRepairDuration;
    uint32 m_shieldBoosterDuration;

    Timer m_processTimer;
    Timer m_mainAttackTimer;
    Timer m_beginFindTarget;
    Timer m_warpScramblerTimer;
    Timer m_webifierTimer;
};

#endif  // __EVEMU_SHIP_DRONEAI_H__