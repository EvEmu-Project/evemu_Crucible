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

PyResult Command_create( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
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

	who->MoveToLocation( loc, p );
	return new PyString( "Translocation successfull." );
}

PyResult Command_giveisk( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
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
		throw PyException( MakeCustomError( "Bad arguments." ) );

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
		throw PyException( MakeCustomError( "Arguments must be 3 numbers" ) );
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
