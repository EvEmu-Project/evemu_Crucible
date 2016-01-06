/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    For the latest information visit http://evemu.org
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
*/

#include "eve-server.h"

#include "Client.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "admin/AllCommands.h"
#include "admin/CommandDB.h"
#include "inventory/AttributeEnum.h"
#include "inventory/InventoryDB.h"
#include "inventory/InventoryItem.h"
#include "manufacturing/Blueprint.h"
#include "ship/DestinyManager.h"
#include "ship/Drone.h"
#include "system/Damage.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"

PyResult Command_create( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    if( args.argCount() < 2 ) {
        throw PyException( MakeCustomError("Correct Usage: /create [typeID]") );
    }

    if( !args.isNumber( 1 ) )
        throw PyException( MakeCustomError( "Argument 1 must be type ID." ) );
    const uint32 typeID = atoi( args.arg( 1 ).c_str() );

    uint32 qty = 1;
    if( 2 < args.argCount() )
    {
        if( args.isNumber( 2 ) )
            qty = atoi( args.arg( 2 ).c_str() );
    }

    _log( COMMAND__MESSAGE, "Create %s %u times", args.arg( 1 ).c_str(), qty );

    //create into their cargo hold unless they are docked in a station,
    //then stick it in their hangar instead.
    uint32 locationID;
    EVEItemFlags flag;
    if( who->IsInSpace() )
    {
        locationID = who->GetShipID();
        flag = flagCargoHold;
    }
    else
    {
        locationID = who->GetStationID();
        flag = flagHangar;
    }

    ItemData idata(
        typeID,
        who->GetCharacterID(),
        0, //temp location
        flag,
        qty
    );

    InventoryItemRef i = services->item_factory.SpawnItem( idata );
    if( !i )
        throw PyException( MakeCustomError( "Unable to create item of type %s.", args.arg( 1 ).c_str() ) );

    //Move to location
	if( who->IsInSpace() )
		who->GetShip()->AddItem(flag, i);
	else
		i->Move( locationID, flag, true );

    return new PyString( "Creation successful." );
}

PyResult Command_createitem( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    if( args.argCount() < 2 ) {
        throw PyException( MakeCustomError("Correct Usage: /create [typeID]") );
    }

    //basically, a copy/paste from Command_create. The client seems to call this multiple times,
    //each time it creates an item
    if( !args.isNumber( 1 ) )
        throw PyException( MakeCustomError( "Argument 1 must be type ID." ) );
    const uint32 typeID = atoi( args.arg( 1 ).c_str() );

    uint32 qty = 1;
    if( 2 < args.argCount() )
    {
        if( args.isNumber( 2 ) )
            qty = atoi( args.arg( 2 ).c_str() );
    }

    sLog.Log("command message", "Create %s %u times", args.arg( 1 ).c_str(), qty );

    //create into their cargo hold unless they are docked in a station,
    //then stick it in their hangar instead.
    uint32 locationID;
    EVEItemFlags flag;
    if( who->IsInSpace() )
    {
        locationID = who->GetShipID();
        flag = flagCargoHold;
    }
    else
    {
        locationID = who->GetStationID();
        flag = flagHangar;
    }

    ItemData idata(
        typeID,
        who->GetCharacterID(),
        0, //temp location
        flag,
        qty
    );

    InventoryItemRef i = services->item_factory.SpawnItem( idata );
    if( !i )
        throw PyException( MakeCustomError( "Unable to create item of type %s.", args.arg( 1 ).c_str() ) );

    //Move to location
    i->Move( locationID, flag, true );

    return new PyString( "Creation successful." );
}


PyResult Command_search( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    if( args.argCount() < 2 ) {
        throw PyException( MakeCustomError("Correct Usage: /search [text]") );
    }

    const std::string& query = args.arg( 1 );

    //an empty query is a bad idea.
    if( 0 == query.length() )
        throw PyException( MakeCustomError( "Usage: /search [text]" ) );

    std::map<uint32, std::string> matches;
    if( !db->ItemSearch( query.c_str(), matches ) )
        throw PyException( MakeCustomError( "Failed to query DB." ) );

    std::string result( itoa( matches.size() ) );
    result += " matches found.<br>";

    std::map<uint32, std::string>::iterator cur, end;
    cur = matches.begin();
    end = matches.end();
    for(; cur != end; cur++)
    {
        result += itoa( cur->first );
        result += ": ";
        result += cur->second;
        result += "<br>";
    }

    if( 10 < matches.size() )
    {
        //send the results in an evemail.
        std::string subject( "Search results for " );
        subject += query;

        who->SelfEveMail( subject.c_str(), result.c_str() );

        return new PyString( "Results sent via evemail." );
    }
    else
        return new PyString( result );
}

PyResult Command_translocate( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
	return Command_tr(who,db,services,args);
/*
	if( args.argCount() < 2 ) {
        throw PyException( MakeCustomError("Correct Usage: /transloacte [entityID]") );
    }

    if( !args.isNumber( 1 ) )
        throw PyException( MakeCustomError( "Argument 1 should be an entity ID" ) );
    uint32 loc = atoi( args.arg( 1 ).c_str() );

    sLog.Log( "Command", "Translocate to %u.", loc );

    GPoint p( 0.0f, 1000000.0f, 0.0f ); //when going to a system, who knows where to stick them... could find a stargate and stick them near it I guess...

    if( !IsStation( loc ) && !IsSolarSystem( loc ) )
    {
        Client* target = services->entity_list.FindCharacter( loc );
        if( NULL == target )
            target = services->entity_list.FindByShip( loc );
        if( NULL == target )
            throw PyException( MakeCustomError( "Unable to find location %u.", loc ) );

        loc = target->GetLocationID();
        p = target->GetPosition();
    }

    who->MoveToLocation( loc, p );
    return new PyString( "Translocation successful." );
*/
}


PyResult Command_tr( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
	std::string usageString =
		"Correct Usage:<br><br>\
		- General Notes:<br>\
		+ tr is same as translocate command<br>\
		+ object being teleported MUST be in space<br>\
		+ destination object MUST be in space<br>\
		+ 'entityID #1' MUST be a currently logged-in character<br>\
		+ 'entityID #2' can be a character, ship, NPC, station, belt, stargate, or solar system<br>\
		+ 'me' string is allowed for [character name] to indicate YOU being teleported<br><br>\
		/tr [entityID #1] - teleport YOU to 'entityID'<br>\
		/tr [character name] - teleport YOU to 'character name'<br>\
		/tr [solarSystemID] - teleport YOU into 'solarSystemID' system<br>\
		/tr [solar system name] - teleport YOU into 'solar system name' system<br>\
		/tr [entityID #1|character name] [entityID #2|character name|solarSystemID|solar system name] - teleport 'entityID #1' or 'character name' to 'entityID #2', 'character name' or solar system<br>\
		/tr x y z - teleport YOU to a specific (x,y,z) coordinate in the current solar system<br>\
		";

	// Error if there are NO arguments past first string "tr" or "translocate":
    if( args.argCount() < 2 ) {
		throw PyException( MakeCustomError(usageString.c_str()) );
    }

	// Argument Discovery
	Client * p_targetClient = NULL;
	SystemEntity * destinationEntity = NULL;
	uint32 solarSystemID = 0;
	GPoint destinationPoint(0,0,0);
	uint32 argsCount = args.argCount();
    std::string name1 = args.arg( 1 );
	std::string name2 = "";
	bool isFirstArgName = false;
	bool isSecondArgName = false;
	uint32 trMode = 0;
	enum TR_MODE
	{
		TR_MODE_ME_TO_ENTITY = 1,
		TR_MODE_ME_TO_CHARACTER,
		TR_MODE_ME_TO_SOLARSYSTEMID,
		TR_MODE_ME_TO_SOLARSYSTEM,
		TR_MODE_ENTITY_TO_ENTITY,
		TR_MODE_ENTITY_TO_CHARACTER,
		TR_MODE_ENTITY_TO_SOLARSYSTEMID,
		TR_MODE_ENTITY_TO_SOLARSYSTEM,
		TR_MODE_CHARACTER_TO_ENTITY,
		TR_MODE_CHARACTER_TO_CHARACTER,
		TR_MODE_CHARACTER_TO_SOLARSYSTEMID,
		TR_MODE_CHARACTER_TO_SOLARSYSTEM
	};

	isFirstArgName = args.isNumber(1) ? false : true;

	// First, determine nature of First argument
	if( isFirstArgName )
	{
		// First argument is a string, find out if it's a character or solar system:
		if( (name1 == "me") && (argsCount < 3) )
			throw PyException( MakeCustomError(std::string(usageString+"<br><br>FIRST ARGUMENT WAS 'me' BUT MISSING SECOND ARGUMENT!").c_str()) );

		if( name1 == "me" )
			p_targetClient = who;
		else
		{
			// First argument is a string of a character or solar system:
			//TODO
		}
	}
	else
	{
		// First argument is a number, find out if it's a character, ship, NPC, station, belt, stargate, or solar system:
		//TODO
		p_targetClient = who;
		solarSystemID = atoi( args.arg( 1 ).c_str() );
		destinationPoint = GPoint( 1245789420000.0f, -1725486480000.0f, 1485125480000.0f );
		trMode = 1;
	}

	if( argsCount == 3 )
	{
		// We are transporting either THIS client 'who' or some other entity or character to somewhere:
		name2 = args.arg( 2 );
		isSecondArgName = args.isNumber(2) ? false : true;

		// Determine nature of Second argument
		if( isSecondArgName )
		{
			// Second argument is a string, find out if it's a character or solar system:
			//TODO
			throw PyException( MakeCustomError(std::string(usageString+"<br><br>NOT SUPPORTED YET!").c_str()) );
		}
		else
		{
			// Second argument is a number, find out if it's a character, ship, NPC, station, belt, stargate, or solar system:
			//TODO
			throw PyException( MakeCustomError(std::string(usageString+"<br><br>NOT SUPPORTED YET!").c_str()) );
		}
	}

	if( argsCount == 4 )
	{
		// SPECIAL CASE:  We are transporting ourselves to a specific (x,y,z) coordinate in the current solar system:
		p_targetClient = who;
		solarSystemID = who->GetLocationID();
		if( !IsSolarSystem(solarSystemID) )
			throw PyException( MakeCustomError(std::string(usageString+"<br><br>YOU MUST BE IN SPACE!").c_str()) );

		if( args.isNumber(2) && args.isNumber(3) && args.isNumber(4) )
			destinationPoint = GPoint( atoi(args.arg(2).c_str()), atoi(args.arg(3).c_str()), atoi(args.arg(4).c_str()) );
	}

	if( trMode == 0 )
		throw PyException( MakeCustomError(std::string(usageString+"<br><br>UNABLE TO DETERMINE FORMAT OF ARGUMENTS 1 and 2!").c_str()) );

    //solarSystemID = destinationEntity->GetLocationID();
    //destinationPoint = destinationEntity->GetPosition();

	// We're still going, so we know now we have a target to translocate AND a destination solar system AND destination coordinates, so let's do the translocate:
	//    p_targetClient - target character in a ship to translocate (Client *)
	//    solarSystemID - destination solar system ID
	//    destinationPoint - destination coordinates (GPoint)

	p_targetClient->Destiny()->SendJumpOutEffect("effects.JumpOut", solarSystemID);
	p_targetClient->MoveToLocation(solarSystemID, destinationPoint);
	p_targetClient->Destiny()->SendJumpInEffect("effects.JumpIn");

/*
	uint32 loc = atoi( args.arg( 2 ).c_str() );

    sLog.Log( "Command", "Translocate to %u.", loc );

    GPoint p( 0.0f, 0.0f, 1000000000.0f ); //when going to a system, who knows where to stick them... could find a stargate and stick them near it I guess...


    if( !IsStation( loc ) && !IsSolarSystem( loc ) )
    {
        Client* target = services->entity_list.FindCharacter( loc );
        if( NULL == target )
            target = services->entity_list.FindByShip( loc );
        if( NULL == target )
            throw PyException( MakeCustomError( "Unable to find location %u.", loc ) );

        loc = target->GetLocationID();
        p = target->GetPosition();
    }

    who->MoveToLocation( loc , p );
*/
    return new PyString( "Translocation successful." );
}

PyResult Command_giveisk( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{

    if( args.argCount() < 3 ) {
        throw PyException( MakeCustomError("Correct Usage: /giveisk [entityID ('me'=self)] [amount]") );
    }

    // Check for target (arg #1) for either a number or the string "me":
    std::string target = "";
    if( !args.isNumber( 1 ) )
    {
        target = args.arg( 1 );
        if( target != "me" )
            throw PyException( MakeCustomError( "Argument 1 should be an entity ID ('me'=self)" ) );
    }

    // If target (arg #1) is not the string "me" then decode number from argument string, otherwise get this character's ID:
    uint32 entity;
    if( target == "" )
        entity = atoi( args.arg( 1 ).c_str() );
    else
        entity = who->GetCharacterID();

    if( !args.isNumber( 2 ) )
        throw PyException( MakeCustomError( "Argument 2 should be an amount of ISK" ) );
    double amount = strtod( args.arg( 2 ).c_str(), NULL );

    Client* tgt;
    if( entity >= EVEMU_MINIMUM_ID )
    {
        tgt = services->entity_list.FindCharacter( entity );
        if( NULL == tgt )
            throw PyException( MakeCustomError( "Unable to find character %u", entity ) );
    }
    else
        throw PyException( MakeCustomError( "Invalid entityID for characters %u", entity ) );

    tgt->AddBalance( amount );
    return new PyString( "Operation successful." );
}

PyResult Command_pop( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    if( 4 != args.argCount() )
        throw PyException( MakeCustomError( "Correct Usage: /pop [message type] [key] [text]" ) );

    //CustomNotify: notify
    //ServerMessage: msg
    //CustomError: error

    const std::string& msgType = args.arg( 1 );
    const std::string& key = args.arg( 2 );
    const std::string& text = args.arg( 3 );

    Notify_OnRemoteMessage n;
    n.msgType = msgType;
    n.args[ key ] = new PyString( text );

    PyTuple* t = n.Encode();
    who->SendNotification( "OnRemoteMessage", "charid", &t );
    PySafeDecRef( t );

    return new PyString( "Message sent." );
}

PyResult Command_goto( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if( 3 != args.argCount()
        || !args.isNumber( 1 )
        || !args.isNumber( 2 )
        || !args.isNumber( 3 ) )
    {
        throw PyException( MakeCustomError( "Correct Usage: /goto [x coord] [y coor] [z coord]" ) );
    }

    GPoint p( atof( args.arg( 1 ).c_str() ),
              atof( args.arg( 2 ).c_str() ),
              atof( args.arg( 3 ).c_str() ) );

    sLog.Log( "Command", "%s: Goto (%.13f, %.13f, %.13f)", who->GetName(), p.x, p.y, p.z );

    who->MoveToPosition( p );
    return new PyString( "Goto successful." );
}

PyResult Command_spawnn( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    uint32 typeID = 0;
    uint32 actualTypeID = 0;
    std::string actualTypeName = "";
    uint32 actualGroupID = 0;
    uint32 actualCategoryID = 0;
    double actualRadius = 0.0;
    InventoryItemRef item;
    ShipRef ship;

    // "/spawnn" arguments:
    // #1 = quantity ?
    // #2 = some double value ?
    // #3 = typeID

    if( (args.argCount() < 4) || (args.argCount() > 4) )
    {
        throw PyException( MakeCustomError("LOL, we don't know the correct usage of /spawnn, sorry you're S.O.L., BUT it should have 4 arguments.") );
    }

    // Since we don't know what args 1 and 2 are, we don't care about them right now...

    if( !(args.isNumber( 3 )) )
        throw PyException( MakeCustomError( "Argument 3 should be an item type ID" ) );

    typeID = atoi( args.arg( 3 ).c_str() );

    if( !who->IsInSpace() )
        throw PyException( MakeCustomError( "You must be in space to spawn things." ) );

    // Search for item type using typeID:
    if( !(db->ItemSearch(typeID, actualTypeID, actualTypeName, actualGroupID, actualCategoryID, actualRadius) ) )
    {
        return new PyString( "Unknown typeID or typeName returned no matches." );
    }

    GPoint loc( who->GetPosition() );
    // Calculate a random coordinate on the sphere centered on the player's position with
    // a radius equal to the radius of the ship/celestial being spawned times 10 for really good measure of separation:
    double radius = (actualRadius * 5.0) * (double)(MakeRandomInt( 1, 3));     // Scale the distance from player that the object will spawn to between 10x and 15x the object's radius
    loc.MakeRandomPointOnSphere( radius );

    // Spawn the item:
    ItemData idata(
        actualTypeID,
        1, // owner is EVE System
        who->GetLocationID(),
        flagAutoFit,
        actualTypeName.c_str(),
        loc
    );

    item = services->item_factory.SpawnItem( idata );
    if( !item )
        throw PyException( MakeCustomError( "Unable to spawn item of type %u.", typeID ) );

    DBSystemDynamicEntity entity;

    entity.allianceID = 0;
    entity.categoryID = actualCategoryID;
    entity.corporationID = 0;
    entity.flag = 0;
    entity.groupID = actualGroupID;
    entity.itemID = item->itemID();
    entity.itemName = actualTypeName;
    entity.locationID = who->GetLocationID();
    entity.ownerID = 1;
    entity.typeID = actualTypeID;
    entity.x = loc.x;
    entity.y = loc.y;
    entity.z = loc.z;

    // Actually do the spawn using SystemManager's BuildEntity:
    if( !(who->System()->BuildDynamicEntity( who, entity )) )
        return new PyString( "Spawn Failed: typeID or typeName not supported." );

    sLog.Log( "Command", "%s: Spawned %u.", who->GetName(), typeID );

    return new PyString( "Spawn successful." );
}

PyResult Command_spawn( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    uint32 typeID = 0;
	uint32 spawnCount = 1;
	uint32 spawnIndex = 0;
	uint32 maximumSpawnCountAllowed = 100;
    uint32 actualTypeID = 0;
    std::string actualTypeName = "";
    uint32 actualGroupID = 0;
    uint32 actualCategoryID = 0;
    double actualRadius = 0.0;
    InventoryItemRef item;
    ShipRef ship;
    double radius;
    bool offsetLocationSet = false;
	std::string usage = "Correct Usage: <br><br> /spawn [typeID(int)/typeName(string)] <br><br>With optional spawn count: <br> /spawn [typeID(int)/typeName(string)] [count] <br><br>With optional count and (X,Y,Z) coordinate: <br> /spawn [typeID(int/typeName(string)] [count] [x(float)] [y(float)] [z(float)]";

    if( !who->IsInSpace() )
		throw PyException( MakeCustomError( usage.c_str() ) );

    if( args.argCount() < 2 ) {
		throw PyException( MakeCustomError( usage.c_str() ) );
    }

    if( !(args.isNumber( 1 )) )
        throw PyException( MakeCustomError( "Argument 1 should be an item type ID" ) );

    typeID = atoi( args.arg( 1 ).c_str() );

    // Search for item type using typeID:
    if( !(db->ItemSearch(typeID, actualTypeID, actualTypeName, actualGroupID, actualCategoryID, actualRadius) ) )
    {
        return new PyString( "Unknown typeID or typeName returned no matches." );
    }

	if( args.argCount() > 2 )
    {
		if( !(args.isNumber(2)) )
            throw PyException( MakeCustomError( "Argument 3 should be the number of spawns of this type you want to create" ) );

		spawnCount = atoi( args.arg(2).c_str() );
		if( spawnCount > maximumSpawnCountAllowed)
            throw PyException( MakeCustomError( "Argument 3, spawn count, is allowed to be no more than 100" ) );
	}

    // Check to see if the X Y Z optional coordinates were supplied with the command:
    GPoint offsetLocation;
    if( args.argCount() > 3 )
    {
        if( !(args.isNumber(3)) )
            throw PyException( MakeCustomError( "Argument 4 should be the X distance from your ship in meters you want the item spawned" ) );

		if( args.argCount() > 4 )
		{
			if( !(args.isNumber(4)) )
	            throw PyException( MakeCustomError( "Argument 5 should be the Y distance from your ship in meters you want the item spawned" ) );
		}
		else
			throw PyException( MakeCustomError( "TOO FEW PARAMETERS: %s", usage.c_str() ) );

		if( args.argCount() > 5 )
		{
			if( !(args.isNumber(5)) )
				throw PyException( MakeCustomError( "Argument 6 should be the Z distance from your ship in meters you want the item spawned" ) );
		}
		else
			throw PyException( MakeCustomError( "TOO FEW PARAMETERS: %s", usage.c_str() ) );

        offsetLocation.x = atoi( args.arg( 3 ).c_str() );
        offsetLocation.y = atoi( args.arg( 4 ).c_str() );
        offsetLocation.z = atoi( args.arg( 5 ).c_str() );
        offsetLocationSet = true;
    }

    GPoint loc;

	for(spawnIndex=0; spawnIndex < spawnCount; spawnIndex++)
	{
		loc = who->GetPosition();

		if( offsetLocationSet )
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
			radius = (actualRadius * 5.0) * (double)(MakeRandomInt( 1, 3));     // Scale the distance from player that the object will spawn to between 10x and 15x the object's radius
			loc.MakeRandomPointOnSphere( radius );
		}

		// Spawn the item:
		ItemData idata(
			actualTypeID,
			1, // owner is EVE System
			who->GetLocationID(),
			flagAutoFit,
			actualTypeName.c_str(),
			loc
		);

		item = services->item_factory.SpawnItem( idata );
		if( !item )
			throw PyException( MakeCustomError( "Unable to spawn item of type %u.", typeID ) );

		DBSystemDynamicEntity entity;

		entity.allianceID = 0;
		entity.categoryID = actualCategoryID;
		entity.corporationID = 0;
		entity.flag = 0;
		entity.groupID = actualGroupID;
		entity.itemID = item->itemID();
		entity.itemName = actualTypeName;
		entity.locationID = who->GetLocationID();
		entity.ownerID = 1;
		entity.typeID = actualTypeID;
		entity.x = loc.x;
		entity.y = loc.y;
		entity.z = loc.z;

		// Actually do the spawn using SystemManager's BuildEntity:
		if( !(who->System()->BuildDynamicEntity( who, entity )) )
			return new PyString( "Spawn Failed: typeID or typeName not supported." );

		// TEST FOR FUN:  If this is a drone, make its destiny manager orbit the ship that spawned it like a little lost puppy...
		if( item->categoryID() == EVEDB::invCategories::Drone )
		{
			((DroneEntity *)(who->System()->get( entity.itemID )))->Destiny()->SetSpeedFraction( 1.0, true );
			((DroneEntity *)(who->System()->get( entity.itemID )))->Destiny()->Orbit( who, 1000.0, true );
		}

		// TEST FOR FUN:  If this is a missile, torpedo, bomb, etc, then make its destiny manager Approach a target of the entity spawning it...
		uint32 groupID = item->groupID();
		if( groupID == 84 || groupID == 88 || groupID == 89 || groupID == 90 || groupID == 384 || groupID == 385 ||
			groupID == 386 || groupID == 387 || groupID == 395 || groupID == 396 || groupID == 476 || groupID == 648 ||
			groupID == 653 || groupID == 654 || groupID == 655 || groupID == 656 || groupID == 657 || groupID == 772 || groupID == 1019)
		{
			// This is NOT even going to make it here since Missiles/Torpedos/Bombs/etc are not supported yet by SystemManager::BuildDynamicEntity()
		}
	}

	sLog.Log( "Command", "%s: Spawned %u in space, %u times", who->GetName(), typeID, spawnCount );

    return new PyString( "Spawn successful." );
}

PyResult Command_location( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    if( !who->IsInSpace() )
        throw PyException( MakeCustomError( "You're not in space." ) );

    if( NULL == who->Destiny() )
        throw PyException( MakeCustomError( "You have no Destiny manager." ) );

    DestinyManager* dm = who->Destiny();

    const GPoint& loc = dm->GetPosition();
    const GVector& vel = dm->GetVelocity();

    char reply[128];
    snprintf( reply, 128,
        "<br>"
        "x: %lf<br>"
        "y: %lf<br>"
        "z: %lf<br>"
        "speed: %lf",
        loc.x, loc.y, loc.z,
        vel.length()
    );

	who->SendInfoModalMsg( reply );

    return new PyString( reply );
}

PyResult Command_syncloc( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    if( !who->IsInSpace() )
        throw PyException( MakeCustomError( "You're not in space." ) );

    if( NULL == who->Destiny() )
        throw PyException( MakeCustomError( "You have no Destiny manager." ) );

    DestinyManager* dm = who->Destiny();
    dm->SetPosition( dm->GetPosition(), true );

    return new PyString( "Position synchronized." );
}

// command to modify blueprint's attributes, we have to give it blueprint's itemID ...
// isn't much comfortable, but I don't know about better solution ...
PyResult Command_setbpattr( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{

    if( args.argCount() < 6 ) {
        throw PyException( MakeCustomError("Correct Usage: /setbpattr [blueprintID] [0 (not copy) or 1 (copy)] [material level] [productivity level] [remaining runs]") );
    }

    if( !args.isNumber( 1 ) )
        throw PyException( MakeCustomError( "Argument 1 must be blueprint ID. (got %s)", args.arg( 1 ).c_str() ) );
    const uint32 blueprintID = atoi( args.arg( 1 ).c_str() );

    if( "0" != args.arg( 2 ) && "1" != args.arg( 2 ) )
        throw PyException( MakeCustomError( "Argument 2 must be 0 (not copy) or 1 (copy). (got %s)", args.arg( 2 ).c_str() ) );
    const bool copy = ( atoi( args.arg( 2 ).c_str() ) ? true : false );

    if( !args.isNumber( 3 ) )
        throw PyException( MakeCustomError( "Argument 3 must be material level. (got %s)", args.arg( 3 ).c_str() ) );
    const uint32 materialLevel = atoi( args.arg( 3 ).c_str() );

    if( !args.isNumber( 4 ) )
        throw PyException( MakeCustomError( "Argument 4 must be productivity level. (got %s)", args.arg( 4 ).c_str() ) );
    const uint32 productivityLevel = atoi( args.arg( 4 ).c_str() );

    if( !args.isNumber( 5 ) )
        throw PyException( MakeCustomError( "Argument 5 must be remaining licensed production runs. (got %s)", args.arg( 5 ).c_str() ) );
    const uint32 licensedProductionRunsRemaining = atoi( args.arg( 5 ).c_str() );

    BlueprintRef bp = services->item_factory.GetBlueprint( blueprintID );
    if( !bp )
        throw PyException( MakeCustomError( "Failed to load blueprint %u.", blueprintID ) );

    bp->SetCopy( copy );
    bp->SetMaterialLevel( materialLevel );
    bp->SetProductivityLevel( productivityLevel );
    bp->SetLicensedProductionRunsRemaining( licensedProductionRunsRemaining );

    return new PyString( "Properties modified." );
}

PyResult Command_state(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
    if(!who->IsInSpace())
        throw(PyException(MakeCustomError("You must be in space.")));

    DestinyManager *destiny = who->Destiny();
    if(destiny == NULL)
        throw(PyException(MakeCustomError("You have no destiny manager.")));

    destiny->SendSetState(who->Bubble());

    return(new PyString("Update sent."));
}

PyResult Command_getattr( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    if( args.argCount() < 3 ) {
        throw PyException( MakeCustomError("Correct Usage: /getattr [itemID] [attributeID]") );
    }
    if( !args.isNumber( 1 ) )
        throw PyException( MakeCustomError( "1st argument must be itemID (got %s).", args.arg( 1 ).c_str() ) );
    const uint32 itemID = atoi( args.arg( 1 ).c_str() );

    if( !args.isNumber( 2 ) )
        throw PyException( MakeCustomError( "2nd argument must be attributeID (got %s).", args.arg( 2 ).c_str() ) );
    const ItemAttributeMgr::Attr attribute = (ItemAttributeMgr::Attr)atoi( args.arg( 2 ).c_str() );

    InventoryItemRef item = services->item_factory.GetItem( itemID );
    if( !item )
        throw PyException( MakeCustomError( "Failed to load item %u.", itemID ) );

    //return item->attributes.PyGet( attribute );
    return item->GetAttribute(attribute).GetPyObject();
}

PyResult Command_setattr( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    if( args.argCount() < 4 ) {
        throw PyException( MakeCustomError("Correct Usage: /setattr [itemID] [attributeID] [value]") );
    }

    // Check for target (arg #1) for either a number or the string "myship":
    uint32 itemID = 0;
    std::string target = "";
    if( !args.isNumber( 1 ) )
    {
        target = args.arg( 1 );
        if( target != "myship" )
            throw PyException( MakeCustomError( "1st argument should be an entity ID ('myship'=current ship) (got %s).", args.arg( 1 ).c_str() ) );

        itemID = who->GetShipID();
    }
    else
    {
        // target (arg #1) is a number, so decode it and move on:
        itemID = atoi( args.arg( 1 ).c_str() );
    }

    if( !args.isNumber( 2 ) )
        throw PyException( MakeCustomError( "2nd argument must be attributeID (got %s).", args.arg( 2 ).c_str() ) );
    const ItemAttributeMgr::Attr attribute = (ItemAttributeMgr::Attr)atoi( args.arg( 2 ).c_str() );

    if( !args.isNumber( 3 ) )
        throw PyException( MakeCustomError( "3rd argument must be value (got %s).", args.arg( 3 ).c_str() ) );
    const double value = atof( args.arg( 3 ).c_str() );

    if( itemID < EVEMU_MINIMUM_ID )
        throw PyException( MakeCustomError( "1st argument must be a valid 'entity' table itemID (got %s) that MUST be larger >= 140000000.", args.arg( 1 ).c_str() ) );

    InventoryItemRef item = services->item_factory.GetItem( itemID );
    if( !item )
        throw PyException( MakeCustomError( "Failed to load item %u.", itemID ) );

    //item->attributes.SetReal( attribute, value );
    sLog.Warning( "GMCommands: Command_dogma()", "This command will modify attribute and send change to client, but change does not take effect in client for some reason." );
    item->SetAttribute(attribute, (float)value);

    return new PyString( "Operation successful." );
}

PyResult Command_fit(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{

    if( args.argCount() < 2 ) {
        throw PyException( MakeCustomError("Correct Usage: /fit [typeID] ") );
    }

    uint32 typeID = 0;

    if( args.argCount() == 3)
    {
        if( !args.isNumber( 2 ) )
            throw PyException( MakeCustomError( "Argument 1 must be type ID." ) );
        typeID = atoi( args.arg( 2 ).c_str() );
    }
    else if( args.argCount() == 2 )
    {
        if( !args.isNumber( 1 ) )
            throw PyException( MakeCustomError( "Argument 1 must be type ID." ) );
        typeID = atoi( args.arg( 1 ).c_str() );
    }

    uint32 qty = 1;

    _log( COMMAND__MESSAGE, "Create %s %u times", typeID, qty );


    EVEItemFlags flag;
    uint32 powerSlot;
    uint32 useableSlot;
    std::string affectName = "online";

    if( typeID == 0 )
    {
        throw PyException( MakeCustomError( "Unable to create item of type %u.", typeID ) );
        return new PyString( "Creation FAILED." );
    }
    else
    {
        //Get Range of slots for item
        InventoryDB::GetModulePowerSlotByTypeID( typeID, powerSlot );

        //Get open slots available on ship
        InventoryDB::GetOpenPowerSlots(powerSlot, who->GetShip(), useableSlot);

        ItemData idata(
            typeID,
            who->GetCharacterID(),
            0, //temp location
            flag = (EVEItemFlags)useableSlot,
            qty
        );

        InventoryItemRef i = services->item_factory.SpawnItem( idata );
        if( !i )
            throw PyException( MakeCustomError( "Unable to create item of type %u.", typeID ) );

        who->MoveItem( i->itemID(), who->GetShipID(), flag );

        return new PyString( "Creation successful." );
    }
}

PyResult Command_giveallskills( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    uint8 level = 5;			// Ensure that ALL skills trained are trained to level 5
    CharacterRef character;
    EVEItemFlags flag;
    uint32 gty = 1;
    //uint8 oldSkillLevel = 0;
    EvilNumber oldSkillLevel(0);
    uint32 ownerID = 0;
	Client * clientPtr = NULL;

    if( args.argCount() >= 2 )
    {
        if( args.isNumber( 1 ) )
        {
            ownerID = atoi( args.arg( 1 ).c_str() );
            clientPtr = services->entity_list.FindCharacter( ownerID );
			if( clientPtr == NULL )
				throw PyException( MakeCustomError( "ERROR: Cannot find character #%d", ownerID ) );
			else
				character = clientPtr->GetChar();
        }
        else if( args.arg( 1 ) == "me" )
        {
            ownerID = who->GetCharacterID();
            character = who->GetChar();
        }
        else if( !args.isNumber( 1 ) )
        {
            throw PyException( MakeCustomError( "The use of string based Character names for this command is not yet supported!  Use 'me' instead or the entityID of the character to which you wish to give skills." ) );
            /*
            const char *name = args.arg( 1 ).c_str();
            Client *target = services->entity_list.FindCharacter( name );
            if(target == NULL)
                throw PyException( MakeCustomError( "Cannot find Character by the name of %s", name ) );
            ownerID = target->GetCharacterID();
            character = target->GetChar();
            */
        }
        else
            throw PyException( MakeCustomError( "Argument 1 must be Character ID or Character Name ") );
    }
	else
        throw PyException( MakeCustomError("Correct Usage: /giveallskills [Character Name or ID]") );

    SkillRef skill;

    // Make sure character reference is not NULL before trying to use it:
    if( character.get() != NULL )
    {
		// Query Database to get list of ALL skills, then LOOP through each one, checking character for skill, setting level to 5:
		// QUERY DB FOR LIST OF ALL SKILLS:
		//		SELECT * FROM `invTypes` WHERE `groupID` IN (SELECT groupID FROM invGroups WHERE categoryID = 16)
		// LOOP through each skill
		std::vector<uint32> skillList;
		db->FullSkillList( skillList );

		std::vector<uint32>::const_iterator skill_cur, skill_end;
		skill_cur = skillList.begin();
		skill_end = skillList.end();

		uint32 skillID = 0;
		for( ; skill_cur != skill_end; skill_cur++ )
		{
			skillID = *skill_cur;
			if(character->HasSkill( skillID ) )
			{
				// Character already has this skill, so let's get the current level and check to see
				// if we need to update its level to what's required:
				SkillRef oldSkill = character->GetSkill( skillID );
				oldSkillLevel = oldSkill->GetAttribute( AttrSkillLevel );

				// Now check the current level to the required level and update it
				if( oldSkillLevel < level )
					character->InjectSkillIntoBrain( oldSkill, level);
			}
			else
			{
				// Character DOES NOT have this skill, so spawn a new one and then add this
				// to the character with required level and skill points:
				ItemData idata(
					skillID,
					ownerID,
					0, //temp location
					flag = (EVEItemFlags)flagSkill,
					gty
				);

				InventoryItemRef item = services->item_factory.SpawnItem( idata );
				skill = SkillRef::StaticCast( item );

				if( !item )
					throw PyException( MakeCustomError( "ERROR: Unable to create item of type %s.", item->typeID() ) );

				character->InjectSkillIntoBrain( skill, level);
			}
		}
		// END LOOP
    }

    return new PyString ("Skill Gifting Failure");
}

PyResult Command_giveskills( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args ) {

    //pass to command_giveskill
    Command_giveskill(who,db,services,args);

    return NULL;

}

PyResult Command_giveskill( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{

    EvilNumber typeID;
    int level;
    CharacterRef character;
    uint32 ownerID = 0;

    if( args.argCount() == 4 )
    {
        if( args.isNumber( 1 ) )
        {
            ownerID = atoi( args.arg( 1 ).c_str() );
            character = services->entity_list.FindCharacter( ownerID )->GetChar();
        }
        else if( args.arg( 1 ) == "me" )
        {
            ownerID = who->GetCharacterID();
            character = who->GetChar();
        }
        else if( !args.isNumber( 1 ) )
        {
            throw PyException( MakeCustomError( "The use of string based Character names for this command is not yet supported!  Use 'me' instead or the entityID of the character to which you wish to give skills." ) );
            /*
            const char *name = args.arg( 1 ).c_str();
            Client *target = services->entity_list.FindCharacter( name );
            if(target == NULL)
                throw PyException( MakeCustomError( "Cannot find Character by the name of %s", name ) );
            ownerID = target->GetCharacterID();
            character = target->GetChar();
            */
        }
        else
            throw PyException( MakeCustomError( "Argument 1 must be Character ID or Character Name ") );


        if( !args.isNumber( 2 ) )
            throw PyException( MakeCustomError( "Argument 2 must be type ID." ) );
        typeID = atoi( args.arg( 2 ).c_str() );

        if( !args.isNumber( 3 ) )
            throw PyException( MakeCustomError( "Argument 3 must be level" ) );
        level = atoi( args.arg( 3 ).c_str() );

        //levels don't go higher than 5
        if( level > 5 )
            level = 5;
    } else
        throw PyException( MakeCustomError("Correct Usage: /giveskill [Character Name or ID] [skillID] [desired level]") );
    // Make sure Character reference is not NULL before trying to use it:
    if(character.get() != NULL)
    {
        SkillRef skill;
        uint8 skillLevel;
        if(character->HasSkill( typeID.get_int() ) )
        {
            // Character already has this skill, so let's get the current level and check to see
            // if we need to update its level to what's required:
            skill = character->GetSkill( typeID.get_int() );
            skillLevel = skill->GetAttribute(AttrSkillLevel).get_int();
            if( skillLevel >= level )
            {
                return new PyNone;
            }
            else
            {
                EvilNumber tmp = EVIL_SKILL_BASE_POINTS * skill->GetAttribute(AttrSkillTimeConstant) * EvilNumber::pow(2, (2.5*(level - 1)));
                skill->SetAttribute(AttrSkillLevel, level);
                skill->SetAttribute(AttrSkillPoints, tmp);
            }
        }
        else
        {
            // Character DOES NOT have this skill, so spawn a new one and then add this
            // to the character with required level and skill points:
            ItemData idata(
                typeID.get_int(),
                ownerID,
                ownerID,
                flagSkill,
                1
            );

            InventoryItemRef item = services->item_factory.SpawnItem( idata );

            if( !item )
            {
                throw PyException( MakeCustomError( "Unable to create item of type %s.", item->typeID() ) );
                return new PyString ("Skill Gifting Failure - Unable to create new skill %s.", item->typeID() );
            }
            else
            {
                skill = SkillRef::StaticCast( item );
                EvilNumber tmp = EVIL_SKILL_BASE_POINTS * skill->GetAttribute(AttrSkillTimeConstant) * EvilNumber::pow(2, (2.5*(level - 1)));
                skill->SetAttribute(AttrSkillLevel, level);
                skill->SetAttribute(AttrSkillPoints, tmp);
            }
        }

		// Either way, this character now has this skill trained to the specified level, so inform client:
        if( who != NULL )
        {
            OnSkillTrained ost;
            ost.itemID = skill->itemID();

            PyTuple* tmp = ost.Encode();
            who->QueueDestinyEvent( &tmp );
            PySafeDecRef( tmp );

            who->UpdateSkillTraining();
        }
    }
    /** commented out to prevent function cease on error.  seems to work correctly without this msg  -allan 10Jul14 */
    //throw PyException( MakeCustomError( "ERROR: Unable to validate character object, it was found to be NULL!" ) );
    return new PyString ("Skill Gifting Failure - Character Ref = NULL");
}


PyResult Command_online(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {

    if( args.argCount() == 2 )
    {
        if( strcmp("me", args.arg( 1 ).c_str())!=0 )
            if( !args.isNumber( 1 ) )
                throw PyException( MakeCustomError( "Argument 1 should be an entity ID or me (me=self)" ) );
            uint32 entity = atoi( args.arg( 1 ).c_str() );

        Client* tgt;
        if( strcmp("me", args.arg( 1 ).c_str())==0 )
            tgt = who;
        else
        {
            tgt = services->entity_list.FindCharacter( entity );
            if( NULL == tgt )
                throw PyException( MakeCustomError( "Unable to find character %u", entity ) );
        }

        if( !tgt->InPod() )
            tgt->GetShip()->OnlineAll();
        else
            throw PyException( MakeCustomError( "Command failed: You can't activate mModulesMgr while in pod"));

        return(new PyString("All mModulesMgr have been put Online"));
    }
    else
        throw PyException( MakeCustomError( "Command failed: You got the arguments all wrong!"));
}

PyResult Command_unload(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {

    if( args.argCount() >= 2 && args.argCount() <= 3 )
    {
        uint32 item=0,entity=0;

        if( strcmp("me", args.arg( 1 ).c_str())!=0 )
            if( !args.isNumber( 1 ) )
            {
                throw PyException( MakeCustomError( "Argument 1 should be an entity ID or me (me=self)" ) );
            }
            entity = atoi( args.arg( 1 ).c_str() );

        if( args.argCount() ==3 )
        {
            if( strcmp("all", args.arg( 2 ).c_str())!=0 )
                if( !args.isNumber( 2 ) )
                    throw PyException( MakeCustomError( "Argument 2 should be an item ID or all" ) );
                item = atoi( args.arg( 2 ).c_str() );
        }

        //select character
        Client* tgt;
        if( strcmp("me", args.arg( 1 ).c_str())==0 )
            tgt = who;
        else
        {
            tgt = services->entity_list.FindCharacter( entity );

            if( NULL == tgt )
                throw PyException( MakeCustomError( "Unable to find character %u", entity ) );
        }

        if( tgt->IsInSpace() )
            throw PyException( MakeCustomError( "Character needs to be docked!" ) );

        if( args.argCount() == 3 && strcmp("all", args.arg( 2 ).c_str())!=0)
            tgt->GetShip()->UnloadModule(item);

        if( args.argCount() == 3 && strcmp("all", args.arg( 2 ).c_str())==0)
            tgt->GetShip()->UnloadAllModules();

        return(new PyString("All mModulesMgr have been unloaded"));
    }
    else
        throw PyException( MakeCustomError( "Command failed: You got the arguments all wrong!"));
}

PyResult Command_heal( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    if( args.argCount()== 1 )
    {
        /*who->GetShip()->Set_armorDamage(0);
        who->GetShip()->Set_damage(0);
        who->GetShip()->Set_shieldCharge(who->GetShip()->shieldCapacity());*/

        who->GetShip()->SetAttribute(AttrArmorDamage, 0);
        who->GetShip()->SetAttribute(AttrDamage, 0);
        EvilNumber shield_charge = who->GetShip()->GetAttribute(AttrShieldCapacity);
        who->GetShip()->SetAttribute(AttrShieldCharge, shield_charge);
		who->GetShip()->SetAttribute(AttrCharge, who->GetShip()->GetAttribute(AttrCapacitorCapacity) );
    }
    if( args.argCount() == 2 )
    {
        if( !args.isNumber( 1 ) )
            {
                throw PyException( MakeCustomError( "Argument 1 should be a character ID" ) );
            }
        uint32 entity = atoi( args.arg( 1 ).c_str() );

        Client *target = services->entity_list.FindCharacter( entity );
        if(target == NULL)
            throw PyException( MakeCustomError( "Cannot find Character by the entity %d", entity ) );

        who->GetShip()->SetAttribute(AttrArmorDamage, 0);
        who->GetShip()->SetAttribute(AttrDamage, 0);
        EvilNumber shield_charge = who->GetShip()->GetAttribute(AttrShieldCapacity);
        who->GetShip()->SetAttribute(AttrShieldCharge, shield_charge);
    }

    return(new PyString("Heal successful!"));
}

PyResult Command_repairmodules( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{

    if(args.argCount()==1)
    {
        who->GetShip()->RepairModules();
    }
    if(args.argCount()==2)
    {
        if( !args.isNumber( 1 ) )
            {
                throw PyException( MakeCustomError( "Argument 1 should be a character ID" ) );
            }
        uint32 charID = atoi( args.arg( 1 ).c_str() );

        Client *target = services->entity_list.FindCharacter( charID );
        if(target == NULL)
            throw PyException( MakeCustomError( "Cannot find Character by the entity %d", charID ) );
        target->GetShip()->RepairModules();
    }

    return(new PyString("Modules repaired successful!"));
}

PyResult Command_unspawn( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    uint32 entityID = 0;
    uint32 itemID = 0;

    if( (args.argCount() < 3) || (args.argCount() > 3) )
        throw PyException( MakeCustomError("Correct Usage: /unspawn (entityID) (itemID), and for now (entityID) is unused, so just type 0, and use the itemID from the entity table for (itemID)") );

    if( !(args.isNumber( 1 )) )
        throw PyException( MakeCustomError( "Argument 1 should be an item entity ID" ) );

    if( !(args.isNumber( 2 )) )
        throw PyException( MakeCustomError( "Argument 2 should be an item item ID" ) );

    entityID = atoi( args.arg( 1 ).c_str() );
    itemID = atoi( args.arg( 2 ).c_str() );

    if( !who->IsInSpace() )
        throw PyException( MakeCustomError( "You must be in space to unspawn things." ) );

    // Search for the itemRef for itemID:
    InventoryItemRef itemRef = who->services().item_factory.GetItem( itemID );
    SystemEntity * entityRef = who->System()->get( itemID );

    // Actually do the unspawn using SystemManager's RemoveEntity:
    if( entityRef == NULL )
    {
        return new PyString( "Un-Spawn Failed: itemID not found." );
    }
    else
    {
        who->System()->RemoveEntity( entityRef );
        itemRef->Delete();
    }

    sLog.Log( "Command", "%s: Un-Spawned %u.", who->GetName(), itemID );

    return new PyString( "Un-Spawn successful." );
}

PyResult Command_dogma( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    //"dogma" "140019878" "agility" "=" "0.2"

    if( !(args.argCount() == 5) ) {
        throw PyException( MakeCustomError("Correct Usage: /dogma [itemID] [attributeName] = [value]") );
    }

    if( !args.isNumber( 1 ) ){
        throw PyException( MakeCustomError("Invalid itemID. \n Correct Usage: /dogma [itemID] [attributeName] = [value]") );
    }
    uint32 itemID = atoi( args.arg( 1 ).c_str() );

    if( args.isNumber( 2 ) ) {
        throw PyException( MakeCustomError("Invalid attributeName. \n Correct Usage: /dogma [itemID] [attributeName] = [value]") );
    }
    const char *attributeName = args.arg( 2 ).c_str();

    if( !args.isNumber( 4 ) ){
        throw PyException( MakeCustomError("Invalid attribute value. \n Correct Usage: /dogma [itemID] [attributeName] = [value]") );
    }
    double attributeValue = atof( args.arg( 4 ).c_str() );

    //get item
    InventoryItemRef item = services->item_factory.GetItem( itemID );

    //get attributeID
    uint32 attributeID = db->GetAttributeID( attributeName );

    sLog.Warning( "GMCommands: Command_dogma()", "This command will modify attribute and send change to client, but change does not take effect in client for some reason." );
    item->SetAttribute( attributeID, attributeValue );

    return NULL;
}

PyResult Command_kick( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    Client *target;

    if( args.argCount() == 2 )
    {

        if( args.isNumber( 1 ) )
        {
            int id = atoi( args.arg( 1 ).c_str() );
            target = services->entity_list.FindCharacter( id );
        }
        else
        {
            const char *name = args.arg( 1 ).c_str();
            target = services->entity_list.FindCharacter( name );
        }
    }
    //support for characters with first and last names
    else if( args.argCount() == 3 )
    {
        if( args.isHexNumber( 1 ) )
            throw PyException( MakeCustomError("Unknown arguments") );

        std::string name = args.arg( 1 ) + " " + args.arg( 2 );
        target = services->entity_list.FindCharacter( name.c_str() ) ;
    }
    else
        throw PyException( MakeCustomError("Correct Usage: /kick [Character Name]") );

    if(target == NULL)
        throw PyException( MakeCustomError( "Cannot find Character" ) );
    else
        target->DisconnectClient();

    return NULL;
}

PyResult Command_ban( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    Client *target;

    if( args.argCount() == 2 )
    {

        if( !args.isNumber( 1 ) )
        {
            const char *name = args.arg( 1 ).c_str();
            target = services->entity_list.FindCharacter( name );
        }
        else
            throw PyException( MakeCustomError("Correct Usage: /ban [Character Name]") );
    }
    //support for characters with first and last names
    else if( args.argCount() == 3 )
    {
        if( args.isHexNumber( 1 ) )
            throw PyException( MakeCustomError("Unknown arguments") );

        std::string name = args.arg( 1 ) + " " + args.arg( 2 );
        target = services->entity_list.FindCharacter( name.c_str() ) ;
    }
    else
        throw PyException( MakeCustomError("Correct Usage: /ban [Character Name]") );

    //ban client
    target->BanClient();

    //disconnect client
    target->DisconnectClient();

    return NULL;
}

PyResult Command_unban( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    if( args.argCount() == 2 )
    {

        if( !args.isNumber( 1 ) )
        {
            const char *name = args.arg( 1 ).c_str();
            services->serviceDB().SetAccountBanStatus(db->GetAccountID(name),false);
        }
        else
            throw PyException( MakeCustomError("Correct Usage: /ban [Character Name]") );
    }
    //support for characters with first and last names
    else if( args.argCount() == 3 )
    {
        if( args.isHexNumber( 1 ) )
            throw PyException( MakeCustomError("Unknown arguments") );

        std::string name = args.arg( 1 ) + " " + args.arg( 2 );
        services->serviceDB().SetAccountBanStatus(db->GetAccountID(name),false);
    }
    else
        throw PyException( MakeCustomError("Correct Usage: /unban [Character Name / Character ID]") );

    return NULL;
}

PyResult Command_kenny( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    if( args.argCount() == 2 )
    {
        if( args.arg(1) == "ON" || args.arg(1) == "On" || args.arg(1) == "oN" || args.arg(1) == "on" ||
            args.arg(1) == "1" )
        {
            // Enable Kenny Translator
            who->EnableKennyTranslator();
        }
        else if( args.arg(1) == "OFF" || args.arg(1) == "off" || args.arg(1) == "Off" ||
            args.arg(1) == "0" )
        {
            // Disable Kenny Translator
            who->DisableKennyTranslator();
        }
        else
            throw PyException( MakeCustomError("Correct Usage: /kenny ON/OFF On/Off on/off 1/0") );
    }
    else
        throw PyException( MakeCustomError("Correct Usage: /kenny ON/OFF On/Off on/off 1/0") );

    return NULL;
}

PyResult Command_kill( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    if( args.argCount() == 2 )
    {
        if( !args.isNumber( 1 ) )
            {
                throw PyException( MakeCustomError( "Argument 1 should be a character ID" ) );
            }
        uint32 entity = atoi( args.arg( 1 ).c_str() );

        InventoryItemRef itemRef = services->item_factory.GetShip(entity);
        if( itemRef == NULL )
            throw PyException( MakeCustomError("/kill NOT supported on non-ship types at this time") );

        // WARNING: This cast of SystemEntity * to DynamicSystemEntity * will CRASH if the get() does not return
        // an object that IS a DynamicSystemEntity!!!
        DynamicSystemEntity * shipEntity = (DynamicSystemEntity *)(who->System()->get(entity));
        if( shipEntity == NULL )
		{
			throw PyException( MakeCustomError("/kill cannot process this object") );
			sLog.Error("GMCommands - Command_kill()", "Cannot process this object, aborting kill: %s [%u]", itemRef->itemName().c_str(), itemRef->itemID());
		}
		else
		{
			if(shipEntity->IsNPC())
			{
				NPC * npcEntity = shipEntity->CastToNPC();
				//npcEntity->AI()->ClearAllTargets();
				//npcEntity->Destiny()->SendTerminalExplosion();
				//npcEntity->Bubble()->Remove(npcEntity, true);
				//npcEntity->System()->RemoveEntity(npcEntity);
				Damage fatal_blow((static_cast<SystemEntity*>(who)),true);
				npcEntity->Killed(fatal_blow);
				delete npcEntity;
			}
			else
			{
				//shipEntity->Destiny()->SendTerminalExplosion();
				//shipEntity->Bubble()->Remove(shipEntity, true);
				Damage fatal_blow((static_cast<SystemEntity*>(who)),true);
				shipEntity->Killed(fatal_blow);
				itemRef->Delete();
			}
		}
    }
    else
        throw PyException( MakeCustomError("Correct Usage: /kill <entityID>") );

    return NULL;
}

PyResult Command_killallnpcs( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    if( args.argCount() == 1 )
    {
		std::set<SystemEntity *> whosBubbleEntityList;
		who->Bubble()->GetEntities(whosBubbleEntityList);
		std::set<SystemEntity *>::const_iterator cur, end;
		cur = whosBubbleEntityList.begin();
		end = whosBubbleEntityList.end();
	    for(; cur != end; cur++) {
			if( (*cur)->IsNPC() )
			{
				//shipEntity->Killed(<fill with an instance of Damage class populated with appropriate information>);
				NPC * npcEntity = (*cur)->CastToNPC();
				//npcEntity->AI()->ClearAllTargets();
				//npcEntity->Destiny()->SendTerminalExplosion();
				//npcEntity->Bubble()->Remove(npcEntity, true);
				//npcEntity->System()->RemoveEntity(npcEntity);
				Damage fatal_blow((static_cast<SystemEntity*>(who)),true);
				npcEntity->Killed(fatal_blow);
				delete npcEntity;
			}
		}
    }
    else
        throw PyException( MakeCustomError("Correct Usage: /killallnpcs") );

    return NULL;
}

PyResult Command_cloak( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    if( args.argCount() == 1 )
    {
		if( who->IsInSpace() )
		{
			if( who->Destiny()->IsCloaked() )
				who->Destiny()->UnCloak();
			else
				who->Destiny()->Cloak();
		}
		else
			throw PyException( MakeCustomError("ERROR!  You MUST be in space to cloak!") );
    }
    else
        throw PyException( MakeCustomError("Correct Usage: /cloak") );

    return NULL;
}

