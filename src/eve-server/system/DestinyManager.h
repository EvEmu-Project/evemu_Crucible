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
    Author:        Zhur, Aknor Jaden
    Updates:        Allan
*/

#ifndef __DESTINYMANAGER_H_INCL__
#define __DESTINYMANAGER_H_INCL__

#include "eve-compat.h"

#include "services/Callable.h"
#include "destiny/DestinyStructs.h"
#include "inventory/ItemRef.h"

//0=no orbit, >0=in orbit, 1=at distance 2=too close , 3=too far, 4=way too close, 5=way too far
namespace Destiny {
    namespace Ball {
        struct stateStamp {
            uint8 state;
            uint16 time;
        };
        struct timeStamp {
            uint8 mode;
            int64 time;
        };
        namespace Orbit {
            enum {
                None = 0,
                Orbiting = 1,
                Close = 2,
                Far = 3,
                TooClose = 4,
                TooFar = 5
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

// common variables to denote accpetable alignment deviations
static const float TURN_ALIGNMENT = 4.0f;
static const float WARP_ALIGNMENT = 6.0f;
static const uint16 BUMP_DISTANCE = 50;     //in meters.  < this = hit.

/*
namespace Destiny {
    namespace Warp {
        struct State {
            uint32 start_time;          //from sEntityList::GetStamp()
            double total_distance;      //in m
            double warpSpeed;           //in m/s
            double accelDist;           //in m
            double cruiseDist;          //in m
            double decelDist;           //in m
            float warpTime;             //in s
            bool accel;
            bool cruise;
            bool decel;
            GVector warp_vector;        //target direction based on ship's initial position
        };
    }
}
*/

//this object manages an entity's position and movement in a system.

class DestinyManager {
public:
    DestinyManager(SystemEntity* self);
    ~DestinyManager();

    void Process();

    void SendSingleDestinyEvent(PyTuple** ev, bool self_only=false) const;
    void SendSingleDestinyUpdate(PyTuple** up, bool self_only=false) const;
    void SendDestinyUpdate(std::vector<PyTuple*> &updates, bool self_only=false) const;
    void SendDestinyUpdate(std::vector<PyTuple*> &updates, std::vector<PyTuple*> &events, bool self_only=false) const;

    /* Informational query functions: */
    const GPoint &GetPosition() const                   { return m_position; }
    const GVector &GetVelocity() const                  { return m_velocity; }
    float GetSpeedFraction()                            { return m_timeFraction; }
    float GetSpeed()                                    { return (m_maxShipSpeed * m_timeFraction); }

    // this is only used by my bubble debug command
    uint8 GetState()                                    { return m_ballMode; }

    void EntityRemoved(SystemEntity* pSE);

    /* Configuration methods */
    void WebbedMe(InventoryItemRef modRef, bool apply=false);
    void SpeedBoost(bool deactivate=false);             // reset speed variables and bubblecast ship's AB/MWD modified speed (module activate/deactivate)
    void SetPosition(const GPoint& pt, bool update=false);
    void SetRadius(double radius, bool update = false);
    void SetMaxVelocity(float maxVelocity);
    void UpdateShipVariables();

    /* Global Actions */
    void Stop();
    void Halt();     // puts entity at 0 velocity
    void Eject();   // avoid numerous other redirect calls
    void SetCloak(bool set=false)                       { m_cloaked = set; }

    /* TractorBeam */
    void TractorBeamStop();
    void TractorBeamStart(SystemEntity* pShipSE, EvilNumber speed);

    /* Local Movement */
    void Orbit(SystemEntity* pSE, uint32 distance=0);
    void Follow( SystemEntity* pSE, uint32 distance );
    void AlignTo(SystemEntity* pSE);
    void GotoPoint(const GPoint &point);
    void GotoDirection(const GPoint &direction);
    void SetSpeedFraction(float fraction=1.0f, bool startMovement=false);

    /* Larger movement */
    void WarpTo(const GPoint& where, int32 distance = 0, bool autoPilot = false, SystemEntity* pSE = nullptr);

    /* Ship State Query functions */
    bool IsMoving()                                     { return (m_timeFraction > 0); }

    /* Movement checks */
    bool IsAligned(GPoint &targetPoint);
    bool IsGoto()                                       { return (m_ballMode == Destiny::Ball::Mode::GOTO); }
    bool IsStopped()                                    { return (m_ballMode == Destiny::Ball::Mode::STOP); }
    bool IsOrbiting()                                   { return (m_ballMode == Destiny::Ball::Mode::ORBIT); }
    bool IsFollowing()                                  { return (m_ballMode == Destiny::Ball::Mode::FOLLOW); }
    //bool IsJumping()                                  { return (m_ballMode == Destiny::Ball::Mode::STOP); }
    bool IsWarping()                                    { return (m_warpState ? true : false); }
    bool IsCloaked()                                    { return m_cloaked; }
    bool IsTurning()                                    { return m_turning; }
    bool IsTractored()                                  { return m_tractored; }

    //Destiny Update stuff:
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

    //  functions to return protected variables for SystemBubble exclusive WarpTo updates and other methods that need Destiny Variables
    int32 GetDistance()                                 { return m_stopDistance; }
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

    float GetAlignTime()                                { return m_alignTime; }
    float GetAccelTime()                                { return m_shipMaxAccelTime; }
    float GetWarpTime()                                 { return m_timeToEnterWarp; }
    float GetWarpDropSpeed()                            { return m_speedToLeaveWarp; }
    double GetRadius()                                  { return m_radius; }
    double GetCapNeed()                                 { return m_warpCapacitorNeed; }

    float GetRadTic()                                   { return m_orbitRadTic; }

    void SetCallTime(double set=0)                      { m_callTime = set; }
    double GetCallTime()                                { return m_callTime; }

    // set all movement vars for missile and add to system
    //  this is used by all entities (pc, npc, drone, sentry, pos, etc)
    void MakeMissile(Missile* missile);

    bool IsFrozen()                                     { return m_frozen; }
    void SetFrozen(bool set=false)                      { m_frozen = set; }

    // Prevents actions if the player is performing the login warp
    bool AbortIfLoginWarping(bool showMsg);

protected:
    void ProcessState();

    SystemEntity* const mySE;			//we do not own this.
    SystemBubble* m_targBubble;         //we do not own this.

    bool IsTargetInvalid();              //performs common target checks

    bool m_hasSentShipUpdates;

    //things dictated by our entity's configuration:
    uint8 m_warpAccelTime;              //in s      - calculated internally for warp stages
    uint8 m_warpDecelTime;              //in s      - calculated internally for warp stages

    float m_mass;                       //in kg
    float m_massMKg;                    //in mg     - Millionths of kg (mg)
    float m_alignTime;                  //in s      - align and enter warp are same (for our purposes)
    float m_prevSpeed;                  //in m/s    - used to calculate speed during decel
    float m_maxShipSpeed;               //in m/s
    float m_shipWarpSpeed;              //in au/s
    float m_timeToEnterWarp;            //in s
    float m_speedToLeaveWarp;           //in m/s    - this is set to 75% of m_maxShipSpeed

    double m_radius;                    //in m
    double m_capNeeded;                 //in GJ     - variable to drain cap during warp init
    double m_warpCapacitorNeed;         //in GJ     - capacitor charged needed to initiate warp
    // ship motion factors for complicated maths
    double m_shipAgility;               //in s/Mkg  - time-constant of movement for objects in eve physics (and 't' in Dr. SS's calculations)
                                        //          - characteristic of time that governs the rate of change in motion of an object
    double m_shipInertia;               //in s/Mkg  - reciprocal of drag constant in EvE
                                        //          - the drag coefficient is 1/I and in Mkg/s

    //derived from above params:
    float m_maxSpeed;                   //in m/s
    float m_degPerTic;                  //in deg/s  - used to determine rate of direction change
    float m_shipAccelTime;              //in s      - used to check time for speed change
    float m_shipMaxAccelTime;           //in s      - used to determine accel rate, and total accel time

    double m_radians;                   //in rad    - radians left in an ongoing turn

    GPoint m_position;                  //in m
    GVector m_velocity;                 //in m/s

    //User controlled information used by a state to determine what to do.
    bool m_stop;                        //used to denote Stop() has been called to avoid multiple stops (and associated decel)
    bool m_accel;                       //used to execute code for increasing ship speed
    bool m_decel;                       //used to execute code for decreasing ship speed
    bool m_cloaked;
    bool m_turning;                     //used to execute code for ship turning
    bool m_tractored;
    bool m_tractorPause;

    uint8 m_ballMode;                   //current state of ball

    int32 m_stopDistance;               //from destination, in m

    uint8 m_turnTic;                    //time into turn
    int8 m_orbiting;                    // 0=no orbit, >0=in orbit, 1=at distance, 2=too close , 3=too far, 4=way too close, 5=way too far
    //Destiny::Ball::stateStamp m_stateStamp; //state and count of current state since beginning, in seconds
    //Destiny::Ball::timeStamp m_timeStamp; //mode and timestamp of when current mode began
    uint32 m_stateStamp;                //statestamp of when current state began, in seconds

    float m_orbitTime;                  //in s - time to complete one orbit using current variables
    float m_orbitRadTic;                //in rad/sec  - radians around orbit per tic
    float m_timeFraction;               //fuzzy logic - holds current euler value for time
    float m_turnFraction;               //fuzzy logic - used for turn accel/decel checks
    float m_prevSpeedFraction;          //fuzzy logic - previous percent of full speed.  used for speed changes
    float m_userSpeedFraction;          //fuzzy logic - user commanded percent of max speed
    float m_activeSpeedFraction;        //fuzzy logic - current percent of max speed
    float m_maxOrbitSpeedFraction;      //fuzzy logic - ship's max speed based on orbit data

    uint32 m_followDistance;            //in m
    uint32 m_targetDistance;            //in m
    double m_moveTime;                  //in ms       - time when speed change started.  used to calculate m_timeFraction
    double m_callTime;                  //in ms       - time client call was processed.  this is to coordinate tic calculations

    GPoint m_targetPoint;               //vector      - point in space used as current destination
    GVector m_shipHeading;              //direction ship is facing
    GVector m_targetHeading;            //direction to target from current heading
    std::pair<uint32, SystemEntity*> m_targetEntity;   //we do not own the SystemEntity*

    // movement methods
    void MoveObject();                  //apply velocity to our position for this round of movement
    void Orbit();
    void Follow();                      //follow or approach object in space
    void BeginMovement();               //set initial variables for all movement (common code)
    void UpdateVelocity(bool isMoving=false);

private:
    bool m_frozen;                      // hack to keep ship from moving when using modules that prevent movement
    bool m_changeDelay;                 // this is to try to sync destiny with client, as client has a delay when changing destiny states.

    // check to align destiny movement to tic
    bool m_ticAlign;
    void SendMovementPacket();
    PyTuple *mvPacket;

    // Internal Collision Methods   -allan Nov 2015
    bool m_bump;
    void CheckBump();                              //iterate thru objects in current bubble to check for collisions
    void Bump(SystemEntity* who);                  //math methods for determining direction and speed of bumped ships
    void Bounce(GVector direction, float speed);   //packet sending for ships after bounce

    // Internal Turn Methods    -allan  Aug - Oct, 2015
    bool IsTurn();                     //check for current heading vs target direction. return true if degrees > 2 for warp align and > 0.8 for normal movement
    void Turn();                       //apply velocity and heading updates as needed for turning
    void ClearTurn();

    // Internal Orbit shit      -allan  Jan 2020
    GPoint ComputePosition(double curRad);   // currently testing...wip
    double m_inclination;               //inclination of orbit
    double m_longAscNode;               //longitude of ascending node
    void ClearOrbit();

    // Internal Warp Methods
    Timer m_warpTimer;
    void InitWarp();
    void WarpAccel(uint16 sec_into_warp);
    void WarpCruise(uint16 sec_into_warp);
    void WarpDecel(uint16 sec_into_warp);
    void WarpStop(double currentShipSpeed);
    void WarpUpdate(double currentShipSpeed);

    // Variables used during Warp.
    class WarpState {
    public:
        WarpState(
            uint32 start_time_,
            double total_distance_,
            double warp_speed_,
            double accel_dist_,
            double cruise_dist_,
            double decel_dist_,
            float warp_time_,
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
        uint32 start_time;          //from sEntityList::GetStamp()
        double total_distance;      //in m
        double warpSpeed;           //in m/s
        double accelDist;           //in m
        double cruiseDist;          //in m
        double decelDist;           //in m
        float warpTime;             //in s
        bool accel;
        bool cruise;
        bool decel;
        GVector warp_vector;        //target direction based on ship's initial position
    };
    WarpState* m_warpState;		    //we own this.
};

#endif

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