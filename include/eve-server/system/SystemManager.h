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

#ifndef __SYSTEMMANAGER_H_INCL__
#define __SYSTEMMANAGER_H_INCL__

#include "system/BubbleManager.h"
#include "system/SystemDB.h"

//#define ONE_AU_IN_METERS 1.495978707e11     // 1 astronomical unit in meters
#define ONE_AU_IN_METERS 149598000000.0     // 1 astronomical unit in meters, per EVElopedia: http://wiki.eveonline.com/en/wiki/Astronomical_Unit
#define BASE_WARP_SPEED 3.0                 // base default max warp speed of 3.0 AU/s

class PyRep;
class PyDict;
class PyTuple;
class PyList;
class Client;
class NPC;
class InventoryItem;
class SystemEntity;
class SystemBubble;
class DoDestiny_SetState;


class SpawnManager;
class PyServiceMgr;

class SystemManager
//: public Inventory,
//  public InventoryItem
{
public:
	SystemManager(uint32 systemID, PyServiceMgr &svc);//, ItemData idata);
	virtual ~SystemManager();
	
	//bubble stuff:
	BubbleManager bubbles;
	
	uint32 GetID() const { return(m_systemID); }
	const std::string &GetName() const { return(m_systemName); }
	double GetWarpSpeed() const;
	
	bool BootSystem();
	
	bool Process();
	void ProcessDestiny();	//called once for each destiny second.

    bool BuildDynamicEntity(Client *who, const DBSystemDynamicEntity &entity);

	void AddClient(Client *who);
	void RemoveClient(Client *who);
	void AddNPC(NPC *who);
	void RemoveNPC(NPC *who);
	void AddEntity(SystemEntity *who);
	void RemoveEntity(SystemEntity *who);
	
	SystemEntity *get(uint32 entityID) const;
	
	void MakeSetState(const SystemBubble *bubble, DoDestiny_SetState &into) const;

	SystemDB *GetSystemDB() { return(&m_db); }
	const char * GetSystemSecurity() { return m_systemSecurity.c_str(); }

	ItemFactory &itemFactory() const;

    PyServiceMgr * GetServiceMgr() { return &m_services; }

    void AddItemToInventory(InventoryItemRef item);
    ShipRef GetShipFromInventory(uint32 shipID);
    void RemoveItemFromInventory(InventoryItemRef item);

protected:
    // Solar System Inventory Functions:
	//uint32 inventoryID() const { return itemID(); }
	//PyRep *GetItem() const { return new PyNone(); }
    //void AddItem(InventoryItemRef item);

    // Solar System Dynamic Inventory manager:
    SolarSystemRef m_solarSystemRef;    // we do not own this

    bool _LoadSystemCelestials();
	bool _LoadSystemDynamics();
	
	const uint32 m_systemID;
	std::string m_systemName;
	std::string m_systemSecurity;

	SystemDB m_db;
	PyServiceMgr &m_services;	//we do not own this
	SpawnManager *m_spawnManager;	//we own this, never NULL, dynamic to keep the knowledge down.
	
	//overall system entity lists:
	bool m_entityChanged;
	std::map<uint32, SystemEntity *> m_entities;	//we own these, but they are also referenced in m_bubbles
};





#endif


