/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "StaticDataMgr.h"
#include "map/MapData.h"
#include "map/MapService.h"
#include "system/SystemManager.h"

PyCallable_Make_InnerDispatcher(MapService)

MapService::MapService(PyServiceMgr *mgr)
: PyService(mgr, "map"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(MapService, GetHistory);
    PyCallable_REG_CALL(MapService, GetBeaconCount);    //ColorStarsByCynosuralFields
    PyCallable_REG_CALL(MapService, GetStationCount);    //ColorStarsByStationCount
    PyCallable_REG_CALL(MapService, GetMyExtraMapInfo);     //ColorStarsByCorpMates
    PyCallable_REG_CALL(MapService, GetStationExtraInfo);
    PyCallable_REG_CALL(MapService, GetSolarSystemVisits);
    //PyCallable_REG_CALL(MapService, GetLinkableJumpArrays);
    PyCallable_REG_CALL(MapService, GetMyExtraMapInfoAgents);  //ColorStarsByMyAgents
    PyCallable_REG_CALL(MapService, GetSolarSystemPseudoSecurities);

    /**  not handled yet...these are empty calls  */
    PyCallable_REG_CALL(MapService, GetStuckSystems);
    PyCallable_REG_CALL(MapService, GetRecentSovActivity);
    PyCallable_REG_CALL(MapService, GetDeadspaceAgentsMap);
    PyCallable_REG_CALL(MapService, GetDeadspaceComplexMap);
    PyCallable_REG_CALL(MapService, GetIncursionGlobalReport);
    PyCallable_REG_CALL(MapService, GetSystemsInIncursions);    //ColorStarsByIncursions
    PyCallable_REG_CALL(MapService, GetSystemsInIncursionsGM);  //ColorStarsByIncursions
    PyCallable_REG_CALL(MapService, GetVictoryPoints);
    PyCallable_REG_CALL(MapService, GetAllianceJumpBridges);
    PyCallable_REG_CALL(MapService, GetAllianceBeacons);
    PyCallable_REG_CALL(MapService, GetCurrentSovData);
    // custom call for displaying all items in system
    PyCallable_REG_CALL(MapService, GetCurrentEntities);

}

MapService::~MapService()
{
    delete m_dispatch;
}

PyResult MapService::Handle_GetCurrentEntities(PyCallArgs &call)
{
    return call.client->SystemMgr()->GetCurrentEntities();
}

PyResult MapService::Handle_GetSolarSystemVisits(PyCallArgs &call)
{
    return MapDB::GetSolSystemVisits(call.client->GetCharacterID());
}

PyResult MapService::Handle_GetMyExtraMapInfoAgents(PyCallArgs &call)
{
    return StandingDB::GetMyStandings(call.client->GetCharacterID());
}

PyResult MapService::Handle_GetMyExtraMapInfo(PyCallArgs &call)
{
    return CharacterDB::GetMyCorpMates(call.client->GetCorporationID());
}

PyResult MapService::Handle_GetBeaconCount(PyCallArgs &call)
{
    return MapDB::GetDynamicData(2, 24);
}

PyResult MapService::Handle_GetStationExtraInfo(PyCallArgs &call)
{
    return sMapData.GetStationExtraInfo();
}

PyResult MapService::Handle_GetSolarSystemPseudoSecurities(PyCallArgs &call)
{
    // cant find a call to this in client (possible old call)
    return sMapData.GetPseudoSecurities();
}

PyResult MapService::Handle_GetStationCount(PyCallArgs &call)
{
    // cached on client side.  if cache is empty, this call is made.
    return sDataMgr.GetStationCount();
}

PyResult MapService::Handle_GetHistory(PyCallArgs &call) {
    uint8 int1 = PyRep::IntegerValueU32(call.tuple->GetItem(0));
    uint8 int2 = PyRep::IntegerValueU32(call.tuple->GetItem(1));
    if (is_log_enabled(SERVICE__CALLS))
        sLog.Cyan( "MapService::Handle_GetHistory()", "type: %i, timeframe: %i", int1, int2 );

    return MapDB::GetDynamicData(int1, int2);
}
/*
PyResult MapService::Handle_GetLinkableJumpArrays(PyCallArgs &call)
{   // working
    DBQueryResult res;
    PosMgrDB::GetLinkableJumpArrays(call.client->GetCorporationID(), res);
    PyList* list = new PyList();
    DBResultRow row;
    while (res.GetRow(row)) {
        // SELECT systemID, itemID
        PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyInt(row.GetInt(0)));
        tuple->SetItem(1, new PyInt(row.GetInt(1)));
        list->AddItem(tuple);
    }

    return list;
}*/



/** not handled */

PyResult MapService::Handle_GetAllianceJumpBridges(PyCallArgs &call)
{
    /**     bridgesByLocation = m.GetAllianceJumpBridges()
     *      for toLocID, fromLocID in bridgesByLocation:
     */
    sLog.Warning( "MapService::Handle_GetAllianceJumpBridges()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    DBQueryResult res;
    //PosMgrDB::GetAllianceJumpBridges(call.client->GetCorporationID(), res);
    PyList* list = new PyList();
    DBResultRow row;
    while (res.GetRow(row)) {
        // SELECT systemID, itemID
        PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyInt(row.GetInt(0)));
        tuple->SetItem(1, new PyInt(row.GetInt(1)));
        list->AddItem(tuple);
    }

    return list;
}

PyResult MapService::Handle_GetAllianceBeacons(PyCallArgs &call)
{/**
    beacons = sm.RemoteSvc('map').GetAllianceBeacons()
    for solarSystemID, structureID, structureTypeID in beacons:
        if solarSystemID != session.solarsystemid:
            solarsystem = cfg.evelocations.Get(solarSystemID)
            invType = cfg.invtypes.Get(structureTypeID)
            structureName = uiutil.MenuLabel('UI/Menusvc/BeaconLabel', {'name': invType.name,
            'system': solarSystemID})
            allianceMenu.append((solarsystem.name, (solarSystemID, structureID, structureName)))
            */
            sLog.Warning( "MapService::Handle_GetAllianceBeacons()", "size= %u", call.tuple->size() );
        call.Dump(SERVICE__CALL_DUMP);

    // copy format from GetLinkableJumpArrays()
    DBQueryResult res;
    //PosMgrDB::GetAllianceBeacons(call.client->GetCorporationID(), res);
    PyList* list = new PyList();
    DBResultRow row;
    while (res.GetRow(row)) {
        // SELECT solarSystemID, structureID, structureTypeID
        PyTuple* tuple = new PyTuple(3);
        tuple->SetItem(0, new PyInt(row.GetInt(0)));
        tuple->SetItem(1, new PyInt(row.GetInt(1)));
        tuple->SetItem(1, new PyInt(row.GetInt(2)));
        list->AddItem(tuple);
    }

    return list;
}

PyResult MapService::Handle_GetCurrentSovData(PyCallArgs &call)
{/**
    data = sm.RemoteSvc('map').GetCurrentSovData(constellationID)
    returns locationID, ?
    return sm.RemoteSvc('map').GetCurrentSovData(locationID)
    */
    sLog.Warning( "MapService::Handle_GetCurrentSovData()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return PyStatic.NewNone();
}
PyResult MapService::Handle_GetRecentSovActivity(PyCallArgs &call)
{
    /** @todo will have to make db table for this one.  */
    /*
     *    data = sm.RemoteSvc('map').GetRecentSovActivity()
     *    changes = []
     *    resultMap = {}
     *    toPrime = set()
     *    for item in data:
     *        if item.stationID is None:
     *            if bool(changeMode & mapcommon.SOV_CHANGES_SOV_GAIN) and item.oldOwnerID is None:
     *                changes.append((item.solarSystemID, 'UI/Map/StarModeHandler/sovereigntySovGained', (None, item.ownerID)))
     *                toPrime.add(item.ownerID)
     *            elif bool(changeMode & mapcommon.SOV_CHANGES_SOV_LOST) and item.ownerID is None:
     *                changes.append((item.solarSystemID, 'UI/Map/StarModeHandler/sovereigntySovLost', (item.oldOwnerID, None)))
     *                toPrime.add(item.oldOwnerID)
     *        elif bool(changeMode & mapcommon.SOV_CHANGES_SOV_GAIN) and item.oldOwnerID is None:
     *            changes.append((item.solarSystemID, 'UI/Map/StarModeHandler/sovereigntyNewOutpost', (None, item.ownerID)))
     *            toPrime.add(item.ownerID)
     *        elif bool(changeMode & mapcommon.SOV_CHANGES_SOV_GAIN) and item.ownerID is not None:
     *            changes.append((item.solarSystemID, 'UI/Map/StarModeHandler/sovereigntyConqueredOutpost', (item.ownerID, item.oldOwnerID)))
     *            toPrime.add(item.ownerID)
     *            toPrime.add(item.oldOwnerID)
     *
     */

    PyDict* result = new PyDict();

    return result;
}

//   DED Agent Site Report
PyResult MapService::Handle_GetDeadspaceAgentsMap(PyCallArgs &call)
{/* no packet data
        dungeons = sm.RemoteSvc('map').GetDeadspaceAgentsMap(eve.session.languageID)
        solarSystemID, dungeonID, difficulty, dungeonName = dungeons
*/
    PyRep *result = new PyDict();

    return result;
}

//  DED Deadspace Report
//22:37:54 L MapService::Handle_GetDeadspaceComplexMap(): size= 1
PyResult MapService::Handle_GetDeadspaceComplexMap(PyCallArgs &call)
{/* no packet data
        dungeons = sm.RemoteSvc('map').GetDeadspaceComplexMap(eve.session.languageID)
        solarSystemID, dungeonID, difficulty, dungeonName = dungeons

        get this data from managerDB.GetAnomalyList(DBQueryResult& res)
        res =  sysSignatures (sigID,sigItemID,dungeonType,sigName,systemID,sigTypeID,sigGroupID,scanGroupID,scanAttributeID,x,y,z)
*/
    sLog.Warning( "MapService::Handle_GetDeadspaceComplexMap()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    PyRep *result = new PyDict();

    return result;
}

PyResult MapService::Handle_GetSystemsInIncursions(PyCallArgs &call) {
    /**  EVE_Incursion.h
     *        participatingSystems = ms.GetSystemsInIncursions()
     *        for solarSystemID, sceneType in participatingSystems:
     *            -- (sceneType = staging, vanguard)
     */

    // copy format from GetLinkableJumpArrays()
    DBQueryResult res;
    //PosMgrDB::GetAllianceBeacons(call.client->GetCorporationID(), res);
    PyList* list = new PyList();
    DBResultRow row;
    while (res.GetRow(row)) {
        // SELECT systemID, sceneType
        PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyInt(row.GetInt(0)));   //solarSystemID
        tuple->SetItem(1, new PyInt(row.GetInt(1)));   //sceneType
        list->AddItem(tuple);
    }

    return list;
}

PyResult MapService::Handle_GetSystemsInIncursionsGM(PyCallArgs &call) {
    /**
     *        participatingSystems = ms.GetSystemsInIncursionsGM()
     *        for solarSystemID, sceneType in participatingSystems:
     *            -- (sceneType = staging, vanguard, assault, headquarters)
     */

    // copy format from GetLinkableJumpArrays()
    DBQueryResult res;
    //PosMgrDB::GetAllianceBeacons(call.client->GetCorporationID(), res);
    PyList* list = new PyList();
    DBResultRow row;
    while (res.GetRow(row)) {
        // SELECT systemID, sceneType
        PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyInt(row.GetInt(0)));    //solarSystemID
        tuple->SetItem(1, new PyInt(row.GetInt(1)));    //sceneType
        list->AddItem(tuple);
    }

    return list;
}

//05:52:07 L MapService::Handle_GetIncursionGlobalReport(): size= 0
PyResult MapService::Handle_GetIncursionGlobalReport(PyCallArgs &call) {
  /**
            report = sm.RemoteSvc('map').GetIncursionGlobalReport()
            rewardGroupIDs = [ r.rewardGroupID for r in report ]
            delayedRewards = sm.GetService('incursion').GetDelayedRewardsByGroupIDs(rewardGroupIDs)
            scrolllist = []
            factionsToPrime = set()
            for data in report:
                data.jumps = GetJumps(data.stagingSolarSystemID)
                data.influenceData = util.KeyVal(influence=data.influence, lastUpdated=data.lastUpdated, graceTime=data.graceTime, decayRate=data.decayRate)
                ssitem = map.GetItem(data.stagingSolarSystemID)
                data.stagingSolarSystemName = ssitem.itemName
                data.security = map.GetSecurityStatus(data.stagingSolarSystemID)
                data.constellationID = ssitem.locationID
                data.constellationName = map.GetItem(ssitem.locationID).itemName
                data.factionID = ssitem.factionID or starmap.GetAllianceSolarSystems().get(data.stagingSolarSystemID, None)
                factionsToPrime.add(data.factionID)
                rewards = delayedRewards.get(data.rewardGroupID, None)
                data.loyaltyPoints = rewards[0].rewardQuantity if rewards else 0
                scrolllist.append(listentry.Get('GlobalIncursionReportEntry', data))
*/
  /*
      [PySubStream 1128 bytes]
        [PyObjectData Name: objectCaching.CachedMethodCallResult]
          [PyTuple 3 items]
            [PyDict 1 kvp]
              [PyString "versionCheck"]
              [PyTuple 3 items]
                [PyNone]
                [PyNone]
                [PyString "15 minutes"]
            [PySubStream 1027 bytes]
              [PyList 7 items]
                [PyObjectData Name: util.KeyVal]
                  [PyDict 9 kvp]
                    [PyString "graceTime"]
                    [PyFloat 30]
                    [PyString "decayRate"]
                    [PyFloat 0.00999999977648258]
                    [PyString "influence"]
                    [PyFloat 0.0145000005140901]
                    [PyString "lastUpdated"]
                    [PyIntegerVar 129492976800000000]
                    [PyString "state"]
                    [PyInt 1]
                    [PyString "hasBoss"]
                    [PyInt 0]
                    [PyString "stagingSolarSystemID"]
                    [PyInt 30004323]
                    [PyString "rewardGroupID"]
                    [PyInt 192]
                    [PyString "taleID"]
                    [PyInt 192]
            [PyList 2 items]
              [PyIntegerVar 129493861959830226]
              [PyInt -950263469]
              */
    sLog.Warning( "MapService::Handle_GetIncursionGlobalReport()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return PyStatic.NewNone();
}

//   factional warfare shit
//https://wiki.eveonline.com/en/wiki/Victory_Points_and_Command_Bunker
PyResult MapService::Handle_GetVictoryPoints(PyCallArgs &call)
{/**           factionID, viewmode, solarsystemid, threshold, current in oldhistory.iteritems():
                 */
    sLog.Warning( "MapService::Handle_GetVictoryPoints()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return PyStatic.NewNone();
}


PyResult MapService::Handle_GetStuckSystems(PyCallArgs &call)
{
    // cant find a call to this in client (possible old call)
    sLog.Warning( "MapService::Handle_GetStuckSystems()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    uint8 none = 0;

    PyTuple* res = NULL;
    PyTuple* tuple0 = new PyTuple( 1 );

    tuple0->items[ 0 ] = new PyInt( none );

    res = tuple0;

    return res;
}

