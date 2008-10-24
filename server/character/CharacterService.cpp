/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

	PyCallable_REG_CALL(CharacterService, GetCharacterDescription)//mandela
	PyCallable_REG_CALL(CharacterService, SetCharacterDescription)//mandela

	PyCallable_REG_CALL(CharacterService, GetNote)//LSMoura
	PyCallable_REG_CALL(CharacterService, SetNote)//LSMoura
}

CharacterService::~CharacterService() {
	delete m_dispatch;
}

PyResult CharacterService::Handle_GetCharactersToSelect(PyCallArgs &call) {
	
	PyRep *result = NULL;

	result = m_db.GetCharacterList(call.client->GetAccountID());
	if(result == NULL) {
		//TODO: throw exception
		result = new PyRepNone();
	}
	
	return(result);
}

PyResult CharacterService::Handle_GetCharacterToSelect(PyCallArgs &call) {
	PyRep *result = NULL;
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return(NULL);
	}
	
	result = m_db.GetCharSelectInfo(args.arg);
	if(result == NULL) {
		_log(CLIENT__ERROR, "Failed to load character %d", args.arg);
		return(NULL);
	}
	
	return(result);
}

PyResult CharacterService::Handle_SelectCharacterID(PyCallArgs &call) {
	CallSelectCharacterID args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Failed to parse args from account %lu", call.client->GetAccountID());
		//TODO: throw exception
		return(NULL);
	}
	
	/*uint32 ship_id = m_db.GetCurrentShipID(args.arg);
	_log(CLIENT__MESSAGE, "Determined that %lu is the current ship for char %lu", ship_id, args.arg);
	
	if(!m_db.LoadCharacter(args.arg, call.client->GetChar())) {
		_log(CLIENT__ERROR, "Failed to load char %lu for selection", args.arg);
		//TODO: throw exception
	} else {


		if(!call.client->LoadInventory(ship_id)) {
			_log(CLIENT__ERROR, "Failed to load character inventory");
		} else if(!call.client->EnterSystem(call.client->Ship()->position())) {
			//error prints handled in EnterSystem.
			//TODO: throw exception
		} else {
			call.client->InitialEnterGame();
			//send the session changes to kick off the client state changes.
			call.client->SessionSync();
		}
	}*/

	//we don't care about tutorial dungeon right now
	if(call.client->Load(args.charID))
		//johnsus - characterOnline mod
		m_db.SetCharacterOnlineStatus(args.charID,true);

	return(NULL);
}

PyResult CharacterService::Handle_GetOwnerNoteLabels(PyCallArgs &call) {
	return m_db.GetOwnerNoteLabels(call.client->GetCharacterID());
}

PyResult CharacterService::Handle_GetCharCreationInfo(PyCallArgs &call) {
	PyRep *result = NULL;

	PyRepDict *cachables = new PyRepDict();
	result = cachables;

	//send all the cache hints needed for char creation.
	call.client->GetServices()->GetCache()->InsertCacheHints(
		ObjCacheService::hCharCreateCachables,
		cachables );
		
	_log(CLIENT__MESSAGE, "Sending char creation info reply");
	
	return(result);
}

PyResult CharacterService::Handle_GetCharNewExtraCreationInfo(PyCallArgs &call) {
	PyRepDict *result = new PyRepDict();

	m_manager->GetCache()->InsertCacheHints(
		ObjCacheService::hCharNewExtraCreateCachables,
		result);
	_log(CLIENT__MESSAGE, "Sending char new extra creation info reply");

	return(result);
}

PyResult CharacterService::Handle_GetAppearanceInfo(PyCallArgs &call) {
	PyRep *result = NULL;

	PyRepDict *cachables = new PyRepDict();
	result = cachables;

	//send all the cache hints needed for char creation.
	call.client->GetServices()->GetCache()->InsertCacheHints(
		ObjCacheService::hAppearanceCachables,
		cachables );
	
	_log(CLIENT__MESSAGE, "Sending appearance info reply");
	
	return(result);
}

PyResult CharacterService::Handle_ValidateName(PyCallArgs &call) {
	Call_SingleStringArg arg;

	if(!arg.Decode(&call.tuple)) {
		_log(SERVICE__ERROR, "Failed to decode args.");
		return(NULL);
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
		return(NULL);
	}

	CharacterData cdata;
	cdata.name = arg.name;
	cdata.bloodlineID = arg.bloodlineID;
	cdata.genderID = arg.genderID;
	cdata.ancestryID = arg.ancestryID;
	cdata.Intelligence = arg.IntelligenceAdd;
	cdata.Charisma = arg.CharismaAdd;
	cdata.Perception = arg.PerceptionAdd;
	cdata.Memory = arg.MemoryAdd;
	cdata.Willpower = arg.WillpowerAdd;

	std::map<std::string, PyRep *>::iterator itr = call.byname.find("careerID");
	if(itr != call.byname.end()) {
		if(!itr->second->CheckType(PyRep::Integer)) {
			_log(CLIENT__ERROR, "Wrong careerID type! Expected integer, got %s.", itr->second->TypeString());
			return(NULL);
		}
		cdata.careerID = ((PyRepInteger *)itr->second)->value;
	} else {
		_log(CLIENT__ERROR, "CareerID not found.");
		return(NULL);
	}

	itr = call.byname.find("careerSpecialityID");
	if(itr != call.byname.end()) {
		if(!itr->second->CheckType(PyRep::Integer)) {
			_log(CLIENT__ERROR, "Wrong careerSpecialityID type! Expected integer, got %s.", itr->second->TypeString());
			return(NULL);
		}
		cdata.careerSpecialityID = ((PyRepInteger *)itr->second)->value;
	} else {
		_log(CLIENT__ERROR, "CareerSpecialityID not found.");
		return(NULL);
	}

	_log(CLIENT__MESSAGE, "CreateCharacter called for '%s'", cdata.name.c_str());
	_log(CLIENT__MESSAGE, "  bloodlineID=%lu genderID=%lu ancestryID=%lu careerID=%lu careerSpecialityID=%lu", 
			cdata.bloodlineID, cdata.genderID, cdata.ancestryID, cdata.careerID, cdata.careerSpecialityID);
	_log(CLIENT__MESSAGE, "  +INT=%lu +CHA=%lu +PER=%lu +MEM=%lu +WIL=%lu", 
			cdata.Intelligence, cdata.Charisma, cdata.Perception, cdata.Memory, cdata.Willpower);

	CharacterAppearance capp;
	//this builds appearance data from strdict
	capp.Build(arg.appearance);

	//TODO: choose these based on selected parameters...
	// Setting character's starting position, and getting it's location...
	double x=0,y=0,z=0;

	if(   !m_db.GetLocationCorporationByCareer(cdata, x, y, z)
	   || !m_db.GetAttributesFromBloodline(cdata)
	   || !m_db.GetAttributesFromAncestry(cdata) ) {
		codelog(CLIENT__ERROR, "Failed to load char create details. Bloodline %lu, ancestry %lu, career %lu.",
			cdata.bloodlineID, cdata.ancestryID, cdata.careerID);
		return(NULL);
	}
	
	cdata.raceID = m_db.GetRaceByBloodline(cdata.bloodlineID);
	if(cdata.raceID == 0) {
		codelog(CLIENT__ERROR, "Failed to find raceID from bloodline %lu",
			cdata.bloodlineID);
		return(NULL);
	}

	//NOTE: these are currently hard coded to Todaki because other things are
	//also hard coded to only work in Todaki. Once these various things get fixed,
	//just take this code out and the above calls should have cdata populated correctly.
	cdata.stationID = 60004420;
	cdata.solarSystemID = 30001407;
	cdata.constellationID = 20000206;
	cdata.regionID = 10000016;

	cdata.bounty = 0;
	cdata.balance = 6666000000.0f;
	cdata.securityRating = 0;
	cdata.logonMinutes = 0;
	cdata.title = "No Title";

	cdata.startDateTime = Win32TimeNow();
	cdata.createDateTime = cdata.startDateTime;
	cdata.corporationDateTime = cdata.startDateTime;

	//obtain ship type
	uint32 shipTypeID;
	if(!m_db.GetShipTypeByBloodline(cdata.bloodlineID, shipTypeID))
		//error was already printed
		return(NULL);

	//load skills
	std::map<uint32, uint32> startingSkills;
	if(   !m_db.GetSkillsByRace(cdata.raceID, startingSkills)
	   || !m_db.GetSkillsByCareer(cdata.careerID, startingSkills)
	   || !m_db.GetSkillsByCareerSpeciality(cdata.careerSpecialityID, startingSkills)
	) {
		codelog(CLIENT__ERROR, "Failed to load char create skills. Bloodline %lu, ancestry %lu.",
			cdata.bloodlineID, cdata.ancestryID);
		return(NULL);
	}

	//now we have all the data we need, stick it in the DB
	InventoryItem *char_item = m_db.CreateCharacter(call.client->GetAccountID(), m_manager->item_factory, cdata, capp);
	if(char_item == NULL) {
		//a return to the client of 0 seems to be the only means of marking failure
		codelog(CLIENT__ERROR, "Failed to create character '%s'", cdata.name.c_str());
		return(NULL);
	}
	cdata.charid = char_item->itemID();

	//spawn all the skills
	InventoryItem *i;
	std::map<uint32, uint32>::iterator cur, end;
	cur = startingSkills.begin();
	end = startingSkills.end();
	for(; cur != end; cur++) {
		i = char_item->SpawnSingleton(cur->first, cdata.charid, flagSkill);
		if(i == NULL) {
			_log(CLIENT__ERROR, "Failed to add skill %lu to char %s (%lu) during char create.", cur->first, cdata.name.c_str(), cdata.charid);
		} else {
			i->Set_skillLevel(cur->second);
			i->Set_skillPoints(GetSkillPointsForSkillLevel(i, cur->second));
			_log(CLIENT__MESSAGE, "Training skill %lu to level %d (%d points)", i->typeID(), i->skillLevel(), i->skillPoints());
			//we dont actually need the item anymore...
			i->Release();
		}
	}

	//now set up some initial inventory:
	{
		//create them a nice capsule
		std::string capsule_name = cdata.name + "'s Capsule";
		InventoryItem *capsule = m_manager->item_factory->SpawnSingleton(
			itemTypeCapsule,
			cdata.charid,
			cdata.stationID,
			flagHangar,
			capsule_name.c_str());
		if(capsule == NULL) {
			codelog(CLIENT__ERROR, "Failed to create capsule for character '%s'", cdata.name.c_str());
			//what to do?
			return(NULL);
		}
		capsule->Relocate(GPoint(x,y,z));
	
		//put the player in their capsule
		char_item->MoveInto(capsule, flagPilot, false);
		capsule->Release();	//no longer needed.
	}
	
	{	//item scope
		InventoryItem *junk;
		junk = m_manager->item_factory->Spawn(
			2046,	//Damage Control I
			1,
			cdata.charid, cdata.stationID, flagHangar);
		if(junk == NULL)
			codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", cdata.name.c_str());
		else
			junk->Release();
		
		junk = m_manager->item_factory->Spawn(
			34,		//Tritanium
			1,
			cdata.charid,
			cdata.stationID,
			flagHangar);
		if(junk == NULL)
			codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", cdata.name.c_str());
		else
			junk->Release();
	}

	{	//item scope
		std::string ship_name = cdata.name + "'s Ship";
		InventoryItem *ship_item;
		ship_item = m_manager->item_factory->SpawnSingleton(
			shipTypeID,		// The race-specific start ship
			cdata.charid,
			cdata.stationID,
			flagHangar,
			ship_name.c_str());
		if(ship_item == NULL) {
			codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", cdata.name.c_str());
		} else {
			ship_item->Relocate(GPoint(x,y,z));

			ship_item->Release();
			ship_item = NULL;
		}
	}
	
	//recursively save everything we just did.
	char_item->Save(true);
	char_item->Release();

	_log(CLIENT__MESSAGE, "Sending char create ID %d as reply", cdata.charid);

	return(new PyRepInteger(cdata.charid));
}

PyResult CharacterService::Handle_Ping(PyCallArgs &call) {
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
		return(NULL);
	}

	//TODO: make sure this person actually owns this char...
	
	_log(CLIENT__MESSAGE, "Timed delete of char %lu unimplemented. Deleting Immediately.", args.arg);
	InventoryItem *char_item = m_manager->item_factory->Load(args.arg, true);
	if(char_item == NULL) {
		codelog(CLIENT__ERROR, "Failed to load char item %lu.", args.arg);
		return(NULL);
	}
	//does the recursive delete of all contained items
	char_item->Delete();

	//now, clean up all items which werent deleted
	std::set<uint32> items;
	if(!m_db.GetCharItems(args.arg, items)) {
		codelog(CLIENT__ERROR, "Unable to get items of char %lu.", args.arg);
		return(NULL);
	}

	InventoryItem *i;
	std::set<uint32>::iterator cur, end;
	cur = items.begin();
	end = items.end();
	for(; cur != end; cur++) {
		i = m_manager->item_factory->Load(*cur, true);
		if(i == NULL)
			codelog(CLIENT__ERROR, "Failed to load item %lu to delete. Skipping.", *cur);
		else
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
		return(NULL);
	}
	
	_log(CLIENT__ERROR, "Cancel delete (of char %lu) unimplemented.", args.arg);

	//returns nothing.
	return(NULL);
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
	
	result = new PyRepInteger(call.client->GetChar().stationID);

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

	return(rs.Encode());
}




/////////////////////////
PyResult CharacterService::Handle_GetCharacterDescription(PyCallArgs &call) {
	PyRep *result = NULL;
	Call_SingleIntegerArg args;
	
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return(NULL);
	}
		
	result = m_db.GetCharDesc(args.arg);
	if(result == NULL) {
		_log(CLIENT__ERROR, "Failed to get character description %d", args.arg);
		return(NULL);
	}
		
	return(result);
}



//////////////////////////////////
PyResult CharacterService::Handle_SetCharacterDescription(PyCallArgs &call) {
	Call_SingleStringArg args;
	
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return(NULL);
	}
	
	m_db.SetCharDesc(call.client->GetCharacterID(), args.arg.c_str());

	return(NULL);
}

PyResult CharacterService::Handle_GetCharacterAppearanceList(PyCallArgs &call) {
	Call_SingleIntList args;
	
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: Invalid arguments", call.client->GetName());
		return(NULL);
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
		return(NULL);
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
		return(NULL);
	}

	if (!m_db.SetNote(call.client->GetCharacterID(), args.itemID, args.note.c_str()))
		codelog(CLIENT__ERROR, "Error while storing the note on the database.");
	
	return(NULL);
}

/** Calculates skill points for current level.
 *
 *  Johnsus: 5/11/2008
 */
uint32 CharacterService::GetSkillPointsForSkillLevel(InventoryItem *i, uint8 level) {
	return(SkillBasePoints * i->skillTimeConstant() * pow(32, (level - 1) / 2.0));
}
