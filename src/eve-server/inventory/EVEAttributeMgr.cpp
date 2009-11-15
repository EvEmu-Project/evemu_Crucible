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
    Author:     Zhur
*/

#include "EVEServerPCH.h"

/*
 * EVEAttributeMgr
 */
bool EVEAttributeMgr::m_persistentLoaded = false;
bool EVEAttributeMgr::m_persistent[EVEAttributeMgr::Invalid_Attr];

PyRep *EVEAttributeMgr::PyGet(Attr attr) const {
    return _PyGet(GetReal(attr));
}

void EVEAttributeMgr::EncodeAttributes(std::map<int32, PyRep *> &into) const {
    // integers first
    {
        std::map<Attr, int_t>::const_iterator cur, end;
        cur = m_ints.begin();
        end = m_ints.end();
        for(; cur != end; cur++) {
            if(into.find(cur->first) != into.end())
                PyDecRef( into[cur->first] );
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
                PyDecRef( into[cur->first] );
            into[cur->first] = _PyGet(cur->second);
        }
    }
}

PyRep *EVEAttributeMgr::_PyGet(const real_t &v)
{
    if(_IsInt(v) == true)
    {
        return new PyInt(v);
    }
    else
    {
        return new PyFloat(v);
    }
}

void EVEAttributeMgr::_LoadPersistent() {
    if(!m_persistentLoaded) {
        memset(m_persistent, false, sizeof(m_persistent));

        #define ATTR(ID, name, default_value, persistent) \
            m_persistent[Attr_##name] = persistent;
        #include "inventory/EVEAttributes.h"

        m_persistentLoaded = true;
    }
}

/*
 * EVEAdvancedAttributeMgr
 */
void EVEAdvancedAttributeMgr::EncodeAttributes(std::map<int32, PyRep *> &into) const {
    // integers first
    {
        std::map<Attr, int_t>::const_iterator cur, end;
        cur = m_ints.begin();
        end = m_ints.end();
        for(; cur != end; cur++) {
            if(into.find(cur->first) != into.end())
                PyDecRef( into[cur->first] );
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
                PyDecRef( into[cur->first] );
            into[cur->first] = PyGet(cur->first);
        }
    }
}

/*
 * TypeAttributeMgr
 */
bool TypeAttributeMgr::Load(InventoryDB &db) {
    // load new contents from DB
    return db.LoadTypeAttributes(type().id(), *this);
}

/*
 * ItemAttributeMgr
 */
ItemAttributeMgr::ItemAttributeMgr( ItemFactory &factory, const InventoryItem &item, bool save, bool notify) :
    m_factory(factory), m_item(item), m_save(save), m_notify(notify) {}

ItemAttributeMgr::real_t ItemAttributeMgr::GetReal(Attr attr) const {
    real_t v;
    if(!_Get(attr, v))
        if(!m_item.type().attributes._Get(attr, v)) // try the type attributes
            v = GetDefault(attr);

    _CalcTauCap(attr, v);

    return v;
}

void ItemAttributeMgr::SetIntEx(Attr attr, const int_t &v, bool persist) {
    PyRep *oldValue = NULL;
    if(GetNotify() == true && !IsRechargable(attr)) {
        // get old value
        oldValue = PyGet(attr);
    }
    // set the attribute value
    EVEAdvancedAttributeMgr::SetInt(attr, v);
    // check if we shall save to DB
    if(GetSave() == true && (persist || IsPersistent(attr))) {
        // save to DB
        m_factory.db().UpdateAttribute_int(m_item.itemID(), attr, v);
    }
    if(GetNotify() == true) {
        std::map<Attr, TauCap>::const_iterator i = m_tauCap.find(attr);
        if(i != m_tauCap.end()) {
            // build the special list for rechargables
            PyList *l = new PyList;

            l->AddItemInt( v );
            l->AddItemLong( Win32TimeNow() );
            l->AddItem( _PyGet( GetReal( i->second.tau ) / 5.0 ) );
            l->AddItem( PyGet( i->second.cap ) );

            oldValue = l;
        }
        // send change
        _SendAttributeChange(attr, oldValue, new PyFloat(v));
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
        if(GetNotify() == true && !IsRechargable(attr)) {
            // get old value
            oldValue = PyGet(attr);
        }
        // set the attribute value
        EVEAdvancedAttributeMgr::SetReal(attr, v);
        // check if we shall save to DB
        if(GetSave() == true && (persist || IsPersistent(attr))) {
            // save to DB
            m_factory.db().UpdateAttribute_double(m_item.itemID(), attr, v);
        }
        if(GetNotify() == true) {
            std::map<Attr, TauCap>::const_iterator i = m_tauCap.find(attr);
            if(i != m_tauCap.end()) {
                // build the special list for rechargables
                PyList *l = new PyList;

                l->AddItemReal( v );
                l->AddItemLong( Win32TimeNow() );
                l->AddItem( _PyGet( GetReal( i->second.tau ) / 5.0 ) );
                l->AddItem( PyGet( i->second.cap ) );

                oldValue = l;
            }
            // send change
            _SendAttributeChange(attr, oldValue, new PyFloat(v));
        }
    }
}

void ItemAttributeMgr::Clear(Attr attr) {
    PyRep *oldValue = NULL;
    if(GetNotify() == true && !IsRechargable(attr)) {
        // get old value
        oldValue = PyGet(attr);
    }
    // clear the attribute
    EVEAdvancedAttributeMgr::Clear(attr);
    // delete the attribute from DB (no matter if it really is there)
    if(GetSave() == true) {
        m_factory.db().EraseAttribute(m_item.itemID(), attr);
    }
    if(GetNotify() == true) {
        std::map<Attr, TauCap>::const_iterator i = m_tauCap.find(attr);
        if(i != m_tauCap.end()) {
            // build the special list for rechargables
            PyList *l = new PyList;

            l->AddItem( PyGet( attr ) );
            l->AddItemLong( Win32TimeNow() );
            l->AddItem( _PyGet( GetReal( i->second.tau ) / 5.0 ) );
            l->AddItem( PyGet( i->second.cap ) );

            oldValue = l;
        }
        // send change
        _SendAttributeChange(attr, oldValue, new PyFloat(GetReal(attr)));
    }
}

void ItemAttributeMgr::DeleteEx(bool notify) {
    // save & set notify state
    bool old_notify = GetNotify();
    SetNotify(notify);

    // delete the attributes
    EVEAdvancedAttributeMgr::Delete();

    // restore old notify state
    SetNotify(old_notify);
}

bool ItemAttributeMgr::Load(bool notify) {

    // save & set notify state
    bool old_notify = GetNotify();
    SetNotify(notify);

    // save & set save state
    bool old_save = GetSave();
    SetSave(false);

    // delete old contents
    EVEAdvancedAttributeMgr::Delete();
    // load the new contents
    bool res = m_factory.db().LoadItemAttributes(item().itemID(), *this);

    // restore save state
    SetSave(old_save);

    // restore notify state
    SetNotify(old_notify);

    return res;
}

void ItemAttributeMgr::Save() const {

    /* check if we have something to save, if not return*/
    if(m_ints.empty() == true && m_reals.empty() == true)
        return;

    /* disabled because its useless logging */
    //_log(ITEM__TRACE, "Saving %lu attributes of item %u.", m_ints.size()+m_reals.size(), m_item.itemID());
    // integers first
    {
        std::map<Attr, int_t>::const_iterator cur, end;
        cur = m_ints.begin();
        end = m_ints.end();
        for(; cur != end; cur++) {
            real_t v = GetReal(cur->first);
            if(_IsInt(v))
                m_factory.db().UpdateAttribute_int(m_item.itemID(), cur->first, v);
            else
                m_factory.db().UpdateAttribute_double(m_item.itemID(), cur->first, v);
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
                m_factory.db().UpdateAttribute_int(m_item.itemID(), cur->first, v);
            else
                m_factory.db().UpdateAttribute_double(m_item.itemID(), cur->first, v);
        }
    }
}

void ItemAttributeMgr::EncodeAttributes(std::map<int32, PyRep *> &into) const {
    // first insert type attributes
    m_item.type().attributes.EncodeAttributes(into);
    // now isert (or overwrite) with our values
    EVEAdvancedAttributeMgr::EncodeAttributes(into);
}

void ItemAttributeMgr::_SendAttributeChange(Attr attr, PyRep *oldValue, PyRep *newValue) {
    if(GetNotify() == false)
        return;

    Client *c = m_factory.entity_list.FindCharacter( item().ownerID() );
    if(c != NULL)
    {
        Notify_OnModuleAttributeChange omac;
        omac.ownerID = m_item.ownerID();
        omac.itemKey = m_item.itemID();
        omac.attributeID = attr;
        omac.time = Win32TimeNow();
        omac.oldValue = oldValue;
        omac.newValue = newValue;

        PyTuple* tmp = omac.Encode();
        c->QueueDestinyEvent(&tmp);
    }
    else
    {
        // delete the reps
        PyDecRef( oldValue );
        PyDecRef( newValue );
    }
}



