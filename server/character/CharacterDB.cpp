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



#include "CharacterDB.h"
#include "../common/dbcore.h"
#include "../common/logsys.h"
#include "../common/EVEDBUtils.h"
#include "../common/PyRep.h"
#include "../Client.h"
#include "../common/EVEUtils.h"
#include "../inventory/InventoryItem.h"
#include "../inventory/ItemFactory.h"

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
		" bloodlineID,gender,bounty,character_.corporationID,title,startDateTime,"
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

//if the value is not 999, return it, otherwise, return NULL
static std::string _FoN(double v) {
	if(v == 999)
		return("NULL");
	char buf[32];
	snprintf(buf, 32, "%.13f", v);
	return(buf);
}
static std::string _IoN(uint32 v) {
	if(v == 999999)
		return("NULL");
	char buf[32];
	snprintf(buf, 32, "%lu", v);
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
	"	bloodlineID,gender,"
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
	"	%lu,%lu,"
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
		data.bloodlineID, data.genderID,
		_IoN(app.accessoryID).c_str(),_IoN(app.beardID).c_str(),app.costumeID,_IoN(app.decoID).c_str(),app.eyebrowsID,app.eyesID,
		app.hairID,_IoN(app.lipstickID).c_str(),_IoN(app.makeupID).c_str(),app.skinID,app.backgroundID,app.lightID,
		app.headRotation1, app.headRotation2, app.headRotation3, 
		app.eyeRotation1, app.eyeRotation2, app.eyeRotation3,
		app.camPos1, app.camPos2, app.camPos3,
		_FoN(app.morph1e).c_str(), _FoN(app.morph1n).c_str(), _FoN(app.morph1s).c_str(), _FoN(app.morph1w).c_str(),
		_FoN(app.morph2e).c_str(), _FoN(app.morph2n).c_str(), _FoN(app.morph2s).c_str(), _FoN(app.morph2w).c_str(),
		_FoN(app.morph3e).c_str(), _FoN(app.morph3n).c_str(), _FoN(app.morph3s).c_str(), _FoN(app.morph3w).c_str(),
		_FoN(app.morph4e).c_str(), _FoN(app.morph4n).c_str(), _FoN(app.morph4s).c_str(), _FoN(app.morph4w).c_str()
	)) {
		codelog(SERVICE__ERROR, "Failed to inser new char: %s", err.c_str());
		char_item->Delete();
		return(NULL);
	}
	
	

#ifndef WIN32
#warning hacking a channel in for new chars:
#endif
	m_db->RunQuery(err, "INSERT INTO channels VALUES(%lu, %lu, NULL, NULL, NULL, 1, NULL, 1, 100, 0, 15, 1, 0)", char_item->itemID(), char_item->itemID());

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
	if (!m_db->RunQuery(
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

bool CharacterDB::LoadCharacter(uint32 characterID, CharacterData &into) {
	
	DBQueryResult res;
	
	//we really should derive the char's location from the entity table...
	if(!m_db->RunQuery(res,
	"SELECT "
	"	character_.characterName,character_.title,character_.description,character_.typeID,"
	"	character_.createDateTime,character_.startDateTime,"
	"	character_.bounty,character_.balance,character_.securityRating,character_.logonMinutes,"
	"	character_.corporationID,character_.corporationDateTime,corporation.allianceID,"
	"	character_.stationID,character_.solarSystemID,character_.constellationID,character_.regionID,"
	"	character_.bloodlineID,character_.gender,character_.raceID"
	" FROM character_"
	" LEFT JOIN corporation ON corporation.corporationID = character_.corporationID"
	" WHERE characterID=%lu", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Error in LoadCharacter query: no data for char %d", characterID);
		return(false);
	}

	uint32 i = 0;
	into.name = row.GetText(i++);
	into.title = row.GetText(i++);
	into.description = row.GetText(i++);
	into.typeID = row.GetUInt(i++);
	into.createDateTime = row.GetUInt64(i++);
	into.startDateTime = row.GetUInt64(i++);
	into.bounty = row.GetDouble(i++);
	into.balance = row.GetDouble(i++);
	into.securityRating = row.GetDouble(i++);
	into.logonMinutes = row.GetUInt(i++);
	into.corporationID = row.GetUInt(i++);
	into.corporationDateTime = row.GetUInt64(i++);
	into.allianceID = row.GetUInt(i++);
	into.stationID = row.GetUInt(i++);
	into.solarSystemID = row.GetUInt(i++);
	into.constellationID = row.GetUInt(i++);
	into.regionID = row.GetUInt(i++);
	into.bloodlineID = row.GetUInt(i++);
	into.genderID = row.GetUInt(i++);
	into.raceID = row.GetUInt(i++);
	
	into.charid = characterID;
	return(true);
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
	into.accessoryID = row.IsNull(i)?999999:row.GetUInt(i); i++;
	into.beardID = row.IsNull(i)?999999:row.GetUInt(i); i++;
	into.costumeID = row.IsNull(i)?999999:row.GetUInt(i); i++;
	into.decoID = row.IsNull(i)?999999:row.GetUInt(i); i++;
	into.eyebrowsID = row.IsNull(i)?999999:row.GetUInt(i); i++;
	into.eyesID = row.IsNull(i)?999999:row.GetUInt(i); i++;
	into.hairID = row.IsNull(i)?999999:row.GetUInt(i); i++;
	into.lipstickID = row.IsNull(i)?999999:row.GetUInt(i); i++;
	into.makeupID = row.IsNull(i)?999999:row.GetUInt(i); i++;
	into.skinID = row.IsNull(i)?999999:row.GetUInt(i); i++;
	into.backgroundID = row.IsNull(i)?999999:row.GetUInt(i); i++;
	into.lightID = row.IsNull(i)?999999:row.GetUInt(i); i++;
	into.headRotation1 = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.headRotation2 = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.headRotation3 = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.eyeRotation1 = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.eyeRotation2 = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.eyeRotation3 = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.camPos1 = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.camPos2 = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.camPos3 = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph1e = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph1n = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph1s = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph1w = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph2e = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph2n = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph2s = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph2w = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph3e = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph3n = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph3s = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph3w = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph4e = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph4n = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph4s = row.IsNull(i)?999:row.GetDouble(i); i++;
	into.morph4w = row.IsNull(i)?999:row.GetDouble(i); i++;
	
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

//Try to run through all of the tables which might have data relavent to
//this person in it, and run a delete against it.
bool CharacterDB::DeleteCharacter(uint32 characterID) {
	DBerror err;
	DBQueryResult res;
	DBResultRow row;
	
	//from now on, if the query fails, keep going.
	if(!m_db->RunQuery(err,
		"DELETE FROM channels WHERE channelID=%lu OR ownerID=%lu", characterID, characterID))
	{
		codelog(SERVICE__ERROR, "Error in query (not stopping): %s", err.c_str());
	}

	//we delete mail sent FROM this player too, otherwise intersting things will happen when it is read again.
	if (!m_db->RunQuery(res,
		"SELECT messageID FROM evemail WHERE channelID=%lu OR senderID=%lu", characterID, characterID))
	{
		codelog(SERVICE__ERROR, "Error in mail query (not stopping): %s", res.error.c_str());
	} else {
		bool found = false;	//simple optimization
		while (res.GetRow(row)) {
			found = true;
			if (!m_db->RunQuery(err, 
				"DELETE FROM evemailDetails WHERE messageID=%lu", row.GetUInt(0)))
			{
				codelog(SERVICE__ERROR, "Error in query (not stopping): %s", err.c_str());
			}
		}
		if(found) {
			if(!m_db->RunQuery(err,
				"DELETE FROM evemail WHERE channelID=%lu OR senderID=%lu", characterID, characterID))
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

	//TODO: we really need to do a hierarchial delete! ownerID will prolly get most of 
	// it, but its possible to still break things.
	if(!m_db->RunQuery(err,
		"DELETE FROM entity WHERE itemID=%lu OR ownerID=%lu", characterID, characterID))
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
bool CharacterDB::GetLocationCorporationFromSchool(CharacterData & cdata, double & x, double & y, double & z) {
	// Getting corporation id from school's info
	DBQueryResult res;
	if (!m_db->RunQuery(res, 
	 "SELECT "
	 "	chrSchools.corporationID, "
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
	 " WHERE schoolID = %lu ", cdata.schoolID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return (false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Failed to find school %lu", cdata.schoolID);
		return(false);
	}
	
	cdata.corporationID = row.GetUInt(0);
	cdata.stationID = row.GetUInt(1);
	cdata.solarSystemID = row.GetUInt(2);
	cdata.constellationID = row.GetUInt(3);
	cdata.regionID = row.GetUInt(4);
	
	x = row.GetDouble(5);
	y = row.GetDouble(6);
	z = row.GetDouble(7);
	
	return (true);
}

bool CharacterDB::GetSkillsFromBloodline(uint32 bloodlineID, std::vector<uint32> &into, uint32 & shipTypeID) {
	DBQueryResult res;

	if (!m_db->RunQuery(res,
		" SELECT "
		"        skillTypeID1, skillTypeID2, shipTypeID, raceID "
		" FROM chrBloodlines "
		" WHERE bloodlineID = %lu ", bloodlineID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return (false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Failed to find bloodline information for bloodline %lu", bloodlineID);
		return(false);
	}

	if (!row.IsNull(0))
		into.push_back(row.GetUInt(0));
	if (!row.IsNull(1))
		into.push_back(row.GetUInt(1));

	shipTypeID = row.GetUInt(2);
	uint32 raceID = row.GetUInt(3);

	if (!m_db->RunQuery(res,
		" SELECT "
		"        skillTypeID1 "
		" FROM chrRaces "
		" WHERE raceID = %ld ", raceID))
	{
		_log(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}

	if(!res.GetRow(row) ) {
		codelog(SERVICE__ERROR, "Failed to find race information for race %lu", raceID);
		return(false);
	}

	if (!row.IsNull(0))
		into.push_back(row.GetUInt(0));
	return (true);
}


bool CharacterDB::GetSkillsFromAncestry(uint32 ancestryID, std::vector<uint32> &into) {
	DBQueryResult res;
	DBResultRow row;

	if (!m_db->RunQuery(res,
		" SELECT "
		"        skillTypeID1, skillTypeID2 "
		" FROM chrAncestries "
		" WHERE ancestryID = %ld ", ancestryID))
	{
		_log(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}

	if(!res.GetRow(row) ) {
		codelog(SERVICE__ERROR, "Failed to find ancestry information for ancestry %lu", ancestryID);
		return(false);
	}

	if (!row.IsNull(0))
		into.push_back(row.GetUInt(0));
	if (!row.IsNull(1))
		into.push_back(row.GetUInt(1));
	
	return true;
}

bool CharacterDB::GetSkillsFromDepartment(uint32 departmentID, std::vector<uint32> &into) {
	DBQueryResult res;
	DBResultRow row;

	if (!m_db->RunQuery(res,
		" SELECT "
		"        skillTypeID1, skillTypeID2, skillTypeID3 "
		" FROM chrDepartments "
		" WHERE departmentID = %ld ", departmentID))
	{
		_log(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}
	
	if(!res.GetRow(row) ) {
		codelog(SERVICE__ERROR, "Failed to find department information for departmentID %lu", departmentID);
		return(false);
	}

	if (!row.IsNull(0))
		into.push_back(row.GetUInt(0));
	if (!row.IsNull(1))
		into.push_back(row.GetUInt(1));
	if (!row.IsNull(2))
		into.push_back(row.GetUInt(2));

	return true;
}

bool CharacterDB::GetSkillsFromField(uint32 fieldID, std::vector<uint32> &into) {
	DBQueryResult res;
	DBResultRow row;

	if (!m_db->RunQuery(res,
		" SELECT "
		"        skillTypeID1, skillTypeID2 "
		" FROM chrFields "
		" WHERE fieldID = %ld ", fieldID))
	{
		_log(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}
	
	if(!res.GetRow(row) ) {
		codelog(SERVICE__ERROR, "Failed to find field information for fieldID %lu", fieldID);
		return(false);
	}

	if (!row.IsNull(0))
		into.push_back(row.GetUInt(0));
	if (!row.IsNull(1))
		into.push_back(row.GetUInt(1));
	
	return true;
}

bool CharacterDB::GetSkillsFromSpeciality(uint32 specialityID, std::vector<uint32> &into) {
	DBQueryResult res;
	DBResultRow row;

	if (!m_db->RunQuery(res,
		" SELECT "
		"        skillTypeID1, skillTypeID2 "
		" FROM chrSpecialities "
		" WHERE specialityID = %ld ", specialityID))
	{
		_log(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}
	
	if(!res.GetRow(row) ) {
		codelog(SERVICE__ERROR, "Failed to find speciality information for specialityID %lu", specialityID);
		return(false);
	}

	if (!row.IsNull(0))
		into.push_back(row.GetUInt(0));
	if (!row.IsNull(1))
		into.push_back(row.GetUInt(1));

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








