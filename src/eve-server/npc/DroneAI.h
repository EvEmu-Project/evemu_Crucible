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

namespace DroneAI {
    // drone activity states, as the client's drones window labels them.  sent in OnDroneStateChange and the SetState droneState
    namespace State {
        enum {
            Idle            = 0,
            Combat          = 1,    // "Fighting"
            Mining          = 2,
            Approaching     = 3,
            Returning       = 4,    // returning to orbit the ship
            ReturningToBay  = 5,
            Following       = 6,    // guarding or assisting another pilot
            Fleeing         = 7,
            Operating       = 9,    // electronic warfare
            Repairing       = 10    // logistics
        };
    }
    // what the drone has been told to do
    namespace Order {
        enum {
            Idle            = 0,    // orbit the controlling ship
            Engage          = 1,
            Mine            = 2,    // one cycle, then return and orbit
            MineRepeatedly  = 3,
            ReturnHome      = 4,
            ReturnBay       = 5,
            Guard           = 6,
            Assist          = 7
        };
    }
    // what the drone's main effect does to its target
    namespace Action {
        enum {
            None            = 0,
            Attack          = 1,
            Mine            = 2,
            RepairShield    = 3,
            RepairArmor     = 4,
            Web             = 5,
            Ewar            = 6     // jammers, painters, dampeners, disruptors, neutralizers.  effects only, for now
        };
    }
}

class DroneSE;
class SystemEntity;
class ShipSE;

class DroneAIMgr {
public:
    DroneAIMgr(DroneSE* pDrone);

    // called once per tick while the drone is controlled
    void Process();

    /* orders.  from EntityBound commands, drone settings and aggression */
    void Engage(SystemEntity* pTarget);
    void Mine(SystemEntity* pTarget, bool repeat);
    void ReturnHome();
    void ReturnToBay();
    void Guard(uint32 charID);
    void Assist(uint32 charID);
    void SetIdle();             // orbit the controlling ship
    void Stop();                // control lost.  the drone stops where it is

    // our target lock was lost (target destroyed, removed, jammed...)
    void TargetLost(SystemEntity* pTarget);
    // someone attacked our controller, a pilot we are guarding, or us
    void Aggressed(SystemEntity* pAttacker);

    uint8 GetState() const                              { return m_state; }
    uint8 GetOrder() const                              { return m_order; }
    uint8 GetAction() const                             { return m_action; }
    uint32 GetTargetID() const                          { return m_targetID; }
    uint32 GetActivityCharID() const                    { return m_activityCharID; }
    bool IsMining() const                               { return (m_cycleActive and (m_action == DroneAI::Action::Mine)); }

private:
    void ProcessIdle(ShipSE* pController);
    void ProcessReturn(ShipSE* pController, bool toBay);
    void ProcessEngage(ShipSE* pController);
    void ProcessMine(ShipSE* pController);
    void ProcessGuard(ShipSE* pController);

    void SetOrder(uint8 order, uint32 targetID=0);
    void SetState(uint8 state);
    void StopCycle();

    // movement.  each only sends anything when it changes what the drone is doing
    void OrbitTarget(SystemEntity* pTarget, double range, double speed);
    void ApproachTarget(SystemEntity* pTarget, double range, double speed);
    void StopMoving();

    // effects of one cycle of our main effect
    void Fire(SystemEntity* pTarget);
    void FinishMiningCycle(ShipSE* pController, SystemEntity* pRoid);
    void SendEffect(SystemEntity* pTarget, bool start);
    void StartWeb(SystemEntity* pTarget);
    void StopWeb();

    // target lock with the drone's own sensors
    bool LockTarget(SystemEntity* pTarget);
    void ReleaseTarget();

    // the target this drone should be attacking, for guard/assist/aggression
    SystemEntity* PickAggressor();

    DroneSE* m_pDrone;          //we do not own this

    uint8 m_state;
    uint8 m_order;
    uint8 m_action;
    uint8 m_moveMode;
    uint32 m_targetID;          // engage/mine target
    uint32 m_activityCharID;    // pilot we guard/assist
    uint32 m_moveTargetID;
    uint32 m_aggressorID;       // most recent attacker, for aggressive mode and guarding
    uint32 m_webbedID;          // target we are currently webbing
    double m_moveRange;
    double m_moveSpeed;

    bool m_cycleActive;
    Timer m_cycleTimer;         // weapon/mining/repair cycle

    TurretFormulas m_formula;
};

#endif  // __EVEMU_SHIP_DRONEAI_H__
