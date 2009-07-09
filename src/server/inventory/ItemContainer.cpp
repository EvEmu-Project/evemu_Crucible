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
 * ItemContainer
 */
ItemContainer::ItemContainer()
: m_contentsLoaded(false)
{
}

ItemContainer::~ItemContainer()
{
	std::map<uint32, InventoryItem *>::iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++)
		cur->second->DecRef();
}

bool ItemContainer::LoadContents(ItemFactory &factory)
{
	if( ContentsLoaded() )
		return true;

	_log( ITEM__TRACE, "Recursively loading contents of container %u", containerID() );

	//load the list of items we need
	std::vector<uint32> items;
	if( !factory.db().GetItemContents( containerID(), items ) )
		return false;
	
	//Now get each one from the factory (possibly recursing)
	std::vector<uint32>::iterator cur, end;
	cur = items.begin();
	end = items.end();
	for(; cur != end; cur++)
	{
		InventoryItem *i = factory.GetItem( *cur );
		if( i == NULL )
		{
			_log( ITEM__ERROR, "Failed to load item %u contained in %u. Skipping.", *cur, containerID() );
			continue;
		}

		AddContainedItem( *i );
		i->DecRef();
	}

	m_contentsLoaded = true;
	return true;
}

void ItemContainer::DeleteContents(ItemFactory &factory)
{
	LoadContents( factory );

	std::map<uint32, InventoryItem *>::iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; ) {
		InventoryItem *i = cur->second->IncRef();
		//iterator will become invalid after calling
		//Delete(), so increment now.
		cur++;

		i->Delete();
	}
	//Delete() releases our ref on these.
	//so it should be empty anyway.
	m_contents.clear();
}

PyRepNewObject *ItemContainer::List(EVEItemFlags _flag, uint32 forOwner) const
{
	dbutil_CRowset rowset;

	ItemRow_Columns cols;
	rowset.header.columns = cols.FastEncode();

	List( rowset, _flag, forOwner );
	return rowset.FastEncode();
}

void ItemContainer::List(dbutil_CRowset &into, EVEItemFlags _flag, uint32 forOwner) const
{
	//there has to be a better way to build this...
	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++) {
		InventoryItem *i = cur->second;

		if(    (i->flag() == _flag       || _flag == flagAnywhere)
		    && (i->ownerID() == forOwner || forOwner == 0) )
		{
			into.root_list.push_back( i->GetItemRow() );
		}
	}
}

InventoryItem *ItemContainer::FindFirstByFlag(EVEItemFlags _flag, bool newref) const
{
	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++)
	{
		InventoryItem *i = cur->second;

		if( i->flag() == _flag )
			return newref ? i->IncRef() : i;
	}

	return NULL;
}

InventoryItem *ItemContainer::GetByID(uint32 id, bool newref) const
{
	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++)
	{
		InventoryItem *i = cur->second;

		if( i->itemID() == id )
			return newref ? i->IncRef() : i;
	}

	return NULL;
}

InventoryItem *ItemContainer::GetByTypeFlag(uint32 typeID, EVEItemFlags flag, bool newref) const
{
	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++)
	{
		InventoryItem *i = cur->second;

		if( i->typeID() == typeID
			&& i->flag() == flag )
		{
			return newref ? i->IncRef() : i;
		}
	}

	return NULL;
}

uint32 ItemContainer::FindByFlag(EVEItemFlags _flag, std::vector<InventoryItem *> &items, bool newref) const
{
	uint32 count = 0;

	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++)
	{
		InventoryItem *i = cur->second;

		if( i->flag() == _flag )
		{
			items.push_back( newref ? i->IncRef() : i );
			count++;
		}
	}

	return count;
}

uint32 ItemContainer::FindByFlagRange(EVEItemFlags low_flag, EVEItemFlags high_flag, std::vector<InventoryItem *> &items, bool newref) const
{
	uint32 count = 0;

	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++)
	{
		InventoryItem *i = cur->second;

		if( i->flag() >= low_flag
			&& i->flag() <= high_flag )
		{
			items.push_back( newref ? i->IncRef() : i );
			count++;
		}
	}

	return count;
}

uint32 ItemContainer::FindByFlagSet(std::set<EVEItemFlags> flags, std::vector<InventoryItem *> &items, bool newref) const
{
	uint32 count = 0;

	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++)
	{
		InventoryItem *i = cur->second;

		if( flags.find( i->flag() ) != flags.end() )
		{
			items.push_back( newref ? i->IncRef() : i );
			count++;
		}
	}

	return count;
}

void ItemContainer::AddContainedItem(InventoryItem &item)
{
	std::map<uint32, InventoryItem *>::iterator res = m_contents.find( item.itemID() );
	if( res == m_contents.end() )
	{
		m_contents[ item.itemID() ] = item.IncRef();

		_log( ITEM__TRACE, "   Updated location %u to contain item %u with flag %d.", containerID(), item.itemID(), (int)item.flag() );
	}
	else if( res->second != &item )
	{
		_log( ITEM__ERROR, "Both object %p and object %p represent item %u!", res->second, &item, item.itemID() );
	} //else already here
}

void ItemContainer::RemoveContainedItem(uint32 itemID)
{
	std::map<uint32, InventoryItem *>::iterator old_inst = m_contents.find( itemID );
	if( old_inst != m_contents.end() )
	{
		old_inst->second->DecRef();
		m_contents.erase( old_inst );

		_log( ITEM__TRACE, "   Updated location %u to no longer contain item %u.", containerID(), itemID );
	}
}

void ItemContainer::StackAll(EVEItemFlags locFlag, uint32 forOwner)
{
	std::map<uint32, InventoryItem *> types;

	std::map<uint32, InventoryItem *>::iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; )
	{
		InventoryItem *i = cur->second;
		cur++;

		if( !i->singleton()
		    && (forOwner == 0
			   || forOwner == i->ownerID() ) )
		{
			std::map<uint32, InventoryItem *>::iterator res = types.find( i->typeID() );
			if( res == types.end() )
				types[ i->typeID() ] = i;
			else
				//dont forget to make ref (which is consumed)
				res->second->Merge( i->IncRef() );
		}
	}
}

double ItemContainer::GetStoredVolume(EVEItemFlags locationFlag) const
{
	double totalVolume = 0.0;
	//TODO: And implement Sizes for packaged ships

 	std::map<uint32, InventoryItem *>::const_iterator cur, end;
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
 * ItemContainerEx
 */
void ItemContainerEx::ValidateAdd(EVEItemFlags flag, InventoryItem &item) const
{
	double volume = item.quantity() * item.volume();
	double capacity = GetRemainingCapacity( flag );
	if( volume > capacity )
	{
		std::map<std::string, PyRep *> args;

		args["available"] = new PyRepReal( capacity );
		args["volume"] = new PyRepReal( volume );

		throw PyException( MakeUserError( "NotEnoughCargoSpace", args ) );
	}
}


