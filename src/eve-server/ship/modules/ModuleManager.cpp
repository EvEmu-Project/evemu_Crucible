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


#include "eve-server.h"

#include "EVEServerConfig.h"
#include "Client.h"
#include "StaticDataMgr.h"
#include "effects/EffectsDataMgr.h"
#include "ship/Ship.h"
#include "ship/modules/ModuleItem.h"
#include "ship/modules/ModuleManager.h"
#include "ship/modules/ModuleFactory.h"
#include "ship/modules/ActiveModule.h"
#include "station/Station.h"
#include "system/DestinyManager.h"

/*
 * MODULE__ERROR
 * MODULE__WARNING
 * MODULE__MESSAGE
 * MODULE__INFO
 * MODULE__TRACE
 * MODULE__DEBUG
 * MODULE__DAMAG
 */

ModuleManager::ModuleManager(ShipItem *const pShip)
: pShipItem(pShip),
m_initalized(false),
m_rigScanBonus(0),
m_LowSlots(0),
m_MidSlots(0),
m_HighSlots(0),
m_RigSlots(0),
m_SubSystemSlots(0)
{
    assert(pShip != nullptr);
}

ModuleManager::~ModuleManager()
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
    for (; itr != m_modules.end(); ++itr)
        SafeDelete(itr->second);
}

bool ModuleManager::Initialize() {
    if (m_initalized)
        return true;

    _log(MODULE__TRACE, "MM::Initialize() - ship %s", pShipItem->name() );

    m_LowSlots = pShipItem->GetAttribute(AttrLowSlots).get_uint32();
    m_MidSlots = pShipItem->GetAttribute(AttrMedSlots).get_uint32();
    m_HighSlots = pShipItem->GetAttribute(AttrHiSlots).get_uint32();
    m_RigSlots = pShipItem->GetAttribute(AttrRigSlots).get_uint32();
    m_SubSystemSlots = pShipItem->GetAttribute(AttrSubSystemSlot).get_uint32();

    // modules - 3 banks of 8 slots each
    for (uint8 flag = flagLowSlot0; flag < flagFixedSlot; ++flag) {
        m_modules.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
        m_fittings.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
    }
    // rigs - one bank of 3 slots
    for (uint8 flag = flagRigSlot0; flag < flagRigSlot3; ++flag) {
        m_modules.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
        m_systems.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
    }
    //subsystems - one bank of 5 slots
    for (uint8 flag = flagSubSystem0; flag < flagSubSystem5; ++flag) {
        m_modules.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
        m_systems.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
    }

    // Load modules, rigs and subsystems into module maps:
    std::vector<InventoryItemRef> itemVec;
    // this will order by mod, charge, cargo
    pShipItem->GetMyInventory()->GetInventoryVec(itemVec);
    GenericModule* pMod(nullptr);
    for (auto cur : itemVec) {
        // this is a hack.  dont know why any ship item would have flagNone set, but have seen random errors where charges are set to flagNone
        if (cur->flag() == flagNone) {
            _log(MODULE__ERROR, "MM::Initialize() - %s(%u) has flagNone set in ship %s",\
                    cur->name(), cur->itemID(), pShipItem->name() );
            // put that bitch back in cargo
            cur->SetFlag(flagCargoHold);
        }
        if (IsModuleSlot(cur->flag())) {
            switch (cur->categoryID()) {
                case EVEDB::invCategories::Module: {
                    _log(MODULE__TRACE, "MM::Initialize() - loading %s(%u) to %s.",\
                            cur->name(), cur->itemID(), sDataMgr.GetFlagName(cur->flag()));
                    ModuleItemRef mRef = ModuleItemRef::StaticCast(cur);
                    // hack to get total scan bonus from rigs, if applicable
                    // do we need to check for and set anything else here?
                    if (mRef->groupID() == EVEDB::invGroups::Rig_Electronics) {
                        switch (mRef->typeID()) {
                            case 25936:   // Large Gravity Capacitor Upgrade I
                            case 31213:   // Small Gravity Capacitor Upgrade I
                            case 31215:   // Medium Gravity Capacitor Upgrade I
                            case 31217:   // Capital Gravity Capacitor Upgrade I
                            case 26350:   // Large Gravity Capacitor Upgrade II
                            case 31220:   // Small Gravity Capacitor Upgrade II
                            case 31222:   // Medium Gravity Capacitor Upgrade II
                            case 31224: { // Capital Gravity Capacitor Upgrade II
                                m_rigScanBonus += (0.01 * mRef->GetAttribute(AttrScanStrengthBonus).get_float());
                            } break;
                        }
                    }
                    AddModule(mRef, cur->flag());
                } break;
                case EVEDB::invCategories::Subsystem: {
                    _log(MODULE__TRACE, "MM::Initialize() - loading %s(%u) to %s.",\
                            cur->name(), cur->itemID(), sDataMgr.GetFlagName(cur->flag()));
                    ModuleItemRef mRef = ModuleItemRef::StaticCast(cur);
                    AddModule(mRef, cur->flag());
                } break;
                case EVEDB::invCategories::Charge: {
                    pMod = GetModule(cur->flag());
                    if (pMod == nullptr) {
                        // module to load not found...
                        _log(MODULE__ERROR, "MM::Initialize() - No module at %s to load charge %s(%u) into",\
                                sDataMgr.GetFlagName(cur->flag()), cur->name(), cur->itemID() );
                        // put that bitch back in cargo
                        cur->SetFlag(flagCargoHold);
                    } else {
                        if (pMod->IsLoaded()) {
                            _log(MODULE__TRACE, "MM::Initialize() - %s at %s is already loaded.  Moving %s to cargo.",\
                                    pMod->GetSelf()->name(), sDataMgr.GetFlagName(cur->flag()), cur->name());
                            // put that bitch back in cargo
                            cur->SetFlag(flagCargoHold);
                            continue;
                        }
                        _log(MODULE__TRACE, "MM::Initialize() - loading %s(%u) at %s with %s(%u).",\
                                pMod->GetSelf()->name(), pMod->GetSelf()->itemID(), \
                                sDataMgr.GetFlagName(cur->flag()), cur->name(), cur->itemID());
                        pMod->LoadCharge(cur);
                        //cur->SetQuantity(cur->quantity());    //OIC
                        cur->SetAttribute(AttrQuantity, cur->quantity(), false);   // OMAC
                        m_charges.emplace(cur->flag(), cur);
                    }
                    pMod = nullptr;
                } break;
            }
        }
    }

    return m_initalized = true;
}

void ModuleManager::LoadOnline() {
    // must proc modules in order of (subsys -> rig -> low -> mid -> high) for proper fx application
    // online subsystems, then rigs before hi,mid,lo slots
    std::map<uint8, GenericModule*>::reverse_iterator ritr = m_systems.rbegin(), rend = m_systems.rend();
    while (ritr != rend) {
        if (ritr->second != nullptr)
            ritr->second->Online();
            ++ritr;
    }
    // process lo,mid,hi slots in that order.
    std::map<uint8, GenericModule*>::iterator itr = m_fittings.begin(), end = m_fittings.end();
    while (itr != end) {
        if (itr->second != nullptr)
            if (itr->second->GetAttribute(AttrOnline).get_bool())
                itr->second->Online();
            ++itr;
    }
}

void ModuleManager::Process()
{
    double profileStartTime(GetTimeUSeconds());

    // proc modules in order of (low -> mid -> high) for proper fx application
    // NOTE: rigs and subsystems dont need proc tic.
    std::map<uint8, GenericModule*>::iterator itr = m_fittings.begin(), end = m_fittings.end();
    while (itr != end) {
        if (itr->second != nullptr)
            if (itr->second->GetAttribute(AttrOnline).get_bool())
                itr->second->Process();
            ++itr;
    }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::modules, GetTimeUSeconds() - profileStartTime);
}

bool ModuleManager::IsSlotOccupied(EVEItemFlags flag)
{
    return (m_modules.find((uint8)flag)->second != nullptr);
}

void ModuleManager::RemoveTarget(SystemEntity* pSE) {
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            cur.second->RemoveTarget(pSE);
        /*
    for (auto cur : m_fittings)
        if (cur.second != nullptr)
            cur.second->RemoveTarget(pSE);
        */
}

uint16 ModuleManager::GetAvailableSlotInBank(EVEEffectID slotBank)
{
    switch (slotBank) {
        case EVEEffectID::loPower: {
            uint8 max = flagLowSlot0 + pShipItem->GetAttribute(AttrLowSlots).get_uint32();
            for (uint8 slot=flagLowSlot0; slot < max; ++slot)
                if ( m_modules[slot] == nullptr )
                    return slot;
        } break;
        case EVEEffectID::medPower: {
            uint8 max = flagMidSlot0 + pShipItem->GetAttribute(AttrMedSlots).get_uint32();
            for (uint8 slot=flagMidSlot0; slot < max; ++slot)
                if ( m_modules[slot] == nullptr )
                    return slot;
        } break;
        case EVEEffectID::hiPower: {
            uint8 max = flagHiSlot0 + pShipItem->GetAttribute(AttrHiSlots).get_uint32();
            for (uint8 slot=flagHiSlot0; slot < max; ++slot)
                if ( m_modules[slot] == nullptr )
                    return slot;
        } break;
        case EVEEffectID::rigSlot: {
            uint8 max = flagRigSlot0 + pShipItem->GetAttribute(AttrRigSlots).get_uint32();
            for (uint8 slot=flagRigSlot0; slot < max; ++slot)
                if ( m_modules[slot] == nullptr )
                    return slot;
        } break;
        case EVEEffectID::subSystem: {
            uint8 max = flagSubSystem0 + pShipItem->GetAttribute(AttrSubSystemSlot).get_uint32();
            for (uint8 slot=flagSubSystem0; slot < max; ++slot)
                if ( m_modules[slot] == nullptr )
                    return slot;
        } break;
    }
    return flagIllegal;
}

void ModuleManager::GetModulesInBank(EVEItemFlags flag, std::vector<GenericModule*>& modVec)
{
    switch (flag) {
        case flagLowSlot0:
        case flagLowSlot1:
        case flagLowSlot2:
        case flagLowSlot3:
        case flagLowSlot4:
        case flagLowSlot5:
        case flagLowSlot6:
        case flagLowSlot7: {
            for (uint8 slot=flagLowSlot0; slot < (flagLowSlot0 + 8); ++slot)
                if ( m_modules[slot] != nullptr )
                    modVec.push_back(m_modules[slot]);
        } break;
        case flagMidSlot0:
        case flagMidSlot1:
        case flagMidSlot2:
        case flagMidSlot3:
        case flagMidSlot4:
        case flagMidSlot5:
        case flagMidSlot6:
        case flagMidSlot7: {
            for (uint8 slot=flagMidSlot0; slot < (flagMidSlot0 + 8); ++slot)
                if ( m_modules[slot] != nullptr )
                    modVec.push_back(m_modules[slot]);
        } break;
        case flagHiSlot0:
        case flagHiSlot1:
        case flagHiSlot2:
        case flagHiSlot3:
        case flagHiSlot4:
        case flagHiSlot5:
        case flagHiSlot6:
        case flagHiSlot7: {
            for (uint8 slot=flagHiSlot0; slot < (flagHiSlot0 + 8); ++slot)
                if ( m_modules[slot] != nullptr )
                    modVec.push_back(m_modules[slot]);
        } break;
    }
}

GenericModule* ModuleManager::GetModule(EVEItemFlags flag)
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.find((uint8)flag);
    if (itr != m_modules.end())
        return itr->second;

    return nullptr;
}

GenericModule* ModuleManager::GetModule(uint32 itemID)
{
    InventoryItemRef iRef = sItemFactory.GetItem(itemID);
    if (iRef.get() != nullptr)
        return GetModule(iRef->flag());

    return nullptr;
}

GenericModule* ModuleManager::GetRandModule()
{
    std::vector<GenericModule*> modVec;
    for (uint8 flag = flagLowSlot0; flag < flagFixedSlot; ++flag)
        if (m_modules[flag] != nullptr)
            modVec.push_back(m_modules[flag]);

    if (modVec.empty () == true)
        return nullptr;

    return modVec[MakeRandomInt(0, modVec.size())];
}

bool ModuleManager::InstallRig(ModuleItemRef mRef, EVEItemFlags flag) {
    if (((mRef->groupID() >= EVEDB::invGroups::Rig_Armor) and (mRef->groupID() <= EVEDB::invGroups::Rig_Astronautic))
    or (mRef->groupID() == EVEDB::invGroups::Rig_Electronics_Superiority)) {
        AddModule(mRef,flag);
        // hack to get total scan bonus from rigs, if applicable
        // do we need to check for and set anything else here?
        if (mRef->groupID() == EVEDB::invGroups::Rig_Electronics) {
            switch (mRef->typeID()) {
                case 25936:   // Large Gravity Capacitor Upgrade I
                case 31213:   // Small Gravity Capacitor Upgrade I
                case 31215:   // Medium Gravity Capacitor Upgrade I
                case 31217:   // Capital Gravity Capacitor Upgrade I
                case 26350:   // Large Gravity Capacitor Upgrade II
                case 31220:   // Small Gravity Capacitor Upgrade II
                case 31222:   // Medium Gravity Capacitor Upgrade II
                case 31224: { // Capital Gravity Capacitor Upgrade II
                    m_rigScanBonus += (0.01 * mRef->GetAttribute(AttrScanStrengthBonus).get_float());
                } break;
            }
        }
        return true;
    } else {
        codelog(MODULE__TRACE, "ModuleManager","%s tried to fit item %s(%u), which is not a rig", pShipItem->GetPilot()->GetName(), mRef->name(), mRef->itemID());
    }

    return false;

    /*
    10%
    15%
    */
}

void ModuleManager::UninstallRig(uint32 itemID)
{
    GenericModule* pMod = GetModule(itemID);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::UninstallRig() -  Rig %u not found", itemID);
        return;
    }

    _log(MODULE__TRACE, "%s(%u) calling MM::UninstallRig()", pMod->GetSelf()->name(), pMod->itemID());

    pMod->Offline();

    if (pMod->groupID() == EVEDB::invGroups::Rig_Electronics) {
        switch (pMod->typeID()) {
            case 25936:   //  Large Gravity Capacitor Upgrade I
            case 31213:   //  Small Gravity Capacitor Upgrade I
            case 31215:   //  Medium Gravity Capacitor Upgrade I
            case 31217:   //  Capital Gravity Capacitor Upgrade I
            case 26350:   //  Large Gravity Capacitor Upgrade II
            case 31220:   //  Small Gravity Capacitor Upgrade II
            case 31222:   //  Medium Gravity Capacitor Upgrade II
            case 31224: { //  Capital Gravity Capacitor Upgrade II
                m_rigScanBonus -= (0.01 * pMod->GetAttribute(AttrScanStrengthBonus).get_float());
            } break;
        }
    }

    if (!sConfig.debug.IsTestServer)
        pMod->RemoveRig();
    deleteModuleRef(pMod->flag(), pMod);
}

bool ModuleManager::InstallSubSystem(ModuleItemRef mRef, EVEItemFlags flag)
{
    if (mRef->categoryID() != EVEDB::invCategories::Subsystem) {
        sLog.Warning("ModuleManager","%s tried to fit %s(%u) at %s, which is not a subsystem", \
                pShipItem->GetPilot()->GetName(), mRef->name(), mRef->itemID(), sDataMgr.GetFlagName(flag));
        return false;
    }

    AddModule(mRef,flag);
    return true;
}

// not used
void ModuleManager::CheckSlotFitLimited(EVEItemFlags flag)
{
    if (IsRigSlot(flag))
        if (m_RigSlots)
            return;
    if (IsHiSlot(flag))
        if (m_HighSlots)
            return;
    if (IsMidSlot(flag))
        if (m_MidSlots)
            return;
    if (IsLowSlot(flag))
        if (m_LowSlots)
            return;
    if (IsSubSystem(flag))
        if (m_SubSystemSlots)
            return;

    throw UserError ("NoFreeShipSlots");
}

// not used
void ModuleManager::CheckGroupFitLimited(EVEItemFlags flag, InventoryItemRef iRef)
{
    if (iRef->HasAttribute(AttrMaxGroupFitted)) {
        // some of these are checked client-side (by attrib) so this may not be needed.
        if (GetFittedModuleCountByGroup(iRef->groupID()) >= iRef->GetAttribute(AttrMaxGroupFitted).get_int()) {
            /*
            std::map<std::string, PyRep *> args;
            args["noOfModules"]         = new PyInt(iRef->GetAttribute(AttrMaxGroupFitted).get_int());
            args["noOfModulesFitted"]   = new PyInt(GetFittedModuleCountByGroup(iRef->groupID()));
            args["ship"]                = new PyInt(pShipItem->itemID());
            args["groupName"]           = new PyString(sDataMgr.GetGroupName(iRef->groupID()));
            args["module"]              = new PyInt(iRef->itemID());
            throw PyException( MakeUserError("CantFitTooManyByGroup", args));   // bad msgID in client.
            */
            throw CustomError ("Group Fit Limited.<br>You cannot fit the %s to your %s.", iRef->name(), pShipItem->name());
            /*CantFitTooManyByGroupBody'}(
             * u"You're unable to fit {[item]module.name} to {[item]ship.name}.
             * You can only fit {[numeric]noOfModules} of type {groupName} but already have {[numeric]noOfModulesFitted}.", None,
             * {u'{[numeric]noOfModules}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'noOfModules'},
             * u'{[numeric]noOfModulesFitted}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'noOfModulesFitted'},
             * u'{[item]ship.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'ship'},
             * u'{groupName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'groupName'},
             * u'{[item]module.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'module'}})
             */
        }
    }
}

void ModuleManager::UnfitModule(uint32 itemID)
{
    GenericModule* pMod = GetModule(itemID);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::UnfitModule() -  Module %u not found", itemID);
        return;
    }

    _log(MODULE__TRACE, "%s(%u) calling MM::UnfitModule(itemID)", pMod->GetSelf()->name(), pMod->itemID());

    pMod->AbortCycle();
    if (pMod->IsLoaded())
         UnloadCharge(pMod);

    if (pMod->IsLinked())
        pShipItem->UnlinkGroup(itemID, true);

    pMod->Offline();

    deleteModuleRef(pMod->flag(), pMod);
    // delete the GenericModule object (but not the ModuleItem object)
    SafeDelete(pMod);
}

void ModuleManager::UnfitModule(EVEItemFlags flag) {
    GenericModule* pMod = GetModule(flag);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::UnfitModule() -  Module not found at %s", sDataMgr.GetFlagName(flag));
        return;
    }

    _log(MODULE__TRACE, "%s(%u) calling MM::UnfitModule(flag)", pMod->GetSelf()->name(), pMod->itemID());

    pMod->AbortCycle();
    if (pMod->IsLoaded())
        UnloadCharge(pMod);

    if (pMod->IsLinked())
        pShipItem->UnlinkGroup(pMod->itemID(), true);

    pMod->Offline();

    deleteModuleRef(flag, pMod);
    // delete the GenericModule object (but not the ModuleItem object)
    SafeDelete(pMod);
}

// cannot throw without test
bool ModuleManager::AddModule(ModuleItemRef mRef, EVEItemFlags flag)
{
    if (!IsModuleSlot(flag)) {
        sLog.Warning("MM::AddModule","%s is not a module slot.", sDataMgr.GetFlagName(flag));
        return false;
    }
    if (IsSlotOccupied(flag)) {
        GenericModule* pMod = GetModule(flag);
        if (pMod == nullptr)
            return false;

        if (pShipItem->HasPilot()) {
            if (!pShipItem->GetPilot()->IsLogin()) {
                if (pShipItem->GetPilot()->CanThrow()) {
                    pShipItem->GetPilot()->SendErrorMsg("You cannot add %s to %s because %s is already there.", \
                            mRef->name(), sDataMgr.GetFlagName(flag), pMod->GetSelf()->name());
                } else {
                    throw UserError ("SlotAlreadyOccupied");
                }
            }
        }

        // change this to use movemodule?
        return false;
    }

    // create new module object
    GenericModule* pMod = ModuleFactory(mRef, ShipItemRef(pShipItem));
    if (pMod == nullptr)
        return false; // error here?

    pMod->SetModuleState(Module::State::Offline);

    addModuleRef(flag, pMod);

    if (m_initalized and pShipItem->HasPilot())
        if (mRef->GetAttribute(AttrOnline).get_bool())
            pMod->Online();

    // verify module's singleton flag is set
    mRef->ChangeSingleton(true, pShipItem->HasPilot()?pShipItem->GetPilot()->IsInSpace():false);

    return true;
    /*
    if (is_log_enabled(MODULE__DEBUG)) { // debug msg?
        std::map<std::string, PyRep *> args;
        args["item"]  = new PyString(iRef->itemName());
        args["slot"]  = new PyString(sDataMgr.GetFlagName(flag));
        throw PyException( MakeUserError("ModuleFit", args));
        */
    /*{'messageKey': 'ModuleFit', 'dataID': 17883325, 'suppressable': False, 'bodyID': 259463, 'messageType': 'notify', 'urlAudio': 'wise:/msg_ModuleFit_play', 'urlIcon': '', 'titleID': None, 'messageID': 1227}
     * u'ModuleFitBody'}(u'{item} fitted onto slot {slot}', None, {
     * u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'},
     * u'{slot}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'slot'}})
     *
    }*/
}

void ModuleManager::Online(uint32 itemID)
{
    GenericModule* pMod = GetModule(itemID);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::Online(itemID) -  Module %u not found", itemID);
        return;
    }
    if (pMod->isOnline()) {
        _log(MODULE__WARNING, "MM::Online(itemID) -  %s already Online", pMod->GetSelf()->name());
        if (pShipItem->HasPilot())
            if (pShipItem->GetPilot()->CanThrow()) {
                throw UserError ("EffectAlreadyActive2")
                        .AddTypeName ("modulename", pMod->GetSelf ()->typeID ());
            }
        return;
    }

    _log(MODULE__MESSAGE, "MM::Online(itemID) -  %s going Online", pMod->GetSelf()->name());
    pMod->Online();
}

void ModuleManager::Online(EVEItemFlags flag)
{
    GenericModule* pMod = GetModule(flag);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::Online(flag) -  Module not found in %s", sDataMgr.GetFlagName(flag));
        return;
    }
    if (pMod->isOnline()) {
        _log(MODULE__WARNING, "MM::Online(flag) -  %s already Online", pMod->GetSelf()->name());
        return;
    }

    _log(MODULE__MESSAGE, "MM::Online(flag) -  %s going Online", pMod->GetSelf()->name());
    pMod->Online();
}

void ModuleManager::Offline(uint32 itemID)
{
    GenericModule* pMod = GetModule(itemID);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::Offline(itemID) -  Module %u not found", itemID);
        return;
    }
    if (!pMod->isOnline()) {
        _log(MODULE__WARNING, "MM::Offline(itemID) -  %s not Online", pMod->GetSelf()->name());
        pMod->SetModuleState(Module::State::Offline);
        return;
    }

    _log(MODULE__MESSAGE, "MM::Offline(itemID) -  %s going Offline", pMod->GetSelf()->name());
    pMod->Offline();
}

void ModuleManager::Offline(EVEItemFlags flag)
{
    GenericModule* pMod = GetModule(flag);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::Offline(flag) -  Module not found in %s", sDataMgr.GetFlagName(flag));
        return;
    }
    if (!pMod->isOnline()) {
        _log(MODULE__WARNING, "MM::Offline(flag) -  %s not Online", pMod->GetSelf()->name());
        pMod->SetModuleState(Module::State::Offline);
        return;
    }
    _log(MODULE__MESSAGE, "MM::Offline(flag) -  %s going Offline", pMod->GetSelf()->name());
    pMod->Offline();
}

void ModuleManager::AbortCycle()
{
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            cur.second->AbortCycle();
}

void ModuleManager::OnlineAll()
{
    // must proc modules in order of (subsys -> rig -> high -> mid -> low) for proper fx application
    std::map<uint8, GenericModule*>::reverse_iterator itr = m_modules.rbegin(), end = m_modules.rend();
    while (itr != end) {
        if (itr->second != nullptr)
            itr->second->Online();
        ++itr;
    }
}

void ModuleManager::OfflineAll()
{
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            cur.second->Offline();
}

void ModuleManager::DeactivateAllModules()
{
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            cur.second->Deactivate();
}

void ModuleManager::Activate(int32 itemID, uint16 effectID, int32 targetID, int32 repeat)
{
    if (!pShipItem->HasPilot()) {
        _log(MODULE__ERROR, "MM::Activate() - Called from a ship with no pilot." );
        return;
    }

    DestinyManager* pDestiny = pShipItem->GetPilot()->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        _log(PLAYER__ERROR, "%s: Ship has no destiny manager!", pShipItem->GetPilot()->GetName());
        return;
    }

    GenericModule* pMod = GetModule(itemID);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::Activate() - Called on module %u that is not loaded.", itemID );
        return;
    }

    _log(MODULE__TRACE, "MM::Activate() - %s (%u - %s)  targetID: %i, repeat: %i.", \
                pMod->GetSelf()->name(), effectID, sFxDataMgr.GetEffectName(effectID).c_str(), targetID, repeat);

    if (effectID == 16) { //16    online
        pMod->Online();
        return;
    }
    /*{'FullPath': u'UI/Messages', 'messageID': 259628, 'label': u'InvalidTargetCanAlreadyTractoredBody'}(u'The {[item]module.name} cannot engage a tractor beam on that object as it is already being tractor beamed by something else.', None, {u'{[item]module.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'module'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259629, 'label': u'InvalidTargetCanOwnerBody'}(u'The {[item]module.name} cannot engage a tractor beam on that object as it is not owned by you, a fellow fleet member or another member of a player corporation you belong to.', None, {u'{[item]module.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'module'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259630, 'label': u'InvalidTargetGroupBody'}(u'Invalid target, can only activate this on {groupName}.', None, {u'{groupName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'groupName'}})
     */

    if (effectID == 2255) { // tractorBeamCan
        SystemEntity* pSE = pShipItem->GetPilot()->SystemMgr()->GetSE(targetID);
        if (pSE == nullptr)
            throw UserError ("DeniedActivateTargetNotPresent");
        if (pSE->DestinyMgr()->IsTractored()) {
            // report player tractoring item?
            pShipItem->GetPilot()->SendNotifyMsg("Your %s cannot engage the %s, which is already being tractor beamed by something else.", pMod->GetSelf()->name(), pSE->GetName());
            return;
        //std::map<std::string, PyRep *> args;
        //args["module"]  = new PyInt(itemID);
        //throw PyException(MakeUserError("InvalidTargetCanAlreadyTractored", args));
        }
    }

    if (!pMod->isOnline()) {
        // client wont allow activating an offline module.  this is catchall. (but should never hit)
        pShipItem->GetPilot()->SendErrorMsg("You cannot activate an offline module. Ref: ServerError 25164");
        return;
    } else if (pDestiny->IsWarping()) {
        if (pMod->HasAttribute(AttrDisallowActivateOnWarp) or !sFxDataMgr.isWarpSafe(effectID))
            throw UserError ("DeniedActivateInWarp");
    } else if (pDestiny->IsCloaked()) {
        throw UserError ("DeniedActivateCloaked");
    } else if (pShipItem->GetPilot()->IsJump()) {
        throw UserError ("DeniedActivateInJump");
    }

    if (!pMod->IsLinked() or pMod->IsMaster())
        pMod->Activate(effectID, targetID, repeat);
}

void ModuleManager::Deactivate(uint32 itemID, std::string effectName)
{
    GenericModule* pMod = GetModule(itemID);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::Deactivate() - Called on module %u that is not loaded.", itemID );
        return;
    }

    // test for effectName "online", which is sent thru rclick menu in HUD to offline module
    if (effectName.compare("online") == 0) {
        _log(MODULE__TRACE, "MM::Deactivate() - %s Offlining - '%s'", pMod->GetSelf()->name(), effectName.c_str());
        pMod->Offline();
        return;
    }
    if (pMod->GetModuleState() != Module::State::Activated)  // we dont need an error msgs here....this is acceptable, as the module may not be active
        return;

    _log(MODULE__TRACE, "MM::Deactivate() - %s Deactivating - '%s'", pMod->GetSelf()->name(), effectName.c_str());
    pMod->Deactivate(effectName);
}

void ModuleManager::Overload(uint32 itemID)
{
    GenericModule* pMod = GetModule(itemID);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::Overload() - Called on module %u that is not loaded.", itemID);
        return;
    }
    _log(MODULE__TRACE, "MM::Overload() - %s Overloading...", pMod->GetSelf()->name());
    pMod->Overload();
}

void ModuleManager::DeOverload(uint32 itemID)
{
    GenericModule* pMod = GetModule(itemID);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::DeOverload() - Called on module %u that is not loaded.", itemID);
        return;
    }
    _log(MODULE__TRACE, "MM::DeOverload() - %s DeOverload...", pMod->GetSelf()->name());
    pMod->DeOverload();
}

void ModuleManager::DamageModule(uint32 itemID, float amount)
{
    DamageModule(GetModule(itemID), amount);
}

void ModuleManager::DamageRandModule()
{
    DamageModule(GetRandModule(), 1.0f);
}

void ModuleManager::DamageRandModule(float amount)
{
    DamageModule(GetRandModule(), amount);
}

void ModuleManager::DamageModule(GenericModule* pMod, float amount)
{
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::DamageModule() - Module not found.");
        return;
    }

    pMod->SetAttribute(AttrDamage, (pMod->GetAttribute(AttrDamage) + amount));  //verify this works as intended
    _log(MODULE__DAMAGE, "MM::DamageModule() - %s taking %.2f damage.  current damage %.2f",  \
                pMod->GetSelf()->name(), amount, pMod->GetAttribute(AttrDamage).get_float());
    if (pMod->GetAttribute(AttrDamage) >= pMod->GetAttribute(AttrHP)) {
        //  this is for offlining entire group...this isnt right.
        /*
        if (pMod->IsLinked()) {
            // loop thru linked modules and offline all
            pShipItem->GetPilot()->SendNotifyMsg("Your group of %s has gone offline due to damage.", pMod->GetSelf()->name());
            pShipItem->OfflineGroup(pMod);
        } else */
        pShipItem->GetPilot()->SendNotifyMsg("Your %s in %s has gone offline due to damage.", pMod->GetSelf()->name(), sDataMgr.GetFlagName(pMod->flag()));
        pMod->Offline();
    }
}

void ModuleManager::RepairModule(uint32 itemID, EvilNumber amount)
{
    RepairModule(GetModule(itemID), amount);
}

void ModuleManager::RepairModule(GenericModule* pMod, EvilNumber amount)
{
    if (pMod != nullptr){
        _log(MODULE__ERROR, "MM::RepairModule() - Called on module that is not loaded.");
        return;
    }
    pMod->Repair(amount);
}

void ModuleManager::RepairModules()
{
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            cur.second->Repair();
}

PyRep* ModuleManager::ModuleRepair(uint32 modID)
{
    /*  Restrictions/Capabilities
     *
     *    Cannot be used while overloading any modules.
     *    Cannot be used on an active module.
     *    Cannot be used to repair a 100% damaged module (0/40hp). These must first be repaired to at least 1hp at a station.
     *    Can be used to repair an offline module with at least 1hp remaining.
     *    Can be used on an inactive module while other modules are active.
     *    Can be used to repair any inactive modules while cloaked (everything but the cloak itself, of course).
     *    Can repair multiple modules at once.
     *    Can be canceled mid-repair, and will retain whatever repairs could be completed in the time it was active. Canceling a repair on a module that takes 1-2 paste to repair fully is occasionally problematic, and will round down - e.g. you need to have repaired enough HP to take at least one unit of paste before canceling mid-repair will result in any repaired damage or paste consumed. No paste is ever consumed without appropriate repairs being done, however.
     *    You can jump or dock while repairing, which will have the same effect as canceling the repair manually.
     *    You can repair a passive module (such as a plate, extender, or EANM) without taking it offline, and you still receive the benefit from passive modules while repairing them. Capacitor batteries were fixed and can now be repaired while online!
     *
     * Efficiency
     *
     *    Nanite efficiency is based on the base cost of the module, rather than amount of HP repaired.
     *    All modules have 40hp, but base cost varies wildly.
     *    Base efficiency for a theoretical (but impossible) full repair (0/40hp remaining) is approximately 0.0000775 paste per isk of base item cost.
     *    To couch this in more relatable terms, this means that an item with a base cost of 100k isk will cost 7 or 8 paste to repair; an item with a base cost of 1m isk will cost 77-78 paste to repair.
     *    The Nanite Operation skill reduces consumption by 5% per level. At V, nanite efficiency will be 0.000058125/isk, or ~58 units of paste per 1m isk base cost.
     *    NPC station repair costs are equal to the item base cost, modified marginally by standing. Base repair cost with nanites is 7.7x11700=90,090 per 100,000 isk, or 10% less. Even without the Nanite Operation skill, repairs with nanite paste are always slightly cheaper than repairs at NPC stations. With Nanite Operation trained up, repairing with nanite paste is significantly cheaper than at NPC stations.
     *
     * Speed
     *
     *    Nanite paste has a base repair speed of 10hp per minute, independent of module type or cost.
     *    As all modules have 40hp, and paste cannot be used to repair 100% damaged items, the most time a repair can take at base skill levels is 3:54 on a 97% damaged module.
     *    The Nanite Interfacing skill improves repair speed by 20% (or 2hp) per level. At V, repair rate is 20hp per minute with a max repair time of 1:57.
     *
     * Module Repair Costs
     *
     * Again, the quantity of nanite paste consumed to repair an item is dependent on its base cost.
     * You can easily find the base cost of a module by looking it up in the Item Browser subsection of Evemon's skill plans.
     *
     *    As a general rule, base cost is usually around 1/4 of the Empire price of an item. This mostly applies to T1 items, but T2 items for which demand is not extremely high tend to follow this as well.
     *    As you might suspect, battleship-class modules have much higher base costs than frigate-class modules. This applies mostly to Afterburners, Microwarpdrives, Armor Repairers, and Shield Boosters. Most larger modules also produce less heat damage however, so the difference in nanites consumed per amount of time overheated is not as pronounced.
     *    T2 items have the highest base cost at around 2-6x that of T1.
     *    Named items often have lower base costs than T1, and are never higher.
     *    Faction, Deadspace, and Officer items have wildly varying base costs.
     * Most are similar or identical to named, others T1, and a few are higher than T1 but still much lower than T2.
     * Officer/Deadspace MWDs are an odd exception to this, as all of them have the same base cost regardless of size class (790k).
     * In most cases, this makes faction items prime candidates for overloading as they produce similar or less heat damage while being radically more
     * effective and cheaper to repair than their T1 or T2 counterparts.
     */

    GenericModule* pMod = GetModule(modID);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::ModuleRepair() - module %s not found.", modID);
        return PyStatic.NewFalse();
    }

    //return PyStatic.NewTrue();  // can repair
    return PyStatic.NewFalse(); // cannot repair (for whatever reason)  do they/we send msgs based on why here?
}

void ModuleManager::StopModuleRepair(uint32 modID)
{
    GenericModule* pMod = GetModule(modID);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::ModuleRepair() - module %s not found.", modID);
        return;
    }
}

void ModuleManager::LoadCharge(InventoryItemRef chargeRef, EVEItemFlags flag)
{
    GenericModule* pMod = GetModule(flag);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::LoadCharge() - module not found at %s", sDataMgr.GetFlagName(flag));
        return;
    }
    float modCapacity = pMod->GetAttribute(AttrCapacity).get_float();
    float chargeVolume = chargeRef->GetAttribute(AttrVolume).get_float();

    bool loaded = pMod->IsLoaded();

    if (loaded) {
        if (chargeRef->typeID() == pMod->GetLoadedChargeRef()->typeID()) {
            modCapacity -= (chargeVolume * pMod->GetLoadedChargeRef()->quantity());
            _log(MODULE__TRACE, "MM::LoadCharge() - %s reloading with same type. remaining capy:%.2f", pMod->GetSelf()->name(), modCapacity);
        } else {
            // change charges
            UnloadCharge(pMod);
            loaded = false;
            // update module capy
            modCapacity = pMod->GetAttribute(AttrCapacity).get_float();
            _log(MODULE__TRACE, "MM::LoadCharge() - %s reloading with different type. empty capy:%.2f", pMod->GetSelf()->name(), modCapacity);
        }
    } else {
        _log(MODULE__TRACE, "MM::LoadCharge() - %s not loaded. capy:%.2f", pMod->GetSelf()->name(), modCapacity);
    }

    //{'FullPath': u'UI/Messages', 'messageID': 256676, 'label': u'CannotLoadNotEnoughChargesBody'}(u'There are not enough charges to fully load all of your modules. Some of your modules have been left partially loaded or empty.', None, None)

    // check quantities
    if (modCapacity < chargeVolume)
        return;

    int32 loadQty = floor((modCapacity / chargeVolume));
    if (loadQty < 1)
        return;

    if (loadQty < chargeRef->quantity()) {
        InventoryItemRef cRef(chargeRef);
        chargeRef = chargeRef->Split(loadQty, false, true);
        if (chargeRef.get() == nullptr) {
            _log(MODULE__ERROR, "");
            if (pShipItem->HasPilot())
                pShipItem->GetPilot()->SendNotifyMsg("Could not split stack of %s.  %s was not reloaded.", \
                        cRef->name(), pMod->GetSelf()->name());
            return;
        }
    } else {
        loadQty = chargeRef->quantity();
    }

    /* loaded charges are set in a "SubLocation" in client.
     * they then become a nullItem, with only type and qty references.
     * the sublocation is identified as a tuple of itemKey(shipID, flagID, typeID)
     * all updates for that charge (which seems to be only attrib changes), are referenced using it's itemKey
     * when sending charge item change with locationID or flagID, client will correctly add as sublocation
     *    when in space, but only for the first update.  subsequent changes need to use OnModuleAttributeChange (OMAC)
     *
     * when docked, List() sends charges as invItem, and client doesnt process that as a sublocation.  (OMAC method)
     * because of this, OMAC isn't used when docked, and OnItemChange (OIC) is the update client will correctly process.
     */

    if (loaded) {
        //  merge addt'l charges with currently loaded charges (fillup)
        pMod->GetLoadedChargeRef()->Merge(chargeRef, loadQty);
    } else {
        // if module wasnt previously loaded, add to ship's inventory and charge map
        chargeRef->SetAttribute(AttrQuantity, EvilZero, false);    // OMAC
        chargeRef->Move(pShipItem->itemID(), flag, pShipItem->HasPilot()?pShipItem->GetPilot()->IsDocked():false);
        //chargeRef->Move(pShipItem->itemID(), flag, false);
        m_charges.emplace(flag, chargeRef);
    }

    // this will enable module loading blink if ship in space, even on reload/fillup
    pMod->LoadCharge(chargeRef);

    //  just found a call i was missing.... 19Aug20
    /*
     *                  [PyTuple 3 items]
     *                    [PyString "OnGodmaPrimeItem"]
     *                    [PyIntegerVar 1002332228246]  <<-- item locationID
     *                    [PyObjectData Name: util.KeyVal]
     *                      [PyDict 5 kvp]
     *                        [PyString "itemID"]
     *                        [PyTuple 3 items]
     *                          [PyIntegerVar 1002332228246]
     *                          [PyInt 27]
     *                          [PyInt 30028]
     *                        [PyString "attributes"]
     *                        [PyDict 23 kvp]
     *                        [PyString "invItem"]
     *                        [PyNone]
     *                        [PyString "time"]
     *                        [PyIntegerVar 129527520208602524]
     *                        [PyString "activeEffects"]
     *                        [PyDict 0 kvp]
     *    [PyDict 1 kvp]
     *      [PyString "sn"]
     *      [PyIntegerVar 131]
     */
    // send update to client for processing new subLocation   -found/added 19Aug20
    if (!loaded)
        if (pShipItem->HasPilot() and pShipItem->GetPilot()->IsInSpace()) {
            Rsp_CommonGetInfo_Entry entry2;
            if (chargeRef->Populate(entry2)) {
                PyTuple* tuple = new PyTuple(3);
                    tuple->SetItem(0, new PyInt(chargeRef->locationID()));
                    tuple->SetItem(1, new PyInt(chargeRef->flag()));
                    tuple->SetItem(2, new PyInt(chargeRef->typeID()));
                PyTuple* result = new PyTuple(2);
                    result->SetItem(0, new PyInt(chargeRef->locationID()));
                    result->SetItem(1, new PyObject("util.KeyVal", entry2.Encode()));
                pShipItem->GetPilot()->SendNotification("OnGodmaPrimeItem", "clientID", result);     // this is sequenced
            } else {
                sLog.Error("MM::LoadCharge","cannot Populate() %s", chargeRef->name());
            }
        }

    //chargeRef->SetQuantity(loadQty, true);                  // OIC
    chargeRef->SetAttribute(AttrQuantity, loadQty, true);   // OMAC
}

void ModuleManager::UnloadCharge(GenericModule* pMod)
{
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::UnloadCharge() - module invalid");
        return;
    }

    _log(MODULE__TRACE, "%s(%u) calling MM::UnloadCharge()", pMod->GetSelf()->name(), pMod->itemID());

    // verify no charge at flag in map
    m_charges.erase(pMod->flag());

    if (!pMod->IsLoaded()) {
        _log(MODULE__ERROR, "MM::UnloadCharge() - %s at %s is not loaded", \
                pMod->GetSelf()->name(), sDataMgr.GetFlagName(pMod->flag()));
        return;
    }

    InventoryItemRef chargeRef(pMod->GetLoadedChargeRef());
    if (chargeRef.get() == nullptr) {
        _log(MODULE__ERROR, "MM::UnloadCharge() - charge not found on module %s at %s", \
                pMod->GetSelf()->name(), sDataMgr.GetFlagName(pMod->flag()));
        return;
    }

    pMod->UnloadCharge();

    // if charge is depleted, update has already been sent to client thru OMAC
    if (chargeRef->quantity() < 1)
        return;

    // at this point, we are changing a loaded charge.  remove charge from module, check for stacking, update client.
    /* to trigger client to remove and clear out a loaded charge, send a qty=0 update using either
     *  OnModuleAttributeChange thru Godma when in space or OnItemChange thru InventoryItem when docked
     */

    chargeRef->SetAttribute(AttrQuantity, EvilZero);    // OMAC

    if (sDataMgr.IsStation(pShipItem->locationID())) {
        StationItemRef sRef = sEntityList.GetStationByID(pShipItem->locationID());
        if (sRef.get() != nullptr) {
            InventoryItemRef iRef = sRef->GetMyInventory()->GetByTypeFlag(chargeRef->typeID(), flagHangar);
            if (iRef.get() != nullptr) {
                // merge with existing item
                iRef->Merge(chargeRef);         // OIC  preferred
                //iRef->AlterQuantity(chargeRef->quantity() + iRef->quantity(), true);
                //chargeRef->SetQuantity(0, true);         // OIC alternate
                return;
            } else {
                chargeRef->Move(pShipItem->locationID(), flagHangar, true);
            }
        } else { // this will be an error.  cant find station ship is docked in
            _log(MODULE__ERROR, "MM::UnloadCharge() - Station %u not found for ship %u owned by %s",\
                    pShipItem->locationID(), pShipItem->itemID(), \
                    pShipItem->HasPilot()? pShipItem->GetPilot()->GetName() : itoa(pShipItem->ownerID()));
            chargeRef->Move(pShipItem->locationID(), flagHangar, true);
        }
    } else {
        /*
        EVEItemFlags flag = flagCargoHold;
        // check for existence of ammohold...not yet.  only one ship has it (cockroach)
        if (pShipItem->HasAttribute(AttrAmmoHoldCapacity)) {
            // verify hold capy
            if (pShipItem->GetMyInventory()->HasAvailableSpace(flag, chargeRef))
                flag = flagAmmoHold;
        } else
        */
        if (pShipItem->GetMyInventory()->HasAvailableSpace(flagCargoHold, chargeRef)) {
            InventoryItemRef iRef = pShipItem->GetMyInventory()->GetByTypeFlag(chargeRef->typeID(), flagCargoHold);
            if (iRef.get() != nullptr) {
                // merge with existing item
                // in case we're merging into existing stack, get current qty
                int16 qty(chargeRef->quantity());
                chargeRef->Delete();
                iRef->AlterQuantity(qty, true);
                return;
            }

            chargeRef->Relocate(pShipItem->itemID(), flagCargoHold);
        } else {
            // dunno what to do at this point....the charge is already removed but cargo is full...pilot not paying attention
            // fuckit....space the bitch
            if (pShipItem->HasPilot())
                pShipItem->GetPilot()->SendNotifyMsg("Your cargo is full, so your %s was jettisoned and lost.", chargeRef->name());

            chargeRef->Delete();
        }
    }
}

InventoryItemRef ModuleManager::GetLoadedChargeOnModule(EVEItemFlags flag) {
    GenericModule* pMod = GetModule(flag);
    if ((pMod != nullptr) and pMod->IsLoaded() )
        return pMod->GetLoadedChargeRef();
    return InventoryItemRef(nullptr);
}

InventoryItemRef ModuleManager::GetLoadedChargeOnModule(InventoryItemRef moduleRef) {
    return GetLoadedChargeOnModule(moduleRef->flag());
}

void ModuleManager::UnloadModule(uint32 itemID) {
    GenericModule* pMod = GetModule(itemID);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::UnloadModule() - module not found for %u", itemID);
        return;
    }

    _log(MODULE__TRACE, "%s(%u) calling MM::UnloadModule(item)", pMod->GetSelf()->name(), pMod->itemID());

    UnloadCharge(pMod);
}

void ModuleManager::UnloadModule(EVEItemFlags flag) {
    GenericModule* pMod = GetModule(flag);
    if (pMod == nullptr) {
        _log(MODULE__ERROR, "MM::UnloadModule() - module not found at %s", sDataMgr.GetFlagName(flag));
        return;
    }

    _log(MODULE__TRACE, "%s(%u) calling MM::UnloadModule(flag)", pMod->GetSelf()->name(), pMod->itemID());

    UnloadCharge(pMod);
}

void ModuleManager::UnloadModule(GenericModule* pMod) {
    _log(MODULE__TRACE, "%s(%u) calling MM::UnloadModule(mod)", pMod->GetSelf()->name(), pMod->itemID());

    UnloadCharge(pMod);
}

void ModuleManager::UnloadWeapons()
{
    std::map<uint8, GenericModule*>::iterator mItr;
    for (uint8 i = flagHiSlot0; 1 < flagFixedSlot; ++i) {
        mItr = m_modules.find((uint8)i);
        if (mItr->second != nullptr)
            UnloadCharge(mItr->second);
    }
}

void ModuleManager::UnloadAllModules()
{
    /** @todo look into this....is this right?  */
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            cur.second->UnloadCharge();
    // can this be called when docked?
    //bool docked = sDataMgr.IsStation(pShipItem->locationID());
    for (auto cur : m_charges)
        UnloadModule(cur.second->flag());

    m_charges.clear();
}

void ModuleManager::UpdateModules(std::vector<uint32> modVec)
{
    if (is_log_enabled(MODULE__WARNING))
        sLog.Magenta("MM::UpdateModules()","testing");
    // this one is called from BoardShip() and Ship::Undock()
    pShipItem->SetAttribute(AttrCpuLoad,     EvilZero);
    pShipItem->SetAttribute(AttrPowerLoad,   EvilZero);
    pShipItem->SetAttribute(AttrUpgradeLoad, EvilZero);
    //OfflineAll();   // set all modules to offline.  this verifies the following Online() call will only online previously-set modules.  (elusive error)
    if (!modVec.empty()) {
        _log(MODULE__TRACE, "MM::UpdateModules(modVec)");
        // gotta add rigs and Subsystems to the vector, as they wont be listed in the "modules to online" list when undocking.
        GetShipRigs(modVec);
        GetShipSubSystems(modVec);
        std::vector< GenericModule* > modList;
        SortModulesBySlotDec(modVec, modList);
        /** @todo check this.  may have to rework */
        for (auto cur : modList) {
            cur->Update();
            cur->Online();
        }
    }
}

void ModuleManager::UpdateModules(EVEItemFlags flag)
{
    /** @todo  figure out what needs to be done here and implement it. */
    //  this should update all ship attribs for this bank.
    if (is_log_enabled(MODULE__WARNING))
        sLog.Magenta("MM::UpdateModules(flag)","Needs to be implemented");

    // reset ship and module effect data, and reapply?
    // call ProcessEffects(false), ApplyEffects(), then UpdateModules() ?
    std::vector< GenericModule* > modVec;
    // this returns only populated modules for this bank
    GetModulesInBank(flag, modVec);
    for (auto cur : modVec)
        cur->Online();
}

void ModuleManager::CharacterBoardingShip()
{
    if (is_log_enabled(MODULE__WARNING))
        sLog.Magenta("MM::CharacterBoardingShip()","Needs to be tested");
    if (!m_initalized)
        Initialize();
    std::vector<uint32> modVec;
    GetShipRigs(modVec);
    GetShipSubSystems(modVec);
    std::vector< GenericModule* > modList;
    SortModulesBySlotDec(modVec, modList);
    /** @todo check this.  may have to rework...do rigs and subsystems have online attrib set?  */
    for (auto cur : modList)
        if (cur->GetAttribute(AttrOnline).get_bool())
            cur->Online();
}

void ModuleManager::CharacterLeavingShip()
{
    // if ship is killed, no point setting modules to offline...just return
    if (pShipItem->IsPopped())
        return;

    //OfflineAll();

    /*  this is complicated and im gonna leave it alone for now
     * this will include checking ship HP, cargo holds, and possibably other things
     *  that havent been written yet.
     * see if these can throw, else we'll have to do a bool return from calls and go from there.
     *
     * will also have to check current levels of hp and cargo AFTER pilot has been removed (lost skills)
    if (is_log_enabled(MODULE__WARNING))
        sLog.Magenta("MM::CharacterLeavingShip()","Needs to be implemented");
     */
    //CheckNewHP();
    //CheckNewCargo();
}

void ModuleManager::ShipWarping()
{
    if (is_log_enabled(MODULE__WARNING))
        sLog.Magenta("MM::ShipWarping()","Deactivating non-warpsafe modules.");
    // check modules for warpsafe-ness and Deactivate accordingly
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            if (!cur.second->isWarpSafe())
                cur.second->AbortCycle();
}

void ModuleManager::ShipJumping()
{
    if (is_log_enabled(MODULE__WARNING))
        sLog.Magenta("MM::ShipJumping()","Deactivating all modules.");

    // no modules are jumpsafe
    AbortCycle();
}

void ModuleManager::CargoFull() {
    // loop thru modules and deactivate anything that drops items in cargo
    //std::string effect = "CargoFull";
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            switch (cur.second->groupID()) {
                case EVEDB::invGroups::Mining_Laser:
                case EVEDB::invGroups::Gas_Cloud_Harvester:
                case EVEDB::invGroups::Frequency_Mining_Laser:
                case EVEDB::invGroups::Strip_Miner:
                case EVEDB::invGroups::Salvager:
                case EVEDB::invGroups::Data_Miner: {
                    cur.second->Deactivate();
                } break;
                // no default
            }
}

void ModuleManager::GetWeapons(std::list< GenericModule* >& weaponList)
{
    for (uint8 flag = flagHiSlot0; flag < flagFixedSlot; ++flag)
        if (m_modules[flag] != nullptr)
            if (m_modules[flag]->IsLauncherModule() or m_modules[flag]->IsTurretModule())
                weaponList.push_back(m_modules[flag]);
}

void ModuleManager::GetModuleListOfRefsAsc(std::vector<InventoryItemRef>& modVec)
{
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            modVec.push_back(cur.second->GetSelf());
}

void ModuleManager::GetModuleListOfRefsDec(std::vector< InventoryItemRef >& modVec)
{
    std::map<uint8, GenericModule*>::reverse_iterator itr = m_modules.rbegin(), end = m_modules.rend();
    while (itr != end) {
        if (itr->second != nullptr)
            modVec.push_back( itr->second->GetSelf() );
        ++itr;
    }
}

void ModuleManager::GetModuleListOfRefsOrdered( std::vector< InventoryItemRef >& modVec )
{
    // this is to list all ship modules by order of subsys, rig, low, mid, hi
    // subsystems
    for (uint8 flag = flagSubSystem0; flag < flagSubSystem5; ++flag)
        if (m_modules[flag] != nullptr)
            modVec.push_back(m_modules[flag]->GetSelf());
    // rigs
    for (uint8 flag = flagRigSlot0; flag < flagRigSlot3; ++flag)
        if (m_modules[flag] != nullptr)
            modVec.push_back(m_modules[flag]->GetSelf());
    // low slots
    for (uint8 flag = flagLowSlot0; flag < flagMidSlot0; ++flag)
        if (m_modules[flag] != nullptr)
            modVec.push_back(m_modules[flag]->GetSelf());
    // mid slots
    for (uint8 flag = flagMidSlot0; flag < flagHiSlot0; ++flag)
        if (m_modules[flag] != nullptr)
            modVec.push_back(m_modules[flag]->GetSelf());
    // hi slots
    for (uint8 flag = flagHiSlot0; flag < flagFixedSlot; ++flag)
        if (m_modules[flag] != nullptr)
            modVec.push_back(m_modules[flag]->GetSelf());
}

void ModuleManager::GetModuleListOfRefsOrderedRev( std::vector< InventoryItemRef >& modVec ) {
    // this is to list all ship modules by order of hi, mid, low, rig, subsys
    // hi slots
    for (uint8 flag = flagHiSlot0; flag < flagFixedSlot; ++flag)
        if (m_modules[flag] != nullptr)
            modVec.push_back(m_modules[flag]->GetSelf());
    // mid slots
    for (uint8 flag = flagMidSlot0; flag < flagHiSlot0; ++flag)
        if (m_modules[flag] != nullptr)
            modVec.push_back(m_modules[flag]->GetSelf());
    // low slots
    for (uint8 flag = flagLowSlot0; flag < flagMidSlot0; ++flag)
        if (m_modules[flag] != nullptr)
            modVec.push_back(m_modules[flag]->GetSelf());
    // rigs
    for (uint8 flag = flagRigSlot0; flag < flagRigSlot3; ++flag)
        if (m_modules[flag] != nullptr)
            modVec.push_back(m_modules[flag]->GetSelf());
    // subsystems
    for (uint8 flag = flagSubSystem0; flag < flagSubSystem5; ++flag)
        if (m_modules[flag] != nullptr)
            modVec.push_back(m_modules[flag]->GetSelf());
}

void ModuleManager::GetModuleListByReqSkill(uint16 skillID, std::vector< InventoryItemRef >& modVec)
{
    std::vector<InventoryItemRef> moduleList;
    GetModuleListOfRefsAsc(moduleList);
    for (auto cur : moduleList)
        if (cur->HasReqSkill(skillID))
            modVec.push_back(cur);
}

void ModuleManager::SaveModules()
{
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            cur.second->GetSelf()->SaveItem();
    for (auto cur : m_charges)
        cur.second->SaveItem();
}

void ModuleManager::GetShipRigs(std::vector< uint32 >& modVec)
{
    // get rigs on ship, by itemID (there's only 3 slots...)
    GenericModule* pMod(nullptr);
    for (uint8 i = flagRigSlot0; i < flagRigSlot3; ++i) {
        pMod = GetModule((EVEItemFlags)i);
        if (pMod != nullptr)
            modVec.push_back(pMod->itemID());
        pMod = nullptr;
    }
}

void ModuleManager::GetShipSubSystems(std::vector< uint32 >& modVec)
{
    // get subsystems on ship, by itemID (there's only 5 slots...)
    GenericModule* pMod(nullptr);
    for (uint8 i = flagSubSystem0; i < flagSubSystem5; ++i) {
        pMod = GetModule((EVEItemFlags)i);
        if (pMod != nullptr)
            modVec.push_back(pMod->itemID());
        pMod = nullptr;
    }
}

void ModuleManager::SortModulesBySlotDec(std::vector<uint32>& modVec, std::vector< GenericModule* >& pModList)
{
    if (modVec.empty())
        return;
    GenericModule* pMod(nullptr);
    std::map<uint8, GenericModule*> tmpList;
    for (auto cur : modVec) {
        pMod = GetModule(cur);
        if (pMod != nullptr)
            tmpList.insert(std::pair<uint8, GenericModule*>((uint8)pMod->flag(), pMod));
        pMod = nullptr;
    }
    if (tmpList.empty())
        return;
    std::map<uint8, GenericModule*>::reverse_iterator itr = tmpList.rbegin();
    for (; itr != tmpList.rend(); ++itr)
        pModList.push_back(itr->second);
}

void ModuleManager::GetActiveModules(uint8 rack, std::vector< GenericModule* >& modVec)
{
    std::vector< GenericModule* > modVecAll;
    switch (rack) {
        case EVEEffectID::hiPower: {
            GetModulesInBank(flagHiSlot0, modVecAll);
        } break;
        case EVEEffectID::medPower: {
            GetModulesInBank(flagMidSlot0, modVecAll);
        } break;
        case EVEEffectID::loPower: {
            GetModulesInBank(flagLowSlot0, modVecAll);
        } break;
    }

    for (auto cur : modVecAll)
        if (cur->IsActive())
            if (!cur->IsOverloaded())
                modVec.push_back(cur);
}

void ModuleManager::GetActiveModulesHeat(uint8 rack, float& heat)
{
    std::vector< GenericModule* > modVecAll;
    switch (rack) {
        case EVEEffectID::hiPower: {
            GetModulesInBank(flagHiSlot0, modVecAll);
        } break;
        case EVEEffectID::medPower: {
            GetModulesInBank(flagMidSlot0, modVecAll);
        } break;
        case EVEEffectID::loPower: {
            GetModulesInBank(flagLowSlot0, modVecAll);
        } break;
    }

    for (auto cur : modVecAll)
        if (cur->IsActive()) {
            if (!cur->IsOverloaded())
                heat += cur->GetAttribute(AttrHeatDamage).get_float() /10;
        } else {
            //AttrHeatAbsorbtionRateModifier    -- if this module is inactive, it will absorb this much heat.
            heat -= cur->GetAttribute(AttrHeatAbsorbtionRateModifier).get_float() *10;
        }
}

uint8 ModuleManager::GetActiveModulesCount(uint8 rack)
{
    uint8 count = 0;
    std::vector< GenericModule* > modVec;
    switch (rack) {
        case EVEEffectID::hiPower: {
            GetModulesInBank(flagHiSlot0, modVec);
        } break;
        case EVEEffectID::medPower: {
            GetModulesInBank(flagMidSlot0, modVec);
        } break;
        case EVEEffectID::loPower: {
            GetModulesInBank(flagLowSlot0, modVec);
        } break;
    }

    for (auto cur : modVec)
        if (cur->IsActive())
            if (!cur->IsOverloaded())
                ++count;

    return count;
}

uint8 ModuleManager::GetFittedModuleCountByGroup(uint16 groupID)
{
    if (m_modByGroup.find(groupID) != m_modByGroup.end() )
        return m_modByGroup.find(groupID)->second;

    return 0;
}

void ModuleManager::addModuleRef(EVEItemFlags flag, GenericModule* pMod)
{
    // add module to main map
    m_modules.at(flag) = pMod;
    // add module to proc maps
    if (IsFittingSlot(flag)) {
        m_fittings.at(flag) = pMod;
    } else if (IsRigSlot(flag)) {
        m_systems.at(flag) = pMod;
    } else if (IsSubSystem(flag)) {
        m_systems.at(flag) = pMod;
    } else {  // error here.
        sLog.Error("MM::addModuleRef()", "%s(%u) sent flag '%s'", pMod->GetSelf()->name(), pMod->itemID(), sDataMgr.GetFlagName(flag));
        if (sConfig.debug.IsTestServer)
            EvE::traceStack();
        return;
    }

    _log(MODULE__TRACE, "MM::addModuleRef() - adding %s in %s to map.", pMod->GetSelf()->name(), sDataMgr.GetFlagName(flag));

    // Maintain the Modules Fitted By Group counter for this module group:
    if (m_modByGroup.find(pMod->groupID()) != m_modByGroup.end()) {
        m_modByGroup.find(pMod->groupID())->second += 1;
    } else {
        m_modByGroup.emplace(pMod->groupID(), 1);
    }

    // update available slots
    if (pMod->isHighPower()) {
        bool update = !pShipItem->GetPilot()->IsLogin();
        if (pMod->isTurretFitted()) {
            // apply config modifier, if applicable
            pMod->GetSelf()->MultiplyAttribute(AttrSpeed, sConfig.rates.turretRoF);
            uint8 count = pShipItem->GetAttribute(AttrTurretSlotsLeft).get_uint32() -1;
            pShipItem->SetAttribute(AttrTurretSlotsLeft, count, update);
        } else if (pMod->isLauncherFitted()) {
            // apply config modifier, if applicable
            pMod->GetSelf()->MultiplyAttribute(AttrSpeed, sConfig.rates.missileRoF);
            uint8 count = pShipItem->GetAttribute(AttrLauncherSlotsLeft).get_uint32() -1;
            pShipItem->SetAttribute(AttrLauncherSlotsLeft, count, update);
        }
        --m_HighSlots;
    } else if (pMod->isMediumPower()) {
        --m_MidSlots;
    } else if (pMod->isLowPower()) {
        --m_LowSlots;
    } else if (pMod->isSubSystem()) {
        --m_SubSystemSlots;
    } else if (pMod->isRig()) {
        --m_RigSlots;
        bool update = !pShipItem->GetPilot()->IsLogin();
        pShipItem->SetAttribute(AttrUpgradeLoad, (pShipItem->GetAttribute(AttrUpgradeLoad) + pMod->GetAttribute(AttrUpgradeCost)), update);
        pShipItem->SetAttribute(AttrUpgradeSlotsLeft, (pShipItem->GetAttribute(AttrUpgradeSlotsLeft) -1), update);
    }
}

void ModuleManager::deleteModuleRef(EVEItemFlags flag, GenericModule* pMod)
{
    // remove module from main map
    m_modules.at(flag) = nullptr;
    // remove module from proc maps
    if (IsFittingSlot(flag)) {
        m_fittings.at(flag) = nullptr;
    } else if (IsRigSlot(flag)) {
        m_systems.at(flag) = nullptr;
    } else if (IsSubSystem(flag)) {
        m_systems.at(flag) = nullptr;
    } else {  // error here.
        sLog.Error("MM::deleteModuleRef()", "%s(%u) sent flag '%s'", pMod->GetSelf()->name(), pMod->itemID(), sDataMgr.GetFlagName(flag));
        if (sConfig.debug.IsTestServer)
            EvE::traceStack();
        return;
    }

    _log(MODULE__TRACE, "MM::deleteModuleRef() - removing %s from %s.", pMod->GetSelf()->name(), sDataMgr.GetFlagName(flag));

    // Maintain the Modules Fitted By Group counter for this module group:
    if (m_modByGroup.find(pMod->groupID()) != m_modByGroup.end()) {
        m_modByGroup.find(pMod->groupID())->second -= 1;
        if (m_modByGroup.find(pMod->groupID())->second < 1) {
            // This was the last (or only) module of this group fitted, so remove the entry from the map:
            m_modByGroup.erase(pMod->groupID());
        }
    }

    // update available slots
    if (pMod->isHighPower()) {
        if (pMod->isTurretFitted()) {
            uint8 count = pShipItem->GetAttribute(AttrTurretSlotsLeft).get_uint32() + 1;
            pShipItem->SetAttribute(AttrTurretSlotsLeft, count);
        } else if (pMod->isLauncherFitted()) {
            uint8 count = pShipItem->GetAttribute(AttrLauncherSlotsLeft).get_uint32() + 1;
            pShipItem->SetAttribute(AttrLauncherSlotsLeft, count);
        }
        ++m_HighSlots;
    } else if (pMod->isMediumPower()) {
        ++m_MidSlots;
    } else if (pMod->isLowPower()) {
        ++m_LowSlots;
    } else if (pMod->isSubSystem()) {
        ++m_SubSystemSlots;
    } else if (pMod->isRig()) {
        ++m_RigSlots;
        pShipItem->SetAttribute(AttrUpgradeLoad, (pShipItem->GetAttribute(AttrUpgradeLoad) - pMod->GetAttribute(AttrUpgradeCost)));
        pShipItem->SetAttribute(AttrUpgradeSlotsLeft, pShipItem->GetAttribute(AttrUpgradeSlotsLeft) + 1);
    }
}

bool ModuleManager::VerifySlotExchange(EVEItemFlags slot1, EVEItemFlags slot2)
{
    if (!IsSlotOccupied(slot1) or !IsSlotOccupied(slot2))
        return true;
    return (GetModule(slot1)->GetModulePowerLevel() == GetModule(slot2)->GetModulePowerLevel());
}

/*
 * {'messageKey': 'EffectDeactivationCloaking', 'dataID': 17883455, 'suppressable': False, 'bodyID': 259510, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 849}
 * {'messageKey': 'EffectDeactivationWarping', 'dataID': 17883458, 'suppressable': False, 'bodyID': 259511, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 850}
 *
 * {'FullPath': u'UI/Messages', 'messageID': 259510, 'label': u'EffectDeactivationCloakingBody'}(u'As certain activated effects interfere with the warping process, these are automatically being deactivated before the warp proceeds.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259511, 'label': u'EffectDeactivationWarpingBody'}(u'As certain activated effects interfere with the warping process, these are automatically being deactivated before the warp proceeds.', None, None)
 */

void ModuleManager::UpdateChargeQty() {
    for (auto cur : m_charges) {
        cur.second->AlterQuantity(+1, false);
        cur.second->AlterQuantity(-1, false);
    }
}
