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
	Author:		Bloody.Rabbit
*/

#include "EvemuPCH.h"

/*
 * Inventory
 */
Inventory::Inventory()
: m_contentsLoaded(false)
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
	if( !factory.db().GetItemContents( inventoryID(), items ) )
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

	m_contentsLoaded = true;
	return true;
}

void Inventory::DeleteContents(ItemFactory &factory)
{
	LoadContents( factory );

	std::map<uint32, InventoryItemRef>::iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; )
	{
		// Our "cur" iterator becomes invalid once RemoveItem
		// for its item is called, so we need to increment it
		// before calling Delete().
		InventoryItemRef i = cur->second;
		cur++;

		i->Delete();
	}

	m_contents.clear();
}

PyRepObjectEx *Inventory::List(EVEItemFlags _flag, uint32 forOwner) const
{
	dbutil_CRowset rowset;

	blue_DBRowDescriptor desc;

	ItemRow_Columns cols;
	desc.columns = cols.FastEncode();

	rowset.header = desc.FastEncode();

	List( rowset, _flag, forOwner );
	return rowset.FastEncode();
}

void Inventory::List(dbutil_CRowset &into, EVEItemFlags _flag, uint32 forOwner) const
{
	//there has to be a better way to build this...
	std::map<uint32, InventoryItemRef>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++) {
		InventoryItemRef i = cur->second;

		if(    (i->flag() == _flag       || _flag == flagAnywhere)
		    && (i->ownerID() == forOwner || forOwner == 0) )
		{
			into.root_list.push_back( i->GetItemRow() );
		}
	}
}

InventoryItemRef Inventory::FindFirstByFlag(EVEItemFlags _flag) const
{
	std::map<uint32, InventoryItemRef>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++)
	{
		if( cur->second->flag() == _flag )
			return cur->second;
	}

	return InventoryItemRef();
}

InventoryItemRef Inventory::GetByID(uint32 id) const
{
	std::map<uint32, InventoryItemRef>::const_iterator res = m_contents.find( id );
	if( res != m_contents.end() )
		return res->second;
	else
		return InventoryItemRef();
}

InventoryItemRef Inventory::GetByTypeFlag(uint32 typeID, EVEItemFlags flag) const
{
	std::map<uint32, InventoryItemRef>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
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
	cur = m_contents.begin();
	end = m_contents.end();
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
	cur = m_contents.begin();
	end = m_contents.end();
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
	cur = m_contents.begin();
	end = m_contents.end();
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
	std::map<uint32, InventoryItemRef>::iterator res = m_contents.find( item->itemID() );
	if( res == m_contents.end() )
	{
		m_contents.insert( std::make_pair( item->itemID(), item ) );

		_log( ITEM__TRACE, "   Updated location %u to contain item %u with flag %d.", inventoryID(), item->itemID(), (int)item->flag() );
	}
	//else already here
}

void Inventory::RemoveItem(uint32 itemID)
{
	std::map<uint32, InventoryItemRef>::iterator res = m_contents.find( itemID );
	if( res != m_contents.end() )
	{
		m_contents.erase( res );

		_log( ITEM__TRACE, "   Updated location %u to no longer contain item %u.", inventoryID(), itemID );
	}
}

void Inventory::StackAll(EVEItemFlags locFlag, uint32 forOwner)
{
	std::map<uint32, InventoryItemRef> types;

	std::map<uint32, InventoryItemRef>::iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; )
	{
		InventoryItemRef i = cur->second;

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
	cur = m_contents.begin();
	end = m_contents.end();
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

		args["available"] = new PyRepReal( capacity );
		args["volume"] = new PyRepReal( volume );

		throw PyException( MakeUserError( "NotEnoughCargoSpace", args ) );
	}
}


