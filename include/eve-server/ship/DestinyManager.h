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
	Author:		Zhur
*/

#ifndef __DESTINYMANAGER_H_INCL__
#define __DESTINYMANAGER_H_INCL__

class SystemEntity;
class SystemManager;
class InventoryItem;
class PyRep;
class PyList;
class PyTuple;
class SystemBubble;

extern const double SPACE_FRICTION;
extern const double SPACE_FRICTION_SQUARED;
extern const double TIC_DURATION_IN_SECONDS;

//this object manages an entity's position in the system.
//NOTE: we currently have no inertial mass
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
	void SendDestinyUpdate(std::vector<PyTuple *> &updates, bool self_only) const;
	void SendDestinyUpdate(std::vector<PyTuple *> &updates, std::vector<PyTuple *> &events, bool self_only) const;
	
	const GPoint &GetPosition() const { return(m_position); }
	const GVector &GetVelocity() const { return(m_velocity); }
    double GetSpeedFraction() { return(m_activeSpeedFraction); }
	
	//called whenever an entity is going away and can no longer be used as a target
	void EntityRemoved(SystemEntity *who);
	
	//Configuration:
	void SetShipCapabilities(InventoryItemRef ship);
	void SetPosition(const GPoint &pt, bool update=true);
	
	//Global Actions:
	void Stop(bool update=true);
	void Halt(bool update=true);	//like stop with no de-acceleration.
	
	//Local Movement:
	void Follow(SystemEntity *who, double distance, bool update=true);
	void Orbit(SystemEntity *who, double distance, bool update=true);
	void SetSpeedFraction(double fraction, bool update=true);
	void AlignTo(const GPoint &direction, bool update=true);
	void GotoDirection(const GPoint &direction, bool update=true);
    PyResult AttemptDockOperation();
	
	//bigger movement:
	void WarpTo(const GPoint &where, double distance, bool update=true);

	//Destiny Update stuff:
	void SendSetState(const SystemBubble *b) const;
	void SendJumpIn() const;
	void SendJumpOut(uint32 stargateID) const;
	void SendGateActivity() const;
	void SendTerminalExplosion() const;
    void SendBoardShip(const ShipRef boardShipRef) const;
    void SendEjectShip(const ShipRef capsuleRef, const ShipRef oldShipRef) const;
    void SendJettisonCargo(const InventoryItemRef itemRef) const;
    void SendAnchorDrop(const InventoryItemRef itemRef) const;
    void SendAnchorLift(const InventoryItemRef itemRef) const;
    void SendCloakShip(const ShipRef shipRef, const bool IsWarpSafe) const;
    void SendUncloakShip(const ShipRef shipRef) const;

protected:
	void ProcessTic();
	
	SystemEntity *const m_self;	//we do not own this.
	SystemManager *const m_system;	//we do not own this.
//	Ga::Body *m_body;		//we own a reference to this
//	Ga::Shape *m_shape;		//we own a reference to this
	
//	Timer m_destinyTimer;
	static uint32 m_stamp;
	static Timer m_stampTimer;
//	uint32 m_lastDestinyTime;	//from Timer::GetTimeSeconds()
	
	//the results of our labors:
	GPoint m_position;		//in m
	GVector m_velocity;		//in m/s
//	GVector m_direction;		//normalized, `m_velocity` stores our magnitude
//	double m_velocity;		//in m/s, the magnitude of direction
//	double m_acceleration;	//in m/s^2, should probably be using a vector here too.
	
	//derrived from other params:
	void _UpdateDerrived();
	double m_maxVelocity;	//in m/s
	double m_accelerationFactor;	//crazy units
	double m_velocityAdjuster;		//unitless
	
    double m_warpDecelerateFactor;
	
	//User controlled information used by a state to determine what to do.
	Destiny::BallMode State;
	double m_userSpeedFraction;		//unitless
	double m_activeSpeedFraction;	//unitless
	GPoint m_targetPoint;
	double m_targetDistance;
	uint32 m_stateStamp;			//some states need to know when they were entered.
    std::pair<uint32, SystemEntity *> m_targetEntity;   //we do not own the SystemEntity *
    //SystemEntity *m_targetEntity;	//we do not own this.
	
	//things dictated by our entity's configuration/equipment:
	double m_radius;			//in m
	double m_mass;				//in kg
	double m_maxShipVelocity;	//in m/s
	double m_shipAgility;		//unitless
	double m_shipInertia;
//	GVector m_inertia;
	
	bool _Turn();	//compare m_targetDirection and m_direction, and turn as needed.
	void _Move();	//apply our velocity and direction to our position for 1 unit of time (a second)
	void _Follow();
	void _Warp();	//carry on our current warp.
	void _MoveAccel(const GVector &calc_acceleration);
	void _Orbit();

private:

	//internal state variables used during warp.
	class WarpState {
	public:
		WarpState(uint32 start_stamp_, double total_distance_, double speed_, double acceleration_time_, double slow_time_, const GVector &normvec_them_to_us_)
			: start_stamp(start_stamp_), total_distance(total_distance_), speed(speed_), acceleration_time(acceleration_time_), slow_time(slow_time_), normvec_them_to_us(normvec_them_to_us_) {}
		uint32 start_stamp;	//destiny stamp of when the warp started.
		double total_distance;
		double speed;
		double acceleration_time;
		double slow_time;
		GVector normvec_them_to_us;
	};
	const WarpState *m_warpState;	//we own this. Allocated so we can have consts.
	void _InitWarp();
};

#endif
