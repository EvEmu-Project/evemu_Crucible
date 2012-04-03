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
    Author:     Zhur, Captnoord
*/

#include "EVEServerPCH.h"
#include "inventory/InventoryBound.h"

PyCallable_Make_InnerDispatcher(InventoryBound)

InventoryBound::InventoryBound( PyServiceMgr *mgr, Inventory &inventory, EVEItemFlags flag) :
    PyBoundObject(mgr), m_dispatch(new Dispatcher(this)), mInventory(inventory), mFlag(flag)
{
    _SetCallDispatcher(m_dispatch);
    
    m_strBoundObjectName = "InventoryBound";

    PyCallable_REG_CALL(InventoryBound, List)
    PyCallable_REG_CALL(InventoryBound, Add)
    PyCallable_REG_CALL(InventoryBound, MultiAdd)
    PyCallable_REG_CALL(InventoryBound, GetItem)
    PyCallable_REG_CALL(InventoryBound, ListStations)
    PyCallable_REG_CALL(InventoryBound, ReplaceCharges)
    PyCallable_REG_CALL(InventoryBound, MultiMerge)
    PyCallable_REG_CALL(InventoryBound, StackAll)
	PyCallable_REG_CALL(InventoryBound, StripFitting)
	PyCallable_REG_CALL(InventoryBound, DestroyFitting)
    PyCallable_REG_CALL(InventoryBound, SetPassword)
}

InventoryBound::~InventoryBound()
{
	delete m_dispatch;
}

PyResult InventoryBound::Handle_List(PyCallArgs &call) {
    //TODO: check to make sure we are allowed to list this inventory
    return mInventory.List( mFlag, call.client->GetCharacterID() );
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
    InventoryItemRef new_charge = mInventory.GetByID( args.itemID );
    if( !new_charge ) {
        codelog(SERVICE__ERROR, "%s: Unable to find charge %d", call.client->GetName(), args.itemID);
        return NULL;
    }

    if(new_charge->ownerID() != call.client->GetCharacterID()) {
        codelog(SERVICE__ERROR, "Character %u tried to load charge %u of character %u.", call.client->GetCharacterID(), new_charge->itemID(), new_charge->ownerID());
        return NULL;
    }

    if(new_charge->quantity() < (uint32)args.quantity) {
        codelog(SERVICE__ERROR, "%s: Item %u: Requested quantity (%d) exceeds actual quantity (%d), using actual.", call.client->GetName(), args.itemID, args.quantity, new_charge->quantity());
    } else if(new_charge->quantity() > (uint32)args.quantity) {
        new_charge = new_charge->Split(args.quantity);  // split item
        if( !new_charge ) {
            codelog(SERVICE__ERROR, "%s: Unable to split charge %d into %d", call.client->GetName(), args.itemID, args.quantity);
            return NULL;
        }
    }

    // new ref is consumed, we don't release it
    call.client->GetShip()->ReplaceCharges( (EVEItemFlags) args.flag, (InventoryItemRef)new_charge );

    return(new PyInt(1));
}


PyResult InventoryBound::Handle_ListStations( PyCallArgs& call )
{
    sLog.Debug( "InventoryBound", "Called ListStations stub." );

    util_Rowset rowset;

    rowset.header.push_back( "stationID" );
    rowset.header.push_back( "itemCount" );

    return rowset.Encode();
}

PyResult InventoryBound::Handle_GetItem(PyCallArgs &call) {
    return mInventory.GetItem();
}

PyResult InventoryBound::Handle_Add(PyCallArgs &call) {
    if( call.tuple->items.size() == 3 )
    {
        // TODO: Add comments here to describe what kind of client action results in having
        // to use the 'Call_Add_3' packet structure
        Call_Add_3 args;
        if(!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "Unable to decode arguments from '%s'", call.client->GetName());
            return NULL;
        }

        std::vector<int32> items;
        items.push_back(args.itemID);

        return _ExecAdd( call.client, items, args.quantity, (EVEItemFlags)args.flag );
    }
    else if( call.tuple->items.size() == 2 )
    {
        // TODO: Add comments here to describe what kind of client action results in having
        // to use the 'Call_Add_2' packet structure
        // * Moving cargo items from ship cargo bay to a container in space goes here
        Call_Add_2 args;
        //chances are its trying to transfer into a cargo container
        if(!args.Decode(&call.tuple))
        {
            codelog(SERVICE__ERROR, "Unable to decode arguments from '%s'", call.client->GetName());
            return NULL;
        }

        uint32 flag = 0;
        if( call.byname.find("flag") == call.byname.end() )
        {
            sLog.Debug( "InventoryBound::Handle_Add()", "Cannot find key 'flag' from call.byname dictionary." );
            flag = flagCargoHold;    // hard-code this since ship cargo to cargo container move flag since key 'flag' in client.byname does not exist
        }
        else
            flag = call.byname.find("flag")->second->AsInt()->value();

        if( call.byname.find("qty") == call.byname.end() )
        {
            sLog.Debug( "InventoryBound::Handle_Add()", "Cannot find key 'qty' from call.byname dictionary." );
            return NULL;
        }
        
        uint32 quantity = 0;
        if( call.byname.find("qty")->second->IsNone() )
            quantity = 1;
        else
            quantity = call.byname.find("qty")->second->AsInt()->value();

        std::vector<int32> items;
        items.push_back(args.itemID);

        return _ExecAdd( call.client, items, quantity, (EVEItemFlags)flag );
    }
    else if( call.tuple->items.size() == 1 )
    {
        // TODO: Add comments here to describe what kind of client action results in having
        // to use the 'Call_SingleIntegerArg' packet structure
        Call_SingleIntegerArg arg;
        if( !arg.Decode( &call.tuple ) )
        {
            codelog( SERVICE__ERROR, "Failed to decode arguments from '%s'.", call.client->GetName() );
            return NULL;
        }

        std::vector<int32> items;
        items.push_back( arg.arg );

        // no quantity given, pass 0 quantity so it assumes all.
        return _ExecAdd( call.client, items, 0, mFlag );
    }
    else
    {
        codelog( SERVICE__ERROR, "[Add] Unknown number of elements in a tuple: %lu.", call.tuple->items.size() );
        return NULL;
    }
}

PyResult InventoryBound::Handle_MultiAdd(PyCallArgs &call) {
    
	ShipRef ship = call.client->GetShip();
	uint32 typeID;
	uint32 powerSlot;
	uint32 useableSlot;

	
	if( call.tuple->items.size() == 3 )
    {
        // TODO: Add comments here to describe what kind of client action results in having
        // to use the 'Call_MultiAdd_3' packet structure
        Call_MultiAdd_3 args;
        if(!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "Unable to decode arguments");
            return NULL;
        }

		if((EVEItemFlags)args.flag == 0 )
		{

			//Get Item TypeID - this is bad
			InventoryDB::GetTypeID(args.itemIDs[0], typeID);

			//Get Range of slots for item
			InventoryDB::GetModulePowerSlotByTypeID(typeID, powerSlot);

			//Get open slots available on ship
			InventoryDB::GetOpenPowerSlots(powerSlot,ship , useableSlot);			
			
			//Set item flag to first useable open slot found
			args.flag = useableSlot;

		}
		
        //NOTE: They can specify "None" in the quantity field to indicate
        //their intention to move all... we turn this into a 0 for simplicity.

        return _ExecAdd( call.client, args.itemIDs, args.quantity, (EVEItemFlags)args.flag );
    }
    else if( call.tuple->items.size() == 2 )
    {
        // TODO: Add comments here to describe what kind of client action results in having
        // to use the 'Call_MultiAdd_2' packet structure
        Call_MultiAdd_2 args;
        if(!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "Unable to decode arguments");
            return NULL;
        }

        // no quantity given, assume 1
        return _ExecAdd( call.client, args.itemIDs, 1, mFlag );
    }
    else if( call.tuple->items.size() == 1 )
    {
        // TODO: Add comments here to describe what kind of client action results in having
        // to use the 'Call_SingleIntList' packet structure
        Call_SingleIntList args;
        if(!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "Unable to decode arguments");
            return NULL;
        }

        // no quantity given, assume 1
        return _ExecAdd( call.client, args.ints, 1, mFlag );
    }
    else
    {
        codelog( SERVICE__ERROR, "[MultiAdd] Unknown number of elements in a tuple: %lu.", call.tuple->items.size() );
        return NULL;
    }
}

PyResult InventoryBound::Handle_MultiMerge(PyCallArgs &call) {
    //Decode Args
    Inventory_CallMultiMerge elements;

    if(!elements.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Unable to decode elements");
        return NULL;
    }

    Inventory_CallMultiMergeElement element;

    std::vector<PyRep *>::const_iterator cur, end;
    cur = elements.MMElements->begin();
    end = elements.MMElements->end();
    for (; cur != end; cur++) {
        if(!element.Decode( *cur )) {
            codelog(SERVICE__ERROR, "Unable to decode element. Skipping.");
            continue;
        }

        InventoryItemRef stationaryItem = m_manager->item_factory.GetItem( element.stationaryItemID );
        if( !stationaryItem ) {
            _log(SERVICE__ERROR, "Failed to load stationary item %u. Skipping.", element.stationaryItemID);
            continue;
        }

        InventoryItemRef draggedItem = m_manager->item_factory.GetItem( element.draggedItemID );
        if( !draggedItem ) {
            _log(SERVICE__ERROR, "Failed to load dragged item %u. Skipping.", element.draggedItemID);
            continue;
        }

        stationaryItem->Merge( (InventoryItemRef)draggedItem, element.draggedQty );
    }

    return NULL;
}

PyResult InventoryBound::Handle_StackAll(PyCallArgs &call) {
    EVEItemFlags stackFlag = mFlag;

    if(call.tuple->items.size() != 0) {
        Call_SingleIntegerArg arg;
        if(!arg.Decode(&call.tuple)) {
            _log(SERVICE__ERROR, "Failed to decode args.");
            return NULL;
        }

        stackFlag = (EVEItemFlags)arg.arg;
    }

    //Stack Items contained in this inventory
    mInventory.StackAll(stackFlag, call.client->GetCharacterID());

    return NULL;
}

PyResult InventoryBound::Handle_StripFitting(PyCallArgs &call) {
	sLog.Debug("Server", "Called StripFitting Stub.");

	return NULL;
}

PyResult InventoryBound::Handle_DestroyFitting(PyCallArgs &call) {

	sLog.Debug("InventoryBound","Called DestroyFittings stub");

	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)){
		sLog.Error("Destroy Fittings","Failed to decode args.");
	}

	//get the actual item
	InventoryItemRef item = m_manager->item_factory.GetItem(args.arg);

	//remove the rig effects from the ship
	call.client->GetShip()->RemoveRig(item, mInventory.inventoryID());

	//move the item to the void or w/e
	call.client->MoveItem(item->itemID(), mInventory.inventoryID(), flagAutoFit);

	//delete the item
	item->Delete();

	return NULL;
}

PyResult InventoryBound::Handle_SetPassword(PyCallArgs &call) {
    // TODO
    uint32 item = 0;
    item++;

    return NULL;
}

PyRep *InventoryBound::_ExecAdd(Client *c, const std::vector<int32> &items, uint32 quantity, EVEItemFlags flag) {
    //If were here, we can try move all the items (validated)

	std::vector<int32>::const_iterator cur, end;
    cur = items.begin();
    end = items.end();
    for(; cur != end; cur++) {
        InventoryItemRef sourceItem = m_manager->item_factory.GetItem( *cur );
        if( !sourceItem ) {
            sLog.Error("_ExecAdd","Failed to load item %u. Skipping.", *cur);
            continue;
        }

		//Get old position
		EVEItemFlags old_flag = sourceItem->flag();

        //NOTE: a multi add can come in with quantity 0 to indicate "all"
        if( quantity == 0 )
            quantity = sourceItem->quantity();

        /*Check if its a simple item move or an item split qty is diff if its a
        split also multiple items cannot be split so the size() should be 1*/
        if( quantity != sourceItem->quantity() && items.size() == 1 )
        {
            InventoryItemRef newItem = sourceItem->Split(quantity);
            if( !newItem ) {
                sLog.Error("_ExecAdd", "Error splitting item %u. Skipping.", sourceItem->itemID() );
            }
            else
            {
                //Unlike the other validate item requests, fitting an item requires a skill check, which means passing the character
                // (This also allows for flagAutoFit when someone drags a module or a stack of modules onto the middle of the fitting
                // window and NOT onto a specific slot.  'flagAutoFit' means "put this module into which ever slot makes sense")
                if( (flag == flagAutoFit) )
                {
                    sLog.Error( "InventoryBound::_ExecAdd()", "ERROR: handling adding modules where flag = flagAutoFit not yet supported!!!" );
                    // uint32 newFlag = c->GetShip()->FindAvailableModuleSlot( newItem );
                    // c->GetShip()->AddItem( newFlag, newItem );
                }
				else if( (flag >= flagLowSlot0 && flag <= flagHiSlot7) || (flag >= flagRigSlot0 && flag <= flagRigSlot7) )
				{
					c->GetShip()->AddItem( flag, newItem );
				}
				else
				{
					mInventory.ValidateAddItem( flag, newItem );
				}

				if(old_flag >= flagLowSlot0 && old_flag <= flagHiSlot7)
				{

					c->GetShip()->RemoveItem( newItem, mInventory.inventoryID(), flag );

					//Create new item id return result
					Call_SingleIntegerArg result;
					result.arg = newItem->itemID();

					//Return new item result
					return result.Encode(); 

				} else if(old_flag >= flagRigSlot0 && old_flag <= flagRigSlot7) {
					
					c->GetShip()->RemoveRig( newItem, mInventory.inventoryID() );

				} else {

					//Move New item to its new location
					c->MoveItem(newItem->itemID(), mInventory.inventoryID(), flag); // properly refresh mModulesMgr

					//Create new item id return result
					Call_SingleIntegerArg result;
					result.arg = newItem->itemID();

					//Return new item result
					return result.Encode(); 
				}
            }
        }
        else
        {
			//Unlike the other validate item requests, fitting an item requires a skill check
            // (This also allows for flagAutoFit when someone drags a module or a stack of modules onto the middle of the fitting
            // window and NOT onto a specific slot.  'flagAutoFit' means "put this module into which ever slot makes sense")
            if( (flag == flagAutoFit) )
            {
                sLog.Error( "InventoryBound::_ExecAdd()", "ERROR: handling adding modules where flag = flagAutoFit not yet supported!!!" );
                // uint32 newFlag = c->GetShip()->FindAvailableModuleSlot( sourceItem );
                // c->GetShip()->AddItem( newFlag, sourceItem );
            }
			else if( (flag >= flagLowSlot0 && flag <= flagHiSlot7) || (flag >= flagRigSlot0 && flag <= flagRigSlot7) )
			{
				c->GetShip()->AddItem( flag, sourceItem );
			}
			else
			{
				mInventory.ValidateAddItem( flag, sourceItem );
			}

			if(old_flag >= flagLowSlot0 && old_flag <= flagHiSlot7)
			{
				c->GetShip()->RemoveItem( sourceItem, mInventory.inventoryID(), flag );
			} 
			else if(old_flag >= flagRigSlot0 && old_flag <= flagRigSlot7) 
			{
				//remove the rig
				c->GetShip()->RemoveRig(sourceItem, mInventory.inventoryID());
			} 
			else 
			{

				c->MoveItem(sourceItem->itemID(), mInventory.inventoryID(), flag);
			
			}
        }

		//update modules
		c->GetShip()->UpdateModules();
    }

    //Return Null if no item was created
    return NULL;
}
