 /*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        Zhur
    Updates:        Allan
*/

/** @todo this file needs to be updated... */


#include "eve-server.h"
//#include "../../eve-common/EVE_Skills.h"

#include "Client.h"
#include "ConsoleCommands.h"
#include "admin/AllCommands.h"
#include "admin/CommandDB.h"
#include "inventory/AttributeEnum.h"
#include "inventory/InventoryDB.h"
#include "inventory/InventoryItem.h"
#include "manufacturing/Blueprint.h"
#include "map/MapConnections.h"
#include "npc/Drone.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "system/Damage.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "corporation/LPService.h"
#include "corporation/CorporationDB.h"

PyResult Command_search(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args) {
    if (args.argCount() < 2) {
        throw CustomError ("Correct Usage: /search [text]");
    }

    const std::string& query = args.arg(1);

    //an empty query is a bad idea.
    if (query.length() == 0)
        throw CustomError ("Usage: /search [text]");

    std::map<uint32, std::string> matches;
    if (!db->ItemSearch(query.c_str(), matches))
        throw CustomError ("Failed to query DB.");

    std::string result(std::to_string(matches.size()));
    result += " matches found.<br>";

    std::map<uint32, std::string>::iterator cur, end;
    cur = matches.begin();
    end = matches.end();
    for(; cur != end; cur++) {
        result += std::to_string(cur->first);
        result += ": ";
        result += cur->second;
        result += "<br>";
    }

    if (10 < matches.size()) {
        //send the results in an evemail.
        std::string subject("Search results for ");
        subject += query;

        who->SelfEveMail(subject.c_str(), result.c_str());

        return new PyString("Results sent via evemail.");
    } else
        return new PyString(result);
}

PyResult Command_giveisk(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    if (args.argCount() < 3) {
        throw CustomError ("Correct Usage: /giveisk [entityID ('me'=self)] [amount]");
    }

    // Check for target (arg #1) for either a number or the string "me":
    std::string target = "";
    if (!args.isNumber(1))
    {
        target = args.arg(1);
        if (target != "me")
            throw CustomError ("Argument 1 should be an entity ID ('me'=self)");
    }

    // If target (arg #1) is not the string "me" then decode number from argument string, otherwise get this character's ID:
    uint32 entity;
    if (target == "")
        entity = atol(args.arg(1).c_str());
    else
        entity = who->GetCharacterID();

    if (!args.isNumber(2))
        throw CustomError ("Argument 2 should be an amount of ISK");
    double amount = strtod(args.arg(2).c_str(), NULL);

    Client* tgt;
    if (entity >= maxNPCItem)
    {
        tgt = sEntityList.FindClientByCharID(entity);
        if (!tgt)
            throw CustomError ("Unable to find character %u", entity);
    }
    else
        throw CustomError ("Invalid entityID for characters %u", entity);
/** @todo update this */
    // GiveCash
    tgt->AddBalance(amount, Account::CreditType::ISK);
    return new PyString("Operation successful.");
}

PyResult Command_pop(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    if (4 != args.argCount())
        throw CustomError ("Correct Usage: /pop [message type] [key] [text]");

    //CustomNotify: notify
    //ServerMessage: msg
    //CustomError: error

    const std::string& msgType = args.arg(1);
    const std::string& key = args.arg(2);
    const std::string& text = args.arg(3);

    Notify_OnRemoteMessage n;
    n.msgType = msgType;
    n.args[ key ] = new PyString(text);

    PyTuple* t = n.Encode();
    who->SendNotification("OnRemoteMessage", "charid", &t);
    PySafeDecRef(t);

    return new PyString("Message sent.");
}

PyResult Command_spawnn(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    int typeID = 0;
    uint32 actualTypeID = 0;
    std::string actualTypeName = "";
    uint32 actualGroupID = 0;
    uint32 actualCategoryID = 0;
    double actualRadius = 0.0;
    InventoryItemRef item;
    ShipItemRef ship;

    // Updated(groove)
    // "/spawnn" arguments:
    // #1 = quantity ?
    // #2 = some double value deviation
    // #3 = typeID

    if ((args.argCount() < 4) || (args.argCount() > 4))
    {
        throw CustomError ("LOL, we don't know the correct usage of /spawnn, sorry you're S.O.L., BUT it should have 4 arguments.");
    }

    // Since we don't know what args 1 and 2 are, we don't care about them right now...

    if (!args.isNumber(3))
        throw CustomError ("Argument 3 should be an item type ID");

    typeID = atoi(args.arg(3).c_str());
    if (typeID < 34)
        throw CustomError ("Invalid Type ID.");

    if (!who->IsInSpace())
        throw CustomError ("You must be in space to spawn things.");

    // Search for item type using typeID:
    if (!db->ItemSearch(typeID, actualTypeID, actualTypeName, actualGroupID, actualCategoryID, actualRadius)) {
        return new PyString("Unknown typeID or typeName returned no matches.");
    }

    GPoint loc(who->GetShipSE()->GetPosition());
    // Calculate a random coordinate on the sphere centered on the player's position with
    // a radius equal to the radius of the ship/celestial being spawned times 10 for really good measure of separation:
    double radius = (actualRadius * 5.0) * (double)(MakeRandomInt(1, 3));     // Scale the distance from player that the object will spawn to between 10x and 15x the object's radius
    loc.MakeRandomPointOnSphere(radius);

    // Spawn the item:
    ItemData idata(
        actualTypeID,
        1, // owner is EVE System
        who->GetLocationID(),
        flagNone,
        actualTypeName.c_str(),
        loc
   );

    item = sItemFactory.SpawnItem(idata);
    if (item.get() == nullptr)
        throw CustomError ("Unable to spawn item of type %u.", typeID);

    DBSystemDynamicEntity entity = DBSystemDynamicEntity();
        entity.categoryID = actualCategoryID;
        entity.groupID = actualGroupID;
        entity.itemID = item->itemID();
        entity.itemName = actualTypeName;
        entity.typeID = actualTypeID;
        entity.position = loc;
        /** @todo need to get faction info here... */
        entity.allianceID = 1;
        entity.corporationID = 1;
        entity.factionID = 1;
        entity.ownerID = 1;

    // Actually do the spawn using SystemManager's BuildEntity:
    if (!who->SystemMgr()->BuildDynamicEntity(entity))
        return new PyString("Spawn Failed: typeID or typeName not supported.");

    sLog.Yellow("Command", "%s: Spawned %u.", who->GetName(), typeID);

    return new PyString("Spawn successful.");
}

PyResult Command_spawn(Client* pClient, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    int spawnCount = 1;
    uint32 spawnIndex = 0;
    uint32 maximumSpawnCountAllowed = 100;
    uint32 actualTypeID = 0;
    std::string actualTypeName = "";
    uint32 actualGroupID = 0;
    uint32 actualCategoryID = 0;
    double actualRadius = 0.0;
    InventoryItemRef item;
    ShipItemRef ship;
    double radius;
    bool offsetLocationSet = false;
    std::string usage = "Correct Usage: <br><br> /spawn [typeID(int)/typeName(string)] <br><br>With optional spawn count: <br> /spawn [typeID(int)/typeName(string)] [count] <br><br>With optional count and (X,Y,Z) coordinate: <br> /spawn [typeID(int/typeName(string)] [count] [x(float)] [y(float)] [z(float)]";

    if (!pClient->IsInSpace())
        throw CustomError ("You must be in space to spawn things.");

    if (args.argCount() < 2) {
        throw CustomError (usage.c_str());
    }

    if (!args.isNumber(1))
        throw CustomError ("Argument 1 should be an item type ID");

    int typeID = atoi(args.arg(1).c_str());

    // Search for item type using typeID:
    if (!(db->ItemSearch(typeID, actualTypeID, actualTypeName, actualGroupID, actualCategoryID, actualRadius)))
    {
        return new PyString("Unknown typeID or typeName returned no matches.");
    }

    if (args.argCount() > 2)
    {
        if (!(args.isNumber(2)))
            throw CustomError ("Argument 3 should be the number of spawns of this type you want to create");

        spawnCount = atoi(args.arg(2).c_str());
        if (spawnCount > maximumSpawnCountAllowed)
            throw CustomError ("Argument 3, spawn count, is allowed to be no more than 100");
    }

    // Check to see if the X Y Z optional coordinates were supplied with the command:
    GPoint offsetLocation;
    if (args.argCount() > 3)
    {
        if (!(args.isNumber(3)))
            throw CustomError ("Argument 4 should be the X distance from your ship in meters you want the item spawned");

        if (args.argCount() > 4)
        {
            if (!(args.isNumber(4)))
                throw CustomError ("Argument 5 should be the Y distance from your ship in meters you want the item spawned");
        }
        else
            throw CustomError ("TOO FEW PARAMETERS: %s", usage.c_str());

        if (args.argCount() > 5)
        {
            if (!(args.isNumber(5)))
                throw CustomError ("Argument 6 should be the Z distance from your ship in meters you want the item spawned");
        }
        else
            throw CustomError ("TOO FEW PARAMETERS: %s", usage.c_str());

        offsetLocation.x = atoll(args.arg(3).c_str());
        offsetLocation.y = atoll(args.arg(4).c_str());
        offsetLocation.z = atoll(args.arg(5).c_str());
        offsetLocationSet = true;
    }

    GPoint loc;

    for(spawnIndex=0; spawnIndex < spawnCount; ++spawnIndex)
    {
        loc = pClient->GetShipSE()->GetPosition();

        if (offsetLocationSet)
        {
            // An X, Y, Z coordinate offset was specified along with the command, so use this to calculate
            // the final cooridnate of the newly spawned item:
            loc.x += offsetLocation.x;
            loc.y += offsetLocation.y;
            loc.z += offsetLocation.z;
        }
        else
        {
            // Calculate a random coordinate on the sphere centered on the player's position with
            // a radius equal to the radius of the ship/celestial being spawned times 10 for really good measure of separation:
            radius = (actualRadius * 5.0) * (double)(MakeRandomInt(1, 3));     // Scale the distance from player that the object will spawn to between 10x and 15x the object's radius
            loc.MakeRandomPointOnSphere(radius);
        }

        // Spawn the item:
        ItemData idata(
            actualTypeID,
            1, // owner is EVE System
            pClient->GetLocationID(),
            flagNone,
            actualTypeName.c_str(),
            loc
       );

        item = sItemFactory.SpawnItem(idata);
        if (item.get() == nullptr)
            throw CustomError ("Unable to spawn item of type %u.", typeID);

        DBSystemDynamicEntity entity = DBSystemDynamicEntity();
        entity.categoryID = actualCategoryID;
        entity.groupID = actualGroupID;
        entity.itemID = item->itemID();
        entity.itemName = actualTypeName;
        entity.typeID = actualTypeID;
        entity.position = loc;
        // set owner data according to type and location
        switch (actualCategoryID) {
            case EVEDB::invCategories::Entity: {
                // owned by system rats
                entity.factionID = sDataMgr.GetRegionRatFaction(pClient->GetRegionID());
                entity.allianceID = entity.factionID;
                entity.corporationID = sDataMgr.GetFactionCorp(entity.factionID);
                entity.ownerID = entity.corporationID;
            } break;
            case EVEDB::invCategories::Ship: {
                // owned by calling char
                entity.allianceID = pClient->GetAllianceID();
                entity.corporationID = pClient->GetCorporationID();
                entity.factionID = pClient->GetWarFactionID();
                entity.ownerID = pClient->GetCharacterID();
            } break;
            default: {
                //owned by system sov holder
                entity.factionID = sDataMgr.GetRegionFaction(pClient->GetRegionID());
                entity.allianceID = entity.factionID;
                entity.corporationID = sDataMgr.GetFactionCorp(entity.factionID);
                entity.ownerID = entity.corporationID;
            } break;
        }


        // Actually do the spawn using SystemManager's BuildEntity:
        if (!pClient->SystemMgr()->BuildDynamicEntity(entity)) {
            return new PyString("Spawn Failed: typeID or typeName not supported.");
        }
    }

    sLog.Yellow("Command_spawn", "%s: Spawned %u in space, %u times", pClient->GetName(), typeID, spawnCount);

    return new PyString("Spawn successful.");
}

// command to modify blueprint's attributes, we have to give it blueprint's itemID ...
// isn't much comfortable, but I don't know about better solution ...
PyResult Command_setbpattr(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    if (args.argCount() < 6)
        throw CustomError ("Correct Usage: /setbpattr [blueprintID] [0 (not copy) or 1 (copy)] [material level] [productivity level] [remaining runs]");
    if (!args.isNumber(1))
        throw CustomError ("Argument 1 must be blueprint ID. (got %s)", args.arg(1).c_str());
    if ("0" != args.arg(2) && "1" != args.arg(2))
        throw CustomError ("Argument 2 must be 0 (original) or 1 (copy). (got %s)", args.arg(2).c_str());
    if (!args.isNumber(3))
        throw CustomError ("Argument 3 must be material level. (got %s)", args.arg(3).c_str());
    if (!args.isNumber(4))
        throw CustomError ("Argument 4 must be productivity level. (got %s)", args.arg(4).c_str());
    if (!args.isNumber(5))
        throw CustomError ("Argument 5 must be remaining licensed production runs. (got %s)", args.arg(5).c_str());

    int blueprintID = atoi(args.arg(1).c_str());
    BlueprintRef bp = sItemFactory.GetBlueprintRef(blueprintID);
    if (!bp)
        throw CustomError ("Failed to load blueprint %u.", blueprintID);

    // these need to check current settings to see if anything changed
    bp->SetCopy((atoi(args.arg(2).c_str()) ? true : false));
    bp->SetMLevel(atoi(args.arg(3).c_str()));
    bp->SetPLevel(atoi(args.arg(4).c_str()));
    bp->SetRuns(atoi(args.arg(5).c_str()));

    return new PyString("Properties modified.");
}

PyResult Command_getattr(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    /** @todo update to new attrib system
    if (args.argCount() < 3) {
        throw CustomError ("Correct Usage: /getattr [itemID] [attributeID]");
    }
    if (!args.isNumber(1))
        throw PyException(MakeCustomError("1st argument must be itemID (got %s).", args.arg(1).c_str()));
    const uint32 itemID = atoi(args.arg(1).c_str());

    if (!args.isNumber(2))
        throw PyException(MakeCustomError("2nd argument must be attributeID (got %s).", args.arg(2).c_str()));
    const ItemAttributeMgr::Attr attribute = (ItemAttributeMgr::Attr)atoi(args.arg(2).c_str());

    InventoryItemRef item = sItemFactory.GetItemRef(itemID);
    if (item.get() == nullptr)
        throw PyException(MakeCustomError("Failed to load item %u.", itemID));
    */
    //return item->attributes.PyGet(attribute);
    //return item->GetAttribute(attribute).GetPyObject();
    return nullptr;
}

PyResult Command_setattr(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    throw CustomError ("disabled");
    if (0) {
    if (args.argCount() < 4) {
        throw CustomError ("Correct Usage: /setattr [itemID] [attributeID] [value]");
    }

    // Check for target (arg #1) for either a number or the string "myship":
    int itemID = 0;
    std::string target = "";
    if (!args.isNumber(1))
    {
        target = args.arg(1);
        if (target != "myship")
            throw CustomError ("1st argument should be an entity ID ('myship'=current ship) (got %s).", args.arg(1).c_str());

        itemID = who->GetShipID();
    }
    else
    {
        // target (arg #1) is a number, so decode it and move on:
        itemID = atoi(args.arg(1).c_str());
    }

    if (!args.isNumber(2))
        throw CustomError ("2nd argument must be attributeID (got %s).", args.arg(2).c_str());
    //const ItemAttributeMgr::Attr attribute = (ItemAttributeMgr::Attr)atoi(args.arg(2).c_str());

    if (!args.isNumber(3))
        throw CustomError ("3rd argument must be value (got %s).", args.arg(3).c_str());
    //const double value = atof(args.arg(3).c_str());

    if (itemID < minPlayerItem)
        throw CustomError ("1st argument must be a valid 'entity' table itemID that MUST be larger >= 140000000. (got %s)", args.arg(1).c_str());

    InventoryItemRef item = sItemFactory.GetItemRef(itemID);
    if (item.get() == nullptr)
        throw CustomError ("Failed to load item %u.", itemID);

    //item->attributes.SetReal(attribute, value);
    sLog.Warning("GMCommands: Command_setattr()", "This command will modify attribute and send change to client, but change does not take effect in client for some reason.");
   // item->SetAttribute(attribute, (float)value);

    return new PyString("Operation successful.");
    }
}

PyResult Command_fit(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    throw CustomError ("This command is currently disabled.");
/*
    uint32 typeID = 0;
    uint32 itemID = 0;
    EVEItemFlags flag = (EVEItemFlags)0;
    uint32 powerSlot = 0;
    uint32 useableSlot = 0;
    std::string affectName = "online";

    if (args.argCount() < 3) {
        throw CustomError ("Correct Usage: /fit [me|itemID] [typeID] [flag=??]");
    }

    // DNA tells us what slot to use but we're going to ignore it
    if (args.arg(1) == "me") {
        itemID = who->GetShip()->itemID();
    } else {
        itemID = atoi(args.arg(1).c_str());
    }

    typeID = atoi(args.arg(2).c_str());
    if (args.argCount() >= 4) {
        std::string::size_type n = args.arg(3).find("flag=");
        if (n != std::string::npos) {
            flag = (EVEItemFlags)atoi(args.arg(3).substr(5).c_str());
        }
    }


    if (typeID == 0) {
        throw PyException(MakeCustomError("Unable to create item of type %u.", typeID));
    } else {
        ItemData idata(
                       typeID,
                       who->GetCharacterID(),
                       30002187, //temp location
                       flagHangar,
                       1
                       );

        InventoryItemRef i = sItemFactory.SpawnItem(idata);
        if (!i) {
            throw PyException(MakeCustomError("Unable to create item of type %u.", typeID));
        }

        //ShipItemRef ship = services->item_factory->GetShip(itemID);
        //who->MoveItem(i->itemID(), itemID, flag);
        //i->Move(itemID, flag);
        //ship->AddItem(i);

        //return new PyBool(true);
    }
    //return new PyBool(false);
    return PyStatic.NewOne();
    */
}

PyResult Command_giveallskills(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args) {
    uint8 level = 5;            // Ensure that ALL skills are trained to level 5
    CharacterRef character;
    uint32 ownerID = 0;
    Client * pTarget = NULL;

    if (args.argCount() >= 2) {
        if (args.isNumber(1)) {
            ownerID = atoi(args.arg(1).c_str());
            pTarget = sEntityList.FindClientByCharID(ownerID);
            if (!pTarget)
                throw CustomError ("ERROR: Cannot find character #%d", ownerID);
            else
                character = pTarget->GetChar();
        } else if (args.arg(1) == "me") {
            ownerID = who->GetCharacterID();
            character = who->GetChar();
            pTarget = who;
        } else if (!args.isNumber(1)) {
            throw CustomError ("The use of string based Character names for this command is not yet supported!  Use 'me' instead or the entityID of the character to which you wish to give skills.");
            /*
             *            const char *name = args.arg(1).c_str();
             *            Client *target = sEntityList.FindCharacter(name);
             *           if (target == NULL)
             *                throw PyException(MakeCustomError("Cannot find Character by the name of %s", name));
             *            ownerID = target->GetCharacterID();
             *            character = target->GetChar();
             */
        } else
            throw CustomError ("Argument 1 must be Character ID or Character Name ");
    } else
        throw CustomError ("Correct Usage: /giveallskills [Character Name or ID]");

    // Make sure character reference is not NULL before trying to use it:
    if (character.get()) {
        CommandDB::charSkillStates skillList;
        // Query Database to get the list of those skills, not trained to 5 (either not injected yet or having level up to 4)
        CommandDB::NotFullyLearnedSkillList(skillList, ownerID);

        SkillRef skill;
        uint16 skillID;

        for (auto cur: skillList) {
            skillID = cur.first;
            if (cur.second >= 0) {  // the same check as character->HasSkill(skillID)
                skill = character->GetCharSkillRef(skillID);
                skill->SetAttribute(AttrSkillLevel, level);
                skill->SetAttribute(AttrSkillPoints, skill->GetSPForLevel(level));
                character->RemoveFromQueue(skill);
            } else {
                std::ostringstream str;
                str << "Skill Gift by ";
                str << who->GetCharName();
                ItemData idata(skillID, ownerID, pTarget->GetLocationID(), flagSkill, 1, str.str().c_str());
                skill = sItemFactory.SpawnSkill(idata);

                if (skill.get() == nullptr)
                    throw CustomError ("Unable to create item of type %s.", skill->typeID());

                skill->SetAttribute(AttrSkillLevel, level);
                skill->SetAttribute(AttrSkillPoints, skill->GetSPForLevel(level));
                // this will inform the client that the skill got injected into their brains!
                skill->ChangeSingleton(true);
                skill->Move(ownerID, flagSkill, true);
            }

            OnAdminSkillChange oasc;
            oasc.skillItemID = skill->itemID();
            oasc.skillTypeID = skillID;
            oasc.newSP = skill->GetSPForLevel(level);
            PyTuple* tmp = oasc.Encode();
            pTarget->QueueDestinyEvent(&tmp);

            //  save gm skill gift in history  -allan
            //  maybe not for this....WAAAAYYY  to much DB traffic for this.
            character->SaveSkillHistory(EvESkill::Event::GMGift, Win32TimeNow(), ownerID, skillID, level, \
            skill->GetAttribute(AttrSkillPoints).get_double());
        }
        // END LOOP
        // pTarget->SendErrorMsg("You need to relog for skills to get saved and show in character sheet.");
        return new PyString ("Gifting skills complete");
    }

    return new PyString ("Skill Gifting Failure");
}

PyResult Command_giveskills(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args) {
    //pass to command_giveskill
    Command_giveskill(who, db, services, args);
    return nullptr;
}

PyResult Command_giveskill(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args) {
    uint32 ownerID = 0, skillID = 0, level = 0;
    uint32 newPoints = 0;
    CharacterRef character;
    Client *pTarget(nullptr);

    if (args.argCount() == 4) {
        if (args.isNumber(1)) {
            ownerID = atoi(args.arg(1).c_str());
            character = sEntityList.FindClientByCharID(ownerID)->GetChar();
            pTarget = character->GetClient();
        } else if (args.arg(1) == "me") {
            ownerID = who->GetCharacterID();
            character = who->GetChar();
            pTarget = who;
        } else if (!args.isNumber(1)) {
            throw CustomError ("The use of string based Character names for this command is not yet supported!  Use 'me' instead or the entityID of the character to which you wish to give skills.");
            /*
            const char *name = args.arg(1).c_str();
            pTarget = sEntityList.FindClientByName(name);
            if (!pTarget)
                throw PyException(MakeCustomError("Cannot find Character by the name of %s", name));
            ownerID = pTarget->GetCharacterID();
            character = pTarget->GetChar();
            */
        } else
            throw CustomError ("Argument 1 must be Character ID or Character Name ");

        if (!args.isNumber(2))
            throw CustomError ("Argument 2 must be type ID.");
        skillID = atoi(args.arg(2).c_str());

        if (!args.isNumber(3))
            throw CustomError ("Argument 3 must be level");
        level = atoi(args.arg(3).c_str());

        if (level > EvESkill::MAXSKILLLEVEL)
            level = EvESkill::MAXSKILLLEVEL;
    } else
        throw CustomError ("Correct Usage: .giveskill [me/CharacterID] [skillID] [level]");

    // Make sure references are not NULL before trying to use them
    if ((pTarget == nullptr) or (character.get() == nullptr))
        throw CustomError ("ERROR: Unable to validate character object.");

    SkillRef skill;
    if (character->HasSkillTrainedToLevel(skillID, level)) {
        // already trained to requested level
        return PyStatic.NewNone();
    } else if (character->HasSkill(skillID)) {
        // has skill injected, so update level
        skill = character->GetCharSkillRef(skillID);
        if (skill.get() == nullptr){
            throw CustomError ("Unable to get item for skillID %u.", skillID);
            return new PyString ("Skill Gifting Failure - Unable to get item for skillID %u.", skillID);
        }
        newPoints = skill->GetSPForLevel(level);
        skill->SetAttribute(AttrSkillLevel, level);
        skill->SetAttribute(AttrSkillPoints, newPoints);
        //check for and remove this skill from training queue
        character->RemoveFromQueue(skill);
    } else {
        // Character does not have this skill
        std::ostringstream str;
        str << "Skill Gift by ";
        str << who->GetCharName();
        ItemData idata(skillID, ownerID, pTarget->GetLocationID(), flagSkill, 1, str.str().c_str());
        skill = sItemFactory.SpawnSkill(idata);
        if (skill.get() == nullptr)
            throw CustomError ("Unable to create item for skillID %u.", skillID);

        newPoints = skill->GetSPForLevel(level);
        skill->SetAttribute(AttrSkillLevel, level);
        skill->SetAttribute(AttrSkillPoints, newPoints);
        skill->ChangeSingleton(true);
        skill->Move(pTarget->GetCharacterID(), flagSkill, true);
    }

    skill->SaveItem();
    //  save gm skill gift in history  -allan
    character->SaveSkillHistory(EvESkill::Event::GMGift, GetFileTimeNow(), ownerID, skillID, level, newPoints);

    OnAdminSkillChange oasc;
        oasc.skillItemID = skill->itemID();
        oasc.skillTypeID = skillID;
        oasc.newSP = newPoints;
    PyTuple* tmp = oasc.Encode();
    pTarget->QueueDestinyEvent(&tmp);

    _log(SKILL__MESSAGE, "GM::GiveSkill - %s(%u) set to level %u with %u SP.", skill->name(), skillID, level, newPoints);
    return PyStatic.NewNone();
}

PyResult Command_online(Client *who, CommandDB *db, EVEServiceManager& services, const Seperator &args) {
    if (args.argCount() == 2) {
        if (strcmp("me", args.arg(1).c_str())!=0)
            if (!args.isNumber(1))
                throw CustomError ("Argument 1 should be an entity ID or me (me=self)");

        uint32 entity(atoi(args.arg(1).c_str()));
        Client* tgt(nullptr);

        if (strcmp("me", args.arg(1).c_str())==0) {
            tgt = who;
        } else {
            tgt = sEntityList.FindClientByCharID(entity);
            if (!tgt)
                throw CustomError ("Unable to find character %u", entity);
        }

        if (!tgt->InPod()) {
            tgt->GetShip()->OnlineAll();
        } else {
            throw CustomError ("Command failed: You can't activate modules while in a pod");
        }

        return(new PyString("All modules have been put Online"));
    } else {
        throw CustomError ("Command failed: You got the arguments all wrong.");
    }
}

PyResult Command_unload(Client *who, CommandDB *db, EVEServiceManager& services, const Seperator &args) {
// this means 'remove'

    if (args.argCount() >= 2 && args.argCount() <= 3)
    {
        uint32 itemID=0,entityID=0;

        if (strcmp("me", args.arg(1).c_str())!=0)
            if (!args.isNumber(1))
            {
                throw CustomError ("Argument 1 should be an entity ID or me (me=self)");
            }
            entityID = atoi(args.arg(1).c_str());

        if (args.argCount() ==3)
        {
            if (strcmp("all", args.arg(2).c_str())!=0)
                if (!args.isNumber(2))
                    throw CustomError ("Argument 2 should be an item ID or all");
                itemID = atoi(args.arg(2).c_str());
        }

        //select character
        Client* tgt(nullptr);
        if (strcmp("me", args.arg(1).c_str())==0)
            tgt = who;
        else
        {
            tgt = sEntityList.FindClientByCharID( entityID );

            if (!tgt)
                throw CustomError ("Unable to find character %u", entityID );
        }

        /// This doesn't seem like a valid requirement
        //if (tgt->IsInSpace())
        //    throw CustomError ("Character needs to be docked.");

        if (args.argCount() == 3 && strcmp("all", args.arg(2).c_str())!=0)
            tgt->GetShip()->GetModuleManager()->UnfitModule( itemID );

        if (args.argCount() == 3 && strcmp("all", args.arg(2).c_str())==0)
            tgt->GetShip()->UnloadAllModules();

        return(new PyString("All Modules have been unloaded"));
    }
    else
        throw CustomError ("Command failed: You got the arguments all wrong.");
}

PyResult Command_repairmodules(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    if (args.argCount()==1)
        who->GetShip()->RepairModules();
    if (args.argCount()==2) {
        if (!args.isNumber(1))
            throw CustomError ("Argument 1 should be a character ID");
        uint32 charID = atoi(args.arg(1).c_str());

        Client *target = sEntityList.FindClientByCharID(charID);
        if (target == NULL)
            throw CustomError ("Cannot find CharacterID %u", charID);
        target->GetShip()->RepairModules();
    }

    return(new PyString("Modules repaired successful."));
}

PyResult Command_dogma(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    //"dogma" "140019878" "agility" "=" "0.2"

    if (!(args.argCount() == 5)) {
        throw CustomError ("Correct Usage: /dogma [itemID|me] [attributeName] = [value]");
    }

    // First argument could be both
    if (args.isNumber(2)) {
        throw CustomError ("/dogma Second argument must be a string");
    }

    if (args.arg(3) != "=") {
        throw CustomError ("/dogma You didn't use an '=' in between your attribute name and value.");
    }
    if (!args.isNumber(4)) {
        throw CustomError ("/dogma The last argument must be a number");
    }

    const char *attributeName = args.arg(2).c_str();
    float attributeValue = atof(args.arg(4).c_str());

    InventoryItemRef i;
    if (args.arg(1) == "me") {
        i = sItemFactory.GetItemRef(who->GetShip().get()->itemID());
    } else {
        i = sItemFactory.GetItemRef(atoi(args.arg(1).c_str()));
    }


    i->SetAttribute(db->GetAttributeID(attributeName), attributeValue);
    /** @todo  for modules and ships, this will need to call some kind of 'reload' to reset the attrib mem object before new attrib takes affect.  */

    return nullptr;
}

PyResult Command_kick(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    Client *target;
    if (args.argCount() == 2) {
        if (args.isNumber(1)) {
            int id = atoi(args.arg(1).c_str());
            target = sEntityList.FindClientByCharID(id);
        } else {
            const char *name = args.arg(1).c_str();
            target = sEntityList.FindClientByName(name);
        }
    }
    //support for characters with first and last names
    else if (args.argCount() == 3) {
        if (args.isHexNumber(1))
            throw CustomError ("Unknown arguments");

        std::string name = args.arg(1) + " " + args.arg(2);
        target = sEntityList.FindClientByName(name.c_str()) ;
    } else {
        throw CustomError ("Correct Usage: /kick [Character Name]");
    }

    if (target == NULL) {
        throw CustomError ("Cannot find Character");
    } else {
        target->DisconnectClient();
    }

    return nullptr;
}

PyResult Command_ban(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    Client *target;

    if (args.argCount() == 2)
    {
        if (!args.isNumber(1))
        {
            const char *name = args.arg(1).c_str();
            target = sEntityList.FindClientByName(name);
        }
        else
            throw CustomError ("Correct Usage: /ban [Character Name]");
    }
    //support for characters with first and last names
    else if (args.argCount() == 3)
    {
        if (args.isHexNumber(1))
            throw CustomError ("Unknown arguments");

        std::string name = args.arg(1) + " " + args.arg(2);
        target = sEntityList.FindClientByName(name.c_str()) ;
    }
    else
        throw CustomError ("Correct Usage: /ban [Character Name]");

    //ban client
    target->BanClient();

    //disconnect client
    target->DisconnectClient();

    return nullptr;
}

PyResult Command_unban(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    if (args.argCount() == 2) {
        if (!args.isNumber(1)) {
            const char *name = args.arg(1).c_str();
            ServiceDB::SetAccountBanStatus(db->GetAccountID(name),false);
        } else {
            throw CustomError ("Correct Usage: /ban [Character Name]");
        }
    }
    //support for characters with first and last names
    else if (args.argCount() == 3) {
        if (args.isHexNumber(1))
            throw CustomError ("Unknown arguments");

        std::string name = args.arg(1) + " " + args.arg(2);
        ServiceDB::SetAccountBanStatus(db->GetAccountID(name),false);
    } else {
        throw CustomError ("Correct Usage: /unban [Character Name / Character ID]");
    }

    return nullptr;
}

PyResult Command_givelp(Client* who, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    if (args.argCount() == 4) {
        uint32 characterID;
        if (strcmp("me", args.arg(1).c_str())==0) {
            characterID = who->GetCharID();
        } else if (args.isNumber(1)) {
            characterID = stoi(args.arg(1));
            if (sEntityList.FindClientByCharID(characterID) == nullptr){ 
                throw CustomError ("Provided characterID is invalid or character is offline");
            }
        } else {
            throw CustomError ("Argument 1 should be a characterID or 'me'");
        }
        uint32 corporationID;
        std::string corpName;
        if (args.isNumber(2)) {
            corporationID = stoi(args.arg(2));
            corpName = CorporationDB::GetCorpName(corporationID);
            if (strcmp("Unknown", corpName.c_str())==0) {
                throw CustomError ("Invalid corporationID provided");
            }
        } else {
            throw CustomError ("Argument 2 should be a corporationID");
        }
        int amount;
        if (args.isNumber(3)) {
            amount = stoi(args.arg(3));
            if (amount <= 0) {
                throw CustomError ("Argument 3 should be a positive number");
            }
        } else {
            throw CustomError ("Argument 3 should be a valid number");
        }
        LPService::AddLP(characterID, corporationID, amount);
        std::string message = ("Added %i LP to char %i for corp %s." \
                               ,amount , characterID, corpName);
        return(new PyString(message));
    } else {
        throw CustomError ("Correct Usage: /givelp ['me'|<characterID>] [corporationID] [amount]");
    }
    return nullptr;
}
