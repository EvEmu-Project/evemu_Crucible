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
    Author:        Bloody.Rabbit
    Rewrite:    Allan
*/

#ifndef __ITEM_TYPE__H__INCL__
#define __ITEM_TYPE__H__INCL__

#include <unordered_map>

#include "StaticDataMgr.h"
#include "effects/EffectsData.h"
//#include "inventory/AttributeMap.h"
//#include "inventory/ItemFactory.h"

//struct Effect;

/*
 * LOADING INVOKATION EXPLANATION:
 *     (written by original author (Bloody.Rabbit?)) [updated version by allan in InventoryItem.h]
 * ItemType and InventoryItem classes and their children have special loading. Every such type has following methods:
 *
 *  static Load(<identifier>):
 *    Merges static and virtual loading trees.
 *    First calls static _Load() to create desired object and
 *    then calls its virtual _Load() (if the type has any).
 *
 *  static _Load(<identifier>[, <data-argument>, ...]):
 *    These functions gradually, one by one, load any data needed to create desired
 *    type and in the end they create the type object.
 *
 *  virtual _Load() (optional):
 *    Performs any post-construction loading.
 */

/*
 * Class which maintains type data.
 */
class ItemType {
public:
    virtual ~ItemType()                                 { /* do nothing here */ }

    /* Helper methods  */
    uint16 id() const                                   { return m_type.id; }
    uint16 groupID() const                              { return m_group.id; }
    const std::string &groupName() const                { return m_group.name; }
    uint8 categoryID() const                            { return m_group.catID; }

    float radius() const                                { return m_type.radius; }
    float mass() const                                  { return m_type.mass; }
    float volume() const                                { return m_type.volume; }
    float capacity() const                              { return m_type.capacity; }
    uint8 race() const                                  { return m_type.race; }

    const std::string &name() const                     { return m_type.name; }
    const std::string &description() const              { return m_type.description; }

    uint16 portionSize() const                          { return m_type.portionSize; }
    double basePrice() const                            { return m_type.basePrice; }
    uint32 marketGroupID() const                        { return m_type.marketGroupID; }
    float chanceOfDuplicating() const                   { return m_type.chanceOfDuplicating; }

    bool published() const                              { return m_type.published; }
    bool refinable() const                              { return m_type.isRefinable; }
    bool recyclable() const                             { return m_type.isRecyclable; }

    /* new attribute system */
    const bool HasAttribute(const uint16 attributeID) const;
    EvilNumber GetAttribute(const uint16 attributeID) const;
    const void CopyAttributes(InventoryItem& itemRef) const;

    bool HasReqSkill(const uint16 skillID) const;

    /* new effects processing system */
    void GetEffectMap(const int8 state, std::map<uint16, Effect>& effectMap) const;
    uint16 GetDefaultEffect() const                     { return m_defaultFxID; }

    bool HasEffect(uint16 effectID) const;

    // load method
    static ItemType* Load( uint16 typeID);

protected:
    ItemType(uint16 _id, const Inv::TypeData &_data);

    /*
     * Member functions
     */
    // Template helper:
    template<class _Ty>
    static _Ty *Load( uint16 typeID)
    {
        // static load
        _Ty *t = _Ty::template _Load<_Ty>(typeID);
        if (t == nullptr)
            return nullptr;

        // dynamic load
        if (!t->_Load()) {
            delete t;
            return nullptr;
        }

        return t;
    }

    // Template loader:
    template<class _Ty>
    static _Ty *_Load(uint16 typeID)
    {
        // pull data
        Inv::TypeData data = Inv::TypeData();
        sDataMgr.GetType(typeID, data);
        if (data.id == 0)
            return nullptr;
        /** @todo  this needs work.  other items we need are "non-published" */
        if (data.groupID > 23)  // gID < 23 are map items.  will need to search for others
            if (!data.published)
                return nullptr;

        return _Ty::template _LoadType<_Ty>(typeID, data );
    }

    // Actual loading stuff:
    template<class _Ty>
    static _Ty *_LoadType( uint16 typeID, const Inv::TypeData &data);

    virtual bool _Load();

    void LoadEffects();

public:
    // i dont like this......MUST fix later....
    // UD: ive no clue what i didnt like about it
    std::unordered_multimap<int8, Effect> m_stateFxMap; // k,v map of state, data   -to search by state

private:
    Inv::GrpData m_group;
    Inv::TypeData m_type;
    uint16 m_defaultFxID;                 // default effectID

    std::map<uint16, uint8> m_reqSkillMap;              // k,v map of required skill, level for this ItemType, if any.
    std::map<uint16, EvilNumber> m_AttributeMap;        // k,v map of attributeID, value

};

/*
 * Simple container for raw item data.
 */
class ItemData {
public:
    // default constructor:
    ItemData( const char *_name = "", uint16 _typeID = 0, uint32 _ownerID = ownerSystem, uint32 _locationID = locTemp,
              EVEItemFlags _flag = flagNone, bool _contraband = false, bool _singleton = false, uint32 _quantity = 0,
              const GPoint &_position = NULL_ORIGIN, const char *_customInfo = "");

    // non-singleton constructor:
    ItemData( uint16 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, uint32 _quantity,
              const char *_customInfo = "", bool _contraband = false);

    // Singleton constructor:
    ItemData( uint16 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, const char *_name = "",
              const GPoint &_position = NULL_ORIGIN, const char *_customInfo = "", bool _contraband = false);

    // Content:
    bool            contraband :1;
    bool            singleton :1;            // singletonBlueprintCopy = 2
    EVEItemFlags    flag;
    uint16          typeID;
    int32           quantity;
    uint32          ownerID;
    uint32          locationID;
    GPoint          position;
    std::string     name;
    std::string     customInfo;
};

#endif /* __ITEM_TYPE__H__INCL__ */



