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

CharacterDB::CharacterDB(DBcore *db) : ServiceDB(db)
{
	load_name_validation_set();
}

CharacterDB::~CharacterDB() {}

PyRepObject *CharacterDB::GetCharacterList(uint32 accountID) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT"
		" characterID,itemName AS characterName,0 as deletePrepareDateTime,"
		" gender,accessoryID,beardID,costumeID,decoID,eyebrowsID,eyesID,hairID,"
		" lipstickID,makeupID,skinID,backgroundID,lightID,"
		" headRotation1,headRotation2,headRotation3,eyeRotation1,"
		" eyeRotation2,eyeRotation3,camPos1,camPos2,camPos3,"
		" morph1e,morph1n,morph1s,morph1w,morph2e,morph2n,"
		" morph2s,morph2w,morph3e,morph3n,morph3s,morph3w,"
		" morph4e,morph4n,morph4s,morph4w"
		" FROM character_ "
		"	LEFT JOIN entity ON characterID = itemID"
		" WHERE accountID=%u", accountID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}
	
	return(DBResultToRowset(res));
}

bool CharacterDB::ValidateCharName(const char *name)
{
	if (name == NULL || *name == '\0')
		return false;

	/* hash the name */
	uint32 hash = djb2_hash(name);

	/* check if its in our std::set */
	CharValidationSetItr itr = mNameValidation.find(hash);

	/* if itr is not equal to the end of the set it means that the same hash has been found */
	if (itr != mNameValidation.end())
		return false;

	/* if we got here the name is "new" */
	return true;
}

PyRepObject *CharacterDB::GetCharSelectInfo(uint32 characterID) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" itemName AS shortName,bloodlineID,gender,bounty,character_.corporationID,allianceID,title,startDateTime,createDateTime,"
		" securityRating,character_.balance,character_.stationID,solarSystemID,constellationID,regionID,"
		" petitionMessage,logonMinutes,tickerName"
		" FROM character_ "
		"	LEFT JOIN entity ON characterID = itemID"
		"	LEFT JOIN corporation USING (corporationID)"
		"	LEFT JOIN bloodlineTypes USING (typeID)"
		" WHERE characterID=%u", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}
	
	return(DBResultToRowset(res));
}

/*
 * This macro checks given CharacterAppearance object (app) if given value (v) is NULL:
 *  if yes, macro evaluates to string "NULL"
 *  if no, macro evaluates to call to function _ToStr, which turns given value to string.
 *
 * This macro is needed when saving CharacterAppearance values into DB (NewCharacter, SaveCharacterAppearance).
 * Resulting value must be const char *.
 */
#define _VoN(app, v) \
	((const char *)(app.IsNull_##v() ? "NULL" : _ToStr(app.Get_##v()).c_str()))

/* a 32 bits unsigned integer can be max 0xFFFFFFFF.
   this results in a text string: '4294967295' which
   is 10 long. Including the '\0' at the end of the
   string it is max 11.
 */
static std::string _ToStr(uint32 v) {
	char buf[11];
	snprintf(buf, 11, "%u", v);
	return(buf);
}

static std::string _ToStr(double v) {
	char buf[32];
	snprintf(buf, 32, "%.13f", v);
	return(buf);
}

PyRepObject *CharacterDB::GetCharPublicInfo(uint32 characterID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT "
		" entity.typeID,"
		" character_.corporationID,"
        " chrBloodlines.raceID,"
		" bloodlineTypes.bloodlineID,"
		" character_.ancestryID,"
		" character_.careerID,"
		" character_.schoolID,"
		" character_.careerSpecialityID,"
		" entity.itemName AS characterName,"
		" 0 as age,"	//hack
		" character_.createDateTime,"
		" character_.gender,"
		" character_.characterID,"
		" character_.description,"
		" character_.corporationDateTime"
        " FROM character_ "
		"	LEFT JOIN entity ON characterID = itemID"
		"	LEFT JOIN bloodlineTypes USING (typeID)"
		"	LEFT JOIN chrBloodlines USING (bloodlineID)"
		" WHERE characterID=%u", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Error in GetCharPublicInfo query: no data for char %d", characterID);
		return NULL;
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
		" WHERE characterID=%u", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}
	
	return(DBResultToRowset(res));
}

PyRepObject *CharacterDB::GetCharDesc(uint32 characterID) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" description "
		" FROM character_ "
		" WHERE characterID=%u", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Error in query: no data for char %d", characterID);
		return NULL;
	}

	return(DBRowToRow(row));
	
}

bool CharacterDB::SetCharDesc(uint32 characterID, const char *str) {
	DBerror err;

	std::string stringEsc;
	m_db->DoEscapeString(stringEsc, str);

	if(!m_db->RunQuery(err,
		"UPDATE character_"
		" SET description ='%s'"
		" WHERE characterID=%u", stringEsc.c_str(),characterID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		return false;
	}

	return (true);
}

//just return all itemIDs which has ownerID set to characterID
bool CharacterDB::GetCharItems(uint32 characterID, std::vector<uint32> &into) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		"  itemID"
		" FROM entity"
		" WHERE ownerID = %u",
		characterID))
	{
		_log(DATABASE__ERROR, "Failed to query items of char %u: %s.", characterID, res.error.c_str());
		return false;
	}

	DBResultRow row;
	while(res.GetRow(row))
		into.push_back(row.GetUInt(0));
	
	return true;
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
		" WHERE characterID=%u", charID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}

	return(DBResultToRowset(res));
}

bool CharacterDB::GetAttributesFromAncestry(uint32 ancestryID, uint8 &intelligence, uint8 &charisma, uint8 &perception, uint8 &memory, uint8 &willpower) {
	DBQueryResult res;

	if (!m_db->RunQuery(res,
		" SELECT "
		"        intelligence, charisma, perception, memory, willpower "
		" FROM chrAncestries "
		" WHERE ancestryID = %u ", ancestryID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return (false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Failed to find ancestry information for ancestry %u", ancestryID);
		return false;
	}
	
	intelligence += row.GetUInt(0);
	charisma += row.GetUInt(1);
	perception += row.GetUInt(2);
	memory += row.GetUInt(3);
	willpower += row.GetUInt(4);

	return (true);
}

/**
  * @todo Here should come a call to Corp??::CharacterJoinToCorp or what the heck... for now we only put it there
  */
bool CharacterDB::GetLocationCorporationByCareer(CharacterData &cdata) {
	DBQueryResult res;
	if (!m_db->RunQuery(res, 
	 "SELECT "
	 "  chrSchools.corporationID, "
	 "  chrSchools.schoolID, "
	 "  corporation.allianceID, "
	 "  corporation.stationID, "
	 "  staStations.solarSystemID, "
	 "  staStations.constellationID, "
	 "  staStations.regionID "
	 " FROM staStations"
	 "  LEFT JOIN corporation ON corporation.stationID=staStations.stationID"
	 "  LEFT JOIN chrSchools ON corporation.corporationID=chrSchools.corporationID"
	 "  LEFT JOIN chrCareers ON chrSchools.careerID=chrCareers.careerID"
	 " WHERE chrCareers.careerID = %u", cdata.careerID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return (false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Failed to find career %u", cdata.careerID);
		return false;
	}
	
	cdata.corporationID = row.GetUInt(0);
	cdata.schoolID = row.GetUInt(1);
	cdata.allianceID = row.GetUInt(2);

	cdata.stationID = row.GetUInt(3);
	cdata.solarSystemID = row.GetUInt(4);
	cdata.constellationID = row.GetUInt(5);
	cdata.regionID = row.GetUInt(6);

	return (true);
}

bool CharacterDB::GetSkillsByRace(uint32 raceID, std::map<uint32, uint32> &into) {
	DBQueryResult res;

	if (!m_db->RunQuery(res,
		"SELECT "
		"        skillTypeID, levels"
		" FROM chrRaceSkills "
		" WHERE raceID = %u ", raceID))
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
		" WHERE careerID = %u", careerID))
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
		" WHERE specialityID = %u", careerSpecialityID))
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

PyRepString *CharacterDB::GetNote(uint32 ownerID, uint32 itemID) {
	DBQueryResult res;

	if (!m_db->RunQuery(res,
			"SELECT `note` FROM `chrNotes` WHERE ownerID = %u AND itemID = %u",
			ownerID, itemID)
		)
	{
		codelog(SERVICE__ERROR, "Error on query: %s", res.error.c_str());
		return NULL;
	}
	DBResultRow row;
	if(!res.GetRow(row))
		return NULL;

	return(new PyRepString(row.GetText(0)));
}

bool CharacterDB::SetNote(uint32 ownerID, uint32 itemID, const char *str) {
	DBerror err;

	if (str[0] == '\0') {
		// str is empty
		if (!m_db->RunQuery(err,
			"DELETE FROM `chrNotes` "
			" WHERE itemID = %u AND ownerID = %u LIMIT 1",
			ownerID, itemID)
			)
		{
			codelog(CLIENT__ERROR, "Error on query: %s", err.c_str());
			return false;
		}
	} else {
		// escape it before insertion
		std::string escaped;
		m_db->DoEscapeString(escaped, str);

		if (!m_db->RunQuery(err,
			"REPLACE INTO `chrNotes` (itemID, ownerID, note)	"
			"VALUES (%u, %u, '%s')",
			ownerID, itemID, escaped.c_str())
			)
		{
			codelog(CLIENT__ERROR, "Error on query: %s", err.c_str());
			return false;
		}
	}

	return true;
}

uint32 CharacterDB::AddOwnerNote(uint32 charID, const std::string & label, const std::string & content) {
	DBerror err;
	uint32 id;

	std::string lblS;
	m_db->DoEscapeString(lblS, label);

	std::string contS;
	m_db->DoEscapeString(contS, content);

	if (!m_db->RunQueryLID(err, id, 
		"INSERT INTO chrOwnerNote (ownerID, label, note) VALUES (%u, '%s', '%s');",
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
		"UPDATE chrOwnerNote SET note = '%s' WHERE ownerID = %u AND noteID = %u;",
		contS.c_str(), charID, noteID))
	{
		codelog(SERVICE__ERROR, "Error on query: %s", err.c_str());
		return false;
	}

	return true;
}

PyRepObject *CharacterDB::GetOwnerNoteLabels(uint32 charID) {
	DBQueryResult res;

	if (!m_db->RunQuery(res, "SELECT noteID, label FROM chrOwnerNote WHERE ownerID = %u", charID))
	{
		codelog(SERVICE__ERROR, "Error on query: %s", res.error.c_str());
		return (NULL);
	}

	return DBResultToRowset(res);
}

PyRepObject *CharacterDB::GetOwnerNote(uint32 charID, uint32 noteID) {
	DBQueryResult res;

	if (!m_db->RunQuery(res, "SELECT note FROM chrOwnerNote WHERE ownerID = %u AND noteID = %u", charID, noteID))
	{
		codelog(SERVICE__ERROR, "Error on query: %s", res.error.c_str());
		return (NULL);
	}

	return DBResultToRowset(res);
}

uint32 CharacterDB::djb2_hash( const char* str )
{
	uint32 hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

void CharacterDB::load_name_validation_set()
{
	DBQueryResult res;
	if(!m_db->RunQuery(res,
		"SELECT"
		" characterID, itemName AS characterName"
		" FROM character_"
		"	JOIN entity ON characterID = itemID"
		))
	{
		codelog(SERVICE__ERROR, "Error in query for %s", res.error.c_str());
		return;
	}

	DBResultRow row;
	while(res.GetRow(row) == true)
	{
		uint32 characterID = row.GetUInt(0);
		const char* name = row.GetText(1);

		//printf("initializing name validation: %s\n", name);
		uint32 hash = djb2_hash(name);

		mNameValidation.insert(hash);
		mIdNameContainer.insert(std::make_pair(characterID, name));
	}
}

bool CharacterDB::add_name_validation_set( const char* name, uint32 characterID )
{
	if (name == NULL || *name == '\0')
		return false;

	uint32 hash = djb2_hash(name);

	/* check if the name is already present ( this should not be possible but we all know how hackers are ) */
	if (mNameValidation.find(hash) != mNameValidation.end())
	{
		printf("CharacterDB::add_name_validation_set: unable to add: %s as its a dupe", name);
		return false;
	}

	mNameValidation.insert(hash);
	mIdNameContainer.insert(std::make_pair(characterID, name));
	return true;
}

bool CharacterDB::del_name_validation_set( uint32 characterID )
{
	CharIdNameMapItr helper_itr = mIdNameContainer.find(characterID);

	/* if we are unable to find the entry... return.
	 * @note we do risk keeping the name in the name validation. 
	 * which I am willing to take.
	 */
	if (helper_itr == mIdNameContainer.end())
		return false;

	const char* name = helper_itr->second.c_str();
	if (name == NULL || *name == '\0')
		return false;

	uint32 hash = djb2_hash(name);

	CharValidationSetItr name_itr = mNameValidation.find(hash);
	if (name_itr != mNameValidation.end())
	{
		// we found the name hash... deleting
		mNameValidation.erase(name_itr);
		mIdNameContainer.erase(helper_itr);
		return true;
	}
	else
	{
		/* normally this should never happen... */
		printf("CharacterDB::del_name_validation_set: unable to remove: %s as its not in the set", name);
		return false;
	}
}
