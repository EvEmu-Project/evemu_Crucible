
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

SBUSE::SBUSE(StructureItemRef structure, PyServiceMgr &services, SystemManager *system, const FactionData &fData)
    : StructureSE(structure, services, system, fData)
{
}

void SBUSE::Init()
{
    _log(SE__TRACE, "SBUSE %s(%u) is being initialised", m_self->name(), m_self->itemID());
    StructureSE::Init();

    // check for valid bubble
    if (m_bubble == nullptr)
        assert(0);
    m_bubble->SetSBUSE(this);

    // set global attribute
    m_self->SetAttribute(AttrIsGlobal, EvilOne, false);
}

void SBUSE::SetOnline()
{
    _log(SOV__DEBUG, "Onlining SBU...");

    // Ensure the SBU is online before we check the ratio
    StructureSE::SetOnline();

    // If more than 50 percent of gates have an SBU, the TCU should be vulnerable
    if ((GetSBUs() / GetGates()) > 0.5)
    {
        _log(SOV__DEBUG, "- STATE CHANGE - This system has %u SBUs installed in the total of %u stargates. The TCU is now vulnerable to attack.", (int)GetSBUs(), (int)GetGates());
        // Make the TCU vulnerable
        for (auto cur : m_system->GetStaticEntities())
        {
            if (cur.second->IsTCUSE())
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

    if ((GetSBUs() / GetGates()) <= 0.5)
    {
        _log(SOV__DEBUG, "- STATE CHANGE - This system has %u SBUs installed in the total of %u stargates. The TCU is no longer vulnerable to attack.", (int)GetSBUs(), (int)GetGates());
        // Make the TCU invulnerable
        for (auto cur : m_system->GetStaticEntities())
        {
            if (cur.second->IsTCUSE())
            {
                cur.second->GetTCUSE()->SetOnline();
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
        if (cur->GetChar().get() != nullptr)
        {
            cur->SendNotification("ProcessSovStatusChanged", "clientID", &data);
            _log(SOV__DEBUG, "ProcessSovStatusChanged sent to client %u", cur->GetClientID());
        }
    }
}

// Calculate number of gates in the system
float SBUSE::GetGates()
{
    int numberOfGates = 0;
    for (auto cur : m_system->GetStaticEntities())
    {
        if (cur.second->IsGateSE())
        {
            numberOfGates++;
        }
    }
    return (float)numberOfGates;
}

// Calculate number of SBUs in the system
float SBUSE::GetSBUs()
{
    int numberOfSBUs = 0;
    for (auto cur : m_system->GetStaticEntities())
    {
        if (cur.second->IsSBUSE())
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