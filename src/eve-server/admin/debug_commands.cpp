

#include <stdio.h>
#include "eve-server.h"

#include "PyBoundObject.h"
#include "Client.h"
#include "ConsoleCommands.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "admin/AllCommands.h"
#include "admin/CommandDB.h"
#include "fleet/FleetService.h"
#include "inventory/AttributeEnum.h"
#include "inventory/InventoryDB.h"
#include "inventory/InventoryItem.h"
#include "manufacturing/Blueprint.h"
#include "npc/Drone.h"
#include "planet/Moon.h"
#include "station/Station.h"
#include "system/Damage.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "system/cosmicMgrs/DungeonMgr.h"
#include "testing/test.h"


PyResult Command_siglist(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to test anomaly system   -allan 21Feb15
     *   will list all anomalies, by systemID.
     */

    AnomalyMgr* pAM = pClient->SystemMgr()->GetAnomMgr();
    std::vector<CosmicSignature> sig;
    pAM->GetAnomalyList(sig);
    pAM->GetSignatureList(sig);

    int count = sig.size();
    std::ostringstream str;
    str.clear();
    str << "There are currently %u active signals in %s(%u)<br>"; //80
    str << "aID iID bubbleID type 'Name'<br>"; //30

    for (auto sigs : sig) {
        // sysSignatures (sigID,sigItemID,dungeonType,sigName,systemID,sigTypeID,sigGroupID,scanGroupID,scanAttributeID,x,y,z)
        str << sigs.sigID.c_str() << " "  << sigs.sigItemID << " " << sigs.bubbleID << " " << pAM->GetScanGroupName(sigs.scanGroupID) << " '" << sigs.sigName.c_str() << "'<br>"; //120
    }

    int size(120);
    size += count * 120;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), count, pClient->SystemMgr()->GetName(), pClient->SystemMgr()->GetID());

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_heal(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount()== 1) {
        pClient->GetShip()->Heal();
    } else if (args.argCount() == 2) {
        if (!args.isNumber(1))
            throw CustomError ("Argument 1 should be a character ID");

        uint32 entity = atoi(args.arg(1).c_str());

        Client *target = sEntityList.FindClientByCharID(entity);
        if (target == NULL)
            throw CustomError ("Cannot find Character by the entityID %d", entity);

        target->GetShip()->Heal();
    }

    return(new PyString("Heal successful!"));
}

PyResult Command_healtarget(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount()== 1) {
        pClient->GetShip()->Heal();
    } else if (args.argCount() == 2) {
        if (!args.isNumber(1))
            throw CustomError ("Argument 1 should be a character ID");

        uint32 entity = atoi(args.arg(1).c_str());

        Client *target = sEntityList.FindClientByCharID(entity);
        if (target == NULL)
            throw CustomError ("Cannot find Character by the entityID %d", entity);

        target->GetShip()->Heal();
    }

    return(new PyString("Heal successful!"));
}

PyResult Command_status(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    //if (!pClient->IsInSpace())
    //    throw CustomError ("You're not in space.");
    if (!pClient->GetShipSE()->SysBubble())
        pClient->EnterSystem(pClient->GetSystemID());
    if (!pClient->GetShipSE()->DestinyMgr())
        pClient->SetDestiny(NULL_ORIGIN);

    ShipItem* pShip = pClient->GetShip().get();

    char reply[150];
    snprintf(reply, 150,
             "PG: %.2f(%.3f)<br>" //25
             "Cap: %.2f(%.3f)<br>" //28
             "CPU: %.2f(%.3f)<br>" //28
             "Hull: %.2f(%.3f)<br>" //32
             "Armor: %.2f(%.3f)<br>" //27
             "Shield: %.2f(%.3f)", //28
             pShip->GetShipPGLevel(), pShip->GetShipPGPercent().get_float(),
             pShip->GetShipCapacitorLevel(), pShip->GetShipCapacitorPercent().get_float(),
             pShip->GetShipCPULevel(), pShip->GetShipCPUPercent().get_float(),
             pShip->GetShipHullHP(), pShip->GetShipHullPercent().get_float(),
             pShip->GetShipArmorHP(), pShip->GetShipArmorPercent().get_float(),
             pShip->GetShipShieldHP(), pShip->GetShipShieldPercent().get_float()
    );

    pClient->SendInfoModalMsg(reply);

    return new PyString(reply);
}

PyResult Command_list(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug system entities
     * wip.   -allan 25Apr15    -UD 15July19
     */

    if (!pClient->IsInSpace())
        throw CustomError ("You must be in space to list system inventory.");

    if (!pClient->GetShipSE()->SysBubble())
        if (pClient->IsInSpace())
            pClient->EnterSystem(pClient->GetSystemID());
        else
            throw CustomError ("You must be in space to list system inventory.");

    SystemManager* pSys = pClient->GetShipSE()->SystemMgr();
    uint16 beltCount = pSys->BeltCount();
    uint32 roidSpawns = pSys->GetRoidSpawnCount();
    uint32 ratSpawns = pSys->GetRatSpawnCount();
    uint32 npcs = pSys->GetSysNPCCount();
    uint32 players = pSys->PlayerCount();
    uint32 bubbles = sBubbleMgr.GetBubbleCount(pSys->GetID());

    std::map<uint32, SystemEntity*> into = pSys->GetEntities();

    std::ostringstream str;
    str.clear();
    str << "System: %s(%u)<br>"; //42
    str << "Belts: %u<br>"; //20
    str << "Bubbles: %u<br>"; //25
    str << "RoidSpawns: %u<br>"; //25
    str << "RatSpawns: %u<br>"; //18
    str << "Players: %u<br>"; //23
    str << "<br>"; //5

    for (auto cur : into) {
        if (cur.second == nullptr)
            continue;
        str << cur.first << ": ";
        std::string global = "(non-global)";
        if (cur.second->isGlobal())
            global = "(global)";
        str << global.c_str();
        if (cur.second->SysBubble() != nullptr)
            str << " bubbleID: " << cur.second->SysBubble()->GetID() << "  "; // 13 + 27 + 40 for name (80)
        else
            str << " (no bubble)  "; // 13 + 27 + 40 for name (80)
        if (cur.second->DestinyMgr() != nullptr) {
            std::string modeStr = "Rigid";
            if (!cur.second->IsStaticEntity()) {
                switch (cur.second->DestinyMgr()->GetState()) {
                    case 0: modeStr = "Goto"; break;
                    case 1: modeStr = "Follow"; break;
                    case 2: modeStr = "Stop"; break;
                    case 3: modeStr = "Warp"; break;
                    case 4: modeStr = "Orbit"; break;
                    case 5: modeStr = "Missile"; break;
                    case 6: modeStr = "Mushroom"; break;
                    case 7: modeStr = "Boid"; break;
                    case 8: modeStr = "Troll"; break;
                    case 9: modeStr = "Miniball"; break;
                    case 10: modeStr = "Field"; break;
                    case 11: modeStr = "Rigid"; break;
                    case 12: modeStr = "Formation"; break;
                }
            }

            str << modeStr.c_str() << " (csf: " << cur.second->DestinyMgr()->GetSpeedFraction() << ") speed: ";
            str << cur.second->DestinyMgr()->GetSpeed() << " [" << cur.second->GetName() << "]<br>"; // 13 + 27 + 40 for name (80)
        } else
            str << " [" << cur.second->GetName() << "]<br>"; // 13 + 27 + 40 for name (80)
    }

    int count = into.size();
    int size = count * 90;
    size += 130;    // header
    char reply[size];
    snprintf(reply, size, str.str().c_str(), pSys->GetName(), pSys->GetID(), beltCount, bubbles, roidSpawns, ratSpawns, npcs, players);

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_bubblelist(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug bubble entities
     * wip.   -allan 2June16
     */

    if (pClient->IsDocked())
        throw CustomError ("You must be in space to list bubble inventory.");

    SystemBubble *b = pClient->GetShipSE()->SysBubble();
    uint32 bubble = b->GetID();
    uint32 dynamics = b->CountDynamics();
    uint32 npcs = b->CountNPCs();
    uint32 players = b->CountPlayers();

    std::map<uint32, SystemEntity*> into;
    b->GetEntities(into);

    std::ostringstream str;
    str.clear();
    str << "Bubble: %u<br>"; //22
    str << "Dynamics: %u<br>"; //19
    str << "NPCs: %u<br>"; //18
    str << "Players: %u<br>"; //23
    str << "<br>"; //5

    for (auto cur : into) {
        if (cur.second == nullptr)
            continue;
        if (cur.second->DestinyMgr() != nullptr) {
            std::string modeStr = "Rigid";
            if (!cur.second->IsStaticEntity()) {
                switch (cur.second->DestinyMgr()->GetState()) {
                    case 0: modeStr = "Goto"; break;
                    case 1: modeStr = "Follow"; break;
                    case 2: modeStr = "Stop"; break;
                    case 3: modeStr = "Warp"; break;
                    case 4: modeStr = "Orbit"; break;
                    case 5: modeStr = "Missile"; break;
                    case 6: modeStr = "Mushroom"; break;
                    case 7: modeStr = "Boid"; break;
                    case 8: modeStr = "Troll"; break;
                    case 9: modeStr = "Miniball"; break;
                    case 10: modeStr = "Field"; break;
                    case 11: modeStr = "Rigid"; break;
                    case 12: modeStr = "Formation"; break;
                }
            }
            str << cur.first;
            if (cur.second->isGlobal())
                str << ": (global) ";
            else
                str << ": ";
            str << modeStr.c_str() << " (csf: " << cur.second->DestinyMgr()->GetSpeedFraction() << ") speed: ";
            str << cur.second->DestinyMgr()->GetSpeed() << " [" << cur.second->GetName() << "]<br>"; // 13 + 27 + 40 for name (80)
        } else {
            str << cur.first << ": None (csf: 0) speed: 0 [" << cur.second->GetName() << "]<br>"; // 13 + 27 + 40 for name (80)
        }
    }

    int count = into.size();
    int size = count * 80;
    size += 90;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), bubble, dynamics, npcs, players);

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_commandlist(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /*
     * this command will send the client a list of loaded game commands, role required, and description.  -allan 23May15
     */

    char reply[65];
    snprintf(reply, 65,
             "Working on making this list...check back later.<br>" //53
             " -Allan"); //9

    pClient->SendInfoModalMsg(reply);

    return new PyString(reply);
}


PyResult Command_secstatus(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /*
     * this command will send the client the security status of the current Character.  -allan 5July15
     */

    char reply[65];
    snprintf(reply, 65,
             "SecStatus: %f.", pClient->GetSecurityRating()); //53

    pClient->SendInfoModalMsg(reply);

    return new PyString(reply);
}

PyResult Command_destinyvars(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!pClient->IsInSpace())
        throw CustomError ("You're not in space.  This call needs DestinyMgr.");
    if (!pClient->GetShipSE()->SysBubble())
        pClient->EnterSystem(pClient->GetSystemID());
    if (!pClient->GetShipSE()->DestinyMgr())
        pClient->SetDestiny(NULL_ORIGIN);

    DestinyManager* dm = pClient->GetShipSE()->DestinyMgr();

    char reply[250];
    snprintf(reply, 250,
             "ShipID: %u<br>"
             "IsCloaked: %u<br>" //28
             "IsWarping: %u<br>" //27
             "InPod: %u<br>" //27
             "IsInSpace: %u<br>" //27
             "IsDocked: %u<br>" //27
             "IsJump: %u<br>" //27
             "IsInvul: %u<br>" //27
             "IsLogin: %u<br>" //27
             "IsUndock: %u<br>" //27
             "HasBeyonce: %u<br>" //27
             "IsBubbleWait: %u<br>" //27
             "IsSetStateSent: %u<br>", //27
                pClient->GetShipID(), dm->IsCloaked(), dm->IsWarping(), pClient->InPod(), pClient->IsInSpace(), pClient->IsDocked(), pClient->IsJump(),
                pClient->IsInvul(), pClient->IsLogin(),  pClient->IsUndock(), pClient->HasBeyonce(), pClient->IsBubbleWait(), pClient->IsSetStateSent()
            );

    pClient->SendInfoModalMsg(reply);

    return new PyString(reply);
}

PyResult Command_shipvars(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!pClient->IsInSpace())
        throw CustomError ("You're not in space.");
    if (!pClient->GetShipSE()->SysBubble())
        pClient->EnterSystem(pClient->GetSystemID());
    if (!pClient->GetShipSE()->DestinyMgr())
        pClient->SetDestiny(NULL_ORIGIN);

    DestinyManager* dm = pClient->GetShipSE()->DestinyMgr();
    GPoint heading(dm->GetHeading());

    char reply[300];
    snprintf(reply, 300,
             "Destiny Variable List for %s<br><br>" //60
             "ShipID: %u<br>"
             "Mass: %.2f<br>" //28
             "AlignTime: %.2f<br>" //27
             "AccelTime: %.2f<br>"
             "MaxSpeed: %.2f<br>" //27
             "WarpSpeed: %.2f<br>" //27
             "WarpTime: %.2f<br>" //27
             "WarpDropSpeed: %.2f<br>" //27
             "Radius: %.2f<br>" //27
             "CapNeed: %.2f<br>" //27
             "Agility: %.3f<br>" //27
             "Inertia: %.3f<br>" //27
             "Heading: %.3f,%.3f,%.3f<br>", //21
                pClient->GetShipSE()->GetName(), pClient->GetShipID(), dm->GetMass(), dm->GetAlignTime(),
             dm->GetAccelTime(), dm->GetMaxVelocity(), (float)(dm->GetWarpSpeed() /10), dm->GetWarpTime(),
             dm->GetWarpDropSpeed(), dm->GetRadius(), dm->GetCapNeed(), dm->GetAgility(), dm->GetInertia(),
             heading.x, heading.y, heading.z
            );

    pClient->SendInfoModalMsg(reply);

    return new PyString(reply);
}

PyResult Command_halt(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!pClient->IsInSpace())
        throw CustomError ("You're not in space.");
    if (!pClient->GetShipSE()->SysBubble())
        pClient->EnterSystem(pClient->GetSystemID());
    if (!pClient->GetShipSE()->DestinyMgr())
        pClient->SetDestiny(NULL_ORIGIN);

    pClient->GetShipSE()->DestinyMgr()->Halt();

    char reply[25];
    snprintf(reply, 25,
             "Ship Halted.");

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_shutdown(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* ingame command to immediatly save loaded items and halt server.
     */
    sConsole.HaltServer();
    return nullptr;
}

PyResult Command_beltlist(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug asteroid creation/management
     * wip.   -allan 15April16
     */

    std::vector<AsteroidSE*> invMap;
    invMap.clear();
    uint32 beltID = sBubbleMgr.GetBeltID(pClient->GetShipSE()->SysBubble()->GetID());
    BeltMgr* belt = pClient->GetShipSE()->SystemMgr()->GetBeltMgr();
    belt->GetList(beltID, invMap);

    std::ostringstream str;
    str.clear();
    str << "BeltID %u has %u roids in it.<br><br>"; //40

    for (auto cur : invMap)
        str << cur->GetName() << ": " << cur->GetID() << "<br>"; // 20 + 40 for name (60)

    int count = invMap.size();
    int size = count * 60;
    size += 50;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), beltID, count);

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_inventory(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug inventory
     * wip.   -allan 15Mar16
     */

    std::map<uint32, InventoryItemRef> invMap;
    invMap.clear();

    InventoryItem* item(nullptr);
    Inventory* inv(nullptr);
    uint32 inventoryID = 0;
    if (pClient->IsDocked()) {
        inventoryID = pClient->GetStationID();
        StationItemRef station = sEntityList.GetStationByID(inventoryID);
        if (station.get() == nullptr)
            throw CustomError ("Cannot find Station Reference for stationID %u", inventoryID);
        inv = station->GetMyInventory();
        if (inv == nullptr)
            throw CustomError ("Cannot find inventory for locationID %u", inventoryID);
        inv->GetInventoryMap(invMap);
        item = station.get();
    } else {
        //Command_list(pClient,db,services,args);
        inventoryID = pClient->GetSystemID();
        SolarSystemRef system = sItemFactory.GetSolarSystem(inventoryID);
        if (system.get() == nullptr)
            throw CustomError ("Cannot find System Reference for systemID %u", inventoryID);
        inv = system->GetMyInventory();
        if (inv == nullptr)
            throw CustomError ("Cannot find inventory for locationID %u", inventoryID);
        inv->GetInventoryMap(invMap);
        item = system.get();
    }

    std::ostringstream str;
    str.clear();
    str << "%s<br>";
    str << "InventoryID %u(%p) (Item %p) has %u items.<br><br>"; //70

    if (IsSolarSystemID(inventoryID)) {
        SystemEntity* pSE(nullptr);
        SystemManager* sMgr = pClient->SystemMgr();
        for (auto cur : invMap) {
            pSE = sMgr->GetEntityByID(cur.first);
            str << cur.first << "(" << sDataMgr.GetFlagName(cur.second->flag()) << ")[" << pSE->SysBubble()->GetID() << "]: " << cur.second->itemName() << "<br>"; // 20 + 70 for name (90)
        }
    } else {
        for (auto cur : invMap)
            str << cur.first << "(" << sDataMgr.GetFlagName(cur.second->flag()) << "): " << cur.second->itemName() << "<br>"; // 20 + 70 for name (90)
    }

    int count = invMap.size();
    int size = count * 90;
    size += 70;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), item->name(), inventoryID, inv, item, count);

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_shipinventory(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug inventory
     * wip.   -allan 15Mar16
     */

    std::map<uint32, InventoryItemRef> invMap;
    invMap.clear();
    uint32 inventoryID = pClient->GetShipID();
    ShipItemRef ship = sItemFactory.GetShip(inventoryID);
    Inventory* inv = ship->GetMyInventory();
    inv->GetInventoryMap(invMap);

    std::ostringstream str;
    str.clear();
    str << "%s<br>"; //40
    str << "InventoryID %u(%p) (Ship %p) has %u items.<br><br>"; //50

    // update to sort by slot...pilot, lo, mid, hi, rig, subsystem, cargo, {other cargo}
    for (auto cur : invMap)
        str << cur.first << "(" << sDataMgr.GetFlagName(cur.second->flag()) << "): " << cur.second->itemName() << "<br>"; // 20 + 40 for name (60)

    int count = invMap.size();
    int size = count * 60;
    size += 90;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), ship->name(), inventoryID, inv, ship.get(), count);

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_skilllist(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug char skills
     * wip.   -allan 15Mar16
     */

    std::map<uint32, InventoryItemRef> invMap;
    invMap.clear();
    uint32 inventoryID = pClient->GetCharacterID();
    Inventory* inv = pClient->GetChar()->GetMyInventory();
    inv->GetInventoryMap(invMap);

    std::ostringstream str;
    str.clear();
    str << "InventoryID %u(%p) of %s has %u skills.<br><br>"; //80

    for (auto cur : invMap) {
        if (cur.second->flag() == flagSkillInTraining)
            str << "<color=aqua>";
        str << cur.first << " - " << cur.second->itemName();    //45
        str  << " (" << cur.second->GetAttribute(AttrSkillLevel).get_uint32() << ") "; //3
        if (cur.second->GetAttribute(AttrSkillPoints).get_type() == evil_number_int)    //15
            str << "[i-" << cur.second->GetAttribute(AttrSkillPoints).get_int();
        else
            str << "[f-" << cur.second->GetAttribute(AttrSkillPoints).get_float();
        if (cur.second->flag() == flagSkillInTraining)
            str << "]</color><br>";
        else
            str << "]<br>"; // 45 + 3 + 15 + 5 (70)
    }

    int count = invMap.size();
    int size = count * 80;
    size += 80;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), inventoryID, inv, pClient->GetChar()->name(), count);

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_attrlist(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug attributes
     * wip.   -allan 15Mar17
     */

    if (!args.isNumber(1))
        throw CustomError ("Argument 1 must be a valid itemID.");
    uint32 itemID(atol(args.arg(1).c_str()));

    InventoryItemRef iRef(sItemFactory.GetItem(itemID));
    if (iRef.get() == nullptr) {
        // make error msg here
        return nullptr;
    }

    std::map<uint16, EvilNumber> attrMap;
    iRef->GetAttributeMap()->CopyAttributes(attrMap);

    std::ostringstream str;
    str.clear();
    str << "%s (%u) has %u attributes.<br><br>"; //70

    for (auto cur : attrMap) {
        str << sDataMgr.GetAttrName(cur.first) << " ("<< cur.first << ") ";  //35
        if (cur.second.get_type() == evil_number_int)    //15
            str << "i- " << cur.second.get_int();
        else
            str << "f- " << cur.second.get_float();
        str << "<br>"; // 4 + 15 + 35 (54)
    }

    int count = attrMap.size();
    int size = count * 60;
    size += 70;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), iRef->name(), itemID, count);

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_showsession(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    std::ostringstream str;
    str.clear();
    str << "Current Session Values.<br><br>"; //32

    str << "charid: %i <br>"; //14+10
    str << "charname: %s <br>"; //16+20
    str << "shipid: %u <br>"; //14+10
    str << "cloneStationID: %u <br>"; //21+10

    str << "clientid: %li <br>"; //16+10
    str << "userid: %u <br>"; //14+10
    str << "sessionID: %li <br>"; //18+20

    str << "locationid: %u <br>"; //18+10
    str << "stationid: %i <br>"; //17+10
    str << "stationid2: %i <br>"; //17+10
    str << "solarsystemid2: %u <br>"; //22+10
    str << "constellationid: %u <br>"; //23+10
    str << "regionid: %u <br>";

    str << "corpid: %u <br>"; //14+10
    str << "hqID: %u <br>"; //12+10
    str << "corpAccountKey: %i <br>"; //22+10
    str << "corpRole: %li <br>"; //17+20
    str << "rolesAtAll: %li <br>"; //19+20
    str << "rolesAtBase: %li <br>"; //20+20
    str << "rolesAtHQ: %li <br>"; //18+20
    str << "rolesAtOther: %li <br>"; //21+20

    str << "fleetID: %i <br>"; //14+10
    str << "wingID: %i <br>"; //13+10
    str << "squadID: %i <br>"; //14+10
    str << "job: %s <br>"; //10+10
    str << "role: %s <br>"; //11+10
    str << "booster: %s <br>"; //14+10
    str << "joinTime: %li <br>"; //16+20

    int size = 32;  // header
    size += 445;    // text
    size += 170;    // %i
    size += 140;    // %l*
    size += 50;     // %s
    char reply[size];
    snprintf(reply, size, str.str().c_str(),
             pClient->GetCharacterID(), pClient->GetName(), pClient->GetShipID(), pClient->GetCloneStationID(), pClient->GetClientID(), pClient->GetUserID(),
             pClient->GetSession()->GetSessionID(), pClient->GetLocationID(), pClient->GetStationID(), pClient->GetStationID2(), pClient->GetSystemID(), pClient->GetConstellationID(),
             pClient->GetRegionID(), pClient->GetCorporationID(), pClient->GetCorpHQ(), pClient->GetCorpAccountKey(), pClient->GetCorpRole(), pClient->GetRolesAtAll(),
             pClient->GetRolesAtBase(), pClient->GetRolesAtHQ(), pClient->GetRolesAtOther(), pClient->GetChar()->fleetID(), pClient->GetChar()->wingID(),
             pClient->GetChar()->squadID(), sFltSvc.GetJobName(pClient->GetChar()->fleetJob()).c_str(), sFltSvc.GetRoleName(pClient->GetChar()->fleetRole()).c_str(),
             sFltSvc.GetBoosterName(pClient->GetChar()->fleetBooster()).c_str(),pClient->GetChar()->fleetJoinTime());

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_shipdna(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    char reply[200];
    snprintf(reply, 200, "%s", pClient->GetShip()->GetShipDNA().c_str());

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_targlist(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!pClient->IsInSpace()) {
        pClient->SendInfoModalMsg("You are not in Space.");
        return nullptr;
    }

    uint16 length = 1, count = 0;
    std::string into = pClient->GetShipSE()->TargetMgr()->TargetList(length, count);

    std::ostringstream str;
    str.clear();
    str << "Target List for %s in shipID %u<br>"; //30+30
    str << "    %u entries in list<br>";   //30
    str << "%s"; //length

    int size = 60;  // header
    size += 30;    // text
    size += length;

    char reply[size];
    snprintf(reply, size, str.str().c_str(), pClient->GetName(), pClient->GetShipID(), count, into.c_str());

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_track(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    bool tracking = sEntityList.GetTracking();
    std::string track = "enabled";
    if (tracking) {
        sEntityList.SetTracking(false);
        track = "disabled";
    } else
        sEntityList.SetTracking(true);

    char reply[30];
    snprintf(reply, 30, "Ship Tracking is %s.", track.c_str());

    pClient->SendNotifyMsg(reply);
    return new PyString(reply);
}

PyResult Command_bubbletrack(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    std::string track = "enabled";
    std::string type = "bubble";

    if (sConfig.debug.BubbleTrack) {
        sConfig.debug.BubbleTrack = false;
        track = "disabled";
    } else {
        sConfig.debug.BubbleTrack = true;
    }

    // begin argument processing
    int locationID = 0;
    if (args.argCount() < 2) {
        if (sConfig.debug.BubbleTrack) {
            pClient->GetShipSE()->SysBubble()->MarkCenter();
        } else {
            pClient->GetShipSE()->SysBubble()->RemoveMarkers();
        }
    } else if (args.argCount() == 2) { // single arg - help, bubble, system, universe, {invalid}
        if (strcmp(args.arg(1).c_str(), "help") == 0) {
            //  {.bubbletrack help}  will display the following list of options in notification window
            std::ostringstream str; // for 'help' printing
            str << ".bubbletrack [arg]<br>"; //22
            str << "no args = mark current bubble.<br>"; //35
            str << "arg = help|bubble|system|universe<br>"; //40
            str << "arg = help - display this information.<br>"; //45
            str << "arg = bubble - mark current bubble.<br>"; //42
            str << "arg = system - mark all bubbles in current system.  center markers are global.<br>";  //80
            str << "arg = universe - mark all bubbles in all systems.  center markers are global.<br>"; //80
            str << "typical use is .bubbletrack with no args to mark current bubble.<br>";  //70
            str << "'system' and 'universe' are used to show bubbles using solarsystem map from scan window (using '.showall').<br>";  //110
            int size = 520;
            char reply[size];
            snprintf(reply, size, str.str().c_str());
            pClient->SendInfoModalMsg(reply);
            return nullptr;
        } else if (strcmp(args.arg(1).c_str(), "bubble") == 0) {
            if (sConfig.debug.BubbleTrack) {
                pClient->GetShipSE()->SysBubble()->MarkCenter();
            } else {
                pClient->GetShipSE()->SysBubble()->RemoveMarkers();
            }
        } else if (strcmp(args.arg(1).c_str(), "system") == 0) {
            type = "system";
            if (sConfig.debug.BubbleTrack) {
                sBubbleMgr.MarkCenters(pClient->GetSystemID());
            } else {
                sBubbleMgr.RemoveMarkers(pClient->GetSystemID());
            }
        } else if (strcmp(args.arg(1).c_str(), "universe") == 0) {
            type = "universe";
            if (sConfig.debug.BubbleTrack) {
                sBubbleMgr.MarkCenters();
            } else {
                sBubbleMgr.RemoveMarkers();
            }
        } else {
            throw CustomError ("BubbleTrack: Unrecognized Argument.");
        }
    } else {
        throw CustomError ("BubbleTrack: Too Many Arguments.");
    }

    char reply[45];
    snprintf(reply, 45, "Bubble Tracking for %s is %s.", type.c_str(), track.c_str());

    pClient->SendNotifyMsg(reply);
    return new PyString(reply);
}

PyResult Command_warpto(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!pClient->IsInSpace())
        throw CustomError ("You're not in space.");
    if (!pClient->GetShipSE()->SysBubble())
        pClient->EnterSystem(pClient->GetSystemID());
    if (!pClient->GetShipSE()->DestinyMgr())
        pClient->SetDestiny(NULL_ORIGIN);

    /** @todo  finish this.... */
    pClient->GetShipSE()->DestinyMgr()->Halt();

    char reply[55];
    snprintf(reply, 55, "Command Unavailible.\nShip Halted.");

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}


PyResult Command_entityspawn(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!pClient->IsInSpace())
        throw CustomError ("You're not in space.");
    if (!pClient->GetShipSE()->SysBubble())
        pClient->EnterSystem(pClient->GetSystemID());
    if (!pClient->GetShipSE()->DestinyMgr())
        pClient->SetDestiny(NULL_ORIGIN);

//sm.RemoteSvc('slash').SlashCmd('/entityspawn {0} {1} {2} 0 {3}'.format(recipeID, typeID, x, y))
    /** @todo  finish this.... */
    pClient->GetShipSE()->DestinyMgr()->Halt();

    char reply[55];
    snprintf(reply, 55, "Command Unfinished.\nShip Halted.");

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_fleetboost(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    uint32 fleetID = pClient->GetChar()->fleetID();

    if (fleetID == 0) {
        pClient->SendInfoModalMsg("You are not in a fleet");
        return nullptr;
    }

    uint16 length = 1;
    std::string into = sFltSvc.GetBoosterData(fleetID, length);

    std::ostringstream str;
    str.clear();
    str << "<color=aqua>FleetID %u Command and Boost Data Window.</color><br><br>";   //77
    str << "%s"; //length

    int size = 77;  // header
    size += length;

    char reply[size];
    snprintf(reply, size, str.str().c_str(), fleetID, into.c_str());

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_fleetinvite(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!pClient->InFleet())
        throw CustomError ("You're not in a fleet.");
    if (!pClient->IsFleetBoss())
        throw CustomError ("You're not fleet boss.");

    if (args.isNumber(1))
        throw PyException(CustomError("Argument 1 should be one of 'None', 'Corp', 'Alliance', 'Faction', or 'All'"));

    char reply[55];
    snprintf(reply, 55, "Command Unfinished.");

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_getposition(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!pClient->IsInSpace())
        throw CustomError ("You're not in space.");
    if (!pClient->GetShipSE()->SysBubble())
        throw CustomError ("You're not in a bubble.");
    if (!pClient->GetShipSE()->DestinyMgr())
        throw CustomError ("You have no destiny manager.");

    GPoint sPos(pClient->GetShipSE()->GetPosition());
    GPoint mPos(pClient->SystemMgr()->GetClosestMoonSE(sPos)->GetPosition());
    GVector vec(sPos, mPos);

    float normProd = sPos.normalize() * mPos.normalize();
    float dotProd = sPos.dotProduct(mPos);
    float angle = std::acos( dotProd / normProd);

    float azimuth = std::atan2(vec.z, vec.x);
    float elevation = std::atan2(vec.y, std::sqrt(std::pow(vec.x,2) + std::pow(vec.z,2)));

    std::ostringstream str;
    str.clear();
    str << "Angle for current position is " << angle << "<br>";
    str << "Az: " << azimuth << " Ele: " << elevation;
    int size = 70;
    char reply[size];
    snprintf(reply, size, str.str().c_str());

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_players(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    std::vector<Client*> cVec;
    sEntityList.GetClients(cVec);
    std::ostringstream str;
    str.clear();
    str << "Active Player List:<br>" << std::to_string(cVec.size()) << " Online Players.<br>";

    for (auto cur : cVec) {
        str << cur->GetName();
        if (cur->IsDocked())
            str << " is docked in ";
        else
            str << " is flying around ";
        str << cur->GetSystemName().c_str() << "<br>";
    }

    int size = cVec.size() * 100;
    size += 80;
    char reply[size];
    snprintf(reply, size, str.str().c_str());

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_showall(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    std::string showall = "Enabled";
    if (pClient->IsShowall()) {
        pClient->SetShowAll(false);
        showall = "Disabled";
    } else
        pClient->SetShowAll(true);

    char reply[35];
    snprintf(reply, 35, "Show All on Scanner is %s.", showall.c_str());

    pClient->SendNotifyMsg(reply);
    return new PyString(reply);
}

PyResult Command_autostop(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    std::string stop = "Enabled";
    if (pClient->AutoStop()) {
        pClient->SetAutoStop(false);
        stop = "Disabled";
    } else
        pClient->SetAutoStop(true);

    char reply[35];
    snprintf(reply, 35, "Module Auto-Stop is %s.", stop.c_str());

    pClient->SendNotifyMsg(reply);
    return new PyString(reply);
}

PyResult Command_cargo(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug inventory
     *   -allan 2Jul20
     */

    // check for pod...no cargo
    if (pClient->GetShip()->typeID() == itemTypeCapsule) {
        char reply[21];
        snprintf(reply, 21, "Your Pod's current cargo is you.<br>There is no room for anything else.");
        pClient->SendInfoModalMsg(reply);
        return nullptr;
    }

    // build attr-to-flag map
    std::map<uint16, EVEItemFlags> cargoAttrToFlag;
    cargoAttrToFlag[AttrDroneCapacity] = flagDroneBay;
    cargoAttrToFlag[AttrHasShipMaintenanceBay] = flagShipHangar;
    cargoAttrToFlag[AttrFuelBayCapacity] = flagFuelBay;
    cargoAttrToFlag[AttrOreHoldCapacity] = flagOreHold;
    cargoAttrToFlag[AttrGasHoldCapacity] = flagGasHold;
    cargoAttrToFlag[AttrAmmoHoldCapacity] = flagAmmoHold;
    cargoAttrToFlag[AttrShipHoldCapacity] = flagShipHold;
    cargoAttrToFlag[AttrMineralHoldCapacity] = flagMineralHold;
    cargoAttrToFlag[AttrSalvageHoldCapacity] = flagSalvageHold;
    cargoAttrToFlag[AttrSmallShipHoldCapacity] = flagSmallShipHold;
    cargoAttrToFlag[AttrMediumShipHoldCapacity] = flagMediumShipHold;
    cargoAttrToFlag[AttrLargeShipHoldCapacity] = flagLargeShipHold;
    cargoAttrToFlag[AttrIndustrialShipHoldCapacity] = flagIndustrialShipHold;
    cargoAttrToFlag[AttrCommandCenterHoldCapacity] = flagCommandCenterHold;
    cargoAttrToFlag[AttrPlanetaryCommoditiesHoldCapacity] = flagPlanetaryCommoditiesHold;
    cargoAttrToFlag[AttrQuafeHoldCapacity] = flagQuafeBay;

    uint32 qty = 0, count = 0, corp = 0;
    std::multimap<uint8, InventoryItemRef> cargoMap;
    ShipItemRef shipRef = pClient->GetShip();
    Inventory* inv = shipRef->GetMyInventory();
    inv->GetCargoList(cargoMap);

    std::ostringstream str;
    str.clear();
    str << "Reported Cargo in %s (%u)  [LineCount:%u]<br>"; //60
    str << "Hold Name    Volume in m3. Stored/Total<br>"; //40
    str << "    Qty  ItemName  (volume each)  stack volume<br>"; //50

    // get available cargo holds in ship and list contents for each

    // all ships have flagCargoHold (except pod)
    ++count;
    str << "<br>" << sDataMgr.GetFlagName(flagCargoHold);
    str << "    " << inv->GetStoredVolume(flagCargoHold);
    str << "/" << inv->GetCapacity(flagCargoHold) << "<br>";
    if (cargoMap.find(flagCargoHold) == cargoMap.end()) {
        ++count;
        str << "  Empty<br>";
    } else {
        qty = 0;
        auto range = cargoMap.equal_range(flagCargoHold);
        for ( auto itr = range.first; itr != range.second; ++itr ) {
            ++count;
            qty = itr->second->quantity();
            str << "    " << qty << " " << itr->second->itemName();
            str << "  (" << itr->second->type().volume() << ") " << itr->second->type().volume() *qty << "<br>";
        }
    }

    // loop thru cargo list in all possible holds
    for (auto cur : cargoAttrToFlag) {
        if ( shipRef->HasAttribute(cur.first) and ( shipRef->GetAttribute(cur.first) > EvilZero)) {
            ++count;
            str << "<br>" << sDataMgr.GetFlagName(cur.second);
            str << "    " << inv->GetStoredVolume(cur.second);
            str << "/" << inv->GetCapacity(cur.second) << "<br>";
            if (cargoMap.find(cur.second) == cargoMap.end()) {
                ++count;
                str << "  Empty<br>";
            } else {
                qty = 0;
                auto range = cargoMap.equal_range(cur.second);
                for ( auto itr = range.first; itr != range.second; ++itr ) {
                    ++count;
                    qty = itr->second->quantity();
                    str << "    " << qty << " " << itr->second->itemName();
                    str << "  (" << itr->second->type().volume() << ") " << itr->second->type().volume() *qty << "<br>";
                }
            }
        }
    }

    // check for corp hangars
    if ( shipRef->HasAttribute(AttrHasCorporateHangars)) {
        ++count;
        corp = 190;
        // get corp's hangar names instead of default flag names
        std::map<uint8, std::string> hangarNames;
        ServiceDB::GetCorpHangarNames(pClient->GetCorporationID(), hangarNames);
        str << "<br><br>Corp Hangars share capacity. (currently incomplete)<br>"; // 20
        str << "Currently using " << inv->GetCorpHangerCapyUsed() << " of " << inv->GetCapacity(flagHangar) << "m3<br>"; //40
        str << "Hold Name    Volume Stored in m3.<br>"; //40
        str << "    Qty  ItemName  (volume each)  stack volume<br>"; //50
        str << "<br>" << hangarNames[flagHangar] << "    " << inv->GetStoredVolume(flagHangar, false) << "<br>";
        if (cargoMap.find(flagHangar) == cargoMap.end()) {
            ++count;
            str << "  Empty<br>";
        } else {
            qty = 0;
            auto range = cargoMap.equal_range(flagHangar);
            for ( auto itr = range.first; itr != range.second; ++itr ) {
                ++count;
                qty = itr->second->quantity();
                str << "    " << qty << " " << itr->second->itemName();
                str << "  (" << itr->second->type().volume() << ") " << itr->second->type().volume() *qty << "<br>";
            }
        }
        for (uint8 i = flagCorpHangar2; i < flagSecondaryStorage; ++i) {
            ++count;
            str << "<br>" << hangarNames[i]; //sDataMgr.GetFlagName(i);
            str << "    " << inv->GetStoredVolume((EVEItemFlags)i, false) << "<br>";
            if (cargoMap.find(i) == cargoMap.end()) {
                ++count;
                str << "  Empty<br>";
            } else {
                qty = 0;
                auto range = cargoMap.equal_range(i);
                for ( auto itr = range.first; itr != range.second; ++itr ) {
                    ++count;
                    qty = itr->second->quantity();
                    str << "    " << qty << " " << itr->second->itemName();
                    str << "  (" << itr->second->type().volume() << ") " << itr->second->type().volume() *qty << "<br>";
                }
            }
        }
    }

    int size = count * 100;
    size += 150;
    size += corp;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), shipRef->name(), shipRef->itemID(), count);

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_bubblewarp(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!pClient->IsInSpace())
        throw CustomError ("You're not in space.");
    if (!pClient->GetShipSE()->SysBubble())
        throw CustomError ("You're not in a bubble.");
    if (!pClient->GetShipSE()->DestinyMgr())
        throw CustomError ("You have no destiny manager.");

    if (!args.isNumber(1))
        throw CustomError ("Argument 1 must be a valid bubbleID.");
    uint16 bubbleID = atoi(args.arg(1).c_str());
    SystemBubble* pBubble = sBubbleMgr.FindBubbleByID(bubbleID);
    if (pBubble == nullptr)
        throw CustomError ("Bubble %u not found.", bubbleID);

    if (pBubble->GetSystemID() != pClient->GetSystemID())
        throw CustomError ("Cannot warp to bubble %u because it is in %s and you are in %s", \
                    bubbleID, pBubble->GetSystem()->GetName(), pClient->GetSystemName().c_str());

    pClient->GetShipSE()->DestinyMgr()->WarpTo(pBubble->GetCenter());

    int size(50);
    char reply[size];
    snprintf(reply, size, "Ship Initalized warp to bubbleID %u", bubbleID);

    pClient->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_runtest(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!pClient->IsInSpace())
        throw CustomError ("You're not in space.");

    int size(30);
    char reply[size];
    snprintf(reply, size, "Running posTest()");

    testing::posTest(pClient);
    return nullptr;
}

PyResult Command_bindList(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    std::ostringstream str;
    str.clear();
    str << "Current Bound Object Listing (%u)<br><br>"; //45

    std::vector<PyServiceMgr::BoundObj> vec;
    pClient->services().BoundObjectVec(vec);
    for (auto cur : vec) {
        str << cur.client->GetName() << ": " << cur.object->bindID() << " ";    //40
        str << cur.object->GetName() << "<br>";         //60
    }

    int count = vec.size();
    int size = count * 100;
    size += 45;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), count);

    pClient->SendInfoModalMsg(reply);

    return nullptr;
}

PyResult Command_dropLoot(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!pClient->IsInSpace())
        throw CustomError ("You're not in space.");
    if (!pClient->GetShipSE()->SysBubble())
        throw CustomError ("You're not in a bubble.");
    if (!pClient->GetShipSE()->DestinyMgr())
        throw CustomError ("You have no destiny manager.");

    uint16 bubbleID = atoi(args.arg(1).c_str());
    SystemBubble* pBubble = sBubbleMgr.FindBubbleByID(bubbleID);
    if (pBubble == nullptr)
        throw CustomError ("Bubble %u not found.", bubbleID);

    if (pBubble->GetSystemID() != pClient->GetSystemID())
        throw CustomError ("bubble %u is in %s and you are in %s", \
                bubbleID, pBubble->GetSystem()->GetName(), pClient->GetSystemName().c_str());

    pBubble->CmdDropLoot();

    return nullptr;
}

/* groove's new command.....
 *    /fit [me|itemID] [typeID] [flag=slot]
 * then sends ScatterEvent OnRefreshModuleBanks after successful call.
 */

/*defaultMacros = {'GMH: Unload All': '/unload me all',
 ' WM: Remove All Drones': '/unspawn range=500000* only=categoryDrone',
 'WM: Remove All Wrecks': '/unspawn range=500000 only=groupWreck',
 'WM: Remove CargoContainers': '/unspawn range=500000 only=groupCargoContainer',
 'WM: Remove SecureContainers': '/unspawn range=500000 only=groupSecureCargoContainer',
 'HEALSELF: Repair My Ship': '/heal',
 'HEALSELF: Repair My Modules': '/repairmodules',
 'GMH: Online My Modules': '/online me',
 'GML: Session Change 5sec': '/sessionchange 5'}
 */

/*@Allan new command to implement.  I may try but I don't know fuckall for destiny stuff
 * /reportdesync
 *
 * It wants a tuple of (dict, timestamp)
 * dict is:
 * key = ballID
 * val = pos (vector probably best to use util.KeyVal)(uses val.x, val.y, val.z)
 *
 * It compares the balls trying to find position and velocity inconsistencies showing them in a model window
 */
