#ifndef __DESTINYMANAGER_H_INCL__
#define __DESTINYMANAGER_H_INCL__

#include "eve-compat.h"

#include "services/Callable.h"
#include "destiny/DestinyStructs.h"
#include "inventory/ItemRef.h"

// ===== КОНСТАНТЫ ФИЗИКИ ИЗ ОФИЦИАЛЬНОГО КОДА EVE =====
#define PHYSICS_FRICTION 0.5
#define PHYSICS_TIME_FACTOR 1.0
#define MAX_IMPACT_ANGLE 0.25f
#define MIN_ANGULAR_VELOCITY 0.01f
#define _HALFPI_  1.57079632679489655
#define _PI_      3.1415926535897931
#define _2PI_     6.2831853071795862
// ===== КОНЕЦ КОНСТАНТ =====

// Используем существующие определения из DestinyStructs.h
// Там Mode определен как namespace с константами
namespace Destiny {
    namespace Ball {
        // Используем константы из DestinyStructs.h: 
        // Mode::STOP, Mode::GOTO, Mode::FOLLOW, Mode::ORBIT, Mode::WARP, etc.
        // Mode::MISSILE, Mode::MUSHROOM, Mode::BOID, Mode::TROLL, Mode::MINIBALL, Mode::FIELD, Mode::FORMATION, Mode::RIGID
        
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

    void SendSingleDestinyEvent(PyTuple** ev, bool self_only=false) const;
    void SendSingleDestinyUpdate(PyTuple** up, bool self_only=false) const;
    void SendDestinyUpdate(std::vector<PyTuple*> &updates, bool self_only=false) const;
    void SendDestinyUpdate(std::vector<PyTuple*> &updates, std::vector<PyTuple*> &events, bool self_only=false) const;

    const GPoint &GetPosition() const                   { return m_position; }
    const GVector &GetVelocity() const                  { return m_velocity; }
    double GetSpeedFraction()                           { return m_timeFraction; }
    double GetSpeed()                                   { return (m_maxShipSpeed * m_timeFraction); }

    uint8 GetState()                                    { return m_ballMode; }

    void EntityRemoved(SystemEntity* pSE);

    void WebbedMe(InventoryItemRef modRef, bool apply=false);
    void SpeedBoost(bool deactivate=false);
    void SetPosition(const GPoint& pt, bool update=false);
    void SetRadius(double radius, bool update = false);
    void SetMaxVelocity(double maxVelocity);
    void UpdateShipVariables();

    void Stop();
    void Halt();
    void Eject();
    void SetCloak(bool set=false)                       { m_cloaked = set; }

    void TractorBeamStop();
    void TractorBeamStart(SystemEntity* pShipSE, EvilNumber speed);

    void Orbit(SystemEntity* pSE, uint32 distance=0);
    void Follow( SystemEntity* pSE, uint32 distance );
    void AlignTo(SystemEntity* pSE);
    void GotoPoint(const GPoint &point);
    void GotoDirection(const GPoint &direction);
    void SetSpeedFraction(double fraction=1.0, bool startMovement=false);

    void WarpTo(const GPoint& where, int32 distance = 0, bool autoPilot = false, SystemEntity* pSE = nullptr);

    bool IsMoving()                                     { return (m_timeFraction > 0); }

    bool IsAligned(GPoint &targetPoint);
    bool IsGoto()                                       { return (m_ballMode == Destiny::Ball::Mode::GOTO); }
    bool IsStopped()                                    { return (m_ballMode == Destiny::Ball::Mode::STOP); }
    bool IsOrbiting()                                   { return (m_ballMode == Destiny::Ball::Mode::ORBIT); }
    bool IsFollowing()                                  { return (m_ballMode == Destiny::Ball::Mode::FOLLOW); }
    bool IsWarping()                                    { return (m_warpState ? true : false); }
    bool IsCloaked()                                    { return m_cloaked; }
    bool IsTurning()                                    { return m_turning; }
    bool IsTractored()                                  { return m_tractored; }

    void Jump(bool showCloak=true);
    void Cloak();
    void UnCloak();

    PyResult AttemptDockOperation();
    void Undock(GPoint dir);
    void SetUndockSpeed();
    void DockingAccepted();
    void SendSetState() const;
    void SendJumpOut(uint32 gateID) const;
    void SendJumpOutWormhole(uint32 wormholeID) const;
    void SendGateActivity(uint32 gateID) const;
    void SendWormholeActivity(uint32 wormholeID) const;
    void SendJumpInEffect(std::string JumpEffect) const;
    void SendJumpOutEffect(std::string JumpEffect, uint32 shipID) const;
    void SendTerminalExplosion(uint32 shipID, uint32 bubbleID, bool isGlobal=false) const;
    void SendBallInteractive(const ShipItemRef shipRef, bool set = false) const;
    void UpdateNewShip(const ShipItemRef newShipRef);
    void UpdateOldShip(ShipSE* pShipSE);
    void SendJettisonPacket() const;
    void SendAnchorDrop() const;
    void SendAnchorLift() const;
    void SendCloakFx(bool apply=false, bool module=false) const;
    void SendSpecialEffect10(uint32 entityID, uint32 targetID, std::string guid, bool isOffensive, bool start, bool isActive) const;
    void SendSpecialEffect(uint32 entityID, uint32 moduleID, uint32 moduleTypeID, uint32 targetID, uint32 chargeTypeID, std::string guid, bool isOffensive, bool start, bool isActive, int32 duration, uint32 repeat, int32 graphicInfo = 0) const;

    int32 GetDistance()                                 { return static_cast<int32>(m_stopDistance); }
    int32 GetWarpSpeed()                                { return static_cast<int32>(m_shipWarpSpeed * 10); }
    uint32 GetTargetID()                                { return m_targetEntity.first; }
    SystemEntity* GetTargetEntity()                     { return m_targetEntity.second; }
    GPoint GetTargetPoint()                             { return m_targetPoint; }
    double GetMaxVelocity()                             { return m_maxShipSpeed; }
    double GetFollowDistance()                          { return m_targetDistance; }
    double GetMass()                                    { return m_mass; }
    double GetAgility()                                 { return m_shipAgility; }
    double GetInertia()                                 { return m_shipInertia; }
    uint32 GetStateStamp()                              { return m_stateStamp; }
    GVector GetHeading()                                { return m_shipHeading; }

    double GetAlignTime()                               { return m_alignTime; }
    double GetAccelTime()                               { return m_shipMaxAccelTime; }
    double GetWarpTime()                                { return m_timeToEnterWarp; }
    double GetWarpDropSpeed()                           { return m_speedToLeaveWarp; }
    double GetRadius()                                  { return m_radius; }
    double GetCapNeed()                                 { return m_warpCapacitorNeed; }

    double GetRadTic()                                  { return m_orbitRadTic; }

    void SetCallTime(double set=0)                      { m_callTime = set; }
    double GetCallTime()                                { return m_callTime; }

    void MakeMissile(Missile* missile);

    bool IsFrozen()                                     { return m_frozen; }
    void SetFrozen(bool set=false)                      { m_frozen = set; }

    bool AbortIfLoginWarping(bool showMsg);

    // ===== НОВЫЕ МЕТОДЫ ДЛЯ ФИЗИКИ =====
    void CalculateYawPitchRoll(bool snap = false);
    void Modulo2pi(double& a, double& b);

    // ===== НОВЫЕ ГЕТТЕРЫ ДЛЯ УГЛОВ =====
    double GetYaw() const                               { return m_yaw; }
    double GetPitch() const                             { return m_pitch; }
    double GetRoll() const                              { return m_roll; }

protected:
    void ProcessState();

    SystemEntity* const mySE;
    SystemBubble* m_targBubble;

    bool IsTargetInvalid();

    bool m_hasSentShipUpdates;

    uint8 m_warpAccelTime;
    uint8 m_warpDecelTime;

    double m_mass;
    double m_massMKg;
    double m_alignTime;
    double m_prevSpeed;
    double m_maxShipSpeed;
    double m_shipWarpSpeed;
    double m_timeToEnterWarp;
    double m_speedToLeaveWarp;

    double m_radius;
    double m_capNeeded;
    double m_warpCapacitorNeed;

    double m_shipAgility;
    double m_shipInertia;

    double m_maxSpeed;
    double m_degPerTic;
    double m_shipAccelTime;
    double m_shipMaxAccelTime;

    double m_radians;

    GPoint m_position;
    GVector m_velocity;

    bool m_stop;
    bool m_accel;
    bool m_decel;
    bool m_cloaked;
    bool m_turning;
    bool m_tractored;
    bool m_tractorPause;

    uint8 m_ballMode;

    double m_stopDistance;

    uint8 m_turnTic;
    int16 m_orbiting;
    uint32 m_stateStamp;

    double m_orbitTime;
    double m_orbitRadTic;
    double m_timeFraction;
    double m_turnFraction;
    double m_prevSpeedFraction;
    double m_userSpeedFraction;
    double m_activeSpeedFraction;
    double m_maxOrbitSpeedFraction;

    double m_followDistance;
    double m_targetDistance;
    double m_moveTime;
    double m_callTime;

    GPoint m_targetPoint;
    GVector m_shipHeading;
    GVector m_targetHeading;
    std::pair<uint32, SystemEntity*> m_targetEntity;

    void MoveObject();
    void Orbit();
    void Follow();
    void BeginMovement();
    void UpdateVelocity(bool isMoving=false);

private:
    bool m_frozen;
    bool m_changeDelay;

    bool m_ticAlign;
    void SendMovementPacket();
    PyTuple *mvPacket;

    bool m_bump;
    void CheckBump();
    void Bump(SystemEntity* who);
    void Bounce(GVector direction, double speed);

    bool IsTurn();
    void Turn();
    void ClearTurn();

    GPoint ComputePosition(double curRad);
    double m_inclination;
    double m_longAscNode;
    void ClearOrbit();

    Timer m_warpTimer;
    void InitWarp();
    void WarpAccel(uint16 sec_into_warp);
    void WarpCruise(uint16 sec_into_warp);
    void WarpDecel(uint16 sec_into_warp);
    void WarpStop(double currentShipSpeed);
    void WarpUpdate(double currentShipSpeed);

    // ===== НОВЫЕ ПЕРЕМЕННЫЕ ДЛЯ ЭКСПОНЕНЦИАЛЬНОЙ ФИЗИКИ =====
    double m_speedChangeTime;      // Время последнего изменения скорости
    double m_speedAtChange;        // Скорость в момент изменения (м/с)
    double m_targetSpeed;          // Целевая скорость (м/с)
    double m_timeSinceChange;      // Время с последнего изменения (сек)

    // ===== НОВЫЕ ПЕРЕМЕННЫЕ ДЛЯ YAW/PITCH/ROLL =====
    double m_yaw;                  // Текущий рысканье
    double m_pitch;                // Текущий тангаж
    double m_roll;                 // Текущий крен
    double m_oldYaw;               // Предыдущий рысканье
    double m_oldPitch;             // Предыдущий тангаж
    double m_oldRoll;              // Предыдущий крен
    double m_yawSpeed;             // Скорость рысканья
    double m_pitchSpeed;           // Скорость тангажа
    double m_rollSpeed;            // Скорость крена
    double m_oldRollSpeed;         // Предыдущая скорость крена
    double m_yawDelta;             // Изменение рысканья

    // ===== НОВЫЕ ПЕРЕМЕННЫЕ ДЛЯ МАССОВОГО МОДИФИКАТОРА =====
    double m_massModifier;         // Модификатор массы для поворота
    double m_physicsFriction;      // Коэффициент трения (аналог mPark->mFriction)
    double m_timeFactor;           // Фактор времени (обычно 1.0)

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
};

#endif // __DESTINYMANAGER_H_INCL__