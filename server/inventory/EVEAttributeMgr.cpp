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

#include "EvemuPCH.h"

/*
 * EVEAttributeMgr
 */
bool EVEAttributeMgr::m_persistentLoaded = false;
bool EVEAttributeMgr::m_persistent[EVEAttributeMgr::Invalid_Attr];

bool EVEAttributeMgr::m_defaultLoaded = false;
EVEAttributeMgr::int_t EVEAttributeMgr::m_default[EVEAttributeMgr::Invalid_Attr];

EVEAttributeMgr::real_t EVEAttributeMgr::GetReal(Attr attr) const {
	real_t v;
	if(!_Get(attr, v))
		v = GetDefault(attr);
	return(v);
}

void EVEAttributeMgr::BuildAttributesDict(std::map<uint32, PyRep *> &into) const {
	// integers first
	{
		std::map<Attr, int_t>::const_iterator cur, end;
		cur = m_ints.begin();
		end = m_ints.end();
		for(; cur != end; cur++) {
			if(into.find(cur->first) != into.end())
				delete into[cur->first];
			into[cur->first] = new PyRepInteger(cur->second);
		}
	}
	// then reals
	{
		std::map<Attr, real_t>::const_iterator cur, end;
		cur = m_reals.begin();
		end = m_reals.end();
		for(; cur != end; cur++) {
			if(into.find(cur->first) != into.end())
				delete into[cur->first];
			into[cur->first] = new PyRepReal(cur->second);
		}
	}
}

void EVEAttributeMgr::_LoadPersistent() {
	if(!m_persistentLoaded) {
		memset(m_persistent, false, sizeof(m_persistent));

		#define ATTR(ID, name, default_value, persistent) \
			m_persistent[Attr_##name] = persistent;
		#include "EVEAttributes.h"

		m_persistentLoaded = true;
	}
}

void EVEAttributeMgr::_LoadDefault() {
	if(!m_defaultLoaded) {
		memset(m_default, 0, sizeof(m_default));

		#define ATTR(ID, name, default_value, persistent) \
			m_default[Attr_##name] = default_value;
		#include "EVEAttributes.h"

		m_defaultLoaded = true;
	}
}

/*
 * TypeAttributeMgr
 */
bool TypeAttributeMgr::Load(InventoryDB &db) {
	// delete old contents
	EVEAttributeMgr::Delete();
	// load the new contants
	return(db.LoadTypeAttributes(m_type.id, *this));
}

/*
 * ItemAttributeMgr
 */
ItemAttributeMgr::real_t ItemAttributeMgr::GetReal(Attr attr) const {
	real_t v;
	if(!_Get(attr, v))
		if(!m_item.type()->attributes._Get(attr, v))	// try the type attributes
			v = GetDefault(attr);
	return(v);
}

void ItemAttributeMgr::SetInt(Attr attr, const int_t &v) {
	// set the attribute value
	EVEAttributeMgr::SetInt(attr, v);
	// check if the attribute is persistent
	if(IsPersistent(attr)) {
		// it is, save to DB
		m_db.UpdateAttribute_int(m_item.itemID(), attr, v);
	}
}

void ItemAttributeMgr::SetReal(Attr attr, const real_t &v) {
	// first check if it can be stored as integer
	if(_IsInt(v)) {
		// store as integer
		SetIntPersist(attr, v);
	} else {
		// store as real
		// set the attribute value
		EVEAttributeMgr::SetReal(attr, v);
		// check if the attribute is persistent
		if(IsPersistent(attr)) {
			// it is, save to DB
			m_db.UpdateAttribute_double(m_item.itemID(), attr, v);
		}
	}
}

void ItemAttributeMgr::Clear(Attr attr) {
	// clear the attribute
	EVEAttributeMgr::Clear(attr);
	// delete the attribute from DB (no matter if it really is there)
	m_db.EraseAttribute(m_item.itemID(), attr);
}

void ItemAttributeMgr::Delete() {
	// delete the attributes
	EVEAttributeMgr::Delete();
	// delete them from DB as well
	m_db.EraseAttributes(m_item.itemID());
}

void ItemAttributeMgr::SetIntPersist(Attr attr, const int_t &v) {
	// set the attribute value
	EVEAttributeMgr::SetInt(attr, v);
	// persist it into DB
	m_db.UpdateAttribute_int(m_item.itemID(), attr, v);
}

void ItemAttributeMgr::SetRealPersist(Attr attr, const real_t &v) {
	// first check if it can be stored as integer
	if(_IsInt(v)) {
		// store as integer
		SetIntPersist(attr, v);
	} else {
		// store as real
		// set the attribute value
		EVEAttributeMgr::SetReal(attr, v);
		// persist it into DB
		m_db.UpdateAttribute_double(m_item.itemID(), attr, v);
	}
}

bool ItemAttributeMgr::Load() {
	// delete old contents
	EVEAttributeMgr::Delete();
	// load the new contants
	return(m_db.LoadItemAttributes(m_item.itemID(), *this));
}

void ItemAttributeMgr::Save() const {
	// integers first
	{
		std::map<Attr, int_t>::const_iterator cur, end;
		cur = m_ints.begin();
		end = m_ints.end();
		for(; cur != end; cur++)
			m_db.UpdateAttribute_int(m_item.itemID(), cur->first, cur->second);
	}
	// then reals
	{
		std::map<Attr, real_t>::const_iterator cur, end;
		cur = m_reals.begin();
		end = m_reals.end();
		for(; cur != end; cur++)
			m_db.UpdateAttribute_double(m_item.itemID(), cur->first, cur->second);
	}
}

void ItemAttributeMgr::BuildAttributesDict(std::map<uint32, PyRep *> &into) const {
	// first insert type attributes
	m_item.type()->attributes.BuildAttributesDict(into);
	// now isert (or overwrite) with our values
	EVEAttributeMgr::BuildAttributesDict(into);
}



