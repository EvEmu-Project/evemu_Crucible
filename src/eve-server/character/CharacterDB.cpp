/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2011 The EVEmu Team
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
	Author:		Zhur
*/

#include "EVEServerPCH.h"

CharacterDB::CharacterDB()
{
	load_name_validation_set();
}

/*
SELECT characterID,itemName AS characterName,0 as deletePrepareDateTime, gender,accessoryID,beardID,costumeID,decoID,eyebrowsID,eyesID,hairID, lipstickID,makeupID,skinID,backgroundID,lightID, headRotation1,headRotation2,headRotation3,eyeRotation1, eyeRotation2,eyeRotation3,camPos1,camPos2,camPos3, morph1e,morph1n,morph1s,morph1w,morph2e,morph2n, morph2s,morph2w,morph3e,morph3n,morph3s,morph3w, morph4e,morph4n,morph4s,morph4w FROM character_ LEFT JOIN entity ON characterID = itemID WHERE accountID=%u", accountID))
*/

PyRep *CharacterDB::GetCharacterList(uint32 accountID) {
	DBQueryResult res;
	
	if(!sDatabase.RunQuery(res,
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
	
	return DBResultToCRowset(res);
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

PyRep *CharacterDB::GetCharSelectInfo(uint32 characterID) {
	DBQueryResult res;
	
	if(!sDatabase.RunQuery(res,
		"SELECT "
		" itemName AS shortName,bloodlineID,gender,bounty,character_.corporationID,allianceID,title,startDateTime,createDateTime,"
		" securityRating,character_.balance,character_.stationID,solarSystemID,constellationID,regionID,"
		" petitionMessage,logonMinutes,tickerName, 0 AS worldSpaceID, 'Filler' AS shipName, 606 AS shipTypeID, 0 AS unreadMailCount, 0 AS upcomingEventCount, 0 AS unprocessedNotifications, 14 AS daysLeft, 23 AS userType, 0 AS paperDollState, 0 AS newPaperdollState, 0 AS oldPaperdollState, 1337 AS skillPoints, 0 AS skillQueueEndTime, 0 AS allianceMemberStartDate, 0 AS startDate"
		" FROM character_ "
		"	LEFT JOIN entity ON characterID = itemID"
		"	LEFT JOIN corporation USING (corporationID)"
		"	LEFT JOIN bloodlineTypes USING (typeID)"
		" WHERE characterID=%u", characterID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}
	
	return DBResultToCRowset(res);
}

PyObject *CharacterDB::GetCharPublicInfo(uint32 characterID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
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

//void CharacterDB::GetCharacterData(uint32 characterID, std::vector<uint32> &characterDataVector) {
void CharacterDB::GetCharacterData(uint32 characterID, std::map<std::string, uint32> &characterDataMap) {

	DBQueryResult res;
    DBResultRow row;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "  character_.corporationID, "
        "  character_.stationID, "
        "  character_.solarSystemID, "
        "  character_.constellationID, "
        "  character_.regionID, "
        "  corporation.stationID, "
        "  character_.corpRole, "
        "  character_.rolesAtAll, "
        "  character_.rolesAtBase, "
        "  character_.rolesAtHQ, "
        "  character_.rolesAtOther, "
        "  entity.locationID "
        " FROM character_ "
        "  LEFT JOIN corporation USING (corporationID) "
        "  LEFT JOIN entity ON entity.itemID = character_.characterID "
        " WHERE characterID = %u",
        characterID))
    {
        sLog.Error("CharacterDB::GetCharPublicInfo2()", "Failed to query HQ of character's %u corporation: %s.", characterID, res.error.c_str());
    }

	if(!res.GetRow(row))
    {
        sLog.Error("CharacterDB::GetCharacterData()", "No valid rows were returned by the database query.");
	}
	
//    std::map<std::string,uint32> characterDataMap;
//    for( uint32 i=0; i<=characterDataVector.size(); i++ )
//        characterDataVector.push_back( row.GetUInt(i) );

    characterDataMap["corporationID"] = row.GetUInt(0);
    characterDataMap["stationID"] = row.GetUInt(1);
    characterDataMap["solarSystemID"] = row.GetUInt(2);
    characterDataMap["constellationID"] = row.GetUInt(3);
    characterDataMap["regionID"] = row.GetUInt(4);
    characterDataMap["corporationHQ"] = row.GetUInt(5);
    characterDataMap["corpRole"] = row.GetUInt(6);
    characterDataMap["rolesAtAll"] = row.GetUInt(7);
    characterDataMap["rolesAtBase"] = row.GetUInt(8);
    characterDataMap["rolesAtHQ"] = row.GetUInt(9);
    characterDataMap["rolesAtOther"] = row.GetUInt(10);
    characterDataMap["locationID"] = row.GetUInt(11);
}

PyObject *CharacterDB::GetCharPublicInfo3(uint32 characterID) {

	DBQueryResult res;
	
	if(!sDatabase.RunQuery(res,
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
	
	return DBResultToRowset(res);
}

//just return all itemIDs which has ownerID set to characterID
bool CharacterDB::GetCharItems(uint32 characterID, std::vector<uint32> &into) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
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

//returns a list of the itemID for all the clones belonging to the character
bool CharacterDB::GetCharClones(uint32 characterID, std::vector<uint32> &into) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		"  itemID"
		" FROM entity"
		" WHERE ownerID = %u"
		" and flag='400'",
		characterID))
	{
		_log(DATABASE__ERROR, "Failed to query clones of char %u: %s.", characterID, res.error.c_str());
		return false;
	}

	DBResultRow row;
	while(res.GetRow(row))
		into.push_back(row.GetUInt(0));
	
	return true;
}

//returns the itemID of the active clone
//if you want to get the typeID of the clone, please use GetActiveCloneType
bool CharacterDB::GetActiveClone(uint32 characterID, uint32 &itemID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		"  itemID"
		" FROM entity"
		" WHERE ownerID = %u"
		" and flag='400'"
		" and customInfo='active'",
		characterID))
	{
		_log(DATABASE__ERROR, "Failed to query active clone of char %u: %s.", characterID, res.error.c_str());
		return false;
	}

	DBResultRow row;
	res.GetRow(row);
	itemID=row.GetUInt(0);
	
	return true;
}

//we use this function because, when we change the clone type,
//the cached item type doesn't change, so we need to read it 
//directly from the db
bool CharacterDB::GetActiveCloneType(uint32 characterID, uint32 &typeID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		"  typeID"
		" FROM entity"
		" WHERE ownerID = %u"
		" and flag='400'"
		" and customInfo='active'",
		characterID))
	{
		_log(DATABASE__ERROR, "Failed to query active clone of char %u: %s.", characterID, res.error.c_str());
		return false;
	}

	DBResultRow row;
	res.GetRow(row);
	typeID=row.GetUInt(0);
	
	return true;
}



PyObject *CharacterDB::GetCharacterAppearance(uint32 charID) {
	DBQueryResult res;
	
	if(!sDatabase.RunQuery(res,
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

	return DBResultToRowset(res);
}

bool CharacterDB::GetAttributesFromAncestry(uint32 ancestryID, uint8 &intelligence, uint8 &charisma, uint8 &perception, uint8 &memory, uint8 &willpower) {
	DBQueryResult res;

	if (!sDatabase.RunQuery(res,
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

bool CharacterDB::GetCareerBySchool(uint32 schoolID, uint32 &careerID) {
	DBQueryResult res;
	if (!sDatabase.RunQuery(res, 
	 "SELECT "
	 " careerID, "
	 " schoolID, "
	 " raceID "
	 " FROM careers"
	 " WHERE schoolID = %u", schoolID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return (false);
	}
	
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Failed to find matching career for school %u", schoolID);
		return false;
	}

	careerID = row.GetInt(0);

	return (true);
}


/**
  * @todo Here should come a call to Corp??::CharacterJoinToCorp or what the heck... for now we only put it there
  */
bool CharacterDB::GetLocationCorporationByCareer(CharacterData &cdata) {
	DBQueryResult res;
	if (!sDatabase.RunQuery(res, 
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
	 "  LEFT JOIN careers ON chrSchools.schoolID=careers.schoolID"
	 " WHERE careers.careerID = %u", cdata.careerID))
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

bool CharacterDB::GetLocationByStation(uint32 staID, CharacterData &cdata) {
	DBQueryResult res;
	if (!sDatabase.RunQuery(res, 
	 "SELECT "
	 "  stationID, "
	 "  solarSystemID, "
	 "  constellationID, "
	 "  regionID "
	 " FROM staStations"
	 " WHERE stationID = %u", staID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return (false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Failed to find station %u", staID);
		return false;
	}
	
	cdata.stationID = staID;
	cdata.solarSystemID = row.GetUInt(1);
	cdata.constellationID = row.GetUInt(2);
	cdata.regionID = row.GetUInt(3);

	return (true);

}

bool CharacterDB::DoesCorporationExist(uint32 corpID) {
	DBQueryResult res;
	if (!sDatabase.RunQuery(res, 
	 "SELECT "
	 "  corporationID, "
	 "  corporationName "
	 " FROM corporation"
	 " WHERE corporationID = %u", corpID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Failed to find corporation %u", corpID);
		return false;
	}
	return true;
}


bool CharacterDB::GetSkillsByRace(uint32 raceID, std::map<uint32, uint32> &into) {
	DBQueryResult res;

	if (!sDatabase.RunQuery(res,
		"SELECT "
		"        skillTypeID, levels"
		" FROM raceSkills "
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

	if (!sDatabase.RunQuery(res,
		"SELECT "
		"        skillTypeID, levels"
		" FROM careerSkills"
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

	if (!sDatabase.RunQuery(res,
		"SELECT "
		"        skillTypeID, levels"
		" FROM specialitySkills"
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

PyString *CharacterDB::GetNote(uint32 ownerID, uint32 itemID) {
	DBQueryResult res;

	if (!sDatabase.RunQuery(res,
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

	return(new PyString(row.GetText(0)));
}

bool CharacterDB::SetNote(uint32 ownerID, uint32 itemID, const char *str) {
	DBerror err;

	if (str[0] == '\0') {
		// str is empty
		if (!sDatabase.RunQuery(err,
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
		sDatabase.DoEscapeString(escaped, str);

		if (!sDatabase.RunQuery(err,
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
	sDatabase.DoEscapeString(lblS, label);

	std::string contS;
	sDatabase.DoEscapeString(contS, content);

	if (!sDatabase.RunQueryLID(err, id, 
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
	sDatabase.DoEscapeString(contS, content);

	if (!sDatabase.RunQuery(err,
		"UPDATE chrOwnerNote SET note = '%s' WHERE ownerID = %u AND noteID = %u;",
		contS.c_str(), charID, noteID))
	{
		codelog(SERVICE__ERROR, "Error on query: %s", err.c_str());
		return false;
	}

	return true;
}

PyObject *CharacterDB::GetOwnerNoteLabels(uint32 charID) {
	DBQueryResult res;

	if (!sDatabase.RunQuery(res, "SELECT noteID, label FROM chrOwnerNote WHERE ownerID = %u", charID))
	{
		codelog(SERVICE__ERROR, "Error on query: %s", res.error.c_str());
		return (NULL);
	}

	return DBResultToRowset(res);
}

PyObject *CharacterDB::GetOwnerNote(uint32 charID, uint32 noteID) {
	DBQueryResult res;

	if (!sDatabase.RunQuery(res, "SELECT note FROM chrOwnerNote WHERE ownerID = %u AND noteID = %u", charID, noteID))
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
	if(!sDatabase.RunQuery(res,
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
