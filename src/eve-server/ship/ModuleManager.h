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
    Author:        Luck, Aknor Jaden
*/


#ifndef __MODULEMANAGER_H_INCL__
#define __MODULEMANAGER_H_INCL__

class DestinyManager;
class InventoryItem;
class GenericModule;
class SystemEntity;
class Client;
class ModuleManager;
class Basic_Log;

#include "ship/modules/Modules.h"
#include "ship/modules/ModuleDefs.h"


//////////////////////////////////////////////////////////////////////////////////
//
// --- Explanation of all this confusing crap in here:
//
// We need a map of maps to store modifiers from activated effects of modules, skills, ships, implants, subsystems, rigs, etc
// So, what we've done here is to create a std::map<uint32 attributeID, ModifierMap>  This contains a ModifierMap class object
// for each attributeID added to this outer map.  For each attribute that is modified by one or more originators (module, rig,
// ship, skill, implant, etc), an inner map is created and added to this outer map.  The ModifierMap class contains this inner map,
// which is actually a std::multimap<double modifierValue, Modifier modifierObject>  The reason we use a multimap here is that
// a multimap allows multiple key values that are exactly the same, which you'll get if you fit more than one module of the exact
// same type.  The modifier value is used as the key since we need the list of all modifiers for a single attributeID to be sorted
// largest modifier value to smallest to zero to smaller negative modifier values to largest negative modifier value.  This sort
// is needed in order to properly apply stacking penalties.  The stacking penalties as explained here http://wiki.eveuniversity.org/Eve_math
// indicate that the largest modifier value does not get penalized, but the 2nd largest on down get increasingly larger penalties applied.
// The penalties will NOT be applied into the Modifier class objects, but only when the ModuleManager class methods process these modifiers
// onto the attributes AFTER any Module class objects are called to change state and apply either new or updated modifiers into
// these Modifier Maps.
//
// Each Module class object will make the public calls to the ModuleManager to add Modifiers to the Modifier Map for a particular
// attributeID, so let's talk about these classes.
//
// The Modifier class just contains basic information on a single modifier value inserted by an 'originator', some source of the
// modifier value (module, skill, ship, implant, rig, subsystem, etc).  Every Modifier class object created by an originator
// is inserted into a ModifierMapType, which is a std::multimap<double modifierValue, ModifierRef modifierRef> structure.  It is
// a std::multimap<> with double as key value, which are the modifier values themselves so that the multimap can sort the pairs
// according to the values of the modifier values.
//
// Each of these ModifierMapType structures are inserted into one of a handful of ModifierMaps structures, each of which are
// std::map<uint32 attributeID, ModifierMap * modifierMap>  Each pair is keyed to the attributeID for the attribute that will
// be modified by the list of modifier values stored in the ModifierMap object.  There are multiple ModifierMaps objects in the
// ModuleManager class, one for Subsystems, one for Ships and Skills, one for Modules and Rigs, one for Implants, and one for
// Remotely applied modifiers from external hostile entities.
//
// More to follow, and this will be copied to http://wiki.evemu.org
//
//     -- Aknor Jaden
//
//////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// Modifier classes containing all data to modify an attribute
#pragma region Modifier

class Modifier
: public RefObject
{
public:
    Modifier(uint32 originatorID, uint32 targetAttributeID, uint32 targetID, bool penaltiesApply, double modifierValue, uint32 calcTypeID, uint32 revCalcTypeID)
    : RefObject( 0 )
    {
        m_OriginatorID = originatorID;
        m_TargetAttributeID = targetAttributeID;
        m_TargetID = targetID;
        m_bPenaltiesApply = penaltiesApply;
        m_ModifierValue = modifierValue;
        m_CalculationTypeID = calcTypeID;
        m_ReverseCalculationTypeID = revCalcTypeID;
    }

    ~Modifier();
    
    double GetModifierValue() { return m_ModifierValue; }
    void SetModifierValue(double newModifierValue) { m_ModifierValue = newModifierValue; }
    uint32 GetOriginatorID() { return m_OriginatorID; }

protected:
    uint32 m_OriginatorID;
    uint32 m_TargetAttributeID;
    uint32 m_TargetID;
    bool m_bPenaltiesApply;
    double m_ModifierValue;
    uint32 m_CalculationTypeID;
    uint32 m_ReverseCalculationTypeID;
};

typedef RefPtr<Modifier> ModifierRef;

typedef std::multimap<double, ModifierRef> ModifierMapType;     // The ModifierRef is NOT owned by the owner of instances of this type

class ModifierMap
{
public:
    ModifierMap() { m_MapIsDirty = false; }
    ~ModifierMap();

    bool m_MapIsDirty;
    ModifierMapType m_ModifierMap;   // Key= modifier value, Value= Modifier class object containing all data describing this modifier for this attribute
};

typedef std::map<uint32, ModifierMap *> ModifierMaps;   // Key= attributeID, Value= ModifierMap class object containing a map of all modifiers for this attribute

#pragma endregion
/////////////////////////////// END MODIFIER /////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// Container for all ships modules
#pragma region ModuleContainer

class ModuleContainer
{
public:
    ModuleContainer(uint32 lowSlots, uint32 medSlots, uint32 highSlots, uint32 rigSlots, uint32 subSystemSlots,
        uint32 turretSlots, uint32 launcherSlots, ModuleManager * myManager);
    ~ModuleContainer();

    bool AddModule(uint32 flag, GenericModule * mod);
    bool RemoveModule(EVEItemFlags flag);
    bool RemoveModule(uint32 itemID);
    GenericModule * GetModule(EVEItemFlags flag);
    GenericModule * GetModule(uint32 itemID);

	uint32 GetAvailableSlotInBank(EveEffectEnum slotBank);

    int NumberOfSameType(uint32 typeID);

    //batch processes handlers
    void Process();
    void OfflineAll();
    void OnlineAll();
    void DeactivateAll();
    void UnloadAll();

    //useful accessors
    bool isHighPower(uint32 itemID);
    bool isMediumPower(uint32 itemID);
    bool isLowPower(uint32 itemID);
    bool isRig(uint32 itemID);
    bool isSubSystem(uint32 itemID);

    uint32 GetFittedTurretCount() { return m_TotalTurretsFitted; }
    uint32 GetFittedLauncherCount() { return m_TotalLaunchersFitted; }
    uint32 GetFittedModuleCountByGroup(uint32 groupID);

	void GetModuleListOfRefs(std::vector<InventoryItemRef> * pModuleList);
    void SaveModules();

private:

    //internal enums
    enum processType
    {
        typeOnlineAll,
        typeOfflineAll,
        typeDeactivateAll,
        typeUnloadAll,
        typeProcessAll
    };

    enum slotType
    {
        highSlot,
        mediumSlot,
        lowSlot,
        rigSlot,
        subSystemSlot
    };

    void _removeModule(EVEItemFlags flag, GenericModule * mod);

    void _process(processType p);
    void _processEx(processType p, slotType t);

    GenericModule * _getHighSlotModule(uint32 flag);
    GenericModule * _getMediumSlotModule(uint32 flag);
    GenericModule * _getLowSlotModule(uint32 flag);
    GenericModule * _getRigModule(uint32 flag);
    GenericModule * _getSubSystemModule(uint32 flag);

    void _removeHighSlotModule(uint32 flag);
    void _removeMediumSlotModule(uint32 flag);
    void _removeLowSlotModule(uint32 flag);
    void _removeRigSlotModule(uint32 flag);
    void _removeSubSystemModule(uint32 flag);

    EVEItemSlotType _checkBounds(uint32 flag);

    bool _isLowSlot(uint32 flag);
    bool _isMediumSlot(uint32 flag);
    bool _isHighSlot(uint32 flag);
    bool _isRigSlot(uint32 flag);
    bool _isSubSystemSlot(uint32 flag);

    void _initializeModuleContainers();

    //we own these
    GenericModule ** m_HighSlotModules;
    GenericModule ** m_MediumSlotModules;
    GenericModule ** m_LowSlotModules;
    GenericModule ** m_RigModules;
    GenericModule ** m_SubSystemModules;

    uint32 m_LowSlots;
    uint32 m_MediumSlots;
    uint32 m_HighSlots;
    uint32 m_RigSlots;
    uint32 m_SubSystemSlots;
    uint32 m_TurretSlots;
    uint32 m_LauncherSlots;

    uint32 m_TotalTurretsFitted;
    uint32 m_TotalLaunchersFitted;
    std::map<uint32, uint32> m_ModulesFittedByGroupID;

    ModuleManager * m_MyManager;        // we do not own this
};

#pragma endregion
/////////////////////////// END MODULECONTAINER //////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// Classes for passing effects around to targets
#pragma region Effect Passing

static const uint8 MAX_EFFECT_COUNT = 5;  //arbitrary, lazy etc.  The bigger this number, the larger these message classes will be

class SubEffect
{
public:

    SubEffect(uint32 attrID, EVECalculationType type, EvilNumber val, uint32 targetItemID = 0)
    : m_AttrID( attrID ), m_TargetItemID( targetItemID ), m_CalcType( type ), m_Val( val )
    {

    }

    ~SubEffect() { }

    //gets
    uint32 AttributeID()                    { return m_AttrID; }
    uint32 TargetItemID()                   { return m_TargetItemID; }
    EVECalculationType CalculationType()    { return m_CalcType; }
    EvilNumber AppliedValue()               { return m_Val; }

private:
    uint32 m_AttrID;
    uint32 m_TargetItemID;
    EVECalculationType m_CalcType;
    EvilNumber m_Val;

};



class Effect
{
public:
    Effect()
    : m_Count( 0 )
    {

    }

    ~Effect()
    {
        for (int i = 0; i <= m_Count; i++)
        {
            delete m_SubEffects[i];
        }
    }

    void AddEffect(uint32 attributeID, EVECalculationType type, EvilNumber val, uint32 targetItemID = 0)
    {
        SubEffect * s = new SubEffect(attributeID, type, val, targetItemID);
        if( m_Count + 1 < MAX_EFFECT_COUNT )
        {
            m_SubEffects[m_Count] = s;
            m_Count++;
        }
    }

    bool hasEffect() { return (m_Count > 0);  }

    SubEffect * next()
    {
        m_Count--;
        return m_SubEffects[m_Count];

    }

private:
    SubEffect * m_SubEffects[MAX_EFFECT_COUNT];
    int m_Count;

};

#pragma endregion
/////////////////////////// END MODULE EFFECTS //////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// Primary Module Manager class
#pragma region ModuleManager

class ModuleManager
{
public:
    ModuleManager(Ship *const ship);
    ~ModuleManager();

    bool IsSlotOccupied(uint32 flag);
	uint32 GetAvailableSlotInBank(EveEffectEnum slotBank);

    bool InstallRig(InventoryItemRef item, EVEItemFlags flag);
    void UninstallRig(uint32 itemID);
    bool InstallSubSystem(InventoryItemRef item, EVEItemFlags flag);
    bool SwapSubSystem(InventoryItemRef item, EVEItemFlags flag);
    bool FitModule(InventoryItemRef item, EVEItemFlags flag);
    void UnfitModule(uint32 itemID);
    void Online(uint32 itemID);
    void OnlineAll();
    void Offline(uint32 itemID);
    void OfflineAll();
    int32 Activate(uint32 itemID, std::string effectName, uint32 targetID, uint32 repeat);
    void Deactivate(uint32 itemID, std::string effectName);
    void DeactivateAllModules();
    void Overload(uint32 itemID);
    void DeOverload(uint32 itemID);
    void DamageModule(uint32 itemID, EvilNumber val);
    void RepairModule(uint32 itemID);
    void ReplaceCharges();
    void UnloadAllModules();
    void CharacterLeavingShip();
    void CharacterBoardingShip();
    void ShipWarping();
    void ShipJumping();
    void Process();
    void ProcessExternalEffect(Effect * e);
    std::vector<GenericModule *> GetStackedItems(uint32 typeID, ModulePowerLevel level);  //should only be used by components

    GenericModule * GetModule(EVEItemFlags flag)    { return m_Modules->GetModule(flag); }
    GenericModule * GetModule(uint32 itemID)        { return m_Modules->GetModule(itemID); }

	void GetModuleListOfRefs(std::vector<InventoryItemRef> * pModuleList);
    void SaveModules();

    // External Methods For use by hostile entities directing effects to this entity:
    int32 ApplyRemoteEffect(uint32 attributeID, uint32 originatorID, SystemEntity * systemEntity, ModifierRef modifierRef);
    int32 RemoveRemoteEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);

    int32 ApplySubsystemEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);
    int32 RemoveSubsystemEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);

    int32 ApplyShipSkillEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);
    int32 RemoveShipSkillEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);

    int32 ApplyModuleRigEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);
    int32 RemoveModuleRigEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);

    int32 ApplyImplantEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);
    int32 RemoveImplantEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);

	Basic_Log * GetLogger() { return m_pLog; }

private:
    bool _fitModule(InventoryItemRef item, EVEItemFlags flag);

    void _processExternalEffect(SubEffect * e);

    ModuleCommand _translateEffectName(std::string s);

    void _SendInfoMessage(const char* fmt, ...);
    void _SendErrorMessage(const char* fmt, ...);

    //access to the ship that owns us.  We do not own this
    Ship * m_Ship;

    //access to destiny.  We do not own this
    DestinyManager * m_Destiny;

    //modules storage, we own this
    ModuleContainer * m_Modules;                    // Holds Module class objects in container arrays, one for each slot bank, rig, subsystem

    //modifier maps, we own these
    ModifierMaps * m_LocalSubsystemModifierMaps;    // Holds std::map<> maps of Modifiers for attributes applied by SUBSYSTEMS
    ModifierMaps * m_LocalShipSkillModifierMaps;    // Holds std::map<> maps of Modifiers for attributes applied by SHIPS and SKILLS
    ModifierMaps * m_LocalModuleRigModifierMaps;    // Holds std::map<> maps of Modifiers for attributes applied by MODULES and RIGS
    ModifierMaps * m_LocalImplantModifierMaps;      // Holds std::map<> maps of Modifiers for attributes applied by IMPLANTS
    ModifierMaps * m_RemoteModifierMaps;            // Holds std::map<> maps of Modifiers for attributes applied by EXTERNAL ENTITY MODULES

	Basic_Log * m_pLog;
};

#pragma endregion
/////////////////////////// END MODULE MANAGER //////////////////////////////////

#endif  /* MODULE_MANAGER_H */


