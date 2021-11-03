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

#include "PyServiceCD.h"
#include "config/ConfigService.h"

PyCallable_Make_InnerDispatcher(ConfigService)

ConfigService::ConfigService(PyServiceMgr *mgr)
: PyService(mgr, "config"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(ConfigService, GetMultiOwnersEx);
    PyCallable_REG_CALL(ConfigService, GetMultiLocationsEx);
    PyCallable_REG_CALL(ConfigService, GetMultiStationEx);
    PyCallable_REG_CALL(ConfigService, GetMultiAllianceShortNamesEx);
    PyCallable_REG_CALL(ConfigService, GetMultiCorpTickerNamesEx);
    PyCallable_REG_CALL(ConfigService, GetUnits);
    PyCallable_REG_CALL(ConfigService, GetMap);
    PyCallable_REG_CALL(ConfigService, GetMapOffices);
    PyCallable_REG_CALL(ConfigService, GetMapObjects);
    PyCallable_REG_CALL(ConfigService, GetMapConnections);
    PyCallable_REG_CALL(ConfigService, GetMultiGraphicsEx);
    PyCallable_REG_CALL(ConfigService, GetMultiInvTypesEx);
    PyCallable_REG_CALL(ConfigService, GetStationSolarSystemsByOwner);
    PyCallable_REG_CALL(ConfigService, GetCelestialStatistic);
    PyCallable_REG_CALL(ConfigService, GetDynamicCelestials);
    PyCallable_REG_CALL(ConfigService, GetMapLandmarks);
    PyCallable_REG_CALL(ConfigService, SetMapLandmarks);
}

ConfigService::~ConfigService() {
    delete m_dispatch;
}

/** @todo put these next two in static data to avoid db hits  */
PyResult ConfigService::Handle_GetUnits(PyCallArgs &call) {
    return m_db.GetUnits();
}

PyResult ConfigService::Handle_GetMapLandmarks(PyCallArgs &call) {
    return m_db.GetMapLandmarks();
}

PyResult ConfigService::Handle_GetMultiOwnersEx(PyCallArgs &call) {
  /*
23:14:21 L ConfigService: Handle_GetMultiOwnersEx
23:14:21 [SvcCall]   Call Arguments:
23:14:21 [SvcCall]       Tuple: 1 elements
23:14:21 [SvcCall]         [ 0] List: 1 elements
23:14:21 [SvcCall]         [ 0]   [ 0] Integer field: 140000053
  */
    _log(CACHE__DUMP, "ConfigService::Handle_GetMultiOwnersEx" );
    call.Dump(CACHE__DUMP);

    Call_SingleIntList arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetMultiOwnersEx(arg.ints);
}

PyResult ConfigService::Handle_GetMultiAllianceShortNamesEx(PyCallArgs &call) {
    Call_SingleIntList arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetMultiAllianceShortNamesEx(arg.ints);
}


PyResult ConfigService::Handle_GetMultiLocationsEx(PyCallArgs &call) {      // now working correctly  -allan  25April
    _log(CACHE__DUMP,  "ConfigService::Handle_GetMultiLocationsEx" );
    call.Dump(CACHE__DUMP);
    Call_SingleIntList arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetMultiLocationsEx(arg.ints);
}

PyResult ConfigService::Handle_GetMultiStationEx(PyCallArgs &call) {
    _log(CACHE__DUMP,  "ConfigService::Handle_GetMultiStationEx" );
    call.Dump(CACHE__DUMP);
    Call_SingleIntList arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetMultiStationEx(arg.ints);
}

PyResult ConfigService::Handle_GetMultiCorpTickerNamesEx(PyCallArgs &call) {
    Call_SingleIntList arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetMultiCorpTickerNamesEx(arg.ints);
}

PyResult ConfigService::Handle_GetMultiGraphicsEx(PyCallArgs &call) {
    Call_SingleIntList arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetMultiGraphicsEx(arg.ints);
}

PyResult ConfigService::Handle_GetMap(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetMap(args.arg);
}

PyResult ConfigService::Handle_GetMapOffices(PyCallArgs &call) {
  /*
22:38:58 [SvcCall] Service config: calling GetMapOffices
22:38:58 [SvcCall]   Call Arguments:
22:38:58 [SvcCall]       Tuple: 1 elements
22:38:58 [SvcCall]         [ 0] Integer field: 30002507     -solarSystemID
22:38:58 [SvcCall]   Call Named Arguments:
22:38:58 [SvcCall]     Argument 'machoVersion':
22:38:58 [SvcCall]         Integer field: 1
  */
  Call_SingleIntegerArg args;
  if (!args.Decode(&call.tuple)) {
      codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
      return nullptr;
  }

    return m_db.GetMapOffices(args.arg);
}

PyResult ConfigService::Handle_GetMapObjects(PyCallArgs &call) {
    Call_GetMapObjects args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetMapObjects( args.systemID, args.reg, args.con, args.sys, args.sta);
}

PyResult ConfigService::Handle_GetMultiInvTypesEx(PyCallArgs &call) {
    _log(CACHE__DUMP,  "ConfigService::Handle_GetMultiInvTypesEx" );
    call.Dump(CACHE__DUMP);

    //parse the PyRep to get the list of IDs to query.
    Call_SingleIntList arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetMultiInvTypesEx(arg.ints);
}


//02:10:35 L ConfigService::Handle_GetMapConnections(): size= 6
//15:12:56 W ConfigDB::GetMapConnections: DB query - System:20000307, B1:0, B2:0, B3:1, Cel:0, _c:1  <-- this means cached
PyResult ConfigService::Handle_GetMapConnections(PyCallArgs &call) {
/**
        this is cached on clientside.  only called if not in client cache
*/
    Call_GetMapConnections args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    /** @todo check into id sending.... 9 is EvE Universe and 9000001 is EvE WormHole Universe */
    if (args.id == 9 || args.sol) {
        //sLog.Warning( "ConfigService::Handle_GetMapConnections()::args.id = 9 | args.sol");
        return m_db.GetMapConnections(call.client->GetSystemID(), args.sol, args.reg, args.con, args.cel, args._c);
    } else {
        return m_db.GetMapConnections(args.id, args.sol, args.reg, args.con, args.cel, args._c);
    }
}

PyResult ConfigService::Handle_GetStationSolarSystemsByOwner(PyCallArgs &call) {
  // solorSys = sm.RemoteSvc('config').GetStationSolarSystemsByOwner(itemID)
  // solarSys.solarSystemID
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    // this seems to ONLY return solarSystemIDs
    return m_db.GetStationSolarSystemsByOwner(arg.arg);
}

PyResult ConfigService::Handle_GetCelestialStatistic(PyCallArgs &call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetCelestialStatistic(arg.arg);
}

PyResult ConfigService::Handle_GetDynamicCelestials(PyCallArgs &call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (sDataMgr.IsSolarSystem(arg.arg)) {
        //sLog.Green("GetDynamicCelesitals", " IsSolarSystem %u", arg.arg);
        return m_db.GetDynamicCelestials(arg.arg);
    } else {
        sLog.Error("GetDynamicCelesitals", "!IsSolarSystem %u", arg.arg);
        return new PyInt( 0 );
    }
}

PyResult ConfigService::Handle_SetMapLandmarks(PyCallArgs &call) {
  /**
            x, y, z = landmark.translation
            data = (landmark.landmarkID,
             x / STARMAP_SCALE,
             y / STARMAP_SCALE,
             z / STARMAP_SCALE,
             landmark.GetRadius())

        sm.RemoteSvc('config').SetMapLandmarks(landmarkData)
             */
    _log(CACHE__DUMP,  "MapService::Handle_SetMapLandmarks()");
    call.Dump(CACHE__DUMP);

    return nullptr;
}

