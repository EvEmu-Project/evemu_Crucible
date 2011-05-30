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
	Author:		Bloody.Rabbit
*/

#ifndef __OWNER__H__INCL__
#define __OWNER__H__INCL__

#include "inventory/InventoryItem.h"

/**
 * Generic class for owners (Character, Corporation).
 */
class Owner
: public InventoryItem
{
	friend class InventoryItem; // to let it construct us
public:
	/**
	 * Loads Owner from DB.
	 *
	 * @param[in] factory ItemFactory to use when loading.
	 * @param[in] ownerID ID of owner to load.
	 * @return Ref to loaded Owner object.
	 */
	static OwnerRef Load(ItemFactory &factory, uint32 ownerID);
	/**
	 * Spawns new Owner.
	 *
	 * @param[in] factory ItemFactory to use when spawning.
	 * @param[in] data ItemData of new Owner.
	 * @return Ref to spawned Owner object.
	 */
	static OwnerRef Spawn(ItemFactory &factory, ItemData &data);

protected:
	Owner(
		ItemFactory &_factory,
		uint32 _ownerID,
		// InventoryItem stuff:
		const ItemType &_type,
		const ItemData &_data);

	/*
	 * Member functions
	 */
	using InventoryItem::_Load;

	// Template loader:
	template<class _Ty>
	static RefPtr<_Ty> _LoadItem(ItemFactory &factory, uint32 ownerID,
		// InventoryItem stuff:
		const ItemType &type, const ItemData &data)
	{
		// check it's an owner
		if( type.categoryID() != EVEDB::invCategories::Owner )
		{
			sLog.Error("Owner", "Trying to load %s as Owner.", type.category().name().c_str() );
			return RefPtr<_Ty>();
		}

		// no additional stuff

		return _Ty::template _LoadOwner<_Ty>( factory, ownerID, type, data );
	}

	// Actual loading stuff:
	template<class _Ty>
	static RefPtr<_Ty> _LoadOwner(ItemFactory &factory, uint32 ownerID,
		// InventoryItem stuff:
		const ItemType &type, const ItemData &data
	);

	static uint32 _Spawn(ItemFactory &factory,
		// InventoryItem stuff:
		ItemData &data
	);
};

#endif /* !__OWNER__H__INCL__ */

