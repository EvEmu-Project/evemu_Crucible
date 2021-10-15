
 /**
  * @name GenericModule.h
  *   base module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */


#ifndef _EVE_SHIP_MODULES_GENERIC_MODULE_H
#define _EVE_SHIP_MODULES_GENERIC_MODULE_H

#include "EVEServerConfig.h"
#include "effects/EffectsProcessor.h"
#include "inventory/InventoryItem.h"
#include "ship/Ship.h"
#include "ship/modules/ModuleDefs.h"
#include "ship/modules/ModuleItem.h"
#include "system/SystemEntity.h"

class ActiveModule;
class PassiveModule;
class MiningLaser;
class Prospector;
class TurretModule;
class SuperWeapon;
class RigModule;
class CynoModule;
class SubSystemModule;

/* generic module base class */
class GenericModule
{
public:
    GenericModule(ModuleItemRef mRef, ShipItemRef sRef);
    virtual ~GenericModule();

    /* generic functions handled in base class */
    void                Online();    // this function must NOT throw
    void                Offline();   // this function must NOT throw

    ModuleItemRef       GetSelf()                       { return m_modRef; }
    ShipItemRef         GetShipRef()                    { return m_shipRef; }

    void                ProcessEffects(int8 state, bool active = false);

    void                Repair()                        { m_modRef->ResetAttribute(AttrDamage, true); }
    void                Repair(EvilNumber amount);

    bool HasAttribute(uint32 attrID)                    { return m_modRef->HasAttribute(attrID); }
    void SetAttribute(uint32 attrID, EvilNumber val, bool update=true) { m_modRef->SetAttribute(attrID, val, update); }
    void ResetAttribute(uint32 attrID)                  { m_modRef->ResetAttribute(attrID); }
    EvilNumber GetAttribute(uint32 attrID)              { return m_modRef->GetAttribute(attrID); }

    bool                isWarpSafe()                    { return m_isWarpSafe; }
    bool                isTurretFitted()                { return m_modRef->type().HasEffect(EVEEffectID::turretFitted); }
    bool                isLauncherFitted()              { return m_modRef->type().HasEffect(EVEEffectID::launcherFitted); }

    /* class type pointer querys, public for anyone to access. */
    virtual ActiveModule*       GetActiveModule()       { return nullptr; }
    virtual PassiveModule*      GetPassiveModule()      { return nullptr; }
    virtual MiningLaser*        GetMiningModule()       { return nullptr; }
    virtual Prospector*         GetProspectModule()     { return nullptr; }
    virtual TurretModule*       GetTurretModule()       { return nullptr; }
    virtual SuperWeapon*        GetSuperWeapon()        { return nullptr; }
    virtual RigModule*          GetRigModule()          { return nullptr; }
    virtual SubSystemModule*    GetSubSystemModule()    { return nullptr; }
    virtual CynoModule*         GetCynoModule()         { return nullptr; }
    /* class type helpers.  public for anyone to access. */
    virtual bool        IsGenericModule() const         { return true; }
    virtual bool        IsPassiveModule() const         { return false; }
    virtual bool        IsActiveModule() const          { return false; }
    virtual bool        IsMiningLaser() const           { return false; }
    virtual bool        IsProspectModule() const        { return false; }
    virtual bool        IsCynoModule() const            { return false; }
    virtual bool        IsRigModule() const             { return false; }   // check this in m_rigSlot?
    virtual bool        IsSubSystemModule() const       { return false; }   // check this in m_subSystem?

    bool                IsLoaded()                      { return (m_chargeLoaded and (m_chargeRef.get() != nullptr)); }
    bool                IsTurretModule()                { return m_turret; }
    bool                IsLauncherModule()              { return m_launcher; }
    bool                IsOverloaded()                  { return m_overLoaded; }
    bool                IsLinked()                      { return m_linked; }
    bool                IsMaster()                      { return m_linkMaster; }
    bool                IsDamaged()                     { return m_modRef->GetAttribute(AttrDamage) != EvilZero; }
    bool                IsActive()                      { return (m_ModuleState == Module::State::Activated ? true : m_ModuleState == Module::State::Deactivating ? true : false); }
    bool                IsLoading()                     { return m_ModuleState == Module::State::Loading; }

    /* generic access functions handled here, but set elsewhere. */
    bool                isOnline()                      { return m_modRef->IsOnline(); }
    bool                isLowPower()                    { return m_loPower; }
    bool                isHighPower()                   { return m_hiPower; }
    bool                isMediumPower()                 { return m_medPower; }
    bool                isRig()                         { return m_rigSlot; }
    bool                isSubSystem()                   { return m_subSystem; }

    uint32              itemID()                        { return m_modRef->itemID(); }
    uint32              typeID()                        { return m_modRef->typeID(); }
    uint32              groupID()                       { return m_modRef->groupID(); }
    EVEItemFlags        flag()                          { return m_modRef->flag(); }

    void SetChargeRef(InventoryItemRef iRef)            { m_chargeRef = iRef; }
    void SetModuleState(int8 state)                     { m_ModuleState = state; }
    void SetChargeState(int8 state)                     { m_ChargeState = state; }
    void SetLinked(bool set=false)                      { m_linked = set; }
    void SetLinkMaster(bool set=false)                  { m_linkMaster = set; }

    int8 GetModuleState()                               { return m_ModuleState; }
    int8 GetChargeState()                               { return m_ChargeState; }
    InventoryItemRef GetLoadedChargeRef()               { return m_chargeRef; }

    /* generic access functions to be handled in derived classes (must override) */
    virtual void Process()                              { /* do nothing here */ }
    virtual void Deactivate(std::string effect="")      { /* do nothing here */ }
    virtual void AbortCycle()                           { /* do nothing here */ }
    // this will not move charge item.
    // must NOT throw
    virtual void LoadCharge(InventoryItemRef charge)    { /* do nothing here */ }
    // this will not move charge item.
    // must NOT throw
    virtual void UnloadCharge()                         { /* do nothing here */ }
    // this will physically add charge to module,
    virtual void ReloadCharge()                         { /* do nothing here */ }
    virtual void RemoveRig()                            { /* do nothing here */ }
    virtual void DestroyRig()                           { /* do nothing here */ }
    virtual void ReprocessCharge()                      { /* do nothing here */ }

    virtual void Activate(uint16 effectID, uint32 targetID=0, int16 repeat=0)
                                                        { /* do nothing here */ }
    virtual void RemoveTarget(SystemEntity* pSE)        { /* do nothing here */ }

    // check for Prospector (so far) modules defaulted here for generic access
    virtual bool IsSuccess()                            { return false; }

    /* generic access functions to be overridden in derived classes as needed */
    virtual void Update()      // this is used to set char mods on active modules created before pilot entered ship
                                                        { /* do nothing here */ }
    virtual void Overload();
    virtual void DeOverload();
    virtual uint16 GetReloadTime()                      { return 0; }
    virtual uint32 GetTargetID()                        { return 0; }

    //
    int8 GetModulePowerLevel();


protected:
    const char*         GetModuleStateName(int8 state);

    ModuleItemRef       m_modRef;
    ShipItemRef         m_shipRef;
    InventoryItemRef    m_chargeRef;

    int8                m_ModuleState;
    int8                m_ChargeState;

    int16               m_repeat;

    bool                m_linkMaster   :1;
    bool                m_linked       :1;
    bool                m_isWarpSafe   :1;
    bool                m_hiPower      :1;
    bool                m_medPower     :1;
    bool                m_loPower      :1;
    bool                m_rigSlot      :1;
    bool                m_subSystem    :1;
    bool                m_launcher     :1;
    bool                m_turret       :1;
    bool                m_overLoaded   :1;
    bool                m_chargeLoaded :1;
};

#endif  // _EVE_SHIP_MODULES_GENERIC_MODULE_H
