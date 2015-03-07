/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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

#include "PyCallable.h"
#include "destiny/DestinyStructs.h"
#include "inventory/ItemRef.h"
#include "system/SystemEntity.h"
#include <eve-compat.h>

class SystemEntity;
class SystemManager;
class InventoryItem;
class PyRep;
class PyList;
class PyTuple;
class SystemBubble;

extern const float SPACE_FRICTION;
extern const double SPACE_FRICTION_SQUARED;
extern const float TIC_DURATION_IN_SECONDS;

static const GPoint NULL_ORIGIN = (0,0,0);

//this object manages an entity's position in the system.

class DestinyManager {
public:
    static uint32 GetStamp() { return(m_stamp); }
    static bool IsTicActive() { return(m_stampTimer.Check(false)); }
    static void TicCompleted() { if(m_stampTimer.Check(true)) m_stamp++; }
    Destiny::BallMode GetState() { return State; }

    DestinyManager(SystemEntity *self, SystemManager *system);
    ~DestinyManager();

    void Process();

    void SendSingleDestinyUpdate(PyTuple **up, bool self_only=false) const;
    void SendDestinyUpdate(std::vector<PyTuple *> &updates, bool self_only=false) const;
    void SendDestinyUpdate(std::vector<PyTuple *> &updates, std::vector<PyTuple *> &events, bool self_only=false) const;

	// Information query functions:
    const GPoint &GetPosition() const { return(m_position); }
    const GVector &GetVelocity() const { return(m_velocity); }
    double GetSpeedFraction() { return(m_activeSpeedFraction); }
	SystemManager * const GetSystemManager() { return m_system; }
	SystemBubble * const GetCurrentBubble() { return m_self->Bubble(); }

    //called whenever an entity is going away and can no longer be used as a target
    void EntityRemoved(SystemEntity *who);

    //Configuration:
    void SetShipCapabilities(InventoryItemRef ship, bool undock=false);
    void SetShipVariables(InventoryItemRef ship/*, bool undock=false*/);
	void SetMaxVelocity(double maxVelocity) { m_maxShipVelocity = maxVelocity; }
    void SetPosition(const GPoint &pt, bool update=true, bool selfOnly=false);
    void SetBubble(bool set = false) { m_inBubble = set; }

    //Global Actions:
    void Stop();
    void Halt();     // puts entity at 0 velocity
	void TractorBeamHalt(bool update = true);
    void TractorBeamFollow(SystemEntity *who, double mass, double maxVelocity, double distance, bool update=true);

    //Local Movement:
    void Follow(SystemEntity *who, double distance, bool update=true);
    void Orbit(SystemEntity *who, double distance, bool update=true);
    void OrbitingCruise(SystemEntity *who, double distance, bool update=false, double cruiseSpeed=-1.0);
    void SetSpeedFraction(float fraction=1.0f, bool startMovement=false, bool undock=false);
    void AlignTo(SystemEntity *ent);
    void GotoDirection(const GPoint &direction, float speed=1.0f);
    void GotoPoint(const GPoint &point);

    PyResult AttemptDockOperation();

	void Cloak();
	void UnCloak();

    void Undock(GPoint dockPosition, GPoint direction);

    //bigger movement:
    void WarpTo(const GPoint where, int32 distance=0);

    //Ship State Query functions:
    bool IsMoving() {
        if ((State == Destiny::DSTBALL_GOTO)
            || (State == Destiny::DSTBALL_FOLLOW)
            || (State == Destiny::DSTBALL_ORBIT)
            //|| (State == Destiny::DSTBALL_ALIGN)
            //|| (State == Destiny::DSTBALL_WARP)
        ) return true; else return false;
    }

    bool IsAligned();
    //bool IsAligning() { return ((State == Destiny::DSTBALL_ALIGN) ? true : false); }
    bool IsStopped() { return ((State == Destiny::DSTBALL_STOP) ? true : false); }
    bool IsOrbiting() { return ((State == Destiny::DSTBALL_ORBIT) ? true : false); }
    bool IsFollowing() { return ((State == Destiny::DSTBALL_FOLLOW) ? true : false); }
    //bool IsJumping() { return ((State == Destiny::DSTBALL_STOP) ? true : false); }
    bool IsWarping() { return ((m_warpState == NULL) ? false : true); }
	bool IsCloaked() { return m_cloaked; }

    //Destiny Update stuff:
    void SendSetState(const SystemBubble *b) const;
    void SendBallInfoOnUndock() const;
    void SendJumpIn() const;
    void SendJumpOut(uint32 stargateID) const;
	void SendJumpInEffect(std::string JumpEffect) const;
	void SendJumpOutEffect(std::string JumpEffect, uint32 locationID) const;
	void SendGateActivity(uint32 gateID) const;
    void SendTerminalExplosion() const;
    void SendBoardShip(const ShipRef boardShipRef) const;
    void SendEjectShip(const ShipRef capsuleRef, const ShipRef oldShipRef) const;
    void SendJettisonCargo(const InventoryItemRef itemRef) const;
    void SendAnchorDrop(const InventoryItemRef itemRef) const;
    void SendAnchorLift(const InventoryItemRef itemRef) const;
    void SendCloakShip(const bool IsWarpSafe) const;
    void SendUncloakShip() const;
	void SendSpecialEffect10(uint32 gateID, const ShipRef shipRef, uint32 targetID, std::string effectString, bool isOffensive, bool start, bool isActive) const;
	void SendSpecialEffect(const ShipRef shipRef, uint32 moduleID, uint32 moduleTypeID,
    uint32 targetID, uint32 chargeTypeID, std::string effectString, bool isOffensive, bool start, bool isActive, double duration, uint32 repeat) const;

    //  functions to return protected variables for SystemBubble exclusive WarpTo updates
    GPoint GetTargetPoint() { return (m_targetPoint); }
    uint32 GetTargetID() { return m_targetEntity.first; }
    double GetFollowDistance() { return m_targetDistance; }
    int32 GetDistance() { return (m_stopDistance); }
    int32 GetWarpSpeed() { return static_cast<int32>(m_shipWarpSpeed * 10); }

protected:
    void ProcessState();

    SystemEntity *const m_self;			//we do not own this.
    SystemManager *const m_system;		//we do not own this.

	//Timer m_destinyTimer;
    static uint32 m_stamp;
    static Timer m_stampTimer;
	//uint32 m_lastDestinyTime;			//from Timer::GetTimeSeconds()

    //the results of our labors:
    GPoint m_position;					//in m
    GVector m_velocity;					//in m/s

    //things dictated by our entity's configuration/equipment:
    double m_radius;                    //in m
    float m_mass;                       //in kg
    float m_massMKg;                    //in Millions of kg
    double m_maxShipVelocity;           //in m/s
    double m_shipAgility;               //in s/kg
    float m_shipWarpSpeed;              //in au/s
    float m_speedToLeaveWarp;           //in m/s
    uint8 m_warpAccelTime;              //in s
    uint8 m_warpDecelTime;              //in s
    double m_warpCapacitorNeed;         //in GJ
    double m_shipInertiaModifier;
    uint8 m_warpStrength;               //interger
    double m_capCharge;

    //derrived from other params:
    double m_maxVelocity;               //in m/s
    double m_shipMaxAccelTime;              //in s  this is used to determine accel rate, and total accel time

    //User controlled information used by a state to determine what to do.
    Destiny::BallMode State;
    float m_userSpeedFraction;			//speed % from 0.0 to 1.0  - set by user command
    float m_activeSpeedFraction;		//speed % from 0.0 to 1.0  - current ship speed
    GPoint m_targetPoint;
    double m_targetDistance;            //in m
    GPoint m_moveDirection;             //direction ship is facing
    int32 m_stopDistance;               //from destination, in m
    double m_moveTimer;
    bool m_inBubble;                    //used to tell if client is in bubble or not.
    uint32 m_stateStamp;				//some states need to know when they were entered.
    bool m_cloaked;
    bool m_stop;                        //used to denote Stop() has been called to avoid multiple stops (and associated decel)
    std::pair<uint32, SystemEntity *> m_targetEntity;   //we do not own the SystemEntity *

    void _UpdateVelocity(bool isMoving=false, bool undock=false/*, bool changeSpeed=false*/);

    void _Move();						//apply our current velocity to our position for for this round of movement
    void _Follow();                     //or approach object in space
    void _Orbit();

private:
    void _InitWarp();
    void _WarpAccel(uint16 sec_into_warp);
    void _WarpCruise(uint16 sec_into_warp);
    void _WarpDecel(uint16 sec_into_warp);
    void _WarpUpdate(double currentShipSpeed);
    void _WarpStop(uint16 sec_into_warp, double currentShipSpeed);

    //internal state variables used during warp.
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
            bool stop_,
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
        stop(stop_),
        warp_vector(warp_vector_)
        { }
        uint32 start_time;          //from Destiny::GetStamp()
        double total_distance;      //in m
        double warpSpeed;           //in m/s
        double accelDist;           //in m
        double cruiseDist;          //in m
        double decelDist;           //in m
        float warpTime;             //in s
        bool accel;
        bool cruise;
        bool decel;
        bool stop;
        GVector warp_vector;        //target direction based on ship's initial position
    };
    WarpState *m_warpState;		//we own this.
};

#endif
