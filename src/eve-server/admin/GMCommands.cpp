/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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
	Author:		Zhur
*/

#include "EVEServerPCH.h"

//#include "PyCallable.h"
#include "Client.h"
#include "inventory/InventoryDB.h"
#include "inventory/InventoryItem.h"

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
	i->Move( locationID, flag, true );

	return new PyString( "Creation successfull." );
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
	i->Move( locationID, flag, true );

	return new PyString( "Creation successfull." );
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
	return new PyString( "Translocation successfull." );
}


PyResult Command_tr( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
	if( args.argCount() < 3 ) {
		throw PyException( MakeCustomError("Correct Usage: /tr [entityID]") );
	}

	const std::string& name = args.arg( 1 );
	if( "me" != name )
		throw PyException( MakeCustomError( "Translocate (/TR) to non-me who '%s' is not supported yet.", name.c_str() ) );
	
	if( !args.isNumber( 2 ) )
		throw PyException( MakeCustomError( "Argument 1 should be an entity ID" ) );
    uint32 loc = atoi( args.arg( 2 ).c_str() );

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

	who->MoveToLocation( loc , p );
	return new PyString( "Translocation successfull." );
}

PyResult Command_giveisk( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{

	if( args.argCount() < 3 ) {
		throw PyException( MakeCustomError("Correct Usage: /giveisk [entityID (0=self)] [amount]") );
	}

	if( !args.isNumber( 1 ) )
		throw PyException( MakeCustomError( "Argument 1 should be an entity ID (0=self)" ) );
	uint32 entity = atoi( args.arg( 1 ).c_str() );

	if( !args.isNumber( 2 ) )
		throw PyException( MakeCustomError( "Argument 2 should be an amount of ISK" ) );
	double amount = strtod( args.arg( 2 ).c_str(), NULL );
	
	Client* tgt;
	if( 0 == entity )
		tgt = who;
	else
    {
		tgt = services->entity_list.FindCharacter( entity );
		if( NULL == tgt )
			throw PyException( MakeCustomError( "Unable to find character %u", entity ) );
	}
	
	tgt->AddBalance( amount );
	return new PyString( "Operation successfull." );
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
	return new PyString( "Goto successfull." );
}

PyResult Command_spawn( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
	if( args.argCount() < 2 ) {
		throw PyException( MakeCustomError("Correct Usage: /spawn [typeID]") );
	}
	
	if( !args.isNumber( 1 ) )
		throw PyException( MakeCustomError( "Argument 1 should be an item type ID" ) );
    const uint32 typeID = atoi( args.arg( 1 ).c_str() );

    sLog.Log( "Command", "%s: Summon %u.", who->GetName(), typeID );

	if( !who->IsInSpace() )
		throw PyException( MakeCustomError( "You must be in space to spawn things." ) );
	
	//hacking it...
	GPoint loc( who->GetPosition() );
	loc.x += 1500;

	ItemData idata(
		typeID,
		who->GetCorporationID(), //owner
		who->GetLocationID(),
		flagAutoFit,
		"",
		loc
	);

	InventoryItemRef i = services->item_factory.SpawnItem( idata );
	if( !i )
		throw PyException( MakeCustomError( "Unable to spawn item of type %u.", typeID ) );

	SystemManager* sys = who->System();
	NPC* it = new NPC( sys, *services, i, who->GetCorporationID(), who->GetAllianceID(), loc );
	sys->AddNPC( it );

	return new PyString( "Spawn successfull." );
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

	return item->attributes.PyGet( attribute );
}

PyResult Command_setattr( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
	if( args.argCount() < 4 ) {
		throw PyException( MakeCustomError("Correct Usage: /setattr [itemID] [attributeID] [value]") );
	}

	if( !args.isNumber( 1 ) )
		throw PyException( MakeCustomError( "1st argument must be itemID (got %s).", args.arg( 1 ).c_str() ) );
    const uint32 itemID = atoi( args.arg( 1 ).c_str() );

	if( !args.isNumber( 2 ) )
		throw PyException( MakeCustomError( "2nd argument must be attributeID (got %s).", args.arg( 2 ).c_str() ) );
    const ItemAttributeMgr::Attr attribute = (ItemAttributeMgr::Attr)atoi( args.arg( 2 ).c_str() );

	if( !args.isNumber( 3 ) )
		throw PyException( MakeCustomError( "3rd argument must be value (got %s).", args.arg( 3 ).c_str() ) );
    const double value = atof( args.arg( 3 ).c_str() );

	InventoryItemRef item = services->item_factory.GetItem( itemID );
	if( !item )
		throw PyException( MakeCustomError( "Failed to load item %u.", itemID ) );

	item->attributes.SetReal( attribute, value );

	return new PyString( "Operation successfull." );
}

PyResult Command_fit(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
	
	if( args.argCount() < 2 ) {
		throw PyException( MakeCustomError("Correct Usage: /fit [typeID] ") );
	}
	
	uint32 typeID;

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
		throw PyException( MakeCustomError( "Unable to create item of type %s.", typeID ) );

	who->MoveItem( i->itemID(), who->GetShipID(), flag );

	return new PyString( "Creation successfull." );

}

PyResult Command_giveskills( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args ) {

	//pass to command_giveskill
	Command_giveskill(who,db,services,args);

	return NULL;

}

PyResult Command_giveskill( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{

	uint32 typeID;
	uint8 level;
	CharacterRef character;
	EVEItemFlags flag;
	uint32 gty = 1;
	uint8 oldSkillLevel = 0;
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
			const char *name = args.arg( 1 ).c_str();
			Client *target = services->entity_list.FindCharacter( name );
			if(target == NULL)
				throw PyException( MakeCustomError( "Cannot find Character by the name of %s", name ) );
			ownerID = target->GetCharacterID();
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

	ItemData idata(
		typeID,
		ownerID,
		0, //temp location
		flag = (EVEItemFlags)flagSkill,
		gty
	);

	InventoryItemRef item = services->item_factory.SpawnItem( idata );
	SkillRef skill = SkillRef::StaticCast( item );


	if( !item )
		throw PyException( MakeCustomError( "Unable to create item of type %s.", item->typeID() ) );
	
	if(character->HasSkill( skill->typeID() ) )
	{
		SkillRef oldSkill = character->GetSkill( skill->typeID() );
		oldSkillLevel = oldSkill->attributes.GetInt( oldSkill->attributes.Attr_skillLevel );
	}
		
	if( level > oldSkillLevel )
	{
		character->InjectSkillIntoBrain( skill, level);
		return new PyString ( "Gifting skills complete" );
	}



	return new PyString ("Skill Gifting Failure");
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
		//check if in capsule. this is very bad. a better aproach would be an
		//inPod function, but this is a fast hack
		if( tgt->GetShip()->typeID()!=670 )
			tgt->modules.OnlineAll();
		else
			throw PyException( MakeCustomError( "Command failed: You can't activate modules while in pod"));

		return(new PyString("All modules have been put Online"));
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
			tgt->modules.UnloadModule(item);

		if( args.argCount() == 3 && strcmp("all", args.arg( 2 ).c_str())==0)
			tgt->modules.UnloadAllModules();

		return(new PyString("All modules have been unloaded"));
	}
	else
		throw PyException( MakeCustomError( "Command failed: You got the arguments all wrong!"));
}

PyResult Command_heal( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
	if( args.argCount()== 1 )
	{
		who->GetShip()->Set_armorDamage(0);
		who->GetShip()->Set_damage(0);
		who->GetShip()->Set_shieldCharge(who->GetShip()->shieldCapacity());
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
		target->GetShip()->Set_armorDamage(0);
		target->GetShip()->Set_damage(0);
		target->GetShip()->Set_shieldCharge(who->GetShip()->shieldCapacity());
	}

	return(new PyString("Heal successful!"));
}

PyResult Command_repairmodules( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
	
	if(args.argCount()==1)
	{
		who->modules.RepairModules();
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
		target->modules.RepairModules();
	}

	return(new PyString("Modules repaired successful!"));
}

PyResult Command_unspawn( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
	throw PyException( MakeCustomError("Unspawn not implemented yet!"));

	return NULL;
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
	double atributeValue = atof( args.arg( 4 ).c_str() );

	//get item
	InventoryItemRef item = services->item_factory.GetItem( itemID );

	//get attributeID
	uint32 attributeID = db->GetAttributeID( attributeName );
	
	




	return NULL;
}