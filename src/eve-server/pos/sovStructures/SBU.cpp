
/**
 * @name SBU.cpp
 *   Class for Sovereignty Blockade Units.
 *
 * @Author:         James
 * @date:   8 April 2021
 */

/*
 * POS__ERROR
 * POS__WARNING
 * POS__MESSAGE
 * POS__DUMP
 * POS__DEBUG
 * POS__DESTINY
 * POS__SLIMITEM
 * POS__TRACE
 */

#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "planet/Planet.h"
#include "pos/sovStructures/SBU.h"
#include "pos/sovStructures/TCU.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/sov/SovereigntyDataMgr.h"

SBUSE::SBUSE(StructureItemRef structure, EVEServiceManager &services, SystemManager *system, const FactionData &fData)
    : StructureSE(structure, services, system, fData)
{
}

void SBUSE::Init()
{
    _log(SE__TRACE, "SBUSE %s(%u) is being initialised", m_self->name(), m_self->itemID());
    StructureSE::Init();

    // A missing grid or failed base POS initialization must not crash
    // the whole solar system during boot.
    if (!IsPOSInitialized() || m_bubble == nullptr) {
        _log(SOV__ERROR, "SBUSE::Init: missing bubble or POS state for %u.", m_self->itemID());
        return;
    }
    m_bubble->SetSBUSE(this);

    // set global attribute
    m_self->SetAttribute(AttrIsGlobal, EvilOne, false);
}

void SBUSE::SetOnline()
{
    _log(SOV__DEBUG, "Onlining SBU...");

    // Prevent contesting an unclaimed/empire system. The static owner and
    // gate count must also exist before this can affect sovereignty.
    const SovereigntyData sov = svDataMgr.GetSovereigntyData(m_system->GetID());
    if (!sov.claimStructureID || GetGates() <= 0.0f) {
        _log(SOV__ERROR, "SBU %u cannot online: system %u is unclaimed or has no gates.",
             m_self->itemID(), m_system->GetID());
        return;
    }

    // Ensure the SBU is online before we check the ratio
    StructureSE::SetOnline();

    // If more than 50 percent of gates have an SBU, the TCU should be vulnerable
    // Count gates once; gate-less systems must not divide by zero.
    const float gates = GetGates();
    const float sbus = GetSBUs();
    if (gates > 0.0f && sbus * 2.0f > gates)
    {
        _log(SOV__DEBUG, "- STATE CHANGE - This system has %u SBUs installed in the total of %u stargates. The TCU is now vulnerable to attack.", (int)GetSBUs(), (int)GetGates());
        // Make the TCU vulnerable
        for (auto cur : m_system->GetOperationalStatics())
        {
            if (cur.second != nullptr && cur.second->IsTCUSE())
            {
                cur.second->GetTCUSE()->SetVulnerable();
            }
        }
        // Mark the system as contested
        MarkContested(m_system->GetID(), true);
    }
    else
    {
        _log(SOV__DEBUG, "This system has %u SBUs installed in the total of %u stargates. The TCU is invulnerable.", (int)GetSBUs(), (int)GetGates());
    }
}

void SBUSE::SetOffline()
{
    // Ensure that the SBU is offline before we check ratio
    StructureSE::SetOffline();

    const float gates = GetGates();
    const float sbus = GetSBUs();
    if (gates > 0.0f && sbus * 2.0f <= gates)
    {
        _log(SOV__DEBUG, "- STATE CHANGE - This system has %u SBUs installed in the total of %u stargates. The TCU is no longer vulnerable to attack.", (int)GetSBUs(), (int)GetGates());
        // Make the TCU invulnerable
        for (auto cur : m_system->GetOperationalStatics())
        {
            if (cur.second != nullptr && cur.second->IsTCUSE())
            {
                // Restoring invulnerability must not call TCUSE::SetOnline:
                // that would create a new sovereignty claim and reset claimTime.
                cur.second->GetTCUSE()->SetInvulnerable();
            }
        }
        // Unmark the system as contested
        MarkContested(m_system->GetID(), false);
    }
    else
    {
        _log(SOV__DEBUG, "This system has %u SBUs installed in the total of %u stargates. The TCU is vulnerable.", (int)GetSBUs(), (int)GetGates());
    }
}

void SBUSE::Process()
{
    StructureSE::Process();
}

void SBUSE::MarkContested(uint32 systemID, bool contested)
{
    //Send ProcessSovStatusChanged Notification
    PyDict *args = new PyDict;
    _log(SOV__DEBUG, "SBUSE::MarkContested(): Sending ProcessSovStatusChanged for %u", systemID);

    //Update datamgr records with new contested state
    svDataMgr.MarkContested(systemID, contested);

    //Get the new sov data so we can send it to the client in a notification
    SovereigntyData sovData = svDataMgr.GetSovereigntyData(systemID);

    args->SetItemString("corporationID", new PyInt(sovData.corporationID));
    args->SetItemString("claimTime", new PyLong(sovData.claimTime));
    args->SetItemString("claimStructureID", new PyInt(sovData.claimStructureID));
    args->SetItemString("hubID", new PyInt(sovData.hubID));
    args->SetItemString("allianceID", new PyInt(sovData.allianceID));
    args->SetItemString("contested", new PyInt(int(contested)));
    args->SetItemString("solarSystemID", new PyInt(systemID));

    PyTuple *data = new PyTuple(2);
    data->SetItem(0, new PyInt(systemID));
    data->SetItem(1, new PyObject("util.KeyVal", args));

    std::vector<Client *> list;
    sEntityList.GetClients(list);
    for (auto cur : list)
    {
        if (cur != nullptr && cur->GetChar().get() != nullptr)
        {
            PyIncRef(data);
            cur->SendNotification("ProcessSovStatusChanged", "clientID", &data);
            _log(SOV__DEBUG, "ProcessSovStatusChanged sent to %s(%u)", cur->GetName(), cur->GetCharID());
        }
    }

    // cleanup
    PyDecRef(data);
}

// Calculate number of gates in the system
float SBUSE::GetGates()
{
    return (float)m_system->GetGates().size();
}

// Calculate number of SBUs in the system
float SBUSE::GetSBUs()
{
    int numberOfSBUs = 0;
    for (auto cur : m_system->GetOperationalStatics())
    {
        if (cur.second != nullptr && cur.second->IsSBUSE())
        {
            //Only increment the number of SBUs if they are online
            if (cur.second->GetSBUSE()->GetState() == EVEPOS::StructureState::Online)
            {
                numberOfSBUs++;
            }
        }
    }
    return (float)numberOfSBUs;
}