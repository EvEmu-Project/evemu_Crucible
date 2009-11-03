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
    Author:     Bloody.Rabbit
*/

#include "EVEServerPCH.h"

/*
 * Inventory
 */
Inventory *Inventory::Cast(InventoryItemRef item)
{
    if( item )
    {
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
            case EVEDB::invCategories::Structure:
            case EVEDB::invCategories::Reaction:
            case EVEDB::invCategories::Asteroid:
                break;
            case EVEDB::invCategories::Ship:    return ShipRef::StaticCast( item ).get();
        }

        switch( item->groupID() )
        {
            case EVEDB::invGroups::Station:     return StationRef::StaticCast( item ).get();
            case EVEDB::invGroups::Character:   return CharacterRef::StaticCast( item ).get();
        }
    }

    return NULL;
}

Inventory::Inventory()
: mContentsLoaded(false)
{
}

Inventory::~Inventory()
{
}

bool Inventory::LoadContents(ItemFactory &factory)
{
    if( ContentsLoaded() )
        return true;

    _log( ITEM__TRACE, "Recursively loading contents of inventory %u", inventoryID() );

    //load the list of items we need
    std::vector<uint32> items;
    if( !GetItems( factory, items ) )
        return false;

    //Now get each one from the factory (possibly recursing)
    std::vector<uint32>::iterator cur, end;
    cur = items.begin();
    end = items.end();
    for(; cur != end; cur++)
    {
        InventoryItemRef i = factory.GetItem( *cur );
        if( !i )
        {
            _log( ITEM__ERROR, "Failed to load item %u contained in %u. Skipping.", *cur, inventoryID() );
            continue;
        }

        AddItem( i );
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
	header->AddColumn( "flag",       DBTYPE_UI1 );
	header->AddColumn( "contraband", DBTYPE_BOOL );
	header->AddColumn( "singleton",  DBTYPE_BOOL );
	header->AddColumn( "quantity",   DBTYPE_I4 );
	header->AddColumn( "groupID",    DBTYPE_I2 );
	header->AddColumn( "categoryID", DBTYPE_UI1 );
	header->AddColumn( "customInfo", DBTYPE_STR );

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
    uint32 count = 0;

    std::map<uint32, InventoryItemRef>::const_iterator cur, end;
    cur = mContents.begin();
    end = mContents.end();
    for(; cur != end; cur++)
    {
        if( cur->second->flag() == _flag )
        {
            items.push_back( cur->second );
            count++;
        }
    }

    return count;
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

        _log( ITEM__TRACE, "   Updated location %u to contain item %u with flag %d.", inventoryID(), item->itemID(), (int)item->flag() );
    }
    //else already here
}

void Inventory::RemoveItem(uint32 itemID)
{
    std::map<uint32, InventoryItemRef>::iterator res = mContents.find( itemID );
    if( res != mContents.end() )
    {
        mContents.erase( res );

        _log( ITEM__TRACE, "   Updated location %u to no longer contain item %u.", inventoryID(), itemID );
    }
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

        if( !i->singleton()
            && (forOwner == 0
               || forOwner == i->ownerID() ) )
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
    double totalVolume = 0.0;
    //TODO: And implement Sizes for packaged ships

    std::map<uint32, InventoryItemRef>::const_iterator cur, end;
    cur = mContents.begin();
    end = mContents.end();
    for(; cur != end; cur++)
    {
        if( cur->second->flag() == locationFlag )
            totalVolume += cur->second->quantity() * cur->second->volume();
    }

    return totalVolume;
}

/*
 * InventoryEx
 */
void InventoryEx::ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const
{
    double volume = item->quantity() * item->volume();
    double capacity = GetRemainingCapacity( flag );
    if( volume > capacity )
    {
        std::map<std::string, PyRep *> args;

        args["available"] = new PyFloat( capacity );
        args["volume"] = new PyFloat( volume );

        throw PyException( MakeUserError( "NotEnoughCargoSpace", args ) );
    }
}


