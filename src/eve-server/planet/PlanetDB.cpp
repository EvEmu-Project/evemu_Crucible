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
    Author:        Allan, based on original design by Comet0
*/

/*
 * PLANET__ERROR
 * PLANET__WARNING
 * PLANET__MESSAGE
 * PLANET__DEBUG
 * PLANET__INFO
 * PLANET__TRACE
 * PLANET__DUMP
 * PLANET__RES_DUMP
 * PLANET__GC_DUMP
 * PLANET__PKT_TRACE
 * PLANET__DB_ERROR
 * PLANET__DB_WARNING
 */


#include "planet/Colony.h"
#include "planet/PlanetDB.h"
#include "planet/PlanetDataMgr.h"


void PlanetDB::GetSchematicData(DBQueryResult& res)
{
    // load info into PIDataMgr
    if(!sDatabase.RunQuery(res, "SELECT schematicID, typeID, quantity, isInput FROM piTypeMap" ))
        _log(DATABASE__ERROR, "Error in GetSchematicData Query: %s", res.error.c_str());
}

void PlanetDB::GetSchematicTimes(DBQueryResult& res)
{
    // load info into PIDataMgr
    if(!sDatabase.RunQuery(res, "SELECT schematicID, cycleTime FROM piSchematics"))
        _log(DATABASE__ERROR, "Error in GetSchematicTimes Query: %s", res.error.c_str());
}

void PlanetDB::GetPlanetData(DBQueryResult& res)
{
    // load info into PlanetDataMgr
    if(!sDatabase.RunQuery(res,
        "SELECT planet.typeID AS planetTypeID,"
        " resource.typeID AS resourceID"
        " FROM invTypes planet, invTypes resource, dgmTypeAttributes dgm1, dgmTypeAttributes dgm2 "
        " WHERE dgm1.typeID = dgm2.typeID AND dgm1.attributeID = 1632 AND dgm1.valueFloat = planet.typeID AND dgm2.attributeID = 709"
        " AND dgm2.valueFloat = resource.typeID ORDER BY planet.typeID ")) {
        _log(DATABASE__ERROR, "Error in GetPlanetData Query: %s", res.error.c_str());
        }
}


PyRep* PlanetDB::GetPlanetsForChar(uint32 charID) {
  /** self.colonizationData = sm.RemoteSvc('planetMgr').GetPlanetsForChar()
        returns  (solarSystemID, planetID, typeID, numberOfPins)
    */
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT solarSystemID, planetID, typeID, numberOfPins"
        " FROM piPlanets WHERE charID = %u", charID)) {
        _log(DATABASE__ERROR, "Error in GetPlanetsForChar query: %s", res.error.c_str());
        return nullptr;
    }
    _log(DATABASE__RESULTS, "GetPlanetsForChar returned %u items", res.GetRowCount());
    return DBResultToCRowset(res);
}

void PlanetDB::AddPlanetForChar(uint32 solarSystemID, uint32 planetID, uint32 charID, uint32 ccPinID, uint16 typeID)
{
    DBerror err;
    if(!sDatabase.RunQuery(err,
        "INSERT INTO piPlanets (solarSystemID, planetID, charID, typeID, ccPinID)"
        " VALUES (%u, %u, %u, %u, %u)", solarSystemID, planetID, charID, typeID, ccPinID))
    {
        _log(DATABASE__ERROR, "AddPlanetForChar - Unable to add planet %u for char %u: %s", planetID, charID, err.GetError());
    }

}
void PlanetDB::UpdatePlanetPins(uint32 ccPinID, uint8 pins)
{
    DBerror err;
    if(!sDatabase.RunQuery(err, "UPDATE piPlanets SET numberOfPins = %u WHERE ccPinID = %u ", pins, ccPinID))
    {
        _log(DATABASE__ERROR, "UpdatePlanetPins - Unable to update ccPinID %u : %s", ccPinID, err.GetError());
    }
}

//  expired = not blue.os.GetWallclockTime() - launch.launchTime < const.piLaunchOrbitDecayTime (5d)
PyRep* PlanetDB::GetMyLaunchesDetails(uint32 charID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT itemID AS launchID, status, itemID, solarSystemID, planetID, launchTime, x, y, z"
        " FROM piLaunches WHERE charID = %u", charID)) {
        _log(DATABASE__ERROR, "Error in GetMyLaunchesDetails Query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToRowset(res);
}

GPoint PlanetDB::GetLaunchPos(uint32 launchID)
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT x,y,z FROM piLaunches WHERE launchID = %u", launchID)) {
        _log(DATABASE__ERROR, "Error in GetLaunchPos query: %s", res.error.c_str());
    }
    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Error in GetLaunchPos query: %s", res.error.c_str());
        return NULL_ORIGIN;
    }
    GPoint pos(row.GetDouble(0), row.GetDouble(1), row.GetDouble(2));
    return pos;
}

uint32 PlanetDB::GetLaunchPlanet(uint32 launchID)
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT planetID FROM piLaunches WHERE launchID = %u", launchID)) {
        _log(DATABASE__ERROR, "Error in GetLaunchPlanet query: %s", res.error.c_str());
    }
    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Error in GetLaunchPlanet query: %s", res.error.c_str());
        return 0;
    }

    return row.GetInt(0);
}

void PlanetDB::GetLaunchDetails(Launch::Data& data)
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT itemID, status, solarSystemID, planetID, launchTime, x, y, z FROM piLaunches WHERE launchID = %u", data.launchID)) {
        _log(DATABASE__ERROR, "Error in GetLaunchDetails query: %s", res.error.c_str());
    }
    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Error in GetLaunchDetails query: %s", res.error.c_str());
        return;
    }

    data.itemID = row.GetInt(0);
    data.status = row.GetInt(1);
    data.solarSystemID = row.GetInt(2);
    data.planetID = row.GetInt(3);
    data.launchTime = row.GetInt64(4);
    data.x = row.GetDouble(5);
    data.y = row.GetDouble(6);
    data.z = row.GetDouble(7);
}
uint32 PlanetDB::GetLaunchItemID(uint32 launchID)
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT itemID FROM piLaunches WHERE launchID = %u", launchID)) {
        _log(DATABASE__ERROR, "Error in GetLaunchItemID query: %s", res.error.c_str());
    }
    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Error in GetLaunchItemID query: %s", res.error.c_str());
        return 0;
    }

    return row.GetInt(0);
}

void PlanetDB::UpdateLaunchStatus(uint32 itemID, uint8 status)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE piLaunches SET status=%u WHERE itemID = %u", status, itemID);
}


void PlanetDB::GetExtractorsForPlanet(uint32 planetID, DBQueryResult& res)
{
    //SELECT ccPinID, ownerID, ecuID, headID, typeID, latitude, longitude FROM piECUHeads WHERE 1
    if (!sDatabase.RunQuery(res,
        "SELECT headID, typeID, ownerID, latitude, longitude"
        " FROM piECUHeads"
        " WHERE ccPinID IN"
        " (SELECT pinID FROM piCCPin"
        " WHERE planetID = %u)",
        planetID))
    {
        _log(DATABASE__ERROR, "Error in LoadPins Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "GetExtractorsForPlanet returned %u items", res.GetRowCount());
}

bool PlanetDB::LoadColony(uint32 charID, uint32 planetID, PI_CCPin* ccPin)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT pinID,level"
        " FROM piCCPin"
        " WHERE charID = %u"
        " AND planetID = %u",
        charID, planetID ))
    {
        _log(DATABASE__ERROR, "Error in LoadPins Query: %s", res.error.c_str());
        return false;
    }
    DBResultRow row;
    if (res.GetRow(row)) {
        ccPin->level = row.GetInt(1);
        ccPin->ccPinID = row.GetInt(0);
        return true;
    }
    return false;
}

void PlanetDB::LoadPins(uint32 ccPinID, std::map<uint32, PI_Pin>& pins)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT pinID, typeID, ownerID, state, level, latitude, longitude, "
        " isCommandCenter, isLaunchable, isProcess, isStorage, isECU,"
        " schematicID, programType, headRadius,"
        " launchTime, cycleTime, expiryTime, installTime, lastRunTime,"
        " hasReceivedInputs, receivedInputsLastCycle, qtyPerCycle"
        " FROM piPins"
        " WHERE ccPinID = %u", ccPinID))
    {
        _log(DATABASE__ERROR, "Error in LoadPins Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "LoadPins returned %u items", res.GetRowCount());

    DBResultRow row;
    while (res.GetRow(row)) {
        PI_Pin pin = PI_Pin();
            pin.typeID                  = row.GetInt(1);
            pin.ownerID                 = row.GetInt(2);
            pin.state                   = row.GetInt(3);
            pin.level                   = row.GetInt(4);
            pin.latitude                = row.GetFloat(5);
            pin.longitude               = row.GetFloat(6);
            pin.isCommandCenter         = row.GetBool(7);
            pin.isLaunchable            = row.GetBool(8);
            pin.isProcess               = row.GetBool(9);
            pin.isStorage               = row.GetBool(10);
            pin.isECU                   = row.GetBool(11);
            pin.schematicID             = row.GetInt(12);
            pin.programType             = row.GetInt(13);
            pin.headRadius              = row.GetFloat(14);
            pin.lastLaunchTime          = row.GetInt64(15);
            pin.cycleTime               = row.GetInt64(16);
            if (pin.isECU)              // ecu time is stored as filetime, but used as hours
                pin.cycleTime           /= EvE::Time::Hour;
            pin.expiryTime              = row.GetInt64(17);
            pin.installTime             = row.GetInt64(18);
            pin.lastRunTime             = row.GetInt64(19);
            pin.hasReceivedInputs       = row.GetBool(20);
            pin.receivedInputsLastCycle = row.GetBool(21);
            pin.qtyPerCycle             = row.GetInt(22);

        if (pin.isStorage or pin.isProcess)
            LoadContents(row.GetInt(0), pin.contents);

        if (pin.isECU)
            LoadHeads(row.GetInt(0), pin.heads);

        pins[row.GetInt(0)] = pin;
    }
}

void PlanetDB::LoadLinks(uint32 ccPinID, std::map<uint32, PI_Link >& links)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT linkID, level, state, endpoint1, endpoint2"
        " FROM piLinks WHERE ccPinID = %u",
        ccPinID ))
    {
        _log(DATABASE__ERROR, "Error in LoadPins Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "LoadLinks returned %u items", res.GetRowCount());

    DBResultRow row;
    while (res.GetRow(row)) {
        PI_Link link = PI_Link();
            link.level = row.GetInt(1);
            link.state = row.GetInt(2);
            link.typeID = 2280; // only link type in game
            link.endpoint1 = row.GetInt(3);
            link.endpoint2 = row.GetInt(4);
        links[row.GetInt(0)] = link;
    }
}

void PlanetDB::LoadRoutes(uint32 ccPinID, std::map<uint16, PI_Route >& routes)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT routeID, srcPinID, destPinID, state, priority, path, typeID, itemQty"
        " FROM piRoutes WHERE ccPinID = %u",
        ccPinID ))
    {
        _log(DATABASE__ERROR, "Error in LoadPins Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "LoadRoutes returned %u items", res.GetRowCount());

    DBResultRow row;
    std::string tempPath;
    while (res.GetRow(row)) {
        tempPath.clear();
        PI_Route route = PI_Route();
            route.srcPinID = row.GetInt(1);
            route.destPinID = row.GetInt(2);
            route.state = row.GetInt(3);
            route.priority = row.GetInt(4);
            route.commodityTypeID = row.GetInt(6);
            route.commodityQuantity = row.GetInt(7);

        tempPath = row.GetText(5);
        if (!tempPath.empty()) {
            // route string is not empty, so extract one number at a time until it is
            int pos = 0;
            std::string tempString = "";

            while ((pos = tempPath.find_first_of(':')) > 0 ) {
                tempString = tempPath.substr(0,pos);
                tempPath = tempPath.substr(pos+1,tempPath.length()-1);
                route.path.insert(route.path.end(), (atoi(tempString.c_str())));
            }
            // insert last pinID
            route.path.insert(route.path.end(), (atoi(tempPath.c_str())));
        }

        routes[row.GetInt(0)] = route;
    }
}

void PlanetDB::LoadContents(uint32 pinID, std::map<uint16, uint32>& contents)
{
    //SELECT ccPinID, pinID, typeID, itemQty FROM piPinContents
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT typeID, itemQty"
        " FROM piPinContents WHERE pinID = %u",
        pinID ))
    {
        _log(DATABASE__ERROR, "Error in LoadContents Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "LoadContents returned %u items", res.GetRowCount());

    DBResultRow row;
    while (res.GetRow(row)) {
        contents[row.GetInt(0)] = row.GetInt(1);
    }
}

void PlanetDB::LoadHeads(uint32 ecuID, std::map< uint16, PI_Heads >& heads)
{
    //SELECT ecuID, headID, typeID, latitude, longitude FROM piECUHeads
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT headID, typeID, latitude, longitude"
        " FROM piECUHeads WHERE ecuID = %u",
        ecuID ))
    {
        _log(DATABASE__ERROR, "Error in LoadHeads Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "LoadHeads returned %u items", res.GetRowCount());

    DBResultRow row;
    while (res.GetRow(row)) {
        PI_Heads head = PI_Heads();
            head.typeID = row.GetInt(1);
            head.ecuPinID = ecuID;
            head.latitude = row.GetDouble(2);
            head.longitude = row.GetDouble(3);
        heads[(uint16)row.GetInt(0)] = head;
    }
}

void PlanetDB::SaveLaunch(uint32 contID, uint32 charID, uint32 systemID, uint32 planetID, GPoint& pos)
{
    DBerror err;
    if(!sDatabase.RunQuery(err,
        "INSERT INTO piLaunches (itemID, status, charID, solarSystemID, planetID, launchTime, x, y, z) "
        " VALUES (%u, 0, %u, %u, %u, %f, %f, %f, %f)",
                           contID, charID, systemID, planetID, GetFileTimeNow(), pos.x, pos.y, pos.z))
        {
            _log(DATABASE__ERROR, "SaveLaunch - Unable to save Launch: %s", err.GetError());
        }
}

void PlanetDB::SaveCommandCenter(uint32 pinID, uint32 charID, uint32 planetID, uint32 typeID, double latitude, double longitude)
{
    DBerror err;
    if(!sDatabase.RunQuery(err,
        "INSERT INTO piCCPin (pinID, charID, planetID, typeID, latitude, longitude) "
        " VALUES (%u, %u, %u, %u, %f, %f)",
                           pinID, charID, planetID, typeID, latitude, longitude))
    {
        _log(DATABASE__ERROR, "SaveCommandCenter - Unable to save CommandCenter: %s", err.GetError());
    }
}

void PlanetDB::SaveCCLevel(uint32 pinID, uint8 level)
{
    DBerror err;
    if(!sDatabase.RunQuery(err, "UPDATE piCCPin SET level = %u WHERE pinID = %u", level, pinID))
        _log(DATABASE__ERROR, "SaveCCLevel - Unable to save CCLevel: %s", err.GetError());
    if(!sDatabase.RunQuery(err, "UPDATE piPins SET level = %u WHERE pinID = %u", level, pinID))
        _log(DATABASE__ERROR, "SaveCCLevel - Unable to save CCLevel: %s", err.GetError());

}

void PlanetDB::SavePins(PI_CCPin* ccPin)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO piPins";
    Inserts << " (ccPinID, pinID, typeID, ownerID, level, latitude, longitude,";
    Inserts << " isCommandCenter, isLaunchable, isProcess, isStorage, isECU,";
    Inserts << " schematicID, programType, headRadius, launchTime,";
    Inserts << " cycleTime, expiryTime, installTime, lastRunTime)";

    bool first = true;
    uint32 ccPinID = ccPin->ccPinID;
    for (auto cur : ccPin->pins) {
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else {
            Inserts << ", ";
        }
        Inserts << "(" << ccPinID << ", " << cur.first << ", " << cur.second.typeID << ", " << cur.second.ownerID << ", " << cur.second.level << ", " << cur.second.latitude << ", " << cur.second.longitude << ", ";
        Inserts << cur.second.isCommandCenter << ", " << cur.second.isLaunchable << ", " << cur.second.isProcess << ", " << cur.second.isStorage <<", " << cur.second.isECU << ", ";
        Inserts << cur.second.schematicID << ", " << cur.second.programType << ", " << cur.second.headRadius << ", " << cur.second.lastLaunchTime;
        Inserts << ", " << cur.second.cycleTime << ", " << cur.second.expiryTime << ", " << cur.second.installTime << ", " << cur.second.lastRunTime << ")";
    }

    if (!first) {
        // finish creating the command.
        Inserts << " ON DUPLICATE KEY UPDATE ";
        Inserts << " schematicID=VALUES(schematicID), ";
        Inserts << " programType=VALUES(programType), ";
        Inserts << " headRadius=VALUES(headRadius),";
        Inserts << " launchTime=VALUES(launchTime), ";
        Inserts << " cycleTime=VALUES(cycleTime),";
        Inserts << " expiryTime=VALUES(expiryTime), ";
        Inserts << " installTime=VALUES(installTime),";
        Inserts << " lastRunTime=VALUES(lastRunTime);";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SavePins - unable to save pins: %s", err.c_str());
    }
}

void PlanetDB::UpdatePins(uint32 pinID, PI_CCPin* ccPin)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO piPins";
    Inserts << " (ccPinID, pinID, schematicID, programType, launchTime,";
    Inserts << " cycleTime, installTime, lastRunTime, receivedInputsLastCycle, hasReceivedInputs)";

    bool first = true;
    if (pinID) {
        std::map<uint32, PI_Pin>::iterator itr;
        itr = ccPin->pins.find(pinID);
        if (itr != ccPin->pins.end()) {
            first = false;
            Inserts << " VALUES ";
            Inserts << "(" << ccPin->ccPinID << ", " << itr->first << ", "<< itr->second.schematicID << ", " << itr->second.programType;
            Inserts << ", " << itr->second.lastLaunchTime << ", " << itr->second.cycleTime << ", " << itr->second.installTime;
            Inserts  << ", " << itr->second.lastRunTime << ", " << itr->second.receivedInputsLastCycle << ", " << itr->second.hasReceivedInputs << ")";
        }
    } else {
        for (auto cur : ccPin->pins) {
            if (first) {
                Inserts << " VALUES ";
                first = false;
            } else {
                Inserts << ", ";
            }
            Inserts << "(" << ccPin->ccPinID << ", " << cur.first << ", "<< cur.second.schematicID << ", " << cur.second.programType;
            Inserts << ", " << cur.second.lastLaunchTime << ", " << cur.second.cycleTime << ", " << cur.second.installTime;
            Inserts << ", " << cur.second.lastRunTime << ", " << cur.second.receivedInputsLastCycle << ", " << cur.second.hasReceivedInputs << ")";
        }
    }

    if (!first) {
        // finish creating the command.
        Inserts << " ON DUPLICATE KEY UPDATE ";
        Inserts << " schematicID=VALUES(schematicID), ";
        Inserts << " programType=VALUES(programType), ";
        Inserts << " launchTime=VALUES(launchTime), ";
        Inserts << " cycleTime=VALUES(cycleTime),";
        Inserts << " installTime=VALUES(installTime),";
        Inserts << " lastRunTime=VALUES(lastRunTime),";
        Inserts << " hasReceivedInputs=VALUES(hasReceivedInputs),";
        Inserts << " receivedInputsLastCycle=VALUES(receivedInputsLastCycle);";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SavePins - unable to save pins: %s", err.c_str());
    }
}

void PlanetDB::UpdateECUPin(uint32 pinID, PI_CCPin* ccPin)
{
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(pinID);
    if (itr == ccPin->pins.end()) {
        _log(PLANET__ERROR, "PlanetDB::UpdateECUPin() - pinID %u not found in ccPin.pins map", pinID);
        return;
    }

    DBerror err;
    if(!sDatabase.RunQuery(err,
        "UPDATE piPins SET"
        "  programType = %u,"
        "  headRadius = %f,"
        "  qtyPerCycle = %u,"
        "  schematicID = %u,"
        "  expiryTime = %li,"
        "  cycleTime = %li,"
        "  installTime = %li,"
        "  lastRunTime = %li"
        " WHERE pinID = %u",
        itr->second.programType, itr->second.headRadius, itr->second.qtyPerCycle, itr->second.schematicID, itr->second.expiryTime,
        itr->second.cycleTime, itr->second.installTime, itr->second.lastRunTime, pinID))
    {
        _log(DATABASE__ERROR, "UpdateECUPin - Unable to update pin: %s", err.GetError());
    }
}

void PlanetDB::UpdatePinTimes(PI_CCPin* ccPin)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO piPins";
    Inserts << " (pinID, launchTime, installTime, lastRunTime)";

    bool first = true;
    for (auto cur : ccPin->pins) {
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else {
            Inserts << ", ";
        }
        Inserts << "(" << cur.first << ", " << cur.second.lastLaunchTime << ", ";
        Inserts << cur.second.installTime << ", " << cur.second.lastRunTime << ")";
    }

    if (!first) {
        // finish creating the command.
        Inserts << " ON DUPLICATE KEY UPDATE ";
        Inserts << " launchTime=VALUES(launchTime),";
        Inserts << " installTime=VALUES(installTime),";
        Inserts << " lastRunTime=VALUES(lastRunTime);";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SavePins - unable to save pins: %s", err.c_str());
    }
}

void PlanetDB::SaveHeads(uint32 ccPinID, uint32 ownerID, uint32 ecuID, std::map< uint16, PI_Heads >& heads)
{
    if (heads.empty())
        return;

    DBerror err;
    for (auto cur : heads) {
        // save the head data separately
        if (!sDatabase.RunQuery(err,
            "INSERT INTO piECUHeads (ccPinID, ownerID, ecuID, headID, typeID, latitude, longitude)"
            " VALUES (%u, %u, %u, %u, %u, %f, %f)"
            " ON DUPLICATE KEY UPDATE "
            " typeID=VALUES(typeID),"
            " latitude=VALUES(latitude),"
            " longitude=VALUES(longitude)",
            ccPinID, ownerID, ecuID, cur.first, cur.second.typeID, cur.second.latitude, cur.second.longitude))
        {
            _log(DATABASE__ERROR, "SaveHeads - Unable to save heads: %s", err.GetError());
        }
    }
}

void PlanetDB::SavePinLevel(uint32 pinID, uint8 level)
{
    DBerror err;
    if(!sDatabase.RunQuery(err, "UPDATE piPins SET level = %u WHERE pinID = %u", level, pinID))
    {
        _log(DATABASE__ERROR, "SavePinLevel - Unable to save PinLevel: %s", err.GetError());
    }
}

void PlanetDB::SaveLinks(PI_CCPin* ccPin)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO piLinks";
    Inserts << " (ccPinID, linkID, level, endpoint1, endpoint2)";

    bool first = true;
    uint32 ccPinID = ccPin->ccPinID;
    for (auto cur : ccPin->links) {
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else {
            Inserts << ", ";
        }
        Inserts << "(" << ccPinID << ", " << cur.first << ", " << cur.second.level << ", " << cur.second.endpoint1 << ", " << cur.second.endpoint2 << ")";
    }

    if (!first) {
        // finish creating the command.
        Inserts << " ON DUPLICATE KEY UPDATE";
        Inserts << " level=VALUES(level), ";
        Inserts << " endpoint1=VALUES(endpoint1), ";
        Inserts << " endpoint2=VALUES(endpoint2)";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveLinks - unable to save links - %s", err.c_str());
    }
}

void PlanetDB::SaveLinkLevel(uint32 linkID, uint8 level)
{
    DBerror err;
    if(!sDatabase.RunQuery(err, "UPDATE piLinks SET level = %u WHERE linkID = %u", level, linkID))
    {
        _log(DATABASE__ERROR, "SaveLinkLevel - Unable to save LinkLevel: %s", err.GetError());
    }
}

uint16 PlanetDB::SaveRoute(uint32 ccPinID, PI_Route& route)
{
    DBerror err;
    uint32 routeID = 0;
    std::string path;
    path.clear();
    std::list<uint32>::iterator itr = route.path.begin();
    while (itr != route.path.end()) {
        path += std::to_string(*itr);
        if (++itr != route.path.end())
            path += ":";
    }
    if (!sDatabase.RunQueryLID(err, routeID,
        "INSERT INTO `piRoutes`(`ccPinID`, `srcPinID`, `destPinID`, `state`, `priority`, `path`, `typeID`, `itemQty`) "
        " VALUES (%u, %u, %u, %u, %u, '%s', %u, %u)",
        ccPinID, route.srcPinID, route.destPinID, route.state, route.priority, path.c_str(), route.commodityTypeID, route.commodityQuantity))
    {
        _log(DATABASE__ERROR, "SaveHeads - Unable to save heads: %s", err.GetError());
    }
    return (uint16)routeID;
}

void PlanetDB::SaveRoutes(PI_CCPin* ccPin)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO piRoutes";
    Inserts << " (ccPinID, routeID, srcPinID, destPinID, path, typeID, itemQty)";

    bool first = true;
    std::string path;
    std::list<uint32>::iterator itr;
    uint32 ccPinID = ccPin->ccPinID;
    for (auto cur : ccPin->routes) {
        path.clear();
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else {
            Inserts << ", ";
        }
        itr = cur.second.path.begin();
        while (itr != cur.second.path.end()) {
            path += std::to_string(*itr);
            if (++itr != cur.second.path.end())
                path += ":";
        }
        Inserts << "(" << ccPinID << ", " << cur.first << ", " << cur.second.srcPinID << ", " << cur.second.destPinID << ", '" << path << "', ";
        Inserts << cur.second.commodityTypeID << ", " << cur.second.commodityQuantity << ")";
    }

    if (!first) {
        // finish creating the command.
        Inserts << " ON DUPLICATE KEY UPDATE";
        Inserts << " path=VALUES(path),";
        Inserts << " srcPinID=VALUES(srcPinID),";
        Inserts << " destPinID=VALUES(destPinID),";
        Inserts << " typeID=VALUES(typeID),";
        Inserts << " itemQty=VALUES(itemQty);";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveRoutes - unable to save route - %s", err.c_str());
    }
}

void PlanetDB::SaveContents(PI_CCPin* ccPin)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO piPinContents";
    Inserts << " (ccPinID, pinID, typeID, itemQty)";

    bool first = true;
    uint32 ccPinID = ccPin->ccPinID;
    std::map<uint16, uint32>::iterator itr;
    for (auto cur : ccPin->pins) {
        if (cur.second.isStorage) {
            for (itr = cur.second.contents.begin(); itr != cur.second.contents.end(); ++itr) {
                if (first) {
                    Inserts << " VALUES ";
                    first = false;
                } else {
                    Inserts << ", ";
                }
                Inserts << "(" << ccPinID << ", " << cur.first << ", " << itr->first << ", " << itr->second << ")";
            }
        }
    }
    if (!first) {
        // finish creating the command.
        Inserts << " ON DUPLICATE KEY UPDATE";
        Inserts << " itemQty=VALUES(itemQty);";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveContents - unable to save contents - %s", err.c_str());
    }
}

void PlanetDB::SavePinContents(uint32 ccPinID, uint32 pinID, std::map< uint16, uint32 >& contents)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO piPinContents";
    Inserts << " (ccPinID, pinID, typeID, itemQty)";

    bool first = true;
    std::map<uint16, uint32>::iterator itr;
    for (itr = contents.begin(); itr != contents.end(); ++itr) {
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else {
            Inserts << ", ";
        }
        Inserts << "(" << ccPinID << ", " << pinID << ", " << itr->first << ", " << itr->second << ")";
    }
    if (!first) {
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SavePinContents - unable to save contents - %s", err.c_str());
    }
}

void PlanetDB::RemovePin(uint32 pinID)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM piPins WHERE pinID = %u", pinID);
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE itemID = %u", pinID);
    sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID = %u", pinID);
}

void PlanetDB::RemoveHead(uint32 ecuID, uint32 headID)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM piECUHeads WHERE ecuID = %u AND headID = %u", ecuID, headID);
}

void PlanetDB::RemoveLink(uint32 linkID)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM piLinks WHERE linkID = %u", linkID);
}

void PlanetDB::RemoveRoute(uint16 routeID)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM piRoutes WHERE routeID = %u", routeID);
}

void PlanetDB::RemoveContents(uint32 pinID)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM piPinContents WHERE pinID = %u", pinID);
}

void PlanetDB::DeleteLaunch(uint32 contID)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM piLaunches WHERE itemID = %u", contID);
}

void PlanetDB::DeleteColony(uint32 ccPinID, uint32 planetID, uint32 charID)
{
    /** @todo  remove items from entity* table... */
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE locationID = %u AND ownerID = %u", planetID, charID);
    sDatabase.RunQuery(err, "DELETE FROM piCCPin WHERE pinID = %u", ccPinID);
    sDatabase.RunQuery(err, "DELETE FROM piPins WHERE ccPinID = %u", ccPinID);
    sDatabase.RunQuery(err, "DELETE FROM piLinks WHERE ccPinID = %u", ccPinID);
    sDatabase.RunQuery(err, "DELETE FROM piRoutes WHERE ccPinID = %u", ccPinID);
    sDatabase.RunQuery(err, "DELETE FROM piPlanets WHERE ccPinID = %u", ccPinID);
    sDatabase.RunQuery(err, "DELETE FROM piECUHeads WHERE ccPinID = %u", ccPinID);
    sDatabase.RunQuery(err, "DELETE FROM piPinContents WHERE ccPinID = %u", ccPinID);
}
