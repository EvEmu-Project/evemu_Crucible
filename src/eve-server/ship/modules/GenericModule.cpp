
 /**
  * @name GenericModule.cpp
  *   base module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */

/*
MODULE__ERROR
MODULE__WARNING
MODULE__MESSAGE
MODULE__INFO
MODULE__DEBUG
MODULE__TRACE
*/
#include "Client.h"
#include "ship/modules/GenericModule.h"
#include "ship/modules/ActiveModule.h"


GenericModule::GenericModule(ModuleItemRef mRef, ShipItemRef sRef)
: m_repeat(0),
m_modRef(mRef),
m_shipRef(sRef),
m_chargeRef(InventoryItemRef(nullptr)),
m_ModuleState(Module::State::Unfitted),
m_ChargeState(Module::State::Unloaded),
m_linked(false),
m_linkMaster(false),
m_isWarpSafe(false),
m_overLoaded(false),
m_chargeLoaded(false),
m_hiPower(false),
m_medPower(false),
m_loPower(false),
m_rigSlot(false),
m_subSystem(false),
m_turret(false),
m_launcher(false)
{
    if (mRef->type().HasEffect(EVEEffectID::loPower)) {
        m_loPower = true;
    } else if (mRef->type().HasEffect(EVEEffectID::medPower)) {
        m_medPower = true;
    } else if (mRef->type().HasEffect(EVEEffectID::hiPower)) {
        m_hiPower = true;
        if (mRef->type().HasEffect(EVEEffectID::turretFitted)) {
            m_turret = true;
        } else if (mRef->type().HasEffect(EVEEffectID::launcherFitted)) {
            m_launcher = true;
        }
    } else if (mRef->type().HasEffect(EVEEffectID::rigSlot)) {
        m_rigSlot = true;
    } else if (mRef->type().HasEffect(EVEEffectID::subSystem)) {
        m_subSystem = true;
    }

    _log(MODULE__DEBUG, "Created GenericModule %p for item %s (%u).", this, mRef->name(), mRef->itemID());
}

GenericModule::~GenericModule()
{

}

// this function must NOT throw
// throwing an error negates further processing
void GenericModule::Online()
{
    if (m_shipRef->GetPilot()->IsDocked() and (!m_shipRef->IsUndocking())) {
        m_ModuleState = Module::State::Online;
        SetAttribute(AttrOnline, EvilOne, !m_shipRef->GetPilot()->IsLogin());
        return;
    }

    if (m_ModuleState == Module::State::Unfitted) {
        _log(MODULE__ERROR, "GenericModule::Online() called for unfitted module %u(%s).",itemID(), m_modRef->name());
        return;
    }
    if (m_ModuleState != Module::State::Offline) {
        _log(MODULE__MESSAGE, "GenericModule::Online() called for non-offline module %u(%s).  State is %s", \
                itemID(), m_modRef->name(), GetModuleStateName(m_ModuleState));
        return;     // already online
    }

    if (GetAttribute(AttrDamage) >= GetAttribute(AttrHP)) {
        m_shipRef->GetPilot()->SendNotifyMsg("Your %s is too damaged to be put online.", m_modRef->name());
        return;
        /*{'messageKey': 'ModuleTooDamagedToBeOnlined', 'dataID': 17878773, 'suppressable': False, 'bodyID': 257752, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2303}
         *   u'ModuleTooDamagedToBeOnlinedBody'}(u'The module is too damaged to be onlined'
         */
    }
    // check PG and CPU usage to see if we have enough to online this module
    EvilNumber cpuNeed(m_shipRef->GetAttribute(AttrCpuLoad) + GetAttribute(AttrCpu));
    if (cpuNeed  > m_shipRef->GetAttribute(AttrCpuOutput)) {
        _log(MODULE__TRACE, "GenericModule::Online() %u(%s) - not enough CPU. (%.1f/%.1f)", \
                itemID(), m_modRef->name(), cpuNeed.get_float(), m_shipRef->GetAttribute(AttrCpuOutput).get_float());
        if (!m_shipRef->GetPilot()->IsLogin()) {
            m_modRef->SetOnline(false, isRig());
            float require(GetAttribute(AttrCpu).get_float());
            float total(m_shipRef->GetAttribute(AttrCpuOutput).get_float());
            float remaining(total - m_shipRef->GetAttribute(AttrCpuLoad).get_float());
            std::string str = "To bring " + m_modRef->itemName() + " online requires %.2f cpu units, ";
            str += "but only %.2f of the %.2f units that your computer produces are still available.";
            m_shipRef->GetPilot()->SendNotifyMsg(str.c_str(), require, remaining, total);
        }
        return;
    }
    EvilNumber pgNeed(m_shipRef->GetAttribute(AttrPowerLoad) + GetAttribute(AttrPower));
    if (pgNeed > m_shipRef->GetAttribute(AttrPowerOutput)) {
        _log(MODULE__TRACE, "GenericModule::Online() %u(%s) - not enough PG. (%.1f/%.1f)", \
                itemID(), m_modRef->name(), pgNeed.get_float(), m_shipRef->GetAttribute(AttrPowerOutput).get_float());
        if (!m_shipRef->GetPilot()->IsLogin()) {
            m_modRef->SetOnline(false, isRig());
            float require(GetAttribute(AttrPower).get_float());
            float total(m_shipRef->GetAttribute(AttrPowerOutput).get_float());
            float remaining(total - m_shipRef->GetAttribute(AttrPowerLoad).get_float());
            std::string str = "To bring " + m_modRef->itemName() + " online requires %.2f power units, ";
            str += "but only %.2f of the %.2f units that your power core produces are still available.";
            m_shipRef->GetPilot()->SendNotifyMsg(str.c_str(), require, remaining, total);
        }
        return;
    }

    m_modRef->SetOnline(true, isRig());
    m_ModuleState = Module::State::Online;
    _log(MODULE__MESSAGE, "GenericModule::Online() - %u(%s) cpu: %.2f, pg: %.2f, loaded: %s", \
            itemID(), m_modRef->name(), cpuNeed.get_float(), pgNeed.get_float(), m_ChargeState == Module::State::Loaded?"true":"false");

    ProcessEffects(FX::State::Passive, true);
    ProcessEffects(FX::State::Online, true);
    if (m_ChargeState == Module::State::Loaded) {
        if (!m_chargeLoaded) {
            _log(MODULE__ERROR, "GenericModule::Online() - module %u(%s) has ChargeState(CHG_LOADED) but m_chargeLoaded = false.", \
                    itemID(), m_modRef->name());
        } else if (m_chargeRef.get() == nullptr) {
            _log(MODULE__ERROR, "GenericModule::Online() - module %u(%s) has ChargeState(CHG_LOADED) but m_chargeRef = NULL.", \
                    itemID(), m_modRef->name());
        } else {
            _log(MODULE__MESSAGE, "GenericModule::Online() - module %u(%s) loading charge fx for %s.", itemID(), m_modRef->name(), m_chargeRef->name());
            for (auto it : m_chargeRef->type().m_stateFxMap) {
                fxData data = fxData();
                data.action = FX::Action::Invalid;
                data.srcRef = m_chargeRef;
                sFxProc.ParseExpression(m_modRef.get(), sFxDataMgr.GetExpression(it.second.preExpression), data, this);
            }
        }
    }

    // update available ship resources.
    m_shipRef->SetAttribute(AttrCpuLoad, cpuNeed, !m_shipRef->IsUndocking());
    m_shipRef->SetAttribute(AttrPowerLoad, pgNeed, !m_shipRef->IsUndocking());
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), !m_shipRef->IsUndocking());
}

void GenericModule::Offline()
{
    if (m_shipRef->GetPilot()->IsDocked()) {
        m_ModuleState = Module::State::Offline;
        SetAttribute(AttrOnline, EvilZero, !m_shipRef->IsUndocking());
        return;
    }

    //if (m_shipRef->GetPilot()->GetShipSE()->IsDead()) //  SE->IsDead() for all SEs
    if (m_shipRef->IsPopped())                          // only for player ships
        return;

    switch(m_ModuleState) {
        case Module::State::Unfitted: {
            m_modRef->SetOnline(false, isRig());
            _log(MODULE__WARNING, "GenericModule::Offline() called for unfitted module %u(%s).",itemID(), m_modRef->name());
            return;
        } break;
        case Module::State::Offline: {
            m_modRef->SetOnline(false, isRig());
            _log(MODULE__WARNING, "GenericModule::Offline() called for offline module %u(%s).",itemID(), m_modRef->name());
            return;
        } break;
        // these two should only be called for activeModules...
        case Module::State::Deactivating: {
            _log(MODULE__MESSAGE, "GenericModule::Offline() called for deactivating module %u(%s).",itemID(), m_modRef->name());
            if (IsActiveModule())
                GetActiveModule()->AbortCycle();
        } break;
        case Module::State::Activated: {
            _log(MODULE__MESSAGE, "GenericModule::Offline() called for active module %u(%s).",itemID(), m_modRef->name());
            if (IsActiveModule())
                GetActiveModule()->AbortCycle();
        } break;
    }

    m_ModuleState = Module::State::Deactivating;

    /* code for offlining module before MOD_OFFLINE state is set. */
    EvilNumber cpuNeed(m_shipRef->GetAttribute(AttrCpuLoad) - GetAttribute(AttrCpu));
    EvilNumber pgNeed(m_shipRef->GetAttribute(AttrPowerLoad) - GetAttribute(AttrPower));
    m_shipRef->SetAttribute(AttrCpuLoad, cpuNeed);
    m_shipRef->SetAttribute(AttrPowerLoad, pgNeed);

    _log(MODULE__MESSAGE, "GenericModule::Offline() - %u(%s) cpu: %.2f, pg: %.2f",itemID(), m_modRef->name(), cpuNeed.get_float(), pgNeed.get_float());

    if (m_ChargeState == Module::State::Loaded) {
        if (m_chargeRef.get() == nullptr) {
            _log(MODULE__ERROR, "GenericModule::Offline() - module %u(%s) has ChargeState(CHG_LOADED) but m_chargeRef = NULL.", \
                    itemID(), m_modRef->name());
        } else {
            for (auto it : m_chargeRef->type().m_stateFxMap) {
                fxData data = fxData();
                data.action = FX::Action::Invalid;
                data.srcRef = m_chargeRef;
                sFxProc.ParseExpression(m_modRef.get(), sFxDataMgr.GetExpression(it.second.postExpression), data, this);
            }
        }
    }

    ProcessEffects(FX::State::Passive, false);
    ProcessEffects(FX::State::Online, false);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);

    m_ModuleState = Module::State::Offline;
    m_modRef->SetOnline(false, isRig());
}

void GenericModule::Overload()
{
    ProcessEffects(FX::State::Overloaded, true);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
    //m_modRef->ClearModifiers();
}

void GenericModule::DeOverload()
{
    ProcessEffects(FX::State::Overloaded, false);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
    //m_modRef->ClearModifiers();
}

void GenericModule::ProcessEffects(int8 state, bool active/*false*/)
{
    // get module/charge pre/post effects in state x
    std::map<uint16, Effect> effectMap;
    m_modRef->type().GetEffectMap(state, effectMap);
    _log(EFFECTS__TRACE, "GenericModule::ProcessEffects() called for %s. effects: %u, state: %s, online: %s", \
            m_modRef->name(), effectMap.size(), sFxProc.GetStateName(state), (active ? "true" : "false"));
    for (auto it : effectMap) {
        if (it.first == 16)    // skip the online effect.  this is done internally elsewhere
            continue;
        fxData data = fxData();
        data.action = FX::Action::Invalid;
        data.srcRef = m_modRef;
        /* module and charge effects will be added/removed from it's item
         * active/overload/gang/other effects will be applied and removed when called.
         */
        if (active) {
            sFxProc.ParseExpression(m_modRef.get(), sFxDataMgr.GetExpression(it.second.preExpression), data, this);
        } else {
            sFxProc.ParseExpression(m_modRef.get(), sFxDataMgr.GetExpression(it.second.postExpression), data, this);
        }
    }
}

// not used
void GenericModule::Repair(EvilNumber amount)
{
    if (GetAttribute(AttrDamage) > 0) {
        EvilNumber newAmount = GetAttribute(AttrDamage) - amount;
        if (newAmount < EvilZero)
            newAmount = EvilZero;
        SetAttribute(AttrDamage, newAmount);
    }
    _log(MODULE__DAMAGE, "GenericModule::Repair() - %s repaired %u damage.  new damage %u", m_modRef->name(), amount, GetAttribute(AttrDamage).get_int());
}

const char* GenericModule::GetModuleStateName(int8 state)
{
    using namespace Module;
    switch(state) {
        case State::Unloaded:       return "Unloaded";
        case State::Loaded:         return "Loaded";
        case State::Loading:        return "Loading";
        case State::Reloading:      return "Reloading";
        case State::Unfitted:       return "Unfitted";
        case State::Offline:        return "Offline";
        case State::Online:         return "Online";
        case State::Activated:      return "Activated";
        case State::Deactivating:   return "Deactivating";
        default:                    return "Invalid";
    }
}

int8 GenericModule::GetModulePowerLevel() {
    {
        if (m_hiPower) {
            return Module::Bank::High;
        } else if (m_medPower) {
            return Module::Bank::Mid;
        } else if (m_loPower) {
            return Module::Bank::Low;
        } else if (m_rigSlot) {
            return Module::Bank::Rig;
        } else if (m_subSystem) {
            return Module::Bank::Subsystem;
        } else {
            return Module::Bank::Undefined;
        }
    }
}
