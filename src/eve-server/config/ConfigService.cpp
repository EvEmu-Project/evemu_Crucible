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

#include "config/ConfigService.h"

ConfigService::ConfigService() :
    Service("config", eAccessLevel_LocationPreferred)
{
    this->Add("GetMultiOwnersEx", &ConfigService::GetMultiOwnersEx);
    this->Add("GetMultiLocationsEx", &ConfigService::GetMultiLocationsEx);
    this->Add("GetMultiStationEx", &ConfigService::GetMultiStationEx);
    this->Add("GetMultiAllianceShortNamesEx", &ConfigService::GetMultiAllianceShortNamesEx);
    this->Add("GetMultiCorpTickerNamesEx", &ConfigService::GetMultiCorpTickerNamesEx);
    this->Add("GetUnits", &ConfigService::GetUnits);
    this->Add("GetMap", &ConfigService::GetMap);
    this->Add("GetMapOffices", &ConfigService::GetMapOffices);
    this->Add("GetMapObjects", &ConfigService::GetMapObjects);
    this->Add("GetMapConnections", &ConfigService::GetMapConnections);
    this->Add("GetMultiGraphicsEx", &ConfigService::GetMultiGraphicsEx);
    this->Add("GetMultiInvTypesEx", &ConfigService::GetMultiInvTypesEx);
    this->Add("GetStationSolarSystemsByOwner", &ConfigService::GetStationSolarSystemsByOwner);
    this->Add("GetCelestialStatistic", &ConfigService::GetCelestialStatistic);
    this->Add("GetDynamicCelestials", &ConfigService::GetDynamicCelestials);
    this->Add("GetMapLandmarks", &ConfigService::GetMapLandmarks);
    this->Add("SetMapLandmarks", &ConfigService::SetMapLandmarks);
}

/** @todo put these next two in static data to avoid db hits  */
PyResult ConfigService::GetUnits(PyCallArgs &call) {
    return m_db.GetUnits();
}

PyResult ConfigService::GetMapLandmarks(PyCallArgs &call) {
    return m_db.GetMapLandmarks();
}

PyResult ConfigService::GetMultiOwnersEx(PyCallArgs &call, PyList* ownerIDs) {
  /*
23:14:21 L ConfigService: Handle_GetMultiOwnersEx
23:14:21 [SvcCall]   Call Arguments:
23:14:21 [SvcCall]       Tuple: 1 elements
23:14:21 [SvcCall]         [ 0] List: 1 elements
23:14:21 [SvcCall]         [ 0]   [ 0] Integer field: 140000053
  */
    _log(CACHE__DUMP, "ConfigService::Handle_GetMultiOwnersEx" );
    call.Dump(CACHE__DUMP);

    std::vector<int32> ints;

    PyList::const_iterator list_2_cur = ownerIDs->begin();
    for (size_t list_2_index(0); list_2_cur != ownerIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_SingleIntList failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        ints.push_back(t->value());
    }

    return m_db.GetMultiOwnersEx(ints);
}

PyResult ConfigService::GetMultiAllianceShortNamesEx(PyCallArgs &call, PyList* allianceIDs) {

    std::vector<int32> ints;

    PyList::const_iterator list_2_cur = allianceIDs->begin();
    for (size_t list_2_index(0); list_2_cur != allianceIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_SingleIntList failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        ints.push_back(t->value());
    }

    return m_db.GetMultiAllianceShortNamesEx(ints);
}


PyResult ConfigService::GetMultiLocationsEx(PyCallArgs &call, PyList* locationIDs) {      // now working correctly  -allan  25April
    _log(CACHE__DUMP,  "ConfigService::Handle_GetMultiLocationsEx" );
    call.Dump(CACHE__DUMP);

    std::vector<int32> ints;

    PyList::const_iterator list_2_cur = locationIDs->begin();
    for (size_t list_2_index(0); list_2_cur != locationIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_SingleIntList failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        ints.push_back(t->value());
    }

    return m_db.GetMultiLocationsEx(ints);
}

PyResult ConfigService::GetMultiStationEx(PyCallArgs &call, PyList* stationIDs) {
    _log(CACHE__DUMP,  "ConfigService::Handle_GetMultiStationEx" );
    call.Dump(CACHE__DUMP);

    std::vector<int32> ints;

    PyList::const_iterator list_2_cur = stationIDs->begin();
    for (size_t list_2_index(0); list_2_cur != stationIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_SingleIntList failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        ints.push_back(t->value());
    }

    return m_db.GetMultiStationEx(ints);
}

PyResult ConfigService::GetMultiCorpTickerNamesEx(PyCallArgs &call, PyList* corporationIDs) {
    std::vector<int32> ints;

    PyList::const_iterator list_2_cur = corporationIDs->begin();
    for (size_t list_2_index(0); list_2_cur != corporationIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_SingleIntList failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        ints.push_back(t->value());
    }

    return m_db.GetMultiCorpTickerNamesEx(ints);
}

PyResult ConfigService::GetMultiGraphicsEx(PyCallArgs &call, PyList* graphicIDs) {
    std::vector<int32> ints;

    PyList::const_iterator list_2_cur = graphicIDs->begin();
    for (size_t list_2_index(0); list_2_cur != graphicIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_SingleIntList failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        ints.push_back(t->value());
    }

    return m_db.GetMultiGraphicsEx(ints);
}

PyResult ConfigService::GetMap(PyCallArgs &call, PyInt* solarSystemID) {
    return m_db.GetMap(solarSystemID->value());
}

PyResult ConfigService::GetMapOffices(PyCallArgs &call, PyInt* solarSystemID) {
  /*
22:38:58 [SvcCall] Service config: calling GetMapOffices
22:38:58 [SvcCall]   Call Arguments:
22:38:58 [SvcCall]       Tuple: 1 elements
22:38:58 [SvcCall]         [ 0] Integer field: 30002507     -solarSystemID
22:38:58 [SvcCall]   Call Named Arguments:
22:38:58 [SvcCall]     Argument 'machoVersion':
22:38:58 [SvcCall]         Integer field: 1
  */
    return m_db.GetMapOffices(solarSystemID->value());
}

PyResult ConfigService::GetMapObjects(PyCallArgs &call, PyInt* systemID, PyInt* region, PyInt* constellation, PyInt* system, PyInt* station, PyInt* unknown) {
    return m_db.GetMapObjects( systemID->value(), region->value(), constellation->value(), system->value(), station->value());
}

PyResult ConfigService::GetMultiInvTypesEx(PyCallArgs &call, PyList* typeIDs) {
    _log(CACHE__DUMP,  "ConfigService::Handle_GetMultiInvTypesEx" );
    call.Dump(CACHE__DUMP);

    std::vector<int32> ints;

    PyList::const_iterator list_2_cur = typeIDs->begin();
    for (size_t list_2_index(0); list_2_cur != typeIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_SingleIntList failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        ints.push_back(t->value());
    }

    return m_db.GetMultiInvTypesEx(ints);
}


//02:10:35 L ConfigService::Handle_GetMapConnections(): size= 6
//15:12:56 W ConfigDB::GetMapConnections: DB query - System:20000307, B1:0, B2:0, B3:1, Cel:0, _c:1  <-- this means cached
PyResult ConfigService::GetMapConnections(PyCallArgs &call, PyInt* itemID, PyInt* reg, PyInt* con, PyInt* sol, PyInt* cel, PyInt* _c) {
/**
        this is cached on clientside.  only called if not in client cache
*/
    /** @todo check into id sending.... 9 is EvE Universe and 9000001 is EvE WormHole Universe */
    if (itemID->value() == 9 || sol->value()) {
        //sLog.Warning( "ConfigService::Handle_GetMapConnections()::args.id = 9 | args.sol");
        return m_db.GetMapConnections(call.client->GetSystemID(), sol->value(), reg->value(), con->value(), cel->value(), _c->value());
    } else {
        return m_db.GetMapConnections(itemID->value(), sol->value(), reg->value(), con->value(), cel->value(), _c->value());
    }
}

PyResult ConfigService::GetStationSolarSystemsByOwner(PyCallArgs &call, PyInt* ownerID) {
  // solorSys = sm.RemoteSvc('config').GetStationSolarSystemsByOwner(itemID)
  // solarSys.solarSystemID
    // this seems to ONLY return solarSystemIDs
    return m_db.GetStationSolarSystemsByOwner(ownerID->value());
}

PyResult ConfigService::GetCelestialStatistic(PyCallArgs &call, PyInt* celestialID) {
    return m_db.GetCelestialStatistic(celestialID->value());
}

PyResult ConfigService::GetDynamicCelestials(PyCallArgs &call, PyInt* locationID) {
    if (sDataMgr.IsSolarSystem(locationID->value())) {
        //sLog.Green("GetDynamicCelesitals", " IsSolarSystem %u", arg.arg);
        return m_db.GetDynamicCelestials(locationID->value());
    } else {
        sLog.Error("GetDynamicCelesitals", "!IsSolarSystem %u", locationID->value());
        return new PyInt( 0 );
    }
}

PyResult ConfigService::SetMapLandmarks(PyCallArgs &call, PyList* landmarkData) {
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

