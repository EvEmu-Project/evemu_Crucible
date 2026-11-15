
 /**
  * @name AgentDB.cpp
  *   Agent DB operations
  *    original agent code by zhur, this was completely rewritten based on new data.
  *
  * @Author:        Allan
  * @date:      24 June 2018
  *
  */

/** @todo  fix this....not all agents have an entry in chrNPCCharacters table.  */

#include "eve-server.h"

#include "agents/AgentDB.h"
#include "StaticDataMgr.h"


bool AgentDB::LoadAgentData(uint32 agentID, AgentData& data)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   agt.agentTypeID,"
        "   agt.divisionID,"
        "   agt.level,"
        "   agt.quality,"
        "   agt.corporationID,"
        "   agt.locationID, "   //5
        "   agt.isLocator,"
        "   COALESCE(NULLIF(chr.solarSystemID,0), st.solarSystemID, itm.solarSystemID, 0) AS solarSystemID,"
        "   COALESCE(NULLIF(chr.stationID,0), st.stationID, 0) AS stationID,"
        "   COALESCE(chr.gender,0) AS gender,"
        "   COALESCE(bl.bloodlineID,0) AS bloodlineID,"    //10
        "   COALESCE(itm.typeID,0) AS typeID, "
        "   crp.friendID, "
        "   crp.enemyID, "
        "   crp.factionID, "
        "   chr.characterName " //15
        " FROM agtAgents AS agt"
        " LEFT JOIN chrNPCCharacters AS chr ON chr.characterID = agt.agentID"
        " LEFT JOIN staStations AS st ON st.stationID = agt.locationID"
        " LEFT JOIN crpNPCCorporations AS crp ON crp.corporationID = agt.corporationID"
        " LEFT JOIN bloodlineTypes AS bl ON bl.typeID = chr.typeID"
        " LEFT JOIN mapDenormalize AS itm ON itm.itemID = agt.locationID"
        " WHERE agt.agentID = %u", agentID))
    {
        codelog(DATABASE__ERROR, "Error in LoadAgentData query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (res.GetRow(row)) {
        // Optional NPC portrait/bloodline metadata must not prevent an
        // agent's core agtAgents identity from loading successfully.
        data.typeID         = row.GetUInt(0);
        data.divisionID     = row.GetUInt(1);
        data.level          = row.GetUInt(2);
        data.quality        = row.GetInt(3);
        data.corporationID  = row.GetUInt(4);
        data.locationID     = row.GetUInt(5);
        data.locator        = row.GetBool(6);
        data.solarSystemID  = row.GetUInt(7);
        data.stationID      = row.GetUInt(8);
        data.gender         = row.GetBool(9);
        data.bloodlineID    = row.GetUInt(10);
        data.locationTypeID = row.GetUInt(11);
        data.friendCorp     = (row.IsNull(12) ? 0 : row.GetInt(12));
        data.enemyCorp      = (row.IsNull(13) ? 0 : row.GetInt(13));
        data.factionID      = row.GetInt(14);
        data.raceID         = sDataMgr.GetFactionRace(data.factionID);
        data.name           = row.IsNull(15)
            ? ("Agent " + std::to_string(agentID)) : row.GetText(15);
        data.research       = (data.typeID == Agents::Type::Research);
        if (data.stationID == 0 && IsStationID(data.locationID))
            data.stationID = data.locationID;
        return true;
    }
    _log(AGENT__WARNING, "LoadAgentData: agent %u is not present in agtAgents.", agentID);
    return false;
}

void AgentDB::LoadAgentSkills(uint32 agentID, std::map< uint16, uint8 >& data)
{
    // SELECT typeID, level FROM agtSkillLevel WHERE agentID = %u
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT typeID, level FROM agtSkillLevel WHERE agentID = %u", agentID)) {
        codelog(DATABASE__ERROR, "Error in LoadAgentSkills query: %s", res.error.c_str());
        return;
    }

    DBResultRow row;
    while (res.GetRow(row))
        data[row.GetInt(0)] = row.GetInt(1);
}
