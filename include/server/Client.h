/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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

detect clients moving into agro radius
*/

#ifndef EVE_CLIENT_H
#define EVE_CLIENT_H

#include "../common/packet_types.h"
#include "../common/timer.h"
#include "../common/gpoint.h"
#include "../common/EVEUtils.h"
#include "../common/EVEPresentation.h"

#include "ClientSession.h"

#include "inventory/InventoryItem.h"
#include "character/Character.h"
#include "ship/Ship.h"

#include "system/SystemEntity.h"
#include "ship/ModuleManager.h"

class CryptoChallengePacket;
class EVENotificationStream;
class PyRepSubStream;
class InventoryItem;
class SystemManager;
class PyServiceMgr;
class PyCallStream;
class PyRepTuple;
class LSCChannel;
class PyAddress;
class PyRepList;
class PyRepDict;
class PyPacket;
class Client;
class PyRep;

// not needed now:
/*
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
	//working first, it is call encapsulated in here, so it should
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
*/
//DO NOT INHERIT THIS OBJECT!
class Client
: public DynamicSystemEntity
{
public:
	Client(PyServiceMgr &services, EVETCPConnection *&con);
	virtual ~Client();
	
	ClientSession session;
	ModuleManager modules;

	virtual void    QueuePacket(PyPacket *p);
	virtual void    FastQueuePacket(PyPacket **p);
	bool            ProcessNet();
	virtual void    Process();

	uint32 GetAccountID() const                     { return m_accountID; }
	uint32 GetAccountRole() const                   { return m_accountRole; }

	// character data
	Character *GetChar() const                      { return m_char; }

	uint32 GetCharacterID() const                   { return GetChar() == NULL ? 0 : GetChar()->itemID(); }
	uint32 GetCorporationID() const                 { return GetChar() == NULL ? 0 : GetChar()->corporationID(); }
	uint32 GetAllianceID() const                    { return GetChar() == NULL ? 0 : GetChar()->allianceID(); }
	uint32 GetStationID() const                     { return GetChar() == NULL ? 0 : GetChar()->stationID(); }
	uint32 GetSystemID() const                      { return GetChar() == NULL ? 0 : GetChar()->solarSystemID(); }
	uint32 GetConstellationID() const               { return GetChar() == NULL ? 0 : GetChar()->constellationID(); }
	uint32 GetRegionID() const                      { return GetChar() == NULL ? 0 : GetChar()->regionID(); }

	uint32 GetLocationID() const
	{
		if (IsInSpace() == true)
			return GetSystemID();
		else
			return GetStationID();
	}

	uint32 GetCorpHQ() const                        { return GetChar() == NULL ? 0 : GetChar()->corporationHQ(); }
	uint32 GetCorpRole() const                      { return GetChar() == NULL ? 0 : GetChar()->corpRole(); }
	uint32 GetRolesAtAll() const                    { return GetChar() == NULL ? 0 : GetChar()->rolesAtAll(); }
	uint32 GetRolesAtBase() const                   { return GetChar() == NULL ? 0 : GetChar()->rolesAtBase(); }
	uint32 GetRolesAtHQ() const                     { return GetChar() == NULL ? 0 : GetChar()->rolesAtHQ(); }
	uint32 GetRolesAtOther() const                  { return GetChar() == NULL ? 0 : GetChar()->rolesAtOther(); }

	uint32 GetShipID() const { return GetID(); }
	// this is kinda hacky:
	Ship *GetShip() const { return static_cast<Ship *>( Item() ); }

	bool IsInSpace() const { return(GetStationID() == 0); }
	inline double x() const { return(GetPosition().x); }	//this is terribly inefficient.
	inline double y() const { return(GetPosition().y); }	//this is terribly inefficient.
	inline double z() const { return(GetPosition().z); }	//this is terribly inefficient.

	double GetBounty() const                       { return GetChar() == NULL ? 0.0 : GetChar()->bounty(); }
	double GetSecurityRating() const               { return GetChar() == NULL ? 0.0 : GetChar()->securityRating(); }
	double GetBalance() const                      { return GetChar() == NULL ? 0.0 : GetChar()->balance(); }
	bool AddBalance(double amount);

	PyServiceMgr &services() const { return(m_services); }

	void Login(CryptoChallengePacket *pack);
	void BoardShip(Ship *new_ship);
	void MoveToLocation(uint32 location, const GPoint &pt);
	void MoveToPosition(const GPoint &pt);
	void MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag);
	bool EnterSystem();
	bool SelectCharacter(uint32 char_id);
	void JoinCorporationUpdate(uint32 corp_id);
	void SavePosition();
	void SetTrainStatus(bool status, uint64 timeEndTrain);
	
	double GetPropulsionStrength() const;
	
	bool LaunchDrone(InventoryItem *drone);
	
	void SendNotification(const PyAddress &dest, EVENotificationStream *noti, bool seq=true);
	void SendNotification(const char *notifyType, const char *idType, PyRepTuple **payload, bool seq=true);
	void SessionSync();

	//destiny stuff...
	void WarpTo(const GPoint &p, double distance);
	void StargateJump(uint32 fromGate, uint32 toGate);
	
	//SystemEntity interface:
	virtual EntityClass GetClass() const { return(ecClient); }
	virtual bool IsClient() const { return true; }
	virtual Client *CastToClient() { return(this); }
	virtual const Client *CastToClient() const { return(this); }

	virtual const char *GetName() const { return GetChar() == NULL ? "(null)" : GetChar()->itemName().c_str(); }
	virtual PyRepDict *MakeSlimItem() const;
	virtual void QueueDestinyUpdate(PyRepTuple **du);
	virtual void QueueDestinyEvent(PyRepTuple **multiEvent);

	virtual void TargetAdded(SystemEntity *who);
	virtual void TargetLost(SystemEntity *who);
	virtual void TargetedAdd(SystemEntity *who);
	virtual void TargetedLost(SystemEntity *who);
	virtual void TargetsCleared();

	virtual void ApplyDamageModifiers(Damage &d, SystemEntity *target);
	virtual bool ApplyDamage(Damage &d);
	virtual void Killed(Damage &fatal_blow);
	virtual SystemManager *System() const { return(m_system); }
	
	void SendErrorMsg(const char *fmt, ...);
	void SendNotifyMsg(const char *fmt, ...);
	void SendInfoModalMsg(const char *fmt, ...);
	void SelfChatMessage(const char *fmt, ...);
	void SelfEveMail(const char *subject, const char *fmt, ...);
	void ChannelJoined(LSCChannel *chan);
	void ChannelLeft(LSCChannel *chan);

	//FunctorTimerQueue::TimerID Delay( uint32 time_in_ms, void (Client::* clientCall)() );
	//FunctorTimerQueue::TimerID Delay( uint32 time_in_ms, ClientFunctor **functor );

	/** character notification messages
	  */
	void OnCharNoLongerInStation();
	void OnCharNowInStation();
	
protected:
	void _SendPingRequest();
	
	void _ProcessNotification(PyPacket *packet);
	void _CheckSessionChange();
	
	void _ReduceDamage(Damage &d);
	
	//call stuff
	void _ProcessCallRequest(PyPacket *packet);
	void _SendCallReturn(PyPacket *req, PyRep **return_value, const char *channel = NULL);
	void _SendException(PyPacket *req, MACHONETERR_TYPE type, PyRep **payload);

	Character *m_char;

	PyServiceMgr &m_services;
	EVEPresentation m_net;
	Timer m_pingTimer;

	uint32 m_accountID;
	uint32 m_accountRole;
	uint32 m_gangRole;

	SystemManager *m_system;	//we do not own this

	std::set<LSCChannel *> m_channels;	//we do not own these.
	
	//this whole move system is a piece of crap:
	typedef enum {
		msIdle,
		msJump
	} _MoveState;
	void _postMove(_MoveState type, uint32 wait_ms=500);
	_MoveState m_moveState;
	Timer m_moveTimer;
	uint32 m_moveSystemID;
	GPoint m_movePoint;
	void _ExecuteJump();

	uint64 m_timeEndTrain;
	bool m_statusTrain;

private:
	//queues for destiny updates:
	std::vector<PyRep *> m_destinyEventQueue;	//we own these. These are events as used in OnMultiEvent
	std::vector<PyRep *> m_destinyUpdateQueue;	//we own these. They are the `update` which go into DoDestinyAction
	void _SendQueuedUpdates();

	//FunctorTimerQueue m_delayQueue;
	
	uint32 m_nextNotifySequence;
};


//simple functor for void calls.
//not needed right now
/*class SimpleClientFunctor : public ClientFunctor {
public:
	typedef void (Client::* clientCall)();
	SimpleClientFunctor(Client *c, clientCall call) : ClientFunctor(c), m_call(call) {}
	virtual ~SimpleClientFunctor() {}
	virtual void operator()() {
		(m_client->*m_call)();
	}
public:
	const clientCall m_call;
};*/

#endif
