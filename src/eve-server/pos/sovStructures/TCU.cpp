
/**
 * @name TCU.cpp
 *   Class for Territorial Claim Units.
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
#include "pos/sovStructures/TCU.h"
#include "packets/Sovereignty.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/sov/SovereigntyDataMgr.h"

TCUSE::TCUSE(StructureItemRef structure, PyServiceMgr &services, SystemManager *system, const FactionData &fData)
    : StructureSE(structure, services, system, fData)
{
}

void TCUSE::Init()
{
    _log(SE__TRACE, "TCUSE %s(%u) is being initialised", m_self->name(), m_self->itemID());
    StructureSE::Init();

    // check for valid bubble
    if (m_bubble == nullptr)
        assert(0);
    m_bubble->SetTCUSE(this);

    // set global attribute
    m_self->SetAttribute(AttrIsGlobal, EvilOne, false);
}

void TCUSE::SetOnline()
{
    _log(SOV__DEBUG, "Onlining TCU... Creating a new claim.");

    StructureSE::SetOnline();

    SovereigntyData sovData;

    sovData.solarSystemID = m_system->GetID();
    sovData.regionID = m_system->GetRegionID();
    sovData.constellationID = m_system->GetConstellationID();
    sovData.corporationID = m_corpID;
    sovData.allianceID = m_allyID;
    sovData.claimStructureID = m_data.itemID;
    sovData.claimTime = GetFileTimeNow();
    sovData.hubID = 0;
    sovData.contested = 0;
    sovData.stationCount = 0;
    sovData.militaryPoints = 0;
    sovData.industrialPoints = 0;

    svDataMgr.AddSovClaim(sovData);

    //Send ProcessSovStatusChanged Notification
    PyDict *args = new PyDict;
    _log(SOV__DEBUG, "Sending ProcessSovStatusChanged for %u:%u", sovData.solarSystemID, sovData.allianceID);

    args->SetItemString("contested", new PyInt(sovData.contested));
    args->SetItemString("corporationID", new PyInt(sovData.corporationID));
    args->SetItemString("claimTime", new PyLong(sovData.claimTime));
    args->SetItemString("claimStructureID", new PyInt(sovData.claimStructureID));
    args->SetItemString("hubID", new PyInt(sovData.hubID));
    args->SetItemString("allianceID", new PyInt(sovData.allianceID));
    args->SetItemString("solarSystemID", new PyInt(sovData.solarSystemID));

    PyTuple* data = new PyTuple(2);
        data->SetItem(0, new PyInt(sovData.solarSystemID));
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

void TCUSE::SetOffline()
{
    _log(SOV__DEBUG, "Offlining TCU... Removing claim.");
    svDataMgr.RemoveSovClaim(m_system->GetID());

    //Send ProcessSovStatusChanged Notification
    _log(SOV__DEBUG, "Sending ProcessSovStatusChanged (removing sov claim) %u", m_system->GetID());

    PyTuple* data = new PyTuple(2);
        data->SetItem(0, new PyInt(m_system->GetID()));
        data->SetItem(1, PyStatic.NewNone());

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

    StructureSE::SetOffline();
}

void TCUSE::Process()
{
    StructureSE::Process();
}
