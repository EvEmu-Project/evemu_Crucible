/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*

Dynamic Bodies:
	- shape
	- `mass`
	- `radius`
	- `volume`???
	- `Inertia`???
	- inertia information
	- position
	- velocity
	- angular velocity
	- collide with things
		- approximate eve "run into and stop/turn around" collisions

Ship: extends Dynamic
	- `maxVelocity`
	- thrust (propulsion + speed ratio)
	- angular thrust
	- "stopping" algorithm
	- "orbit" algorithm
	- "turning" algorithm 

Static Bodies:
	- shape
	- position


detech clients moving into aggro radii











*/




#ifndef EVE_CLIENT_H
#define EVE_CLIENT_H

#include "../common/packet_types.h"
#include "../common/timer.h"
#include "../common/gpoint.h"

#include <string>
#include <map>
#include <set>

#include "ClientSession.h"
#include "system/SystemEntity.h"
#include "ship/ModuleManager.h"
#include "../common/EVEUtils.h"

class EVEPresentation;
class PyPacket;
class PyRep;
class PyRepDict;
class PyRepTuple;
class PyRepList;
class PyRepSubStream;
class PyCallStream;
class PyServiceMgr;
class PyAddress;
class EVENotificationStream;
class InventoryItem;
class LSCChannel;
class SystemManager;

class CharacterData {
public:
	uint32 charid;
	
	std::string name;
	std::string title;
	std::string description;

	double bounty;
	double balance;
	double securityRating;
	uint32 logonMinutes;

	uint32 corporationID;
	uint64 corporationDateTime;
	uint32 allianceID;
	
	uint32 stationID;
	uint32 solarSystemID;
	uint32 constellationID;
	uint32 regionID;

	uint32 typeID;
	uint32 bloodlineID;
	uint32 genderID;
	uint32 ancestryID;
	uint32 schoolID;
	uint32 departmentID;
	uint32 fieldID;
	uint32 specialityID;
	
	uint32 Intelligence;
	uint32 Charisma;
	uint32 Perception;
	uint32 Memory;
	uint32 Willpower;

	uint64 startDateTime;
	uint64 createDateTime;

	uint8 raceID;	//must correspond to our bloodlineID!

	void ModifyAttributes(uint32 add_int, uint32 add_cha, uint32 add_per, uint32 add_mem, uint32 add_wll) {
		Intelligence += add_int;
		Charisma += add_cha;
		Perception += add_per;
		Memory += add_mem;
		Willpower += add_wll;
	}
};

class CharacterAppearance {
public:
	uint32 accessoryID;
	uint32 beardID;
	uint32 costumeID;
	uint32 decoID;
	uint32 eyebrowsID;
	uint32 eyesID;
	uint32 hairID;
	uint32 lipstickID;
	uint32 makeupID;
	uint32 skinID;
	uint32 backgroundID;
	uint32 lightID;
	
	double headRotation1;
	double headRotation2;
	double headRotation3;
	double eyeRotation1;
	double eyeRotation2;
	double eyeRotation3;
	double camPos1;
	double camPos2;
	double camPos3;
	double morph1e;
	double morph1n;
	double morph1s;
	double morph1w;
	double morph2e;
	double morph2n;
	double morph2s;
	double morph2w;
	double morph3e;
	double morph3n;
	double morph3s;
	double morph3w;
	double morph4e;
	double morph4n;
	double morph4s;
	double morph4w;
};

class CorpMemberInfo {
public:
	CorpMemberInfo()
	: corpHQ(0), corprole(0), rolesAtAll(0), rolesAtBase(0), rolesAtHQ(0), rolesAtOther(0) {}
	uint32 corpHQ;	//this really dosent belong here...
	uint64 corprole;
	uint64 rolesAtAll;
	uint64 rolesAtBase;
	uint64 rolesAtHQ;
	uint64 rolesAtOther;
};

class Client;

class Functor {
public:
	Functor() {}
	virtual ~Functor() {}
	virtual void operator()() = 0;
};

class ClientFunctor : public Functor {
public:
	ClientFunctor(Client *c) : m_client(c) {}
	virtual ~ClientFunctor() {}
	//leave () pure
protected:
	Client *const m_client;
};

class FunctorTimerQueue {
public:
	typedef uint32 TimerID;
	
	FunctorTimerQueue();
	~FunctorTimerQueue();

	//schedule takes ownership of the functor.
	TimerID Schedule(Functor **what, uint32 in_how_many_ms);
	bool Cancel(TimerID id);
	
	void Process();
	
protected:
	//this could be done a TON better, but want to get something
	//working first, it is call encapsulated in here, so it shoud
	//be simple to gut this object and put in a real scheduler.
	class Entry {
	public:
		Entry(TimerID id, Functor *func, uint32 time_ms);
		~Entry();
		const TimerID id;
		Functor *const func;
		Timer when;
	};
	TimerID m_nextID;
	std::vector<Entry *> m_queue;	//not ordered or anything useful.
};




//DO NOT INHERIT THIS OBJECT!
class Client : public DynamicSystemEntity {
public:
	Client(PyServiceMgr *services, EVEPresentation **net);
	virtual ~Client();
	
	ClientSession session;
	ModuleManager modules;

	virtual void	QueuePacket(PyPacket *p);
	virtual void	FastQueuePacket(PyPacket **p);
	bool			ProcessNet();
	virtual void	Process();
	virtual void	ProcessDestiny(uint32 stamp);

	uint32 GetAccountID() const { return(m_accountID); }
	uint32 GetRole() const { return(m_role); }

	uint32 GetCharacterID() const { return(m_char.charid); }
	CharacterData &GetChar() { return(m_char); }
	uint32 GetCorporationID() const { return(m_char.corporationID); }
	uint32 GetAllianceID() const { return(m_char.allianceID); }
	const CorpMemberInfo &GetCorpInfo() const { return(m_corpstate); }

	uint32 GetLocationID() const { return(IsInSpace() ? GetSystemID() : GetStationID()); }
	uint32 GetStationID() const { return(m_char.stationID); }
	uint32 GetSystemID() const { return(m_char.solarSystemID); }
	uint32 GetConstellationID() const { return(m_char.constellationID); }
	uint32 GetRegionID() const { return(m_char.regionID); }
	inline double x() const { return(GetPosition().x); }	//this is terribly inefficient.
	inline double y() const { return(GetPosition().y); }	//this is terribly inefficient.
	inline double z() const { return(GetPosition().z); }	//this is terribly inefficient.
	bool IsInSpace() const { return(GetStationID() == 0); }
	
	double GetBalance() const { return(m_char.balance); }
	bool AddBalance(double amount);

	uint32 GetShipID() const;
	void BoardShip(InventoryItem *new_ship);
	void MoveToLocation(uint32 location, const GPoint &pt);
	void MoveToPosition(const GPoint &pt);
	void MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag);
	bool EnterSystem(const GPoint &p);
	void WarpTo(const GPoint &p);
	bool LoadInventory(uint32 ship_id);
	void JoinCorporationUpdate(uint32 corp_id);
	inline InventoryItem *Ship() const { return(m_ship); }
	void SavePosition();
	
	double GetPropulsionStrength() const;
	
	bool LaunchDrone(InventoryItem *drone);
	
	//
	void SendHandshake();
	
	void SendNotification(const PyAddress &dest, EVENotificationStream *noti, bool seq=true);
	void SendNotification(const char *notifyType, const char *idType, PyRepTuple **payload, bool seq=true);
	void SessionSync();

	//destiny stuff...
	void SendInitialDestinySetstate();
	void StargateJump(uint32 fromGate, uint32 toGate);
	
	//some hacks to handle movement updates until a better destiny system is figured out...
	void InitialEnterGame();
	void UndockingIntoSpace();
	void CheckLogIntoSpace();
	
	//SystemEntity interface:
	virtual EntityClass GetClass() const { return(ecClient); }
	virtual bool IsClient() const { return(true); }
	virtual Client *CastToClient() { return(this); }
	virtual const Client *CastToClient() const { return(this); }

	virtual uint32 GetID() const { return(GetShipID()); }	//our entity in space is our ship!
	virtual const char *GetName() const { return(m_char.name.c_str()); }
	virtual double GetRadius() const;
	virtual PyRepDict *MakeSlimItem() const;
	virtual void MakeDamageState(DoDestinyDamageState &into) const;
	virtual void QueueDestinyUpdate(PyRepTuple **du);
	virtual void QueueDestinyEvent(PyRepTuple **multiEvent);

	virtual void TargetAdded(SystemEntity *who);
	virtual void TargetLost(SystemEntity *who);
	virtual void TargetedAdd(SystemEntity *who);
	virtual void TargetedLost(SystemEntity *who);
	virtual void TargetsCleared();

	virtual double GetMass() const;
	virtual double GetMaxVelocity() const;
	virtual double GetAgility() const;

	virtual void ApplyDamageModifiers(Damage &d, SystemEntity *target);
	virtual void ApplyDamage(Damage &d);
	virtual void Killed(Damage &fatal_blow);
	virtual SystemManager *System() const { return(m_system); }
	
	void SendErrorMsg(const char *fmt, ...);
	void SendNotifyMsg(const char *fmt, ...);
	void SendInfoModalMsg(const char *fmt, ...);
	void SelfChatMessage(const char *fmt, ...);
	void SelfEveMail(const char *subject, const char *fmt, ...);
	void ChannelJoined(LSCChannel *chan);
	void ChannelLeft(LSCChannel *chan);

	PyServiceMgr *GetServices() const { return(m_services); }
	
	FunctorTimerQueue::TimerID Delay( uint32 time_in_ms, void (Client::* clientCall)() );
	FunctorTimerQueue::TimerID Delay( uint32 time_in_ms, ClientFunctor **functor );
	
protected:
	//login stuff:
	void _ProcessLogin(PyPacket *packet);
	void _SendLoginFailed(uint32 callid);
	void _SendLoginSuccess(uint32 callid);
	void _SendPingRequest();
	void _BuildServiceListDict(PyRepDict *into);
	
	void _ProcessNotification(PyPacket *packet);
	void _CheckSessionChange();

	void _ExecuteLogIntoSpace();
	void _ExecuteLogIntoStation();
	void _ExecuteUndockIntoSpace();
	void _SendInitialSkillTraining();
	
	void _ReduceDamage(Damage &d);
	
	//call stuff
	void _ProcessCallRequest(PyPacket *packet);
	void _SendCallReturn(PyPacket *req, PyRepTuple **return_value, const char *channel = NULL);
	void _SendException(PyPacket *req, MACHONETERR_TYPE type, PyRep **payload);

	InventoryItem *m_ship;

	PyServiceMgr *const m_services;
	EVEPresentation *const m_net;	//we own this! never NULL.
	Timer m_pingTimer;

	uint32 m_accountID;
	uint32 m_role;		//really should be a uint64
	uint32 m_gangRole;

	SystemManager *m_system;	//we do not own this

	CharacterData m_char;
	CorpMemberInfo m_corpstate;

	std::set<LSCChannel *> m_channels;	//we do not own these.
	
	//this whole move system is a piece of crap:
	typedef enum {
		msIdle,
		msWarp,
		msJump,
		msJump2,
		msJump3,
		msLogIntoSpace,		//initial login into space
		msLogIntoStation,	//initial login into a station
		msUndockIntoSpace
	} _MoveState;
	void _postMove(_MoveState type, uint32 wait_ms=500);
	_MoveState m_moveState;
	Timer m_moveTimer;
	uint32 m_moveSystemID;
	GPoint m_movePoint;
	//bool m_warpActive;	//only for destiny setstate right now...
	void _ExecuteWarp();
	void _ExecuteJump();
	void _ExecuteJump_Phase2();
	void _ExecuteJump_Phase3();
	
private:
	//queues for destiny updates:
	std::vector<PyRep *> m_destinyEventQueue;	//we own these. These are events as used in OnMultiEvent
	std::vector<PyRepTuple *> m_destinyUpdateQueue;	//we own these. They are the `update` which go into DoDestinyAction
	void _SendQueuedUpdates(uint32 stamp);
	
	FunctorTimerQueue m_delayQueue;
	
	uint32 m_nextNotifySequence;
};


//simple functor for void calls.
class SimpleClientFunctor : public ClientFunctor {
public:
	typedef void (Client::* clientCall)();
	SimpleClientFunctor(Client *c, clientCall call) : ClientFunctor(c), m_call(call) {}
	virtual ~SimpleClientFunctor() {}
	virtual void operator()() {
		(m_client->*m_call)();
	}
public:
	const clientCall m_call;
};



#endif

