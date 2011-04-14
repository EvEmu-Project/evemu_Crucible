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
	Author:		Zhur, Aknor Jaden
*/


#include "EVEServerPCH.h"

PyObject *LSCDB::LookupChars(const char *match, bool exact) {
	DBQueryResult res;
	
	std::string matchEsc;
	sDatabase.DoEscapeString(matchEsc, match);
	if (matchEsc == "__ALL__") {
		if(!sDatabase.RunQuery(res,
			"SELECT "
			"	characterID, itemName AS characterName, typeID"
			" FROM character_"
			"  LEFT JOIN entity ON characterID = itemID"
			" WHERE characterID >= 140000000"))
		{
			_log(SERVICE__ERROR, "Error in LookupChars query: %s", res.error.c_str());
			return NULL;
		}
	} else {
		if(!sDatabase.RunQuery(res,
			"SELECT "
			"	characterID, itemName AS characterName, typeID"
			" FROM character_"
			"  LEFT JOIN entity ON characterID = itemID"
			" WHERE itemName %s '%s'", 
			exact?"=":"RLIKE", matchEsc.c_str()
		))
		{
			_log(SERVICE__ERROR, "Error in LookupChars query: %s", res.error.c_str());
			return NULL;
		}
	}
	
	return DBResultToRowset(res);
}


PyObject *LSCDB::LookupOwners(const char *match, bool exact) {
	DBQueryResult res;
	
	std::string matchEsc;
	sDatabase.DoEscapeString(matchEsc, match);

    // so each row needs "ownerID", "ownerName", and "groupID"
    // ownerID = either characterID or corporationID
    // ownerName = either characterName or corporationName
    // groupID = either 1 for character or 2 for corporation

	if(!sDatabase.RunQuery(res,
        "SELECT"
        "  character_.characterID AS ownerID,"
        "  entity.itemName AS ownerName,"
        "  invTypes.groupID AS groupID"
        " FROM character_"
        "  LEFT JOIN entity ON characterID = itemID"
        "  LEFT JOIN invTypes ON entity.typeID = invTypes.typeID"
        " WHERE character_.characterID >= 140000000"
        "  AND entity.itemName %s '%s'"
        " UNION "
        "SELECT"
        "  corporation.corporationID AS ownerID,"
        "  corporation.corporationName AS ownerName,"
        "  invTypes.groupID AS groupID"
        " FROM corporation"
        "  LEFT JOIN invTypes ON groupID = 2"
        " WHERE corporation.corporationName %s '%s'",
		(exact?"=":"RLIKE"), matchEsc.c_str(), (exact?"=":"RLIKE"), matchEsc.c_str()))
	{
		_log(DATABASE__ERROR, "Failed to lookup player char '%s': %s.", matchEsc.c_str(), res.error.c_str());
		return NULL;
	}
	
	return DBResultToRowset(res);
}


PyObject *LSCDB::LookupPlayerChars(const char *match, bool exact) {
	DBQueryResult res;

	std::string matchEsc;
	sDatabase.DoEscapeString(matchEsc, match);
	if(!sDatabase.RunQuery(res,
		"SELECT"
		" characterID, itemName AS characterName, typeID"
		" FROM character_"
		"  LEFT JOIN entity ON characterID = itemID"
		" WHERE characterID >= 140000000"
		"  AND itemName %s '%s'",
		exact?"=":"RLIKE", matchEsc.c_str()))
	{
		_log(DATABASE__ERROR, "Failed to lookup player char '%s': %s.", matchEsc.c_str(), res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}


PyObject *LSCDB::LookupCorporations(const std::string & search) {
	DBQueryResult res;
	std::string secure;
	sDatabase.DoEscapeString(secure, search);

	if (!sDatabase.RunQuery(res,
		" SELECT "
		" corporationID, corporationName, corporationType "
		" FROM corporation "
		" WHERE corporationName RLIKE '%s'", secure.c_str()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return 0;
	}

	return DBResultToRowset(res);
}


PyObject *LSCDB::LookupFactions(const std::string & search) {
	DBQueryResult res;
	std::string secure;
	sDatabase.DoEscapeString(secure, search);

	if (!sDatabase.RunQuery(res,
		" SELECT "
		" factionID, factionName "
		" FROM chrFactions "
		" WHERE factionName RLIKE '%s'", secure.c_str()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return 0;
	}

	return DBResultToRowset(res);
}


PyObject *LSCDB::LookupCorporationTickers(const std::string & search) {
	DBQueryResult res;
	std::string secure;
	sDatabase.DoEscapeString(secure, search);

	if (!sDatabase.RunQuery(res,
		" SELECT "
		" corporationID, corporationName, tickerName "
		" FROM corporation "
		" WHERE tickerName RLIKE '%s'", secure.c_str()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return 0;
	}

	return DBResultToRowset(res);
}


PyObject *LSCDB::LookupStations(const std::string & search) {
	DBQueryResult res;
	std::string secure;
	sDatabase.DoEscapeString(secure, search);

	if (!sDatabase.RunQuery(res,
		" SELECT "
		" stationID, stationName, stationTypeID "
		" FROM staStations "
		" WHERE stationName RLIKE '%s'", secure.c_str()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return 0;
	}

	return DBResultToRowset(res);
}


PyObject *LSCDB::LookupKnownLocationsByGroup(const std::string & search, uint32 typeID) {
	DBQueryResult res;
	std::string secure;
	sDatabase.DoEscapeString(secure, search);

	if (!sDatabase.RunQuery(res,
		" SELECT "
		" itemID, itemName, typeID "
		" FROM entity "
		" WHERE itemName RLIKE '%s' AND typeID = %u", secure.c_str(), typeID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return 0;
	}

	return DBResultToRowset(res);
}


//temporarily relocated into ServiceDB until some things get cleaned up...
uint32 LSCDB::StoreMail(uint32 senderID, uint32 recipID, const char * subject, const char * message, uint64 sentTime) {
	DBQueryResult res;
	DBerror err;
	DBResultRow row;

	std::string escaped;
	// Escape message header
	sDatabase.DoEscapeString(escaped, subject);

	// Store message header
	uint32 messageID;
	if (!sDatabase.RunQueryLID(err, messageID, 
		" INSERT INTO "
		" eveMail "
		" (channelID, senderID, subject, created) "
		" VALUES (%u, %u, '%s', "I64u") ",
		recipID, senderID, escaped.c_str(), sentTime ))
	{
		codelog(SERVICE__ERROR, "Error in query, message header couldn't be saved: %s", err.c_str());
		return (0);
	}

	_log(SERVICE__MESSAGE, "New messageID: %u", messageID);

	// Escape message content
	sDatabase.DoEscapeString(escaped, message);

	// Store message content
	if (!sDatabase.RunQuery(err,
		" INSERT INTO eveMailDetails "
		" (messageID, mimeTypeID, attachment) VALUES (%u, 1, '%s') ",
		messageID, escaped.c_str()
		))
	{
		codelog(SERVICE__ERROR, "Error in query, message content couldn't be saved: %s", err.c_str());
		// Delete message header
		if (!sDatabase.RunQuery(err, "DELETE FROM `eveMail` WHERE `messageID` = %u;", messageID))
		{
			codelog(SERVICE__ERROR, "Failed to remove invalid header data for messgae id %u: %s", messageID, err.c_str());
		}
		return (0);
	}


	return (messageID);
}


PyObject *LSCDB::GetMailHeaders(uint32 recID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT channelID, messageID, senderID, subject, created, `read` "
		" FROM eveMail "
		" WHERE channelID=%u", recID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}


PyRep *LSCDB::GetMailDetails(uint32 messageID, uint32 readerID) {
	DBQueryResult result;
	DBResultRow row;

	//we need to query out the primary message here... not sure how to properly
	//grab the "main message" though... the text/plain clause is pretty hackish.
	if (!sDatabase.RunQuery(result,
		" SELECT eveMail.messageID, eveMail.senderID, eveMail.subject, " // need messageID as char*
		" eveMailDetails.attachment, eveMailDetails.mimeTypeID, "
		" eveMailMimeType.mimeType, eveMailMimeType.`binary`, "
		" eveMail.created, eveMail.channelID "
		" FROM eveMail "
		" LEFT JOIN eveMailDetails"
		"	ON eveMailDetails.messageID = eveMail.messageID "
		" LEFT JOIN eveMailMimeType"
		"	ON eveMailMimeType.mimeTypeID = eveMailDetails.mimeTypeID "
		" WHERE eveMail.messageID=%u"
		"	AND channelID=%u",
			messageID, readerID
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", result.error.c_str());
		return (NULL);
	}

	if (!result.GetRow(row)) {
		codelog(SERVICE__MESSAGE, "No message with messageID %u", messageID);
		return (NULL);
	}

	Rsp_GetEVEMailDetails details;
	details.messageID = row.GetUInt(0);
	details.senderID = row.GetUInt(1);
	details.subject = row.GetText(2);
	details.body = row.GetText(3);
	details.created = row.GetUInt64(7);
	details.channelID = row.GetUInt(8);
	details.deleted = 0; // If a message's details are sent, then it isn't deleted. If it's deleted, details cannot be sent
	details.mimeTypeID = row.GetInt(4);
	details.mimeType = row.GetText(5);
	details.binary = row.GetInt(6);

	return(details.Encode());
}


bool LSCDB::MarkMessageRead(uint32 messageID) {
	DBerror err;

	if (!sDatabase.RunQuery(err,
		" UPDATE eveMail "
		" SET `read` = 1 "
		" WHERE messageID=%u", messageID
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		return false;
	}

	return true;
}


bool LSCDB::DeleteMessage(uint32 messageID, uint32 readerID) {
	DBerror err;
	bool ret = true;

	if (!sDatabase.RunQuery(err,
		" DELETE FROM eveMail "
		" WHERE messageID=%u AND channelID=%u", messageID, readerID
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		ret = false;
	}
	if (!sDatabase.RunQuery(err,
		" DELETE FROM eveMailDetails "
		" WHERE messageID=%u", messageID
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		ret = false;
	}

	return ret;

}


void LSCDB::GetChannelNames(uint32 charID, std::vector<std::string> & names) {
	DBQueryResult res;

	if (!sDatabase.RunQuery(res, 
		" SELECT "
		"	entity.itemName AS characterName, "
		"	corporation.corporationName, "
		"	mapSolarSystems.solarSystemName, "
		"	mapConstellations.constellationName, "
		"	mapRegions.regionName "
		" FROM character_ "
		"	LEFT JOIN entity ON character_.characterID = entity.itemID "
		"	LEFT JOIN corporation ON character_.corporationID = corporation.corporationID "
		"	LEFT JOIN mapSolarSystems ON character_.solarSystemID = mapSolarSystems.solarSystemID "
		"	LEFT JOIN mapConstellations ON character_.constellationID = mapConstellations.constellationID "
		"	LEFT JOIN mapRegions ON character_.regionID = mapRegions.regionID "
		" WHERE character_.characterID = %u ", charID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return;
	}

	DBResultRow row;

	if (!res.GetRow(row)) {
		_log(SERVICE__ERROR, "CharID %u isn't present in the database", charID);
		return;
	}

	names.push_back(row.GetText(0));	// charName
	names.push_back(row.GetText(1));	// corpName
	names.push_back(row.GetText(2));	// solsysName
	names.push_back(row.GetText(3));	// constName
	names.push_back(row.GetText(4));	// regionName
}


// Function: Query 'channels' table for a list of all channelIDs and traverse that list from the beginning to find
// the first gap in consecutive channelIDs and return the value for the first number in that gap.
uint32 LSCDB::GetNextAvailableChannelID()
{
	DBQueryResult res;

    // Query the 'channels' table to get a list of all channel IDs.
    // NOTE: For large servers, this is inefficient and as everything in this file should be using
    // the cached object system rather than touching the database, this query could cause large server slow-down
    // if there is a very large number of existing channels in the database.
	if( !sDatabase.RunQuery( res,
		" SELECT "
		"	channelID "
		" FROM channels "
		" WHERE channelID >= %u ", LSCService::BASE_CHANNEL_ID ))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return 0;
	}

	uint32 currentChannelID = LSCService::BASE_CHANNEL_ID;

	// Traverse through the rows in the query result until the first gap is found
	// and return the value that would be first (or only one) in the gap as the next
	// free channel ID:
	DBResultRow row;
	while( res.GetRow(row) )
	{
        const uint32 channelID = row.GetUInt( 0 );

        if( currentChannelID < channelID )
            return currentChannelID;

        ++currentChannelID;
	}

        // Check to make sure that the next available channelID is not equal to the Maximum channel ID value
	if( currentChannelID <= LSCService::MAX_CHANNEL_ID )
        return currentChannelID;
	else
        return 0;	// No free channel IDs found (this should never happen as there are way too many IDs to exhaust)
}


// Function: Return true or false result for the check of whether or not the specified
// channel 'displayName' is already being used by a channel.
bool LSCDB::IsChannelNameAvailable(std::string name)
{
	DBQueryResult res;

	// MySQL query channels table for any channel whose displayName matches "name":
	if (!sDatabase.RunQuery(res, 
		" SELECT "
		"	displayName "
		" FROM channels "
		" WHERE displayName = upper('%s')", name.c_str()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}

	DBResultRow row;

    // Return true (this 'displayName' not in use) if there are no rows returned by the query:
	if (!res.GetRow(row))
		return true;
	else
		return false;
}


// Function: Return true or false result for the check of whether or not the specified
// channelID is available to be taken for a new channel's channelID.
bool LSCDB::IsChannelIDAvailable(uint32 channelID)
{
	DBQueryResult res;

	if (!sDatabase.RunQuery(res, 
		" SELECT "
		"	channelID "
		" FROM channels "
		" WHERE channelID = %u", channelID ))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}

	DBResultRow row;

    // Return true (this channelID not in use) if there are no rows returned by the query:
	if (!(res.GetRow(row)))
		return true;
	else
		return false;
}


// Function: Return true or false result for the check of whether or not the channel
// specified by channelID is already subscribed to by the character specified by charID.
bool LSCDB::IsChannelSubscribedByThisChar(uint32 charID, uint32 channelID)
{
	DBQueryResult res;

	if (!sDatabase.RunQuery(res, 
		" SELECT "
		"	channelID, "
		"   charID "
		" FROM channelChars "
		" WHERE channelID = %u AND charID = %u", channelID, charID ))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}

	DBResultRow row;

    // Return false (no subscription exists) if there are no rows returned by the query:
	if (!(res.GetRow(row)))
		return false;
	else
		return true;
}


// Function: Query 'channels' table for the channel whose 'displayName' matches the name specified,
// then return all parameters for that channel.
void LSCDB::GetChannelInformation(std::string & name, uint32 & id,
		std::string & motd, uint32 & ownerid, std::string & compkey,
		bool & memberless, std::string & password, bool & maillist,
		uint32 & cspa, uint32 & temp, uint32 & mode)
{
	DBQueryResult res;

	if (!sDatabase.RunQuery(res, 
		" SELECT "
		"	channelID, "
		"	displayName, "
		"   motd, "
		"   ownerID, "
		"   comparisonKey, "
		"   memberless, "
		"   password, "
		"   mailingList, "
		"   cspa, "
		"   temporary, "
		"   mode "
		" FROM channels "
		" WHERE displayName = upper('%s')", name.c_str()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return;
	}

	DBResultRow row;

	if (!(res.GetRow(row)))
	{
            _log(SERVICE__ERROR, "Channel named '%s' isn't present in the database", name.c_str() );
            return;
	}

	id = row.GetUInt(0);
	name = (row.GetText(1) == NULL ? "" : row.GetText(1));	// empty displayName field in channels table row returns NULL, so fill this string with "" in that case
	motd = (row.GetText(2) == NULL ? "" : row.GetText(2));	// empty motd field in channels table row returns NULL, so fill this string with "" in that case
	ownerid = row.GetUInt(3);
	compkey = (row.GetText(4) == NULL ? "" : row.GetText(4));	// empty comparisonKey field in channels table row returns NULL, so fill this string with "" in that case
	memberless = row.GetUInt(5) ? true : false;
	password = (row.GetText(6) == NULL ? "" : row.GetText(6));	// empty password field in channels table row returns NULL, so fill this string with "" in that case
	maillist = row.GetUInt(7) ? true : false;
	cspa = row.GetUInt(8);
	temp = row.GetUInt(9);
	mode = row.GetUInt(10);
}


// Function: Query 'channels' table for the channel whose 'channelID' matches the ID specified,
// then return all parameters for that channel.
void LSCDB::GetChannelInformation(uint32 channelID, std::string & name,
	    std::string & motd, uint32 & ownerid, std::string & compkey,
	    bool & memberless, std::string & password, bool & maillist,
	    uint32 & cspa, uint32 & temp, uint32 & mode)
{
	DBQueryResult res;

	if (!sDatabase.RunQuery(res, 
		" SELECT "
		"	channelID, "
		"	displayName, "
		"   motd, "
		"   ownerID, "
		"   comparisonKey, "
		"   memberless, "
		"   password, "
		"   mailingList, "
		"   cspa, "
		"   temporary, "
		"   mode "
		" FROM channels "
		" WHERE channelID = %u", channelID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return;
	}

	DBResultRow row;

	if (!(res.GetRow(row)))
	{
            _log(SERVICE__ERROR, "Channel named '%s' isn't present in the database", name.c_str() );
            return;
	}

	name = (row.GetText(1) == NULL ? "" : row.GetText(1));	// empty displayName field in channels table row returns NULL, so fill this string with "" in that case
	motd = (row.GetText(2) == NULL ? "" : row.GetText(2));	// empty motd field in channels table row returns NULL, so fill this string with "" in that case
	ownerid = row.GetUInt(3);
	compkey = (row.GetText(4) == NULL ? "" : row.GetText(4));	// empty comparisonKey field in channels table row returns NULL, so fill this string with "" in that case
	memberless = row.GetUInt(5) ? true : false;
	password = (row.GetText(6) == NULL ? "" : row.GetText(6));	// empty password field in channels table row returns NULL, so fill this string with "" in that case
	maillist = row.GetUInt(7) ? true : false;
	cspa = row.GetUInt(8);
	temp = row.GetUInt(9);
	mode = row.GetUInt(10);
}



// Function: Query the 'channelChars' table for all channels subscribed to by the character specified by charID and
// return lists of parameters for all of those channels as well as a total channel count.
void LSCDB::GetChannelSubscriptions(uint32 charID, std::vector<unsigned long> & ids, std::vector<std::string> & names,
		std::vector<std::string> & MOTDs, std::vector<unsigned long> & ownerids, std::vector<std::string> & compkeys,
		std::vector<int> & memberless, std::vector<std::string> & passwords, std::vector<int> & maillists,
		std::vector<int> & cspas, std::vector<int> & temps, std::vector<int> & modes, int & channelCount)
{
	DBQueryResult res;

	// Cross-reference "channelchars" table with "channels" table using the charID
	// The result is a two column multi-row structure where each row is a channel
	// that the character (charID) is subscribed to where the channel ID is presented
	// in the first column and the display name of that channel in the second column
	if (!sDatabase.RunQuery(res,
		" SELECT "
		"	channelID, "
		"	displayName, "
		"   motd, "
		"   ownerID, "
		"   comparisonKey, "
		"   memberless, "
		"   password, "
		"   mailingList, "
		"   cspa, "
		"   temporary, "
		"   mode "
		" FROM channels "
		" WHERE channelID = ANY ("
		"   SELECT channelID FROM channelChars WHERE charID = %u )", charID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return;
	}

	DBResultRow row;
	int rowCount = 0;

	// Traverse through all rows in the query result and copy the IDs and displayNames to the
	// "ids" and "names" vectors for return to the calling function:
	while(res.GetRow(row))
	{
		++rowCount;

		ids.push_back(row.GetUInt(0));
		names.push_back((row.GetText(1) == NULL ? "" : row.GetText(1)));	// empty displayName field in channels table row returns NULL, so fill this string with "" in that case
		MOTDs.push_back((row.GetText(2) == NULL ? "" : row.GetText(2)));	// empty motd field in channels table row returns NULL, so fill this string with "" in that case
		ownerids.push_back(row.GetUInt(3));
		compkeys.push_back((row.GetText(4) == NULL ? "" : row.GetText(4)));	// empty comparisonKey field in channels table row returns NULL, so fill this string with "" in that case
		memberless.push_back(row.GetUInt(5));
		passwords.push_back((row.GetText(6) == NULL ? "" : row.GetText(6)));	// empty password field in channels table row returns NULL, so fill this string with "" in that case
		maillists.push_back(row.GetUInt(7));
		cspas.push_back(row.GetUInt(8));
		temps.push_back(row.GetUInt(9));
		modes.push_back(row.GetUInt(10));
	}

	if (rowCount == 0) {
		_log(SERVICE__ERROR, "CharID %u isn't present in the database", charID);
		return;
	}

	channelCount = rowCount;
}


std::string LSCDB::GetChannelInfo(uint32 channelID, std::string & name, std::string & motd)
{
	DBQueryResult res;

	if (!sDatabase.RunQuery(res,
		" SELECT "
		"	displayName, "
		"   motd "
		" FROM channels "
		" WHERE channelID = %u ", channelID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		char err[20];
		snprintf(err, 20, "Unknown %u", channelID);
		return(err);
	}

	DBResultRow row;

	if (!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Couldn't find %u in table channels", channelID);
		char err[20];
		snprintf(err, 20, "Unknown %u", channelID);
		return(err);
	}

	name = row.GetText(0);
	motd = row.GetText(1);

	return ("");
}


uint32 LSCDB::GetChannelIDFromComparisonKey(std::string compkey)
{
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT "
		"	channelID "
		" FROM channels"
		" WHERE comparisonKey RLIKE '%s'", 
        compkey.c_str()
	))
	{
		_log(SERVICE__ERROR, "Error in GetChannelIDFromComparisonKey query: %s", res.error.c_str());
		return 0;
	}

	DBResultRow row;

	if (!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Couldn't find %s in table channels", compkey.c_str());
        return 0;
	}

    return (row.GetInt(0));
}

std::string LSCDB::GetChannelName(uint32 id, const char * table, const char * column, const char * key) {
	DBQueryResult res;

	if (!sDatabase.RunQuery(res, 
		" SELECT "
		"	%s "
		" FROM %s "
		" WHERE %s = %u ", column, table, key, id))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		char err[20];
		snprintf(err, 20, "Unknown %u", id);
		return(err);
	}

	DBResultRow row;

	if (!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Couldn't find %s %u in table %s", key, id, table);
		char err[20];
		snprintf(err, 20, "Unknown %u", id);
		return(err);
	}

	return (row.GetText(0));
}


// Take the channelID of a chat channel that a character specified by characterID just subscribed to
// and create a new entry in the 'channelChars' table for this subscription.
int LSCDB::WriteNewChannelSubscriptionToDatabase(uint32 characterID, uint32 channelID, uint32 corpID, uint32 allianceID, uint32 role, uint32 extra)
{
	DBQueryResult res;
	DBerror err;
	DBResultRow row;

	if (!sDatabase.RunQuery(err,
		" INSERT INTO channelChars "
		" (channelID, corpID, charID, allianceID, role, extra) VALUES (%u, %u, %u, %u, %u, %u) ",
		channelID, corpID, characterID, allianceID, role, extra
		))
	{
		_log(SERVICE__ERROR, "Error in query, Channel Subscription content couldn't be saved: %s", err.c_str());
		return 0;
	}
	else
		return 1;
}


// Function: Take channelID, channel name, the characterID of the character creating the channel, and a flag
// indicating whether this channel is a private convo (temporary), then create a new entry in the 'channels'
// table.
int LSCDB::WriteNewChannelToDatabase(uint32 channelID, std::string name, uint32 ownerID, uint32 temporary, uint32 mode)
{
	DBQueryResult res;
	DBerror err;
	DBResultRow row;

	if (!sDatabase.RunQuery(err,
		" INSERT INTO channels "
		"      (channelID, ownerID, displayName, motd, comparisonKey, memberless, password, mailingList,"
		"       cspa, temporary, mode, subscribed, estimatedMemberCount) "
		" VALUES (%u, %u, '%s', '%s', '%s', 0, NULL, 0, 0, %u, %u, 1, 1) ",
		channelID, ownerID, name.c_str(), "", name.c_str(), temporary, mode
		))
	{
		_log(SERVICE__ERROR, "Error in query, New Channel couldn't be saved: %s", err.c_str());
		return 0;
	}
	else
		return 1;
}


// Function: Accept pointer to a LSCChannel object for the channel being updated in its
// existing entry in the 'channels' table.  Take all channel parameters and write them to the
// selected 'channels' table entry based on channelID
int LSCDB::UpdateChannelConfigureInfo(LSCChannel * channel)
{
	DBerror err;
    std::string new_password;

    // Ensure that the string "NULL" is written into the table row for the password field if
    // the channel's password is empty
    if (channel->GetPassword() == "")
        new_password = "NULL";
    else
        new_password = "'" + channel->GetPassword() + "'";

	if (!sDatabase.RunQuery(err,
		" UPDATE channels "
		" SET "
        " displayName = '%s', "
        " motd = '%s', "
        " comparisonKey = '%s', "
        " memberless = %u, "
        " password = %s, "
        " mailingList = %u, "
        " cspa = %u, "
        " temporary = %u, "
        " mode = %u, "
        " subscribed = %u, "
        " estimatedMemberCount = %u "
        " WHERE channelID=%u",
        channel->GetDisplayName().c_str(),
        channel->GetMOTD().c_str(),
        channel->GetComparisonKey().c_str(),
        ((channel->GetMemberless()) ? 1 : 0),
        new_password.c_str(),
        ((channel->GetMailingList()) ? 1 : 0),
        channel->GetCSPA(),
        channel->GetTemporary(),
        channel->GetMode(),
        1,
        channel->GetMemberCount(),
        channel->GetChannelID()
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		return false;
	}

	return true;
}


// Function: Remove subscription to the chat channel specified by channelID for the character
// specified by charID from the 'channelChars' table
int LSCDB::RemoveChannelSubscriptionFromDatabase(uint32 channelID, uint32 charID)
{
	DBerror err;
	bool ret = true;

	if (!sDatabase.RunQuery(err,
		" DELETE FROM channelChars "
		" WHERE channelID=%u AND charID=%u", channelID, charID
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		ret = false;
	}

	return ret;
}


// Function: Remove the chat channel from the 'channels' table using specified channelID
int LSCDB::RemoveChannelFromDatabase(uint32 channelID)
{
	DBerror err;
	bool ret = true;

	if (!sDatabase.RunQuery(err,
		" DELETE FROM channels "
		" WHERE channelID=%u", channelID
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		ret = false;
	}

	return ret;
}
