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

#ifndef __INVENTORY__H__INCL__
#define __INVENTORY__H__INCL__

#include "inventory/InventoryItem.h"

class CRowSet;

class Inventory
{
	friend class InventoryItem;
public:
	/**
	 * Casts given InventoryItemRef to Inventory.
	 *
	 * @return Pointer to Inventory; NULL if given item isn't a valid inventory.
	 */
	static Inventory *Cast(InventoryItemRef item);

	Inventory();
	virtual ~Inventory();

	virtual uint32 inventoryID() const = 0;

	/*
	 * Contents management:
	 */
	bool ContentsLoaded() const { return mContentsLoaded; }
	bool LoadContents(ItemFactory &factory);
	void DeleteContents(ItemFactory &factory);

	bool Contains(uint32 itemID) const { return mContents.find( itemID ) != mContents.end(); }
	InventoryItemRef GetByID(uint32 id) const;
	InventoryItemRef GetByTypeFlag(uint32 typeID, EVEItemFlags flag) const;

    /*
     *
     */
	InventoryItemRef FindFirstByFlag(EVEItemFlags flag) const;
	uint32 FindByFlag(EVEItemFlags flag, std::vector<InventoryItemRef> &items) const;
	uint32 FindByFlagRange(EVEItemFlags low_flag, EVEItemFlags high_flag, std::vector<InventoryItemRef> &items) const;
	uint32 FindByFlagSet(std::set<EVEItemFlags> flags, std::vector<InventoryItemRef> &items) const;

    /* should do exactly the same as FindFirstByFlag, but only searches a single item.
     * it asserts when it finds multiple items. It should be used to search single items... like rigs...
     * @returns true if the object is found and false if its not.
     */
    bool FindSingleByFlag(EVEItemFlags flag, InventoryItemRef &item) const;

    /* checks if a a item is present at a certain location flag.
     *
     * @returns true if its empty and false if its not.
     */
    bool IsEmptyByFlag(EVEItemFlags flag);


	double GetStoredVolume(EVEItemFlags flag) const;

	virtual void ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const {}
	void StackAll(EVEItemFlags flag, uint32 forOwner = 0);

	/*
	 * Primary packet builders:
	 */
	virtual PyRep* GetItem() const = 0;

	CRowSet* List( EVEItemFlags flag, uint32 forOwner = 0 ) const;
	void List( CRowSet* into, EVEItemFlags flag, uint32 forOwner = 0 ) const;

protected:
	virtual void AddItem(InventoryItemRef item);
	virtual void RemoveItem(uint32 itemID);

	virtual bool GetItems(ItemFactory &factory, std::vector<uint32> &into) const { return factory.db().GetItemContents( inventoryID(), into ); }

	bool mContentsLoaded;
	std::map<uint32, InventoryItemRef> mContents;	//maps item ID to its instance. we own a ref to all of these.
};

class InventoryEx
: public Inventory
{
public:
	virtual double GetCapacity(EVEItemFlags flag) const = 0;
	double GetRemainingCapacity(EVEItemFlags flag) const { return GetCapacity( flag ) - GetStoredVolume( flag ); }

	void ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const;
};

#endif /* !__INVENTORY__H__INCL__ */

