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

LSCDB::LSCDB(DBcore *db)
: ServiceDB(db)
{
}

LSCDB::~LSCDB() {
}

PyRepObject *LSCDB::LookupChars(const char *match, bool exact) {
	DBQueryResult res;
	
	std::string matchEsc;
	m_db->DoEscapeString(matchEsc, match);
	if (matchEsc == "__ALL__") {
		if(!m_db->RunQuery(res,
			"SELECT "
			"	characterID, characterName, typeID"
			" FROM character_"
			" WHERE characterID >= 140000000"))
		{
			_log(SERVICE__ERROR, "Error in LookupChars query: %s", res.error.c_str());
			return NULL;
		}
	} else {
		if(!m_db->RunQuery(res,
			"SELECT "
			"	characterID, characterName, typeID"
			" FROM character_"
			" WHERE characterName %s '%s'", 
			exact?"=":"RLIKE", matchEsc.c_str()
		))
		{
			_log(SERVICE__ERROR, "Error in LookupChars query: %s", res.error.c_str());
			return NULL;
		}
	}
	
	
	return(DBResultToRowset(res));
}

PyRepObject *LSCDB::LookupPlayerChars(const char *match, bool exact) {
	DBQueryResult res;

	std::string matchEsc;
	m_db->DoEscapeString(matchEsc, match);
	if(!m_db->RunQuery(res,
		"SELECT"
		" characterID, characterName, typeID"
		" FROM character_"
		" WHERE characterID >= 140000000"
		" AND characterName %s '%s'",
		exact?"=":"RLIKE", matchEsc.c_str()))
	{
		_log(DATABASE__ERROR, "Failed to lookup player char '%s': %s.", matchEsc.c_str(), res.error.c_str());
		return NULL;
	}

	return(DBResultToRowset(res));
}

//temporarily relocated into ServiceDB until some things get cleaned up...
uint32 LSCDB::StoreMail(uint32 senderID, uint32 recipID, const char * subject, const char * message, uint64 sentTime) {
	DBQueryResult res;
	DBerror err;
	DBResultRow row;

	std::string escaped;
	// Escape message header
	m_db->DoEscapeString(escaped, subject);

	// Store message header
	uint32 messageID;
	if (!m_db->RunQueryLID(err, messageID, 
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
	m_db->DoEscapeString(escaped, message);

	// Store message content
	if (!m_db->RunQuery(err,
		" INSERT INTO eveMailDetails "
		" (messageID, mimeTypeID, attachment) VALUES (%u, 1, '%s') ",
		messageID, escaped.c_str()
		))
	{
		codelog(SERVICE__ERROR, "Error in query, message content couldn't be saved: %s", err.c_str());
		// Delete message header
		if (!m_db->RunQuery(err, "DELETE FROM `eveMail` WHERE `messageID` = %u;", messageID))
		{
			codelog(SERVICE__ERROR, "Failed to remove invalid header data for messgae id %u: %s", messageID, err.c_str());
		}
		return (0);
	}


	return (messageID);
}

PyRepObject *LSCDB::GetMailHeaders(uint32 recID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT channelID, messageID, senderID, subject, created, `read` "
		" FROM eveMail "
		" WHERE channelID=%u", recID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}

	return(DBResultToRowset(res));
}

PyRep *LSCDB::GetMailDetails(uint32 messageID, uint32 readerID) {
	DBQueryResult result;
	DBResultRow row;

	//we need to query out the primary message here... not sure how to properly
	//grab the "main message" though... the text/plain clause is pretty hackish.
	if (!m_db->RunQuery(result,
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

	if (!m_db->RunQuery(err,
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

	if (!m_db->RunQuery(err,
		" DELETE FROM eveMail "
		" WHERE messageID=%u AND channelID=%u", messageID, readerID
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		ret = false;
	}
	if (!m_db->RunQuery(err,
		" DELETE FROM eveMailDetails "
		" WHERE messageID=%u", messageID
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		ret = false;
	}

	return ret;

}
PyRepObject *LSCDB::LookupCorporations(const std::string & search) {
	DBQueryResult res;
	std::string secure;
	m_db->DoEscapeString(secure, search);

	if (!m_db->RunQuery(res, 
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
PyRepObject *LSCDB::LookupFactions(const std::string & search) {
	DBQueryResult res;
	std::string secure;
	m_db->DoEscapeString(secure, search);

	if (!m_db->RunQuery(res, 
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
PyRepObject *LSCDB::LookupCorporationTickers(const std::string & search) {
	DBQueryResult res;
	std::string secure;
	m_db->DoEscapeString(secure, search);

	if (!m_db->RunQuery(res, 
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
PyRepObject *LSCDB::LookupStations(const std::string & search) {
	DBQueryResult res;
	std::string secure;
	m_db->DoEscapeString(secure, search);

	if (!m_db->RunQuery(res, 
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
PyRepObject *LSCDB::LookupLocationsByGroup(const std::string & search, uint32 typeID) {
	DBQueryResult res;
	std::string secure;
	m_db->DoEscapeString(secure, search);

	if (!m_db->RunQuery(res, 
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

void LSCDB::GetChannelNames(uint32 charID, std::vector<std::string> & names) {
	DBQueryResult res;

	if (!m_db->RunQuery(res, 
		" SELECT "
		"	character_.characterName, "
		"	corporation.corporationName, "
		"	entity.itemName as 'solarSystemName', "
		"	mapConstellations.constellationName, "
		"	mapRegions.regionName "
		" FROM character_ "
		"	LEFT JOIN corporation ON character_.corporationID = corporation.corporationID "
		"	LEFT JOIN entity ON character_.solarSystemID = entity.itemID "
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

std::string LSCDB::GetChannelName(uint32 id, const char * table, const char * column, const char * key) {
	DBQueryResult res;

	if (!m_db->RunQuery(res, 
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
