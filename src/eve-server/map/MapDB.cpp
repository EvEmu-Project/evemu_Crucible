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
    Author:        Zhur
    Updates:    Allan
*/

#include "eve-server.h"

#include "map/MapDB.h"

PyObject *MapDB::GetPseudoSecurities() {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res, "SELECT solarSystemID, security FROM mapSolarSystems")) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

PyObject *MapDB::GetStationExtraInfo() {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   stationID,"
        "   solarSystemID,"
        "   operationID,"
        "   stationTypeID,"
        "   corporationID AS ownerID"
        " FROM staStations" )) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

PyObject *MapDB::GetStationOpServices() {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT operationID, serviceID FROM staOperationServices")) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

PyObject *MapDB::GetStationServiceInfo() {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT serviceID, serviceName FROM staServices ")) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

void MapDB::GetStationCount(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT map.solarSystemID, count(sta.stationID)"
        " FROM mapSolarSystems AS map"
        "  LEFT JOIN staStations AS sta USING(solarSystemID)"
        " GROUP BY map.solarSystemID"))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
    }
}

PyObject *MapDB::GetSolSystemVisits(uint32 charID)
{
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT"
        "   solarSystemID,"
        "   visits,"
        "   lastDateTime"
        " FROM chrVisitedSystems"
        " WHERE characterID = %u", charID ))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

// load all dynamic statistic data for system when booting.     -allan 5Aug19
//  systemMgr handles all calculations for jumps and kills and will call updates on a 15m timer
void MapDB::LoadDynamicData(uint32 sysID, SystemKillData& data)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT killsHour, kills24Hour, factionKills, factionKills24Hour, podKillsHour, podKills24Hour,"
        " killsDateTime, kills24DateTime, podDateTime, pod24DateTime, factionDateTime, faction24DateTime"
        " FROM mapDynamicData"
        " WHERE solarSystemID = %u", sysID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return;
    }
    DBResultRow row;
    if (res.GetRow(row)) {
        data.killsHour = row.GetInt(0);
        data.kills24Hour = row.GetInt(1);
        data.factionKills = row.GetInt(2);
        data.factionKills24Hour = row.GetInt(3);
        data.podKillsHour = row.GetInt(4);
        data.podKills24Hour = row.GetInt(5);
        data.killsDateTime = row.GetInt64(6);
        data.kills24DateTime = row.GetInt64(7);
        data.factionDateTime = row.GetInt64(8);
        data.faction24DateTime = row.GetInt64(9);
        data.podDateTime = row.GetInt64(10);
        data.pod24DateTime = row.GetInt64(11);
    } else {
        data = SystemKillData();
    }
}

//  called from MapService by multiple functions based on passed values.
///  added jumpsHour and numPilots data inserts.  16Mar14
///  added killsHour, factionKills, podKillsHour  24Mar14
///  NOTE: DB has fields for timing the *Hour and *24Hour parts. need to write checks for that once everything else is working.
///    NOTE:   use averages for *Hour based on current data and serverUpTime.   may be able to do 24Hour same way.
PyRep *MapDB::GetDynamicData(uint8 type, uint8 time) {
    DBQueryResult res;
    switch (type) {
        case 1: {
            sDatabase.RunQuery(res, "SELECT solarSystemID, jumpsHour AS value1 FROM mapDynamicData" );
        } break;
        case 2: {
            // cynos arent implemented yet, so this will always return 0
            sDatabase.RunQuery(res, "SELECT solarSystemID, moduleCnt, structureCnt FROM mapDynamicData WHERE active=1" );
            DBResultRow row;
            PyDict* dict = new PyDict();
            while (res.GetRow(row)) {
                PyTuple* inner = new PyTuple(2);
                    inner->SetItem(0, new PyInt(row.GetInt(1)));    // cyno modules on ships (fields)
                    inner->SetItem(1, new PyInt(row.GetInt(2)));    // cyno generators (POS structures)
                dict->SetItem(new PyInt(row.GetInt(0)), inner);
            }
            return dict;
        };
        case 3: {
            if (time == 1) {
                sDatabase.RunQuery(res, "SELECT solarSystemID, killsHour AS value1, factionKills AS value2, podKillsHour AS value3 FROM mapDynamicData" );
            } else if (time == 24) {
                sDatabase.RunQuery(res, "SELECT solarSystemID, kills24Hour AS value1, factionKills24Hour AS value2, podKills24Hour AS value3 FROM mapDynamicData" );
            }
        } break;
        case 4: {
            // not coded in client
        } break;
        case 5: {   //FacWarSvc.GetMostDangerousSystems
            sDatabase.RunQuery(res, "SELECT solarSystemID, killsHour AS value1, factionKills AS value2 FROM mapDynamicData" );
            //, kills24Hour AS value3, factionKills24Hour AS value4, podKills24Hour AS value5
        } break;
        default:
            return nullptr;
    }

    return DBResultToRowset(res);
}

// for MapData class
void MapDB::GetSystemJumps(DBQueryResult& res)
{
    //sDatabase.RunQuery(res, "SELECT ctype, fromreg, fromcon, fromsol, tosol, tocon, toreg FROM mapConnections");
    sDatabase.RunQuery(res, "SELECT ctype, fromsol, tosol FROM mapConnections");
}

// methods and queries for Solar System Status page -- updated/moved 26Nov18
void MapDB::SystemStartup()
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE mapDynamicData SET active = 0, jumpsHour = 0, pilotsDocked = 0, pilotsInSpace = 0, moduleCnt = 0, structureCnt = 0 WHERE 1");
}

/**
 * UPDATE: this is populated when db is created.  we are not deleting from mapDynamicData, but setting active as needed
 *   notes concerning previous system configuration removed
 */

void MapDB::SetSystemActive(uint32 sysID, bool active/*false*/)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE mapDynamicData SET active = %u WHERE solarSystemID = %u", active?1:0, sysID );
}

void MapDB::AddJump(uint32 sysID)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE mapDynamicData SET jumpsHour = jumpsHour + 1 WHERE solarSystemID = %u", sysID );
}

// these next 3 are updated to use systemMgr to manage dynamic data for it's system, and call these on a timer to update periodically
void MapDB::UpdatePilotCount(uint32 sysID, uint16 docked/*0*/, uint16 space/*0*/)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE mapDynamicData SET pilotsDocked = %u, pilotsInSpace = %u WHERE solarSystemID = %u",
                       docked, space, sysID );
}

void MapDB::UpdateJumps(uint32 sysID, uint16 jumps)
{
    //DBerror err;
    //sDatabase.RunQuery(err, "UPDATE mapDynamicData SET jumpsHour = %u WHERE solarSystemID = %u", jumps, sysID );
    sLog.Warning("MapDB::UpdateJumps", "UPDATE mapDynamicData SET jumpsHour = %u WHERE solarSystemID = %u", jumps, sysID);
}

void MapDB::UpdateKillData(uint32 sysID, SystemKillData& data)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE mapDynamicData SET killsHour = %u, kills24Hour = %u, factionKills = %u, factionKills24Hour = %u,"
    "  podKillsHour = %u, podKills24Hour = %u, killsDateTime = %li, kills24DateTime = %li, podDateTime = %li, pod24DateTime = %li,"
    "  factionDateTime = %li, faction24DateTime = %li",
    data.killsHour, data.kills24Hour, data.factionKills, data.factionKills24Hour, data.podKillsHour, data.podKills24Hour,
    data.killsDateTime, data.kills24DateTime, data.factionDateTime, data.faction24DateTime, data.podDateTime, data.pod24DateTime);
}

//  client logs faction kills in total kills.  return is value1(total kills) - value2(faction kills) > 0:
void MapDB::AddKill(uint32 sysID) {
    DBerror err;
    sDatabase.RunQuery(err,
            "UPDATE mapDynamicData SET killsHour = killsHour + 1, kills24Hour = kills24Hour + 1 WHERE solarSystemID = %u", sysID);
}

void MapDB::AddFactionKill(uint32 sysID) {
    DBerror err;
    sDatabase.RunQuery(err,
            "UPDATE mapDynamicData SET factionKills = factionKills + 1, factionKills24Hour = factionKills24Hour + 1 WHERE solarSystemID = %u", sysID);
}

void MapDB::AddPodKill(uint32 sysID) {
    DBerror err;
    sDatabase.RunQuery(err,
            "UPDATE mapDynamicData SET podKillsHour = podKillsHour + 1, podKills24Hour = podKills24Hour + 1 WHERE solarSystemID = %u", sysID);
}

// will need to write methods to retrieve/manipulate/set system dynamic data as systems may/may not be loaded

void MapDB::ManipulateTimeData()
{

}
