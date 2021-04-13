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
    Rewrite:       Allan
    Updates:       James
*/

#include "Client.h"
#include "packets/Sovereignty.h"
#include "StaticDataMgr.h"
#include "system/sov/SovereigntyDB.h"

void SovereigntyDB::GetSovereigntyData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
                            "SELECT solarSystemID, mapSolarSystems.constellationID, corporationID, "
                            " allianceID, claimStructureID, claimTime, "
                            " hubID, contested, 0 as stationCount, "
                            " 5 as militaryPoints, 5 as industrialPoints "
                            " FROM mapSystemSovInfo "
                            " INNER JOIN mapSolarSystems ON mapSolarSystems.solarSystemID=mapSystemSovInfo.solarSystemID"))
    {
        codelog(SOV__ERROR, "Error in query: %s", res.error.c_str());
    }

/*
    Rsp_GetSystemSovereigntyInfo rsp;

    if (!res.GetRow(row)) //If we don't receive anything from above query, system is either empire or unclaimed
    {
        //Figure out if system is in empire space
        codelog(SOV__INFO, "SovereigntyDB::GetSystemSovInfo()", "No results found for %u in mapSystemSovInfo. Checking if unclaimed or empire system.", systemID);
        sDatabase.RunQuery(res, "SELECT factionID FROM mapSolarSystems WHERE solarSystemID = %u", systemID);

        if (res.GetRow(row))
        { //Faction-controlled system, assign values accordingly.
            args->SetItemString("contested", new PyInt(0));
            args->SetItemString("corporationID", new PyInt(0));
            args->SetItemString("claimTime", new PyLong(0));
            args->SetItemString("claimStructureID", new PyInt(0));
            args->SetItemString("hubID", new PyInt(0));
            args->SetItemString("allianceID", new PyInt(row.GetUInt(0)));
            args->SetItemString("solarSystemID", new PyInt(systemID));
        }
        else
        { //Unclaimed system, return None
            return PyStatic.NewNone();
        }
    }
    else
    {
        args->SetItemString("contested", new PyInt(row.GetUInt(6)));
        args->SetItemString("corporationID", new PyInt(row.GetUInt(1)));
        args->SetItemString("claimTime", new PyLong(row.GetInt64(4)));
        args->SetItemString("claimStructureID", new PyInt(row.GetUInt(3)));
        args->SetItemString("hubID", new PyInt(row.GetUInt(5)));
        args->SetItemString("allianceID", new PyInt(row.GetUInt(2)));
        args->SetItemString("solarSystemID", new PyInt(row.GetUInt(0)));
    }

    return new PyObject("util.KeyVal", args);*/

    /**
     *        sovInfo = sm.RemoteSvc('sovMgr').GetSystemSovereigntyInfo(session.solarsystemid2)
     *        ssAllianceID = sovInfo.allianceID if sovInfo else None
     *
     *        [PyTuple 1 items]
     *            [PySubStream 116 bytes]
     *                [PyObjectData Name: util.KeyVal]
     *                    [PyDict 7 kvp]
     *                        [PyString "contested"]            <<<< interger bool
     *                        [PyInt 0]
     *                        [PyString "corporationID"]
     *                        [PyInt 98049918]
     *                        [PyString "claimTime"]
     *                        [PyIntegerVar 129743663400000000]
     *                        [PyString "claimStructureID"]     <<<<  Territorial Claim Unit
     *                        [PyIntegerVar 1005712174146]
     *                        [PyString "hubID"]                <<<<  iHub (infrastructure hub)
     *                        [PyIntegerVar 1005900797500]
     *                        [PyString "allianceID"]
     *                        [PyInt 99000289]
     *                        [PyString "solarSystemID"]
     *                        [PyInt 30000302]
     */
}

//Just get the factionID or allianceID for this system
uint32 SovereigntyDB::GetSystemSovOwnerID(uint32 systemID)
{
    DBQueryResult res;
    DBResultRow row;

    PyDict *args = new PyDict;

    if (!sDatabase.RunQuery(res,
                            "SELECT"
                            "   allianceID,"
                            " FROM mapSystemSovInfo"
                            "  WHERE solarSystemID = %u",
                            systemID))
    {
        codelog(SOV__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    if (!res.GetRow(row)) //If we don't receive anything from above query, system is either empire or unclaimed
    {
        //Figure out if system is in empire space
        codelog(SOV__INFO, "SovereigntyDB::GetSystemSovOwnerID()", "No results found for %u in mapSystemSovInfo. Checking if unclaimed or empire system.", systemID);
        sDatabase.RunQuery(res, "SELECT factionID FROM mapSolarSystems WHERE solarSystemID = %u", systemID);

        if (res.GetRow(row))
        { //Return factionID for system
            return row.GetUInt(0);
        }
        else
        { //Unclaimed system, return 0 (should never return here as this function is only called when we know system is controlled by someone)
            return 0;
        }
    }
    else
    { //Return allianceID for system
        return row.GetUInt(0);
    }
}

PyRep *SovereigntyDB::GetAllianceSystems()
{
    DBQueryResult res;
    DBResultRow row;

    PyList *list = new PyList();

    if (!sDatabase.RunQuery(res,
                            "SELECT"
                            "   allianceID,"
                            "   solarSystemID "
                            " FROM mapSystemSovInfo"))
    {
        codelog(SOV__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    //Clunky way to do it
    /*while (res.GetRow(row)) {
        PyDict* dict = new PyDict();
        dict->SetItemString("allianceID", new PyInt(row.GetInt(0)));
        dict->SetItemString("solarSystemID", new PyInt(row.GetInt(1)));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }
    return list;*/
    return DBResultToPackedRowList(res);
}

PyRep *SovereigntyDB::GetCurrentSovData(uint32 locationID)
{
    //We get an arbitrary locationID, and need to get a list of solar system IDs along with allianceID
    //TODO: For now, we return 0 as stationCount since such system is not implemented
    //TODO: Handle militaryPoints and industrialPoints, but for now they are 5 by default since we don't have a mechanism for determining them

    DBQueryResult res;
    DBResultRow row;

    if (IsConstellation(locationID))
    { //Convert constellation into list of solar system IDs
        if (!sDatabase.RunQuery(res,
                                "SELECT mapSystemSovInfo.solarSystemID as locationID, mapSystemSovInfo.allianceID, 0 as stationCount, "
                                " 5 as militaryPoints, 5 as industrialPoints, mapSystemSovInfo.allianceID as claimedFor "
                                " FROM mapSystemSovInfo "
                                " INNER JOIN mapSolarSystems ON mapSolarSystems.solarSystemID=mapSystemSovInfo.solarSystemID "
                                " WHERE mapSolarSystems.constellationID=%u",
                                locationID))
        {
            codelog(SOV__ERROR, "Error in query: %s", res.error.c_str());
            return NULL;
        }
    }
    else if (IsSolarSystem(locationID))
    {
        if (!sDatabase.RunQuery(res,
                                "SELECT solarSystemID as locationID, allianceID, 0 as stationCount, "
                                " 0 as militaryPoints, 0 as industrialPoints, mapSystemSovInfo.allianceID as claimedFor "
                                " FROM mapSystemSovInfo "
                                " WHERE solarSystemID=%u",
                                locationID))
        {
            codelog(SOV__ERROR, "Error in query: %s", res.error.c_str());
            return NULL;
        }
    }

    return DBResultToCRowset(res);
}