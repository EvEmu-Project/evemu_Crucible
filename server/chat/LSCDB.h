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


#ifndef __LSCDB_H_INCL__
#define __LSCDB_H_INCL__

#include "../ServiceDB.h"
#include "../common/PyRep.h"
#include <vector>
#include <string>

class LSCService;
class LSCChannel;

class LSCDB
: public ServiceDB {
public:
	LSCDB(DBcore *db);
	virtual ~LSCDB();
	
	PyRepObject *LookupChars(const char *match, bool exact);
	PyRepObject *LookupPlayerChars(const char *match, bool exact);
	PyRepObject *LookupCorporations(const std::string &);
	PyRepObject *LookupFactions(const std::string &);
	PyRepObject *LookupCorporationTickers(const std::string &);
	PyRepObject *LookupStations(const std::string &);
	PyRepObject *LookupLocationsByGroup(const std::string &, uint32);

	uint32 StoreMail(uint32 senderID, uint32 recipID, const char * subject, const char * message, uint64 sentTime);
	PyRepObject *GetMailHeaders(uint32 recID);
	PyRep *GetMailDetails(uint32 messageID, uint32 readerID);
	bool MarkMessageRead(uint32 messageID);
	bool DeleteMessage(uint32 messageID, uint32 readerID);
	void GetChannelNames(uint32 charID, std::vector<std::string> & names);

	std::string GetRegionName(uint32 id) { return GetChannelName(id, "mapRegions", "regionName", "regionID"); }
	std::string GetConstellationName(uint32 id) { return GetChannelName(id, "mapConstellations", "constellationName", "constellationID"); }
	std::string GetSolarSystemName(uint32 id) { return GetChannelName(id, "entity", "itemName", "itemID"); }
	std::string GetCorporationName(uint32 id) { return GetChannelName(id, "corporation", "corporationName", "corporationID"); }
	std::string GetChatacterName(uint32 id) { return GetChannelName(id, "character_", "characterName", "characterID"); }

protected:
	std::string GetChannelName(uint32 id, const char * table, const char * column, const char * key);
};





#endif


