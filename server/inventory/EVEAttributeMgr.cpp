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

PyRep *EVEAttributeMgr::PyGet(Attr attr) const {
	return(_PyGet(GetReal(attr)));
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
			into[cur->first] = _PyGet(cur->second);
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
			into[cur->first] = _PyGet(cur->second);
		}
	}
}

PyRep *EVEAttributeMgr::_PyGet(const real_t &v) {
	if(_IsInt(v))
		return(new PyRepInteger(v));
	else
		return(new PyRepReal(v));
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

/*
 * EVEAdvancedAttributeMgr
 */
void EVEAdvancedAttributeMgr::BuildAttributesDict(std::map<uint32, PyRep *> &into) const {
	// integers first
	{
		std::map<Attr, int_t>::const_iterator cur, end;
		cur = m_ints.begin();
		end = m_ints.end();
		for(; cur != end; cur++) {
			if(into.find(cur->first) != into.end())
				delete into[cur->first];
			into[cur->first] = PyGet(cur->first);
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
			into[cur->first] = PyGet(cur->first);
		}
	}
}

/*
 * TypeAttributeMgr
 */
bool TypeAttributeMgr::Load(InventoryDB &db) {
	// delete old contents
	EVEAttributeMgr::Delete();
	// load the new contents
	return(db.LoadTypeAttributes(type().id, *this));
}

/*
 * ItemAttributeMgr
 */
ItemAttributeMgr::real_t ItemAttributeMgr::GetReal(Attr attr) const {
	real_t v;
	if(!_Get(attr, v))
		if(!m_item.type()->attributes._Get(attr, v))	// try the type attributes
			v = GetDefault(attr);

	_CalcTauCap(attr, v);

	return(v);
}

void ItemAttributeMgr::SetIntEx(Attr attr, const int_t &v, bool persist) {
	PyRep *oldValue = NULL;
	if(notify() && !IsRechargable(attr)) {
		// get old value
		oldValue = PyGet(attr);
	}
	// set the attribute value
	EVEAdvancedAttributeMgr::SetInt(attr, v);
	// check if we shall save to DB
	if(m_db != NULL && (persist || IsPersistent(attr))) {
		// save to DB
		m_db->UpdateAttribute_int(m_item.itemID(), attr, v);
	}
	if(notify()) {
		std::map<Attr, TauCap>::const_iterator i = m_tauCap.find(attr);
		if(i != m_tauCap.end()) {
			// build the special list for rechargables
			PyRepList *l = new PyRepList;
			l->addInt(v);
			l->addInt(Win32TimeNow());
			l->add(_PyGet(GetReal(i->second.tau) / 5.0));
			l->add(PyGet(i->second.cap));

			oldValue = l;
		}
		// send change
		SendAttributeChange(attr, oldValue, new PyRepReal(v));
	}
}

void ItemAttributeMgr::SetRealEx(Attr attr, const real_t &v, bool persist) {
	// first check if it can be stored as integer
	if(_IsInt(v)) {
		// store as integer
		SetIntEx(attr, v, persist);
	} else {
		// store as real
		PyRep *oldValue = NULL;
		if(notify() && !IsRechargable(attr)) {
			// get old value
			oldValue = PyGet(attr);
		}
		// set the attribute value
		EVEAdvancedAttributeMgr::SetReal(attr, v);
		// check if we shall save to DB
		if(m_db != NULL && (persist || IsPersistent(attr))) {
			// save to DB
			m_db->UpdateAttribute_double(m_item.itemID(), attr, v);
		}
		if(notify()) {
			std::map<Attr, TauCap>::const_iterator i = m_tauCap.find(attr);
			if(i != m_tauCap.end()) {
				// build the special list for rechargables
				PyRepList *l = new PyRepList;
				l->addReal(v);
				l->addInt(Win32TimeNow());
				l->add(_PyGet(GetReal(i->second.tau) / 5.0));
				l->add(PyGet(i->second.cap));

				oldValue = l;
			}
			// send change
			SendAttributeChange(attr, oldValue, new PyRepReal(v));
		}
	}
}

void ItemAttributeMgr::Clear(Attr attr) {
	PyRep *oldValue = NULL;
	if(notify() && !IsRechargable(attr)) {
		// get old value
		oldValue = PyGet(attr);
	}
	// clear the attribute
	EVEAdvancedAttributeMgr::Clear(attr);
	// delete the attribute from DB (no matter if it really is there)
	if(m_db != NULL)
		m_db->EraseAttribute(m_item.itemID(), attr);
	if(notify()) {
		std::map<Attr, TauCap>::const_iterator i = m_tauCap.find(attr);
		if(i != m_tauCap.end()) {
			// build the special list for rechargables
			PyRepList *l = new PyRepList;
			l->add(PyGet(attr));
			l->addInt(Win32TimeNow());
			l->add(_PyGet(GetReal(i->second.tau) / 5.0));
			l->add(PyGet(i->second.cap));

			oldValue = l;
		}
		// send change
		SendAttributeChange(attr, oldValue, new PyRepReal(GetReal(attr)));
	}
}

void ItemAttributeMgr::DeleteEx(bool notify) {
	const EntityList *el = m_el;
	if(!notify)
		// disable notifications
		SetEntityList(NULL);

	// delete the attributes
	EVEAdvancedAttributeMgr::Delete();

	// put back the entity list
	SetEntityList(el);
}

bool ItemAttributeMgr::Load(bool notify) {
	if(m_db == NULL)
		return false;

	const EntityList *el = m_el;
	if(!notify)
		// disable notifications
		SetEntityList(NULL);

	// disable saving
	InventoryDB *db = m_db;
	SetDB(NULL);

	// delete old contents
	EVEAdvancedAttributeMgr::Delete();
	// load the new contents
	bool res = db->LoadItemAttributes(item().itemID(), *this);

	// put back DB
	SetDB(db);

	// put back entity list
	SetEntityList(el);

	// return
	return(res);
}

void ItemAttributeMgr::Save() const {
	if(m_db == NULL)
		return;

	_log(ITEM__TRACE, "Saving %lu attributes of item %lu.", m_ints.size()+m_reals.size(), m_item.itemID());
	// integers first
	{
		std::map<Attr, int_t>::const_iterator cur, end;
		cur = m_ints.begin();
		end = m_ints.end();
		for(; cur != end; cur++) {
			real_t v = GetReal(cur->first);
			if(_IsInt(v))
				m_db->UpdateAttribute_int(m_item.itemID(), cur->first, v);
			else
				m_db->UpdateAttribute_double(m_item.itemID(), cur->first, v);
		}
	}
	// then reals
	{
		std::map<Attr, real_t>::const_iterator cur, end;
		cur = m_reals.begin();
		end = m_reals.end();
		for(; cur != end; cur++) {
			real_t v = GetReal(cur->first);
			if(_IsInt(v))
				m_db->UpdateAttribute_int(m_item.itemID(), cur->first, v);
			else
				m_db->UpdateAttribute_double(m_item.itemID(), cur->first, v);
		}
	}
}

void ItemAttributeMgr::BuildAttributesDict(std::map<uint32, PyRep *> &into) const {
	// first insert type attributes
	m_item.type()->attributes.BuildAttributesDict(into);
	// now isert (or overwrite) with our values
	EVEAdvancedAttributeMgr::BuildAttributesDict(into);
}

void ItemAttributeMgr::SendAttributeChange(Attr attr, PyRep *oldValue, PyRep *newValue) {
	Client *c = NULL;
	if(m_el != NULL)
		c = m_el->FindCharacter(m_item.ownerID());

	if(c != NULL) {
		Notify_OnModuleAttributeChange omac;
		omac.ownerID = m_item.ownerID();
		omac.itemKey = m_item.itemID();
		omac.attributeID = attr;
		omac.time = Win32TimeNow();
		omac.oldValue = oldValue;
		omac.newValue = newValue;

		PyRepTuple *tmp = omac.Encode();
		c->QueueDestinyEvent(&tmp);
	} else {
		// delete the reps
		delete oldValue;
		delete newValue;
	}
}



