/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

/*
 * Base EVE attribute manager
 */
class EVEAttributeMgr
: public AttributeMgr<int, double>
{
public:
	/*
	 * Falls to default if not found
	 */
	real_t GetReal(Attr attr) const;

	/*
	 * Checks whether the attribute is persistent
	 */
	static bool IsPersistent(Attr attr) {
		_LoadPersistent();
		return(m_persistent[attr]);
	}

	/*
	 * Returns default value of attribute
	 */
	static int_t GetDefault(Attr attr) {
		_LoadDefault();
		return(m_default[attr]);
	}

	/*
	 * Builds intdict from attributes
	 */
	void BuildAttributesDict(std::map<uint32, PyRep *> &into) const;

protected:
	/*
	 * Persistent stuff
	 */
	static void _LoadPersistent();

	static bool m_persistentLoaded;
	static bool m_persistent[Invalid_Attr];

	/*
	 * Default stuff
	 */
	static void _LoadDefault();

	static bool m_defaultLoaded;
	static int_t m_default[Invalid_Attr];
};

class ItemAttributeMgr;

/*
 * Attribute manager for type attributes
 */
class TypeAttributeMgr
: public EVEAttributeMgr
{
public:
	TypeAttributeMgr(const Type &type)
		: m_type(type) {}

	/*
	 * Attribute load from DB
	 */
	bool Load(InventoryDB &db);

protected:
	friend class ItemAttributeMgr;	// for access to _Get

	const Type &m_type;
};

/*
 * InventoryItem attribute manager
 */
class ItemAttributeMgr
: public EVEAttributeMgr
{
public:
	ItemAttributeMgr(const InventoryItem &item, InventoryDB &db)
		: m_item(item), m_db(db) {}

	/*
	 * Falls to type attributes and then to default if not found
	 */
	real_t GetReal(Attr attr) const;

	/*
	 * Overload to save to DB (if persistent)
	 */
	void SetInt(Attr attr, const int_t &v);
	void SetReal(Attr attr, const real_t &v);

	/*
	 * Overloads to delete attribute(s) from DB
	 */
	void Clear(Attr attr);
	void Delete();

	/*
	 * These enforce save to DB
	 */
	void SetIntPersist(Attr attr, const int_t &v);
	void SetRealPersist(Attr attr, const real_t &v);

	/*
	 * Attribute load from DB
	 */
	bool Load();

	/*
	 * Attribute save to DB
	 */
	void Save() const;

	/*
	 * Overload to include type attributes
	 */
	void BuildAttributesDict(std::map<uint32, PyRep *> &into) const;

	/*
	 * Additional name access
	 */
	#define ATTRI(ID, name, default_value, persistent) \
		void Set_##name##_persist(const int_t &v) { \
			SetIntPersist(Attr_##name, v); \
		}
	#define ATTRD(ID, name, default_value, persistent) \
		void Set_##name##_persist(const real_t &v) { \
			SetRealPersist(Attr_##name, v); \
		}
	#include "EVEAttributes.h"

protected:
	const InventoryItem &m_item;
	InventoryDB &m_db;
};

#endif /* __EVE_ATTRIBUTE_MGR__H__INCL__ */


