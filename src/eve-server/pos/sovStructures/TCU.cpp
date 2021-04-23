
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

    //Send OnSovereigntyChanged Notification
    _log(SOV__DEBUG, "Sending OnSovereigntyChanged for %u:%u", sovData.solarSystemID, sovData.allianceID);

    OnSovereigntyChanged osc;
    osc.solarSystemID = sovData.solarSystemID;
    osc.allianceID = sovData.allianceID;

    PyTuple *ev = osc.Encode();
    std::vector<Client *> list;
    sEntityList.GetClients(list);
    for (auto cur : list)
    {
        if (cur->GetChar().get() != nullptr)
        {
            cur->SendNotification("OnSovereigntyChanged", "clientID", &ev);
            _log(SOV__DEBUG, "OnSovereigntyChanged sent to client %u", cur->GetClientID());
        }
    }
}

void TCUSE::SetOffline()
{
    _log(SOV__DEBUG, "Offlining TCU... Removing claim.");
    svDataMgr.RemoveSovClaim(m_system->GetID());
    StructureSE::SetOffline();
}

void TCUSE::Process()
{
    StructureSE::Process();
}
