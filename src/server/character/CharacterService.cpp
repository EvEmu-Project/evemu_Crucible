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

#include "EvemuPCH.h"

PyCallable_Make_InnerDispatcher(CharacterService)

CharacterService::CharacterService(PyServiceMgr *mgr, DBcore *dbc)
: PyService(mgr, "character"),
  m_dispatch(new Dispatcher(this)),
  m_db(dbc)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(CharacterService, GetCharactersToSelect)
	PyCallable_REG_CALL(CharacterService, GetCharacterToSelect)
	PyCallable_REG_CALL(CharacterService, SelectCharacterID)
	PyCallable_REG_CALL(CharacterService, GetOwnerNoteLabels)
	PyCallable_REG_CALL(CharacterService, GetCharCreationInfo)
	PyCallable_REG_CALL(CharacterService, GetCharNewExtraCreationInfo)
	PyCallable_REG_CALL(CharacterService, GetAppearanceInfo)
	PyCallable_REG_CALL(CharacterService, ValidateName)
	PyCallable_REG_CALL(CharacterService, ValidateNameEx)
	PyCallable_REG_CALL(CharacterService, CreateCharacter)
	PyCallable_REG_CALL(CharacterService, CreateCharacter2) // replica of CreateCharacter for now.
	PyCallable_REG_CALL(CharacterService, Ping)
	PyCallable_REG_CALL(CharacterService, PrepareCharacterForDelete)
	PyCallable_REG_CALL(CharacterService, CancelCharacterDeletePrepare)
	PyCallable_REG_CALL(CharacterService, AddOwnerNote)
	PyCallable_REG_CALL(CharacterService, EditOwnerNote)
	PyCallable_REG_CALL(CharacterService, GetOwnerNote)
	PyCallable_REG_CALL(CharacterService, GetHomeStation)
	PyCallable_REG_CALL(CharacterService, GetCloneTypeID)
	PyCallable_REG_CALL(CharacterService, GetCharacterAppearanceList)
	PyCallable_REG_CALL(CharacterService, GetRecentShipKillsAndLosses)

	PyCallable_REG_CALL(CharacterService, GetCharacterDescription)
	PyCallable_REG_CALL(CharacterService, SetCharacterDescription)

	PyCallable_REG_CALL(CharacterService, GetNote)
	PyCallable_REG_CALL(CharacterService, SetNote)

	PyCallable_REG_CALL(CharacterService, LogStartOfCharacterCreation)
}

CharacterService::~CharacterService() {
	delete m_dispatch;
}

PyResult CharacterService::Handle_GetCharactersToSelect(PyCallArgs &call) {
	return(m_db.GetCharacterList(call.client->GetAccountID()));
}

PyResult CharacterService::Handle_GetCharacterToSelect(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return NULL;
	}

	PyRep *result = m_db.GetCharSelectInfo(args.arg);
	if(result == NULL) {
		_log(CLIENT__ERROR, "Failed to load character %d", args.arg);
		return NULL;
	}

	return(result);
}

PyResult CharacterService::Handle_SelectCharacterID(PyCallArgs &call) {
	CallSelectCharacterID args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Failed to parse args from account %lu", call.client->GetAccountID());
		//TODO: throw exception
		return NULL;
	}

	//we don't care about tutorial dungeon right now
	call.client->SelectCharacter(args.charID);

	return NULL;
}

PyResult CharacterService::Handle_GetOwnerNoteLabels(PyCallArgs &call) {
	return m_db.GetOwnerNoteLabels(call.client->GetCharacterID());
}

PyResult CharacterService::Handle_GetCharCreationInfo(PyCallArgs &call) {
	PyRepDict *result = new PyRepDict();

	//send all the cache hints needed for char creation.
	m_manager->cache_service->InsertCacheHints(
		ObjCacheService::hCharCreateCachables,
		result);
	_log(CLIENT__MESSAGE, "Sending char creation info reply");

	return(result);
}

PyResult CharacterService::Handle_GetCharNewExtraCreationInfo(PyCallArgs &call) {
	PyRepDict *result = new PyRepDict();

	m_manager->cache_service->InsertCacheHints(
		ObjCacheService::hCharNewExtraCreateCachables,
		result);
	_log(CLIENT__MESSAGE, "Sending char new extra creation info reply");

	return(result);
}

PyResult CharacterService::Handle_GetAppearanceInfo(PyCallArgs &call) {
	PyRepDict *result = new PyRepDict();

	//send all the cache hints needed for char creation.
	m_manager->cache_service->InsertCacheHints(
		ObjCacheService::hAppearanceCachables,
		result );

	_log(CLIENT__MESSAGE, "Sending appearance info reply");

	return(result);
}

PyResult CharacterService::Handle_ValidateName(PyCallArgs &call) {
	Call_SingleStringArg arg;
	if(!arg.Decode(&call.tuple)) {
		_log(SERVICE__ERROR, "Failed to decode args.");
		return NULL;
	}

	// perform checks on the "name" that is passed.  we may want to impliment something
	// to limit the kind of names allowed.  We also dont want blank names being allowed.
	if( arg.arg == "" )
	{
		_log(CLIENT__ERROR, "Blank character name passed.  Bailing.");
		return NULL;
	}

	return(new PyRepBoolean(m_db.ValidateCharName(arg.arg.c_str())));
}

PyResult CharacterService::Handle_ValidateNameEx(PyCallArgs &call) {
	//just redirect it now
	return(Handle_ValidateName(call));
}

PyResult CharacterService::Handle_CreateCharacter(PyCallArgs &call) {
	CallCreateCharacter arg;
	if(!arg.Decode(&call.tuple)) {
		_log(CLIENT__ERROR, "Failed to decode CreateCharacter arg.");
		return NULL;
	}

	CharacterData cdata;
	cdata.name = arg.name;
	cdata.bloodlineID = arg.bloodlineID;
	cdata.gender = arg.genderID;
	cdata.ancestryID = arg.ancestryID;
	cdata.intelligence = arg.IntelligenceAdd;
	cdata.charisma = arg.CharismaAdd;
	cdata.perception = arg.PerceptionAdd;
	cdata.memory = arg.MemoryAdd;
	cdata.willpower = arg.WillpowerAdd;

	std::map<std::string, PyRep *>::iterator itr = call.byname.find("careerID");
	if(itr != call.byname.end()) {
		if(!itr->second->IsInteger()) {
			_log(CLIENT__ERROR, "Wrong careerID type! Expected integer, got %s.", itr->second->TypeString());
			return NULL;
		}
		cdata.careerID = itr->second->AsInteger().value;
	} else {
		_log(CLIENT__ERROR, "CareerID not found.");
		return NULL;
	}

	itr = call.byname.find("careerSpecialityID");
	if(itr != call.byname.end()) {
		if(!itr->second->IsInteger()) {
			_log(CLIENT__ERROR, "Wrong careerSpecialityID type! Expected integer, got %s.", itr->second->TypeString());
			return NULL;
		}
		cdata.careerSpecialityID = itr->second->AsInteger().value;
	} else {
		_log(CLIENT__ERROR, "CareerSpecialityID not found.");
		return NULL;
	}

	_log(CLIENT__MESSAGE, "CreateCharacter called for '%s'", cdata.name.c_str());
	_log(CLIENT__MESSAGE, "  bloodlineID=%lu genderID=%lu ancestryID=%lu careerID=%lu careerSpecialityID=%lu",
			cdata.bloodlineID, cdata.gender, cdata.ancestryID, cdata.careerID, cdata.careerSpecialityID);
	_log(CLIENT__MESSAGE, "  +INT=%lu +CHA=%lu +PER=%lu +MEM=%lu +WIL=%lu",
			cdata.intelligence, cdata.charisma, cdata.perception, cdata.memory, cdata.willpower);

	CharacterAppearance capp;
	//this builds appearance data from strdict
	capp.Build(arg.appearance);

	//TODO: choose these based on selected parameters...
	// Setting character's starting position, and getting it's location...
	GPoint pos(0.0, 0.0, 0.0);

	if(	!m_db.GetLocationCorporationByCareer(cdata, pos)
		|| !m_db.GetAttributesFromBloodline(cdata)
		|| !m_db.GetAttributesFromAncestry(cdata) ) 
	{
		codelog(CLIENT__ERROR, "Failed to load char create details. Bloodline %lu, ancestry %lu, career %lu.",
			cdata.bloodlineID, cdata.ancestryID, cdata.careerID);
		return NULL;
	}

	cdata.raceID = m_db.GetRaceByBloodline(cdata.bloodlineID);
	if(cdata.raceID == 0)
	{
		codelog(CLIENT__ERROR, "Failed to find raceID from bloodline %lu", cdata.bloodlineID);
		return NULL;
	}

	//NOTE: these are currently hard coded to Todaki because other things are
	//also hard coded to only work in Todaki. Once these various things get fixed,
	//just take this code out and the above calls should have cdata populated correctly.

	cdata.stationID = 60004420;
	cdata.solarSystemID = 30001407;
	cdata.constellationID = 20000206;
	cdata.regionID = 10000016;

	cdata.bounty = 0;
	cdata.balance = sConfig.server.startBalance;
	cdata.securityRating = 0;
	cdata.logonMinutes = 0;
	cdata.title = "No Title";

	cdata.startDateTime = Win32TimeNow();
	cdata.createDateTime = cdata.startDateTime;
	cdata.corporationDateTime = cdata.startDateTime;

	//obtain ship type
	uint32 shipTypeID;
	if(!m_db.GetShipTypeByBloodline(cdata.bloodlineID, shipTypeID))
		return NULL;

	typedef std::map<uint32, uint32>	CharSkillMap;
	typedef CharSkillMap::iterator		CharSkillMapItr;

	//load skills
	CharSkillMap startingSkills;
	if(	   !m_db.GetSkillsByRace(cdata.raceID, startingSkills)
		|| !m_db.GetSkillsByCareer(cdata.careerID, startingSkills)
		|| !m_db.GetSkillsByCareerSpeciality(cdata.careerSpecialityID, startingSkills ))
	{
		codelog(CLIENT__ERROR, "Failed to load char create skills. Bloodline %lu, ancestry %lu.",
			cdata.bloodlineID, cdata.ancestryID);
		return NULL;
	}

	//now we have all the data we need, stick it in the DB
	InventoryItem *char_item = m_db.CreateCharacter(call.client->GetAccountID(), m_manager->item_factory, cdata, capp);
	if(char_item == NULL)
	{
		//a return to the client of 0 seems to be the only means of marking failure
		codelog(CLIENT__ERROR, "Failed to create character '%s'", cdata.name.c_str());
		return NULL;
	}
	cdata.charid = char_item->itemID();

	//spawn all the skills
	CharSkillMapItr cur, end;
	cur = startingSkills.begin();
	end = startingSkills.end();
	for(; cur != end; cur++)
	{
		ItemData skillItem( cur->first, cdata.charid, cdata.charid, flagSkill );
		InventoryItem *i = m_manager->item_factory.SpawnItem( skillItem );
		if(i == NULL) {
			_log(CLIENT__ERROR, "Failed to add skill %lu to char %s (%lu) during char create.", cur->first, cdata.name.c_str(), cdata.charid);
			continue;
		}

		i->Set_skillLevel(cur->second);
		i->Set_skillPoints( GetSkillPointsForSkillLevel(i, cur->second));
		_log(CLIENT__MESSAGE, "Trained skill %s to level %d (%d points)", i->itemName().c_str(), i->skillLevel(), i->skillPoints());

		//we dont actually need the item anymore...
		i->Release();
	}

	//now set up some initial inventory:
	InventoryItem *initInvItem;

	// add "Damage Control I"
	ItemData itemDamageControl( 2046, cdata.charid, cdata.stationID, flagHangar, 1 );
	initInvItem = m_manager->item_factory.SpawnItem( itemDamageControl );
	if(initInvItem == NULL)
		codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", cdata.name.c_str());
	else
		initInvItem->Release();

	// add 1 unit of "Tritanium"
	ItemData itemTritanium( 34, cdata.charid, cdata.stationID, flagHangar, 1 );
	initInvItem = m_manager->item_factory.SpawnItem( itemTritanium );

	if(initInvItem == NULL)
		codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", cdata.name.c_str());
	else
		initInvItem->Release();
	
	// give the player its ship.
	std::string ship_name = cdata.name + "'s Ship";

	// shipTypeID, // The race-specific start ship
	ItemData shipItem( shipTypeID, cdata.charid, cdata.stationID, flagHangar, ship_name.c_str() );
	InventoryItem *ship_item = m_manager->item_factory.SpawnItem(shipItem);

	if(ship_item == NULL)
	{
		codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", cdata.name.c_str());
	}
	else
	{
		//welcome on board your starting ship
		char_item->MoveInto(ship_item, flagPilot, false);
		ship_item->Release();
	}

	//recursively save everything we just did.
	char_item->Save(true);
	char_item->Release();

	_log(CLIENT__MESSAGE, "Sending char create ID %d as reply", cdata.charid);

	return new PyRepInteger(cdata.charid);
}

/**
 * @todo clean this from unneeded comments. Replace the read query actions by a system that uses memory.
 */
PyResult CharacterService::Handle_CreateCharacter2(PyCallArgs &call)
{
	CallCreateCharacter2 arg;
	if(!arg.Decode(&call.tuple)) {
		_log(CLIENT__ERROR, "Failed to decode CreateCharacter2 arg.");
		return NULL;
	}

	CharacterData cdata;
	memset(&cdata, 0, sizeof(CharacterData));
	cdata.name = arg.name;
	cdata.bloodlineID = arg.bloodlineID;
	cdata.gender = arg.genderID;
	cdata.ancestryID = arg.ancestryID;

	// Since the careerID is no longer passed to CreateCharacter2, we need to find another way to get this,
	// or alter the DB to remove career related fields and FK's from the table defs.
	/* 
	std::map<std::string, PyRep *>::iterator itr = call.byname.find("careerID");
	if(itr != call.byname.end()) {
		if(!itr->second->IsInteger()) {
			_log(CLIENT__ERROR, "Wrong careerID type! Expected integer, got %s.", itr->second->TypeString());
			return NULL;
		}
		cdata.careerID = itr->second->AsInteger().value;
	} else {
		_log(CLIENT__ERROR, "CareerID not found.");
		return NULL;
	}
	*/

	// same with careerSpecialityID, it's not longer passed in CreateCharacter2, find another way to get this and set it.
	/*
	itr = call.byname.find("careerSpecialityID");
	if(itr != call.byname.end()) {
		if(!itr->second->IsInteger()) {
			_log(CLIENT__ERROR, "Wrong careerSpecialityID type! Expected integer, got %s.", itr->second->TypeString());
			return NULL;
		}
		cdata.careerSpecialityID = itr->second->AsInteger().value;
	} else {
		_log(CLIENT__ERROR, "CareerSpecialityID not found.");
		return NULL;
	}
	*/

	_log(CLIENT__MESSAGE, "CreateCharacter2 called for '%s'", cdata.name.c_str());
	_log(CLIENT__MESSAGE, "  bloodlineID=%lu genderID=%lu ancestryID=%lu careerID=%lu careerSpecialityID=%lu",
			cdata.bloodlineID, cdata.gender, cdata.ancestryID, cdata.careerID, cdata.careerSpecialityID);
	_log(CLIENT__MESSAGE, "  +INT=%lu +CHA=%lu +PER=%lu +MEM=%lu +WIL=%lu",
			cdata.intelligence, cdata.charisma, cdata.perception, cdata.memory, cdata.willpower);

	CharacterAppearance capp;
	//this builds appearance data from strdict
	capp.Build(arg.appearance);

	//TODO: choose these based on selected parameters...
	// Setting character's starting position, and getting it's location...
	GPoint pos(0.0, 0.0, 0.0);

	if(	!m_db.GetLocationCorporationByCareer(cdata, pos)
		|| !m_db.GetAttributesFromBloodline(cdata)
		|| !m_db.GetAttributesFromAncestry(cdata) ) {
		codelog(CLIENT__ERROR, "Failed to load char create details. Bloodline %lu, ancestry %lu, career %lu.",
			cdata.bloodlineID, cdata.ancestryID, cdata.careerID);
		return NULL;
	}
	
	cdata.raceID = m_db.GetRaceByBloodline(cdata.bloodlineID);
	if(cdata.raceID == 0) {
		codelog(CLIENT__ERROR, "Failed to find raceID from bloodline %lu",
			cdata.bloodlineID);
		return NULL;
	}

	//NOTE: these are currently hard coded to Todaki because other things are
	//also hard coded to only work in Todaki. Once these various things get fixed,
	//just take this code out and the above calls should have cdata populated correctly.

	cdata.stationID = 60004420;
	cdata.solarSystemID = 30001407;
	cdata.constellationID = 20000206;
	cdata.regionID = 10000016;

	cdata.bounty = 0;
	cdata.balance = sConfig.server.startBalance;
	cdata.securityRating = 0;
	cdata.logonMinutes = 0;
	cdata.title = "No Title";

	cdata.startDateTime = Win32TimeNow();
	cdata.createDateTime = cdata.startDateTime;
	cdata.corporationDateTime = cdata.startDateTime;

	//obtain ship type
	uint32 shipTypeID;
	if(!m_db.GetShipTypeByBloodline(cdata.bloodlineID, shipTypeID))
		return NULL;

	typedef std::map<uint32, uint32>	CharSkillMap;
	typedef CharSkillMap::iterator		CharSkillMapItr;
	
	//load skills
	CharSkillMap startingSkills;
	if( !m_db.GetSkillsByRace(cdata.raceID, startingSkills) )
	{
		codelog(CLIENT__ERROR, "Failed to load char create skills. Bloodline %lu, Ancestry %lu.",
			cdata.bloodlineID, cdata.ancestryID);
		return NULL;
	}

	//now we have all the data we need, stick it in the DB
	InventoryItem *char_item = m_db.CreateCharacter2(call.client->GetAccountID(), m_manager->item_factory, cdata, capp);
	if(char_item == NULL) {
		//a return to the client of 0 seems to be the only means of marking failure
		codelog(CLIENT__ERROR, "Failed to create character '%s'", cdata.name.c_str());
		return NULL;
	}
	cdata.charid = char_item->itemID();

	//spawn all the skills
	CharSkillMapItr cur, end;
	cur = startingSkills.begin();
	end = startingSkills.end();
	for(; cur != end; cur++) 
	{
		ItemData skillItem( cur->first, cdata.charid, cdata.charid, flagSkill );
		InventoryItem *i = m_manager->item_factory.SpawnItem( skillItem );
		if(i == NULL) {
			_log(CLIENT__ERROR, "Failed to add skill %lu to char %s (%lu) during char create.", cur->first, cdata.name.c_str(), cdata.charid);
			continue;
		}

		_log(CLIENT__MESSAGE, "Training skill %lu to level %d (%d points)", i->typeID(), i->skillLevel(), i->skillPoints());
		i->Set_skillLevel(cur->second);
		i->Set_skillPoints(GetSkillPointsForSkillLevel(i, cur->second));

		//we don't actually need the item anymore...
		i->Release();
	}

	//now set up some initial inventory:
	InventoryItem *initInvItem;

	// add "Damage Control I"
	ItemData itemDamageControl( 2046, cdata.charid, cdata.stationID, flagHangar, 1 );
	initInvItem = m_manager->item_factory.SpawnItem( itemDamageControl );
	if(initInvItem == NULL)
		codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", cdata.name.c_str());
	else
		initInvItem->Release();

	// add 1 unit of "Tritanium"
	ItemData itemTritanium( 34, cdata.charid, cdata.stationID, flagHangar, 1 );
	initInvItem = m_manager->item_factory.SpawnItem( itemTritanium );

	if(initInvItem == NULL)
		codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", cdata.name.c_str());
	else
		initInvItem->Release();

	// give the player its ship.
	std::string ship_name = cdata.name + "'s Ship";

	// shipTypeID, // The race-specific start ship
	ItemData shipItem( shipTypeID, cdata.charid, cdata.stationID, flagHangar, ship_name.c_str() );
	InventoryItem *ship_item = m_manager->item_factory.SpawnItem(shipItem);

	if(ship_item == NULL)
	{
		codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", cdata.name.c_str());
	}
	else
	{
		//welcome on board your starting ship
		char_item->MoveInto(ship_item, flagPilot, false);
		ship_item->Release();
	}

	//recursively save everything we just did.
	char_item->Save(true);
	char_item->Release();

	_log(CLIENT__MESSAGE, "Sending char create ID %d as reply", cdata.charid);

	return new PyRepInteger(cdata.charid);
}

PyResult CharacterService::Handle_Ping(PyCallArgs &call)
{
	return(new PyRepInteger(call.client->GetAccountID()));
}

PyResult CharacterService::Handle_PrepareCharacterForDelete(PyCallArgs &call) {
	/*
	 * Deletion is apparently a timed process, and is related to the
	 * deletePrepareDateTime column in the char select result.
	 *
	 * For now, we will just implement an immediate delete as its better than
	 * nothing, since clearing out all the tables by hand is a nightmare.
	 */
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return NULL;
	}

	//TODO: make sure this person actually owns this char...

	_log(CLIENT__MESSAGE, "Timed delete of char %lu unimplemented. Deleting Immediately.", args.arg);
	InventoryItem *char_item = m_manager->item_factory.GetItem(args.arg, true);
	if(char_item == NULL) {
		codelog(CLIENT__ERROR, "Failed to load char item %lu.", args.arg);
		return NULL;
	}
	//does the recursive delete of all contained items
	char_item->Delete();

	//now, clean up all items which werent deleted
	std::set<uint32> items;
	if(!m_db.GetCharItems(args.arg, items)) {
		codelog(CLIENT__ERROR, "Unable to get items of char %lu.", args.arg);
		return NULL;
	}

	std::set<uint32>::iterator cur, end;
	cur = items.begin();
	end = items.end();
	for(; cur != end; cur++) {
		InventoryItem *i = m_manager->item_factory.GetItem(*cur, true);
		if(i == NULL) {
			codelog(CLIENT__ERROR, "Failed to load item %lu to delete. Skipping.", *cur);
			continue;
		}

		i->Delete();
	}
	//finally, delete all 'character_' related tables
	m_db.DeleteCharacter(args.arg);

	//we return deletePrepareDateTime, in eve time format.
	return(new PyRepInteger(Win32TimeNow() + Win32Time_Second*5));
}

PyResult CharacterService::Handle_CancelCharacterDeletePrepare(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return NULL;
	}

	_log(CLIENT__ERROR, "Cancel delete (of char %lu) unimplemented.", args.arg);

	//returns nothing.
	return NULL;
}

PyResult CharacterService::Handle_AddOwnerNote(PyCallArgs &call) {
	Call_AddOwnerNote args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(new PyRepInteger(0));
	}

	uint32 noteID = m_db.AddOwnerNote(call.client->GetCharacterID(), args.label, args.content);
	if (noteID == 0) {
		codelog(SERVICE__ERROR, "%s: Failed to set Owner note.", call.client->GetName());
		return (new PyRepNone());
	}

	return(new PyRepInteger(noteID));
}

PyResult CharacterService::Handle_EditOwnerNote(PyCallArgs &call) {
	Call_EditOwnerNote args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(new PyRepInteger(0));
	}

	if (!m_db.EditOwnerNote(call.client->GetCharacterID(), args.noteID, args.label, args.content)) {
		codelog(SERVICE__ERROR, "%s: Failed to set Owner note.", call.client->GetName());
		return (new PyRepNone());
	}

	return(new PyRepInteger(args.noteID));
}

PyResult CharacterService::Handle_GetOwnerNote(PyCallArgs &call) {
	Call_SingleIntegerArg arg;
	if (!arg.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(new PyRepInteger(0));
	}
	return m_db.GetOwnerNote(call.client->GetCharacterID(), arg.arg);
}


PyResult CharacterService::Handle_GetHomeStation(PyCallArgs &call) {
	//returns the station ID of the station where this player's Clone is
	PyRep *result = NULL;

	_log(CLIENT__ERROR, "GetHomeStation() is not really implemented.");

	result = new PyRepInteger(call.client->GetStationID());

	return(result);
}

PyResult CharacterService::Handle_GetCloneTypeID(PyCallArgs &call) {
	PyRep *result = NULL;

	_log(CLIENT__ERROR, "GetCloneTypeID() is not really implemented.");

	//currently hardcoded Clone Grade Alpha
	result = new PyRepInteger(164);

	return(result);
}

PyResult CharacterService::Handle_GetRecentShipKillsAndLosses(PyCallArgs &call) {
	_log(SERVICE__WARNING, "%s::GetRecentShipKillsAndLosses unimplemented.", GetName());

	util_Rowset rs;

	rs.header.push_back("killID");
	rs.header.push_back("solarSystemID");
	rs.header.push_back("victimCharacterID");
	rs.header.push_back("victimCorporationID");
	rs.header.push_back("victimAllianceID");
	rs.header.push_back("victimFactionID");
	rs.header.push_back("victimShipTypeID");
	rs.header.push_back("finalCharacterID");
	rs.header.push_back("finalCorporationID");
	rs.header.push_back("finalAllianceID");
	rs.header.push_back("finalFactionID");
	rs.header.push_back("finalShipTypeID");
	rs.header.push_back("finalWeaponTypeID");
	rs.header.push_back("killBlob");	//string
	rs.header.push_back("killTime");	//uint64
	rs.header.push_back("victimDamageTaken");
	rs.header.push_back("finalSecurityStatus");	//real
	rs.header.push_back("finalDamageDone");
	rs.header.push_back("moonID");

	return(rs.FastEncode());
}

/////////////////////////
PyResult CharacterService::Handle_GetCharacterDescription(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return NULL;
	}

	PyRep *result = m_db.GetCharDesc(args.arg);
	if(result == NULL) {
		_log(CLIENT__ERROR, "Failed to get character description %d", args.arg);
		return NULL;
	}

	return(result);
}

//////////////////////////////////
PyResult CharacterService::Handle_SetCharacterDescription(PyCallArgs &call) {
	Call_SingleStringArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return NULL;
	}

	m_db.SetCharDesc(call.client->GetCharacterID(), args.arg.c_str());

	return NULL;
}

PyResult CharacterService::Handle_GetCharacterAppearanceList(PyCallArgs &call) {
	Call_SingleIntList args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: Invalid arguments", call.client->GetName());
		return NULL;
	}

	PyRepList *l = new PyRepList();

	std::vector<uint32>::iterator cur, end;
	cur = args.ints.begin();
	end = args.ints.end();
	for(; cur != end; cur++) {
		PyRep *r = m_db.GetCharacterAppearance(*cur);
		if(r == NULL)
			r = new PyRepNone();

		l->add(r);
	}

	return(l);
}

/** Retrieves a Character note.
*
* Checks the database for a given character note and returns it.
*
* @return PyRepSubStream pointer with the string with the note or with PyRepNone if no note is stored.
*
*  **LSMoura
*/
PyResult CharacterService::Handle_GetNote(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return NULL;
	}

	return(m_db.GetNote(call.client->GetCharacterID(), args.arg));
}

/** Stores a Character note.
*
*  Stores the given character note in the database.
*
* @return PyRepSubStream pointer with a PyRepNone() value.
*
*  **LSMoura
*/
PyResult CharacterService::Handle_SetNote(PyCallArgs &call) {
	Call_SetNote args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return NULL;
	}

	if (!m_db.SetNote(call.client->GetCharacterID(), args.itemID, args.note.c_str()))
		codelog(CLIENT__ERROR, "Error while storing the note on the database.");

	return NULL;
}

uint32 CharacterService::GetSkillPointsForSkillLevel(InventoryItem *i, uint8 level)
{
	if (i == NULL)
		return 0;

	float fLevel = level;
	fLevel = (fLevel - 1.0) * 0.5;

	uint32 calcSkillPoints = SKILL_BASE_POINTS * i->skillTimeConstant() * pow(32, fLevel);
	return calcSkillPoints;
}

PyResult CharacterService::Handle_LogStartOfCharacterCreation(PyCallArgs &call)
{
	/* we seem to send nothing back. */
	return NULL;
}
