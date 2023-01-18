/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Update:     Captnoord   - Juni 2010
    Rewrite:    Allan       - 6Feb17
*/

#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "StaticDataMgr.h"
#include "inventory/AttributeMap.h"
#include "inventory/InventoryItem.h"


/*
 * ATTRIBUTE__ADD
 * ATTRIBUTE__CHANGE
 * ATTRIBUTE__DELETE
 * ATTRIBUTE__MISSING
 * ATTRIBUTE__ERROR
 * ATTRIBUTE__WARNING
 * ATTRIBUTE__INFO
 */


AttributeMap::AttributeMap( InventoryItem& item)
: mItem(item)
{
    mAttributes.clear();
}

AttributeMap::~AttributeMap()
{
    //mAttributes.clear();
}


bool AttributeMap::Load(bool reset/*false*/) {
    if (reset) {
        // this will allow total clearing of attribs to eliminate the necessity of 'removing' effects
        mAttributes.clear();
    }
    /* First, we copy default attributes values from our itemType, loaded into memObj when type is loaded */
    // (except char ability scores...dunno why yet)
    mItem.type().CopyAttributes(mItem);

    // check for temp items.  they arent saved to db
    if (!IsTempItem(mItem.itemID()) and !IsNPC(mItem.itemID())) {
        /* load saved attribs from the db, if any, to update the defaults with items current (saved) values*/
        DBQueryResult res;
        if (IsCharacterID(mItem.itemID())) {
            if (!sDatabase.RunQuery(res, "SELECT attributeID, valueInt, valueFloat FROM chrCharacterAttributes WHERE charID=%u", mItem.itemID()))
                _log(DATABASE__ERROR, "AttributeMap Error in db load query: %s", res.error.c_str());
        } else {
            if (!sDatabase.RunQuery(res, "SELECT attributeID, valueInt, valueFloat FROM entity_attributes WHERE itemID=%u", mItem.itemID()))
                _log(DATABASE__ERROR, "AttributeMap Error in db load query: %s", res.error.c_str());
        }

        DBResultRow row;
        EvilNumber value(EvilZero);
        while (res.GetRow(row)) {
            if (row.IsNull(1)) {
                if (row.IsNull(2)) {
                    value = EvilZero;
                } else {
                    value = row.GetDouble(2);
                }
            } else {
                value = row.GetInt64(1);
            }
            SetAttribute(row.GetUInt(0), value, false);
        }
    }
    /* item now has it's own attribute map, and is deleted when item object is destroyed or reset */
    if (is_log_enabled(ATTRIBUTE__INFO))
        _log(ATTRIBUTE__INFO, "AttributeMap::Load()  Loaded %lu attribs for %s.", mAttributes.size(), mItem.name());
    return true;
}

bool AttributeMap::SaveAttributes() {
    return Save();
}

bool AttributeMap::Save() {
    /** @note
     * we are saving:
     *   ability attribs for characters
     *   level, sp and endtime attribs for skills
     *   all attribs for ISEs and CSEs, where applicable
     *   damage and online for modules
     *   damage and quantity for charges, where applicable
     *
     *  note: ship damage saved separately
     */
    if (IsStaticItem(mItem.itemID()))
        return true;

    bool save(false);
    std::vector<Inv::AttrData> attribs;
    attribs.clear();
    AttrMapItr itr = mAttributes.begin(), end = mAttributes.end();
    if (IsCharacterID(mItem.itemID())) {
        for (; itr != end; ++itr) {
            switch (itr->first) {
                case AttrCharisma:
                case AttrIntelligence:
                case AttrMemory:
                case AttrPerception:
                case AttrWillpower:
                case AttrCustomCharismaBonus:
                case AttrCustomWillpowerBonus:
                case AttrCustomPerceptionBonus:
                case AttrCustomMemoryBonus:
                case AttrCustomIntelligenceBonus:
                case AttrCharismaBonus:
                case AttrIntelligenceBonus:
                case AttrMemoryBonus:
                case AttrPerceptionBonus:
                case AttrWillpowerBonus: {
                    if (itr->second == EvilZero)
                        continue;
                    Inv::AttrData data = Inv::AttrData();
                    data.itemID = mItem.itemID();
                    data.attrID = itr->first;
                    data.type = false;
                    data.valueInt = itr->second.get_int();
                    attribs.push_back(data);
                }
            }
        }
    } else {
        bool skill(false), damage(false), owner(false), module(false);
        switch (mItem.categoryID()) {
            case EVEDB::invCategories::Asteroid:    // asteroids and blueprints are NOT saved here
            case EVEDB::invCategories::Blueprint: {
                return false;
            } break;
            case EVEDB::invCategories::Ship: {      // ship attribs saved in shipItem, not here.
                return true;
            } break;
            case EVEDB::invCategories::Skill: {     // save SP, Level and times for skills
                skill = true;
            } break;
            case EVEDB::invCategories::Celestial:       // save all attribs for these
            case EVEDB::invCategories::Structure:
            case EVEDB::invCategories::StructureUpgrade:
            case EVEDB::invCategories::SovereigntyStructure:
            case EVEDB::invCategories::Orbitals:
            case EVEDB::invCategories::Deployable: {
                owner = true;
            } break;
            case EVEDB::invCategories::Module:      // save online state for modules
                module = true;            // we're falling thru on purpose here to save module damage
            case EVEDB::invCategories::Charge:   // remember, crystals and lenses are charges, too.
            case EVEDB::invCategories::Drone:     // this may need more.  check once system is working
            case EVEDB::invCategories::Subsystem: {
                damage = true;
            } break;
        }

        for (; itr != end; ++itr) {
            save = false;
            if (skill)
                if ((itr->first == AttrSkillPoints)
                or  (itr->first == AttrSkillLevel))
                    save = true;
            if (damage)
                if (itr->first == AttrDamage)
                    save = true;
            if (module)
                if (itr->first == AttrOnline)
                    save = true;
            if (save or owner) {
                Inv::AttrData data = Inv::AttrData();
                data.itemID = mItem.itemID();
                data.attrID = itr->first;
                if (itr->second.isInt()) {
                    data.type = false;
                    data.valueInt = itr->second.get_int();
                } else {
                    data.type = true;
                    data.valueFloat = itr->second.get_double();
                }
                attribs.push_back(data);
            }
        }
    }

    if (!attribs.empty())
        ItemDB::SaveAttributes(IsCharacterID(mItem.itemID()), attribs);
    return true;
}


void AttributeMap::SetAttribute(uint16 attrID, EvilNumber& num, bool notify/*true*/)
{
    if (num.isNaN() or num.isInf()) {
        _log(ATTRIBUTE__ERROR, "AttributeMap::SetAttribute() - Something sent NaN or Inf for Attr %u on %s(%u). Returning without modifying.",\
                attrID, mItem.name(), mItem.itemID());
        EvE::traceStack();
        //ResetAttribute(attrID, notify);
        return;
    }
    AttrMapItr itr = mAttributes.find(attrID);
    if (itr == mAttributes.end()) {
        mAttributes.emplace(attrID, num);
        if (notify) {
            Add(attrID, num);
        }
        if (is_log_enabled(ATTRIBUTE__MISSING)) {
            if (num.isFloat()) {
                _log(ATTRIBUTE__MISSING, "Attribute %u not in map.  Adding as %.2f for %s(%u)", \
                        attrID, num.get_float(), mItem.name(), mItem.itemID());
            } else {
                _log(ATTRIBUTE__MISSING, "Attribute %u not in map.  Adding as %lli for %s(%u)", \
                    attrID, num.get_int(), mItem.name(), mItem.itemID());
            }
        }
        if (is_log_enabled(ATTRIBUTE__ADD)) {
            if (num.isFloat()) {
                _log(ATTRIBUTE__ADD, "Attribute %u not in map.  Adding as %.2f for %s(%u)", \
                attrID, num.get_float(), mItem.name(), mItem.itemID());
            } else {
                _log(ATTRIBUTE__ADD, "Attribute %u not in map.  Adding as %lli for %s(%u)", \
                attrID, num.get_int(), mItem.name(), mItem.itemID());
            }
        }

        return;
    }

    if (itr->second == num)
        return;

    if (notify) {
        Change(attrID, itr->second, num);
    }
    if (is_log_enabled(ATTRIBUTE__CHANGE)) {
        if (itr->second.isFloat()) {
            if (num.isFloat()) {
                _log(ATTRIBUTE__CHANGE, "Changing Attribute %u from %.2f to %.2f for %s(%u)", \
                        attrID, itr->second.get_float(), num.get_float(), mItem.name(), mItem.itemID());
            } else {
                _log(ATTRIBUTE__CHANGE, "Changing Attribute %u from %.2f to %lli for %s(%u)", \
                        attrID, itr->second.get_float(), num.get_int(), mItem.name(), mItem.itemID());
            }
        } else {
            if (num.isFloat()) {
                _log(ATTRIBUTE__CHANGE, "Changing Attribute %u from %lli to %.2f for %s(%u)", \
                        attrID, itr->second.get_int(), num.get_float(), mItem.name(), mItem.itemID());
            } else {
                _log(ATTRIBUTE__CHANGE, "Changing Attribute %u from %lli to %lli for %s(%u)", \
                        attrID, itr->second.get_int(), num.get_int(), mItem.name(), mItem.itemID());
            }
        }
    }

    itr->second = num;
}

void AttributeMap::MultiplyAttribute(uint16 attrID, EvilNumber& num, bool notify/*false*/)
{
    if (num.isNaN() or num.isInf()) {
        _log(ATTRIBUTE__ERROR, "AttributeMap::MultiplyAttribute() - Something sent NaN or Inf for %u on %s(%u). Returning without modifying.", \
                attrID, mItem.name(), mItem.itemID());
        EvE::traceStack();
        //ResetAttribute(attrID, notify);
        return;
    }
    if (num == EvilZero) {
        // could this be on purpose?
        //ResetAttribute(attrID, notify);
        _log(ATTRIBUTE__WARNING, "AttributeMap::MultiplyAttribute() - Something sent 0 for %u on %s(%u). Returning without modifying.", \
                attrID, mItem.name(), mItem.itemID());
        EvE::traceStack();
        return;
    }
    AttrMapItr itr = mAttributes.find(attrID);
    if (itr == mAttributes.end())
        return; // it doesnt exist...nothing to do.

    EvilNumber oldValue(itr->second);
    itr->second *= num;

    if (notify)
        Change(attrID, oldValue, itr->second);
}


EvilNumber AttributeMap::GetAttribute(const uint16 attrID) const
{
    AttrMapConstItr itr = mAttributes.find(attrID);
    if (itr != mAttributes.end())
        return itr->second;
    return EvilZero;
}

bool AttributeMap::HasAttribute(const uint16 attrID) const
{
    AttrMapConstItr itr = mAttributes.find(attrID);
    if (itr != mAttributes.end())
        return true;
    return false;
}

bool AttributeMap::HasAttribute(const uint16 attrID, EvilNumber &value) const
{
    AttrMapConstItr itr = mAttributes.find(attrID);
    if (itr != mAttributes.end()) {
        value = itr->second;
        return true;
    }
    value = EvilZero;
    return false;
}

// [eventName,] ownerID, itemID, attributeID, time, newValue, oldValue = change (unless attrib = quantity)
bool AttributeMap::Change(uint16 attrID, EvilNumber& old_val, EvilNumber& new_val) {
    // check for internal skill time data
    if (attrID == AttrStartTime)
        return true;
    // check for changes
    if (old_val == new_val)
        return true;
    // check owner
    if ((mItem.ownerID() == 1)
    and (!IsCharacterID(mItem.itemID())))
        return true;

    // i dunno wtf i put this here....modules maybe?
    /*
    if (IsCharacterID(mItem.ownerID())) {
        Client* pClient = sEntityList.FindClientByCharID(mItem.ownerID());
        if (pClient->IsDocked())
            return true;
    }  */

    Notify_OnModuleAttributeChange modChange;
        modChange.ownerID = mItem.ownerID();

    if (IsFittingSlot(mItem.flag()) and (mItem.categoryID() == EVEDB::invCategories::Charge)) {
        // locationID, flag, typeID = itemKey
        PyTuple* itemKey = new PyTuple(3);
            itemKey->SetItem(0, new PyInt(mItem.locationID()));
            itemKey->SetItem(1, new PyInt(mItem.flag()));
            itemKey->SetItem(2, new PyInt(mItem.typeID()));
        modChange.itemKey = itemKey;
    } else {
        modChange.itemKey = new PyInt(mItem.itemID());
    }

        modChange.attributeID = attrID;
        modChange.time = GetFileTimeNow();
        modChange.newValue = new_val.GetPyObject();
        modChange.oldValue = old_val.GetPyObject();
        /*  not sure about this one yet, used in cap charge (more?)....oldValue is list for this server rsp
         *
                  [PyTuple 7 items]
                    [PyString "OnModuleAttributeChange"]
                    [PyInt 649670823]
                    [PyIntegerVar 1005885567714]
                    [PyInt 18]
                    [PyIntegerVar 129756563388570240]
                    [PyFloat 680.554999862301]
                    [PyList 4 items]
                      [PyFloat 526.692785423517]        <<- old value
                      [PyIntegerVar 129756563391382864] <<- old time?
                      [PyFloat 104400]                  <<- recharge time ??
                      [PyFloat 4860]                    <<-  ??
        */
    return SendChanges(modChange.Encode());
}

bool AttributeMap::Add(uint16 attrID, EvilNumber& num) {
    if (attrID == AttrStartTime)
        return true;

    if ((mItem.ownerID() == 1)
    and (!IsCharacterID(mItem.itemID())))
        return true;
/*
    if (IsCharacterID(mItem.ownerID())) {
        Client* pClient = sEntityList.FindClientByCharID(mItem.ownerID());
        if (pClient->IsDocked())
            return true;
    }
    */

    Notify_OnModuleAttributeChange modChange;
        modChange.ownerID = mItem.ownerID();
    if (IsFittingSlot(mItem.flag()) and (mItem.categoryID() == EVEDB::invCategories::Charge)) {
        // locationID, flag, typeID = itemKey
        PyTuple* itemKey = new PyTuple(3);
            itemKey->SetItem(0, new PyInt(mItem.locationID()));
            itemKey->SetItem(1, new PyInt(mItem.flag()));
            itemKey->SetItem(2, new PyInt(mItem.typeID()));
        modChange.itemKey = itemKey;
    } else {
        modChange.itemKey = new PyInt(mItem.itemID());
    }

        modChange.attributeID = attrID;
        modChange.time = GetFileTimeNow();
        modChange.newValue = num.GetPyObject();
        modChange.oldValue = PyStatic.NewNone();
    return SendChanges(modChange.Encode());
}

bool AttributeMap::SendChanges(PyTuple* attrChange) {
    if (attrChange == nullptr)
        return true;

    /** @todo update this to use my new multicast replacement */
    if (IsPlayerCorp(mItem.ownerID())) {
        // there is no code to get corp AND loc in multicast.  it sends to both
        MulticastTarget mct;
        mct.corporations.insert(mItem.ownerID());
        if (sDataMgr.IsStation(mItem.locationID())) {
            mct.locations.insert(mItem.locationID());
            sEntityList.Multicast("OnItemChange", "*stationid&corpid", &attrChange, mct);
        } else {
            sEntityList.Multicast("OnItemChange", "corpid", &attrChange, mct);
        }
    }

    if (IsCorp(mItem.ownerID()))
        return true;

    Client* pClient(nullptr);
    if (IsCharacterID(mItem.itemID())) {
        pClient = sEntityList.FindClientByCharID(mItem.itemID());
    } else {
        pClient = sEntityList.FindClientByCharID(mItem.ownerID());
    }

    if (pClient == nullptr) {
        _log(PLAYER__WARNING, "AttributeMap::SendChanges() - ownerID for %u not found", mItem.itemID() );
        return false;
    }

    // avoid flooding the client on login
    if (pClient->IsLogin())
        return true;

    if (is_log_enabled(ATTRIBUTE__CHANGE)) {
        _log(ATTRIBUTE__CHANGE, "Sending Attribute changes for %s(%u)", mItem.name(), mItem.itemID());
        attrChange->Dump(ATTRIBUTE__CHANGE, "");
    }

    pClient->QueueDestinyEvent(&attrChange);

    //Save();

    return true;
}

void AttributeMap::ResetAttribute(uint16 attrID, bool notify/*false*/) {
    EvilNumber value(mItem.GetDefaultAttribute(attrID));
    SetAttribute(attrID, value, notify);
}

void AttributeMap::CopyAttributes(std::map< uint16, EvilNumber >& attrMap)
{
    for (auto cur : mAttributes)
        attrMap[cur.first] =  cur.second;
}

void AttributeMap::SaveShipState()
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "REPLACE INTO entity_attributes ";
    Inserts << " (itemID, attributeID, valueInt, valueFloat) VALUES";
    bool save(false);
    AttrMap::iterator itr = mAttributes.find(AttrShieldCharge);
    if (itr != mAttributes.end()) {
        save = true;
        Inserts << "(" << mItem.itemID() << ", " << itr->first << ", ";
        if ( itr->second.get_type() == evil_number_int ) {
            Inserts << itr->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << itr->second.get_double() << ")";
        }
    }
    itr = mAttributes.find(AttrArmorDamage);
    if (itr != mAttributes.end()) {
        if (save)
            Inserts << ",";
        save = true;
        Inserts << "(" << mItem.itemID() << ", " << itr->first << ", ";
        if ( itr->second.get_type() == evil_number_int ) {
            Inserts << itr->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << itr->second.get_double() << ")";
        }
    }
    itr = mAttributes.find(AttrDamage);
    if (itr != mAttributes.end()) {
        if (save)
            Inserts << ",";
        save = true;
        Inserts << "(" << mItem.itemID() << ", " << itr->first << ", ";
        if ( itr->second.get_type() == evil_number_int ) {
            Inserts << itr->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << itr->second.get_double() << ")";
        }
    }
    itr = mAttributes.find(AttrHeatHi);
    if (itr != mAttributes.end()) {
        if (save)
            Inserts << ",";
        save = true;
        Inserts << "(" << mItem.itemID() << ", " << itr->first << ", ";
        if ( itr->second.get_type() == evil_number_int ) {
            Inserts << itr->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << itr->second.get_double() << ")";
        }
    }
    itr = mAttributes.find(AttrHeatMed);
    if (itr != mAttributes.end()) {
        if (save)
            Inserts << ",";
        save = true;
        Inserts << "(" << mItem.itemID() << ", " << itr->first << ", ";
        if ( itr->second.get_type() == evil_number_int ) {
            Inserts << itr->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << itr->second.get_double() << ")";
        }
    }
    itr = mAttributes.find(AttrHeatLow);
    if (itr != mAttributes.end()) {
        if (save)
            Inserts << ",";
        save = true;
        Inserts << "(" << mItem.itemID() << ", " << itr->first << ", ";
        if ( itr->second.get_type() == evil_number_int ) {
            Inserts << itr->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << itr->second.get_double() << ")";
        }
    }

    if (save) {
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str())) {
            _log(DATABASE__ERROR, "SaveShipState - unable to save attributes for %u - %s", mItem.itemID(), err.c_str());
        }
    }
}

// Delete() only called from InventoryItem::Delete()
void AttributeMap::Delete() {
    mAttributes.clear();
}

void AttributeMap::DeleteAttribute(uint16 attrID) {
    _log(ATTRIBUTE__DELETE, "Delete Attribute %u for %s(%u)", attrID, mItem.name(), mItem.itemID());
    AttrMapItr itr = mAttributes.find(attrID);
    if (itr != mAttributes.end()) {
        mAttributes.erase(itr);
        // if it's not in the map, it's not in db, either...
        DBerror err;
        if (IsCharacterID(mItem.itemID())) {
            if (!sDatabase.RunQuery(err, "DELETE FROM chrCharacterAttributes WHERE charID = %u AND attributeID = %u", mItem.itemID(), attrID)) {
                _log(DATABASE__ERROR, "DeleteAttribute - unable to delete attribute %u for %u - %s", attrID, mItem.itemID(), err.c_str());
            }
        } else {
            if (!sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID = %u AND attributeID = %u", mItem.itemID(), attrID)) {
                _log(DATABASE__ERROR, "DeleteAttribute - unable to delete attribute %u for %u - %s", attrID, mItem.itemID(), err.c_str());
            }
        }
    } else {
        _log(ATTRIBUTE__WARNING, "Attribute %u not found in %s(%u) when calling delete ", attrID, mItem.name(), mItem.itemID());
    }
}

AttrMapItr AttributeMap::begin() {
    return mAttributes.begin();
}

AttrMapItr AttributeMap::end() {
    return mAttributes.end();
}
