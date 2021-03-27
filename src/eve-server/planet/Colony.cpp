/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Cometo (basic system idea)
    Updates:    Allan   (mostly complete and working - 27Dec16)
    Major Update:   Allan  process code rewrite/optimized 19Jul19
*/

/*
 * COLONY__ERROR
 * COLONY__WARNING
 * COLONY__MESSAGE
 * COLONY__DEBUG
 * COLONY__INFO
 * COLONY__INFO
 * COLONY__DUMP
 * COLONY__RES_DUMP
 * COLONY__GC_DUMP
 * COLONY__PKT_TRACE
 * COLONY__DB_ERROR
 * COLONY__DB_WARNING
 */

#include "eve-server.h"

#include "PyServiceMgr.h"
#include "Client.h"
#include "inventory/ItemType.h"
#include "planet/Colony.h"
#include "planet/Planet.h"
#include "planet/PlanetMgr.h"
#include "planet/PlanetDataMgr.h"
#include "CustomsOffice.h"
 // fund xfer and journal logging methods
#include "account/AccountService.h"
// for launching shit...
#include "system/Container.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"

/** @todo:
 * items to work on...
 *  import/export tax
 *  cost of building
 *  timers (launch, run, current, logistics)
 *  planet items attribs/effects
 *  item->Move() logistics
 *
    __notifyevents__ = [
    'OnPlanetCommandCenterDeployedOrRemoved',
     'OnPlanetPinsChanged',
     'OnColonyPinCountUpdated',
 */
/* The list of restricted systems is:
    Amarr
    Arnon
    Aunia
    Auvergne
    Balginia
    Dodixie
    Fricoure
    Ichoriya
    Irjunen
    Isaziwa
    Isinokka
    Jita
    Lustrevik
    Motsu
    Oursulaert
    Rens
    Sankkasen
    Umokka
    */
/*10% + % Player Tax - 1% per level of Customs Code Expertise
 * Export fee = Base cost * tax rate (*1.5 if launched via Command Center)
Import fee = Base cost * tax rate * 0.5

Example: Exporting a unit of Biomass (P1) using a Launchpad from a low-sec planet with a 10% Player Tax will cost will cost 40 ISK (400 * 10% * 1 )
Importing that unit of Biomass to a high-sec factory planet will cost at minimum an additional 20 ISK (400 * (10% + % Player Tax) * 0.5 )

Base costs for each tier of products can be found in this table. Base costs are per unit.
level     Base Cost
R0                  5 ISK
P1                400 ISK
P2              7,200 ISK
P3             60,000 ISK
P4          1,200,000 ISK
*/

Colony::Colony(PyServiceMgr* mgr, Client* pClient, SystemEntity* pSE)
:m_svcMgr(mgr),
m_client(pClient),
m_pSE(pSE->GetPlanetSE()),
m_colonyTimer(0)
{
    ccPin = new PI_CCPin();

    m_active = false;
    m_loaded = false;
    m_newHead = false;
    m_toUpdate = false;

    m_pg = 0;
    m_cpu = 0;
    m_pLevel = 5;
    m_colonyID = 0;
    m_procTime = 0; // process check.  init to zero and stores last proc time, which is lastRunTime in command center
    tempPinIDs.clear();
    _log(COLONY__DEBUG, "Colony::Colony() c'tor called for %s(%u) by %s(%u)", pSE->GetName(), pSE->GetID(), pClient->GetName(), pClient->GetCharacterID());
}

Colony::~Colony()
{
    SafeDelete(ccPin);
}

/***********
 * NOTE:  seems that client tracks all PI production,
 *  so doing so here is just a waste of processing power.
 *  all processing is commented out until proven needed.
 */
// update....this will be needed for initial update as client only processes AFTER colony data is sent
void Colony::Init()
{
    if (m_loaded)
        return;

    // check for and load colony if the char has one on this planet
    if (m_db.LoadColony(m_client->GetCharacterID(), m_pSE->GetID(), ccPin)) {
        m_colonyID = ccPin->ccPinID;
        Load();
    }

    if (m_loaded)
        Update();
}

void Colony::Shutdown()
{
    Update();
    UpdatePlantPins();
    m_db.SaveContents(ccPin);
}

void Colony::Load()
{
    // redundant check
    if (m_loaded) {
        Update();
        return;
    }

    m_db.LoadPins(m_colonyID, ccPin->pins);
    m_db.LoadLinks(m_colonyID, ccPin->links);
    m_db.LoadRoutes(m_colonyID, ccPin->routes);

    LoadPlants();

    for (auto cur : ccPin->routes) {
        m_srcRoutes.emplace(cur.second.srcPinID, cur.second);
        m_destRoutes.emplace(cur.second.destPinID, cur.second);
    }

    if (m_procTime < 1)
        m_procTime = GetFileTimeNow();

    if (!ccPin->pins.empty())
        m_loaded = true;
}

void Colony::Save()
{
    /** @todo maybe separate these saves */
    UpdatePlantPins(); // this MUST be called before Save() and GetColony() to update plant pins with current data
    m_db.SavePins(ccPin);
    m_db.SaveLinks(ccPin);
    m_db.SaveRoutes(ccPin);
    m_db.SaveContents(ccPin);
    m_db.UpdatePlanetPins(m_colonyID, ccPin->pins.size());
}

// called by PlanetSE::Process() for loaded colony.
//  NOTE: colony is only loaded AFTER client calls for it.
void Colony::Process()
{
    if (m_colonyTimer.Check()) { //  this will process colony data every 30 mins. (typical cycle time)
        if (ccPin->pins.empty()) {
            m_colonyTimer.Disable();
            return;
        }

        double profileStartTime = GetTimeUSeconds();
        Update();

        // profile timer for the colony updates
        if (sConfig.debug.UseProfiling)
            sProfiler.AddTime(Profile::colony, GetTimeUSeconds() - profileStartTime);
    }

    if (m_toUpdate) {
        //  this is part of clever code to avoid db hits on every update.
        //  this method will check for updated contents and save to db as needed.
        std::map<uint32, PI_Pin>::iterator itr;
        for (auto cur : ccPin->pins)
            if (cur.second.update) {
                m_db.RemoveContents(cur.first);
                m_db.SavePinContents(m_colonyID, cur.first, cur.second.contents);
                cur.second.update = false;
                if (cur.second.isProcess) {
                    itr = ccPin->pins.find(cur.first);
                    if (itr != ccPin->pins.end()) {
                        itr->second.state                   = cur.second.state;
                        itr->second.cycleTime               = cur.second.cycleTime;
                        itr->second.installTime             = cur.second.installTime;
                        itr->second.lastRunTime             = cur.second.lastRunTime;
                        itr->second.schematicID             = cur.second.schematicID;
                        itr->second.qtyPerCycle             = cur.second.qtyPerCycle;
                        itr->second.hasReceivedInputs       = cur.second.hasReceivedInputs;
                        itr->second.receivedInputsLastCycle = cur.second.receivedInputsLastCycle;
                    }
                }
            }

        m_db.UpdatePins(0, ccPin);
        m_toUpdate = false;
    }

}

uint32 Colony::GetOwner()
{
    return m_client->GetCharacterID();
}

void Colony::LoadPlants()
{
    bool update = false;
    for (auto cur: ccPin->pins) {
        // set proc time on load
        if (cur.second.isCommandCenter) {
            m_procTime = cur.second.lastRunTime;
            if (m_procTime < EvE::Time::Hour)
                m_procTime = GetFileTimeNow();
        }

        // load plants in mem objects
        if (cur.second.isProcess) {
            PI_Plant plant                  = PI_Plant();
            plant.data                      = PI_Schematic();
            plant.state                     = cur.second.state;
            plant.cycleTime                 = cur.second.cycleTime;
            plant.installTime               = cur.second.installTime;
            plant.lastRunTime               = cur.second.lastRunTime;
            plant.schematicID               = cur.second.schematicID;
            plant.hasReceivedInputs         = cur.second.hasReceivedInputs;
            plant.receivedInputsLastCycle   = cur.second.receivedInputsLastCycle;

            if (plant.schematicID) {
                sPIDataMgr.GetSchematicData(plant.schematicID, plant.data);
                plant.cycleTime     = plant.data.cycleTime * EvE::Time::Second; // data.cycleTime is in seconds
                plant.pLevel        = sPIDataMgr.GetProductLevel(plant.data.outputType);   // i am ordering plant processing by output's Plevel
                plant.qtyPerCycle   = plant.data.outputQty;     // this is not saved

                if (cur.second.lastRunTime == 0) {
                    update = true;
                    cur.second.lastRunTime = GetFileTimeNow() - plant.cycleTime;
                }
            }

            m_pLevel = (uint8)EvE::min(m_pLevel, plant.pLevel);
            ccPin->plants[cur.first] = plant;
            m_plantMap.emplace(plant.pLevel, cur.first);
        }

    }

    // set process timer to 30m
    if (!m_colonyTimer.Enabled())
        m_colonyTimer.Start(30*60*1000);

    if (update)
        UpdatePlantPins();
}

void Colony::UpdatePlantPins(uint32 pinID/*0*/)
{
    std::map<uint32, PI_Pin>::iterator itr;
    std::map<uint32, PI_Plant>::iterator itr2;
    if (pinID) {
        itr = ccPin->pins.find(pinID);
        if (itr != ccPin->pins.end()) {
            itr2 = ccPin->plants.find(pinID);
            if (itr2 != ccPin->plants.end()) {
                itr->second.state                   = itr2->second.state;
                itr->second.cycleTime               = itr2->second.cycleTime;
                itr->second.installTime             = itr2->second.installTime;
                itr->second.lastRunTime             = itr2->second.lastRunTime;
                itr->second.schematicID             = itr2->second.schematicID;
                itr->second.qtyPerCycle             = itr2->second.qtyPerCycle;
                itr->second.hasReceivedInputs       = itr2->second.hasReceivedInputs;
                itr->second.receivedInputsLastCycle = itr2->second.receivedInputsLastCycle;
            }
        }
    } else {
        for (auto cur : ccPin->plants) {
            itr = ccPin->pins.find(cur.first);
            if (itr != ccPin->pins.end()) {
                itr->second.state                   = cur.second.state;
                itr->second.cycleTime               = cur.second.cycleTime;
                itr->second.installTime             = cur.second.installTime;
                itr->second.lastRunTime             = cur.second.lastRunTime;
                itr->second.schematicID             = cur.second.schematicID;
                itr->second.qtyPerCycle             = cur.second.qtyPerCycle;
                itr->second.hasReceivedInputs       = cur.second.hasReceivedInputs;
                itr->second.receivedInputsLastCycle = cur.second.receivedInputsLastCycle;
            }
        }
    }
    m_db.UpdatePins(pinID, ccPin);
}

void Colony::AbandonColony()
{
    /** @todo  go thru entire pinMap and delete each itemRef to remove pin/link contents from db. */
    for (auto cur : ccPin->pins) {
        m_db.RemovePin(cur.first);
        m_db.RemoveContents(cur.first);
        sItemFactory.RemoveItem(cur.first);
    }
    for (auto cur : ccPin->links) {
        m_db.RemovePin(cur.first);
        sItemFactory.RemoveItem(cur.first);
    }
    InventoryItemRef iRef = sItemFactory.GetItem(m_colonyID);
    iRef->Delete();
    m_db.DeleteColony(m_colonyID, m_pSE->GetID(), m_client->GetCharacterID());
    SafeDelete(ccPin);
    ccPin = new PI_CCPin();
    m_colonyID = 0;
    m_colonyTimer.Disable();
}

void Colony::CreateCommandPin(uint32 itemID, uint32 typeID, double latitude, double longitude) {
    m_colonyID = itemID;
    ccPin->ccPinID = itemID;
    m_db.SaveCommandCenter(itemID, m_client->GetCharacterID(), m_pSE->GetID(), typeID, latitude, longitude);
    m_db.AddPlanetForChar(m_pSE->SystemMgr()->GetID(), m_pSE->GetID(), m_client->GetCharacterID(), m_colonyID, m_pSE->GetTypeID());
    ccPin->level = PI::Pin::Level0;
    m_procTime = GetFileTimeNow();
    CreatePin(EVEDB::invGroups::Command_Centers, itemID, typeID, latitude, longitude);
    m_db.SavePins(ccPin);
}

void Colony::CreatePin(uint32 groupID, uint32 pinID, uint32 typeID, double latitude, double longitude) {
    /** @todo will have to write code for effects and checks for pg/cpu/m3/etc for all of these.  */
    // maybe not...client checks before sending command
    using namespace EVEDB::invGroups;
    PI_Pin pin = PI_Pin();
    InventoryItemRef iRef(nullptr);
    if (groupID == Command_Centers) {
        iRef = sItemFactory.GetItem(m_colonyID);
        if (iRef->quantity() > 1) {
            // check for stack of CC items, and split as needed
            ItemData data(typeID, m_client->GetCharacterID(), locTemp, flagNone, iRef->quantity() -1);
            InventoryItemRef iRef2 = sItemFactory.SpawnItem(data);
            iRef2->Move(m_client->GetShipID(), flagCargoHold);
            iRef->SetQuantity(1);
        }
        m_client->GetShip()->RemoveItem(iRef);
    } else {
        // type, owner, location, flag, qty
        ItemData data(typeID, m_client->GetCharacterID(), m_pSE->GetID(), flagNone, 1);
        iRef = sItemFactory.SpawnItem(data);

        /*  this shit doesnt work....changes arent sent to client.  not sure why
        m_pg = iRef->GetAttribute(AttrPowerLoad).get_int();
        m_cpu = iRef->GetAttribute(AttrCpuLoad).get_int();
        if (groupID != Planetary_Links) {
            // reset pg/cpu needs based on char skills for all modules (ex links)
            m_pg *= (1 - ( 0.05f * (m_client->GetChar()->GetSkillLevel(EvESkill::Engineering, true))));               // 5% decrease in need
            m_pg *= (1 - ( 0.01f * (m_client->GetChar()->GetSkillLevel(EvESkill::EnergyManagement, true))));          // 1% decrease in need
            m_pg *= (1 - ( 0.01f * (m_client->GetChar()->GetSkillLevel(EvESkill::CommandCenterUpgrades, true))));     // 1% decrease in need
            m_pg *= (1 - ( 0.01f * (m_client->GetChar()->GetSkillLevel(EvESkill::EnergySystemsOperation, true))));    // 1% decrease in need

            m_cpu *= (1 - ( 0.05f * (m_client->GetChar()->GetSkillLevel(EvESkill::Electronics, true))));              // 5% decrease in need
            m_cpu *= (1 - ( 0.01f * (m_client->GetChar()->GetSkillLevel(EvESkill::CommandCenterUpgrades, true))));    // 1% decrease in need
        } */
    }

    pin.typeID = typeID;
    pin.ownerID = m_client->GetCharacterID();
    pin.latitude = latitude;
    pin.longitude = longitude;
    pin.state = PI::Pin::State::Idle;

    switch(groupID) {
        case Command_Centers: {     // 1027
            pin.isStorage = true;
            pin.isLaunchable = true;
            pin.isCommandCenter = true;
            pin.lastRunTime = GetFileTimeNow();
            pin.installTime = GetFileTimeNow(); // pin creation time here.
        } break;
        case Processors: {         // 1028
            pin.isStorage = true;
            pin.isProcess = true;
            pin.lastRunTime = 0;
            PI_Plant plant = PI_Plant();
            ccPin->plants[iRef->itemID()] = plant;
        } break;
        case Extractor_Control_Units: { // 1063
            pin.isECU = true;
            pin.lastRunTime = 0;
            pin.qtyPerCycle = (uint16)iRef->GetAttribute(AttrPinExtractionQuantity).get_int();
        } break;
        case Spaceports:{   // 1030
            pin.isStorage = true;
            pin.isLaunchable = true;
            pin.installTime = GetFileTimeNow(); // pin creation time here.
        } break;
        case Storage_Facilities: {  // 1029
            pin.isStorage = true;
            pin.installTime = GetFileTimeNow(); // pin creation time here.
        } break;
        case Mercenary_Bases:
        case Capsuleer_Bases: {
            pin.isBase = true;
            pin.installTime = GetFileTimeNow(); // pin creation time here.
            /* nothing to do yet */
        } break;
        case Planetary_Links:  // 1036
        case Extractors: {  // 1026
            /* make error.  should never get here.  these are NOT pins  */
            /*
            m_cpu = iRef->GetAttribute().get_int();
             1633    powerLoadPerKm          NULL    0.15
             1634    cpuLoadPerKm            NULL    0.2
             1635    cpuLoadLevelModifier    NULL    1.4
             1636    powerLoadLevelModifier  NULL    1.2
             */
        } break;
    }

    iRef->Move(m_pSE->GetID(), flagPlanetSurface, true);
    iRef->ChangeSingleton(true);
    // cannot change attributes on PI items.....  :(
    //iRef->SetAttribute(AttrCpuLoad, m_cpu);
    //iRef->SetAttribute(AttrPowerLoad, m_pg);

    ccPin->pins[iRef->itemID()] = pin;

    if (groupID != Command_Centers)
        tempPinIDs.insert(std::pair<uint8, uint32>(pinID, iRef->itemID()));     // save map of tempID to itemID - this handles the stacked-calls from client to use real itemIDs

    _log(COLONY__INFO, "Colony::CreatePin() - Created pin for %s(%u)", iRef->name(), iRef->itemID());
}

void Colony::CreateLink(uint32 src, uint32 dest, uint16 level) {
    if (IsTempPinID(src) and (tempPinIDs.size() > 0)) {
        std::map<uint8, uint32>::iterator itr = tempPinIDs.find(src);
        if (itr != tempPinIDs.end())
            src = itr->second;
    }
    if (IsTempPinID(dest) and (tempPinIDs.size() > 0)) {
        std::map<uint8, uint32>::iterator itr = tempPinIDs.find(dest);
        if (itr != tempPinIDs.end())
            dest = itr->second;
    }
    ItemData data(2280, m_client->GetCharacterID(), locTemp, flagNone, 1);
    InventoryItemRef iRef = sItemFactory.SpawnItem(data);
    iRef->Move(m_pSE->GetID(), flagPlanetSurface, true);
    iRef->SaveItem();

    PI_Link link = PI_Link();
        link.state = PI::Pin::State::Idle;
        link.level = level;
        link.endpoint1 = src;
        link.endpoint2 = dest;
        link.typeID = 2280; // Only link type in the game.
    ccPin->links[iRef->itemID()] = link;

    m_db.SaveLinks(ccPin);
    _log(COLONY__INFO, "Colony::CreateLink() - Created link - id:%u,  src:%u, dest:%u, level:%u", iRef->itemID(), src, dest, level);
}

void Colony::CreateRoute(uint16 routeID, uint32 typeID, uint32 qty, PyList* path) {
    // routeID is sent as tempID like pins.
    std::list<uint32> list1;
    list1.clear();
    for (size_t i = 0; i < path->size(); ++i) {
        if (path->GetItem(i)->IsTuple()) {
            list1.push_back(PyRep::IntegerValue(path->GetItem(i)->AsTuple()->GetItem(1)));
        } else if (path->GetItem(i)->IsInt()) {
            list1.push_back(PyRep::IntegerValue(path->GetItem(i)));
        } else {
            _log(COLONY__ERROR, "Colony::CreateRoute() - List item type unrecognized: %s", path->GetItem(1)->TypeString());
        }
    }

    if (tempPinIDs.size() > 0) {
        std::list<uint32> list2;
        list2.clear();
        std::map<uint8, uint32>::iterator itr;
        for (auto cur : list1) {
            if (IsTempPinID(cur)) {
                itr = tempPinIDs.find(cur);
                if (itr != tempPinIDs.end())
                    list2.push_back(itr->second);
            } else {
                list2.push_back(cur);
            }
        }
        list1.clear();
        list1 = list2;
    }

    PI_Route route = PI_Route();
        route.state = PI::Pin::State::Idle;
        route.priority = PI::Route::PriorityNorm;
        route.commodityTypeID = typeID;
        route.commodityQuantity = qty;
        route.srcPinID = list1.front();
        route.destPinID = list1.back();
        route.path = list1;

    routeID = m_db.SaveRoute(m_colonyID, route);
    ccPin->routes[routeID] = route;

    m_srcRoutes.emplace(route.srcPinID, route);
    m_destRoutes.emplace(route.destPinID, route);

    _log(COLONY__INFO, "Colony::CreateRoute() - Created route id %u for %u of typeID %u, making %u hops.", routeID, qty, typeID, (uint32)path->size() -1);

    // route has been created and added to list.  check for materials being moved, and if source has the mat, remove qty and send to dest.
    std::map<uint32, PI_Pin>::iterator srcPin = ccPin->pins.find(route.srcPinID);
    if (srcPin == ccPin->pins.end())
        return;  // source not found.  make error here.
    std::map<uint32, PI_Pin>::iterator destPin = ccPin->pins.find(route.destPinID);
    if (destPin == ccPin->pins.end())
        return;  // destination not found.  make error here.
    std::map<uint16, uint32>::iterator itr = srcPin->second.contents.find(route.commodityTypeID);
    if (itr == srcPin->second.contents.end())
        return;  // this material wasnt found in source container....cant move what we aint got..

    uint32 amount = route.commodityQuantity;
    // remove contents from storage pin
    if (itr->second > amount) {
        itr->second -= amount;
    } else {
        amount = itr->second;
        srcPin->second.contents.erase(itr);
    }
    srcPin->second.update = true;

    // add contents to dest pin if we have any
    itr = destPin->second.contents.find(route.commodityTypeID);
    if (itr != destPin->second.contents.end()) {
        itr->second += amount;
    } else {
        destPin->second.contents[route.commodityTypeID] = amount;
    }
    destPin->second.update = true;
    // we have received a material from this route. enable check for all required materials for this Schematic
    if (destPin->second.isProcess) {
        std::map<uint32, PI_Plant>::iterator plantPin = ccPin->plants.find(destPin->first);
        if (plantPin != ccPin->plants.end())
            plantPin->second.hasReceivedInputs = true;
    }
    m_toUpdate = true;
}

void Colony::UpgradeCommandCenter(uint32 pinID, int8 level) {
    ccPin->level = level;
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(pinID);
    if (itr != ccPin->pins.end()) {
        itr->second.level = level;
        m_db.SaveCCLevel(pinID, level);
        _log(COLONY__INFO, "Colony::UpgradeCommandCenter() - Upgraded Command Center %u to level:%u", pinID, level);
    } else {
        _log(COLONY__ERROR, "Colony::UpgradeCommandCenter() - pinID %u not found in ccPin.pins map", pinID);
    }
}

void Colony::UpgradeLink(uint32 src, uint32 dest, uint8 level)
{
    std::map<uint32, PI_Link>::iterator itr = ccPin->links.begin();
    for (; itr != ccPin->links.end(); ++itr)
        if (itr->second.endpoint1 == src)
            if (itr->second.endpoint2 == dest) {
                itr->second.level = level;
                m_db.SaveLinks(ccPin);
                return;
            }
}

void Colony::RemovePin(uint32 pinID)
{
    uint8 linkCount = 0, routeCount = 0, pathCount = 0;
    // check for and remove links to this pinID
    std::map<uint32, PI_Link>::iterator itr = ccPin->links.begin();
    while (itr != ccPin->links.end()) {
        if (itr->second.endpoint1 == pinID) {
            m_db.RemoveLink(itr->first);
            itr = ccPin->links.erase(itr);
            ++linkCount;
            continue;
        }
        if (itr->second.endpoint2 == pinID) {
            m_db.RemoveLink(itr->first);
            itr = ccPin->links.erase(itr);
            ++linkCount;
            continue;
        }
        ++itr;
    }

    // check for routes to, from, or thru this pin
    std::map<uint16, PI_Route>::iterator rItr = ccPin->routes.begin();
    while (rItr != ccPin->routes.end()) {
        /*
            uint32 srcPinID;
            uint32 destPinID;
            std::list<uint32> path;
         */
        if (rItr->second.srcPinID == pinID) {
            m_db.RemoveRoute(rItr->first);
            rItr = ccPin->routes.erase(rItr);
            ++routeCount;
            continue;
        }
        if (rItr->second.destPinID == pinID) {
            m_db.RemoveRoute(rItr->first);
            rItr = ccPin->routes.erase(rItr);
            ++routeCount;
            continue;
        }
        // at this point, the pin being removed isnt src or dest, so check for intermediate routing.
        std::list<uint32>::iterator pItr = rItr->second.path.begin();
        while (pItr != rItr->second.path.end()) {
            //  we may have to reroute from this pin removal...
            if (*pItr == pinID) {
                //reroute = true;
                pItr = rItr->second.path.erase(pItr);
                ++pathCount;
                continue;
            }
            ++pItr;
        }
        ++rItr;
    }

    m_srcRoutes.erase(pinID);
    m_destRoutes.erase(pinID);

    ccPin->pins.erase(pinID);
    ccPin->plants.erase(pinID);  // may or may not be here.
    m_db.RemovePin(pinID);
    m_db.SaveLinks(ccPin);
    m_db.SaveRoutes(ccPin);
    m_db.UpdatePlanetPins(m_colonyID, ccPin->pins.size());
    _log(COLONY__INFO, "Colony::RemovePin() - Removed pin %u with %u routes and %u links.  Upset %u routes by removing this pin", \
                            pinID, routeCount, linkCount, pathCount);
}

void Colony::RemoveLink(uint32 src, uint32 dest)
{
    std::map<uint32, PI_Link>::iterator itr = ccPin->links.begin();
    for (; itr != ccPin->links.end(); ++itr)
        if (itr->second.endpoint1 == src)
            if (itr->second.endpoint2 == dest) {
                _log(COLONY__INFO, "Colony::RemoveLink() - Removing linkID %u - src: %u, dest: %u", itr->first, src, dest);
                m_db.RemoveLink(itr->first);
                ccPin->links.erase(itr);
                return;
            }
}

void Colony::RemoveRoute(uint16 routeID)
{
    std::map<uint16, PI_Route>::iterator routeItr = ccPin->routes.find(routeID);
    m_srcRoutes.erase(routeItr->second.srcPinID);
    m_destRoutes.erase(routeItr->second.destPinID);
    ccPin->routes.erase(routeID);
    m_db.RemoveRoute(routeID);
    _log(COLONY__INFO, "Colony::RemoveRoute() - Removed route: %u", routeID);
}

void Colony::AddExtractorHead(uint32 ecuID, uint16 headID, double latitude, double longitude)
{
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(ecuID);
    if (itr != ccPin->pins.end()) {
        m_newHead = true;
        tempECUs.push_back(ecuID);
        PI_Heads head = PI_Heads();
            head.typeID = itr->second.schematicID;
            head.ecuPinID = ecuID;
            head.latitude = latitude;
            head.longitude = longitude;
        itr->second.heads[headID] = head;
    } else {
        _log(COLONY__ERROR, "Colony::AddExtractorHead() - ecuID %u not found in ccPin.pins map", ecuID);
    }
}

void Colony::MoveExtractorHead(uint32 ecuID, uint16 headID, double latitude, double longitude)
{
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(ecuID);
    if (itr != ccPin->pins.end()) {
        std::map<uint16, PI_Heads>::iterator head = itr->second.heads.find(headID);
        if (head != itr->second.heads.end()) {
            m_newHead = true;
            tempECUs.push_back(ecuID);
            // find head and update....
            head->second.latitude = latitude;
            head->second.longitude = longitude;
        } else {
            _log(COLONY__ERROR, "Colony::MoveExtractorHead() - headID %u not found in pin.heads map", headID);
        }
    } else {
        _log(COLONY__ERROR, "Colony::MoveExtractorHead() - ecuID %u not found in ccPin.pins map", ecuID);
    }
}

void Colony::KillExtractorHead(uint32 ecuID, uint16 headID)
{
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(ecuID);
    if (itr != ccPin->pins.end()) {
        itr->second.heads.erase(headID);
    } else {
        _log(COLONY__ERROR, "Colony::KillExtractorHead() - ecuID %u not found in ccPin.pins map", ecuID);
    }
}

void Colony::SetSchematic(uint32 pinID, uint8 schematicID/*0*/)
{
    if (IsTempPinID(pinID) and (tempPinIDs.size() > 0)) {
        std::map<uint8, uint32>::iterator itr = tempPinIDs.find(pinID);
        if (itr != tempPinIDs.end())
            pinID = itr->second;
    }

    std::map<uint32, PI_Plant>::iterator itr = ccPin->plants.find(pinID);
    if (itr == ccPin->plants.end())
        _log(COLONY__ERROR, "Colony::SetSchematic() - plantID %u not found in ccPin.plants map", pinID);

    if (schematicID) {
        // install new schematic.  set lastRunTime to 0
        sPIDataMgr.GetSchematicData(schematicID, itr->second.data);
        itr->second.state                   = PI::Pin::State::Idle;
        itr->second.pLevel                  = sPIDataMgr.GetProductLevel(itr->second.data.outputType);
        itr->second.cycleTime               = itr->second.data.cycleTime * EvE::Time::Second;
        itr->second.installTime             = GetFileTimeNow();
        itr->second.qtyPerCycle             = itr->second.data.outputQty;
        itr->second.schematicID             = schematicID;
        itr->second.lastRunTime             = 0;
        itr->second.hasReceivedInputs       = false;
        itr->second.receivedInputsLastCycle = false;

        m_pLevel = (uint8)EvE::min(m_pLevel, itr->second.pLevel);

        // set process timer to 30m
        if (!m_colonyTimer.Enabled())
            m_colonyTimer.Start(30*60*1000);
        _log(COLONY__INFO, "Colony::SetSchematic() - Set Schematic %u in plantID %u", schematicID, pinID);
    } else {
        itr->second = PI_Plant();
        itr->second.data = PI_Schematic();
        itr->second.state = PI::Pin::State::Idle;
        _log(COLONY__INFO, "Colony::SetSchematic() - Cleared Schematic from plantID %u", pinID);
    }
    // save schematic update
    UpdatePlantPins(pinID); // this MUST be called before Save() and GetColony() to update plant pin with current data
}

void Colony::InstallProgram(uint32 ecuID, uint16 typeID, float headRadius, PlanetMgr* pPMgr)
{
    /*
     * 09:54:54 [PlanetCallDump]       [ 0]   [10]   [ 1]  Tuple: 3 elements
     * 09:54:54 [PlanetCallDump]       [ 0]   [10]   [ 1]   [ 0]    Integer: 140000565  ecuID
     * 09:54:54 [PlanetCallDump]       [ 0]   [10]   [ 1]   [ 1]    Integer: 2272       typeID
     * 09:54:54 [PlanetCallDump]       [ 0]   [10]   [ 1]   [ 2]       Real: 0.011281   headRadius
     */
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(ecuID);
    if (itr == ccPin->pins.end()) {
        _log(COLONY__ERROR, "Colony::InstallProgram() - ecuPinID %u not found in ccPin.pins map", ecuID);
        return;
    }
    if (typeID < 1) {
        // uninstall program
        itr->second = PI_Pin();
        // reset extraction quantity in ecu attrib.  this doesnt check for invalid item
        sItemFactory.GetItem(ecuID)->ResetAttribute(AttrPinExtractionQuantity);
        return;
    }
    if (itr->second.programType != typeID) {
        // is this installing a new program?
        _log(COLONY__ERROR, "Colony::InstallProgram() - typeID %u does not match previously saved program.  *edit*  not sure wtf that means now.", typeID);
        return;
    }

    itr->second.state = PI::Pin::State::Active;
    itr->second.headRadius = headRadius;
    itr->second.installTime = GetFileTimeNow();
    itr->second.lastRunTime = GetFileTimeNow();

    PyList* list = new PyList();
    for (auto cur : itr->second.heads)
        list->AddItem(new PyInt(cur.second.ecuPinID));
    // set up extractor program data
    sPIDataMgr.GetProgramResultInfo(this, ecuID, typeID, list, headRadius);
}

void Colony::SetProgramResults(uint32 ecuID, uint16 typeID, uint16 numCycles, float headRadius, float cycleTime, uint32 qtyPerCycle)
{
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(ecuID);
    if (itr == ccPin->pins.end()) {
        _log(COLONY__ERROR, "Colony::SetProgramResults() - ecuPinID %u not found in ccPin.pins map", ecuID);
        return;
    }

    itr->second.cycleTime = cycleTime * EvE::Time::Hour ;
    itr->second.programType = typeID;
    itr->second.expiryTime = (cycleTime * numCycles) * EvE::Time::Hour + GetFileTimeNow();
    itr->second.headRadius = headRadius;
    itr->second.qtyPerCycle = qtyPerCycle;
    itr->second.schematicID = sPIDataMgr.GetHeadType(sItemFactory.GetItem(ecuID)->typeID(), typeID);

    m_db.UpdateECUPin(ecuID, ccPin);

    // save extraction quantity in ecu attrib    this doesnt check for invalid item
    sItemFactory.GetItem(ecuID)->SetAttribute(AttrPinExtractionQuantity, qtyPerCycle, false);

    // set process timer to 30m
    if (!m_colonyTimer.Enabled())
        m_colonyTimer.Start(30*60*1000);
}
/*{'FullPath': u'UI/Messages', 'messageID': 256790, 'label': u'PlanetBlackListedBody'}(u'{planet} is not available for the general public.', None, {u'{planet}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'planet'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256791, 'label': u'CannotInstallWithoutScanResultsBody'}(u'Your mining foreman reports that an intern seems to have misplaced the necessary mineral survey results. You will need to order a fresh deposit scan before this {typeName} can begin operating.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256792, 'label': u'QueueCannotTrainPastMaximumLevelBody'}(u'You cannot train {typeName} further, as is it already at maximum level.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256793, 'label': u'CannotUpgradeLinkAlreadyMaxedBody'}(u"You cannot upgrade this {typeName}, as it has already been upgraded to technology's bleeding edge.", None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256794, 'label': u'CreateRouteDestinationCannotAcceptCommodityBody'}(u'You are unable to create that route, as the destination is unable to utilize {typeName}.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256795, 'label': u'CreateRouteCommodityProductionTooSmallBody'}(u"You are unable to create this shipping route as the route's origin would not produce enough {typeName} to fulfill all of its existing routes, in addition to the new one.", None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256796, 'label': u'CreateRouteCommodityNotProducedBody'}(u"You are unable to create a shipping route for {typeName}, as it is not produced at the route's origin.", None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 */
PyDict* Colony::TransferCommodities(uint32 srcID, uint32 destID, std::map< uint16, uint32 > items)
{
    std::map<uint32, PI_Pin>::iterator src = ccPin->pins.find(srcID);
    if (src == ccPin->pins.end()) {
        _log(COLONY__ERROR, "Colony::TransferCommodities() - srcPin %u not found in ccPin.pins map", srcID);
        if (m_client->CanThrow())
            throw PyException(MakeCustomError("Source not found."));
        return nullptr; // make error and exit.
    }
    std::map<uint32, PI_Pin>::iterator dest = ccPin->pins.find(destID);
    if (dest == ccPin->pins.end()) {
        _log(COLONY__ERROR, "Colony::TransferCommodities() - destPin %u not found in ccPin.pins map", destID);
        if (m_client->CanThrow())
            throw PyException(MakeCustomError("Destination not found."));
        return nullptr; // make error and exit.
    }
    /*{'FullPath': u'UI/Messages', 'messageID': 256630, 'label': u'ExpeditedTransferNotEnoughSpaceBody'}(u'There is not enough space at the transfer destination for the selected commodities.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 256775, 'label': u'CannotPutMissionItemInCargolinkBody'}(u'You cannot store the {typeName} in a planetary customs facility, as it an agent has issued a special embargo for this particular item.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 256776, 'label': u'CannotExportCommodityNotEnoughBody'}(u'Your request to export {desired} units of {typeName} cannot be fulfilled, as the spaceport only has {contained} in stock.', None, {u'{contained}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'contained'}, u'{desired}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'desired'}, u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 256777, 'label': u'CannotExportCommodityNotFoundBody'}(u"You cannot export {typeName}, as your spaceport's storehouse does not appear to contain any.", None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 256778, 'label': u'RouteFailedValidationExpeditedSourceLacksCommodityQtyBody'}(u"You cannot perform this expedited transfer as the facility from which you're sourcing your commodities currently lacks the requested {qty} units of {typeName}.", None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}, u'{qty}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'qty'}})
     * {'FullPath': u'UI/Messages', 'messageID': 256779, 'label': u'RouteFailedValidationExpeditedSourceLacksCommodityBody'}(u'You cannot perform this expedited transfer, as the facility from which you are sourcing your commodities appears to lack the {typeName} which you wish to transfer.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
     */

    // capacities are checked in client.  procede with xfer
    for (auto cur : items) {
        std::map<uint16, uint32>::iterator srcItr = src->second.contents.find(cur.first), destItr = dest->second.contents.find(cur.first);
        if (srcItr != src->second.contents.end()) {
            if (srcItr->second > cur.second) {
                srcItr->second -= cur.second;
            } else {
                src->second.contents.erase(srcItr);
            }
        }   //  if src contents not found, assume client is right and procede with xfer
        if (destItr != dest->second.contents.end()) {
            destItr->second += cur.second;
        } else {
            dest->second.contents[cur.first] = cur.second;
        }
    }

    // update colony
    Update(true);   // must update and save CC's lastLaunchTime here
    //do we do a full save here, or just update contents and times?
    //Save();
    m_db.UpdatePins(0, ccPin);
    m_db.SaveContents(ccPin);

    // simTime = time to stop (currentSimTime), sourceRunTime = lastRunTime
    PyDict* args = new PyDict();
    /** @todo this needs to be updated to use process times (for next cycle end) */
    args->SetItemString("simTime", new PyLong(GetFileTimeNow() + (EvE::Time::Minute * 15)));  // arbitrary 15 minute run time
    src->second.lastRunTime = 0; //m_procTime;
    args->SetItemString("sourceRunTime", new PyLong(m_procTime));

    /*
     * def GetExpeditedTransferTime(linkBandwidth, commodities):
     *    commodityVolume = GetCommodityTotalVolume(commodities)
     *    return long(math.ceil(max(5 * MIN, float(commodityVolume) / linkBandwidth * HOUR)))
     */
    return args;
}

PyRep* Colony::LaunchCommodities(uint32 pinID, std::map< uint16, uint32 >& items)
{
    // this will export items from CC to jetcan in space.
    // launchpad (Spaceport) xfers items to/from customs office
    std::map<uint32, PI_Pin>::iterator pin = ccPin->pins.find(pinID);
    if (pin == ccPin->pins.end()) {
        _log(COLONY__ERROR, "Colony::LaunchCommodities() - pinID %u not found in ccPin.pins map", pinID);
        return nullptr;
    }

    // first - create jetcan, add to system, and put in orbit around planet
    // NOTE:  PI launches have 5d timers
    /** @todo check capacities before adding items */
    SystemManager* pSysMgr(m_pSE->SystemMgr());
    GPoint location(pSysMgr->GetSE(m_pSE->GetID())->GetPosition());
    location.MakeRandomPointOnSphere(m_pSE->GetRadius() + 2000000);   //2000km orbit for launch can
    ItemData canData(EVEDB::invTypes::PlanetaryLaunchContainer,
                    m_client->GetCharacterID(),  // owner is Character
                    pSysMgr->GetID(),
                    flagNone,
                    "PI Commodities Container",
                    location);

    CargoContainerRef contRef = sItemFactory.SpawnCargoContainer(canData);
    if (contRef.get() == nullptr) {
        contRef->Delete();
        if (m_client->CanThrow())
            throw PyException(MakeCustomError("Unable to spawn item of type %u.", EVEDB::invTypes::PlanetaryLaunchContainer));
    }

    FactionData data = FactionData();
        data.allianceID = m_client->GetAllianceID();
        data.corporationID = m_client->GetCorporationID();
        data.factionID = m_client->GetWarFactionID();
        data.ownerID = m_client->GetCharacterID();
    // create new container SE
    ContainerSE* cSE = new ContainerSE(contRef, *m_svcMgr, pSysMgr, data);
    contRef->SetMySE(cSE);      // item-to-entity internal interface
    //cSE->AnchorContainer();     // avoid GC checks on this container  -no.  has 5d timer set
    pSysMgr->AddEntity(cSE);

    /* second - reduce qtys in source container (CC pin.contents in this case)
     * create actual item (previously only virtual)
     * add to container
     * calculate taxes on items
     * charge char taxes upon launch
     */
    uint8 count = 0;
    double cost = 0;
    for (auto cur : items) {
        std::map<uint16, uint32>::iterator cont = pin->second.contents.find(cur.first);
        if (cont != pin->second.contents.end()) {
            if (cont->second > cur.second) {
                cont->second -= cur.second;
            } else {
                pin->second.contents.erase(cont);
            }
        } else {
            _log(COLONY__WARNING, "Colony::LaunchCommodities() - item %u not found in command center", cur.first);
        }

        //  if item not found in src contents, assume client is right and procede with xfer
        switch (sPIDataMgr.GetProductLevel(cur.first)) {
            case 0:     cost += (    0.15 * cur.second);    break;
            case 1:     cost += (    1.14 * cur.second);    break;
            case 2:     cost += (   13.50 * cur.second);    break;
            case 3:     cost += (  900.00 * cur.second);    break;
            case 4:     cost += (75000.00 * cur.second);    break;
        }
        ItemData iData(cur.first, m_client->GetCharacterID(), locTemp, flagNone, cur.second);
        InventoryItemRef iRef = sItemFactory.SpawnItem(iData);
        if (iRef.get() == nullptr)
            continue;
        // verify we're not overloading container capy
        if (contRef->GetMyInventory()->HasAvailableSpace(flagNone, iRef)) {
            iRef->Move(cSE->GetID());
            iRef->SaveItem();
            ++count;
        } else {
            _log(COLONY__WARNING, "%s: PI Commodity Container %u is full.", m_client->GetName(), contRef->itemID());
            m_client->SendErrorMsg("Your Commodity Container is full.  Some items were not transferred.");
            break;
        }
    }

    if (count)
        if (is_log_enabled(COLONY__TRACE))
            _log(COLONY__TRACE, "Colony::LaunchCommodities() - Launched %u items from command center %u to %s (%u)", \
                        count, m_colonyID, contRef->name(), contRef->itemID() );

    contRef->SaveItem();
    pin->second.lastLaunchTime = GetFileTimeNow();

    // third - create db entry for launch
    m_db.SaveLaunch(contRef->itemID(), m_client->GetCharacterID(), pSysMgr->GetID(), m_pSE->GetID(), location);

    // update colony
    Update(true);   // must update and save CC's lastLaunchTime here
    //do we do a full save here, or just update contents and times?
    //Save();
    m_db.UpdatePins(0, ccPin);
    m_db.SaveContents(ccPin);

    // fourth - take taxes and record entry in journal
    if (cost) {
        //take the money, send wallet blink event record the transaction in their journal.
        std::string reason = "DESC:  Launching PI items from ";
        reason += m_pSE->GetName();
        AccountService::TranserFunds(
                    m_client->GetCharacterID(),
                    corpCONCORD,  // pSysMgr->GetSovHolder(),
                    cost,
                    reason.c_str(),
                    Journal::EntryType::PlanetaryExportTax,
                    m_pSE->GetID(),
                    Account::KeyType::Cash);
    }

    return new PyLong(pin->second.lastLaunchTime);
}

void Colony::PlanetXfer(uint32 spaceportID, std::map< uint32, uint16 > importItems, std::map< uint32, uint16 > exportItems, double taxRate)
{
    // import is from CO to planet.  export is from planet to CO
    // this method will make the transfer of items from real to virtual and back as necessary

    std::map<uint32, PI_Pin>::iterator pin = ccPin->pins.find(spaceportID);
    if (pin == ccPin->pins.end()) {
        _log(COLONY__ERROR, "Colony::PlanetXfer() - pinID %u not found in ccPin.pins map", spaceportID);
        if (m_client->CanThrow())
            throw PyException(MakeCustomError("Your SpacePort on %s was not found.  Ref: ServerError xxxxx.", m_pSE->GetName()));

        return;
    }

    if (pin->second.lastLaunchTime > GetFileTimeNow() + 30 * EvE::Time::Second) { // launch cycle time is 60s
        if (m_client->CanThrow())
            throw PyException(MakeCustomError("Your Launch crew on %s is still recovering from the last launch.", m_pSE->GetName()));

        return;
    }

    uint8 toColony = 0, fromColony = 0;
    double cost = 0;
    InventoryItemRef iRef(nullptr);
    std::map<uint16, uint32>::iterator itr;
    // import
    for (auto cur : importItems) {
        // xfer real item to virtual
        iRef = sItemFactory.GetItem(cur.first);
        if (iRef.get() == nullptr) {
            _log(COLONY__ERROR, "Colony::PlanetXfer():import - itemRef for id %u not found in ItemFactory", cur.first);
            continue;   // should never happen
        }

        itr = pin->second.contents.find(iRef->typeID());
        /** @todo  check for available capy and adjust qty accordingly.
         *       if spaceport cant hold entire xfer qty, xfer to full, and return rest back to CO.
         */
        if (itr != pin->second.contents.end()) {
            itr->second += cur.second;
        } else {
            pin->second.contents[iRef->typeID()] = cur.second;
        }

        switch (sPIDataMgr.GetProductLevel(iRef->typeID())) {
            case 0:     cost += (    0.05 * cur.second);    break;
            case 1:     cost += (    0.38 * cur.second);    break;
            case 2:     cost += (    4.50 * cur.second);    break;
            case 3:     cost += (  300.00 * cur.second);    break;
            case 4:     cost += (25000.00 * cur.second);    break;
        }
        iRef->ToVirtual(spaceportID);
        ++toColony;
    }

    if (toColony)
        if (is_log_enabled(COLONY__TRACE))
            _log(COLONY__TRACE, "Colony::PlanetXfer() - Imported %u items from customs office %u to spaceport %u", \
                            toColony, m_pSE->GetCustomsOffice()->GetID(), spaceportID);

    if (cost) {
        //take the money, send wallet blink event record the transaction in their journal.
        std::string reason = "DESC:  Importing items to ";
        reason += m_pSE->GetName();
        AccountService::TranserFunds(
                            m_client->GetCharacterID(),
                            m_pSE->GetCustomsOffice()->GetOwnerID(),
                            cost,
                            reason.c_str(),
                            Journal::EntryType::PlanetaryImportTax,
                            m_pSE->GetID(),
                            Account::KeyType::Cash);
    }

    // reset cost for possible export taxes
    cost = 0;
    // export
    for (auto cur : exportItems) {
        std::map<uint16, uint32>::iterator cont = pin->second.contents.find(cur.first);
        if (cont != pin->second.contents.end()) {
            if (cont->second > cur.second) {
                cont->second -= cur.second;
            } else {
                // set qty to amount contained in pin.
                cur.second = cont->second;
                pin->second.contents.erase(cont);   // remove item from pin.contents if exporting entire qty.
            }
        } else {
            _log(COLONY__WARNING, "Colony::PlanetXfer():export - item %u not found in spaceport", cur.first);
        }

        //  if item not found in src contents, assume client is right and procede with xfer
        switch (sPIDataMgr.GetProductLevel(cur.first)) {
            case 0:     cost += (    0.10 * cur.second);    break;
            case 1:     cost += (    0.76 * cur.second);    break;
            case 2:     cost += (    9.00 * cur.second);    break;
            case 3:     cost += (  600.00 * cur.second);    break;
            case 4:     cost += (50000.00 * cur.second);    break;
        }
        // xfer virtual item to real
        ItemData iData(cur.first, m_client->GetCharacterID(), locTemp, flagNone, cur.second);
        InventoryItemRef iRef = sItemFactory.SpawnItem(iData);
        iRef->Move(m_pSE->GetCustomsOffice()->GetID(), flagHangar, true);
        ++fromColony;
    }

    if (fromColony)
        if (is_log_enabled(COLONY__TRACE))
            _log(COLONY__TRACE, "Colony::PlanetXfer() - Exported %u items from spaceport %u to customs office %u", \
                        fromColony, spaceportID, m_pSE->GetCustomsOffice()->GetID());

    if (cost) {
        //take the money, send wallet blink event record the transaction in their journal.
        std::string reason = "DESC:  Exporting items from ";
        reason += m_pSE->GetName();
        AccountService::TranserFunds(
                            m_client->GetCharacterID(),
                            m_pSE->GetCustomsOffice()->GetOwnerID(),
                            cost,
                            reason.c_str(),
                            Journal::EntryType::PlanetaryExportTax,
                            m_pSE->GetID(),
                            Account::KeyType::Cash);
    }

    pin->second.lastLaunchTime = GetFileTimeNow();  // launch cycle time is 60s
    pin->second.lastRunTime = GetFileTimeNow();

    // update colony
    Update(true);   // must update and save SP's lastLaunchTime here
    //do we do a full save here, or just update contents and times?
    //Save();
    m_db.UpdatePins(0, ccPin);
    m_db.SaveContents(ccPin);
}

/*  import/export taxes
 *  GetProductLevel(typeID) will return PLevel of item.
 *  use that to calculate cost for import/export operations
Product     Command Center Export Cost  Launchpad Export Cost   Launchpad Import Cost
    P0         15/m3 or .15/unit           10/m3 or .1/unit        5/m3 or .05/unit
    P1          3/m3 or 1.14/unit           2/m3 or .76/unit       1/m3 or .38/unit
    P2          9/m3 or 13.5/unit           6/m3 or 9/unit         3/m3 or 4.5/unit
    P3        150/m3 or 900/unit          100/m3 or 600/unit      50/m3 or 300/unit
    P4        750/m3 or 75k/unit          500/m3 or 50k/unit     250/m3 or 25k/unit
*/

void Colony::PrioritizeRoute(uint16 routeID, uint8 priority)
{
    // set priority level for route...still not sure how to use it
    std::map<uint16, PI_Route>::iterator itr = ccPin->routes.find(routeID);
    if (itr != ccPin->routes.end()) {
        itr->second.priority = priority;
        m_db.SaveRoutes(ccPin);
    }
}


PyTuple* Colony::GetPins()
{
    uint8 index = 0;
    PyTuple* pins = new PyTuple(ccPin->pins.size());

    for (auto cur : ccPin->pins) {
        PyDict* dict = new PyDict();
        dict->SetItem("id", new PyInt(cur.first));
        dict->SetItem("typeID", new PyInt(cur.second.typeID));
        dict->SetItem("ownerID", new PyInt(cur.second.ownerID));
        dict->SetItem("latitude", new PyFloat(cur.second.latitude));
        dict->SetItem("longitude", new PyFloat(cur.second.longitude));
        dict->SetItem("lastRunTime", (cur.second.lastRunTime > 0 ? new PyLong(cur.second.lastRunTime) : PyStatic.NewNone()));
        dict->SetItem("state", new PyInt(cur.second.state));
        dict->SetItem("level", new PyInt(cur.second.level));

        PyDict* contents = new PyDict();
        contents->clear();
        if (cur.second.isStorage)
            for (auto cur2 : cur.second.contents)
                contents->SetItem(new PyInt(cur2.first), new PyInt(cur2.second));
        dict->SetItem("contents", contents);

        if (cur.second.isLaunchable)
            dict->SetItem("lastLaunchTime", (cur.second.lastLaunchTime > 0 ? new PyLong(cur.second.lastLaunchTime) : PyStatic.NewNone()));

        if (cur.second.isProcess)
            if (cur.second.schematicID) {
                dict->SetItem("schematicID", new PyInt(cur.second.schematicID));
                std::map<uint32, PI_Plant>::iterator plantPin = ccPin->plants.find(cur.first);
                if (plantPin != ccPin->plants.end()) {
                    dict->SetItem("cycleTime", new PyLong(plantPin->second.cycleTime));
                    dict->SetItem("hasReceivedInputs", new PyBool(plantPin->second.hasReceivedInputs));
                    dict->SetItem("receivedInputsLastCycle", new PyBool(plantPin->second.receivedInputsLastCycle));
                } else {
                    dict->SetItem("cycleTime", PyStatic.NewZero());
                    dict->SetItem("hasReceivedInputs", PyStatic.NewFalse());
                    dict->SetItem("receivedInputsLastCycle", PyStatic.NewFalse());
                }
            } else {
                dict->SetItem("cycleTime", PyStatic.NewZero());
                dict->SetItem("hasReceivedInputs", PyStatic.NewFalse());
                dict->SetItem("receivedInputsLastCycle", PyStatic.NewFalse());
            }

        if (cur.second.isECU) {
            if (cur.second.installTime > 0) {
                dict->SetItem("cycleTime", new PyFloat(cur.second.cycleTime / EvE::Time::Hour));
                dict->SetItem("expiryTime", new PyLong(cur.second.expiryTime));
                dict->SetItem("headRadius", new PyFloat(cur.second.headRadius));
                dict->SetItem("installTime", new PyLong(cur.second.installTime));
                dict->SetItem("programType", new PyInt(cur.second.programType));
                dict->SetItem("qtyPerCycle", new PyInt(cur.second.qtyPerCycle));
            }
            PyList* list = new PyList();
            list->clear();
            for (auto head : cur.second.heads) {
                PyTuple* tuple = new PyTuple(3);
                    tuple->SetItem(0, new PyInt(head.first));
                    tuple->SetItem(1, new PyFloat(head.second.latitude));
                    tuple->SetItem(2, new PyFloat(head.second.longitude));
                list->AddItem(tuple);
            }
            dict->SetItem("heads", list);
        }

        pins->SetItem(index++, new PyObject("util.KeyVal", dict));
    }
    return pins;
}

PyTuple* Colony::GetLinks()
{
    uint8 index = 0;
    PyTuple* links = new PyTuple(ccPin->links.size());
    for (auto cur : ccPin->links) {
        PyDict* dict = new PyDict();
            dict->SetItem("linkID", new PyInt(cur.first));                 // this is link itemID
            dict->SetItem("endpoint1", new PyInt(cur.second.endpoint1));
            dict->SetItem("endpoint2", new PyInt(cur.second.endpoint2));
            dict->SetItem("level", new PyInt(cur.second.level));
            dict->SetItem("typeID", new PyInt(cur.second.typeID));          // typeID 2280
        links->SetItem(index++, new PyObject("util.KeyVal", dict));
    }
    return links;
}

PyTuple* Colony::GetRoutes()
{
    uint8 index = 0;
    PyTuple* routes = new PyTuple(ccPin->routes.size());

    for (auto cur : ccPin->routes) {
        PyDict* dict = new PyDict();
            dict->SetItem("routeID", new PyInt(cur.first));                 // this is routeID (low number - assigned by client)
            dict->SetItem("commodityTypeID", new PyInt(cur.second.commodityTypeID));
            dict->SetItem("commodityQuantity", new PyInt(cur.second.commodityQuantity));

        PyList* list = new PyList();
        for (auto cur2 : cur.second.path)                               // path of pinIDs this route will follow
            list->AddItem(new PyInt(cur2));
        dict->SetItem("path", list);                                    // list of paths on this route
        routes->SetItem(index++, new PyObject("util.KeyVal", dict));
    }
    return routes;
}

PyRep* Colony::GetColony()
{
    if (m_newHead) {
        for (auto cur : tempECUs) {
            std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(cur);
            if (itr != ccPin->pins.end()) {
                m_db.SaveHeads(m_colonyID, m_client->GetCharacterID(), cur, itr->second.heads);
            } else {
                _log(COLONY__ERROR, "Colony::GetColony()::SaveHeads() - headID %u not found in ccPin.pins map", cur);
            }
        }
        tempECUs.clear();
        m_newHead = false;
    }

    Update();   // update colony before sending data.

    PyDict* args = new PyDict();
        args->SetItem("pins", GetPins());
        args->SetItem("level", new PyInt(ccPin->level));
        args->SetItem("links", GetLinks());
        args->SetItem("routes", GetRoutes());
        args->SetItem("currentSimTime", new PyLong(m_procTime));
    PyObject* res = new PyObject("util.KeyVal", args);

    if (is_log_enabled(COLONY__GC_DUMP)) {
        _log(COLONY__GC_DUMP, "Colony::GetColony() Dump");
        res->Dump(COLONY__GC_DUMP, "    ");
    }

    // reset tempPinID-to-newPinID map after command loop is completed and all new pins have been created.
    tempPinIDs.clear();

    return res;
}

void Colony::Update(bool updateTimes/*false*/)
{
    double profileStartTime = GetTimeUSeconds();

    /* loop thru process calls to update each pin to simulate production and logistics
     *  this will have to be fast, as there may/will be large time deltas between updates
     *  can loop each item to process for each time step (like i do for skill training)
     */

    if (is_log_enabled(COLONY__DEBUG))
        _log(COLONY__DEBUG, "Colony::Update() - Starting Update for colony %u on %s.", m_colonyID, m_pSE->GetName());

    // first, process ecus for raw matls.
    ProcessECUs(updateTimes);
    // second, process plants for production.
    ProcessPlants(updateTimes);
    // third, update plants for matl's received

    // update colony time to current time (based on this update, prior to sending out current colony status)
    m_procTime = GetFileTimeNow();

    // update CommandCenter pin times
    if (updateTimes) {
        std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(m_colonyID);
        if (itr != ccPin->pins.end())
            itr->second.lastRunTime = m_procTime;

        m_toUpdate = true;
    }

    // empty colony runs in 47 - 80 us
    _log(COLONY__INFO, "Colony::Update() - Update completed in %.3fus with %u links, %u pins, %u plants, and %u routes (s:%u, d:%u) ", \
                    GetTimeUSeconds() - profileStartTime, ccPin->links.size(), ccPin->pins.size(), ccPin->plants.size(), ccPin->routes.size(), \
                    m_srcRoutes.size(), m_destRoutes.size());
}

void Colony::ProcessECUs(bool& updateTimes)
{
    double delta = 0;
    uint16 cycles = 0, quantity = 0, amount = 0, count = 0;
    std::map<uint16, uint32>::iterator itemItr;
    std::map<uint32, PI_Pin>::iterator destPin;
    std::map<uint32, PI_Plant>::iterator plant;
    for (auto ecu : ccPin->pins) {
        if (!ecu.second.isECU)
            continue;

        if ((ecu.second.expiryTime < EvE::Time::Second ) or (ecu.second.expiryTime > m_procTime)) {
            if (is_log_enabled(COLONY__DEBUG))
                _log(COLONY__DEBUG, "Colony::ProcessECUs() - expiryTime (%li) > m_procTime (%li).", \
                        ecu.second.expiryTime, m_procTime);
            continue;
        }
        if (ecu.second.cycleTime < 0.0f) {
            if (is_log_enabled(COLONY__DEBUG))
                _log(COLONY__DEBUG, "Colony::ProcessECUs() - cycleTime < 0.");
            continue;
        }

        if (ecu.second.lastRunTime < EvE::Time::Hour)
            ecu.second.lastRunTime = ecu.second.expiryTime - ecu.second.cycleTime;

        /** @todo  as i dont have data on planet resources, and am not tracking depletion, extraction qtys used here are
         * sent from the client during 'survey program' installation, and do not simulate the diminishing returns as shown in
         * the survey program. (testing diminishing returns @ 95%)
         * because of this, the values used here (and all subsquent processes) will be more than shown in client.
         */
        /** @note this is a simple process, as it only provides raw mats, simulating extraction from planet and
         *  shipped to storage or directly to plant for processing.
         * however, in the case of shipping directly to plant, we will have to store the mats in the plant queue
         * and wait for the ProcessPlants() call to use them, as this will avoid overcomplicating things,
         * but it could get messy later....
         */

        // dont loop...get #cycles to 'run' and set amounts accordingly
        delta = (m_procTime - ecu.second.expiryTime) / EvE::Time::Hour;
        cycles = delta / (ecu.second.cycleTime / EvE::Time::Hour);

        /** @todo  verify cycles isnt over program cycle count */

        // first - see if this ecu has a route and move contents per route.  this will simulate aquisition of raw matls from heads to storage
        if (is_log_enabled(COLONY__DEBUG))
            _log(COLONY__DEBUG, "Colony::ProcessECUs() - ECU pin %u - begin processing with %u cycles (%0.2f / %0.2f)", \
                    ecu.first, cycles, delta, (ecu.second.cycleTime / EvE::Time::Hour));
        auto destRouteItr = m_destRoutes.equal_range(plant->first);
        for (auto it = destRouteItr.first; it != destRouteItr.second; ++it) {
            // verify this route begins at this pin.
            //if (it->second.srcPinID != ecu.first)
            //    continue;

            // second - update current contents per route movement as noted above (there are no stored contents to update in the ECU)
            // get route destination pin and update qty
            destPin = ccPin->pins.find(it->first);
            if (destPin == ccPin->pins.end()) {
                _log(COLONY__ERROR, "Colony::ProcessECUs() - Dest pinID %u not found in ccPin.pins map", it->first);
                continue;
            }
            //  loop thru cycles to apply diminishing returns
            count = cycles;
            quantity = it->second.commodityQuantity;
            while (count) {
                quantity *= 0.95;
                amount += quantity;
                --count;
            }
            // contents are stored in each pin.  PI_Pin.contents(std::map<uint16, uint32>(typeID, qty))
            itemItr = destPin->second.contents.find(it->second.commodityTypeID);
            /** @todo  set/implement storage capy for pin - PI_Pin.capacity, PI_Pin.quantity */
            //  if dest cant hold entire xfer qty, drop remainder in current pin contents (as opposed to loss)
            //      will need a way to set/test for this 'extra' material and xfer on next run
            if (itemItr != destPin->second.contents.end()) {
                itemItr->second += amount;
            } else {
                destPin->second.contents[it->second.commodityTypeID] = amount;
            }
            if (is_log_enabled(COLONY__DEBUG))
                _log(COLONY__DEBUG, "Colony::ProcessECUs() - Dest pinID %u updated with %u %s (%u).", \
                        it->second.destPinID, amount, sPIDataMgr.GetProductName(it->second.commodityTypeID), it->second.commodityTypeID);

            // 'update' is part of clever code to avoid db hits.
            //  this will delete existing contents and insert current contents upon completion of processing
            destPin->second.update = true;
            // if destination pin is plant, put materials in its' storage
            // client verifies mat'l is required before routing
            if (destPin->second.isProcess) {
                // find dest's plant data
                plant = ccPin->plants.find(destPin->first);
                if (plant == ccPin->plants.end()) {
                    _log(COLONY__ERROR, "Colony::ProcessECUs() - Plant pinID %u not found in ccPin.plants map", destPin->first);
                    continue;
                }
                //and set hasReceivedInputs to true for subsequent processing
                plant->second.hasReceivedInputs = true;
                if (is_log_enabled(COLONY__DEBUG))
                    _log(COLONY__DEBUG, "Colony::ProcessECUs() - Dest pinID %u isPlant.", it->second.destPinID);
            }
        }

        // third - reset cycle times.
        // set expiryTime to previous runtime plus cycleTime to simulate end of cycle
        ecu.second.expiryTime = ecu.second.lastRunTime + ecu.second.cycleTime * cycles;
        // set lastRunTime to last-processed cycle's expire time.
        ecu.second.lastRunTime = ecu.second.expiryTime;
        updateTimes = true;

        if (is_log_enabled(COLONY__DEBUG))
            _log(COLONY__DEBUG, "Colony::ProcessECUs() - Processing complete.  timeNow %li, expiryTime %li, lastRunTime %li", \
                    GetFileTimeNow(), ecu.second.expiryTime, ecu.second.lastRunTime);
    }
}

void Colony::ProcessPlants(bool& updateTimes)
{
    if (ccPin->plants.empty() or (m_pLevel < 1))
        return; // nothing to do...

    /** @note  generally-accepted PI design has plant input and output to/from silo(spaceport or storage) for input buffers and follows this guideline...
     * silo->plant->silo->plant->silo
     * however, there may be rare cases where colony is restricted or other design constraints limit routing and
     * plants must be linked together, where the output of one provides the direct input of the next, as follows...
     * silo->plant->plant->plant->silo
     * with plants as needed for production requirements of the colony.
     *
     * this will need to check for and be able to process both cases, and could be somewhat complicated.
     *
     * plants will have to be processed in order from p1 to p4 products, to provide input for downstream plants
     * this WILL have to loop for each cycle to correctly set inputs and outputs for each plant, and provide
     * positive material control (and be more realistic) per run.
     */

    /** @note  plants are stored separate from other pins, to avoid the cycles and checks for plants in this call.
     * this will also avoid the unnecessary plant-specific data to be stored in std pins for all items
     */
    // m_pLevel is set to lowest 'P' level of produced items, and used to properly order plant processing streams
    uint8 curCycle = m_pLevel;
    int32 cycles = 0, cycles2 = 0, amount = 0, divisor = 0, delta = 0;
    std::map<uint32, PI_Pin>::iterator srcPin;
    std::map<uint32, PI_Pin>::iterator destPin;
    std::map<uint16, uint32>::iterator itemItr;
    std::map<uint32, PI_Plant>::iterator destPlant;
    _log(COLONY__INFO, "Colony::ProcessPlants() - Begin Plant Processing.  m_procTime: %li", m_procTime);
    while (curCycle < 5) {
        if (is_log_enabled(COLONY__DEBUG))
            _log(COLONY__DEBUG, "Colony::ProcessPlants() - Begin Process loop for pLevel %u.", curCycle);

        // plants must be processed in order to correctly make products and send to downstream recipients.
        // this allows for both silo->plant->silo->plant and silo->plant->plant->plant->silo routing (or any combination of plant and silo routing)
        auto cycleItr = m_plantMap.equal_range(curCycle);
        for (auto it = cycleItr.first; it != cycleItr.second; ++it) {
            _log(COLONY__INFO, "Colony::ProcessPlants() - Begin Processing for Plant %u", it->second);

            // first, find plant pin in plant map
            std::map<uint32, PI_Plant>::iterator plant = ccPin->plants.find(it->second);
            if (plant == ccPin->plants.end()) {
                _log(COLONY__ERROR, "Colony::ProcessPlants() - Plant %u not found in ccPin.pins map", it->second);
                it = m_plantMap.erase(it);
                continue;
            }
            // plant pin found.  begin basic data integrity  checks

            // check if plant has schematic installed
            if (plant->second.schematicID == 0) {
                _log(COLONY__WARNING, "Colony::ProcessPlants() - No schematic installed in plant %u.", it->second);
                plant->second.hasReceivedInputs = false;
                plant->second.receivedInputsLastCycle = false;
                continue;
            }
            /** if schematicID is valid, input and cycletime *should* be valid also.
             * comment these until proven needed  (gotta get proc times faster)
            // check if plant has valid input map
            if (plant->second.data.inputs.empty()) {
                _log(COLONY__WARNING, "Colony::ProcessPlants() - Empty input map");
                plant->second.hasReceivedInputs = false;
                plant->second.receivedInputsLastCycle = false;
                continue;
            }
            if (plant->second.cycleTime == 0) {
                if (is_log_enabled(COLONY__DEBUG))
                    _log(COLONY__DEBUG, "Colony::ProcessPlants() - cycleTime = 0.");
                continue;
            } */

            if (plant->second.lastRunTime < EvE::Time::Hour)
                plant->second.lastRunTime = m_procTime + EvE::Time::Second;

            if (plant->second.lastRunTime >= m_procTime) {
                if (is_log_enabled(COLONY__DEBUG))
                    _log(COLONY__DEBUG, "Colony::ProcessPlants() - lastRunTime (%li) >= m_procTime (%li).", \
                                plant->second.lastRunTime, m_procTime);
                continue;
            }

            if (is_log_enabled(COLONY__DEBUG))
                _log(COLONY__DEBUG, "Colony::ProcessPlants() - last run time %li.", plant->second.lastRunTime);

            // second, check processing times for active plants
            delta = (m_procTime - plant->second.lastRunTime)  / EvE::Time::Second;
            divisor = (plant->second.cycleTime / EvE::Time::Second);
            if (divisor > 0) {
                if (delta < divisor) {
                    if (plant->second.state < PI::Pin::State::Active) {
                        delta = divisor;
                        plant->second.lastRunTime -= plant->second.cycleTime;
                    } else {
                        if (is_log_enabled(COLONY__DEBUG))
                            _log(COLONY__DEBUG, "Colony::ProcessPlants() - cycle incomplete (%i < %i).", delta, divisor);
                        continue;
                    }
                }
            } else {
                if (is_log_enabled(COLONY__DEBUG))
                    _log(COLONY__DEBUG, "Colony::ProcessPlants() - divisor < 0 (%i).", divisor);
                continue;
            }
            // we are doing 'batch' cycles here.  get #cycles completed based on proc times
            cycles = delta / divisor;
            if (is_log_enabled(COLONY__DEBUG))
                _log(COLONY__DEBUG, "Colony::ProcessPlants() - current cycle count is %i (%i / %i).", \
                            cycles, delta, divisor);

            if (cycles < 1) {
                _log(COLONY__WARNING, "Colony::ProcessPlants() - Cycle count < 1");
                continue;
            }
            // basic data checks done.

            // third, check supply routes for available matls and xfer to this plant
            _log(COLONY__INFO, "Colony::ProcessPlants() - Begin Input Route loop for Plant %u.", plant->first);
            auto destRouteItr = m_destRoutes.equal_range(plant->first);
            for (auto it = destRouteItr.first; it != destRouteItr.second; ++it) {
                // we are ONLY checking routes TO this plant.
                //if (it->second.destPinID != plant->first)
                //    continue;

                // verify plant needs this input....this may be overkill...
                /** overkill...client verifies plant routing before sending to server
                if (plant->second.data.inputs.find(it->second.commodityTypeID) == plant->second.data.inputs.end()) {
                    _log(COLONY__ERROR, "Colony::ProcessPlants() - Routed Commodity %s (%u) not found in input map",\
                            sPIDataMgr.GetProductName(it->second.commodityTypeID), it->second.commodityTypeID);
                    plant->second.lastRunTime = m_procTime;
                    plant->second.hasReceivedInputs = false;
                    plant->second.receivedInputsLastCycle = false;
                    updateTimes = true;
                    // remove route here
                    continue;
                } */
                // this route supplies this plant with input matls.
                srcPin = ccPin->pins.find(it->second.srcPinID);
                if (srcPin == ccPin->pins.end()) {
                    _log(COLONY__ERROR, "Colony::ProcessPlants() - Source %u not found in ccPin.pins map", it->second.srcPinID);
                    plant->second.lastRunTime = m_procTime;
                    plant->second.hasReceivedInputs = false;
                    plant->second.receivedInputsLastCycle = false;
                    updateTimes = true;
                    continue;
                }
                // verify supplier is NOT a plant or ECU here, as this was done in previous cycle checks.
                if (!srcPin->second.isStorage)
                    continue;
                // source is storage.  continue with processing
                itemItr = srcPin->second.contents.find(it->second.commodityTypeID);
                if (itemItr == srcPin->second.contents.end()) {
                    if (is_log_enabled(COLONY__DEBUG))
                        _log(COLONY__DEBUG, "Colony::ProcessPlants() - Routed Commodity %s (%u) not found in Source Inventory.", \
                                sPIDataMgr.GetProductName(it->second.commodityTypeID), it->second.commodityTypeID);
                    continue;// this material wasnt found in source container....cant move what we aint got..
                }

                // remove contents from storage pin
                amount = it->second.commodityQuantity * cycles;
                if (itemItr->second > amount) {
                    itemItr->second -= amount;
                } else {
                    amount = itemItr->second;
                    srcPin->second.contents.erase(itemItr);
                }
                if (is_log_enabled(COLONY__DEBUG))
                    _log(COLONY__DEBUG, "Colony::ProcessPlants() - Removed %i %s (%u) from src %u.", \
                            amount, sPIDataMgr.GetProductName(it->second.commodityTypeID), it->second.commodityTypeID, srcPin->first);

                // update contents of storage pin
                srcPin->second.update = true;

                // add contents to this plant's pin
                destPin = ccPin->pins.find(plant->first);
                if (destPin == ccPin->pins.end()) {
                    _log(COLONY__ERROR, "Colony::ProcessPlants() - Pin %u not found in ccPin.pins map for this plant.", plant->first);
                    plant->second.lastRunTime = m_procTime;
                    plant->second.hasReceivedInputs = false;
                    plant->second.receivedInputsLastCycle = false;
                    updateTimes = true;
                    continue;
                }
                itemItr = destPin->second.contents.find(it->second.commodityTypeID);
                if (itemItr != destPin->second.contents.end()) {
                    itemItr->second += amount;
                } else {
                    destPin->second.contents[it->second.commodityTypeID] = amount;
                }
                destPin->second.update = true;

                // we have received a material from this route. enable check for all required materials in this Schematic for this plant
                plant->second.hasReceivedInputs = true;

                if (is_log_enabled(COLONY__DEBUG))
                    _log(COLONY__DEBUG, "Colony::ProcessPlants() - Added %i %s (%u) to Plant Inventory.", \
                            amount, sPIDataMgr.GetProductName(it->second.commodityTypeID), it->second.commodityTypeID);
            }
            // current plant has received product as defined by routing info

            // fourth, process input material requirements
            cycles2 = 0;
            _log(COLONY__INFO, "Colony::ProcessPlants() - %s Input Check loop for Plant %u.", plant->second.hasReceivedInputs ? "Begin" : "Skipping", plant->first);
            if (is_log_enabled(COLONY__DEBUG))
                _log(COLONY__DEBUG, "Colony::ProcessPlants() - Plant %u processing inputs.", plant->first);

            destPin = ccPin->pins.find(plant->first);
            if (destPin == ccPin->pins.end()) {
                _log(COLONY__ERROR, "Colony::ProcessPlants() - Dest %u not found in ccPin.pins map", plant->first);
                continue;
            }

            plant->second.receivedInputsLastCycle = false;

            if (plant->second.hasReceivedInputs) {
                /*  if plant has received mats from routing (above), then check here for required qtys per Schematic.
                 *     input data is found in plant->second.data.inputs map (std::map<uint16, uint16> {typeID, qty})
                 *
                 *  if required mats are not present, set receivedInputsLastCycle=false, which will deny processing
                 *      and subsquent routing for this plant.
                 *
                 *  if all required qtys have been received, procede with the following:
                 *   - remove mats from pin.contents
                 *   - set receivedInputsLastCycle=true
                 */
                if (plant->second.data.inputs.empty()) {
                    _log(COLONY__WARNING, "Colony::ProcessPlants() - Empty input map");
                    plant->second.hasReceivedInputs = false;
                    plant->second.lastRunTime = m_procTime;
                    updateTimes = true;
                    continue;
                }
                uint16 tempCycles = cycles;
                for (auto mats : plant->second.data.inputs) {
                    // loop thru Schematic inputs to verify all required mats are present
                    itemItr = destPin->second.contents.find(mats.first);
                    if (itemItr == destPin->second.contents.end()) {
                        if (is_log_enabled(COLONY__DEBUG))
                            _log(COLONY__DEBUG, "Colony::ProcessPlants() - %s (%u) not found in Plant Inventory.", \
                                sPIDataMgr.GetProductName(mats.first), mats.first);
                        // this required material was not found in plant inventory.  skip further processing
                        plant->second.state = PI::Pin::State::Idle;
                        plant->second.lastRunTime = m_procTime;
                        updateTimes = true;
                        continue;
                    }
                    if (itemItr->second >= mats.second * cycles) {
                        itemItr->second -= mats.second * cycles;
                        plant->second.receivedInputsLastCycle = true;
                    } else {
                        // this required material was not sufficient quantity for (num cycles) runs.
                        // determine how many cycles we can run with current material quantity
                        if (is_log_enabled(COLONY__DEBUG))
                            _log(COLONY__DEBUG, "Colony::ProcessPlants() - Not enough %s (%u) for %i cycles.  Need %u, Have %u", \
                                    sPIDataMgr.GetProductName(mats.first), mats.first, cycles, mats.second * cycles, itemItr->second);
                        cycles2 = itemItr->second / mats.second;
                        if (cycles2 > 0) {
                            itemItr->second -= mats.second * cycles2;
                            plant->second.receivedInputsLastCycle = true;
                            if (is_log_enabled(COLONY__DEBUG))
                                _log(COLONY__DEBUG, "Colony::ProcessPlants() - Have enough material for %i cycles.", cycles2);
                        } else {
                            plant->second.lastRunTime = m_procTime;
                            plant->second.state = PI::Pin::State::Idle;
                            updateTimes = true;
                            continue;
                        }
                    }
                    // set temp variable with mininmum cycle count
                    if (tempCycles > cycles2)
                        tempCycles = cycles2;
                    cycles2 = 0;
                }
                // we have enough mat'l for at least on process.  set cycles based on material in inventory.
                if (cycles > tempCycles)
                    cycles = tempCycles;    // temp variable not longer needed at this point.
            } else {
                plant->second.lastRunTime = m_procTime;
                updateTimes = true;
            }

            // at this point, we have looped thru all required mats and set plant variables accordingly.

            // fifth, process manufacturing cycle
            _log(COLONY__INFO, "Colony::ProcessPlants() - %s manufacturing loop for Plant %u.", plant->second.receivedInputsLastCycle ? "Begin" : "Skipping", plant->first);
            if (plant->second.receivedInputsLastCycle and (cycles > 0)) {
                /* plant has received all required mats for production.
                 * set timers for runtimes and state to active
                 * this will allow routing (and subsquent process checks) on next loop, as defined in beginning of this loop
                 */
                if (is_log_enabled(COLONY__DEBUG))
                    _log(COLONY__DEBUG, "Colony::ProcessPlants() - Updating timers for %i cycles using current inventory.", cycles);

                plant->second.state = PI::Pin::State::Active;
                updateTimes = true;

                if (is_log_enabled(COLONY__DEBUG))
                    _log(COLONY__DEBUG, "Colony::ProcessPlants() - Received Inputs.  timeNow %li, lastRunTime %li", \
                                GetFileTimeNow(), plant->second.lastRunTime);
            } else {
                cycles = 0;
            }

            // sixth, manufacturing complete.  move finished product per route
            _log(COLONY__INFO, "Colony::ProcessPlants() - %s Output Routing loop for Plant %u.", cycles > 0 ? "Begin" : "Skipping", plant->first);
            if (cycles) {
                // at this point, *SOMETHING* has changed in this plant, so send it to update
                destPin->second.update = true;
                auto srcRouteItr = m_srcRoutes.equal_range(plant->first);
                for (auto it = srcRouteItr.first; it != srcRouteItr.second; ++it) {
                    // verify this route begins at this plant.  should be only ONE route here for this output
                    //if (route.second.srcPinID != plant->first)
                    //    continue;
                    // get destination pin and update qty there for this round
                    destPin= ccPin->pins.find(it->second.destPinID);
                    if (destPin == ccPin->pins.end()) {
                        _log(COLONY__ERROR, "Colony::ProcessPlants() - Dest %u not found in ccPin.pins map", it->second.destPinID);
                        continue;
                    }
                    // contents are stored in each pin.  PI_Pin.contents(std::map<uint16, uint32> typeID, qty)
                    // we have plant cycles for this loop, so multiply output by cycles to get a total to simulate the "active" plant
                    amount = it->second.commodityQuantity * cycles;
                    itemItr = destPin->second.contents.find(it->second.commodityTypeID);
                    if (itemItr != destPin->second.contents.end()) {
                        itemItr->second += amount;
                    } else {
                        destPin->second.contents[it->second.commodityTypeID] = amount;
                    }
                    if (is_log_enabled(COLONY__DEBUG))
                        _log(COLONY__DEBUG, "Colony::ProcessPlants() - Added %u %s (%u) to Dest %u.", \
                                amount, sPIDataMgr.GetProductName(it->second.commodityTypeID), it->second.commodityTypeID, it->second.destPinID);

                    if (destPin->second.isStorage) {
                        /** @todo  set/implement storage capy for pin - PI_Pin.capacity, PI_Pin.quantity */
                        //  if dest cant hold entire xfer qty, drop remainder in current pin contents (as opposed to loss)
                    } else if (destPin->second.isProcess) {
                        // find dest's plant data
                        //  the destination plant will have a P level of curCycle+1, and will process on next iteration
                        destPlant = ccPin->plants.find(destPin->first);
                        if (destPlant == ccPin->plants.end()) {
                            _log(COLONY__ERROR, "Colony::ProcessPlants() - Dest %u not found in ccPin.plants map", destPin->first);
                            continue;
                        }
                        //and set hasReceivedInputs to true for subsquent processing
                        destPlant->second.hasReceivedInputs = true;
                    }

                    destPin->second.update = true;
                    plant->second.hasReceivedInputs = false;
                }
                // update last run time based on current process cycles
                plant->second.lastRunTime += plant->second.cycleTime * cycles;
            } else {
                plant->second.state = PI::Pin::State::Idle;
                plant->second.lastRunTime = m_procTime;
                plant->second.hasReceivedInputs = false;
                plant->second.receivedInputsLastCycle = false;
            }
        }
        if (is_log_enabled(COLONY__DEBUG))
            _log(COLONY__DEBUG, "Colony::ProcessPlants() - Process loop complete for pLevel %u.", curCycle);
        ++curCycle;
    }
}
