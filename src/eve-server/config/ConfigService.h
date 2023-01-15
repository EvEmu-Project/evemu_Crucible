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
    Author:        Zhur, Allan
*/


#ifndef __CONFIG_SERVICE_H_INCL__
#define __CONFIG_SERVICE_H_INCL__

#include "config/ConfigDB.h"
#include "services/Service.h"
#include "Client.h"

class ConfigService : public Service <ConfigService>
{
public:
    ConfigService();

protected:
    ConfigDB m_db;

    PyResult GetUnits(PyCallArgs& call);
    PyResult GetMapLandmarks(PyCallArgs& call);
    PyResult GetMultiOwnersEx(PyCallArgs& call, PyList* ownerIDs);
    PyResult GetMultiAllianceShortNamesEx(PyCallArgs& call, PyList* allianceIDs);
    PyResult GetMultiLocationsEx(PyCallArgs& call, PyList* locationIDs);
    PyResult GetMultiStationEx(PyCallArgs& call, PyList* stationIDs);
    PyResult GetMultiCorpTickerNamesEx(PyCallArgs& call, PyList* corporationIDs);
    PyResult GetMultiGraphicsEx(PyCallArgs& call, PyList* graphicIDs);
    PyResult GetMap(PyCallArgs& call, PyInt* solarSystemID);
    PyResult GetMapOffices(PyCallArgs& call, PyInt* solarSystemID);
    PyResult GetMapObjects(PyCallArgs& call, PyInt* systemID, PyInt* region, PyInt* constellation, PyInt* system, PyInt* station, PyInt* unknown);
    PyResult GetMultiInvTypesEx(PyCallArgs& call, PyList* typeIDs);
    PyResult GetMapConnections(PyCallArgs& call, PyInt* itemID, PyInt* reg, PyInt* con, PyInt* sol, PyInt* cel, PyInt* _c);
    PyResult GetStationSolarSystemsByOwner(PyCallArgs& call, PyInt* ownerID);
    PyResult GetCelestialStatistic(PyCallArgs& call, PyInt* celestialID);
    PyResult GetDynamicCelestials(PyCallArgs& call, PyInt* locationID);
    PyResult SetMapLandmarks(PyCallArgs& call, PyList* landmarkData);
};

#endif