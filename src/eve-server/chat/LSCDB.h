/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        Zhur, Aknor Jaden
*/


#ifndef __LSCDB_H_INCL__
#define __LSCDB_H_INCL__

#include "ServiceDB.h"

class Client;
class LSCService;
class LSCChannel;

class LSCDB
: public ServiceDB
{
public:
    uint32 StoreMail(uint32 senderID, uint32 recipID, const char * subject, const char * message, int64 sentTime);
    PyObject *GetMailHeaders(uint32 recID);
    PyRep *GetMailDetails(uint32 messageID, uint32 readerID);
    bool MarkMessageRead(uint32 messageID);
    bool DeleteMessage(uint32 messageID, uint32 readerID);
    void GetChannelNames(uint32 charID, std::vector<std::string> & names);

    int32 GetNextAvailableChannelID();
    bool IsChannelNameAvailable(std::string name);
    bool IsChannelIDAvailable(int32 channel_ID);
    bool IsChannelSubscribedByThisChar(uint32 char_ID, int32 channel_ID);

    std::string GetRegionName(uint32 id) { return GetChannelName(id, "mapRegions", "regionName", "regionID"); }
    std::string GetConstellationName(uint32 id) { return GetChannelName(id, "mapConstellations", "constellationName", "constellationID"); }
    std::string GetSolarSystemName(uint32 id) { return GetChannelName(id, "mapSolarSystems", "solarSystemName", "solarSystemID"); }
    std::string GetCorporationName(uint32 id) { return GetChannelName(id, "crpCorporation", "corporationName", "corporationID"); }
    std::string GetAllianceName(uint32 id) { return GetChannelName(id, "alnAlliance", "shortName", "allianceID"); }
    std::string GetCharacterName(uint32 id) { return GetChannelName(id, "chrCharacters", "characterName", "charID"); }

    int32 GetChannelID(std::string &name);

    void GetChannelInformation(int32 channelID, std::string & name,
        std::string & motd, uint32 & ownerid, std::string & compkey,
        bool & memberless, std::string & password, bool & maillist,
        uint32 & cspa);

    void GetChannelSubscriptions(uint32 charID, std::vector<long> & ids, std::vector<std::string> & names,
        std::vector<std::string> & MOTDs, std::vector<unsigned long> & ownerids, std::vector<std::string> & compkeys,
        std::vector<int> & memberless, std::vector<std::string> & passwords, std::vector<int> & maillists,
        std::vector<int> & cspas, int & channelCount);

    bool GetChannelInfo(int32 channelID, std::string& name, std::string& motd);

    int32 GetChannelIDFromComparisonKey(std::string compkey);

    void UpdateChannelInfo(LSCChannel *channel);
    void UpdateSubscription(int32 channelID, Client* pClient);

    void DeleteChannel(int32 channelID);
    void DeleteSubscription(int32 channelID, uint32 charID);


protected:
    std::string GetChannelName(uint32 id, const char * table, const char * column, const char * key);
};


#endif
