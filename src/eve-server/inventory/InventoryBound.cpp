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
    Author:     Zhur, Captnoord
*/

#include "EVEServerPCH.h"
#include "inventory/InventoryBound.h"

PyCallable_Make_InnerDispatcher(InventoryBound)

InventoryBound::InventoryBound( PyServiceMgr *mgr, Inventory &inventory, EVEItemFlags flag) :
    PyBoundObject(mgr), m_dispatch(new Dispatcher(this)), mInventory(inventory), mFlag(flag)
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
    call.client->modules.ReplaceCharges( (EVEItemFlags) args.flag, (InventoryItemRef)new_charge );

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
        Call_Add_2 args;
        //chances are its trying to transfer into a cargo container
        if(!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "Unable to decode arguments from '%s'", call.client->GetName());
            return NULL;
        }

        std::vector<int32> items;
        items.push_back(args.itemID);

        return _ExecAdd( call.client, items, args.quantity, mFlag );
    }
    else if( call.tuple->items.size() == 1 )
    {
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
    if( call.tuple->items.size() == 3 )
    {
        Call_MultiAdd_3 args;
        if(!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "Unable to decode arguments");
            return NULL;
        }

        //NOTE: They can specify "None" in the quantity field to indicate
        //their intention to move all... we turn this into a 0 for simplicity.

        //TODO: should verify args.flag before casting!
        return _ExecAdd( call.client, args.itemIDs, args.quantity, (EVEItemFlags)args.flag );
    }
    else if( call.tuple->items.size() == 1 )
    {
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

PyRep *InventoryBound::_ExecAdd(Client *c, const std::vector<int32> &items, uint32 quantity, EVEItemFlags flag) {
    //If were here, we can try move all the items (validated)
    std::vector<int32>::const_iterator cur, end;
    cur = items.begin();
    end = items.end();
    for(; cur != end; cur++) {
        InventoryItemRef sourceItem = m_manager->item_factory.GetItem( *cur );
        if( !sourceItem ) {
            _log(SERVICE__ERROR, "Failed to load item %u. Skipping.", *cur);
            continue;
        }

        //NOTE: a multi add can come in with quantity 0 to indicate "all"
        if( quantity == 0 )
            quantity = sourceItem->quantity();

        /*Check if its a simple item move or an item split qty is diff if its a
        split also multiple items cannot be split so the size() should be 1*/
        if( quantity != sourceItem->quantity() && items.size() == 1 )
        {
            InventoryItemRef newItem = sourceItem->Split(quantity);
            if( !newItem ) {
                codelog(SERVICE__ERROR, "Error splitting item %u. Skipping.", sourceItem->itemID() );
            }
            else
            {
                mInventory.ValidateAddItem( flag, newItem );

                //Move New item to its new location
                c->MoveItem(newItem->itemID(), mInventory.inventoryID(), flag); // properly refresh modules

                //Create new item id return result
                Call_SingleIntegerArg result;
                result.arg = newItem->itemID();

                //Return new item result
                return result.Encode();
            }
        }
        else
        {
            //Its a move request
            mInventory.ValidateAddItem( flag, sourceItem );

            c->MoveItem(sourceItem->itemID(), mInventory.inventoryID(), flag);  // properly refresh modules
        }
    }

    //Return Null if no item was created
    return NULL;
}
