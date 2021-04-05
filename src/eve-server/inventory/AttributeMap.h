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
    Rewrite:    Allan
*/

#ifndef __EVE_ATTRIBUTE_MGR__H__INCL__
#define __EVE_ATTRIBUTE_MGR__H__INCL__

#include "./eve-common.h"

#include "inventory/InventoryDB.h"

typedef std::map<uint16, EvilNumber>    AttrMap;
typedef AttrMap::iterator               AttrMapItr;
typedef AttrMap::const_iterator         AttrMapConstItr;

class PyTuple;
class InventoryItem;

class AttributeMap
{
public:
    AttributeMap(InventoryItem& item);
    ~AttributeMap() noexcept;

    void SetAttribute(uint16 attrID, EvilNumber& num, bool notify=true);
    void MultiplyAttribute(uint16 attrID, EvilNumber& num, bool notify=false);

    EvilNumber GetAttribute(const uint16 attrID) const;

    bool HasAttribute(const uint16 attrID) const;
    bool HasAttribute(const uint16 attrID, EvilNumber& value) const;

    bool Save();

    void Delete();
    void DeleteAttribute(uint16 attrID);

    // load the default attributes that come with the item's typeID
    bool Load(bool reset=false);

    /* only save the ship damage and heat. other attribs are calculated when ship activated */
    void SaveShipState();
    bool SaveAttributes();

    void ResetAttribute(uint16 attrID, bool notify=false);
    void CopyAttributes(std::map<uint16, EvilNumber>& attrMap);

    /**
     * @brief return the begin iterator of the AttributeMap
     * @return the begin iterator of the AttributeMap
     * @note this way to solve the attribute system problems are quite hacky... but atm its needed
     */
    AttrMapItr begin();

    /**
     * @brief return the end iterator of the AttributeMap
     * @return the end iterator of the AttributeMap
     * @note this way to solve the attribute system problems are quite hacky... but atm its needed
     */
    AttrMapItr end();

protected:
    /**
     * @brief internal function to handle the change.
     *
     * @param[in] attributeId the attribute id that needs to be changed.
     * @param[in] num the number the attribute needs to be changed in.
     *
     * @retval true  The attribute change has successfully been set and queued.
     * @retval false The attribute change has not been queued but has been changed.
     */
    bool Change(uint16 attrID, EvilNumber& old_val, EvilNumber& new_val);

    /**
     * @brief internal function to handle adding attributes.
     *
     * @param[in] attributeId the attribute id that needs to be added.
     * @param[in] num the number the attribute needs to be set to.
     *
     * @retval true  The attribute has successfully been added and queued.
     * @retval false The attribute addition has not been queued and not been changed.
     */
    bool Add(uint16 attrID, EvilNumber& num);

    /**
     * @brief queue the attribute changes into the QueueDestinyEvent system.
     *
     * @param[in] attrChange the attribute id that needs to be added.
     *
     * @retval true  The attribute has successfully been added and queued.
     * @retval false The attribute addition has not been queued and not been changed.
     */
    bool SendChanges(PyTuple* attrChange);

    InventoryItem& mItem;

    AttrMap mAttributes;

private:
    InventoryDB m_db;

};

#endif /* __EVE_ATTRIBUTE_MGR__H__INCL__ */

/*      self.electronicAttributes = {const.attributeScanGravimetricStrengthBonus: const.attributeScanGravimetricStrength,
         const.attributeScanLadarStrengthBonus: const.attributeScanLadarStrength,
         const.attributeScanMagnetometricStrengthBonus: const.attributeScanMagnetometricStrength,
         const.attributeScanRadarStrengthBonus: const.attributeScanRadarStrength}
        self.propulsionAttributes = {const.attributePropulsionFusionStrengthBonus: const.attributePropulsionFusionStrength,
         const.attributePropulsionIonStrengthBonus: const.attributePropulsionIonStrength,
         const.attributePropulsionMagpulseStrengthBonus: const.attributePropulsionMagpulseStrength,
         const.attributePropulsionPlasmaStrengthBonus: const.attributePropulsionPlasmaStrength}
        self.heatAttributes = {'heatAbsorbtionRateHi': 'heatHi',
         'heatAbsorbtionRateMed': 'heatMed',
         'heatAbsorbtionRateLow': 'heatLow'}

    def ApplyAttributeChange(self, ownerID, itemKey, attributeID, time, newValue, oldValue, scatterAttr = True):
        item = None
        if self.invitems.has_key(itemKey):
            item = self.invitems[itemKey]
        elif self.sublocationsByKey.has_key(itemKey):
            item = self.sublocationsByKey[itemKey]
        if item is None or not self.attributesByItemAttribute.has_key(itemKey):
            if itemKey not in self.attributesByItemAttribute:
                self.godma.LogInfo('ApplyAttributeChange - item not found', item, ownerID, itemKey, attributeID, time, newValue, oldValue, scatterAttr)
            return
        attributeName = cfg.dgmattribs.Get(attributeID).attributeName
        att = getattr(item, attributeName, 'not there')
        self.attributesByItemAttribute[itemKey][attributeName] = newValue
        if attributeName in self.heatAttributes.values():
            self.UpdateOverloadedAttributesHeat(attributeName, time, newValue)
        if attributeName in self.heatAttributes.keys():
            self.UpdateOverloadedAttributesTau(attributeName, time, newValue)
        if self.chargedAttributeTauCaps.has_key(attributeName):
            if not self.chargedAttributesByItemAttribute.has_key(itemKey):
                self.chargedAttributesByItemAttribute[itemKey] = {}
            self.CreateChargedAttribute(itemKey, attributeName, newValue, time, self.chargedAttributeTauCaps[attributeName])
        elif self.chargedAttributeTauCapsByMax.has_key(attributeName):
            for chargeAttName in self.chargedAttributeTauCapsByMax[attributeName]:
                if self.chargedAttributesByItemAttribute.has_key(itemKey):
                    if self.chargedAttributesByItemAttribute[itemKey].has_key(chargeAttName):
                        self.godma.LogInfo('CHARGEEEEEE')
                        charge = self.GetAttribute(itemKey, chargeAttName)
                        self.CreateChargedAttribute(itemKey, chargeAttName, charge, time, self.chargedAttributeTauCaps[chargeAttName])
                        break
                    elif self.attributesByItemAttribute[itemKey].get(self.chargedAttributeTauCaps[chargeAttName][0]) and self.attributesByItemAttribute[itemKey].get(self.chargedAttributeTauCaps[chargeAttName][1]):
                        self.CreateChargedAttribute(itemKey, chargeAttName, self.attributesByItemAttribute[itemKey][chargeAttName], time, self.chargedAttributeTauCaps[chargeAttName])
                        break

        elif self.chargedAttributesByTau.has_key(attributeName):
            for attName in self.chargedAttributesByTau[attributeName]:
                if self.chargedAttributesByItemAttribute.has_key(itemKey):
                    if self.chargedAttributesByItemAttribute[itemKey].has_key(attName):
                        self.godma.LogInfo('CHARGEEEEEE')
                        charge = self.GetAttribute(itemKey, attName)
                        self.CreateChargedAttribute(itemKey, attName, charge, time, self.chargedAttributeTauCaps[attName])
                        break

        self.godma.LogInfo('Attribute', attributeName, 'of module', itemKey, 'changed from', oldValue, 'to', newValue, 'value I had was', att)
        if attributeID in (const.attributeCapacity,
         const.attributeDroneCapacity,
         const.attributeShipMaintenanceBayCapacity,
         const.attributeCorporateHangarCapacity):
            sm.ScatterEvent('OnCapacityChange', itemKey)
        elif attributeID == const.attributeQuantity:
            self.godma.LogInfo('Changing quantity from', att, 'to', newValue)
            if newValue == att:
                return
            locationID, flag, typeID = itemKey
            currSubLocation = self.GetSubLocation(locationID, flag)
            if currSubLocation and currSubLocation.typeID != typeID:
                self.godma.LogWarn("Trying to modify quantity of sublocation that isn't loaded with my type", currSubLocation, itemKey)
                return
            if newValue == 0:
                self.godma.LogInfo('SubLoc.quantityExpelled', itemKey, (oldValue, newValue))
                locationID, flag = itemKey[0], itemKey[1]
                if self.sublocationsByLocationFlag.has_key(locationID) and self.sublocationsByLocationFlag[locationID].has_key(flag):
                    del self.sublocationsByLocationFlag[locationID][flag]
                del self.sublocationsByKey[itemKey]
            typeOb = cfg.invtypes.Get(typeID)
            item = blue.DBRow(self.godma.sublocrd, [itemKey,
             typeID,
             None,
             locationID,
             flag,
             newValue,
             typeOb.groupID,
             typeOb.categoryID,
             None])
            sm.ScatterEvent('OnItemChange', item, {const.ixStackSize: oldValue})
        if attributeID == const.attributeIsOnline:
            sm.ScatterEvent('OnModuleOnlineChange', item, oldValue, newValue)
            if newValue == 0 and oldValue == 1:
                if item.online.isActive:
                    self.godma.LogWarn('Module put offline silently, faking event', itemKey)
                    self.OnGodmaShipEffect(itemKey, const.effectOnline, blue.os.GetSimTime(), 0, 0, [itemKey,
                     item.ownerID,
                     item.locationID,
                     None,
                     None,
                     [],
                     const.effectOnline], None, -1, -1, None, None)


    def UpdateAttribute(self, itemID, attributes, time):
        self.attributesByItemAttribute[itemID] = attributes
        if time is None:
            time = blue.os.GetSimTime()
        for k in attributes.iterkeys():
            if k in self.heatAttributes.values():
                if not self.overloadedAttributes.has_key(k):
                    self.overloadedAttributes[k] = [0, 0, 0]
                self.overloadedAttributes[k][0] = attributes[k]
                self.overloadedAttributes[k][1] = time
            if k in self.heatAttributes.keys():
                if not self.overloadedAttributes.has_key(self.heatAttributes[k]):
                    self.overloadedAttributes[self.heatAttributes[k]] = [0, 0, 0]
                self.overloadedAttributes[self.heatAttributes[k]][2] = attributes[k]
            if self.chargedAttributeTauCaps.has_key(k):
                if not self.chargedAttributesByItemAttribute.has_key(itemID):
                    self.chargedAttributesByItemAttribute[itemID] = {}
                if self.attributesByItemAttribute[itemID].get(self.chargedAttributeTauCaps[k][0]) and self.attributesByItemAttribute[itemID].get(self.chargedAttributeTauCaps[k][1]):
                    self.CreateChargedAttribute(itemID, k, attributes[k], time, self.chargedAttributeTauCaps[k])

    def CreateChargedAttribute(self, itemID, attribute, value, time, taucap):
        tau, cap = taucap
        tauValue = self.GetAttribute(itemID, tau) / 5.0
        capValue = self.GetAttribute(itemID, cap)
        self.chargedAttributesByItemAttribute[itemID][attribute] = [value,
         time,
         tauValue,
         capValue]
        self.godma.LogInfo('CreateChargedAttribute', itemID, attribute, time, self.chargedAttributesByItemAttribute[itemID][attribute])

    */