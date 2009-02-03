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

#include "EvemuPCH.h"
#include "inventory/InventoryBound.h"

PyCallable_Make_InnerDispatcher(InvBrokerService)

class InvBrokerBound : public PyBoundObject {
public:

	PyCallable_Make_Dispatcher(InvBrokerBound)
	
	InvBrokerBound(uint32 entityID, PyServiceMgr *mgr, InventoryDB *db)
	: PyBoundObject(mgr, "InvBrokerBound"),
	  m_dispatch(new Dispatcher(this)),
	  m_entityID(entityID),
	  m_db(db)
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(InvBrokerBound, GetInventoryFromId)
		PyCallable_REG_CALL(InvBrokerBound, GetInventory)
		PyCallable_REG_CALL(InvBrokerBound, SetLabel)
		PyCallable_REG_CALL(InvBrokerBound, TrashItems)
	}
	virtual ~InvBrokerBound() {}
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL(GetInventoryFromId)
	PyCallable_DECL_CALL(GetInventory)
	PyCallable_DECL_CALL(SetLabel)
	PyCallable_DECL_CALL(TrashItems)
	

protected:
	Dispatcher *const m_dispatch;

	uint32 m_entityID;
	
	InventoryDB *const m_db;
};

InvBrokerService::InvBrokerService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "invbroker"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);
	
	//PyCallable_REG_CALL(InvBrokerService, MachoBindObject)
}

InvBrokerService::~InvBrokerService() {
	delete m_dispatch;
}


PyBoundObject *InvBrokerService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	InvBroker_BindArgs args;
	//temp crap until I rework _CreateBoundObject's signature
	PyRep *t = bind_args->Clone();
	if(!args.Decode(&t)) {
		codelog(SERVICE__ERROR, "Failed to decode bind args from '%s'", c->GetName());
		return NULL;
	}
	_log(CLIENT__MESSAGE, "InvBrokerService bind request for:");
	args.Dump(CLIENT__MESSAGE, "    ");

	return(new InvBrokerBound(args.entityID, m_manager, &m_db));
}

//this is a view into the entire inventory item.
PyResult InvBrokerBound::Handle_GetInventoryFromId(PyCallArgs &call) {
	Call_TwoIntegerArgs args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return (NULL);
	}
    //bool passive = (args.arg2 != 0);	//no idea what this is for.
	
	InventoryItem *item = m_manager->item_factory.Load(args.arg1, true);
	if(item == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to load item %lu", call.client->GetName(), args.arg1);
		return (NULL);
	}
	
	//we just bind up a new inventory object and give it back to them.
	InventoryBound *ib = new InventoryBound(item, flagAutoFit, m_manager, m_db);
	PyRep *result = m_manager->BindObject(call.client, ib);

	return(result);
}

//this is a view into an inventory item using a specific flag.
PyResult InvBrokerBound::Handle_GetInventory(PyCallArgs &call) {
	Inventory_GetInventory args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode arguments");
		return NULL;
	}
	PyRep *result = NULL;
	
	EVEItemFlags flag;
	switch(args.container) {
	case containerWallet:
		flag = flagWallet;
		break;
	case containerCharacter:
		flag = flagSkill;
		break;
	case containerHangar:
		flag = flagHangar;
		break;
		
	case containerGlobal:
	case containerSolarSystem:
	case containerScrapHeap:
	case containerFactory:
	case containerBank:
	case containerRecycler:
	case containerOffices:
	case containerStationCharacters:
	case containerCorpMarket:
	default:
		codelog(SERVICE__ERROR, "Unhandled container type %d", args.container);
		return NULL;
	}
	
	InventoryItem *item = m_manager->item_factory.Load(m_entityID, true);
	if(item == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to load item %lu", call.client->GetName(), m_entityID);
		return (NULL);
	}

	_log(SERVICE__MESSAGE, "%s: Binding inventory object for %s for container %lu with flag %lu", GetName(), call.client->GetName(), m_entityID, flag);
	
	//we just bind up a new inventory object and give it back to them.
	InventoryBound *ib = new InventoryBound(item, flag, m_manager, m_db);
	result = m_manager->BindObject(call.client, ib);
	
	return(result);
}

PyResult InvBrokerBound::Handle_SetLabel(PyCallArgs &call) {
	CallSetLabel args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode arguments");
		return NULL;
	}
	
	InventoryItem *item = m_manager->item_factory.Load(args.itemID, false);
	if(item == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to load item %lu", call.client->GetName(), args.itemID);
		return (NULL);
	}
	
	if(item->ownerID() != call.client->GetCharacterID()) {
		_log(SERVICE__ERROR, "Character %lu tried to rename item %lu of character %lu.", call.client->GetCharacterID(), item->itemID(), item->ownerID());
		item->Release();
		return NULL;
	}

	item->Rename(args.itemName.c_str());
	item->Release();
	
	//do we need to send some sort of update?
	
	return NULL;
}

PyResult InvBrokerBound::Handle_TrashItems(PyCallArgs &call) {
	Call_TrashItems args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode arguments");
		return(new PyRepList());
	}

	InventoryItem *item;
	std::vector<uint32>::const_iterator cur, end;
	cur = args.items.begin();
	end = args.items.end();
	for(; cur != end; cur++) {
		item = m_manager->item_factory.Load(*cur, false);
		if(item == NULL) 
			codelog(SERVICE__ERROR, "%s: Unable to load item %lu to delete it. Skipping.", call.client->GetName(), *cur);
		else if(call.client->GetCharacterID() != item->ownerID()) {
			codelog(SERVICE__ERROR, "%s: Tried to trash item %lu which is not yours. Skipping.", call.client->GetName(), *cur);
			item->Release();
		} else if(item->locationID() != args.locationID) {
			codelog(SERVICE__ERROR, "%s: Item %lu is not in location %lu. Skipping.", call.client->GetName(), *cur, args.locationID);
			item->Release();
		} else
			item->Delete();
	}
	return(new PyRepList());
}
