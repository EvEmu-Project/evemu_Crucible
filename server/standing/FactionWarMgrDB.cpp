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

FactionWarMgrDB::FactionWarMgrDB(DBcore *db)
: ServiceDB(db)
{
}

PyRep *FactionWarMgrDB::GetWarFactions() {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT factionID, militiaCorporationID"
		" FROM chrFactions"
		" WHERE militiaCorporationID IS NOT NULL"))
	{
		_log(DATABASE__ERROR, "Failed to query war factions: %s.");
		return(NULL);
	}

	return(DBResultToIntIntDict(res));
}

PyRep *FactionWarMgrDB::GetFacWarSystems() {
	_log(DATABASE__MESSAGE, "GetFacWarSystems unimplemented.");

	//fill some crap
	PyRepDict *result = new PyRepDict;
	PyRepDict *dict;

	dict = new PyRepDict;
	dict->add("occupierID", new PyRepInteger(500002));
	dict->add("factionID", new PyRepInteger(500002));
	result->add(new PyRepInteger(30002097), dict);

	return(result);
}

uint32 FactionWarMgrDB::GetFactionMilitiaCorporation(const uint32 factionID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT militiaCorporationID"
		" FROM chrFactions"
		" WHERE factionID=%lu",
		factionID))
	{
		_log(DATABASE__ERROR, "Failed to query militia corporation for faction %lu: %s.", factionID);
		return(NULL);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Faction %lu not found.", factionID);
		return(NULL);
	}

	return(row.GetUInt(0));
}

