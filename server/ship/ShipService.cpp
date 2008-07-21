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



#include "ShipService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "../PyBoundObject.h"
#include "../inventory/InventoryItem.h"
#include "../inventory/ItemFactory.h"
#include "../system/SystemManager.h"
#include "../NPC.h"
#include "../ship/DestinyManager.h"

#include "../packets/General.h"
#include "../packets/Inventory.h"
#include "../packets/Destiny.h"

PyCallable_Make_InnerDispatcher(ShipService)



class ShipBound
: public PyBoundObject {
public:

	PyCallable_Make_Dispatcher(ShipBound)
	
	ShipBound(PyServiceMgr *mgr, ShipDB *db)
	: PyBoundObject(mgr, "ShipBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(ShipBound, Board)
		PyCallable_REG_CALL(ShipBound, Undock)
		PyCallable_REG_CALL(ShipBound, AssembleShip)
		PyCallable_REG_CALL(ShipBound, Drop)
		PyCallable_REG_CALL(ShipBound, ScoopDrone)
		PyCallable_REG_CALL(ShipBound, Jettison)
		//PyCallable_REG_CALL(ShipBound, )
	}
	virtual ~ShipBound() {}
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL(Board)
	PyCallable_DECL_CALL(Undock)
	PyCallable_DECL_CALL(AssembleShip)
	PyCallable_DECL_CALL(Drop)
	PyCallable_DECL_CALL(ScoopDrone)
	PyCallable_DECL_CALL(Jettison)
	PyCallable_DECL_CALL(Eject)
	//PyCallable_DECL_CALL()

protected:
	ShipDB *const m_db;
	Dispatcher *const m_dispatch;
};


ShipService::ShipService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "ship"),
m_dispatch(new Dispatcher(this)),
m_db(db)
{
	_SetCallDispatcher(m_dispatch);

//	PyCallable_REG_CALL(ShipService, )
}

ShipService::~ShipService() {
	delete m_dispatch;
}



PyBoundObject *ShipService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	_log(CLIENT__MESSAGE, "ShipService bind request for:");
	bind_args->Dump(CLIENT__MESSAGE, "    ");
	
	return(new ShipBound(m_manager, &m_db));
}


PyCallResult ShipBound::Handle_Board(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return(NULL);
	}

	if(call.client->IsInSpace()) {
		//TODO: handle the logistics of this!
		codelog(CLIENT__ERROR, "%s: Boarding a ship in space is not yet implemented!!!!", call.client->GetName());
	}
	
	call.client->BoardShip(args.arg);
	
	return(NULL);
}


PyCallResult ShipBound::Handle_Undock(PyCallArgs &call) {
	Call_SingleBoolArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Failed to decode arguments");
		//TODO: throw exception
		return(NULL);
	}
	
	//int ignoreContraband = args.arg;
	
	GPoint position;
	if(!m_db->GetStationDockPosition(call.client->GetLocationID(), position)) {
		_log(SERVICE__ERROR, "%s: Failed to query location of station %lu for undock.", call.client->GetLocationID());
		//TODO: throw exception
		return(NULL);
	}

	
	_log(CLIENT__ERROR, "undock not fully implemented!");

	//send an OnItemChange notification
	// tuple: 
	// 	util.Row for item, 
	//		customInfo=Undocking:(insert station ID)
	// 	dict:
	// 		3->(insert station ID)
	// 		4->4

	//Update the custom info field.
	char ci[256];
	snprintf(ci, sizeof(ci), "Undocking:%lu", call.client->GetLocationID());
	call.client->Ship()->SetCustomInfo(ci);
	
	//do session change...
	call.client->MoveToLocation(call.client->GetSystemID(), position);
	call.client->UndockingIntoSpace();

//revert custom info, for testing.
	call.client->Ship()->SetCustomInfo(NULL);
	
	//send OnCharNoLongerInStation

	//should get a stationSvc.GetSolarSystem(solarsystemID)
	
	return(NULL);
}

PyCallResult ShipBound::Handle_AssembleShip(PyCallArgs &call) {
	//TODO: Return correct values
	Call_AssembleShip args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Failed to decode arguments");
		//TODO: throw exception
		return(NULL);
	}

	//Start sloppy implementation
	InventoryItem* item;
	item = m_manager->item_factory->Load( args.items.front(), false);
	item->ChangeSingleton(true, true);
	item->Release();
	//TODO: something...
	return(NULL);
}

PyCallResult ShipBound::Handle_Drop(PyCallArgs &call) {
	Call_SingleIntList args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Failed to decode arguments");
		//TODO: throw exception
		return(NULL);
	}
	
	if(!IsSolarSystem(call.client->GetLocationID())) {
		_log(SERVICE__ERROR, "%s: Trying to drop items when not in space!", call.client->GetName());
		return(new PyRepList());
	}

	Call_SingleIntList successfully_dropped;

	std::vector<uint32>::const_iterator cur, end;
	cur = args.ints.begin();
	end = args.ints.end();
	for(; cur != end; cur++) {
		InventoryItem *item = m_manager->item_factory->Load(*cur, false);
		if(item == NULL) {
			_log(SERVICE__ERROR, "%s: Unable to find item %lu to drop.", call.client->GetName(), *cur);
			continue;
		}
		
		//verify that this item is in fact in the player's ship.
		if(item->locationID() != call.client->GetShipID()) {
			_log(SERVICE__ERROR, "%s: Item %lu is not in our ship (%lu), it is in %lu. Not dropping.", call.client->GetName(), *cur, call.client->GetShipID(), item->locationID());
			continue;
		}
		//TODO: check ownership?
		
		if(item->flag() == flagDroneBay && item->categoryID() == EVEDB::invCategories::Drone) {
			if(call.client->LaunchDrone(item))
				successfully_dropped.ints.push_back(item->itemID());
		} else {
			//TODO: drop the crap into a can in space.
			codelog(SERVICE__ERROR, "%s: Dropping non-drones is not yet implemented.", call.client->GetName());
		}
		item->Release();
	}
	
	return(successfully_dropped.Encode());
}

PyCallResult ShipBound::Handle_ScoopDrone(PyCallArgs &call) {
	Call_SingleIntList args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Failed to decode arguments");
		//TODO: throw exception
		return(NULL);
	}
	
	if(!IsSolarSystem(call.client->GetLocationID())) {
		_log(SERVICE__ERROR, "%s: Trying to drop items when not in space!", call.client->GetName());
		return(NULL);
	}
	
	std::vector<uint32>::const_iterator cur, end;
	cur = args.ints.begin();
	end = args.ints.end();
	for(; cur != end; cur++) {
		SystemManager *sm = call.client->GetSystem();
		SystemEntity *npc = sm->get(*cur);
		if(npc == NULL) {
			_log(SERVICE__ERROR, "%s: Unable to find drone %lu to scoop.", call.client->GetName(), *cur);
			continue;
		}

		//TODO: check to see that this is really a drone.
		//TODO: check ownership/control.
		//TODO: check range.
		//TODO: check drone bay capacity.
		//TODO: check skills?
		
		InventoryItem *item = npc->Item();
		item->Move(call.client->GetShipID(), flagDroneBay);
		
		//I am not happy with this memory management model...
		delete npc;	//should remove itself from everything.
	}
	
	return(NULL);
}

PyCallResult ShipBound::Handle_Jettison(PyCallArgs &call) {
	//TODO: Add correct return values
	
	Call_SingleIntList args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Failed to decode arguments");
		//TODO: throw exception
		return(NULL);
	}

	if(!IsSolarSystem(call.client->GetLocationID())) {
		_log(SERVICE__ERROR, "%s: Trying to jettision items when not in space!", call.client->GetName());
		return(NULL);
	}
	

	//spawn cargo container
	InventoryItem *cargoItem;

	cargoItem = m_manager->item_factory->SpawnSingleton(
		23, //23 = Cargo Container
		call.client->GetCharacterID(),	//owner
		call.client->GetLocationID(),
		flagAutoFit
		);

	if(cargoItem == NULL) {
		call.client->SendErrorMsg("Unable to spawn a container for jettison.");
		return(NULL);
	}
	
	//Get location of our ship
	//GPoint loc(call.client->GetPosition());
	//NPC *cargoNpc = new NPC(call.client->GetSystem(), m_manager, cargoItem, call.client->GetCorporationID(), loc);
	
	//args contains id's of items to jettison
	std::vector<uint32>::const_iterator cur, end;
	cur = args.ints.begin();
	end = args.ints.end();

	for(; cur != end; cur++) {
		//Move item into cargo Container
		call.client->MoveItem(*cur, cargoItem->itemID(), flagAutoFit);
	}
	
	//TODO: properly add the item to the system manager and send the AddBall
	

	//release our ref to cargoItem
	cargoItem->Release();
	//add cargo container to system
	//cargoNpc->Destiny()->SendAddBall();
	//TODO: Send notification SFX effects.jettison
	return(NULL);
}


PyCallResult ShipBound::Handle_Eject(PyCallArgs &call) {
	//no arguments.

	//spawn capsule (inside ship, flagCapsule, singleton)
	//change location of capsule from old ship to systemID (flag 56->0)

	//send session change for shipID change
	
	return(NULL);
	//things which came after the return (but prolly do not have to...

	//DoDestinyUpdate
		//addball for only the capsule.
	
	//DoDestinyUpdate
		//AddBalls:
			//old ship (STOP, w/ship and mass)
			//capsule (STOP, w/ship and mass)
			//and our newbie ship (STOP, w/ship and mass)
		//a bunch of OnModuleAttributeChange and OnGodmaShipEffect for old ship and capsule

	//DoDestinyUpdate
		//effects.Jettison
		//SetBallInteractive (old ship, false)
		//OnSlimItemChange (old ship, SlimItem)
		//SetBallInteractive (capsule, true)
		//OnSlimItemChange (capsule, SlimItem)
		//SetMaxSpeed (capsule)
}


























