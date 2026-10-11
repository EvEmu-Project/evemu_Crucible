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
#include "system/DestinyCollision.h"
#include "inventory/ItemRef.h"

class Buffer;
class InventoryItem;
class Missile;
class PyRep;
class PyList;
class PyTuple;
class ShipSE;
class SystemBubble;
class SystemEntity;
class SystemManager;

/* this object manages an entity's position and movement in a system.
 *
 * the client runs its own destiny simulation and applies every destiny update at the tick it was stamped with.
 * to keep the server in sync, this class runs the same simulation as the Crucible client's destiny.dll, tick for tick.
 * every command that changes movement is applied here with the client's semantics at the moment it is sent,
 * and SystemManager::ProcessDestiny() advances all entities one tick at a time (see DestinyManager.cpp for details)
 */
class DestinyManager {
public:
    DestinyManager(SystemEntity* self);
    ~DestinyManager();

    void Process();

    /* per-tick simulation.  called by SystemManager::ProcessDestiny() for every moving entity in the system, in this order */
    void EvolveThrust(int32 tick);  // phase 1: work out thrust (all entities are evaluated before any of them move)
    void EvolveCollide(Destiny::BallparkSnapshot& ballpark);   // phase 2: collisions, for massive balls
    void EvolveMove(int32 tick);    // phase 3: integrate position and velocity
    void PostEvolve();              // server-side consequences (bubbles, warp side effects).  may add/remove entities

    // appends the DataSector and mode-specific part of this ball for SetState/AddBalls
    void EncodeDestiny(Buffer& into) const;
    // fills in the moving parts of this ball for the collision code
    void GetCollisionState(Destiny::CollisionBall& ball) const;
    // massive (solid) ball.  the client only runs collisions for massive balls, against other massive balls.
    //  this is the flag the client currently has for us, so it is false during warp
    bool IsMassive() const;

    void SendSingleDestinyEvent(PyTuple** ev, bool self_only=false) const;
    void SendSingleDestinyUpdate(PyTuple** up, bool self_only=false) const;
    void SendDestinyUpdate(std::vector<PyTuple*> &updates, bool self_only=false) const;
    void SendDestinyUpdate(std::vector<PyTuple*> &updates, std::vector<PyTuple*> &events, bool self_only=false) const;

    /* Informational query functions: */
    const GPoint &GetPosition() const                   { return m_position; }
    const GVector &GetVelocity() const                  { return m_velocity; }
    float GetSpeedFraction()                            { return m_speedFraction; }
    float GetSpeed()                                    { return static_cast<float>(m_velocity.length()); }

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
    // startMovement will start a stopped ship moving along its current heading
    void SetSpeedFraction(float fraction=1.0f);

    /* Larger movement */
    void WarpTo(const GPoint& where, int32 distance = 0, bool autoPilot = false, SystemEntity* pSE = nullptr);

    /* Ship State Query functions */
    bool IsMoving()                                     { return ((m_ballMode != Destiny::Ball::Mode::STOP) or (m_velocity.lengthSquared() > 1.0)); }

    /* Movement checks */
    bool IsAligned(GPoint &targetPoint);
    bool IsGoto()                                       { return (m_ballMode == Destiny::Ball::Mode::GOTO); }
    bool IsStopped()                                    { return (m_ballMode == Destiny::Ball::Mode::STOP); }
    bool IsOrbiting()                                   { return (m_ballMode == Destiny::Ball::Mode::ORBIT); }
    bool IsFollowing()                                  { return (m_ballMode == Destiny::Ball::Mode::FOLLOW); }
    bool IsAligningForWarp() const                      { return ((m_ballMode == Destiny::Ball::Mode::WARP) and (m_effectStamp == -1)); }
    bool IsWarping() const                              { return ((m_ballMode == Destiny::Ball::Mode::WARP) and (m_effectStamp != -1)); }
    bool IsCloaked()                                    { return m_cloaked; }
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
    void SendJumpInEffect(std::string JumpEffect);
    void SendJumpOutEffect(std::string JumpEffect, uint32 shipID);
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
    int32 GetWarpSpeed()                                { return static_cast<int32>(m_shipWarpSpeed * 10); }
    uint32 GetTargetID()                                { return m_targetEntity.first; }
    SystemEntity* GetTargetEntity()                     { return m_targetEntity.second; }
    GPoint GetTargetPoint()                             { return m_targetPoint; }
    double GetMaxVelocity()                             { return m_maxShipSpeed; }
    double GetFollowDistance()                          { return m_followRange; }
    double GetMass()                                    { return m_mass; }
    double GetAgility()                                 { return m_shipAgility; }
    double GetInertia()                                 { return m_shipInertia; }
    uint32 GetStateStamp()                              { return m_stateStamp; }
    GVector GetHeading()                                { return m_shipHeading; }

    float GetAlignTime()                                { return m_alignTime; }
    float GetAccelTime()                                { return m_shipMaxAccelTime; }
    float GetWarpTime()                                 { return m_timeToEnterWarp; }
    float GetWarpDropSpeed()                            { return m_maxShipSpeed; }  // client drops out of warp below max velocity
    double GetRadius()                                  { return m_radius; }
    double GetCapNeed()                                 { return m_warpCapacitorNeed; }

    // set all movement vars for missile and add to system
    //  this is used by all entities (pc, npc, drone, sentry, pos, etc)
    void MakeMissile(Missile* missile);
    // this missile's path touches its target this tick.  Missile::Process() applies the damage
    bool HasMissileHit() const                          { return m_missileHit; }

    bool IsFrozen()                                     { return m_frozen; }
    void SetFrozen(bool set=false)                      { m_frozen = set; }

    // Prevents actions if the player is performing the login warp
    bool AbortIfLoginWarping(bool showMsg);

protected:
    SystemEntity* const mySE;			//we do not own this.
    SystemBubble* m_targBubble;         //we do not own this.  bubble at the warp exit point

    bool m_hasSentShipUpdates;

    // ship configuration
    float m_mass;                       //in kg
    float m_massMKg;                    //in Mkg  - millions of kg
    float m_alignTime;                  //in s      - time to reach 75% of max speed from a stop
    float m_maxShipSpeed;               //in m/s
    float m_shipWarpSpeed;              //in au/s
    float m_timeToEnterWarp;            //in s
    float m_shipMaxAccelTime;           //in s      - time to reach 99.99% of max speed from a stop
    float m_preTractorMass;             //in kg     - mass to restore when a tractor beam lets go

    double m_radius;                    //in m
    double m_capNeeded;                 //in GJ     - capacitor left after the warp drive engages
    double m_warpCapacitorNeed;         //in GJ     - capacitor charged needed to initiate warp
    double m_shipAgility;               //in s      - time constant of movement (mass * inertia / 1e6)
    double m_shipInertia;               //          - inertia modifier.  this is the client ball's 'agility'

    /* client-mirrored ball state.  names in brackets are the matching destiny Ball members */
    uint8 m_ballMode;                   // [mMode]
    GPoint m_position;                  // [mNewPos]
    GVector m_velocity;                 // [mNewVel]
    GPoint m_targetPoint;               // [mGoto]  goto point, follow point, or warp destination
    GVector m_thrust;                   // [mLastG] acceleration worked out in EvolveThrust()
    float m_speedFraction;              // [mSpeedFraction]
    float m_followRange;                // [mFollowRange]
    std::pair<uint32, SystemEntity*> m_targetEntity;   // [mFollowId/mFollowPtr]  we do not own the SystemEntity*
    int32 m_effectStamp;                // [mEffectStamp] for warp: -1 while aligning, else tick warp proper started
    double m_warpDistance;              // [mLastCollision] total length of warp proper
    double m_warpMinRange;              // [mFollowId as double] warp-to range
    int32 m_warpFactor;                 // [mOwnerId] warp speed in AU/s * 10
    uint32 m_missileOwnerID;            // [mOwnerId] for missiles: the ball that launched us
    double m_missileDragFactor;         // [Ball+0x80] the client works out a missile's drag once, when it is added
    Destiny::CollisionResult m_collision;   // [Ball+0x180, +0x88] this tick's collision response

    GVector m_shipHeading;              // direction ship is facing.  follows velocity, kept when stopped
    uint32 m_stateStamp;                // stamp of when current state began
    bool m_resetSpeedOnMove;            // set by Stop().  next movement command from a stop goes to full speed

    // server-side work queued during the simulation passes, handled in PostEvolve()
    bool m_warpActivated;
    bool m_warpExited;
    bool m_targetLost;
    bool m_missileHit;

    //User controlled information
    bool m_cloaked;
    bool m_tractored;

private:
    bool m_frozen;                      // hack to keep ship from moving when using modules that prevent movement

    // client-semantics state changes.  these do not send anything
    void ApplyStop();
    void ApplyGotoPoint(const GPoint& point);
    void ApplyFollow(SystemEntity* pSE, float range, uint8 mode);
    void ApplyWarpTo(const GPoint& where, double minRange, int32 warpFactor);
    void ApplyLaunchMissile(SystemEntity* pLauncher, SystemEntity* pTarget);
    void MoveBallTo(const GPoint& pt);

    void BeginMovement();               // common setup for all movement commands
    SystemEntity* GetFollowTarget();
    void StopFollowers();
    bool IsTargetInvalid();             // performs common target checks

    // client physics (destiny.dll)
    double BallMass() const;
    double DragFactor() const;
    GVector GotoThrust(const GPoint& target, bool missile=false) const;
    GVector MissileThrust(SystemEntity* pTarget);
    GVector OrbitThrust(SystemEntity* pTarget, int32 tick) const;
    bool IsAlignedForWarp() const;
    void RealWarp(int32 tick);
    bool WarpDistance(GPoint& pos, GVector& vel, double t);
    void Integrate(GPoint& pos, GVector& vel, const GVector& accel) const;

    // server-side warp handling
    void OnWarpActivated();
    void OnWarpExited();
    void UpdateWarpBubble();
    void SendMassive() const;           // tells clients our current massive flag
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