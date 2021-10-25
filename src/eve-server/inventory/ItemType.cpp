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
    Author:     Bloody.Rabbit
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "character/Character.h"
#include "effects/EffectsDataMgr.h"
#include "inventory/ItemType.h"
#include "manufacturing/Blueprint.h"
#include "ship/Ship.h"
#include "station/Station.h"

/*
 * ItemType
 */
ItemType::ItemType(uint16 _id, const Inv::TypeData& _data)
: m_type(_data),
  m_defaultFxID(0),
  m_group(Inv::GrpData())
{
    // assert for data consistency
    assert(m_type.id == _id);
    sDataMgr.GetGroup(_data.groupID, m_group);
    assert(_data.groupID == m_group.id);
    m_AttributeMap.clear();

    _log(ITEM__TRACE, "Created ItemType object %p for type %s (%u).", this, name().c_str(), id());
}

ItemType* ItemType::Load(uint16 typeID) {
    return ItemType::Load<ItemType>(typeID);
}

template<class _Ty>
_Ty* ItemType::_LoadType(uint16 typeID, const Inv::TypeData& data) {
    Inv::GrpData gData = Inv::GrpData();
    sDataMgr.GetGroup(data.groupID, gData);

    switch (gData.catID) { // not complete list
        /*
         * not handled / not needed ?
        case EVEDB::invCategories::_System:
        case EVEDB::invCategories::Trading:
        case EVEDB::invCategories::Bonus:
        case EVEDB::invCategories::Reaction:
        */
        case EVEDB::invCategories::Owner: {
            return CharacterType::_LoadType<CharacterType>(typeID, data );
        }
        case EVEDB::invCategories::Station: {
            if (gData.id == EVEDB::invGroups::Station)
                return StationType::_LoadType<StationType>(typeID, data );
            // i dont think we need anything specific for station services here...
            //case EVEDB::invGroups::Station_Services:
        } break;
        case EVEDB::invCategories::Blueprint: {
            return BlueprintType::_LoadType<BlueprintType>(typeID, data );
        }
        case EVEDB::invCategories::Ship:
        case EVEDB::invCategories::Accessories:  // clone, voucher, outpost improvement/upgrade, plex
        case EVEDB::invCategories::Commodity:
        case EVEDB::invCategories::Celestial:
        case EVEDB::invCategories::Skill:
        case EVEDB::invCategories::Charge:
        case EVEDB::invCategories::Material:
        case EVEDB::invCategories::Module:
        case EVEDB::invCategories::Entity:
        case EVEDB::invCategories::Drone:
        case EVEDB::invCategories::Implant:
        case EVEDB::invCategories::Deployable:
        case EVEDB::invCategories::Structure:
        case EVEDB::invCategories::Orbitals:
        case EVEDB::invCategories::AncientRelics:
        case EVEDB::invCategories::Asteroid: {
            // these are called but not sure if they need to be coded specifically
        } break;
        case EVEDB::invCategories::PlanetaryResources:
        case EVEDB::invCategories::PlanetaryCommodities:
        case EVEDB::invCategories::PlanetaryInteraction:  {
            // these probably dont need to be coded specifically
        } break;
        default:
            _log(ITEM__MESSAGE, "type %u (group: %u, cat: %u) called _LoadType, but is not handled.", typeID, gData.id, gData.catID);
             break;
    }

    // nothing special, create generic object:
    return new ItemType(typeID, data);
}

bool ItemType::_Load()
{
    // load type attribs
    std::vector< DmgTypeAttribute > typeAttrVec;
    sDataMgr.GetDgmTypeAttrVec(m_type.id, typeAttrVec);
    for (auto cur : typeAttrVec)
        m_AttributeMap.insert(std::pair<uint16, EvilNumber>(cur.attributeID, cur.value));

    // load attributes that are needed but NOT in default DgmTypeAttributes set (but found in invTypes)
    if (m_type.mass)
        m_AttributeMap.insert(std::pair<uint16, EvilNumber>(AttrMass, m_type.mass));
    if (m_type.radius)
        m_AttributeMap.insert(std::pair<uint16, EvilNumber>(AttrRadius, m_type.radius));
    if (m_type.volume)
        m_AttributeMap.insert(std::pair<uint16, EvilNumber>(AttrVolume, m_type.volume));
    if (m_type.capacity)
        m_AttributeMap.insert(std::pair<uint16, EvilNumber>(AttrCapacity, m_type.capacity));
    if (m_type.race)
        m_AttributeMap.insert(std::pair<uint16, EvilNumber>(AttrRaceID, m_type.race));

    // load required skills and levels into their own map, for later checks
    if (HasAttribute(AttrRequiredSkill1))
        m_reqSkillMap.insert(std::pair<uint16, uint8>((uint16)GetAttribute(AttrRequiredSkill1).get_uint32(), (uint8)GetAttribute(AttrRequiredSkill1Level).get_uint32()));
    if (HasAttribute(AttrRequiredSkill2))
        m_reqSkillMap.insert(std::pair<uint16, uint8>((uint16)GetAttribute(AttrRequiredSkill2).get_uint32(), (uint8)GetAttribute(AttrRequiredSkill2Level).get_uint32()));
    if (HasAttribute(AttrRequiredSkill3))
        m_reqSkillMap.insert(std::pair<uint16, uint8>((uint16)GetAttribute(AttrRequiredSkill3).get_uint32(), (uint8)GetAttribute(AttrRequiredSkill3Level).get_uint32()));
    if (HasAttribute(AttrRequiredSkill4))
        m_reqSkillMap.insert(std::pair<uint16, uint8>((uint16)GetAttribute(AttrRequiredSkill4).get_uint32(), (uint8)GetAttribute(AttrRequiredSkill4Level).get_uint32()));
    if (HasAttribute(AttrRequiredSkill5))
        m_reqSkillMap.insert(std::pair<uint16, uint8>((uint16)GetAttribute(AttrRequiredSkill5).get_uint32(), (uint8)GetAttribute(AttrRequiredSkill5Level).get_uint32()));
    if (HasAttribute(AttrRequiredSkill6))
        m_reqSkillMap.insert(std::pair<uint16, uint8>((uint16)GetAttribute(AttrRequiredSkill6).get_uint32(), (uint8)GetAttribute(AttrRequiredSkill6Level).get_uint32()));

    LoadEffects();

    return true;
}

const void ItemType::CopyAttributes(InventoryItem& itemRef) const
{
    // set attributes in the item's own attrMap.
    for (auto cur : m_AttributeMap)
        itemRef.SetAttribute(cur.first, cur.second, false);
}

const bool ItemType::HasAttribute(const uint16 attributeID) const
{
    AttrMapConstItr itr = m_AttributeMap.find(attributeID);
    if (itr != m_AttributeMap.end())
        return true;
    return false;
}

EvilNumber ItemType::GetAttribute(const uint16 attributeID) const
{
    AttrMapConstItr itr = m_AttributeMap.find(attributeID);
    if (itr != m_AttributeMap.end())
        return itr->second;
    return 0;
}

bool ItemType::HasReqSkill(const uint16 skillID) const
{
    std::map<uint16, uint8>::const_iterator itr = m_reqSkillMap.find(skillID);
    if (itr != m_reqSkillMap.end())
        return true;
    /*  this part will get prequisites for required skills...this isnt right
    for (auto cur : m_reqSkillMap)
        if (sItemFactory.GetType(cur.first)->HasReqSkill(skillID))
            return true;
    */

    return false;
}

void ItemType::LoadEffects()
{
    std::vector< TypeEffects > typeEffMap;
    sFxDataMgr.GetTypeEffect(m_type.id, typeEffMap);

    for (auto cur : typeEffMap) {
        // remove "online" fx from map
        if (cur.effectID == 16)
            continue;
        Effect mEffect(sFxDataMgr.GetEffect(cur.effectID));
        m_stateFxMap.emplace(mEffect.effectState, mEffect);
        if (cur.isDefault) {
            if (m_defaultFxID) {
                // error here to show multiple defaults set for this type
                _log(ITEM__ERROR, "Type %u has multiple default fxID (%u/%u)", m_type.id, m_defaultFxID, cur.effectID);
            }
            m_defaultFxID = cur.effectID;
        }
    }
}

bool ItemType::HasEffect(uint16 effectID) const
{
    std::unordered_multimap<int8, Effect>::const_iterator itr = m_stateFxMap.begin();
    for (; itr != m_stateFxMap.end(); ++itr)
        if (itr->second.effectID == effectID)
            return true;
    return false;
}

void ItemType::GetEffectMap(const int8 state, std::map<uint16, Effect>& effectMap) const
{
    auto itr = m_stateFxMap.equal_range(state);
    for (auto it = itr.first; it != itr.second; ++it)
        effectMap.insert(std::pair<uint16, Effect>(it->second.effectID, it->second));
}


/*
 * ItemData
 */
ItemData::ItemData(
    const char *_name/*""*/,
    uint16 _typeID/*0*/,
    uint32 _ownerID/*ownerSystem*/,
    uint32 _locationID/*locTemp*/,
    EVEItemFlags _flag/*flagNone*/,
    bool _contraband/*false*/,
    bool _singleton/*false*/,
    uint32 _quantity/*0*/,
    const GPoint &_position/*NULL_ORIGIN*/,
    const char *_customInfo/*""*/)
: name(_name),
typeID(_typeID),
ownerID(_ownerID),
locationID(_locationID),
flag(_flag),
contraband(_contraband),
singleton(_singleton),
quantity(_quantity),
position(_position),
customInfo(_customInfo)
{
}

ItemData::ItemData(
    uint16 _typeID,
    uint32 _ownerID,
    uint32 _locationID,
    EVEItemFlags _flag,
    uint32 _quantity,
    const char *_customInfo/*""*/,
    bool _contraband/*false*/)
: name(""),
typeID(_typeID),
ownerID(_ownerID),
locationID(_locationID),
flag(_flag),
contraband(_contraband),
singleton(false),
quantity(_quantity),
position(NULL_ORIGIN),
customInfo(_customInfo)
{
}

ItemData::ItemData(
    uint16 _typeID,
    uint32 _ownerID,
    uint32 _locationID,
    EVEItemFlags _flag,
    const char* _name/*""*/,
    const GPoint& _position/*NULL_ORIGIN*/,
    const char* _customInfo/*""*/,
    bool _contraband/*false*/)
: name(_name),
typeID(_typeID),
ownerID(_ownerID),
locationID(_locationID),
flag(_flag),
contraband(_contraband),
singleton(true),
quantity(1),
position(_position),
customInfo(_customInfo)
{
}


