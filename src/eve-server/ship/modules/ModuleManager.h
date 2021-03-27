/*
 *  ModuleManager.cpp
 *
 *      this class manages all aspects of modules and charges loaded in ships
 *
 * Author: Allan
 * Started: 30Mar16
 *
 *      loosely based on original evemu code by Aknor Jaden and Luck
 */


#ifndef EVE_SHIP_MODULES_MODULEMANAGER_H
#define EVE_SHIP_MODULES_MODULEMANAGER_H


#include "PyService.h"

class GenericModule;
class SystemEntity;

class ModuleManager
{
public:
    ModuleManager(ShipItem* const pShip);
    ~ModuleManager();

    // i dont think this can throw
    bool Initialize();
    void LoadOnline();
    bool IsSlotOccupied(EVEItemFlags flag);
    uint16 GetAvailableSlotInBank(EVEEffectID slotBank);

    void CargoFull();
    void RemoveTarget(SystemEntity* pSE);

     // verify slot available
    // will throw if no slots available
    void CheckSlotFitLimited(EVEItemFlags flag);
    // verify module isnt group limited
    // will throw if group is over fitLimited.
    void CheckGroupFitLimited(EVEItemFlags flag, InventoryItemRef iRef);

    // returns vector of fitted GenericModule* in specified flag's bank
    void GetModulesInBank(EVEItemFlags flag, std::vector<GenericModule*>& modVec);

    bool InstallRig(ModuleItemRef mRef, EVEItemFlags flag);
    void UninstallRig(uint32 itemID);
    bool InstallSubSystem(ModuleItemRef mRef, EVEItemFlags flag);
    bool AddModule(ModuleItemRef mRef, EVEItemFlags flag);
    // this will deactivate, offline and remove charges from module
    void UnfitModule(uint32 itemID);
    // this will deactivate, offline and remove charges from module
    void UnfitModule(EVEItemFlags flag);
    void Online(uint32 itemID);
    void Offline(uint32 itemID);
    void Online(EVEItemFlags flag);
    void Offline(EVEItemFlags flag);
    void OnlineAll();
    void OfflineAll();
    void Activate(int32 itemID, uint16 effectID, int32 targetID, int32 repeat);
    void Deactivate(uint32 itemID, std::string effectName);
    void DeactivateAllModules();
    void Overload(uint32 itemID);
    void DeOverload(uint32 itemID);
    void DamageModule(uint32 itemID, float amount);
    void DamageModule(GenericModule* pMod, float amount);
    void DamageRandModule();
    void DamageRandModule(float amount);
    void RepairModule(uint32 itemID, EvilNumber amount);
    void RepairModule(GenericModule* pMod, EvilNumber amount);
    void RepairModules();
    // this is for repairing modules with nanite paste
    PyRep* ModuleRepair(uint32 modID);
    void StopModuleRepair(uint32 modID);
    // this will move charge item to module and split stack if needed
    // must NOT throw
    void LoadCharge(InventoryItemRef chargeRef, EVEItemFlags flag);
    // this will remove charge item from module and update client
    // must NOT throw
    void UnloadCharge(GenericModule* pMod);
    // unload charge from module by itemID
    void UnloadModule(uint32 itemID);
    // unload charge from module by flag
    void UnloadModule(EVEItemFlags flag);
    // unload charge from module by module*
    void UnloadModule(GenericModule* pMod);
    // this will remove charges from all modules
    void UnloadAllModules();
    // this will remove charges from weapons only
    void UnloadWeapons();
    void UpdateModules(std::vector<uint32> modVec);
    void UpdateModules(EVEItemFlags flag);
    bool VerifySlotExchange(EVEItemFlags slot1, EVEItemFlags slot2);
    void CharacterLeavingShip();
    void CharacterBoardingShip();
    void ShipWarping();
    void ShipJumping();
    void Process();
    void AbortCycle();

    InventoryItemRef GetLoadedChargeOnModule(EVEItemFlags flag);
    InventoryItemRef GetLoadedChargeOnModule(InventoryItemRef moduleRef);

    void GetLoadedCharges(std::map<EVEItemFlags, InventoryItemRef> &charges)
                                                        { charges = m_charges; }

    void GetWeapons(std::list<GenericModule*>& weaponList);

    void GetShipRigs(std::vector< uint32 >& modVec);
    void GetShipSubSystems(std::vector< uint32 >& modVec);
    void SortModulesBySlotDec(std::vector< uint32 >& modVec, std::vector< GenericModule* >& pModList);
    // low, mid, hi, rig, subsys
    void GetModuleListOfRefsAsc(std::vector<InventoryItemRef>& modVec);
    // subsys, rig, hi, mid, low
    void GetModuleListOfRefsDec(std::vector<InventoryItemRef>& modVec);
    // subsys, rig, low, mid, hi
    void GetModuleListOfRefsOrdered(std::vector<InventoryItemRef>& modVec);
    // hi, mid, low, rig, subsys
    void GetModuleListOfRefsOrderedRev(std::vector<InventoryItemRef>& modVec);
    void GetModuleListByReqSkill(uint16 skillID, std::vector<InventoryItemRef>& modVec);
    void SaveModules();

    void GetActiveModules(uint8 rack, std::vector< GenericModule* >& modVec);
    void GetActiveModulesHeat(uint8 rack, float &heat);
    // returns # of active non-ol'd modules for this rack
    uint8 GetActiveModulesCount(uint8 rack);
    uint8 GetFittedModuleCountByGroup(uint16 groupID);

    // scan method to check for scanning rigs.
    // this is set as % with 1.0 being 100%
    float GetRigScanBonus()                             { return m_rigScanBonus; }

    GenericModule* GetModule(EVEItemFlags flag);        // faster than GetModule(itemID)
    GenericModule* GetModule(uint32 itemID);            // slower than GetModule(flag)
    GenericModule* GetRandModule();


    // huge fucking hack to set charge qty in fit window when in space
    void UpdateChargeQty();

private:
    // adds module ref to map, modifies rof if applicable, sets maps for fit-by-group, adjusts slot count
    void addModuleRef(EVEItemFlags flag, GenericModule* pMod);
    // removes module ref from maps and adjusts slot count
    void deleteModuleRef(EVEItemFlags flag, GenericModule* pMod);

    bool m_initalized;

    ShipItem* pShipItem;

    uint8 m_LowSlots;
    uint8 m_MidSlots;
    uint8 m_HighSlots;
    uint8 m_RigSlots;
    uint8 m_SubSystemSlots;

    // dont like this, but best way to do it...need to set attrib for this, either on char or ship
    float m_rigScanBonus;       // fuzzy logic as %

    std::map<uint16, uint8> m_modByGroup;               // groupID, count
    std::map<uint8, GenericModule*> m_modules;          // slot, module (for all slots)
    std::map<uint8, GenericModule*> m_systems;          // slot, module (for rigs and subsystems)
    std::map<uint8, GenericModule*> m_fittings;         // slot, module (for hi,mid,lo slots)
    std::map<EVEItemFlags, InventoryItemRef> m_charges; // slot, chargeItem
};


#endif  // EVE_SHIP_MODULES_MODULEMANAGER_H

/* {'messageKey': 'ModuleActivatedDeniedForceField', 'dataID': 17881053, 'suppressable': False, 'bodyID': 258630, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1797}
 * {'messageKey': 'ModuleActivationDeniedCriminalAssistance', 'dataID': 17875215, 'suppressable': False, 'bodyID': 256427, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3548}
 * {'messageKey': 'ModuleAlreadyActive', 'dataID': 17882992, 'suppressable': False, 'bodyID': 259340, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259339, 'messageID': 1224}
 * {'messageKey': 'ModuleAlreadyBanked', 'dataID': 17878036, 'suppressable': False, 'bodyID': 257477, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2671}
 * {'messageKey': 'ModuleAlreadyFitting', 'dataID': 17882995, 'suppressable': False, 'bodyID': 259341, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1225}
 * {'messageKey': 'ModuleEffectActive', 'dataID': 17883214, 'suppressable': False, 'bodyID': 259424, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1226}
 * {'messageKey': 'ModuleFitFailed', 'dataID': 17883222, 'suppressable': False, 'bodyID': 259427, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1228}
 *  u'ModuleFitFailedBody'}(u'The {moduleName} cannot be fitted. {reason}', None, {u'{reason}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'reason'}, u'{moduleName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'moduleName'}})
 * {'messageKey': 'ModuleGotDamagedWhileBeingRepaired', 'dataID': 17879486, 'suppressable': False, 'bodyID': 258026, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2389}
 * {'messageKey': 'ModuleIsBlocked', 'dataID': 17878649, 'suppressable': False, 'bodyID': 257706, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2573}
 *  u'ModuleIsBlockedBody'}(u'External factors are preventing your {moduleName} from responding to this command', None, {u'{moduleName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'moduleName'}})
 * {'messageKey': 'ModuleJammedOnBadAmmo', 'dataID': 17883225, 'suppressable': False, 'bodyID': 259428, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1229}
 *  u'ModuleJammedOnBadAmmoBody'}(u'{[item]module.name} jammed on the {[item]ammo.name} within it, which it does not use.', None, {u'{[item]module.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'module'}, u'{[item]ammo.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'ammo'}})
 * {'messageKey': 'ModuleNoLongerPresentForCharges', 'dataID': 17882998, 'suppressable': False, 'bodyID': 259342, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1230}
 * {'messageKey': 'ModuleNotPowered', 'dataID': 17883231, 'suppressable': False, 'bodyID': 259430, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1232}
 * {'messageKey': 'ModuleReactivationDelayed2', 'dataID': 17879578, 'suppressable': False, 'bodyID': 258059, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2307}
 * {'messageKey': 'ModuleRequiresFuel', 'dataID': 17883167, 'suppressable': False, 'bodyID': 259407, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1235}
 * {'messageKey': 'ModuleRequiresLowerSystemSecurity', 'dataID': 17883256, 'suppressable': False, 'bodyID': 259439, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1236}
 * {'messageKey': 'ModuleRequiresTargetOwnerFleetMembership', 'dataID': 17883161, 'suppressable': False, 'bodyID': 259405, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1237}
 * {'messageKey': 'ModuleTooDamagedToRepairGoToStation', 'dataID': 17879349, 'suppressable': False, 'bodyID': 257974, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2387}
 * {'messageKey': 'ModuleUnfit', 'dataID': 17883328, 'suppressable': False, 'bodyID': 259464, 'messageType': 'notify', 'urlAudio': 'wise:/msg_ModuleUnfit_play', 'urlIcon': '', 'titleID': None, 'messageID': 1239}
 * {'messageKey': 'ModulesIncorrectlyFitted', 'dataID': 17878135, 'suppressable': False, 'bodyID': 257513, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2512}
 *  u'ModulesIncorrectlyFittedBody'}(u'Your modules are incorrectly fitted. Possibly your slot layout has changed and a module is in a slot that is no longer valid. Try unfitting your modules and fit them again.', None, None)
 * {'messageKey': 'ModulesNotLoadableInSpace', 'dataID': 17883271, 'suppressable': False, 'bodyID': 259444, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1240}
 *  u'ModulesNotLoadableInSpaceBody'}(u'You can only fit or unfit from a ship while in station. An exception to this rule is when employing a device like the {device}.', None, {u'{device}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'device'}})
 */
