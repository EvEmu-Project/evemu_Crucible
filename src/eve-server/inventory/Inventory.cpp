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
    Author:     Bloody.Rabbit
*/

#include "EVEServerPCH.h"

/*
 * Inventory
 */

Inventory::Inventory() : mContentsLoaded(false) {}
Inventory::~Inventory() {}

Inventory *Inventory::Cast(InventoryItemRef item)
{
    if( !item )
        return NULL;
    switch( item->categoryID() )
    {
        //! TODO: not handled.
        case EVEDB::invCategories::_System:
        case EVEDB::invCategories::Owner:
        case EVEDB::invCategories::Celestial:
        case EVEDB::invCategories::Station:
        case EVEDB::invCategories::Material:
        case EVEDB::invCategories::Accessories:
        case EVEDB::invCategories::Module:
        case EVEDB::invCategories::Charge:
        case EVEDB::invCategories::Blueprint:
        case EVEDB::invCategories::Trading:
        case EVEDB::invCategories::Entity:
        case EVEDB::invCategories::Bonus:
        case EVEDB::invCategories::Skill:
        case EVEDB::invCategories::Commodity:
        case EVEDB::invCategories::Drone:
        case EVEDB::invCategories::Implant:
        case EVEDB::invCategories::Deployable:
        case EVEDB::invCategories::Reaction:
        case EVEDB::invCategories::Asteroid:
            sLog.Warning("Inventory", "unhandled item categoryID used on cast");
            break;
        case EVEDB::invCategories::Structure:
            return StructureRef::StaticCast( item ).get();
            break;
        case EVEDB::invCategories::Ship:
            return ShipRef::StaticCast( item ).get();
            break;
    }

    switch( item->groupID() )
    {
        case EVEDB::invGroups::Wreck:
        case EVEDB::invGroups::Secure_Cargo_Container:
        case EVEDB::invGroups::Audit_Log_Secure_Container:
        case EVEDB::invGroups::Cargo_Container:
        case EVEDB::invGroups::Freight_Container:
        case EVEDB::invGroups::Spawn_Container:
            return CargoContainerRef::StaticCast( item ).get();
        case EVEDB::invGroups::Station:         return StationRef::StaticCast( item ).get();
        case EVEDB::invGroups::Character:       return CharacterRef::StaticCast( item ).get();
        case EVEDB::invGroups::Solar_System:    return SolarSystemRef::StaticCast( item ).get();
    }

    // maybe add extra debug info on what for type or item.
    sLog.Error("Inventory::Cast()", "Item cast not supported for item typeID = %u, groupID = %u, categoryID = %u", item->typeID(), item->groupID(), item->categoryID() );
    return NULL;
}

bool Inventory::LoadContents(ItemFactory &factory)
{
    // check if the contents has already been loaded...
    if( ContentsLoaded() )
    {
        return true;
    }

    sLog.Debug("Inventory", "Recursively loading contents of inventory %u", inventoryID() );

    //load the list of items we need
    std::vector<uint32> items;
    if( !GetItems( factory, items ) )
    {
        sLog.Error("Inventory", "Failed  to get items of %u", inventoryID() );
        return false;
    }

    //Now get each one from the factory (possibly recursing)
    ItemData into;
    uint32 characterID;
    uint32 corporationID;
    uint32 locationID;
    std::vector<uint32>::iterator cur, end;
    cur = items.begin();
    end = items.end();
    for(; cur != end; cur++)
    {
        // Each "cur" item should be checked to see if they are "owned" by the character connected to this client,
        // and if not, then do not "get" the entire contents of this for() loop for that item, except in the case that
        // this item is located in space or belongs to this character's corporation:
        factory.db().GetItem( *cur, into );
        if( factory.GetUsingClient() != NULL )
        {
            characterID = factory.GetUsingClient()->GetCharacterID();
            corporationID = factory.GetUsingClient()->GetCorporationID();
            locationID = factory.GetUsingClient()->GetLocationID();
        }
        else
            sLog.Error( "Inventory::LoadContents()", "Failed to resolve pointer to Client object currently using the ItemFactory." );
        if( (into.ownerID == characterID) || (characterID == 0) || (into.ownerID == corporationID) || (into.locationID == locationID) )
        {
            // Continue to GetItem() if the client calling this is owned by the character that owns this item
            // --OR--
            // The characterID == 0, which means this is attempting to load the character of this client for the first time.

            InventoryItemRef i = factory.GetItem( *cur );
            if( !i )
            {
                sLog.Error("Inventory::LoadContents()", "Failed to load item %u contained in %u. Skipping.", *cur, inventoryID() );
                continue;
            }

            AddItem( i );
        }
    }

    mContentsLoaded = true;
    return true;
}

void Inventory::DeleteContents(ItemFactory &factory)
{
    LoadContents( factory );

    std::map<uint32, InventoryItemRef>::iterator cur, end;
    cur = mContents.begin();
    end = mContents.end();
    for(; cur != end; )
    {
        // Our "cur" iterator becomes invalid once RemoveItem
        // for its item is called, so we need to increment it
        // before calling Delete().
        InventoryItemRef i = cur->second;
        cur++;

        i->Delete();
    }

    mContents.clear();
}

CRowSet* Inventory::List( EVEItemFlags _flag, uint32 forOwner ) const
{
	DBRowDescriptor* header = new DBRowDescriptor;
	header->AddColumn( "itemID",     DBTYPE_I4 );
	header->AddColumn( "typeID",     DBTYPE_I2 );
	header->AddColumn( "ownerID",    DBTYPE_I4 );
	header->AddColumn( "locationID", DBTYPE_I4 );
	header->AddColumn( "flagID",     DBTYPE_I2 );
	header->AddColumn( "singleton",  DBTYPE_BOOL );
	header->AddColumn( "quantity",   DBTYPE_I4 );
	header->AddColumn( "groupID",    DBTYPE_I2 );
	header->AddColumn( "categoryID", DBTYPE_UI1 );
	header->AddColumn( "customInfo", DBTYPE_STR );
	header->AddColumn( "stacksize" , DBTYPE_I4 );




    CRowSet* rowset = new CRowSet( &header );
    List( rowset, _flag, forOwner );
    return rowset;
}

void Inventory::List( CRowSet* into, EVEItemFlags _flag, uint32 forOwner ) const
{
    //there has to be a better way to build this...
    std::map<uint32, InventoryItemRef>::const_iterator cur, end;
    cur = mContents.begin();
    end = mContents.end();
    for(; cur != end; cur++)
    {
        InventoryItemRef i = cur->second;

        if(    ( i->flag() == _flag       || _flag == flagAnywhere )
            && ( i->ownerID() == forOwner || forOwner == 0 ) )
        {
			PyPackedRow* row = into->NewRow();
			i->GetItemRow( row );
        }
    }
}

InventoryItemRef Inventory::FindFirstByFlag(EVEItemFlags _flag) const
{
    std::map<uint32, InventoryItemRef>::const_iterator cur, end;
    cur = mContents.begin();
    end = mContents.end();
    for(; cur != end; cur++)
    {
        if( cur->second->flag() == _flag )
            return cur->second;
    }

    sLog.Error("Inventory", "unable to find first by flag");
    return InventoryItemRef();
}

InventoryItemRef Inventory::GetByID(uint32 id) const
{
    std::map<uint32, InventoryItemRef>::const_iterator res = mContents.find( id );
    if( res != mContents.end() )
        return res->second;
    else
        return InventoryItemRef();
}

InventoryItemRef Inventory::GetByTypeFlag(uint32 typeID, EVEItemFlags flag) const
{
    std::map<uint32, InventoryItemRef>::const_iterator cur, end;
    cur = mContents.begin();
    end = mContents.end();
    for(; cur != end; cur++)
    {
        if( cur->second->typeID() == typeID
            && cur->second->flag() == flag )
        {
            return cur->second;
        }
    }

    return InventoryItemRef();
}

uint32 Inventory::FindByFlag(EVEItemFlags _flag, std::vector<InventoryItemRef> &items) const
{
    std::map<uint32, InventoryItemRef>::const_iterator cur, end;
    cur = mContents.begin();
    end = mContents.end();
    for(; cur != end; cur++)
    {
        if (cur->second)
        {
            if(cur->second->flag() == _flag)
            {
                items.push_back( cur->second );
            }
        }
    }
    return items.size();
}

bool Inventory::FindSingleByFlag( EVEItemFlags flag, InventoryItemRef &item ) const
{
    std::map<uint32, InventoryItemRef>::const_iterator cur, end;
    cur = mContents.begin();
    end = mContents.end();
    for(; cur != end; cur++)
    {
        if (cur->second)
        {
            if(cur->second->flag() == flag)
            {
                item = cur->second;
                return true;
            }
        }
    }
    return false;
}

bool Inventory::IsEmptyByFlag( EVEItemFlags flag )
{
    std::map<uint32, InventoryItemRef>::const_iterator cur, end;
    cur = mContents.begin();
    end = mContents.end();
    for(; cur != end; cur++)
    {
        if (cur->second)
        {
            if(cur->second->flag() == flag)
            {
                return false;
            }
        }
    }
    return true;
}

uint32 Inventory::FindByFlagRange(EVEItemFlags low_flag, EVEItemFlags high_flag, std::vector<InventoryItemRef> &items) const
{
    uint32 count = 0;

    std::map<uint32, InventoryItemRef>::const_iterator cur, end;
    cur = mContents.begin();
    end = mContents.end();
    for(; cur != end; cur++)
    {
        if( cur->second->flag() >= low_flag
            && cur->second->flag() <= high_flag )
        {
            items.push_back( cur->second );
            count++;
        }
    }

    return count;
}

uint32 Inventory::FindByFlagSet(std::set<EVEItemFlags> flags, std::vector<InventoryItemRef> &items) const
{
    uint32 count = 0;

    std::map<uint32, InventoryItemRef>::const_iterator cur, end;
    cur = mContents.begin();
    end = mContents.end();
    for(; cur != end; cur++)
    {
        if( flags.find( cur->second->flag() ) != flags.end() )
        {
            items.push_back( cur->second );
            count++;
        }
    }

    return count;
}

void Inventory::AddItem(InventoryItemRef item)
{
    std::map<uint32, InventoryItemRef>::iterator res = mContents.find( item->itemID() );
    if( res == mContents.end() )
    {
        mContents.insert( std::make_pair( item->itemID(), item ) );

        sLog.Debug("Inventory", "Updated location %u to contain item %u with flag %d.", inventoryID(), item->itemID(), (int)item->flag() );
    }
    //else already here
    sLog.Debug("Inventory", "unable to updated location %u to contain item %u with flag %d, because it already happend.", inventoryID(), item->itemID(), (int)item->flag() );
}

void Inventory::RemoveItem(uint32 itemID)
{
    std::map<uint32, InventoryItemRef>::iterator res = mContents.find( itemID );
    if( res != mContents.end() )
    {
        mContents.erase( res );

        sLog.Debug("Inventory", "Updated location %u to no longer contain item %u.", inventoryID(), itemID );
    }
    sLog.Debug("Inventory", "unable to remove %u from %u.", itemID, inventoryID() );
}

void Inventory::StackAll(EVEItemFlags locFlag, uint32 forOwner)
{
    std::map<uint32, InventoryItemRef> types;

    std::map<uint32, InventoryItemRef>::iterator cur, end;
    cur = mContents.begin();
    end = mContents.end();
    for(; cur != end; )
    {
        // Iterator becomes invalid when the item
        // is moved out; we have to increment before
        // calling Merge().
        InventoryItemRef i = cur->second;
        cur++;

        if( !i->singleton() && ( forOwner == 0 || forOwner == i->ownerID() ) )
        {
            std::map<uint32, InventoryItemRef>::iterator res = types.find( i->typeID() );
            if( res == types.end() )
                types.insert( std::make_pair( i->typeID(), i ) );
            else
                res->second->Merge( i );
        }
    }
}

double Inventory::GetStoredVolume(EVEItemFlags locationFlag) const
{
    //double totalVolume = 0.0;
    EvilNumber totalVolume(0.0);
    //TODO: And implement Sizes for packaged ships

    std::map<uint32, InventoryItemRef>::const_iterator cur, end;
    cur = mContents.begin();
    end = mContents.end();
    for(; cur != end; cur++)
    {
        if( cur->second->flag() == locationFlag )
            //totalVolume += cur->second->quantity() * cur->second->volume();
            totalVolume += cur->second->GetAttribute(AttrQuantity) * cur->second->GetAttribute(AttrVolume);
    }

    // this is crap... bleh... as it should return a EvilNumber
    return totalVolume.get_float();
}

/*
 * InventoryEx
 */
void InventoryEx::ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const
{
    //double volume = item->quantity() * item->volume();
    EvilNumber volume = item->GetAttribute(AttrQuantity) * item->GetAttribute(AttrVolume);
    double capacity = GetRemainingCapacity( flag );
    if( volume > capacity )
    {
        std::map<std::string, PyRep *> args;

        args["available"] = new PyFloat( capacity );
        args["volume"] = volume.GetPyObject();

        throw PyException( MakeUserError( "NotEnoughCargoSpace", args ) );
    }
}
