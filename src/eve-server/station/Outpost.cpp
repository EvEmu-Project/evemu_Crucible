/**
 * @name Outpost.cpp
 *   Class for Outposts.
 *
 * @Author:           James
 * @date:   17 October 2021
 */

#include "station/Outpost.h"
#include "Client.h"
#include "system/SystemManager.h"

OutpostSE::OutpostSE(StationItemRef station, EVEServiceManager &services, SystemManager* system)
: StationSE(station, services, system) 
{
    // Create default dynamic attributes in the AttributeMap:
    station->SetAttribute(AttrOnline,             EvilOne, false);
    station->SetAttribute(AttrCapacity,           STATION_HANGAR_MAX_CAPACITY, false);
    station->SetAttribute(AttrInertia,            EvilOne, false);
    station->SetAttribute(AttrDamage,             EvilZero, false);
    station->SetAttribute(AttrShieldCapacity,     20000000.0, false);
    station->SetAttribute(AttrShieldCharge,       station->GetAttribute(AttrShieldCapacity), false);
    station->SetAttribute(AttrArmorHP,            station->GetAttribute(AttrArmorHP), false);
    station->SetAttribute(AttrArmorUniformity,    station->GetAttribute(AttrArmorUniformity), false);
    station->SetAttribute(AttrArmorDamage,        EvilZero, false);
    station->SetAttribute(AttrMass,               station->type().mass(), false);
    station->SetAttribute(AttrRadius,             station->type().radius(), false);
    station->SetAttribute(AttrVolume,             station->type().volume(), false);
}

// Each outpost has station service entities which are spawned at the same position as the station itself
void OutpostSE::SpawnStationService(Client* pClient, StationData stData, uint32 serviceType) {
    ItemData svcData(serviceType, stData.corporationID, stData.systemID, flagNone);
    InventoryItemRef svcRef = sItemFactory.SpawnItem(svcData);
    ItemSystemEntity* svcSE = new ItemSystemEntity(svcRef, pClient->services(), pClient->SystemMgr());
    svcSE->SetPosition(stData.position);
    svcRef->SaveItem();
    pClient->SystemMgr()->AddEntity(svcSE);
}