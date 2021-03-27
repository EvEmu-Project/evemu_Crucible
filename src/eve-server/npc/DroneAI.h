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

    void Target(SystemEntity *by_who);
    void Targeted(SystemEntity *by_who);
    void TargetLost(SystemEntity *by_who);

    void ClearTargets();
    void ClearAllTargets();

    int8 GetState();

    void SetIdle();
    void Return();
    void AssignShip(ShipSE* pSE)                        { m_assignedShip = pSE; }

protected:
    void Attack(SystemEntity* pTarget);
    void SetEngaged(SystemEntity* pTarget);
    void ClearTarget(SystemEntity* pTarget);
    void AttackTarget(SystemEntity* pTarget);
    void CheckDistance(SystemEntity* pTarget);

    int8 m_state;
    std::string GetStateName(int8 stateID);

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

    DroneSE* m_pDrone;
    ShipSE* m_assignedShip;

    TurretFormulas m_formula;

    Timer m_processTimer;
    Timer m_mainAttackTimer;
    Timer m_beginFindTarget;
    Timer m_warpScramblerTimer;
    Timer m_webifierTimer;
};

#endif  // __EVEMU_SHIP_DRONEAI_H__