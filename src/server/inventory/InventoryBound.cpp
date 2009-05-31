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
	Author:		Zhur, Captnoord
*/

#include "EvemuPCH.h"
#include "inventory/InventoryBound.h"

PyCallable_Make_InnerDispatcher(InventoryBound)

InventoryBound::InventoryBound(
	PyServiceMgr *mgr,
	InventoryItem *item,
	EVEItemFlags flag)
: PyBoundObject(mgr),
  m_dispatch(new Dispatcher(this)),
  m_item(item),
  m_flag(flag)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(InventoryBound, List)
	PyCallable_REG_CALL(InventoryBound, Add)
	PyCallable_REG_CALL(InventoryBound, MultiAdd)
	PyCallable_REG_CALL(InventoryBound, GetItem)
	PyCallable_REG_CALL(InventoryBound, ListStations)
	PyCallable_REG_CALL(InventoryBound, ReplaceCharges)
	PyCallable_REG_CALL(InventoryBound, MultiMerge)
	PyCallable_REG_CALL(InventoryBound, StackAll)
}

InventoryBound::~InventoryBound() {
	m_item->DecRef();
}

PyResult InventoryBound::Handle_List(PyCallArgs &call) {
	PyRep *result = NULL;

	//TODO: check to make sure we are allowed to list this container

	//this is such crap, need better logic.
	/*uint32 list_flag = flagCargoHold;
	if(m_entityID == call.client->GetLocationID())
		list_flag = flagInventory;*/

	result = m_item->GetInventoryRowset(m_flag, call.client->GetCharacterID());
	if(result == NULL)
		result = new PyRepNone();

	return(result);
}

PyResult InventoryBound::Handle_ReplaceCharges(PyCallArgs &call) {
	Inventory_CallReplaceCharges args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode arguments");
		return NULL;
	}

	//validate flag.
	if(args.flag < flagSlotFirst || args.flag > flagSlotLast) {
		codelog(SERVICE__ERROR, "%s: Invalid flag %d", call.client->GetName(), args.flag);
		return NULL;
	}

	// returns new ref
	InventoryItem *new_charge = m_item->GetByID(args.itemID, true);
	if(new_charge == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to find charge %d", call.client->GetName(), args.itemID);
		return NULL;
	}

	if(new_charge->ownerID() != call.client->GetCharacterID()) {
		codelog(SERVICE__ERROR, "Character %u tried to load charge %u of character %u.", call.client->GetCharacterID(), new_charge->itemID(), new_charge->ownerID());
		new_charge->DecRef();
		return NULL;
	}

	if(new_charge->quantity() < args.quantity) {
		codelog(SERVICE__ERROR, "%s: Item %u: Requested quantity (%d) exceeds actual quantity (%d), using actual.", call.client->GetName(), args.itemID, args.quantity, new_charge->quantity());
	} else if(new_charge->quantity() > args.quantity) {
		InventoryItem *new_charge_split = new_charge->Split(args.quantity);	// get new ref on a splitted item
		new_charge->DecRef();	// release the old ref
		new_charge = new_charge_split;	// copy the new ref
		if(new_charge == NULL) {
			codelog(SERVICE__ERROR, "%s: Unable to split charge %d into %d", call.client->GetName(), args.itemID, args.quantity);
			return NULL;
		}
	}

	// new ref is consumed, we don't release it
	call.client->modules.ReplaceCharges((EVEItemFlags) args.flag, new_charge);

	return(new PyRepInteger(1));
}


PyResult InventoryBound::Handle_ListStations(PyCallArgs &call) {
	codelog(SERVICE__ERROR, "Unimplemented.");

	util_Rowset rowset;

	rowset.header.push_back("stationID");
	rowset.header.push_back("itemCount");

	return(rowset.FastEncode());
}

PyResult InventoryBound::Handle_GetItem(PyCallArgs &call) {
	return m_item->GetItemRow();
}

PyResult InventoryBound::Handle_Add(PyCallArgs &call) {
	if(call.tuple->items.size() == 3) {
		Inventory_CallAdd args;
		if(!args.Decode(&call.tuple)) {
			codelog(SERVICE__ERROR, "Unable to decode arguments from '%s'", call.client->GetName());
			return NULL;
		}

		std::vector<uint32> items;
		items.push_back(args.itemID);

		return(_ExecAdd(call.client, items, args.quantity, (EVEItemFlags)args.flag));
	} else if(call.tuple->items.size() == 2) {
		Inventory_CallAddCargoContainer args;
		//chances are its trying to transfer into a cargo container
		if(!args.Decode(&call.tuple)) {
			codelog(SERVICE__ERROR, "Unable to decode arguments from '%s'", call.client->GetName());
			return NULL;
		}

		std::vector<uint32> items;
		items.push_back(args.itemID);

		return(_ExecAdd(call.client, items, args.quantity, flagAutoFit));
	} else {
		codelog(SERVICE__ERROR, "[Add] Unknown number of args in tuple");
		return NULL;
	}
}

PyResult InventoryBound::Handle_MultiAdd(PyCallArgs &call) {
	if(call.tuple->items.size() == 1) {
		Call_SingleIntList args;
		if(!args.Decode(&call.tuple)) {
			codelog(SERVICE__ERROR, "Unable to decode arguments");
			return NULL;
		}

		//not sure about what flag to use here...
		return(_ExecAdd(call.client, args.ints, 1, flagHangar));
	} else {
		Inventory_CallMultiAdd args;
		if(!args.Decode(&call.tuple)) {
			codelog(SERVICE__ERROR, "Unable to decode arguments");
			return NULL;
		}

		//NOTE: They can specify "None" in the quantity field to indicate
		//their intention to move all... we turn this into a 0 for simplicity.

		//TODO: should verify args.flag before casting!
		return(_ExecAdd(call.client, args.itemIDs, args.quantity, (EVEItemFlags)args.flag));
	}

	return NULL;
}

PyResult InventoryBound::Handle_MultiMerge(PyCallArgs &call) {
	//Decode Args
	Inventory_CallMultiMerge elements;

	if(!elements.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Unable to decode elements");
		return NULL;
	}	

	Inventory_CallMultiMergeElement element;
	PyRep* codedElement;

	std::vector<PyRep *>::const_iterator cur, end;
	cur = elements.MMElements.items.begin();
	end = elements.MMElements.items.end();

	for (; cur != end; cur++) {
		codedElement = *cur;

		if(!element.Decode(&codedElement)) {
			codelog(SERVICE__ERROR, "Unable to decode element. Skipping.");
			continue;
		}

		InventoryItem *stationaryItem = m_manager->item_factory.GetItem(element.stationaryItemID, false);
		if(stationaryItem == NULL) {
			_log(SERVICE__ERROR, "Failed to load stationary item %u. Skipping.", element.stationaryItemID);
			continue;
		}

		InventoryItem *draggedItem = m_manager->item_factory.GetItem(element.draggedItemID, false);
		if(draggedItem == NULL) {
			_log(SERVICE__ERROR, "Failed to load dragged item %u. Skipping.", element.draggedItemID);
			stationaryItem->DecRef();
			continue;
		}

		if(!stationaryItem->Merge(draggedItem, element.draggedQty))
			// if Merge failed, draggedItem ref wasn't relased ...
			draggedItem->DecRef();

		stationaryItem->DecRef();
	}

	elements.MMElements.items.clear();
	return NULL;
}

PyResult InventoryBound::Handle_StackAll(PyCallArgs &call) {
	EVEItemFlags stackFlag = flagHangar;

	if(call.tuple->items.size() != 0) {
		Call_SingleIntegerArg arg;
		if(!arg.Decode(&call.tuple)) {
			_log(SERVICE__ERROR, "Failed to decode args.");
			return NULL;
		}

		stackFlag = (EVEItemFlags)arg.arg;
	}

	//Stack Items contained in this inventory
	m_item->StackContainedItems(stackFlag, call.client->GetCharacterID());

	return NULL;
}

void InventoryBound::_ValidateAdd( Client *c, const std::vector<uint32> &items, uint32 quantity, EVEItemFlags flag) {
	double totalVolume = 0.0;

	std::vector<uint32>::const_iterator cur, end;
	cur = items.begin();
	end = items.end();
	for(; cur != end; cur++) {
		InventoryItem *sourceItem = m_manager->item_factory.GetItem((*cur), true);
		if(sourceItem == NULL) {
			_log(SERVICE__ERROR, "Failed to load item %u. Skipping.", *cur);
			continue;
		}

		//If hold already contains this item then we can ignore remaining space
		if ((!m_item->Contains(sourceItem)) || (m_item->flag() != flag))
		{
			//Add volume to totalVolume
			if( items.size() > 1)
			{
				totalVolume += (sourceItem->quantity() * sourceItem->volume());
			}
			else
			{
				totalVolume += (quantity * sourceItem->volume());
			}
		}

		//TODO: check to see if they are allowed to move this item
		if( (flag == flagDroneBay) && (sourceItem->categoryID() != EVEDB::invCategories::Drone) )
		{
			//Can only put drones in drone bay
			//Return ErrorResponse
			sourceItem->DecRef();
			throw(PyException(MakeUserError("ItemCannotBeInDroneBay")));
		}


		//DecRef the sourceItem 
		sourceItem->DecRef();
	}	

	//Check total volume used size
	if( flag != flagHangar)
	{
		double remainingCapacity = m_item->GetRemainingCapacity(flag);

		if( totalVolume > remainingCapacity )
		{
			//The moving item is too big to fit into dest space
			//Log Error
			if( items.size() > 1)
			{
				_log(ITEM__ERROR, "Cannot Perform Add. Items are too large (%f m3) to fit into destination %u (%f m3 capacity remaining)", totalVolume, m_item->itemID(), remainingCapacity); 
			}
			else
			{
				_log(ITEM__ERROR, "Cannot Perform Add. Item %u is too large (%f m3) to fit into destination %u (%f m3 capacity remaining)", items[0], totalVolume, m_item->itemID(), remainingCapacity);
			}
			std::map<std::string, PyRep *> args;
			args["available"] = new PyRepReal(remainingCapacity);
			args["volume"] = new PyRepReal(totalVolume);
			throw(PyException(MakeUserError("NotEnoughCargoSpace", args)));
		}
	}
}


PyRep *InventoryBound::_ExecAdd(Client *c, const std::vector<uint32> &items, uint32 quantity, EVEItemFlags flag) {
	bool fLoadoutRequest = false;

	//TODO: handle auto-fit flag
	if(flag == flagAutoFit)
		flag = flagHangar;


	//Is this a loadout request
	if( flag >= flagSlotFirst && flag <= flagSlotLast)
	{
		//Validate a loadout request
		fLoadoutRequest = true;

	}
	else
	{
		//Make sure all items can be moved successfully will be ok
		_ValidateAdd(c, items, quantity, flag);
	}

	//If were here, we can try move all the items (validated)
	std::vector<uint32>::const_iterator cur, end;
	cur = items.begin();
	end = items.end();
	for(; cur != end; cur++) {
		InventoryItem *sourceItem = m_manager->item_factory.GetItem((*cur), true);
		if(sourceItem == NULL) {
			_log(SERVICE__ERROR, "Failed to load item %u. Skipping.", *cur);
			continue;
		}

		//NOTE: a multiadd can come in with quantity 0 to indicate "all"

		/*Check if its a simple item move or an item split qty is diff if its a 
		split also multiple items cannot be split so the size() should be 1*/
		if( (quantity != 0) && (quantity != sourceItem->quantity()) && (items.size() == 1) )
		{
			InventoryItem *newItem = sourceItem->Split(quantity);

			//Move New item to its new location
			if( newItem == NULL )
			{
				codelog(SERVICE__ERROR, "Error spawning an item of type %u", sourceItem->typeID());
			}
			else
			{
				c->MoveItem(newItem->itemID(), m_item->itemID(), flag);	// properly refresh modules
				if(fLoadoutRequest == true)
				{
					newItem->ChangeSingleton( true );
				}

				//Create new item id return result
				Call_SingleIntegerArg result;
				result.arg = newItem->itemID();

				//DecRef Items
				sourceItem->DecRef();
				newItem->DecRef();

				//Return new item result
				return( result.FastEncode() );
			}
		}
		else
		{
			//Its a move request
			c->MoveItem(sourceItem->itemID(), m_item->itemID(), flag);	// properly refresh modules
			if(fLoadoutRequest == true)
			{
				sourceItem->ChangeSingleton( true );
			}

		}

		//DecRef the source Item PTR, we dont need it anymore
		sourceItem->DecRef();


	}
	//Return Null if no item was created
	return NULL;
}
