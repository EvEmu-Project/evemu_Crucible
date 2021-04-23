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

    //TODO: For now, we return 0 as stationCount since such system is not implemented
    //TODO: Handle militaryPoints and industrialPoints, but for now they are 5 by default since we don't have a mechanism for determining them

void SovereigntyDB::GetSovereigntyData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
                            "SELECT mapSystemSovInfo.solarSystemID, mapSolarSystems.constellationID, mapSolarSystems.regionID, corporationID, "
                            " allianceID, claimStructureID, claimTime, "
                            " hubID, contested, 0 as stationCount, "
                            " 5 as militaryPoints, 5 as industrialPoints, claimID"
                            " FROM mapSystemSovInfo "
                            " INNER JOIN mapSolarSystems ON mapSolarSystems.solarSystemID=mapSystemSovInfo.solarSystemID"))
    {
        codelog(SOV__ERROR, "Error in query: %s", res.error.c_str());
    }
}

void SovereigntyDB::GetSovereigntyDataForSystem(DBQueryResult& res, uint32 systemID)
{
    if (!sDatabase.RunQuery(res,
                            "SELECT mapSystemSovInfo.solarSystemID, mapSolarSystems.constellationID, mapSolarSystems.regionID, corporationID, "
                            " allianceID, claimStructureID, claimTime, "
                            " hubID, contested, 0 as stationCount, "
                            " 5 as militaryPoints, 5 as industrialPoints, claimID"
                            " FROM mapSystemSovInfo "
                            " INNER JOIN mapSolarSystems ON mapSolarSystems.solarSystemID=mapSystemSovInfo.solarSystemID"
                            " WHERE mapSystemSovInfo.solarSystemID=%u ", systemID))
    {
        codelog(SOV__ERROR, "Error in query: %s", res.error.c_str());
    }
}

void SovereigntyDB::AddSovereigntyData(SovereigntyData data, uint32& claimID)
{
    DBerror err;
    if (!sDatabase.RunQueryLID(err, claimID,
                            "INSERT into mapSystemSovInfo (solarSystemID, corporationID, "
                            " allianceID, claimStructureID, claimTime, hubID, contested) "
                            " VALUES (%u, %u, %u, %u, %f, %u, %u)", data.solarSystemID, 
                            data.corporationID, data.allianceID, data.claimStructureID, 
                            GetFileTimeNow(), data.hubID, data.contested))
    {
        codelog(SOV__ERROR, "Error in adding new claim: %s", err.c_str());
    }
}

void SovereigntyDB::RemoveSovereigntyData(uint32 systemID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
                            "DELETE from mapSystemSovInfo WHERE solarSystemID=%u", systemID))
    {
        codelog(SOV__ERROR, "Error in removing claim: %s", err.c_str());
    }
}