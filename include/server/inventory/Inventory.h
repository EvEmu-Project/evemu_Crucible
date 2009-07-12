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

#ifndef __INVENTORY__H__INCL__
#define __INVENTORY__H__INCL__

class InventoryItem;
class ItemFactory;
class dbutil_CRowset;

class Inventory
{
	friend class InventoryItem;
public:
	Inventory();
	virtual ~Inventory();

	virtual uint32 inventoryID() const = 0;

	/*
	 * Contents management:
	 */
	bool ContentsLoaded() const { return m_contentsLoaded; }
	bool LoadContents(ItemFactory &factory);
	void DeleteContents(ItemFactory &factory);

	bool Contains(uint32 itemID) const { return m_contents.find( itemID ) != m_contents.end(); }
	InventoryItem *GetByID(uint32 id, bool newref = false) const;
	InventoryItem *GetByTypeFlag(uint32 typeID, EVEItemFlags flag, bool newref = false) const;

	InventoryItem *FindFirstByFlag(EVEItemFlags flag, bool newref = false) const;
	uint32 FindByFlag(EVEItemFlags flag, std::vector<InventoryItem *> &items, bool newref = false) const;
	uint32 FindByFlagRange(EVEItemFlags low_flag, EVEItemFlags high_flag, std::vector<InventoryItem *> &items, bool newref = false) const;
	uint32 FindByFlagSet(std::set<EVEItemFlags> flags, std::vector<InventoryItem *> &items, bool newref = false) const;

	double GetStoredVolume(EVEItemFlags flag) const;

	virtual void ValidateAddItem(EVEItemFlags flag, InventoryItem &item) const {}
	void StackAll(EVEItemFlags flag, uint32 forOwner = 0);

	/*
	 * Primary packet builders:
	 */
	virtual PyRep *GetItem() const = 0;

	PyRepObjectEx *List(EVEItemFlags flag = flagAnywhere, uint32 forOwner = 0) const;
	void List(dbutil_CRowset &into, EVEItemFlags flag = flagAnywhere, uint32 forOwner = 0) const;

protected:
	virtual void AddItem(InventoryItem &item);
	virtual void RemoveItem(uint32 itemID);

	bool m_contentsLoaded;
	std::map<uint32, InventoryItem *> m_contents;	//maps item ID to its instance. we own a ref to all of these.
};

class InventoryEx
: public Inventory
{
public:
	virtual double GetCapacity(EVEItemFlags flag) const = 0;
	double GetRemainingCapacity(EVEItemFlags flag) const { return GetCapacity( flag ) - GetStoredVolume( flag ); }

	void ValidateAddItem(EVEItemFlags flag, InventoryItem &item) const;
};

#endif /* !__INVENTORY__H__INCL__ */

