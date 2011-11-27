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
        return new PyInt(static_cast<int32>(v));
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
        SetIntEx(attr, static_cast<int32>(v), persist);
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
                m_factory.db().UpdateAttribute_int(m_item.itemID(), cur->first, static_cast<int32>(v));
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
                m_factory.db().UpdateAttribute_int(m_item.itemID(), cur->first, static_cast<int32>(v));
            else
                m_factory.db().UpdateAttribute_double(m_item.itemID(), cur->first, v);
        }
    }
}

void ItemAttributeMgr::EncodeAttributes(std::map<int32, PyRep *> &into) const {
    // first insert type attributes
    m_item.type().attributes.EncodeAttributes(into);
    // now insert (or overwrite) with our values
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

/************************************************************************/
/* Start of new attribute system                                        */
/************************************************************************/
AttributeMap::AttributeMap( InventoryItem & item ) : mItem(item), mChanged(false)
{
    // load the initial attributes for this item
    //Load();
}

bool AttributeMap::SetAttribute( uint32 attributeId, EvilNumber &num, bool nofity /*= true*/ )
{
    AttrMapItr itr = mAttributes.find(attributeId);

    /* most attribute have default value's which are related to the item type */
    if (itr == mAttributes.end()) {
        mAttributes.insert(std::make_pair(attributeId, num));
        if (nofity == true)
            return Add(attributeId, num);
        return true;
    }

    // I dono if this should happen... in short... if nothing changes... do nothing
    if (itr->second == num)
        return false;

    // notify dogma to change the attribute, if we are unable to queue the change
    // event. Don't change the value.
    if (nofity == true)
        if (!Change(attributeId, itr->second, num))
            return false;

    itr->second = num;
    return true;
}

EvilNumber AttributeMap::GetAttribute( uint32 attributeId )
{
    AttrMapItr itr = mAttributes.find(attributeId);
    if (itr != mAttributes.end()) {
        return itr->second;
    }
    else
    {
        // ONLY output ERROR message for a "missing" attributeID if it is not in the list of commonly "not found" attributes:
        switch( attributeId )
        {
            case AttrRequiredSkill2:
            case AttrRequiredSkill3:
            case AttrRequiredSkill4:
            case AttrRequiredSkill5:
            case AttrRequiredSkill6:
            case AttrCanFitShipGroup1:
            case AttrCanFitShipGroup2:
            case AttrCanFitShipGroup3:
            case AttrCanFitShipGroup4:
            case AttrCanFitShipType1:
            case AttrCanFitShipType2:
            case AttrCanFitShipType3:
            case AttrCanFitShipType4:
            case AttrSubSystemSlot:
                // DO NOT OUTPUT AN ERROR ON THESE MISSING ATTRIBUTES SINCE THEY ARE COMMONLY "MISSING" FROM MANY ITEMS
                break;

            default:
                sLog.Error("AttributeMap::GetAttribute()", "unable to find attribute: %u for item %u, '%s' of type %u", attributeId, mItem.itemID(), mItem.itemName().c_str(), mItem.typeID());
                break;
        }

        return EvilNumber(0);
    }
}

EvilNumber AttributeMap::GetAttribute( const uint32 attributeId ) const
{
    AttrMapConstItr itr = mAttributes.find(attributeId);
    if (itr != mAttributes.end()) {
        return itr->second;
    }
    else
    {
        // ONLY output ERROR message for a "missing" attributeID if it is not in the list of commonly "not found" attributes:
        switch( attributeId )
        {
            case AttrRequiredSkill2:
            case AttrRequiredSkill3:
            case AttrRequiredSkill4:
            case AttrRequiredSkill5:
            case AttrRequiredSkill6:
            case AttrCanFitShipGroup1:
            case AttrCanFitShipGroup2:
            case AttrCanFitShipGroup3:
            case AttrCanFitShipGroup4:
            case AttrCanFitShipType1:
            case AttrCanFitShipType2:
            case AttrCanFitShipType3:
            case AttrCanFitShipType4:
            case AttrSubSystemSlot:
                // DO NOT OUTPUT AN ERROR ON THESE MISSING ATTRIBUTES SINCE THEY ARE COMMONLY "MISSING" FROM MANY ITEMS
                break;

            default:
                sLog.Error("AttributeMap::GetAttribute()", "unable to find attribute: %u for item %u, '%s' of type %u", attributeId, mItem.itemID(), mItem.itemName().c_str(), mItem.typeID());
                break;
        }

        return EvilNumber(0);
    }
}

bool AttributeMap::HasAttribute(uint32 attributeID)
{
    AttrMapConstItr itr = mAttributes.find(attributeID);
    if (itr != mAttributes.end())
        return true;
    else
        return false;
}

bool AttributeMap::Change( uint32 attributeID, EvilNumber& old_val, EvilNumber& new_val )
{
    mChanged = true;
    PyTuple* AttrChange = new PyTuple(7);
    AttrChange->SetItem(0, new PyString("OnModuleAttributeChange"));
    AttrChange->SetItem(1, new PyInt(mItem.ownerID()));
    AttrChange->SetItem(2, new PyInt(mItem.itemID()));
    AttrChange->SetItem(3, new PyInt(attributeID));
    AttrChange->SetItem(4, new PyLong(Win32TimeNow()));
    AttrChange->SetItem(5, old_val.GetPyObject());
    AttrChange->SetItem(6, new_val.GetPyObject());

    return SendAttributeChanges(AttrChange);
}

bool AttributeMap::Add( uint32 attributeID, EvilNumber& num )
{
    mChanged = true;
    PyTuple* AttrChange = new PyTuple(7);
    AttrChange->SetItem(0, new PyString( "OnModuleAttributeChange" ));
    AttrChange->SetItem(1, new PyInt( mItem.ownerID() ));
    AttrChange->SetItem(2, new PyInt( mItem.itemID() ));
    AttrChange->SetItem(3, new PyInt( attributeID ));
    AttrChange->SetItem(4, new PyLong( Win32TimeNow() ));
    AttrChange->SetItem(5, num.GetPyObject());
    AttrChange->SetItem(6, num.GetPyObject());

    return SendAttributeChanges(AttrChange);
}

bool AttributeMap::SendAttributeChanges( PyTuple* attrChange )
{
    if (attrChange == NULL)
    {
        sLog.Error("AttributeMap", "unable to send NULL packet");
        return false;
    }

    // Oh hell, this character finding needs to be optimized ( redesigned so its not needed.. ).
    if( (mItem.ownerID() == 1) || (IsStation(mItem.itemID())) )
    {
        // This item is owned by the EVE System either directly, as in the case of a character object,
        // or indirectly, as in the case of a Station, which is owned by the corporation that runs it.
        // So, we don't need to queue up Destiny events in these cases.
        return true;
    }
    else
    {
        Client *client = sEntityList.FindCharacter(mItem.ownerID());
        //Client *client = this->mItem.GetItemFactory()->GetUsingClient();

        if (client == NULL)
        {
            sLog.Error("AttributeMap::SendAttributeChanges()", "unable to find client:%u", mItem.ownerID());
            //return false;
            return true;
        }
        else
        {
            if( client->Destiny() == NULL )
            {
                sLog.Warning( "AttributeMap::SendAttributeChanges()", "client->Destiny() returned NULL" );
                //return false;
            }
            else
                client->QueueDestinyEvent(&attrChange);

            return true;
        }
    }
}

bool AttributeMap::ResetAttribute(uint32 attrID, bool notify)
{
	//this isn't particularly efficient, but until I write a better solution, this will do
	DBQueryResult res;

	if(!sDatabase.RunQuery(res, "SELECT * FROM dgmtypeattributes WHERE typeID='%u'", mItem.typeID())) {
		sLog.Error("AttributeMap", "Error in db load query: %s", res.error.c_str());
		return false;
	}
	
	DBResultRow row;
	EvilNumber attrVal;
	uint32 attributeID;

	int amount = res.GetRowCount();
	for (int i = 0; i < amount; i++)
	{
		res.GetRow(row);
		attributeID = row.GetUInt(1);
		if( attributeID == attrID )
		{
			if(!row.IsNull(2))
				attrVal = row.GetUInt64(2);
			else
				attrVal = row.GetDouble(3);

			SetAttribute(attributeID, attrVal, notify);
		}
	}

	return true;

}

bool AttributeMap::Load()
{
    /* then we possibly overwrite the attributes value's with the default's.. */
    DgmTypeAttributeSet *attr_set = sDgmTypeAttrMgr.GetDmgTypeAttributeSet( mItem.typeID() );
    if (attr_set == NULL)
        return false;

    DgmTypeAttributeSet::AttrSetItr itr = attr_set->attributeset.begin();

    for (; itr != attr_set->attributeset.end(); itr++)
        SetAttribute((*itr)->attributeID, (*itr)->number, false);

    /* first we load the saved attributes from the db */
    DBQueryResult res;

    if(!sDatabase.RunQuery(res, "SELECT * FROM entity_attributes WHERE itemID='%u'", mItem.itemID())) {
        sLog.Error("AttributeMap", "Error in db load query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;

    int amount = res.GetRowCount();
    for (int i = 0; i < amount; i++)
    {
        EvilNumber attr_value;
        res.GetRow(row);
        uint32 attributeID = row.GetUInt(1);
        if (!row.IsNull(2))
            attr_value = row.GetInt64(2);
        else
            attr_value = row.GetDouble(3);
        SetAttribute(attributeID, attr_value, false);
    }

    return true;

/*
	/// EXISTING AttributeMap::Load() function
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,"SELECT * FROM entity_attributes WHERE itemID='%u'", mItem.itemID())) {
        sLog.Error("AttributeMap", "Error in db load query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;

    int amount = res.GetRowCount();

    // Right now, assume that we need to load all attributes with default values from dgmTypeAttributes table
    // IF AND ONLY IF the number of attributes pulled from the entity_attributes table for this item is ZERO:
    if( amount > 0 )
    {
        // This item was found in the 'entity_attributes' table, so load all attributes found there
        // into the Attribute Map for this item:
        for (int i = 0; i < amount; i++)
        {
            res.GetRow(row);
            EvilNumber attr_value;
            uint32 attributeID = row.GetUInt(1);
            if ( !row.IsNull(2) )
                attr_value = row.GetInt64(2);
            else if( !row.IsNull(3) )
                attr_value = row.GetDouble(3);
            else
                sLog.Error( "AttributeMap::Load()", "Both valueInt and valueFloat fields of this (itemID,attributeID) = (%u,%u) are NULL.", row.GetInt(0), attributeID );

            SetAttribute(attributeID, attr_value, false);
            //Add(attributeID, attr_value);
        }
    }
    else
    {
        // This item was NOT found in the 'entity_attributes' table, so let's assume that
        // this item was just created.
        // 1) Get complete list of attributes with default values from dgmTypeAttributes table using the item's typeID:
        DgmTypeAttributeSet *attr_set = sDgmTypeAttrMgr.GetDmgTypeAttributeSet( mItem.typeID() );
        if (attr_set == NULL)
            return false;

        DgmTypeAttributeSet::AttrSetItr itr = attr_set->attributeset.begin();
    
        // Store all these attributes to the item's AttributeMap
        for (; itr != attr_set->attributeset.end(); itr++)
        {
            SetAttribute((*itr)->attributeID, (*itr)->number, false);
            //Add((*itr)->attributeID, (*itr)->number);
        }

        // 2) Save these newly created and loaded attributes to the 'entity_attributes' table
        SaveAttributes();
    }

    return true;
*/
}

bool AttributeMap::SaveIntAttribute(uint32 attributeID, int64 value)
{
    // SAVE INTEGER ATTRIBUTE
    DBerror err;
    if(!sDatabase.RunQuery(err,
        "REPLACE INTO entity_attributes"
        "   (itemID, attributeID, valueInt, valueFloat)"
        " VALUES"
        "   (%u, %u, %d, NULL)",
        mItem.itemID(), attributeID, value)
    ) {
        codelog(SERVICE__ERROR, "Failed to store attribute %d for item %u: %s", attributeID, mItem.itemID(), err.c_str());
        return false;
    }

    return true;
}

bool AttributeMap::SaveFloatAttribute(uint32 attributeID, double value)
{
    // SAVE FLOAT ATTRIBUTE
    DBerror err;
    if(!sDatabase.RunQuery(err,
        "REPLACE INTO entity_attributes"
        "   (itemID, attributeID, valueInt, valueFloat)"
        " VALUES"
        "   (%u, %u, NULL, %f)",
        mItem.itemID(), attributeID, value)
    ) {
        codelog(SERVICE__ERROR, "Failed to store attribute %d for item %u: %s", attributeID, mItem.itemID(), err.c_str());
        return false;
    }

    return true;
}

/* hmmm only save 'state' related attributes... and calculate the rest on the fly....*/
/* we should save skills */
bool AttributeMap::Save()
{
    /* if nothing changed... it means this action has been successful we return true... */
    if (mChanged == false)
        return true;

    AttrMapItr itr = mAttributes.begin();
    AttrMapItr itr_end = mAttributes.end();
    for (; itr != itr_end; itr++)
    {
        if ( itr->second.get_type() == evil_number_int ) {

            DBerror err;
            bool success = sDatabase.RunQuery(err,
                "REPLACE INTO entity_attributes (itemID, attributeID, valueInt, valueFloat) VALUES (%u, %u, "I64d", NULL)",
                mItem.itemID(), itr->first, itr->second.get_int());

            if (!success)
                sLog.Error("AttributeMap", "unable to save attribute");

        } else if (itr->second.get_type() == evil_number_float ) {

            DBerror err;
            bool success = sDatabase.RunQuery(err,
                "REPLACE INTO entity_attributes (itemID, attributeID, valueInt, valueFloat) VALUES (%u, %u, NULL, %f)",
                mItem.itemID(), itr->first, itr->second.get_float());

            if (!success)
                sLog.Error("AttributeMap", "unable to save attribute");
        }
    }

    mChanged = false;

    return true;
}


bool AttributeMap::SaveAttributes()
{
    return Save();
}

bool AttributeMap::Delete()
{
    // Remove all attributes from the entity_attributes table for this item:
    DBerror err;
    if(!sDatabase.RunQuery(err,
        "DELETE"
        " FROM entity_attributes"
        " WHERE itemID=%u",
        mItem.itemID()
    ))
    {
        sLog.Error( "", "Failed to delete item %u: %s", mItem.itemID(), err.c_str());
        return false;
    }
    return true;
}

AttributeMap::AttrMapItr AttributeMap::begin()
{
    return mAttributes.begin();
}

AttributeMap::AttrMapItr AttributeMap::end()
{
    return mAttributes.end();
}
/************************************************************************/
/* End of new attribute system                                          */
/************************************************************************/

