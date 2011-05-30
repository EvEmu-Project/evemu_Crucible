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





#ifndef EVE_ENTITY_LIST_H
#define EVE_ENTITY_LIST_H

#include "threading/Mutex.h"
#include "utils/Singleton.h"

class Client;
class PyAddress;
class EVENotificationStream;
class SystemManager;
class DBcore;
class PyTuple;
class PyServiceMgr;

typedef enum {
	NOTIF_DEST__LOCATION,
	NOTIF_DEST__CORPORATION
} NotificationDestination;

class MulticastTarget {
public:
	//this object is a set of "or"s, matching any criteria is sufficient.
	std::set<uint32> characters;
	std::set<uint32> locations;
	std::set<uint32> corporations;
};

class EntityList
: public Singleton<EntityList>
{
public:
	EntityList();
	virtual ~EntityList();

	void UseServices(PyServiceMgr *svc) { m_services = svc; }

	typedef std::set<uint32> character_set;
	
	void Add(Client **client);
	
	void Process();

	Client *FindCharacter(uint32 char_id) const;
	Client *FindCharacter(const char *name) const;
	Client *FindByShip(uint32 ship_id) const;
	Client *FindAccount(uint32 account_id) const;
        void FindByStationID(uint32 stationID, std::vector<Client *> &result) const;
        void FindByRegionID(uint32 regionID, std::vector<Client *> &result) const;
	uint32 GetClientCount() const { return(uint32(m_clients.size())); }

	SystemManager *FindOrBootSystem(uint32 systemID);

	void Broadcast(const char *notifyType, const char *idType, PyTuple **payload) const;
	void Broadcast(const PyAddress &dest, EVENotificationStream &noti) const;
	void Multicast(const char *notifyType, const char *idType, PyTuple **payload, NotificationDestination target, uint32 target_id, bool seq=true);
	void Multicast(const char *notifyType, const char *idType, PyTuple **payload, const MulticastTarget &mcset, bool seq=true);
	void Multicast(const character_set &cset, const PyAddress &dest, EVENotificationStream &noti) const;
	void Multicast(const character_set &cset, const char *notifyType, const char *idType, PyTuple **payload, bool seq=true) const;
	void Unicast(uint32 charID, const char *notifyType, const char *idType, PyTuple **payload, bool seq=true);
	void GetClients(const character_set &cset, std::vector<Client *> &result) const;
	
protected:
	typedef std::list<Client *> client_list;
	client_list m_clients;
	typedef std::map<uint32, SystemManager *> system_list;
	system_list m_systems;

	Mutex mMutex;

	PyServiceMgr *m_services;	//we do not own this, only used for booting systems.
};

//Singleton
#define sEntityList \
	( EntityList::get() )


#endif

