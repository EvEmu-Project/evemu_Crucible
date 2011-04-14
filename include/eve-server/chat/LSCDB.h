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


#ifndef __LSCDB_H_INCL__
#define __LSCDB_H_INCL__

#include "ServiceDB.h"

class LSCService;
class LSCChannel;

class LSCDB
: public ServiceDB
{
public:
	PyObject *LookupChars(const char *match, bool exact);
	PyObject *LookupOwners(const char *match, bool exact);
	PyObject *LookupPlayerChars(const char *match, bool exact);
	PyObject *LookupCorporations(const std::string &);
	PyObject *LookupFactions(const std::string &);
	PyObject *LookupCorporationTickers(const std::string &);
	PyObject *LookupStations(const std::string &);
	PyObject *LookupKnownLocationsByGroup(const std::string &, uint32);

	uint32 StoreMail(uint32 senderID, uint32 recipID, const char * subject, const char * message, uint64 sentTime);
	PyObject *GetMailHeaders(uint32 recID);
	PyRep *GetMailDetails(uint32 messageID, uint32 readerID);
	bool MarkMessageRead(uint32 messageID);
	bool DeleteMessage(uint32 messageID, uint32 readerID);
	void GetChannelNames(uint32 charID, std::vector<std::string> & names);

	uint32 GetNextAvailableChannelID();
	bool IsChannelNameAvailable(std::string name);
	bool IsChannelIDAvailable(uint32 channel_ID);
	bool IsChannelSubscribedByThisChar(uint32 char_ID, uint32 channel_ID);

	std::string GetRegionName(uint32 id) { return GetChannelName(id, "mapRegions", "regionName", "regionID"); }
	std::string GetConstellationName(uint32 id) { return GetChannelName(id, "mapConstellations", "constellationName", "constellationID"); }
	std::string GetSolarSystemName(uint32 id) { return GetChannelName(id, "mapSolarSystems", "solarSystemName", "solarSystemID"); }
	std::string GetCorporationName(uint32 id) { return GetChannelName(id, "corporation", "corporationName", "corporationID"); }
	std::string GetCharacterName(uint32 id) { return GetChannelName(id, "entity", "itemName", "itemID"); }

	void GetChannelInformation(std::string & name, uint32 & id,
		std::string & motd, uint32 & ownerid, std::string & compkey,
		bool & memberless, std::string & password, bool & maillist,
		uint32 & cspa, uint32 & temp, uint32 & mode);

	void GetChannelInformation(uint32 channelID, std::string & name,
		std::string & motd, uint32 & ownerid, std::string & compkey,
		bool & memberless, std::string & password, bool & maillist,
		uint32 & cspa, uint32 & temp, uint32 & mode);

    void GetChannelSubscriptions(uint32 charID, std::vector<unsigned long> & ids, std::vector<std::string> & names,
		std::vector<std::string> & MOTDs, std::vector<unsigned long> & ownerids, std::vector<std::string> & compkeys,
		std::vector<int> & memberless, std::vector<std::string> & passwords, std::vector<int> & maillists,
		std::vector<int> & cspas, std::vector<int> & temps, std::vector<int> & modes, int & channelCount);

	std::string GetChannelInfo(uint32 channelID, std::string & name, std::string & motd);

    uint32 GetChannelIDFromComparisonKey(std::string compkey);

    int WriteNewChannelSubscriptionToDatabase(uint32 characterID, uint32 channelID, uint32 corpID, uint32 allianceID, uint32 role, uint32 extra);
	int WriteNewChannelToDatabase(uint32 channelID, std::string name, uint32 ownerID, uint32 temporary, uint32 mode);

    int UpdateChannelConfigureInfo(LSCChannel * channel);

    int RemoveChannelSubscriptionFromDatabase(uint32 channelID, uint32 charID);
    int RemoveChannelFromDatabase(uint32 channelID);

protected:
	std::string GetChannelName(uint32 id, const char * table, const char * column, const char * key);
};





#endif


