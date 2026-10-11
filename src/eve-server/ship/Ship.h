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
    Updates:    Allan
*/

#ifndef __SHIP__H__INCL__
#define __SHIP__H__INCL__

#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "effects/EffectsData.h"
#include "fleet/FleetData.h"
#include "inventory/ItemType.h"
#include "inventory/InventoryItem.h"
#include "ship/ShipDB.h"
#include "ship/modules/ModuleManager.h"
#include "system/SystemEntity.h"

/**
 * InventoryItem which represents ShipItem.
 */
class Client;
class GenericModule;
class DroneSE;

class ShipItem
: public InventoryItem
{
    friend class InventoryItem;    // to let it construct us

protected:
    ShipItem(uint32 shipID, const ItemType &type, const ItemData &data);
    ~ShipItem() noexcept;

public:
    /* class type pointer querys. */
    virtual ShipItem* GetShipItem()                     { return this; }
    /* class type tests. */
    virtual bool IsShipItem()                           { return true; }

    void Init();
    void InitPod();
    void InitAttribs();
    void LogOut();
    // adds cargo mass to ship's mass
    void UpdateMass();  // will reset ship mass to default, then loop thru cargo
    static ShipItemRef Load( uint32 shipID);
    static ShipItemRef Spawn( ItemData &data);

    virtual void SetPlayer(Client* pClient);
    virtual bool HasPilot()                             { return (m_pilot != nullptr); }
    virtual Client* GetPilot()                          { return m_pilot; }

    // add item to our inventory and set singleton where applicable
    virtual void AddItem(InventoryItemRef iRef);
    // remove item from our inventory and call Remove() on module/charge
    virtual void RemoveItem(InventoryItemRef iRef);

    bool HasModuleManager()                             { return (m_ModuleManager != nullptr); }
    ModuleManager* GetModuleManager()                   { return m_ModuleManager; }

    virtual void Delete();

    float GetRemainingVolumeByFlag(EVEItemFlags flag) const;
    // this checks destination flag vs item type/group/category for proper placement
    void VerifyHoldType(EVEItemFlags flag, InventoryItemRef iRef, Client* pClient=nullptr);     // this will throw on error.
    bool ValidateItemSpecifics(InventoryItemRef iRef);

    bool IsPopped()                                     { return m_isPopped; }
    void SetPopped(bool set=false)                      { m_isPopped = set; }
    std::string GetShipDNA();

    /*
     * Primary public packet builders:
     */
    PyDict* GetShipInfo();
    PyDict* GetShipState();
    PyList* ShipGetModuleList();
    PyDict* GetChargeState();

    bool ValidateBoardShip(CharacterRef who);
    void SaveShip();
    void RepairShip(float fraction);

    /* Inform Ship that a state change is taking place  */
    void Dock();
    void Heal();
    void Jump();
    void Warp();
    void Eject();
    void Undock();      // reset effects, online mods in list, recharge shield and cap (if session change complete)
    void AddModuleToOnlineVec(uint32 modID);

    /* begin new module manager interface */
    void ProcessModules();
    //this can throw
    void Online(uint32 modID);
    //this can throw
    void Offline(uint32 modID);
    void OnlineAll();
    void OfflineAll();
    void Activate(int32 itemID, std::string effectName, int32 targetID, int32 repeat);
    void Deactivate(int32 itemID, std::string effectName) { m_ModuleManager->Deactivate(itemID, effectName); }
    bool Overload(uint32 itemID, bool set=true)         { return m_ModuleManager->Overload(itemID, set); }
    void CancelOverloading(uint32 itemID, bool stop=false) { m_ModuleManager->CancelOverloading(itemID, stop); }
    void OverloadRack(uint32 itemID, bool set, std::vector<uint32>& changed) { m_ModuleManager->OverloadRack(itemID, set, changed); }
    void ReplaceCharges(EVEItemFlags flag, InventoryItemRef newCharge);
    void RemoveRig(InventoryItemRef iRef);
    void UpdateModules();       // called when undocking to online modules in OnlineModuleList sent from client
    void UpdateModules(EVEItemFlags flag);
    void UnloadModule(uint32 itemID)                    { m_ModuleManager->UnloadModule(itemID); }
    void UnloadModule(GenericModule* pMod)              { m_ModuleManager->UnloadModule(pMod); }
    void UnloadAllModules()                             { m_ModuleManager->UnloadAllModules();  }
    // this will test for active modules and throw where applicable
    void MoveModuleSlot(EVEItemFlags slot1, EVEItemFlags slot2);
    void StripFitting();
    // this will remove all items from all ship cargo holds
    void EmptyCargo();

    // loop thru modules that 'get' things and deactivate them
    void CargoFull();

    void RepairModules(std::vector<InventoryItemRef>& itemRefVec, float fraction);

    void RepairModules()                                { m_ModuleManager->RepairModules(); }

    void AbortCycle()                                   { m_ModuleManager->AbortCycle(); }
    bool IsActive()                                     { return m_isActive; }
    bool IsDocking()                                    { return m_isDocking; }
    bool IsUndocking()                                  { return m_isUndocking; }
    void SetDocked()                                    { m_isDocking = false; }
    void SetUndocking(bool set=false)                   { m_isUndocking = set; }
    InventoryItemRef GetTargetRef()                     { return m_targetRef; }
    void ClearTargetRef()                               { m_targetRef = InventoryItemRef(); }
    // this is for repairing modules with nanite paste
    PyRep* ModuleRepair(uint32 modID)                   { return m_ModuleManager->ModuleRepair(modID); }
    void StopModuleRepair(uint32 modID)                 { m_ModuleManager->StopModuleRepair(modID); }

    /* rack heat.  racks are 0=high, 1=medium, 2=low (-1 for any other slot).
     *  a rack heats up while it has overloaded modules cycling, and cools down otherwise.  heat isn't ticked:
     *  it is worked out from the last change with the same formulas the client uses to animate the HUD gauges.
     */
    static int8 GetHeatRack(EVEItemFlags flag);
    // a module started (or stopped) an overloaded cycle.  updates its rack's heating rate and tells the client
    void SetModuleHeating(GenericModule* pMod, bool heating);
    // current heat of a rack, 0 to its heat capacity
    double GetRackHeat(int8 rack);
    // GM: sets a rack's heat (0 to its heat capacity).  it keeps heating or cooling from there
    void SetRackHeat(int8 rack, double heat);
    // brings the heat attributes up to date, for ship info sent to the client
    void UpdateHeatAttributes();
    // docking: stops all rack heating
    void StopHeating();
    // after one of pMod's overloaded cycles, pMod and its rack may take heat damage
    void HeatDamageCheck(GenericModule* pMod);
    void DamageModule(uint32 modID, float amt=1)        { m_ModuleManager->DamageModule(modID, amt); }
    void DamageRandModule()                             { m_ModuleManager->DamageRandModule(); }

    // get list of fitted modules
    void GetModuleRefVec(std::vector<InventoryItemRef>& iRefVec); // for active ships
    // get list of fitted modules
    void GetModuleItemVec(std::vector<InventoryItemRef>& iRefVec); // for inactive ships
    GenericModule* GetModule(EVEItemFlags flag)         { return m_ModuleManager->GetModule(flag); }
    InventoryItemRef GetModuleRef(EVEItemFlags flag);
    InventoryItemRef GetModuleRef(uint32 modID);

    // this also checks for fit-by-type and rig restrictions
    void TryModuleLimitChecks(EVEItemFlags flag, InventoryItemRef iRef);     // this must throw on error
    EVEItemFlags FindAvailableModuleSlot( InventoryItemRef iRef );
    // make sure this does NOT throw.
    // must return integer
    // will remove item from previous container
    uint32 AddItemByFlag( EVEItemFlags flag, InventoryItemRef iRef, Client* pClient=nullptr);
    // this can throw. returns nothing
    void LoadCharge(InventoryItemRef cRef, EVEItemFlags flag);
    // this can throw. returns nothing
    void LoadChargesToBank(EVEItemFlags flag, std::vector<int32>& chargeIDs);
    // called by client for subLocation (charge) removal
    void RemoveCharge(EVEItemFlags fromFlag);
    /* end new module manager interface */

    // Tactical Interface:
    void SetShipShield(float fraction);
    void SetShipArmor(float fraction);
    void SetShipHull(float fraction);
    void SetShipCapacitorLevel(float fraction);
    float GetShipHullHP()                               { return GetAttribute(AttrHP).get_float(); }
    float GetShipArmorHP()                              { return GetAttribute(AttrArmorHP).get_float(); }
    float GetShipPGLevel()                              { return GetAttribute(AttrPowerOutput).get_float(); }
    float GetShipCPULevel()                             { return GetAttribute(AttrCpuOutput).get_float(); }
    float GetShipShieldHP()                             { return GetAttribute(AttrShieldCharge).get_float(); }
    float GetShipCapacitorLevel()                       { return GetAttribute(AttrCapacitorCharge).get_float(); }
    EvilNumber GetShipHullPercent()                     { return 1 -(GetAttribute(AttrDamage) / GetAttribute(AttrHP)); }
    EvilNumber GetShipCPUPercent()                      { return 1 -(GetAttribute(AttrCpuLoad) / GetAttribute(AttrCpuOutput)); }
    EvilNumber GetShipPGPercent()                       { return 1 -(GetAttribute(AttrPowerLoad) / GetAttribute(AttrPowerOutput)); }
    EvilNumber GetShipArmorPercent()                    { return 1 -(GetAttribute(AttrArmorDamage) / GetAttribute(AttrArmorHP)); }
    EvilNumber GetShipShieldPercent()                   { return (GetAttribute(AttrShieldCharge) / GetAttribute(AttrShieldCapacity)); }
    EvilNumber GetShipCapacitorPercent()                { return (GetAttribute(AttrCapacitorCharge) / GetAttribute(AttrCapacitorCapacity)); }

    // template loading system
    using InventoryItem::_Load;
    virtual bool _Load();
protected:
    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 shipID, const ItemType &type, const ItemData &data) {
        if (type.categoryID() == EVEDB::invCategories::Ship)
            return ShipItemRef( new ShipItem(shipID, type, data ));

        _log(ITEM__ERROR, "Trying to load %s as Ship.", sDataMgr.GetCategoryName(type.categoryID()));
        if (sConfig.debug.StackTrace)
            EvE::traceStack();
        return RefPtr<_Ty>(nullptr);
    }

    //bool LoadAttributes();
    bool m_loaded;

    static uint32 CreateItemID( ItemData &data);

public:
    /* new effects system */
    // only called by ResetEffects
    void ClearModuleModifiers();        // this calls OfflineAll() then ClearModifiers() on all modules and their charges
    // only called by Undock
    void ResetEffects();                // this clears all modifiers, resets all attributes, then calls ProcessEffects()
    // called by AggressionManager::GetCriminalTimeStamps
    void PrepForUndock();               // called by AggresMgr to try saving a bit of runtime by clearing beforehand
    void ProcessEffects(bool add=false, bool update=false);
    void CharacterBoardingShip()                        { m_ModuleManager->CharacterBoardingShip(); }

    /* linking weapons methods */
    uint8 GetLinkedCount(GenericModule* pMod);
    uint8 GetLoadedLinkedCount(GenericModule* pMod);
    bool HasLinkedWeapons()                             { return (!m_linkedWeapons.empty()); }
    void LinkAllWeapons();
    void LinkWeapon(uint32 masterID, uint32 slaveID);           // this should not throw
    void PeelAndLink(uint32 masterID, uint32 slaveID);          // this should not throw
    void MergeModuleGroups(uint32 masterID, uint32 slaveID);    // this should not throw
    uint32 UnlinkWeapon(uint32 moduleID);
    void UnlinkGroup(uint32 memberID, bool update=false);
    void UnlinkAllWeapons();
    PyRep* GetLinkedWeapons();
    void OfflineGroup(GenericModule* pMod);
    void DamageGroup(GenericModule* pMod);
    // to load with ammo
    void LoadLinkedWeapons(GenericModule* pMod, std::vector<int32>& chargeIDs);
    void GetLinkedWeaponMods(EVEItemFlags flag, std::vector<GenericModule*> &modules);

protected:
    /* linking weapons methods */
    void LinkWeapon(GenericModule* pMaster, GenericModule* pSlave);
    void UnlinkWeapon(uint32 masterID, uint32 slaveID);
    // to load saved linked weapons
    void LoadWeaponGroups();
    // to save linked weapons
    void SaveWeaponGroups();
    // this single iteration loop will link same type weapons, then remove the weapon from the list.
    //  non-linked weapons are kept in list to be checked upon return to caller.
    void LinkWeaponLoop(std::list< GenericModule* >& moduleVec);

private:
    Client* m_pilot;

    //the ship's module manager.  We own this
    ModuleManager* m_ModuleManager;

    InventoryItemRef m_targetRef;       // this is only used for module effects that require a target.  is here because of the ease of aquiring/sending (common code)

    std::vector<uint32> m_onlineModuleVec;      // for onlining modules when undocking
    std::map<GenericModule*, std::list<GenericModule*>> m_linkedWeapons; // masterID/data (slaveIDs)

    struct RackHeat {
        bool init = false;
        double heat = 0.0;          // heat when last changed
        int64 time = 0;             // when it last changed (filetime)
        double rate = 0.0;          // heating rate (sum of heatAbsorbtionRateModifier of the heating modules)
        std::set<uint32> modules;   // modules cycling overloaded
    };
    RackHeat m_rackHeat[3];
    RackHeat& GetRackState(int8 rack);
    double RackHeatAt(int8 rack, int64 time);
    void SyncRackHeat(int8 rack);
    void SendHeatChange(uint16 attrID, double value, int64 time);
    void BurnOut(GenericModule* pMod);

    void ProcessShipEffects(bool update=false);

    bool m_isActive :1;
    bool m_isPopped :1;
    bool m_isDocking :1;
    bool m_isUndocking :1;
};

/**
 * DynamicSystemEntity which represents ship object in space
 */
class EVEServiceManager;
class InventoryItem;
class DestinyManager;
class Missile;
class SystemManager;
class ServiceDB;

class ShipSE
: public DynamicSystemEntity
{
public:
    ShipSE(InventoryItemRef self, EVEServiceManager& services, SystemManager* pSystem, const FactionData& data);
    virtual ~ShipSE()                                   { /* do nothing here */ }

    /* class type pointer querys. */
    virtual ShipSE* GetShipSE()                         { return this; }
    /* class type tests. */
    virtual bool IsShipSE()                             { return true; }

    /* SystemEntity interface */
    virtual void Process();
    virtual void EncodeDestiny( Buffer& into );
    virtual void MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict *MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual void Killed(Damage &fatal_blow);            /* This method is defined in Damage.cpp */

    /* virtual functions to be overridden in derived classes */
    virtual void MissileLaunched(Missile* pMissile)     { /* Do nothing here */ }
    virtual bool IsInvul();
    virtual bool IsFrozen()                             { return false; }
    virtual bool IsLogin();

    /* virtual functions in base to allow common interface calls specific to ship entities */
    virtual void SetPilot(Client* pClient);
    virtual bool HasPilot()                             { return ((m_shipRef.get() == nullptr) ? false : m_shipRef->HasPilot()); }
    virtual Client* GetPilot()                          { return ((m_shipRef.get() == nullptr) ? nullptr : m_shipRef->GetPilot()); }

    /* specific functions handled here. */
    void Dock();
    void Jump(bool showCloak=true);        // this sets ship cloak
    void Warp();
    void RemoveTarget(SystemEntity* pSE);
    void SetPassword(std::string pass)                  { m_towerPass = pass; }
    // fleet
    void ClearBoostData();
    bool IsBoosted()                                    { return m_boosted; }
    void SetBoost(bool set=false)                       { m_boosted = set; }
    void RemoveBoost();
    void ApplyBoost(BoostData& bData);
    uint8 GetMiningBoostAmount()                        { return m_boost.mining; }

    // misc
    void PayInsurance();
    void DamageModule(uint32 modID)                     { m_shipRef->DamageModule(modID); }
    void DamageRandModule(float chance);
    void ResetShipSystemMgr(SystemManager* pSystem);    // this is to reset system manager for jumps, etc.
    void SaveShip()                                     { m_shipRef->SaveShip(); }

    //cancel all active modules
    void AbortCycle()                                   { m_shipRef->AbortCycle(); }

    // this is the itemID of the ship that ejected the pod, for setting in slimItemUpdate (launcherID)
    void SetLauncherID(uint32 shipID)                   { m_podShipID = shipID; }
    uint32 GetLauncherID()                              { return m_podShipID; }

    ShipItemRef GetShipItemRef()                        { return m_shipRef; }

    float CalculateRechargeRate(float Capacity, float RechargeTimeMS, float Current);

    DroneSE* LaunchDrone(InventoryItemRef dRef);

    bool GetFleetSMBUsage()                             { return m_allowFleetSMBUsage; }
    void SetFleetSMBUsage(bool set=false)               { m_allowFleetSMBUsage = set; }

protected:
    ShipItemRef m_shipRef;

    const uint32 m_processTimerTick;

private:
    ShipDB m_db;

    Timer m_processTimer;

    uint32 m_podShipID;

    bool m_allowFleetSMBUsage;

    /*  boost data */
    BoostData m_boost = BoostData();
    bool m_boosted;
    uint16 m_oldArmor;
    uint16 m_oldShield;
    uint16 m_oldScanRes;
    uint32 m_oldTargetRange;
    float m_oldInertia;

    /*  for POS field */
    std::string m_towerPass;

};

#endif /* !__SHIP__H__INCL__ */
