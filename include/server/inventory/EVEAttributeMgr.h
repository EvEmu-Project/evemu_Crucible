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
	Author:		Zhur
*/

#ifndef __EVE_ATTRIBUTE_MGR__H__INCL__
#define __EVE_ATTRIBUTE_MGR__H__INCL__

/*
 * This file contains EVE-specific derivations of attribute managers.
 */

#include "AttributeMgr.h"

class PyRep;
class Type;
class InventoryItem;
class InventoryDB;
class EntityList;

/*
 * Base EVE attribute manager
 */
class EVEAttributeMgr
: virtual public AttributeMgr<int, double>
{
public:
	/*
	 * Returns attribute value as PyRep
	 */
	PyRep *PyGet(Attr attr) const;

	/*
	 * Builds intdict from attributes
	 */
	virtual void EncodeAttributes(std::map<uint32, PyRep *> &into) const;

	/*
	 * Checks whether the attribute is persistent
	 */
	static bool IsPersistent(Attr attr) {
		_LoadPersistent();
		return(m_persistent[attr]);
	}

protected:
	/*
	 * Turns given value into proper PyRep
	 */
	static PyRep *_PyGet(const real_t &v);

	/*
	 * Persistent stuff
	 */
	static void _LoadPersistent();

	static bool m_persistentLoaded;
	static bool m_persistent[Invalid_Attr];
};

class EVEAdvancedAttributeMgr
: public AdvancedAttributeMgr<int, double>,
  public EVEAttributeMgr
{
public:
	void EncodeAttributes(std::map<uint32, PyRep *> &into) const;

	/*
	 * These kill warnings about inheritation dominance
	 */
	/*real_t GetReal(Attr attr) const { return(AdvancedAttributeMgr::GetReal(attr)); }

	void SetReal(Attr attr, const real_t &v) { AdvancedAttributeMgr::SetReal(attr, v); }
	void SetInt(Attr attr, const int_t &v) { AdvancedAttributeMgr::SetInt(attr, v); }

	void Clear(Attr attr) { AdvancedAttributeMgr::Clear(attr); }*/
};

class ItemAttributeMgr;

/*
 * Attribute manager for type attributes
 */
class TypeAttributeMgr
: public EVEAttributeMgr
{
	friend class ItemAttributeMgr;	// for access to _Get
public:
	TypeAttributeMgr(const Type &type)
		: m_type(type) {}

	const Type &type() const { return(m_type); }

	/*
	 * Attribute load from DB
	 */
	bool Load(InventoryDB &db);

protected:

	const Type &m_type;
};

/*
 * InventoryItem attribute manager
 */
class ItemAttributeMgr
: public EVEAdvancedAttributeMgr
{
public:
	ItemAttributeMgr(const InventoryItem &item, InventoryDB *db = NULL, const EntityList *el = NULL)
		: m_item(item) { SetDB(db); SetEntityList(el); }

	const InventoryItem &item() const { return(m_item); }

	/*
	 * These may be used to enable/disable saving or notifications
	 */
	void SetDB(InventoryDB *db) { m_db = db; }
	void SetEntityList(const EntityList *el) { m_el = el; }

	/*
	 * Falls to type attributes and then to default if not found
	 */
	real_t GetReal(Attr attr) const;

	/*
	 * Common functions and their expanded versions
	 */
	void SetInt(Attr attr, const int_t &v) { SetIntEx(attr, v); }
	void SetIntEx(Attr attr, const int_t &v, bool persist=false);

	void SetReal(Attr attr, const real_t &v) { SetRealEx(attr, v); }
	void SetRealEx(Attr attr, const real_t &v, bool persist=false);

	void Clear(Attr attr);

	void Delete() { DeleteEx(); }
	void DeleteEx(bool notify=false);

	/*
	 * Additional DB stuff
	 */
	bool Load(bool notify=false);
	void Save() const;

	/*
	 * Overload to include type attributes
	 */
	void EncodeAttributes(std::map<uint32, PyRep *> &into) const;

	/*
	 * Additional name access
	 */
	#define ATTRI(ID, name, default_value, persistent) \
		void Set_##name##_persist(const int_t &v) { \
			SetIntEx(Attr_##name, v, true); \
		}
	#define ATTRD(ID, name, default_value, persistent) \
		void Set_##name##_persist(const real_t &v) { \
			SetRealEx(Attr_##name, v, true); \
		}
	#include "EVEAttributes.h"

protected:
	void _SendAttributeChange(Attr attr, PyRep *oldValue, PyRep *newValue);

	/*
	 * Member variables
	 */
	const InventoryItem &m_item;

	InventoryDB *m_db;
	const EntityList *m_el;
};

#endif /* __EVE_ATTRIBUTE_MGR__H__INCL__ */


