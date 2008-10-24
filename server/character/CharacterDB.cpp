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

CharacterDB::CharacterDB(DBcore *db)
: ServiceDB(db)
{
}

CharacterDB::~CharacterDB() {
}

PyRepObject *CharacterDB::GetCharacterList(uint32 accountID) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT characterID,characterName,0 as deletePrepareDateTime, gender,"
		" accessoryID,beardID,costumeID,decoID,eyebrowsID,eyesID,hairID,"
		" lipstickID,makeupID,skinID,backgroundID,lightID,"
		" headRotation1,headRotation2,headRotation3,eyeRotation1,"
		" eyeRotation2,eyeRotation3,camPos1,camPos2,camPos3,"
		" morph1e,morph1n,morph1s,morph1w,morph2e,morph2n,"
		" morph2s,morph2w,morph3e,morph3n,morph3s,morph3w,"
		" morph4e,morph4n,morph4s,morph4w"
		" FROM character_ "
		" WHERE accountID=%lu", accountID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

bool CharacterDB::ValidateCharName(const char *name) {

	if(!m_db->IsSafeString(name)) {
		_log(SERVICE__ERROR, "Name '%s' contains invalid characters.", name);
		return(false);
	}
	
	//TODO: should reserve the name, but I dont wanna insert a fake char in order to do it.
	
	DBQueryResult res;
	if(!m_db->RunQuery(res, 
		"SELECT characterID FROM character_ WHERE characterName='%s'",
		name))
	{
		codelog(SERVICE__ERROR, "Error in query for '%s': %s", name, res.error.c_str());
		return(false);
	}

	DBResultRow row;
	if(res.GetRow(row)) {
	   _log(SERVICE__MESSAGE, "Name '%s' is already taken", name);
	   return(false); 
	}

	return(true);
}

PyRepObject *CharacterDB::GetCharSelectInfo(uint32 characterID) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" characterName AS shortName,bloodlineID,gender,bounty,character_.corporationID,allianceID,title,startDateTime,createDateTime,"
		" securityRating,character_.balance,character_.stationID,solarSystemID,constellationID,regionID,"
		" petitionMessage,logonMinutes,tickerName"
		" FROM character_ "
		"	LEFT JOIN corporation ON character_.corporationID=corporation.corporationID"
		" WHERE characterID=%lu", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

#define _VoN(v, capp) \
	(capp.IsNull_##v() ? "NULL" : _ToStr(capp.Get_##v()).c_str())

static std::string _ToStr(uint32 v) {
	char buf[32];
	snprintf(buf, 32, "%u", v);
	return(buf);
}

static std::string _ToStr(double v) {
	char buf[32];
	snprintf(buf, 32, "%.13f", v);
	return(buf);
}

InventoryItem *CharacterDB::CreateCharacter(uint32 acct, ItemFactory *fact, const CharacterData &data, CharacterAppearance &app) {
	DBerror err;

	uint32 locationID = 60004420;

	//look up their typeID based on their bloodline...
	DBQueryResult res;
	if(!m_db->RunQuery(res, 
		"SELECT typeID FROM bloodlineTypes WHERE bloodlineID='%lu'",
		data.bloodlineID))
	{
		codelog(SERVICE__ERROR, "Error in type ID query for bloodline %d: %s", data.bloodlineID, res.error.c_str());
		return(NULL);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
	   _log(SERVICE__MESSAGE, "Unable to find typeID in bloodlineTypes for %lu", data.bloodlineID);
	   return(NULL); 
	}

	uint32 typeID;
	typeID = row.GetUInt(0);

	//do the insert into the entity table to get our char ID.
	InventoryItem *char_item = fact->SpawnSingleton(typeID, 1, locationID, flagPilot, data.name.c_str());
	if(char_item == NULL) {
		codelog(SERVICE__ERROR, "Failed to create character entity!");
		return(NULL);
	}

	//set some attributes to char_item
	//use the Set_persist as they are not persistent by default
	char_item->Set_intelligence_persist(data.Intelligence);
	char_item->Set_charisma_persist(data.Charisma);
	char_item->Set_perception_persist(data.Perception);
	char_item->Set_memory_persist(data.Memory);
	char_item->Set_willpower_persist(data.Willpower);

	std::string nameEsc;
	m_db->DoEscapeString(nameEsc, data.name);
	std::string titleEsc;
	m_db->DoEscapeString(titleEsc, data.title);
	std::string descEsc;
	m_db->DoEscapeString(descEsc, data.description);

	if(!m_db->RunQuery(err,
	"INSERT INTO character_ ("
	"	characterID,characterName,accountID,title,description,typeID,"
	"	createDateTime,startDateTime,"
	"	bounty,balance,securityRating,petitionMessage,logonMinutes,"
	"	corporationID,corporationDateTime,"
	"	stationID,solarSystemID,constellationID,regionID,"
	"	raceID,bloodlineID,ancestryID,careerID,schoolID,careerSpecialityID,gender,"
	"	accessoryID,beardID,costumeID,decoID,eyebrowsID,eyesID,"
	"	hairID,lipstickID,makeupID,skinID,backgroundID,lightID,"
	"	headRotation1,headRotation2,headRotation3,"
	"	eyeRotation1,eyeRotation2,eyeRotation3,"
	"	camPos1,camPos2,camPos3,"
	"	morph1e,morph1n,morph1s,morph1w,"
	"	morph2e,morph2n,morph2s,morph2w,"
	"	morph3e,morph3n,morph3s,morph3w,"
	"	morph4e,morph4n,morph4s,morph4w"
	" ) "
	"VALUES(%lu,'%s',%d,'%s','%s',%lu,"
	"	%llu,%llu,"
	"	%.13f, %.13f, %.13f, '', %lu,"
	"	%ld,%lld,"		//corp
	"	%ld,%ld,%ld,%ld,"	//loc
	"	%lu,%lu,%lu,%lu,%lu,%lu,%lu,"
	"	%s,%s,%ld,%s,%ld,%ld,"
	"	%ld,%s,%s,%ld,%ld,%ld,"
	"	%.13f,%.13f,%.13f,"	//head
	"	%.13f,%.13f,%.13f,"	//eye
	"	%.13f,%.13f,%.13f,"	//cam
	"	%s,%s,%s,%s,"
	"	%s,%s,%s,%s,"
	"	%s,%s,%s,%s,"
	"	%s,%s,%s,%s"
	"	)",
		char_item->itemID(), nameEsc.c_str(), acct, titleEsc.c_str(), descEsc.c_str(), data.typeID,
		data.createDateTime, data.startDateTime,
		data.bounty,data.balance,data.securityRating,data.logonMinutes,
		data.corporationID, data.corporationDateTime,
		data.stationID, data.solarSystemID, data.constellationID, data.regionID,
		data.raceID, data.bloodlineID, data.ancestryID, data.careerID, data.schoolID, data.careerSpecialityID, data.genderID,
		_VoN(accessoryID,app),_VoN(beardID,app),app.costumeID,_VoN(decoID,app),app.eyebrowsID,app.eyesID,
		app.hairID,_VoN(lipstickID,app),_VoN(makeupID,app),app.skinID,app.backgroundID,app.lightID,
		app.headRotation1, app.headRotation2, app.headRotation3, 
		app.eyeRotation1, app.eyeRotation2, app.eyeRotation3,
		app.camPos1, app.camPos2, app.camPos3,
		_VoN(morph1e,app), _VoN(morph1n,app), _VoN(morph1s,app), _VoN(morph1w,app),
		_VoN(morph2e,app), _VoN(morph2n,app), _VoN(morph2s,app), _VoN(morph2w,app),
		_VoN(morph3e,app), _VoN(morph3n,app), _VoN(morph3s,app), _VoN(morph3w,app),
		_VoN(morph4e,app), _VoN(morph4n,app), _VoN(morph4s,app), _VoN(morph4w,app)
	)) {
		codelog(SERVICE__ERROR, "Failed to inser new char: %s", err.c_str());
		char_item->Delete();
		return(NULL);
	}

	// Hack in the first employment record
	// TODO: Eventually, this should go under corp stuff...
	if (!m_db->RunQuery(err, 
		" INSERT INTO chrEmployment "
		" VALUES(%lu, %lu, %llu, 0)", char_item->itemID(), data.corporationID, Win32TimeNow()
		))
	{
		codelog(SERVICE__ERROR, "Failed to set new char employment record: %s", err.c_str());
		//just let it go... its a lot easier this way
	}

	// And one more member to the corporation
	if (!m_db->RunQuery(err,
		" UPDATE "
		" corporation "
		" SET memberCount = memberCount + 1 "
		" WHERE corporationID = %lu ", data.corporationID))
	{
		codelog(SERVICE__ERROR, "Couldn't raise corporation's member count for some reason: %s", err.c_str());
		//just let it go... its a lot easier this way
	}

	return(char_item);
}
/*
uint32 CharacterDB::GetRaceFromBloodline(uint32 bloodline) {
	DBQueryResult res;
	if(!m_db->RunQuery(res, 
		"SELECT raceID FROM character_ WHERE bloodlineID=%lu",
		bloodline))
	{
		_log(SERVICE__ERROR, "Error in GetRaceFromBloodline query: %s", res.error.c_str());
		return(false);
	}

	DBResultRow row;
	if(res.GetRow(row)) {
	   _log(SERVICE__MESSAGE, "Name '%s' is already taken", name);
	   return(false); 
	}

	return(true);
}*/

PyRepObject *CharacterDB::GetCharPublicInfo(uint32 characterID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT "
		" character_.typeID,"
		" character_.corporationID,"
        /*" chrBloodlines.raceID,"*/
		" character_.raceID,"
		" character_.bloodlineID,"
		" character_.ancestryID,"
		" character_.careerID,"
		" character_.schoolID,"
		" character_.careerSpecialityID,"
		" character_.characterName,"
		" 0 as age,"	//hack
		" character_.createDateTime,"
		" character_.gender,"
		" character_.characterID,"
		" character_.description,"
		" character_.corporationDateTime"
        " FROM character_ "
        /*"		LEFT JOIN chrBloodlines "
        "		ON character_.bloodlineID=chrBloodlines.bloodlineID"*/
		" WHERE characterID=%lu", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Error in GetCharPublicInfo query: no data for char %d", characterID);
		return(NULL);
	}
	return(DBRowToKeyVal(row));
	
}

PyRepObject *CharacterDB::GetCharPublicInfo3(uint32 characterID) {

	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" character_.bounty,"
		" character_.title,"
		" character_.startDateTime,"
		" character_.description,"
		" character_.corporationID"
		" FROM character_"
		" WHERE characterID=%lu", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

bool CharacterDB::LoadCharacterAppearance(uint32 characterID, CharacterAppearance &into) {
	
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
	"SELECT "
	"	accessoryID,beardID,costumeID,decoID,eyebrowsID,eyesID,"
	"	hairID,lipstickID,makeupID,skinID,backgroundID,lightID,"
	"	headRotation1,headRotation2,headRotation3,"
	"	eyeRotation1,eyeRotation2,eyeRotation3,"
	"	camPos1,camPos2,camPos3,"
	"	morph1e,morph1n,morph1s,morph1w,"
	"	morph2e,morph2n,morph2s,morph2w,"
	"	morph3e,morph3n,morph3s,morph3w,"
	"	morph4e,morph4n,morph4s,morph4w"
	" FROM character_"
	" WHERE characterID=%lu", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Error in LoadCharacterAppearance query: no data for char %d", characterID);
		return(false);
	}

	uint32 i = 0;
	row.IsNull(i)?into.Clear_accessoryID():into.Set_accessoryID(row.GetUInt(i)); i++;
	row.IsNull(i)?into.Clear_beardID():into.Set_beardID(row.GetUInt(i)); i++;
	into.costumeID = row.GetUInt(i); i++;
	row.IsNull(i)?into.Clear_decoID():into.Set_decoID(row.GetUInt(i)); i++;
	into.eyebrowsID = row.GetUInt(i); i++;
	into.eyesID = row.GetUInt(i); i++;
	into.hairID = row.GetUInt(i); i++;
	row.IsNull(i)?into.Clear_lipstickID():into.Set_lipstickID(row.GetUInt(i)); i++;
	row.IsNull(i)?into.Clear_makeupID():into.Set_makeupID(row.GetUInt(i)); i++;
	into.skinID = row.GetUInt(i); i++;
	into.backgroundID = row.GetUInt(i); i++;
	into.lightID = row.GetUInt(i); i++;

	//none of these may be NULL
	into.headRotation1 = row.GetDouble(i); i++;
	into.headRotation2 = row.GetDouble(i); i++;
	into.headRotation3 = row.GetDouble(i); i++;
	into.eyeRotation1 = row.GetDouble(i); i++;
	into.eyeRotation2 = row.GetDouble(i); i++;
	into.eyeRotation3 = row.GetDouble(i); i++;
	into.camPos1 = row.GetDouble(i); i++;
	into.camPos2 = row.GetDouble(i); i++;
	into.camPos3 = row.GetDouble(i); i++;

	//all of these may be NULL
	row.IsNull(i)?into.Clear_morph1e():into.Set_morph1e(row.GetDouble(i)); i++;
	row.IsNull(i)?into.Clear_morph1n():into.Set_morph1n(row.GetDouble(i)); i++;
	row.IsNull(i)?into.Clear_morph1s():into.Set_morph1s(row.GetDouble(i)); i++;
	row.IsNull(i)?into.Clear_morph1w():into.Set_morph1w(row.GetDouble(i)); i++;
	row.IsNull(i)?into.Clear_morph2e():into.Set_morph2e(row.GetDouble(i)); i++;
	row.IsNull(i)?into.Clear_morph2n():into.Set_morph2n(row.GetDouble(i)); i++;
	row.IsNull(i)?into.Clear_morph2s():into.Set_morph2s(row.GetDouble(i)); i++;
	row.IsNull(i)?into.Clear_morph2w():into.Set_morph2w(row.GetDouble(i)); i++;
	row.IsNull(i)?into.Clear_morph3e():into.Set_morph3e(row.GetDouble(i)); i++;
	row.IsNull(i)?into.Clear_morph3n():into.Set_morph3n(row.GetDouble(i)); i++;
	row.IsNull(i)?into.Clear_morph3s():into.Set_morph3s(row.GetDouble(i)); i++;
	row.IsNull(i)?into.Clear_morph3w():into.Set_morph3w(row.GetDouble(i)); i++;
	row.IsNull(i)?into.Clear_morph4e():into.Set_morph4e(row.GetDouble(i)); i++;
	row.IsNull(i)?into.Clear_morph4n():into.Set_morph4n(row.GetDouble(i)); i++;
	row.IsNull(i)?into.Clear_morph4s():into.Set_morph4s(row.GetDouble(i)); i++;
	row.IsNull(i)?into.Clear_morph4w():into.Set_morph4w(row.GetDouble(i)); i++;
	
	return(true);
}

/////////////////////////////////
PyRepObject *CharacterDB::GetCharDesc(uint32 characterID) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" description "
		" FROM character_ "
		" WHERE characterID=%lu", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	


	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Error in query: no data for char %d", characterID);
		return(NULL);
	}

	return(DBRowToRow(row));
	
}

//////////////////////////////////
bool CharacterDB::SetCharDesc(uint32 characterID, const char *str) {
	DBerror err;	
	std::string stringEsc;

	m_db->DoEscapeString(stringEsc, str);

	if(!m_db->RunQuery(err,
		"UPDATE character_"
		" SET description ='%s'"
		" WHERE characterID=%lu", stringEsc.c_str(),characterID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		return(false);
	}

	return (true);
}

//just return all itemIDs which has ownerID set to characterID
bool CharacterDB::GetCharItems(uint32 characterID, std::set<uint32> &into) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT itemID"
		" FROM entity"
		" WHERE ownerID = %lu",
		characterID))
	{
		_log(DATABASE__ERROR, "Failed to query items of char %lu: %s.", characterID, res.error.c_str());
		return(false);
	}

	DBResultRow row;
	while(res.GetRow(row))
		into.insert(row.GetUInt(0));
	
	return(true);
}

//Try to run through all of the tables which might have data relavent to
//this person in it, and run a delete against it.
bool CharacterDB::DeleteCharacter(uint32 characterID) {
	DBerror err;
	DBQueryResult res;
	DBResultRow row;
	
	//we delete mail sent FROM this player too, otherwise intersting things will happen when it is read again.
	if (!m_db->RunQuery(res,
		"SELECT messageID FROM eveMail WHERE channelID=%lu OR senderID=%lu", characterID, characterID))
	{
		codelog(SERVICE__ERROR, "Error in mail query (not stopping): %s", res.error.c_str());
	} else {
		bool found = false;	//simple optimization
		while (res.GetRow(row)) {
			found = true;
			if (!m_db->RunQuery(err, 
				"DELETE FROM eveMailDetails WHERE messageID=%lu", row.GetUInt(0)))
			{
				codelog(SERVICE__ERROR, "Error in query (not stopping): %s", err.c_str());
			}
		}
		if(found) {
			if(!m_db->RunQuery(err,
				"DELETE FROM eveMail WHERE channelID=%lu OR senderID=%lu", characterID, characterID))
			{
				codelog(SERVICE__ERROR, "Error in query (not stopping): %s", err.c_str());
			}
		}
	}
	
	if(!m_db->RunQuery(err,
		"DELETE FROM crpCharShares WHERE characterID=%lu", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query (not stopping): %s", err.c_str());
	}
	
	if(!m_db->RunQuery(err,
		"DELETE FROM bookmarks WHERE ownerID=%lu", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query (not stopping): %s", err.c_str());
	}
	
	if(!m_db->RunQuery(err,
		"DELETE FROM market_journal WHERE characterID=%lu", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query (not stopping): %s", err.c_str());
	}
	
	if(!m_db->RunQuery(err,
		"DELETE FROM market_orders WHERE charID=%lu", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query (not stopping): %s", err.c_str());
	}
	
	if(!m_db->RunQuery(err,
		"DELETE FROM market_transactions WHERE clientID=%lu", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query (not stopping): %s", err.c_str());
	}
	
	if(!m_db->RunQuery(err,
		"DELETE FROM chrStandings WHERE characterID=%lu", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query (not stopping): %s", err.c_str());
	}
	
	if(!m_db->RunQuery(err,
		"DELETE FROM chrNPCStandings WHERE characterID=%lu", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query (not stopping): %s", err.c_str());
	}

	if(!m_db->RunQuery(err,
		"DELETE FROM chrEmployment WHERE characterID=%lu", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query (not stopping): %s", err.c_str());
	}

	if(!m_db->RunQuery(err,
		"DELETE FROM character_ WHERE characterID=%lu", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		return(false);
	}

	return (true);
	
}

PyRepObject *CharacterDB::GetCharacterAppearance(uint32 charID) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" accessoryID,beardID,costumeID,decoID,eyebrowsID,eyesID,hairID,"
		" lipstickID,makeupID,skinID,backgroundID,lightID,"
		" headRotation1,headRotation2,headRotation3,eyeRotation1,"
		" eyeRotation2,eyeRotation3,camPos1,camPos2,camPos3,"
		" morph1e,morph1n,morph1s,morph1w,morph2e,morph2n,"
		" morph2s,morph2w,morph3e,morph3n,morph3s,morph3w,"
		" morph4e,morph4n,morph4s,morph4w"
		" FROM character_ "
		" WHERE characterID=%lu", charID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}



bool CharacterDB::GetAttributesFromBloodline(CharacterData & cdata) {
	DBQueryResult res;

	if (!m_db->RunQuery(res,
		" SELECT "
		"   bloodlineTypes.typeID, chrBloodlines.intelligence, "
		"	chrBloodlines.charisma, chrBloodlines.perception, "
		"	chrBloodlines.memory, chrBloodlines.willpower "
		" FROM chrBloodlines "
		" 	LEFT JOIN bloodlineTypes ON chrBloodlines.bloodlineID = bloodlineTypes.bloodlineID "
		" WHERE chrBloodlines.bloodlineID = %lu ", cdata.bloodlineID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return (false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Failed to find bloodline information for bloodline %lu", cdata.bloodlineID);
		return(false);
	}

	cdata.typeID = row.GetUInt(0);

	cdata.ModifyAttributes(
		row.GetUInt(1),
		row.GetUInt(2),
		row.GetUInt(3),
		row.GetUInt(4),
		row.GetUInt(5));
	
	return (true);
}


bool CharacterDB::GetAttributesFromAncestry(CharacterData & cdata) {
	DBQueryResult res;

	if (!m_db->RunQuery(res,
		" SELECT "
		"        intelligence, charisma, perception, memory, willpower "
		" FROM chrAncestries "
		" WHERE ancestryID = %lu ", cdata.ancestryID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return (false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Failed to find ancestry information for ancestry %lu", cdata.ancestryID);
		return(false);
	}
	
	cdata.ModifyAttributes(
		row.GetUInt(0),
		row.GetUInt(1),
		row.GetUInt(2),
		row.GetUInt(3),
		row.GetUInt(4));

	return (true);
}


/**
  * @todo Here should come a call to Corp??::CharacterJoinToCorp or what the heck... for now we only put it there
  */
bool CharacterDB::GetLocationCorporationByCareer(CharacterData & cdata, double & x, double & y, double & z) {
	// Getting corporation id from school's info
	DBQueryResult res;
	if (!m_db->RunQuery(res, 
	 "SELECT "
	 "	chrSchools.corporationID, "
	 "  chrSchools.schoolID,"
	 "  corporation.allianceID, "
	 "	corporation.stationID, "
	 "	staStations.solarSystemID, "
	 "	staStations.constellationID, "
	 "	staStations.regionID, "
	 "	staStations.x, "
	 "	staStations.y, "
	 "	staStations.z"
	 " FROM staStations"
	 "  LEFT JOIN corporation"
	 "		ON corporation.stationID=staStations.stationID"
	 "	LEFT JOIN chrSchools"
	 "		ON corporation.corporationID=chrSchools.corporationID"
	 "	LEFT JOIN chrCareers"
	 "		ON chrSchools.careerID=chrCareers.careerID"
	 " WHERE chrCareers.careerID = %lu", cdata.careerID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return (false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Failed to find career %lu", cdata.careerID);
		return(false);
	}
	
	cdata.corporationID = row.GetUInt(0);
	cdata.schoolID = row.GetUInt(1);
	cdata.allianceID = row.GetUInt(2);
	cdata.stationID = row.GetUInt(3);
	cdata.solarSystemID = row.GetUInt(4);
	cdata.constellationID = row.GetUInt(5);
	cdata.regionID = row.GetUInt(6);
	
	x = row.GetDouble(7);
	y = row.GetDouble(8);
	z = row.GetDouble(9);
	
	return (true);
}

bool CharacterDB::GetShipTypeByBloodline(uint32 bloodlineID, uint32 &shipTypeID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT shipTypeID"
		" FROM chrBloodlines"
		" WHERE bloodlineID = %lu",
		bloodlineID))
	{
		_log(DATABASE__ERROR, "Failed to query ship type for bloodline %lu: %s.", bloodlineID, res.error.c_str());
		return(false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Bloodline %lu not found.", bloodlineID);
		return(false);
	}

	shipTypeID = row.GetUInt(0);

	return(true);
}


bool CharacterDB::GetSkillsByRace(uint32 raceID, std::map<uint32, uint32> &into) {
	DBQueryResult res;

	if (!m_db->RunQuery(res,
		"SELECT "
		"        skillTypeID, levels"
		" FROM chrRaceSkills "
		" WHERE raceID = %lu ", raceID))
	{
		_log(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}

	DBResultRow row;
	while(res.GetRow(row)) {
		if(into.find(row.GetUInt(0)) == into.end())
			into[row.GetUInt(0)] = row.GetUInt(1);
		else
			into[row.GetUInt(0)] += row.GetUInt(1);
		//check to avoid more than 5 levels by skill
		if(into[row.GetUInt(0)] > 5)
			into[row.GetUInt(0)] = 5;
	}

	return true;
}

bool CharacterDB::GetSkillsByCareer(uint32 careerID, std::map<uint32, uint32> &into) {
	DBQueryResult res;

	if (!m_db->RunQuery(res,
		"SELECT "
		"        skillTypeID, levels"
		" FROM chrCareerSkills"
		" WHERE careerID = %lu", careerID))
	{
		_log(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}
	
	DBResultRow row;
	while(res.GetRow(row)) {
		if(into.find(row.GetUInt(0)) == into.end())
			into[row.GetUInt(0)] = row.GetUInt(1);
		else
			into[row.GetUInt(0)] += row.GetUInt(1);
		//check to avoid more than 5 levels by skill
		if(into[row.GetUInt(0)] > 5)
			into[row.GetUInt(0)] = 5;
	}

	return true;
}

bool CharacterDB::GetSkillsByCareerSpeciality(uint32 careerSpecialityID, std::map<uint32, uint32> &into) {
	DBQueryResult res;

	if (!m_db->RunQuery(res,
		"SELECT "
		"        skillTypeID, levels"
		" FROM chrCareerSpecialitySkills"
		" WHERE specialityID = %lu", careerSpecialityID))
	{
		_log(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}
	
	DBResultRow row;
	while(res.GetRow(row)) {
		if(into.find(row.GetUInt(0)) == into.end())
			into[row.GetUInt(0)] = row.GetUInt(1);
		else
			into[row.GetUInt(0)] += row.GetUInt(1);
		//check to avoid more than 5 levels by skill
		if(into[row.GetUInt(0)] > 5)
			into[row.GetUInt(0)] = 5;
	}

	return true;
}

uint8 CharacterDB::GetRaceByBloodline(uint32 bloodlineID) {
	DBQueryResult res;

	if (!m_db->RunQuery(res,
			"SELECT `raceID`"
			"	FROM `chrBloodlines`"
			"	WHERE bloodlineID = %lu",
			bloodlineID)
		)
	{
		codelog(SERVICE__ERROR, "Error on query: %s", res.error.c_str());
		return(0);
	}
	DBResultRow row;
	
	if(!res.GetRow(row))
		return(0);

	return(row.GetUInt(0));
}

/**
 * Retrieves the character note from the database as a PyRepString pointer.
 *
 * **LSMoura
 */
PyRepString *CharacterDB::GetNote(uint32 ownerID, uint32 itemID) {
	DBQueryResult res;

	if (!m_db->RunQuery(res,
			"SELECT `note` FROM `chrNotes` WHERE ownerID = %ld AND itemID = %ld",
			ownerID, itemID)
		)
	{
		codelog(SERVICE__ERROR, "Error on query: %s", res.error.c_str());
		return(NULL);
	}
	DBResultRow row;
	if(!res.GetRow(row))
		return(NULL);

	return(new PyRepString(row.GetText(0)));
}



/**
 * Stores the character note on the database, given the ownerID and itemID and the string itself.
 *
 * If the String is null or size zero, the entry is removed from the database.
 *
 * @return boolean true if success.
 *
 * **LSMoura
 */
bool CharacterDB::SetNote(uint32 ownerID, uint32 itemID, const char *str) {
	DBerror err;
	std::string escaped;

	if (str[0] == '\0') {
		if (!m_db->RunQuery(err,
			"DELETE FROM `chrNotes` "
			" WHERE itemID = %ld AND ownerID = %ld LIMIT 1",
			ownerID, itemID)
			)
		{
			codelog(CLIENT__ERROR, "Error on query: %s", err.c_str());
			return(false);
		}
	}
	else {
		m_db->DoEscapeString(escaped, str);
		if (!m_db->RunQuery(err,
			"REPLACE INTO `chrNotes` (itemID, ownerID, note)	"
			"VALUES (%lu, %lu, '%s')",
			ownerID, itemID, escaped.c_str())
			)
		{
			codelog(CLIENT__ERROR, "Error on query: %s", err.c_str());
			return(false);
		}
	}

	return(true);
}

uint32 CharacterDB::AddOwnerNote(uint32 charID, const std::string & label, const std::string & content) {
	DBerror err;
	uint32 id;
	std::string lblS;
	std::string contS;
	m_db->DoEscapeString(lblS, label);
	m_db->DoEscapeString(contS, content);
	if (!m_db->RunQueryLID(err, id, 
		"INSERT INTO chrOwnerNote (ownerID, label, note) VALUES (%lu, '%s', '%s');",
		charID, lblS.c_str(), contS.c_str()))
	{
		codelog(SERVICE__ERROR, "Error on query: %s", err.c_str());
		return 0;
	}

	return id;
}

bool CharacterDB::EditOwnerNote(uint32 charID, uint32 noteID, const std::string & label, const std::string & content) {
	DBerror err;

	std::string contS;
	m_db->DoEscapeString(contS, content);

	if (!m_db->RunQuery(err,
		"UPDATE chrOwnerNote SET note = '%s' WHERE ownerID = %lu AND noteID = %lu;",
		contS.c_str(), charID, noteID))
	{
		codelog(SERVICE__ERROR, "Error on query: %s", err.c_str());
		return false;
	}

	return true;
}

PyRepObject *CharacterDB::GetOwnerNoteLabels(uint32 charID) {
	DBQueryResult res;

	if (!m_db->RunQuery(res, "SELECT noteID, label FROM chrOwnerNote WHERE ownerID = %lu", charID))
	{
		codelog(SERVICE__ERROR, "Error on query: %s", res.error.c_str());
		return (NULL);
	}

	return DBResultToRowset(res);
}

PyRepObject *CharacterDB::GetOwnerNote(uint32 charID, uint32 noteID) {
	DBQueryResult res;

	if (!m_db->RunQuery(res, "SELECT note FROM chrOwnerNote WHERE ownerID = %lu AND noteID = %lu", charID, noteID))
	{
		codelog(SERVICE__ERROR, "Error on query: %s", res.error.c_str());
		return (NULL);
	}

	return DBResultToRowset(res);
}








