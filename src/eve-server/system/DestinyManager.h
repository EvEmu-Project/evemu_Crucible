#ifndef __DESTINYMANAGER_H_INCL__
#define __DESTINYMANAGER_H_INCL__

#include "eve-compat.h"

#include "services/Callable.h"
#include "destiny/DestinyStructs.h"
#include "inventory/ItemRef.h"

// ===== КОНСТАНТЫ ФИЗИКИ =====
#define PHYSICS_FRICTION 1.0
#define MIN_VELOCITY 0.001
#define DEFAULT_STOPPING_DISTANCE 100.0
#define ORBITAL_SPEED_RATIO 0.7
#define ORBIT_RADIUS_CORRECTION 0.01
#define _HALFPI_  1.57079632679489655
#define _PI_      3.1415926535897931
#define _2PI_     6.2831853071795862
// ===== КОНЕЦ КОНСТАНТ =====

namespace Destiny {
    namespace Ball {
        namespace Orbit {
            enum {
                None = 0,
                TooFar = 1,
                TooClose = 2,
                Far = 3,
                Close = 4,
                Orbiting = 5
            };
        }
    }
}

class InventoryItem;
class Missile;
class PyRep;
class PyList;
class PyTuple;
class ShipSE;
class SystemBubble;
class SystemEntity;
class SystemManager;

static const double TURN_ALIGNMENT = 4.0;
static const double WARP_ALIGNMENT = 6.0;
static const uint16 BUMP_DISTANCE = 50;

class DestinyManager {
public:
    DestinyManager(SystemEntity* self);
    ~DestinyManager();

    void Process();
    void ForceSync();

    void SendSingleDestinyEvent(PyTuple** ev, bool self_only=false) const;
    void SendSingleDestinyUpdate(PyTuple** up, bool self_only=false) const;
    void SendDestinyUpdate(std::vector<PyTuple*> &updates, bool self_only=false) const;
    void SendDestinyUpdate(std::vector<PyTuple*> &updates, std::vector<PyTuple*> &events, bool self_only=false) const;

    // ===== ГЕТТЕРЫ =====
    const GPoint &GetPosition() const                   { return m_position; }
    const GVector &GetVelocity() const                  { return m_velocity; }
    const GVector &GetHeading() const                   { return m_heading; }
    double GetSpeedFraction() const                     { return m_userSpeedFraction; }
    double GetSpeed() const                             { return m_velocity.length(); }
    double GetMaxSpeed() const                          { return m_maxShipSpeed; }
    double GetMaxVelocity() const                       { return m_maxShipSpeed; }
    double GetMass() const                              { return m_mass; }
    double GetAgility() const                           { return m_agility; }
    double GetInertia() const                           { return m_shipInertia; }
    double GetRadius() const                            { return m_radius; }
    double GetAlignTime() const                         { return m_alignTime; }
    double GetAccelTime() const                         { return m_shipMaxAccelTime; }
    double GetWarpTime() const                          { return m_timeToEnterWarp; }
    double GetWarpDropSpeed() const                     { return m_speedToLeaveWarp; }
    double GetCapNeed() const                           { return m_warpCapacitorNeed; }
    double GetCallTime() const                          { return m_callTime; }
    uint32 GetStateStamp() const                        { return m_stateStamp; }
    uint8 GetState() const                              { return m_ballMode; }
    uint32 GetTargetID() const                          { return m_targetEntity.first; }
    SystemEntity* GetTargetEntity() const               { return m_targetEntity.second; }
    GPoint GetTargetPoint() const                       { return m_targetPoint; }
    double GetFollowDistance() const                    { return m_targetDistance; }
    int32 GetDistance() const                           { return static_cast<int32>(m_stopDistance); }
    double GetWarpSpeed() const                         { return m_shipWarpSpeed * 10.0; } // убираем округление до 0,1 а.е.
    double GetRadTic() const                            { return 0.0; }
    
    // ===== УГЛЫ ДЛЯ КЛИЕНТА =====
    double GetYaw() const                               { return atan2(m_heading.x, m_heading.z); }
    double GetPitch() const                             { return -asin(std::min(1.0, std::max(-1.0, m_heading.y))); }
    double GetRoll() const                              { return m_roll; }

    // ===== СОСТОЯНИЯ =====
    bool IsMoving() const                               { return m_velocity.length() > MIN_VELOCITY; }
    bool IsGoto() const                                 { return (m_ballMode == Destiny::Ball::Mode::GOTO); }
    bool IsStopped() const                              { return (m_ballMode == Destiny::Ball::Mode::STOP); }
    bool IsOrbiting() const                             { return (m_ballMode == Destiny::Ball::Mode::ORBIT); }
    bool IsFollowing() const                            { return (m_ballMode == Destiny::Ball::Mode::FOLLOW); }
    bool IsWarping() const                              { return (m_warpState != nullptr); }
    bool IsCloaked() const                              { return m_cloaked; }
    bool IsTurning() const                              { return false; }
    bool IsTractored() const                            { return m_tractored; }
    bool IsFrozen() const                               { return m_frozen; }
    bool IsAligned(GPoint &targetPoint);

    // ===== КОМАНДЫ =====
    void Stop();
    void Halt();
    void Eject();
    void SetCloak(bool set=false)                       { m_cloaked = set; }
    void SetFrozen(bool set=false)                      { m_frozen = set; }
    void SetPosition(const GPoint& pt, bool update=false);
    void SetRadius(double radius, bool update = false);
    void SetMaxVelocity(double maxVelocity);
    void SetCallTime(double set=0)                      { m_callTime = set; }
    void SetSpeedFraction(double fraction=1.0, bool startMovement=false);

    void Orbit(SystemEntity* pSE, uint32 distance=0);
    void Follow(SystemEntity* pSE, uint32 distance);
    void AlignTo(SystemEntity* pSE);
    void GotoPoint(const GPoint &point);
    void GotoDirection(const GPoint &direction);
    void WarpTo(const GPoint& where, int32 distance = 0, bool autoPilot = false, SystemEntity* pSE = nullptr);

    void WebbedMe(InventoryItemRef modRef, bool apply=false);
    void SpeedBoost(bool deactivate=false);
    void UpdateShipVariables();

    void TractorBeamStop();
    void TractorBeamStart(SystemEntity* pShipSE, EvilNumber speed);

    void Jump(bool showCloak=true);
    void Cloak();
    void UnCloak();

    void EntityRemoved(SystemEntity* pSE);
    void MakeMissile(Missile* missile);
    void Undock(GPoint dir);
    void SetUndockSpeed();
    void DockingAccepted();
    void UpdateNewShip(const ShipItemRef newShipRef);
    void UpdateOldShip(ShipSE* pShipSE);

    PyResult AttemptDockOperation();
    bool AbortIfLoginWarping(bool showMsg);

    // ===== ОТПРАВКА ПАКЕТОВ =====
    void SendSetState() const;
    void SendJumpOut(uint32 gateID) const;
    void SendJumpOutWormhole(uint32 wormholeID) const;
    void SendGateActivity(uint32 gateID) const;
    void SendWormholeActivity(uint32 wormholeID) const;
    void SendJumpInEffect(std::string JumpEffect) const;
    void SendJumpOutEffect(std::string JumpEffect, uint32 shipID) const;
    void SendTerminalExplosion(uint32 shipID, uint32 bubbleID, bool isGlobal=false) const;
    void SendBallInteractive(const ShipItemRef shipRef, bool set = false) const;
    void SendJettisonPacket() const;
    void SendAnchorDrop() const;
    void SendAnchorLift() const;
    void SendCloakFx(bool apply=false, bool module=false) const;
    void SendSpecialEffect10(uint32 entityID, uint32 targetID, std::string guid, bool isOffensive, bool start, bool isActive) const;
    void SendSpecialEffect(uint32 entityID, uint32 moduleID, uint32 moduleTypeID, uint32 targetID, 
                          uint32 chargeTypeID, std::string guid, bool isOffensive, bool start,
                          bool isActive, int32 duration, uint32 repeat, int32 graphicInfo = 0) const;

    // ===== ЗАГЛУШКИ ДЛЯ ОБРАТНОЙ СОВМЕСТИМОСТИ =====
    void MoveObject() { UpdatePhysics(GetDeltaTime()); }
    void Orbit();
    void Follow();
    void Turn() { /* Устарело */ }
    bool IsTurn() { return false; }
    void ClearTurn() { /* Устарело */ }
    void CalculateYawPitchRoll(bool snap = false) { 
        CalculateHeading(); 
        CalculateRoll(0.016); 
    }
    void Modulo2pi(double& a, double& b) { /* Устарело */ }
    void BeginMovement();

protected:
    void ProcessState();
    bool IsTargetInvalid();

    // ===== НОВАЯ ФИЗИКА (ДЛЯ КОРАБЛЕЙ) =====
    void UpdatePhysics(double dt);
    void ApplyBraking(double dt);
    void CalculateHeading();
    void CalculateRoll(double dt);
    void UpdateOrbit(double dt);
    void SendPhysicsUpdate();
    double GetDeltaTime();

    // ===== УПРОЩЁННАЯ ФИЗИКА (ДЛЯ NPC И ДРОНОВ) =====
    void MoveObjectNPC();
    void ApplyBrakingNPC(double dt);
    void UpdateOrbitNPC(double dt);
    void SetSpeedFractionNPC(double fraction);

    void ClearOrbit();
    GPoint ComputePosition(double curRad);

private:
    // ===== ОСНОВНЫЕ КОМПОНЕНТЫ =====
    SystemEntity* const mySE;
    SystemBubble* m_targBubble;

    // ===== РЕЖИМЫ И СОСТОЯНИЯ =====
    uint8 m_ballMode;
    int16 m_orbiting;
    uint32 m_stateStamp;
    bool m_stop;
    bool m_cloaked;
    bool m_tractored;
    bool m_tractorPause;
    bool m_frozen;
    bool m_changeDelay;
    bool m_ticAlign;
    bool m_hasSentShipUpdates;
    bool m_bump;

    // ===== ПОЗИЦИЯ И ДВИЖЕНИЕ =====
    GPoint m_position;
    GVector m_velocity;
    GVector m_oldVelocity;
    GVector m_heading;
    GVector m_targetVelocity;
    GVector m_oldHeading;

    // ===== ДЛЯ СОВМЕСТИМОСТИ =====
    GVector& m_shipHeading;
    GVector m_targetHeading;
    double m_yaw;
    double m_pitch;
    double m_roll;
    double m_rollSpeed;

    // ===== ТОЧКИ И ЦЕЛИ =====
    GPoint m_targetPoint;
    std::pair<uint32, SystemEntity*> m_targetEntity;

    // ===== ПАРАМЕТРЫ ФИЗИКИ =====
    double m_mass;
    double m_massMKg;
    double m_radius;
    double m_maxShipSpeed;
    double m_maxSpeed;
    double m_shipInertia;
    double m_shipAgility;
    double m_agility;
    double m_physicsFriction;
    double m_thrustPower;
    double m_stoppingDistance;
    double m_userSpeedFraction;
    double m_activeSpeedFraction;

    // ===== ПАРАМЕТРЫ ОРБИТЫ =====
    double m_orbitAngle;
    double m_orbitRadius;
    double m_orbitalSpeedRatio;
    double m_angularVelocity;
    GPoint m_orbitCenter;

    // ===== ПАРАМЕТРЫ ВАРПА =====
    double m_shipWarpSpeed;
    double m_timeToEnterWarp;
    double m_speedToLeaveWarp;
    double m_alignTime;
    double m_shipMaxAccelTime;
    double m_shipAccelTime;
    uint8 m_warpAccelTime;
    uint8 m_warpDecelTime;
    double m_warpCapacitorNeed;
    double m_capNeeded;

    // ===== РАССТОЯНИЯ =====
    double m_targetDistance;
    double m_followDistance;
    double m_stopDistance;
    double m_moveTime;
    double m_callTime;

    // ===== УСТАРЕВШИЕ ПЕРЕМЕННЫЕ =====
    double m_prevSpeed;
    double m_timeFraction;
    double m_prevSpeedFraction;
    double m_maxOrbitSpeedFraction;
    double m_degPerTic;
    double m_radians;
    double m_turnFraction;
    double m_orbitTime;
    double m_orbitRadTic;
    double m_massModifier;
    double m_timeFactor;
    double m_yawSpeed;
    double m_pitchSpeed;
    double m_oldRollSpeed;
    double m_yawDelta;
    double m_speedChangeTime;
    double m_speedAtChange;
    double m_targetSpeed;
    double m_timeSinceChange;
    bool m_accel;
    bool m_decel;
    bool m_turning;
    uint8 m_turnTic;

    // ===== ПАКЕТЫ =====
    PyTuple* mvPacket;

    // ===== ВАРП =====
    class WarpState {
    public:
        WarpState(
            uint32 start_time_,
            double total_distance_,
            double warp_speed_,
            double accel_dist_,
            double cruise_dist_,
            double decel_dist_,
            double warp_time_,
            bool accel_,
            bool cruise_,
            bool decel_,
            const GVector &warp_vector_)
        : start_time(start_time_),
          total_distance(total_distance_),
          warpSpeed(warp_speed_),
          accelDist(accel_dist_),
          cruiseDist(cruise_dist_),
          decelDist(decel_dist_),
          warpTime(warp_time_),
          accel(accel_),
          cruise(cruise_),
          decel(decel_),
          warp_vector(warp_vector_)
        {}
        uint32 start_time;
        double total_distance;
        double warpSpeed;
        double accelDist;
        double cruiseDist;
        double decelDist;
        double warpTime;
        bool accel;
        bool cruise;
        bool decel;
        GVector warp_vector;
    };
    WarpState* m_warpState;
    Timer m_warpTimer;

    // ===== МЕТОДЫ =====
    void InitWarp();
    void WarpAccel(uint16 sec_into_warp);
    void WarpCruise(uint16 sec_into_warp);
    void WarpDecel(uint16 sec_into_warp);
    void WarpStop(double currentShipSpeed);
    void WarpUpdate(double currentShipSpeed);

    void CheckBump();
    void Bump(SystemEntity* who);
    void Bounce(GVector direction, double speed);

    void SendMovementPacket();

    // ===== ВРЕМЯ ДЛЯ DELTA TIME =====
    std::chrono::steady_clock::time_point m_lastDeltaTime;
};

#endif // __DESTINYMANAGER_H_INCL__


/*#Embedded file name: c:/depot/games/branches/release/EVE-TRANQUILITY/eve/client/script/environment/effects/Repository.py
 * import effects
 * FX_MERGE_NONE = 0
 * FX_MERGE_GUID = 2
 * FX_MERGE_SHIP = 4
 * FX_MERGE_MODULE = 8
 * FX_MERGE_TARGET = 16
 * FX_TF_NONE = 0
 * FX_TF_POSITION_BALL = 2
 * FX_TF_POSITION_TARGET = 4
 * FX_TF_ROTATION_BALL = 8
 * FX_TF_SCALE_SYMMETRIC = 16
 * FX_TF_SCALE_BOUNDING = 32
 * FX_TF_SCALE_RADIUS = 64
 * definitions = {'effects.AnchorDrop': (effects.AnchorDrop,
 *                        FX_TF_NONE,
 *                        FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                        None,
 *                        1,
 *                        10000),
 * 'effects.AnchorLift': (effects.AnchorLift,
 *                        FX_TF_NONE,
 *                        FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                        None,
 *                        1,
 *                        10000),
 * 'effects.ArmorHardening': (effects.ShipRenderEffect,
 *                            FX_TF_NONE,
 *                            FX_MERGE_SHIP | FX_MERGE_GUID,
 *                            'res:/dx9/Model/Effect/ArmorHardening.red',
 *                            1,
 *                            10000),
 * 'effects.ArmorRepair': (effects.ShipRenderEffect,
 *                         FX_TF_NONE,
 *                         FX_MERGE_SHIP | FX_MERGE_GUID,
 *                         'res:/dx9/Model/Effect/ArmorRepair.red',
 *                         1,
 *                         10000),
 * 'effects.Barrage': (effects.StandardWeapon,
 *                     FX_TF_NONE,
 *                     FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                     None,
 *                     1,
 *                     10000),
 * 'effects.CargoScan': (effects.StretchEffect,
 *                       FX_TF_NONE,
 *                       FX_MERGE_SHIP | FX_MERGE_GUID,
 *                       'res:/Model/Effect3/CargoScan.red',
 *                       1,
 *                       10000),
 * 'effects.Cloak': (effects.Cloak,
 *                   FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                   FX_MERGE_SHIP | FX_MERGE_GUID,
 *                   'res:/Model/Effect3/Cloaking.red',
 *                   1,
 *                   6000),
 * 'effects.CloakNoAmim': (effects.CloakNoAmim,
 *                         FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                         FX_MERGE_SHIP | FX_MERGE_GUID,
 *                         'res:/Model/Effect3/Cloaking.red',
 *                         1,
 *                         6000),
 * 'effects.CloakRegardless': (effects.CloakRegardless,
 *                             FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                             FX_MERGE_SHIP | FX_MERGE_GUID,
 *                             'res:/Model/Effect3/Cloaking.red',
 *                             1,
 *                             6000),
 * 'effects.Cloaking': (effects.Cloaking,
 *                      FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                      FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                      None,
 *                      1,
 *                      10000),
 * 'effects.CloudMining': (effects.CloudMining,
 *                         FX_TF_NONE,
 *                         FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                         None,
 *                         1,
 *                         10000),
 * 'effects.ECMBurst': (effects.ShipEffect,
 *                      FX_TF_POSITION_BALL,
 *                      FX_MERGE_SHIP | FX_MERGE_GUID,
 *                      'res:/Model/Effect3/EcmBurst.red',
 *                      1,
 *                      10000),
 * 'effects.EMPWave': (effects.EMPWave,
 *                     FX_TF_NONE,
 *                     FX_MERGE_SHIP | FX_MERGE_MODULE,
 *                     None,
 *                     1,
 *                     10000),
 * 'effects.ElectronicAttributeModifyActivate': (effects.ShipEffect,
 *                                               FX_TF_SCALE_RADIUS | FX_TF_POSITION_BALL,
 *                                               FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                                               'res:/Model/Effect3/ECM.red',
 *                                               1,
 *                                               10000),
 * 'effects.ElectronicAttributeModifyTarget': (effects.StretchEffect,
 *                                             FX_TF_NONE,
 *                                             FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                                             'res:/Model/Effect3/SensorBoost.red',
 *                                             1,
 *                                             10000),
 * 'effects.EnergyDestabilization': (effects.StretchEffect,
 *                                   FX_TF_NONE,
 *                                   FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                                   'res:/Model/Effect3/EnergyDestabilization.red',
 *                                   1,
 *                                   10000),
 * 'effects.EnergyTransfer': (effects.StretchEffect,
 *                            FX_TF_NONE,
 *                            FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                            'res:/Model/Effect3/EnergyTransfer.red',
 *                            1,
 *                            10000),
 * 'effects.EnergyVampire': (effects.StretchEffect,
 *                           FX_TF_NONE,
 *                           FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                           'res:/Model/Effect3/EnergyVampire.red',
 *                           1,
 *                           10000),
 * 'effects.GateActivity': (effects.GateActivity,
 *                          FX_TF_NONE,
 *                          FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                          None,
 *                          1,
 *                          10000),
 * 'effects.HybridFired': (effects.StandardWeapon,
 *                         FX_TF_NONE,
 *                         FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                         None,
 *                         1,
 *                         10000),
 * 'effects.Jettison': (effects.ShipEffect,
 *                      FX_TF_POSITION_BALL,
 *                      FX_MERGE_SHIP | FX_MERGE_GUID,
 *                      'res:/Model/Effect3/Jettison.red',
 *                      1,
 *                      10000),
 * 'effects.JumpDriveIn': (effects.JumpDriveIn,
 *                         FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                         FX_MERGE_SHIP | FX_MERGE_GUID,
 *                         'res:\\Model\\Effect3\\JumpDrive_in.red',
 *                         1,
 *                         10000),
 * 'effects.JumpDriveInBO': (effects.JumpDriveInBO,
 *                           FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                           FX_MERGE_SHIP | FX_MERGE_GUID,
 *                           'res:\\Model\\Effect3\\JumpDriveBO_in.red',
 *                           1,
 *                           10000),
 * 'effects.JumpDriveOut': (effects.JumpDriveOut,
 *                          FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL,
 *                          FX_MERGE_SHIP | FX_MERGE_GUID,
 *                          'res:\\Model\\Effect3\\JumpDrive_out.red',
 *                          1,
 *                          10000),
 * 'effects.JumpDriveOutBO': (effects.JumpDriveOutBO,
 *                            FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL,
 *                            FX_MERGE_SHIP | FX_MERGE_GUID,
 *                            'res:\\Model\\Effect3\\JumpDriveBO_out.red',
 *                            1,
 *                            10000),
 * 'effects.JumpIn': (effects.JumpIn,
 *                    FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                    FX_MERGE_SHIP | FX_MERGE_GUID,
 *                    'res:/Model/Effect3/warpEntry.red',
 *                    1,
 *                    10000),
 * 'effects.JumpOut': (effects.JumpOut,
 *                     FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                     FX_MERGE_SHIP | FX_MERGE_GUID,
 *                     'res:\\Model\\Effect3\\Jump_out.red',
 *                     1,
 *                     10000),
 * 'effects.JumpOutWormhole': (effects.JumpOutWormhole,
 *                             FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                             FX_MERGE_SHIP | FX_MERGE_GUID,
 *                             'res:\\Model\\Effect3\\WormJump.red',
 *                             1,
 *                             10000),
 * 'effects.JumpPortal': (effects.JumpPortal,
 *                        FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                        FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                        'res:/Model/Effect3/JumpPortal.red',
 *                        1,
 *                        10000),
 * 'effects.JumpPortalBO': (effects.JumpPortalBO,
 *                          FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                          FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                          'res:/Model/Effect3/JumpPortal_BO.red',
 *                          1,
 *                          10000),
 * 'effects.Laser': (effects.StandardWeapon,
 *                   FX_TF_NONE,
 *                   FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                   None,
 *                   1,
 *                   10000),
 * 'effects.Mining': (effects.StandardWeapon,
 *                    FX_TF_NONE,
 *                    FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                    None,
 *                    1,
 *                    10000),
 * 'effects.MissileDeployment': (effects.ShipEffect,
 *                               FX_TF_POSITION_BALL,
 *                               FX_MERGE_SHIP | FX_MERGE_GUID,
 *                               'res:/Model/Effect3/missileLaunch.red',
 *                               1,
 *                               12000),
 * 'effects.ModifyShieldResonance': (effects.ShipRenderEffect,
 *                                   FX_TF_NONE,
 *                                   FX_MERGE_SHIP | FX_MERGE_GUID,
 *                                   'res:/dx9/Model/Effect/ShieldHardening.red',
 *                                   1,
 *                                   10000),
 * 'effects.ModifyTargetSpeed': (effects.ShipEffect,
 *                               FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_TARGET,
 *                               FX_MERGE_TARGET | FX_MERGE_GUID,
 *                               'res:/Model/Effect3/StasisWeb.red',
 *                               1,
 *                               10000),
 * 'effects.ProjectileFired': (effects.StandardWeapon,
 *                             FX_TF_NONE,
 *                             FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                             None,
 *                             1,
 *                             10000),
 * 'effects.ProjectileFiredForEntities': (effects.StandardWeapon,
 *                                        FX_TF_NONE,
 *                                        FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                                        None,
 *                                        1,
 *                                        10000),
 * 'effects.RemoteArmourRepair': (effects.StretchEffect,
 *                                FX_TF_NONE,
 *                                FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                                'res:/Model/Effect3/RemoteArmorRepair.red',
 *                                1,
 *                                10000),
 * 'effects.RemoteECM': (effects.StretchEffect,
 *                       FX_TF_NONE,
 *                       FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                       'res:/Model/Effect3/RemoteECM.red',
 *                       1,
 *                       10000),
 * 'effects.Salvaging': (effects.StandardWeapon,
 *                       FX_TF_NONE,
 *                       FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                       None,
 *                       1,
 *                       10000),
 * 'effects.ScanStrengthBonusActivate': (effects.ShipEffect,
 *                                       FX_TF_SCALE_RADIUS | FX_TF_POSITION_BALL,
 *                                       FX_MERGE_SHIP | FX_MERGE_GUID,
 *                                       'res:/Model/Effect3/ECCM.red',
 *                                       1,
 *                                       10000),
 * 'effects.ScanStrengthBonusTarget': (effects.ShipEffect,
 *                                     FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL,
 *                                     FX_MERGE_SHIP | FX_MERGE_GUID,
 *                                     'res:/Model/Effect3/ECCM.red',
 *                                     1,
 *                                     10000),
 * 'effects.ShieldBoosting': (effects.ShipRenderEffect,
 *                            FX_TF_NONE,
 *                            FX_MERGE_SHIP | FX_MERGE_GUID,
 *                            'res:/dx9/Model/Effect/ShieldBoosting.red',
 *                            0,
 *                            10000),
 * 'effects.ShieldTransfer': (effects.StretchEffect,
 *                            FX_TF_NONE,
 *                            FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                            'res:/Model/Effect3/ShieldTransfer.red',
 *                            1,
 *                            10000),
 * 'effects.ShipScan': (effects.StretchEffect,
 *                      FX_TF_NONE,
 *                      FX_MERGE_SHIP | FX_MERGE_GUID,
 *                      'res:/Model/Effect3/ShipScan.red',
 *                      1,
 *                      10000),
 * 'effects.SiegeMode': (effects.SiegeMode,
 *                       FX_TF_NONE,
 *                       FX_MERGE_SHIP,
 *                       None,
 *                       1,
 *                       10000),
 * 'effects.SpeedBoost': (effects.GenericEffect,
 *                        FX_TF_NONE,
 *                        FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                        None,
 *                        1,
 *                        10000),
 * 'effects.StructureOffline': (effects.StructureOffline,
 *                              FX_TF_NONE,
 *                              FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                              None,
 *                              1,
 *                              10000),
 * 'effects.StructureOnline': (effects.StructureOnline,
 *                             FX_TF_NONE,
 *                             FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                             None,
 *                             1,
 *                             10000),
 * 'effects.StructureOnlined': (effects.StructureOnlined,
 *                              FX_TF_NONE,
 *                              FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                              None,
 *                              1,
 *                              10000),
 * 'effects.StructureRepair': (effects.ShipRenderEffect,
 *                             FX_TF_NONE,
 *                             FX_MERGE_SHIP | FX_MERGE_GUID,
 *                             'res:/dx9/Model/Effect/HullRepair.red',
 *                             1,
 *                             10000),
 * 'effects.SuperWeaponAmarr': (effects.StretchEffect,
 *                              FX_TF_NONE,
 *                              FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                              'res:/Model/Effect3/Superweapon/A_DoomsDay.red',
 *                              False,
 *                              10000),
 * 'effects.SuperWeaponCaldari': (effects.StretchEffect,
 *                                FX_TF_NONE,
 *                                FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                                'res:/Model/Effect3/Superweapon/C_DoomsDay.red',
 *                                False,
 *                                10000),
 * 'effects.SuperWeaponGallente': (effects.StretchEffect,
 *                                 FX_TF_NONE,
 *                                 FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                                 'res:/Model/Effect3/Superweapon/G_DoomsDay.red',
 *                                 False,
 *                                 10000),
 * 'effects.SuperWeaponMinmatar': (effects.StretchEffect,
 *                                 FX_TF_NONE,
 *                                 FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                                 'res:/Model/Effect3/Superweapon/M_DoomsDay.red',
 *                                 False,
 *                                 10000),
 * 'effects.SurveyScan': (effects.ShipEffect,
 *                        FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL,
 *                        FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                        'res:/Model/Effect3/SurveyScan.red',
 *                        1,
 *                        10000),
 * 'effects.TargetPaint': (effects.StretchEffect,
 *                         FX_TF_NONE,
 *                         FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                         'res:/Model/Effect3/TargetPaint.red',
 *                         1,
 *                         10000),
 * 'effects.TargetScan': (effects.StretchEffect,
 *                        FX_TF_NONE,
 *                        FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                        'res:/Model/Effect3/SurveyScan2.red',
 *                        1,
 *                        10000),
 * 'effects.TorpedoDeployment': (effects.GenericEffect,
 *                               FX_TF_NONE,
 *                               FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                               None,
 *                               1,
 *                               10000),
 * 'effects.TractorBeam': (effects.StandardWeapon,
 *                         FX_TF_NONE,
 *                         FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                         None,
 *                         1,
 *                         10000),
 * 'effects.TriageMode': (effects.ShipRenderEffect,
 *                        FX_TF_NONE,
 *                        FX_MERGE_SHIP | FX_MERGE_GUID,
 *                        'res:/dx9/Model/Effect/TriageMode.red',
 *                        0,
 *                        10000),
 * 'effects.TurretWeaponRangeTrackingSpeedMultiplyActivate': (effects.ShipEffect,
 *                                                            FX_TF_POSITION_BALL,
 *                                                            FX_MERGE_SHIP | FX_MERGE_GUID,
 *                                                            'res:/Model/Effect3/TrackingBoost.red',
 *                                                            1,
 *                                                            10000),
 * 'effects.TurretWeaponRangeTrackingSpeedMultiplyTarget': (effects.StretchEffect,
 *                                                          FX_TF_NONE,
 *                                                          FX_MERGE_SHIP | FX_MERGE_GUID,
 *                                                          'res:/Model/Effect3/TrackingBoostTarget.red',
 *                                                          1,
 *                                                          10000),
 * 'effects.Uncloak': (effects.Uncloak,
 *                     FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                     FX_MERGE_SHIP | FX_MERGE_GUID,
 *                     'res:/Model/Effect3/Cloaking.red',
 *                     1,
 *                     7500),
 * 'effects.WarpDisruptFieldGenerating': (effects.WarpDisruptFieldGenerating,
 *                                        FX_TF_POSITION_BALL,
 *                                        FX_MERGE_SHIP | FX_MERGE_GUID,
 *                                        'res:/Model/effect3/WarpDisruptorBubble.red',
 *                                        0,
 *                                        10000),
 * 'effects.WarpGateEffect': (effects.WarpGateEffect,
 *                            FX_TF_NONE,
 *                            FX_MERGE_SHIP | FX_MERGE_GUID,
 *                            None,
 *                            0,
 *                            10000),
 * 'effects.WarpScramble': (effects.StretchEffect,
 *                          FX_TF_NONE,
 *                          FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                          'res:/Model/Effect3/WarpScrambler.red',
 *                          1,
 *                          10000),
 * 'effects.Warping': (effects.Warping,
 *                     FX_TF_NONE,
 *                     FX_MERGE_SHIP | FX_MERGE_GUID,
 *                     'res:/Model/Effect3/warpTunnel2.red',
 *                     False,
 *                     1200000),
 * 'effects.WormholeActivity': (effects.WormholeActivity,
 *                              FX_TF_NONE,
 *                              FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                              None,
 *                              1,
 *                              10000)}
 * if const.useNewMissileSystem_removeThisSoon:
 *    definitions['effects.MissileDeployment'] = (effects.MissileDeployment,
 *     0,
 *     0,
 *     None,
 *     1,
 *     12000)
 *
 * def GetGuids():
 *    return definitions.keys()
 *
 *
 * def GetClassification(guid):
 *    return definitions.get(guid, None)
 *
 *
 * exports = {'effects.GetClassification': GetClassification,
 * 'effects.GetGuids': GetGuids,
 * 'effects.FX_MERGE_NONE': FX_MERGE_NONE,
 * 'effects.FX_MERGE_GUID': FX_MERGE_GUID,
 * 'effects.FX_MERGE_SHIP': FX_MERGE_SHIP,
 * 'effects.FX_MERGE_MODULE': FX_MERGE_MODULE,
 * 'effects.FX_MERGE_TARGET': FX_MERGE_TARGET,
 * 'effects.FX_TF_NONE': FX_TF_NONE,
 * 'effects.FX_TF_POSITION_BALL': FX_TF_POSITION_BALL,
 * 'effects.FX_TF_POSITION_TARGET': FX_TF_POSITION_TARGET,
 * 'effects.FX_TF_ROTATION_BALL': FX_TF_ROTATION_BALL,
 * 'effects.FX_TF_SCALE_SYMMETRIC': FX_TF_SCALE_SYMMETRIC,
 * 'effects.FX_TF_SCALE_BOUNDING': FX_TF_SCALE_BOUNDING,
 * 'effects.FX_TF_SCALE_RADIUS': FX_TF_SCALE_RADIUS}
 */