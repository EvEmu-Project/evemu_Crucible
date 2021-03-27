/*
 * SystemCommands.cpp
 *   this file is commands related to an item's creation/deletion and location/position
 *
 */

#include "eve-server.h"

#include "Client.h"
#include "ConsoleCommands.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "admin/AllCommands.h"
#include "admin/CommandDB.h"
#include "inventory/AttributeEnum.h"
#include "inventory/InventoryDB.h"
#include "inventory/InventoryItem.h"
#include "manufacturing/Blueprint.h"
#include "map/MapConnections.h"
#include "npc/Drone.h"
#include "system/Damage.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "admin/CommandHelper.h"
#include "tables/invGroups.h"
#include "tables/invCategories.h"

PyResult Command_goto(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount() != 4
        || !args.isNumber(1)
        || !args.isNumber(2)
        || !args.isNumber(3))
    {
        throw PyException(MakeCustomError("Correct Usage: /goto [x coord] [y coor] [z coord]"));
    }

    GPoint p(atoll(args.arg(1).c_str()),
             atoll(args.arg(2).c_str()),
             atoll(args.arg(3).c_str()));

    pClient->MoveToPosition(p);
    return new PyString("Goto successful.");
}

PyResult Command_translocate(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    return Command_tr(pClient,db,services,args);
}

/*   hardcoded menu translocates in client
 * ('/tr me ' + str(mapItem.itemID),)   << mapItem
 * ('/tr me ' + str(charID),)           << charItem
 * ('/tr me ' + str(itemID),)           << slimItem
 *
 * ('/tr me last')                      << last Station
 * ('/tr me %s' % stationID)
 * ('/tr me offset=randvec(100au)')     << hop (x,y,z)
 * ('/tr me home')                      << home station
 * ('/tr %d' % solarSystemID)
 * ('/tr %d' % locationID)
 * ('/tr me me offset=%d,%d,%d' % (int(v.x), int(v.y), int(v.z)))
 * ('/tr %d me noblock' % charID)       << used to group-move all players in <chat>
 */
/* {'messageKey': 'LocationNameInvalid', 'dataID': 17882829, 'suppressable': False, 'bodyID': 259279, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1112}
 * {'messageKey': 'LocationNameInvalidBannedWord', 'dataID': 17882832, 'suppressable': False, 'bodyID': 259280, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1113}
 * {'messageKey': 'LocationNameInvalidFirstChar', 'dataID': 17882835, 'suppressable': False, 'bodyID': 259281, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1114}
 * {'messageKey': 'LocationNameInvalidLastChar', 'dataID': 17882838, 'suppressable': False, 'bodyID': 259282, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1115}
 * {'messageKey': 'LocationNameInvalidMaxLength', 'dataID': 17882841, 'suppressable': False, 'bodyID': 259283, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1116}
 * {'messageKey': 'LocationNameInvalidMaxSpaces', 'dataID': 17882844, 'suppressable': False, 'bodyID': 259284, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1117}
 * {'messageKey': 'LocationNameInvalidMinLength', 'dataID': 17882847, 'suppressable': False, 'bodyID': 259285, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1118}
 * {'messageKey': 'LocationNameInvalidSomeChar', 'dataID': 17882850, 'suppressable': False, 'bodyID': 259286, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1119}
 * {'messageKey': 'LocationNameInvalidTaken', 'dataID': 17882853, 'suppressable': False, 'bodyID': 259287, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1120}
 */
/** @todo this is a good start, but will need a bit more logic to idiot-proof and finish  */
PyResult Command_tr(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    // i dont expect this to be used very often, so a bit of bloat is acceptable

    if (pClient == nullptr) // should never hit,
        throw PyException(MakeCustomError("Translocate: Invalid Caller - Client sent NULL."));

    if (args.argCount() < 2)
        throw PyException(MakeCustomError("Translocate: Missing Arguments (use '.tr help' for usage)"));

    Client* pOtherClient(nullptr);
    const char* Help = "help";  // many uses.  avoid creating temp objects for every test
    bool me = false, item = false, player = false, fleet = false;
    GPoint pt(NULL_ORIGIN);
    int locationID = 0, myLocationID = pClient->GetLocationID();

    if (args.argCount() == 2) { // single arg - help, locationID, {invalid}
        if (args.isNumber(1)) {
            // tr <me> to locationID
            locationID = atoi(args.arg(1).c_str());
        } else if (strcmp(args.arg(1).c_str(), Help) == 0) {
            //  {.tr help}  will display the following list of options in notification window
            std::ostringstream str; // for 'help' printing
            str << ".tr [required first arg] [optional second arg] [optional third arg] [optional fourth arg]<br>"; //120
            str << "1st arg = help|me|fleet|player name|shipID|itemID|locationID<br>"; //45
            str << "2nd arg = help|me|fleet|home|last|shipID|itemID|locationID|x coord|moon|planet<br>";  //85
            str << "3ed arg = me|fleet|home|last|shipID|itemID|locationID|y coords|moon|planet <br>"; //80
            str << "4th arg = fleet|home|last|shipID|itemID|locationID|z coords|moon|planet <br>"; //77
            str << "typical use is .tr locationID<br>";  //35
            str << "<br>As there are too many options to explain in this msg, a full usage list can be found on our forums.<br>";  //105
            int size = 500;
            char reply[size];
            snprintf(reply, size, str.str().c_str());
            pClient->SendInfoModalMsg(reply);
            return nullptr;
        } else {
            // tr <me> to <locationName>?
            // this hits db directly, so test for possible sql injection code
            for (const auto cur : badCharsSearch)
                if (EvE::icontains(args.arg(1), cur))
                    throw PyException( MakeCustomError("Location contains invalid characters"));
            locationID = db->GetSolarSystem(args.arg(1).c_str());
            if (!IsSolarSystem(locationID)) {
                locationID = db->GetStation(args.arg(1).c_str());
                if (!IsStation(locationID))
                    throw PyException(MakeCustomError("Translocate: Name Argument is neither SolarSystem nor StationName"));
            }
        }
    } else if (args.argCount() == 3) {  // 2 args - me, player, ship, item, fleet, {invalid} : help, home, last, ship, location, moon, planet, {invalid}
        // test for 'help' command
        if ((strcmp(args.arg(1).c_str(), Help) == 0)
        or (strcmp(args.arg(2).c_str(), Help) == 0))
            throw PyException(MakeCustomError("Translocate: Invalid Arguments - 'help' not avalible for this command."));

            // decode first arg  -- me, shipID, charID, playerName, itemID {invalid}
        if (args.isNumber(1)) {     // charID, shipID, {invalid}
            int objectID = atoi(args.arg(1).c_str());
            if (IsCharacter(objectID)) {
                pOtherClient = sEntityList.FindClientByCharID(objectID);
            } else if (IsPlayerItem(objectID)) {
                // get object's ownerID...this is rather powerful.
                InventoryItemRef iRef = sItemFactory.GetItem(objectID);
                if (iRef.get() == nullptr)
                    throw PyException(MakeCustomError("Translocate: Invalid Arguments - target object was not found."));
                pOtherClient = sEntityList.FindClientByCharID(iRef->ownerID());
            } else if (IsNPC(objectID)) {
                throw PyException(MakeCustomError("Translocate: Invalid Object - You really wanna relocate an NPC?"));
            } else
                throw PyException(MakeCustomError("Translocate: Invalid Object - %i is neither a character nor a ship", objectID));
        } else if (strcmp(args.arg(1).c_str(), "me") == 0) {
            // tr <me> to ?
            //  pClient doesnt change.  do nothing here
            me = true;
        } else if (strcmp(args.arg(1).c_str(), "ship") == 0) {
            // tr <ship|ship owner> to ?
            item = true;
        } else if (strcmp(args.arg(1).c_str(), "item") == 0) {
            // tr me to item
            item = true;
        } else if (strcmp(args.arg(1).c_str(), "fleet") == 0) {
            // tr <my fleet> to ?
            fleet = true;
            // not sure how im gonna do this one yet....
            if (!pClient->InFleet())
                throw PyException(MakeCustomError("Translocate: Fleet Move - You are not in a fleet."));
            throw PyException(MakeCustomError("Translocate: Fleet Move - This command is currently incomplete."));
        } else {
            // check for player name
            pOtherClient = sEntityList.FindClientByName(args.arg(1).c_str());
            if (pOtherClient == nullptr)
                throw PyException(MakeCustomError("Translocate: Bad Name - %s is not online or is not a valid player name", args.arg(1).c_str()));
        }
        // decode second arg    -- me, fleet, home, last, location, moon, planet, {invalid}
        if (args.isNumber(2)) {
            // tr <me|ship|player|fleet> to <locationID>
            locationID = atoi(args.arg(2).c_str());
        } else if (strcmp(args.arg(2).c_str(), "me") == 0) {
            // tr <me|ship|player|fleet> to me
            //  pClient doesnt change.  do nothing here
            me = true;
            locationID = myLocationID;  // this wont do anything if used as ".tr me me"
        } else if (strcmp(args.arg(2).c_str(), "home") == 0) {
            if (fleet)
                throw PyException(MakeCustomError("Translocate: Fleet Move - You cannot tr a fleet 'home'...yet."));
            locationID = pClient->GetCloneStationID();
        } else if (strcmp(args.arg(2).c_str(), "ship") == 0) {
            // tr <me|ship|player|fleet> to <ship|ship owner>
            if (item)
                throw PyException(MakeCustomError("Translocate: Ship Move - You cannot tr one ship to another ship...yet."));
            item = true;
        } else if (strcmp(args.arg(2).c_str(), "last") == 0) {
            // last station <me|player> was docked in
            if (fleet)
                throw PyException(MakeCustomError("Translocate: Fleet Move - You cannot tr a fleet to last docked station...yet."));
            // if called with <player name>, <charID>, or <shipID>, then uses that char's (or owner's) last docked station
            if (pOtherClient != nullptr)
                locationID = pOtherClient->GetDockStationID();
            else
                locationID = pClient->GetDockStationID();
        } else if (strcmp(args.arg(2).c_str(), "moon") == 0) {
            // random moon in <me|player> current system
            SystemGPoint sGP;
            pt = sGP.GetRandPointOnMoon(pOtherClient == nullptr ? pClient->GetSystemID() : pOtherClient->GetSystemID());
            //throw PyException(MakeCustomError("Translocate: This option is Incomplete"));
        } else if (strcmp(args.arg(2).c_str(), "planet") == 0) {
            // random planet in <me|player> current system
            SystemGPoint sGP;
            pt = sGP.GetRandPointOnPlanet(pOtherClient == nullptr ? pClient->GetSystemID() : pOtherClient->GetSystemID());
            //throw PyException(MakeCustomError("Translocate: This option is Incomplete"));
        } else {
            if (me) {
                // tr me to <locationName>?
                // this hits db directly, so test for possible sql injection code
                for (const auto cur : badCharsSearch)
                    if (EvE::icontains(args.arg(1), cur))
                        throw PyException( MakeCustomError("Location contains invalid characters"));
                    locationID = db->GetSolarSystem(args.arg(1).c_str());
                if (!IsSolarSystem(locationID)) {
                    locationID = db->GetStation(args.arg(1).c_str());
                    if (!IsStation(locationID))
                        throw PyException(MakeCustomError("Translocate: Name Argument is neither SolarSystem nor StationName"));
                }
            } else {
                // what are we missing?
                throw PyException(MakeCustomError("Translocate: Missing Arguments"));
            }
        }
    } else if (args.argCount() == 4) { // 3 args - me, player, ship,  coords, {invalid} : me, home, last, location, {invalid} : last, moon, planet, {invalid}
        // test for 'help' command
        if ((strcmp(args.arg(1).c_str(), Help) == 0)
        or (strcmp(args.arg(2).c_str(), Help) == 0)
        or (strcmp(args.arg(3).c_str(), Help) == 0))
            throw PyException(MakeCustomError("Translocate: Invalid Arguments - 'help' not avalible for this command."));

        // check for coords
        if ((args.isNumber(1))
        and (args.isNumber(2))
        and (args.isNumber(3)))
            throw PyException(MakeCustomError("Translocate: Coords in current system - Use '.goto {x,y,z}' instead of this."));

        // decode first arg  -- me, shipID, charID, playerName, {invalid}
        if (args.isNumber(1)) {
            int objectID = atoi(args.arg(1).c_str());
            if (IsCharacter(objectID)) {
                pOtherClient = sEntityList.FindClientByCharID(objectID);
            } else if (IsPlayerItem(objectID)) {
                // get object's ownerID
                pOtherClient = sEntityList.FindClientByCharID(objectID);
            } else
                throw PyException(MakeCustomError("Translocate: Invalid Object - %i is neither a character nor a ship", objectID));
        } else if (strcmp(args.arg(1).c_str(), "me") == 0) {
            // tr <me> to ?
            //  pClient doesnt change.  do nothing here
            me = true;
        } else if (strcmp(args.arg(1).c_str(), "fleet") == 0) {
            // tr <my fleet> to ?
            fleet = true;
        } else {
            // this is either player name or invalid arg
            throw PyException(MakeCustomError("Translocate: Invalid Arguments - searching by player name is incomplete at this time."));
        }

        // decode second arg    -- me, fleet, home, last, location, moon, planet, {invalid}
        if (args.isNumber(2)) {
            locationID = atoi(args.arg(2).c_str());
        } else if (strcmp(args.arg(2).c_str(), "fleet") == 0) {
            if (fleet)
                throw PyException(MakeCustomError("Translocate: Fleet Move - You cannot tr a fleet to 'fleet'."));
            // tr <my|player> fleet to last, moon, planet, {invalid}
            fleet = true;
        } else if (strcmp(args.arg(2).c_str(), "me") == 0) {
            // tr <player|fleet> to me
            me = true;
            locationID = myLocationID;
        } else if (strcmp(args.arg(2).c_str(), "ship") == 0) {
            // tr <me|ship|player|fleet> to <ship|ship owner>
            if (item)
                throw PyException(MakeCustomError("Translocate: Ship Move - You cannot tr one ship to another ship...yet."));
            item = true;
        } else if (strcmp(args.arg(2).c_str(), "home") == 0) {
            // tr <player|fleet> to <my|players> home
            if (fleet)
                throw PyException(MakeCustomError("Translocate: Fleet Move - You cannot tr a fleet 'home'...yet."));
            locationID = pClient->GetCloneStationID();
        } else if (strcmp(args.arg(2).c_str(), "last") == 0) {
            if (fleet)
                throw PyException(MakeCustomError("Translocate: Fleet Move - You cannot tr a fleet to last docked station...yet."));
            //  tr <player> me last -- tr <player> to my last docked station
            // last station client was docked in
            // if called with <player name> then uses that char's last station
            if (pOtherClient != nullptr)
                locationID = pOtherClient->GetDockStationID();
            else
                locationID = pClient->GetDockStationID();
        } else if (strcmp(args.arg(2).c_str(), "moon") == 0) {
            // random moon in client current system
            SystemGPoint sGP;
            pt = sGP.GetRandPointOnMoon(pOtherClient == nullptr ? pClient->GetSystemID() : pOtherClient->GetSystemID());
            //throw PyException(MakeCustomError("Translocate: This option is Incomplete"));
        } else if (strcmp(args.arg(2).c_str(), "planet") == 0) {
            // random planet in client current system
            SystemGPoint sGP;
            pt = sGP.GetRandPointOnPlanet(pOtherClient == nullptr ? pClient->GetSystemID() : pOtherClient->GetSystemID());
            //throw PyException(MakeCustomError("Translocate: This option is Incomplete"));
        } else {
            // what are we missing?
            throw PyException(MakeCustomError("Translocate: Invalid Arguments"));
        }

        // decode third arg     - last, moon, planet, {invalid}
        if (args.isNumber(3)) {
            // sending destination as itemID?
            locationID = atoi(args.arg(2).c_str());
        } else if (strcmp(args.arg(3).c_str(), "last") == 0) {
            if (fleet)
                throw PyException(MakeCustomError("Translocate: Fleet Move - You cannot tr a fleet to last docked station...yet."));
            //  tr <player> me last -- tr <player> to my last docked station
            // last station client was docked in
            // if called with <player name> then uses that char's last station
            if (pOtherClient != nullptr)
                locationID = pOtherClient->GetDockStationID();
            else
                locationID = pClient->GetDockStationID();
        } else if (strcmp(args.arg(3).c_str(), "moon") == 0) {
            // random moon in client current system
            SystemGPoint sGP;
            pt = sGP.GetRandPointOnMoon(pOtherClient == nullptr ? pClient->GetSystemID() : pOtherClient->GetSystemID());
            //throw PyException(MakeCustomError("Translocate: This option is Incomplete"));
        } else if (strcmp(args.arg(3).c_str(), "planet") == 0) {
            // random planet in client current system
            SystemGPoint sGP;
            pt = sGP.GetRandPointOnPlanet(pOtherClient == nullptr ? pClient->GetSystemID() : pOtherClient->GetSystemID());
            //throw PyException(MakeCustomError("Translocate: This option is Incomplete"));
        } else {
            // what are we missing?
            throw PyException(MakeCustomError("Translocate: Invalid Arguments"));
        }

    } else if (args.argCount() == 5) { // 4 args - me, player, ship,  coords, {invalid} : me, home, last, location, {invalid} : moon, planet, {invalid}
        // test for 'help' command
        if ((strcmp(args.arg(1).c_str(), Help) == 0)
        or (strcmp(args.arg(2).c_str(), Help) == 0)
        or (strcmp(args.arg(3).c_str(), Help) == 0)
        or (strcmp(args.arg(4).c_str(), Help) == 0))
            throw PyException(MakeCustomError("Translocate: Invalid Arguments - 'help' not avalible for this command."));

        // check for coords
        if ((args.isNumber(1))
        and (args.isNumber(2))
        and (args.isNumber(3))
        and (args.isNumber(4))) {
            // pClient is caller, args are systemID and coords (assumed)
            locationID = atoi(args.arg(1).c_str());
            if (!IsSolarSystem(locationID))
                throw PyException(MakeCustomError("Translocate: Invalid Arguments - locationID %u is not a SolarSystemID.", locationID));
            pt = GPoint(atoi(args.arg(2).c_str()), atoi(args.arg(3).c_str()), atoi(args.arg(4).c_str()));

            pClient->JumpOutEffect(locationID);
            pClient->MoveToLocation(locationID, pt);
            pClient->JumpInEffect();

            return nullptr;
        }

        throw PyException(MakeCustomError("Translocate: 4 args - This command is currently incomplete."));
    } else if (args.argCount() == 6) { // 5 args - me, player, ship, {invalid} : me, home, last, location,  coords, {invalid} : moon, planet, {invalid}
        // test for 'help' command
        if ((strcmp(args.arg(1).c_str(), Help) == 0)
        or (strcmp(args.arg(2).c_str(), Help) == 0)
        or (strcmp(args.arg(3).c_str(), Help) == 0)
        or (strcmp(args.arg(4).c_str(), Help) == 0)
        or (strcmp(args.arg(5).c_str(), Help) == 0))
            throw PyException(MakeCustomError("Translocate: Invalid Arguments - 'help' not avalible for this command."));

        throw PyException(MakeCustomError("Translocate: 5 args - This command is currently incomplete."));
    } else {
        throw PyException(MakeCustomError("Translocate: Too Many Arguments"));
    }

    if (fleet) {
        // this will take a lil bit of doing....
        throw PyException(MakeCustomError("Translocate: Fleet Move - This command is currently incomplete."));
    }

//  need to test locationID here to verify its valid.

    if (!IsValidLocation(locationID))
        throw PyException(MakeCustomError("Translocate: Invalid Location %i", locationID));

    if (IsPlayerItem(locationID)) {
        // locationID is sent as player item.  this can be any celestial object, ship, pos, jetcan, wreck, etc.
        // this will take a lil bit of doing to get booleans right.
        InventoryItemRef iRef = sItemFactory.GetItem(locationID);
        if (item) { // tr to ship/item location
            pt = iRef->position();
            locationID = iRef->locationID();
        } else { // tr to ship/item owner location
            Client* pClient2 = sEntityList.FindClientByCharID(iRef->ownerID());
            if (pClient2 == nullptr) {
                pt = iRef->position();
                locationID = iRef->locationID();
            } else if (pClient2->IsDocked()) {
                pt = NULL_ORIGIN;
                locationID = pClient2->GetLocationID();
            } else {
                pt = pClient2->GetShipSE()->GetPosition();
                pt.MakeRandomPointOnSphere(100 + pClient->GetShipSE()->GetRadius() + pClient2->GetShipSE()->GetRadius());
                locationID = pClient2->GetSystemID();
            }
        }
    }

    if (pOtherClient != nullptr) {
        pOtherClient->JumpOutEffect(locationID);
        pOtherClient->MoveToLocation(locationID, pt);
        pOtherClient->JumpInEffect();
    } else {
        pClient->JumpOutEffect(locationID);
        pClient->MoveToLocation(locationID, pt);
        pClient->JumpInEffect();
    }
    return nullptr;
}

static PyResult generic_createitem(Client *pClient, CommandDB *db, PyServiceMgr *services, const Seperator &args) {

    int typeID = -1;
    if (args.isNumber(1)) {
        typeID = atoi(args.arg(1).c_str());
    } else {
        // this hits db directly, so test for possible sql injection code
        for (const auto cur : badCharsSearch)
            if (EvE::icontains(args.arg(1), cur))
                throw PyException( MakeCustomError("Name contains invalid characters"));
        std::map<uint32_t, std::string> matches;
        if (!db->ItemSearch(args.arg(1).c_str(), matches))
            throw PyException(MakeCustomError("Item not found"));

        if (matches.size() > 1) {
            auto c = matches.begin();
            auto e = matches.end();
            for (; c != e; c++) {
                _log(COMMAND__MESSAGE, "Got match: %s\n", c->second.c_str());

                // POSIX standard btw
                if (strcasecmp(c->second.c_str(), args.arg(1).c_str()) == 0)
                    typeID = c->first;
            }
            if (typeID == -1)
                throw PyException(MakeCustomError("Item name is ambiguous.  Please use a full item name"));

        } else if (matches.size() == 1) {
            auto cur = matches.begin();
            _log(COMMAND__MESSAGE,
                 "ItemSearch returned type: \"%s\" given \"%s\"\n",
                 cur->second.c_str(), args.arg(1).c_str());
            typeID = cur->first;
        }
    }
    if (typeID == -1)
        throw PyException(MakeCustomError("Unable to find valid type to create"));

    if (typeID < 34)
        throw PyException(MakeCustomError("Invalid Type ID."));

    int qty = 1;
    if (2 < args.argCount()) {
        if (args.isNumber(2))
            qty = atoi(args.arg(2).c_str());
    }

    _log(COMMAND__MESSAGE, "Create %s %u times", args.arg(1).c_str(), qty);

    //create into their cargo hold unless they are docked in a station,
    //then stick it in their hangar instead.
    uint32 locationID = 0;
    EVEItemFlags flag;
    if (pClient->IsInSpace()) {
        locationID = pClient->GetShipID();
        flag = flagCargoHold;
    } else {
        locationID = pClient->GetStationID();
        flag = flagHangar;
    }

    ItemData idata(
        typeID,
        pClient->GetCharacterID(),
                   locTemp, //temp location
                   flag,
                   qty
    );

    InventoryItemRef iRef = sItemFactory.SpawnItem(idata);
    if (iRef.get() == nullptr)
        throw PyException(MakeCustomError("Unable to create item of type %s.", args.arg(1).c_str()));

    //Move to location
    if (pClient->IsInSpace())
        pClient->GetShip()->AddItemByFlag(flag, iRef);
    else
        iRef->Move(locationID, flag, true);

    iRef->SaveItem();

    return new PyInt(iRef.get()->itemID());
}

PyResult Command_create(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    if (args.argCount() < 2) {
        throw PyException(MakeCustomError("Correct Usage: /create [typeID|\"Type Name\"] [qty] [where]"));
    }
    return generic_createitem(pClient, db, services, args);
}

PyResult Command_createitem(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    if (args.argCount() < 2) {
        throw PyException(MakeCustomError("Correct Usage: /createitem [typeID|\"Type Name\"] [qty] [where]"));
    }
    return generic_createitem(pClient, db, services, args);
}


PyResult Command_kill(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount() == 2) {
        if (!args.isNumber(1)) {
            throw PyException(MakeCustomError("Argument 1 should be a character ID"));
        }
        int entity = atoi(args.arg(1).c_str());

        InventoryItemRef itemRef = sItemFactory.GetShip(entity);
        if (itemRef.get() == NULL)
            throw PyException(MakeCustomError("/kill NOT supported on non-ship types at this time"));

        SystemEntity* shipEntity = pClient->SystemMgr()->GetSE(entity);
        if (shipEntity == nullptr) {
            throw PyException(MakeCustomError("/kill cannot process this object"));
            sLog.Error("GMCommands - Command_kill()", "Cannot process this object, aborting kill: %s [%u]", itemRef->name(), itemRef->itemID());
        } else {
            pClient->SystemMgr()->RemoveEntity(shipEntity);
            if (shipEntity->IsNPCSE()) {
                NPC* npcEntity = shipEntity->GetNPCSE();
                Damage fatal_blow(pClient->GetShipSE(),true);
                npcEntity->Killed(fatal_blow);
                delete npcEntity;
            } else {
                Damage fatal_blow(pClient->GetShipSE(),true);
                shipEntity->Killed(fatal_blow);
                itemRef->Delete();
            }
        }
    } else
        throw PyException(MakeCustomError("Correct Usage: /kill <entityID>"));

    return nullptr;
}

PyResult Command_killallnpcs(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!pClient->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (args.argCount() != 1)
        throw PyException(MakeCustomError("Correct Usage: /killallnpcs"));
    if (pClient->GetShipSE() == nullptr)
        throw PyException(MakeCustomError("ShipSE invalid."));
    if (pClient->GetShipSE()->SysBubble() == nullptr)
        throw PyException(MakeCustomError("SysBubble invalid."));

    std::map< uint32, SystemEntity* > entityVec;
    pClient->GetShipSE()->SysBubble()->GetEntities(entityVec);
    for (auto cur : entityVec) {
        if (cur.second == nullptr)
            continue;
        if (cur.second->IsNPCSE()) {
            Damage fatal_blow(pClient->GetShipSE(),true);
            cur.second->GetNPCSE()->Killed(fatal_blow);
        }
    }

    return nullptr;
}

PyResult Command_unspawn(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
#define DEFAULT_RANGE 500000
    if (!pClient->IsInSpace()) {
        throw PyException(MakeCustomError("You must be in space to unspawn things."));
    }

    if (pClient->GetShipSE() == nullptr) {
            throw PyException(MakeCustomError("/unspawn failed. You don't appear to have a ship?"));
    }
    int target_index = cmd_find_nth_noneq(args, 1);
    uint32 target = 0;
    if (target_index > 0) {
        if (!args.isNumber(target_index)) {
            throw PyException(MakeCustomError("/unspawn called with non number"));
        }
        target = atoi(args.arg(target_index).c_str());
    }

    std::string range_str = cmd_parse_eq_arg(args, "range=");
    std::string only_str = cmd_parse_eq_arg(args, "only=");

    codelog(COMMAND__ERROR, "unspawn got: %s %s %u",
            range_str.c_str(), only_str.c_str(), target);

    uint32 range = DEFAULT_RANGE;

    if (range_str.size() > 0) {
        if (!IsNumber(range_str)) {
            throw PyException(MakeCustomError("/unspawn with range=x must be a number"));
        }
        range = atoi(range_str.c_str());
    }

    if ((range != DEFAULT_RANGE or
            only_str.size() > 0) and
            target != 0) {
            throw PyException(MakeCustomError("/unspawn cannot be called with an explcit target and either range= or only="));
    }

    if (target != 0) {
        InventoryItemRef item_ref = sItemFactory.GetItem(target);
        SystemEntity *sys_entity = pClient->SystemMgr()->GetSE(target);
        if (sys_entity == nullptr) {
            throw PyException(MakeCustomError("/unspawn failed.  Item %u not found.", target));
        }

        pClient->SystemMgr()->RemoveEntity(sys_entity);
        item_ref->Delete();
        codelog(COMMAND__MESSAGE, "/unspawn called with single target successful");
        return new PyBool(true);
    }

    if (only_str.size() == 0) {
        throw PyException(MakeCustomError("/unspawn usage:<br>  /unspawn [itemID]<br>/unspawn only=category|group<br>If using only the default range is 10k.  You can set this by adding range=x in meters"));
    }

    bool is_category_match = false;
    bool is_group_match = false;
    uint16 match_id = 0;

    if (strcmp(only_str.c_str(), "categoryDrone") == 0) {
        match_id = EVEDB::invCategories::Drone;
        is_category_match = true;
    } else if (strcmp(only_str.c_str(), "groupWreck") == 0) {
        match_id = EVEDB::invGroups::Wreck;
        is_group_match = true;
    } else {
        throw PyException(MakeCustomError("only='%s' not a supported group or category", only_str.c_str()));
    }

    SystemBubble *pBubble = pClient->GetShipSE()->SysBubble();
    if (pBubble == nullptr) {
        throw PyException(MakeCustomError("/unspawn failed.  You don't appear to be in a bubble.  Try /update"));
    }

    GPoint player_pos = pClient->GetShipSE()->GetPosition();

    std::map< uint32, SystemEntity* > entMap;
    pBubble->GetEntities(entMap);
    for (auto cur : entMap) {
        if (is_group_match and match_id != cur.second->GetGroupID()) {
            codelog(COMMAND__ERROR, "m: g%d c%d skipping match_id %u groupID %u",
                    is_group_match, is_category_match, match_id, cur.second->GetGroupID());
            continue;
        }
        if (is_category_match and match_id != cur.second->GetCategoryID()) {
            codelog(COMMAND__ERROR, "m: g%d c%d skipping match_id %u categoryID %u",
                    is_group_match, is_category_match, match_id, cur.second->GetGroupID());
            continue;
        }

        if (player_pos.distance(cur.second->GetPosition()) > range)
            continue;

        cur.second->Delete();
    }

#undef DEFAULT_RANGE

    return new PyBool(true);
}

PyResult Command_location(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!pClient->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (pClient->GetShipSE()->DestinyMgr() == nullptr)
        pClient->SetDestiny(NULL_ORIGIN);
    if (pClient->GetShipSE()->SysBubble() == nullptr)
        pClient->EnterSystem(pClient->GetSystemID());

    DestinyManager *dm = pClient->GetShipSE()->DestinyMgr();
    SystemBubble *pBubble = pClient->GetShipSE()->SysBubble();
    if (pBubble == nullptr) {
        sBubbleMgr.Add(pClient->GetShipSE());
        pBubble = pClient->GetShipSE()->SysBubble();
    }
    uint16 bubble = pBubble->GetID();

    const GPoint &loc = dm->GetPosition();
    const GVector &vel = dm->GetVelocity();

    char reply[140];
    snprintf(reply, 140,
             "SystemID: %u  BubbleID: %u<br>"
             "x: %.2f<br>"
             "y: %.2f<br>"
             "z: %.2f<br>"
             "speed: %.1f",
             pClient->GetSystemID(), bubble,
             loc.x, loc.y, loc.z,
             vel.length()
    );

    pClient->SendInfoModalMsg(reply);

    return new PyString(reply);
}

PyResult Command_syncloc(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!pClient->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (pClient->GetShipSE()->DestinyMgr() == nullptr)
        pClient->SetDestiny(NULL_ORIGIN);
    if (pClient->GetShipSE()->SysBubble() == nullptr)
        pClient->EnterSystem(pClient->GetSystemID());
    if (pClient->IsSessionChange()) {
        pClient->SendInfoModalMsg("Session Change Active.  Wait %u seconds before issuing another command.",
                                  pClient->GetSessionChangeTime());
        return new PyString("SessionChange Active.  Request Denied.");
    }

    pClient->GetShipSE()->DestinyMgr()->SetPosition(pClient->GetShipSE()->GetPosition(), true);
    pClient->SetSessionTimer();

    return new PyString("Position synchronized.");
}

PyResult Command_syncpos(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!pClient->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (pClient->GetShipSE()->DestinyMgr() == nullptr)
        pClient->SetDestiny(NULL_ORIGIN);
    if (pClient->GetShipSE()->SysBubble() == nullptr)
        pClient->EnterSystem(pClient->GetSystemID());
    if (pClient->IsSessionChange()) {
        pClient->SendInfoModalMsg("Session Change Active.  Wait %u seconds before issuing another command.",
                                  pClient->GetSessionChangeTime());
        return new PyString("SessionChange Active.  Request Denied.");
    }

    pClient->GetShipSE()->SysBubble()->SyncPos();
    pClient->SetSessionTimer();

    return new PyString("All Positions synchronized.");
}

PyResult Command_update(Client *pClient, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
    if (!pClient->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (pClient->GetShipSE()->DestinyMgr() == nullptr)
        pClient->SetDestiny(NULL_ORIGIN);
    if (pClient->GetShipSE()->SysBubble() == nullptr)
        pClient->EnterSystem(pClient->GetSystemID());
    if (pClient->IsSessionChange()) {
        pClient->SendInfoModalMsg("Session Change Active.  Wait %u seconds before issuing another command.",
                                  pClient->GetSessionChangeTime());
        return new PyString("SessionChange Active.  Request Denied.");
    }

    pClient->GetShipSE()->DestinyMgr()->SetPosition(pClient->GetShipSE()->GetPosition(), true);

    SystemBubble *pBubble = pClient->GetShipSE()->SysBubble();
    if (pBubble == nullptr) {
        sBubbleMgr.Add(pClient->GetShipSE());
        pBubble = pClient->GetShipSE()->SysBubble();
    }
    pBubble->SendAddBalls(pClient->GetShipSE());

    pClient->SetStateSent(false);
    pClient->GetShipSE()->DestinyMgr()->SendSetState();
    pClient->SetSessionTimer();
    return new PyString("Update sent.");
}

PyResult Command_sendstate(Client *pClient, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
    if (!pClient->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (pClient->GetShipSE()->DestinyMgr() == nullptr)
        pClient->SetDestiny(NULL_ORIGIN);
    if (pClient->GetShipSE()->SysBubble() == nullptr)
        pClient->EnterSystem(pClient->GetSystemID());
    if (pClient->IsSessionChange()) {
        pClient->SendInfoModalMsg("Session Change Active.  Wait %u seconds before issuing another command.",
                                  pClient->GetSessionChangeTime());
        return new PyString("SessionChange Active.  Request Denied.");
    }

    pClient->SetStateSent(false);
    pClient->GetShipSE()->DestinyMgr()->SendSetState();
    pClient->SetSessionTimer();
    return new PyString("Update sent.");
}

PyResult Command_addball(Client *pClient, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
    if (!pClient->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (pClient->GetShipSE()->DestinyMgr() == nullptr)
        pClient->SetDestiny(NULL_ORIGIN);
    if (pClient->GetShipSE()->SysBubble() == nullptr)
        pClient->EnterSystem(pClient->GetSystemID());

    SystemBubble *pBubble = pClient->GetShipSE()->SysBubble();
    if (pBubble == nullptr) {
        sBubbleMgr.Add(pClient->GetShipSE());
        pBubble = pClient->GetShipSE()->SysBubble();
    }
    pBubble->SendAddBalls(pClient->GetShipSE());

    return new PyString("Update sent.");
}

PyResult Command_addball2(Client *pClient, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
    if (!pClient->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (pClient->GetShipSE()->DestinyMgr() == nullptr)
        pClient->SetDestiny(NULL_ORIGIN);
    if (pClient->GetShipSE()->SysBubble() == nullptr)
        pClient->EnterSystem(pClient->GetSystemID());

    SystemBubble *pBubble = pClient->GetShipSE()->SysBubble();
    if (pBubble == nullptr) {
        sBubbleMgr.Add(pClient->GetShipSE());
        pBubble = pClient->GetShipSE()->SysBubble();
    }
    pBubble->SendAddBalls2(pClient->GetShipSE());

    return new PyString("Update sent.");
}

PyResult Command_cloak(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount() == 1) {
        if (pClient->IsInSpace()) {
            if (pClient->GetShipSE()->DestinyMgr()->IsCloaked())
                pClient->GetShipSE()->DestinyMgr()->UnCloak();
            else
                pClient->GetShipSE()->DestinyMgr()->Cloak();
        } else
            throw PyException(MakeCustomError("ERROR!  You MUST be in space to cloak!"));
    } else
        throw PyException(MakeCustomError("Correct Usage: /cloak"));

    return nullptr;
}

PyResult Command_hop(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /*22:49:01 W GMCommands: Command_hop(): This command passes args.argCount() = 1.
     * sm.RemoteSvc('slash').SlashCmd('/hop %s' % distance)
     */
    return nullptr;
}

//13:54:11 W GMCommands: Command_sov(): This command passes args.argCount() = 3.
PyResult Command_sov(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    sLog.Warning("GMCommands: Command_sov()", "This command passes args.argCount() = %u.", args.argCount());
    /*
     *  ' /sov complete ' + str(itemID)
     */
    /*
     * 16:40:32 [CmdDump]   Call Arguments:
     * 16:40:32 [CmdDump]       Tuple: 1 elements
     * 16:40:32 [CmdDump]         [ 0] String: '/sov complete 140035963'
     */
    return nullptr;
}

//13:54:11 W GMCommands: Command_pos(): This command passes args.argCount() = 3.
PyResult Command_pos(Client* pClient, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    sLog.Warning("GMCommands: Command_pos()", "This command passes args.argCount() = %u.", args.argCount());

    /*
     * ' /pos online ' + str(itemID)
     * ' /pos unanchor ' + str(itemID)
     * ' /pos anchor ' + str(itemID)
     * ' /pos offline ' + str(itemID)
     * ' /pos fuel [itemID]
     */

    /*
     * 16:39:26 [CmdDump]   Call Arguments:
     * 16:39:26 [CmdDump]       Tuple: 1 elements
     * 16:39:26 [CmdDump]         [ 0] String: '/pos offline 140035963'
     */
    return nullptr;
}
