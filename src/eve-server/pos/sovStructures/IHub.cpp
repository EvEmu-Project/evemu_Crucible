
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

IHubSE::IHubSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& fData)
: StructureSE(structure, services, system, fData),
m_pShieldSE(nullptr)
{
    m_structs.clear();
}

IHubSE::~IHubSE()
{
    SafeDelete(m_destiny);
}

void IHubSE::Init()
{
    StructureSE::Init();

    if (!m_db.GetBaseData(m_data)) {
        _log(SE__TRACE, "IHubSE %s(%u) has no saved data.  Initializing default set.", m_self->name(), m_self->itemID());
        // invalid data....init to 0 as this will only hit for currently-launching items (or errors)
        InitData();
    }

    // this OSE needs destiny.
    m_destiny = new DestinyManager(this);

    // set IHub in bubble
    if (m_bubble == nullptr)
        assert(0);
    m_bubble->SetIHubSE(this);

}

void IHubSE::InitData() {
    // init base data first
    StructureSE::InitData();

}

void IHubSE::Scoop() {
    StructureSE::Scoop();
    m_self->ChangeSingleton(false);
    m_self->SaveItem();
}

void IHubSE::Process()
{
    StructureSE::Process();
}

/*
 * 556     anchoringDelay  1800000     NULL
 * 650     maxStructureDistance    50000   NULL
 * 676     unanchoringDelay    NULL    3600000
 * 677     onliningDelay   1800000     NULL
 * 680     shieldRadius    30000   NULL
 * 711     moonAnchorDistance  100000  NULL
 * 1214    posStructureControlDistanceMax  NULL    15000
 */

void IHubSE::SetOnline()
{
    //StructureSE::SetOnline();

    m_data.timestamp = GetFileTimeNow();
    m_self->SetFlag(flagStructureActive);
    m_data.state = EVEPOS::StructureStatus::Online;

    SendSlimUpdate();
    m_destiny->SendSpecialEffect(m_self->itemID(),m_self->itemID(),m_self->typeID(),0,0,"effects.StructureOnline",0,1,1,-1,0);

    m_db.UpdateBaseData(m_data);

}

void IHubSE::SetOffline()
{
    StructureSE::SetOffline();
}

void IHubSE::Online()
{
    StructureSE::Online();
}

void IHubSE::Operating()
{
    StructureSE::SetOperating();
}

void IHubSE::Reinforced()
{
    // something to do with IHubs should go here maybe?
}

PyDict* IHubSE::MakeSlimItem()
{
    _log(SE__SLIMITEM, "MakeSlimItem for IHubSE %u", m_self->itemID());
    _log(POS__SLIMITEM, "MakeSlimItem for IHubSE %u", m_self->itemID());

    PyDict *slim = new PyDict();
    slim->SetItemString("name",                     new PyString(m_self->itemName()));
    slim->SetItemString("itemID",                   new PyLong(m_self->itemID()));
    slim->SetItemString("typeID",                   new PyInt(m_self->typeID()));
    slim->SetItemString("ownerID",                  new PyInt(m_ownerID));
    slim->SetItemString("corpID",                   IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
    slim->SetItemString("allianceID",               IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
    slim->SetItemString("warFactionID",             IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
    slim->SetItemString("posTimestamp",             new PyLong(m_data.timestamp));
    slim->SetItemString("posState",                 new PyInt(m_data.state));
    slim->SetItemString("incapacitated",            new PyInt((m_data.state == EVEPOS::StructureStatus::Incapacitated) ? 1 : 0));
    slim->SetItemString("posDelayTime",             new PyInt(m_delayTime));

    if (is_log_enabled(POS__SLIMITEM)) {
        _log( POS__SLIMITEM, "IHubSE::MakeSlimItem() - %s(%u)", GetName(), GetID());
        slim->Dump(POS__SLIMITEM, "     ");
    }
    return slim;
}
