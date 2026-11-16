
/**
 * @name IHub.cpp
 *   Class for Infrastructure Hubs.
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
#include "pos/sovStructures/IHub.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/sov/SovereigntyDataMgr.h"

IHubSE::IHubSE(StructureItemRef structure, EVEServiceManager& services, SystemManager* system, const FactionData& fData)
: StructureSE(structure, services, system, fData)
{
}


void IHubSE::Init()
{
    _log(SE__TRACE, "IHubSE %s(%u) is being initialised", m_self->name(), m_self->itemID());
    StructureSE::Init();

    if (!IsPOSInitialized() || m_bubble == nullptr) {
        _log(SOV__ERROR, "IHubSE::Init: missing bubble or POS state for %u.", m_self->itemID());
        return;
    }
    m_bubble->SetIHubSE(this);

    // set global attribute
    m_self->SetAttribute(AttrIsGlobal, EvilOne, false);
}

void IHubSE::Process()
{
    StructureSE::Process();
}

void IHubSE::SetOnline()
{
    _log(SOV__DEBUG, "Onlining IHub... Updating claim'd hubID.");
    const SovereigntyData sov = svDataMgr.GetSovereigntyData(m_system->GetID());
    if (!sov.claimStructureID || !m_allyID || sov.allianceID != m_allyID) {
        _log(SOV__ERROR, "IHub %u cannot online: alliance %u does not hold sovereignty in %u.",
             m_self->itemID(), m_allyID, m_system->GetID());
        return;
    }
    StructureSE::SetOnline();
    svDataMgr.UpdateSystemHubID(m_system->GetID(), m_self->itemID());
}

void IHubSE::SetOffline()
{
    _log(SOV__DEBUG, "Offlining IHub... Resetting claim's hubID.");
    const SovereigntyData sov = svDataMgr.GetSovereigntyData(m_system->GetID());
    if (sov.hubID == m_self->itemID())
        svDataMgr.UpdateSystemHubID(m_system->GetID(), 0);
    StructureSE::SetOffline();
}
