
/**
 * @name JumpBridge.cpp
 *   Class for POS JumpBridge Modules.
 *
 * @Author:           James
 * @date:   13 October 2021
 */

#include "pos/JumpBridge.h"
#include "system/sov/SovereigntyDataMgr.h"
#include "system/SystemManager.h"
#include "planet/Moon.h"

JumpBridgeSE::JumpBridgeSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data)
: StructureSE(structure, services, system, data)
{
    m_bridgeData = EVEPOS::JumpBridgeData();
}

void JumpBridgeSE::Init()
{
    StructureSE::Init();
    m_bridgeData.itemID = m_self->itemID();
    if (!m_db.GetBridgeData(m_bridgeData))
    {
        _log(POS__MESSAGE, "Jump Bridge %s(%u) has no saved bridge data.  Initializing default set.", m_self->name(), m_data.itemID);
        InitData();
        m_db.SaveBridgeData(m_bridgeData);
    }
}

void JumpBridgeSE::InitData()
{
    StructureSE::InitData();
    m_moonSE = m_system->GetClosestMoonSE(GetPosition())->GetMoonSE();
    m_data.anchorpointID = m_moonSE->GetID();

    m_bridgeData.itemID = m_data.itemID;
    m_bridgeData.towerID = m_data.towerID;
    m_bridgeData.corpID = m_corpID;
    m_bridgeData.allyID = m_allyID;
    m_bridgeData.systemID = m_system->GetID();
    m_bridgeData.allowCorp = false;
    m_bridgeData.allowAlliance = false;
}

void JumpBridgeSE::Process()
{
    StructureSE::Process();
}

void JumpBridgeSE::SetOnline()
{
    _log(SOV__DEBUG, "Onlining Jump Bridge...");

    // Checks to see if bridge can be brought online

    // Make sure the system is not being jammed
    SovereigntyData sovData = svDataMgr.GetSovereigntyData(m_self->locationID());
    if (sovData.jammerID != 0) {
        throw CustomError("This system is currently being jammed.");
    }

    // Make sure player is not in high-sec (configurable)
    if (!sConfig.world.highSecCyno) {
        if (m_system->GetSecValue() >= 0.5f) {
            throw CustomError("This module may not be used in high security space.");
        }
    }

    // Reload data from DB changes
    m_db.GetBridgeData(m_bridgeData);

    // Send a slim update which will contain the latest changes
    SendSlimUpdate();

    StructureSE::SetOnline();
}

void JumpBridgeSE::SetOffline()
{
    _log(SOV__DEBUG, "Offlining TCU... Resetting claim's hubID.");
    StructureSE::SetOffline();
}

PyDict* JumpBridgeSE::MakeSlimItem()
{
    _log(SE__SLIMITEM, "MakeSlimItem for JumpBridgeSE %u", m_self->itemID());
    _log(POS__SLIMITEM, "MakeSlimItem for JumpBridgeSE %u", m_self->itemID());

    PyDict *slim = new PyDict();
    // General module slim entries
    slim->SetItemString("name", new PyString(m_self->itemName()));
    slim->SetItemString("itemID", new PyLong(m_data.itemID));
    slim->SetItemString("typeID", new PyInt(m_self->typeID()));
    slim->SetItemString("posState", new PyInt(m_data.state));
    slim->SetItemString("ownerID", new PyInt(m_ownerID));
    slim->SetItemString("corpID", IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
    slim->SetItemString("allianceID", IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
    slim->SetItemString("warFactionID", IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
    slim->SetItemString("posTimestamp", new PyLong(m_data.timestamp));
    slim->SetItemString("incapacitated", new PyInt(m_data.state == EVEPOS::StructureState::Incapacitated));
    slim->SetItemString("posDelayTime", new PyInt(m_delayTime));
    slim->SetItemString("controlTowerID", new PyLong(m_data.towerID));

    // Jump Bridge specific entries
    slim->SetItemString("remoteStructureID", new PyInt(m_bridgeData.toItemID));
    slim->SetItemString("remoteSystemID", new PyInt(m_bridgeData.toSystemID));

    if (is_log_enabled(POS__SLIMITEM))
    {
        _log(POS__SLIMITEM, "JumpBridgeSE::MakeSlimItem() - %s(%u)", GetName(), m_data.itemID);
        slim->Dump(POS__SLIMITEM, "     ");
    }
    return slim;
}

void JumpBridgeSE::SendSlimUpdate()
{
    PyDict *slim = new PyDict();
        slim->SetItemString("name", new PyString(m_self->itemName()));
        slim->SetItemString("itemID", new PyLong(m_data.itemID));
        slim->SetItemString("typeID", new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID", new PyInt(m_ownerID));
        slim->SetItemString("corpID", IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
        slim->SetItemString("allianceID", IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
        slim->SetItemString("warFactionID", IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
        slim->SetItemString("posTimestamp", new PyLong(m_data.timestamp));
        slim->SetItemString("posState", new PyInt(m_data.state));
        slim->SetItemString("incapacitated", new PyInt(0));
        slim->SetItemString("posDelayTime", new PyInt(m_delayTime));
        slim->SetItemString("remoteStructureID", new PyInt(m_bridgeData.toItemID));
        slim->SetItemString("remoteSystemID", new PyInt(m_bridgeData.toSystemID));
    PyTuple *shipData = new PyTuple(2);
        shipData->SetItem(0, new PyLong(m_data.itemID));
        shipData->SetItem(1, new PyObject("foo.SlimItem", slim));
    PyTuple *sItem = new PyTuple(2);
        sItem->SetItem(0, new PyString("OnSlimItemChange"));
        sItem->SetItem(1, shipData);
    m_destiny->SendSingleDestinyUpdate(&sItem);
}