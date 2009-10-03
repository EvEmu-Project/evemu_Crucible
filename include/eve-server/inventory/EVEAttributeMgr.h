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

#ifndef __EVE_ATTRIBUTE_MGR__H__INCL__
#define __EVE_ATTRIBUTE_MGR__H__INCL__

/*
 * This file contains EVE-specific derivations of attribute managers.
 */

#include "inventory/AttributeMgr.h"

class PyRep;
class ItemType;
class ItemFactory;
class InventoryItem;
class InventoryDB;

class ItemAttributeMgr;

/**
 * Base EVE AttributeManager, defines types to use.
 */
class EVEAttributeMgr : virtual public AttributeMgr<int, double>
{
public:
    /**
     * Returns attribute value as PyRep.
     *
     * @param[in] attr Attribute which value should be retrieved.
     * @return Pointer to new PyRep object; NULL if fails.
     */
    PyRep *PyGet(Attr attr) const;

    /**
     * Builds Int dictionary from attributes.
     *
     * @param[in] into Int dictionary into which values are saved.
     */
    virtual void EncodeAttributes(std::map<int32, PyRep *> &into) const;

    /**
     * Checks whether the attribute is persistent.
     *
     * @param[in] attr Attribute to be checked.
     * @return True if attribute is persistent, false if not.
     */
    static bool IsPersistent(Attr attr) {
        _LoadPersistent();
        return m_persistent[attr];
    }

protected:
    // Turns given value into proper PyRep
    static PyRep *_PyGet(const real_t &v);

    /*
     * Persistent stuff
     */
    static void _LoadPersistent();

    static bool m_persistentLoaded;
    static bool m_persistent[Invalid_Attr];
};

/**
 * Base EVE AdvancedAttributeMgr, defines type to use.
 */
class EVEAdvancedAttributeMgr : public AdvancedAttributeMgr<int, double>, public EVEAttributeMgr
{
public:
    // Uses PyGet instead of _PyGet to include income of attribute value.
    void EncodeAttributes(std::map<int32, PyRep *> &into) const;

    /*
     * These kill warnings about inheritance's dominance
     */
    real_t GetReal(Attr attr) const { return AdvancedAttributeMgr<int_t, real_t>::GetReal(attr); }

    void SetReal(Attr attr, const real_t &v) { AdvancedAttributeMgr<int_t, real_t>::SetReal(attr, v); }
    void SetInt(Attr attr, const int_t &v) { AdvancedAttributeMgr<int_t, real_t>::SetInt(attr, v); }

    void Clear(Attr attr) { AdvancedAttributeMgr<int_t, real_t>::Clear(attr); }
};

/**
 * Attribute manager for type attributes.
 */
class TypeAttributeMgr : public EVEAttributeMgr
{
    friend class ItemAttributeMgr;  // for access to _Get
public:
    TypeAttributeMgr(const ItemType &type) : m_type(type) {}

    /**
     * @return ItemType which this manager is bound to.
     */
    const ItemType &type() const { return(m_type); }

    /**
     * Loads attributes from DB.
     *
     * @param[in] db Database to use.
     * @return True if load was successful, false if not.
     */
    bool Load(InventoryDB &db);

protected:
    const ItemType &m_type;
};

/**
 * Attribute manager for InventoryItem.
 */
class ItemAttributeMgr : public EVEAdvancedAttributeMgr
{
public:
    /**
     * @param[in] factory ItemFactory to use.
     * @param[in] item Item which attributes are managed.
     * @param[in] save Should attribute changes be immediately saved into DB?
     * @param[in] notify Should attribute changes be sent to owner?
     */
    ItemAttributeMgr(ItemFactory &factory, const InventoryItem &item, bool save=true, bool notify=true);

    /**
     * @return InventoryItem which attributes are managed.
     */
    const InventoryItem &item() const { return(m_item); }
    /**
     * @return State of save order - whether all attribute changes should be immediately saved into DB.
     */
    bool GetSave() const { return m_save; }
    /**
     * @return State of notify order - whether all attribute changes should be sent to owner.
     */
    bool GetNotify() const { return m_notify; }

    // Falls to type attributes and then to default if not found
    real_t GetReal(Attr attr) const;

    // Redirection to SetIntEx
    void SetInt(Attr attr, const int_t &v) { SetIntEx(attr, v); }
    /**
     * Changes value of attribute.
     *
     * @param[in] attr Attribute which value is changed.
     * @param[in] v New value of attribute.
     * @param[in] persist Whether new attribute value should be immediately stored to DB.
     */
    void SetIntEx(Attr attr, const int_t &v, bool persist=false);

    // Redirection to SetRealEx
    void SetReal(Attr attr, const real_t &v) { SetRealEx(attr, v); }
    /**
     * Changes value of attribute.
     *
     * @param[in] attr Attribute which value is changed.
     * @param[in] v New value of attribute.
     * @param[in] persist Whether new attribute value should be immediately stored to DB.
     */
    void SetRealEx(Attr attr, const real_t &v, bool persist=false);

    // Clears attribute.
    void Clear(Attr attr);

    // Redirection to DeleteEx
    void Delete() { DeleteEx(); }
    /**
     * Deletes all attributes.
     *
     * @param[in] notify Whether owner should be notified about all changes.
     */
    void DeleteEx(bool notify=false);

    /**
     * Loads attribute values from DB.
     *
     * @param[in] Whether owner should be notified about new values.
     */
    bool Load(bool notify=false);
    /**
     * Saves all attributes to DB.
     */
    void Save() const;

    /**
     * Changes save order - whether all attribute changes should be immediately saved into DB.
     *
     * @param[in] save New status of order.
     */
    void SetSave(bool save) { m_save = save; }
    /**
     * Changes notify order - whether all attribute changes should be sent to owner.
     *
     * @param[in] notify New status of order.
     */
    void SetNotify(bool notify) { m_notify = notify; }

    // Includes type attributes.
    void EncodeAttributes(std::map<int32, PyRep *> &into) const;

    // Additional by-name access
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
    // Creates & sends attribute change notification.
    void _SendAttributeChange(Attr attr, PyRep *oldValue, PyRep *newValue);

    // Member variables:
    ItemFactory &m_factory;

    const InventoryItem &m_item;

    bool m_save;
    bool m_notify;
};

#endif /* __EVE_ATTRIBUTE_MGR__H__INCL__ */


