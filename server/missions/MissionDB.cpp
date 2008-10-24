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

MissionDB::MissionDB(DBcore *db)
: ServiceDB(db)
{
}

MissionDB::~MissionDB() {
}

PyRepObject *MissionDB::GetAgents() {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT agt.agentID,agt.agentTypeID,agt.divisionID,agt.level,agt.stationID,"
		"	agt.quality,agt.corporationID,chr.bloodlineID,chr.gender"
		" FROM agtAgents AS agt"
		" LEFT JOIN character_ AS chr ON chr.characterID=agt.agentID"
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

#ifdef NOT_DONE
AgentLevel *MissionDB::LoadAgentLevel(uint8 level) {
	AgentLevel *result = new AgentLevel;
	
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT missionID,missionName,missionLevel,"
		"	agtMissions.missionTypeID,missionTypeName,"
		"	importantMission"
		" FROM agtMissions"
		"	NATURAL JOIN agtMissionTypes"
		" WHERE missionLevel=%d",
		level
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		delete result;
		return(NULL);
	}

	std::vector<uint32> IDs;
	DBResultRow row;

	IDs.clear();
	while(res.GetRow(row)) {
		AgentMissionSpec *spec = new AgentMissionSpec;
		spec->missionID = row.GetUInt(0);
		spec->missionName = row.GetText(1);
		spec->missionLevel = row.GetUInt(2);
		spec->missionTypeID = row.GetUInt(3);
		spec->missionTypeName = row.GetText(1);
		spec->importantMission = (row.GetUInt(2)==0)?false:true;
		result->missions.push_back(spec);
	}
	
}
#endif
























