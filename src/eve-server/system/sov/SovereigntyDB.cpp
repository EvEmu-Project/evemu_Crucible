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
    Author:        James
*/

#include "system/sov/SovereigntyDB.h"
#include "EVE_SovPosIntegrity.h"

    //TODO: For now, we return 0 as stationCount since such system is not implemented
    //TODO: Handle militaryPoints and industrialPoints, but for now they are 5 by default since we don't have a mechanism for determining them

void SovereigntyDB::GetSovereigntyData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
                            "SELECT mapSystemSovInfo.solarSystemID, mapSolarSystems.constellationID, mapSolarSystems.regionID, corporationID, "
                            " allianceID, claimStructureID, claimTime, "
                            " hubID, contested, 0 as stationCount, "
                            " 5 as militaryPoints, 5 as industrialPoints, claimID, beaconID, jammerID"
                            " FROM mapSystemSovInfo "
                            " INNER JOIN mapSolarSystems ON mapSolarSystems.solarSystemID=mapSystemSovInfo.solarSystemID"))
    {
        codelog(SOV__ERROR, "Error in query: %s", res.error.c_str());
    }
}

bool SovereigntyDB::GetSovereigntyDataForSystem(DBQueryResult& res, uint32 systemID)
{
    if (!sDatabase.RunQuery(res,
                            "SELECT mapSystemSovInfo.solarSystemID, mapSolarSystems.constellationID, mapSolarSystems.regionID, corporationID, "
                            " allianceID, claimStructureID, claimTime, "
                            " hubID, contested, 0 as stationCount, "
                            " 5 as militaryPoints, 5 as industrialPoints, claimID, beaconID, jammerID"
                            " FROM mapSystemSovInfo "
                            " INNER JOIN mapSolarSystems ON mapSolarSystems.solarSystemID=mapSystemSovInfo.solarSystemID"
                            " WHERE mapSystemSovInfo.solarSystemID=%u ", systemID))
    {
        codelog(SOV__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }
    return true;
}

bool SovereigntyDB::AddSovereigntyData(SovereigntyData data, uint32& claimID)
{
    DBerror err;
    claimID = 0;
    // claimTime is BIGINT FILETIME. %f expects a double and previously
    // caused undefined varargs behavior with GetFileTimeNow's integer.
    const long long timestamp = static_cast<long long>(
        EvESovPos::EffectiveClaimTime(data.claimTime, GetFileTimeNow()));
    if (!sDatabase.RunQueryLID(err, claimID,
                            "INSERT into mapSystemSovInfo (solarSystemID, corporationID, "
                            " allianceID, claimStructureID, claimTime, hubID, contested) "
                            " VALUES (%u, %u, %u, %u, %lli, %u, %u)", data.solarSystemID,
                            data.corporationID, data.allianceID, data.claimStructureID,
                            timestamp, data.hubID, data.contested)) {
        codelog(SOV__ERROR, "Error in adding new claim: %s", err.c_str());
        return false;
    }
    return true;
}

bool SovereigntyDB::RemoveSovereigntyData(uint32 systemID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
                            "DELETE from mapSystemSovInfo WHERE solarSystemID=%u", systemID))
    {
        codelog(SOV__ERROR, "Error in removing claim: %s", err.c_str());
        return false;
    }
    return true;
}

void SovereigntyDB::SetContested(uint32 systemID, bool contested) 
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
                            "UPDATE mapSystemSovInfo SET contested=%u WHERE solarSystemID=%u", int(contested), systemID))
    {
        codelog(SOV__ERROR, "Error in changing contested state: %s", err.c_str());
    }
}

void SovereigntyDB::SetHubID(uint32 systemID, uint32 hubID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
                            "UPDATE mapSystemSovInfo SET hubID=%u WHERE solarSystemID=%u", hubID, systemID))
    {
        codelog(SOV__ERROR, "Error in changing IHub ID: %s", err.c_str());
    }
}

void SovereigntyDB::SetJammerID(uint32 systemID, uint32 jammerID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
                            "UPDATE mapSystemSovInfo SET jammerID=%u WHERE solarSystemID=%u", jammerID, systemID))
    {
        codelog(SOV__ERROR, "Error in changing Jammer ID: %s", err.c_str());
    }
}

void SovereigntyDB::SetBeaconID(uint32 systemID, uint32 beaconID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
                            "UPDATE mapSystemSovInfo SET beaconID=%u WHERE solarSystemID=%u", beaconID, systemID))
    {
        codelog(SOV__ERROR, "Error in changing Beacon ID: %s", err.c_str());
    }
}

// Requires sql/migrations/20261118070000-sov-development-index-overrides.sql.
// A missing table is a failed test setup, not permission to mutate real
// sovereignty claimTime or silently claim the command succeeded.
bool SovereigntyDB::GetIndexOverrides(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
            "SELECT solarSystemID, claimStructureID, militaryPoints, "
            "industrialPoints, strategicDays "
            "FROM mapSovDevelopmentIndexOverrides")) {
        codelog(SOV__ERROR, "GetIndexOverrides query failed: %s", res.error.c_str());
        return false;
    }
    return true;
}

bool SovereigntyDB::SaveIndexOverride(uint32 systemID, uint32 claimStructureID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
            "INSERT INTO mapSovDevelopmentIndexOverrides "
            "(solarSystemID, claimStructureID, militaryPoints, industrialPoints, strategicDays) "
            "VALUES (%u, %u, 5, 5, 100) "
            "ON DUPLICATE KEY UPDATE "
            "claimStructureID=VALUES(claimStructureID), "
            "militaryPoints=5, industrialPoints=5, strategicDays=100",
            systemID, claimStructureID)) {
        codelog(SOV__ERROR, "SaveIndexOverride failed for %u: %s",
                systemID, err.c_str());
        return false;
    }
    return true;
}


bool SovereigntyDB::DeleteIndexOverride(uint32 systemID, uint32 claimStructureID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
            "DELETE FROM mapSovDevelopmentIndexOverrides "
            "WHERE solarSystemID=%u AND claimStructureID=%u",
            systemID, claimStructureID)) {
        codelog(SOV__ERROR, "DeleteIndexOverride failed for system %u: %s",
                systemID, err.c_str());
        return false;
    }
    return true;
}
