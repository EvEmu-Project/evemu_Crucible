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


#ifndef __MAP_SERVICE_H_INCL__
#define __MAP_SERVICE_H_INCL__

#include "services/Service.h"
#include "map/MapDB.h"

class MapService : public Service <MapService>
{
public:
    MapService();

protected:
    MapDB m_db;

    PyResult GetCurrentEntities(PyCallArgs& call);
    PyResult GetSolarSystemVisits(PyCallArgs& call);
    PyResult GetMyExtraMapInfoAgents(PyCallArgs& call);
    PyResult GetMyExtraMapInfo(PyCallArgs& call);
    PyResult GetBeaconCount(PyCallArgs& call);
    PyResult GetStationExtraInfo(PyCallArgs& call);
    PyResult GetSolarSystemPseudoSecurities(PyCallArgs& call);
    PyResult GetStationCount(PyCallArgs& call);
    PyResult GetHistory(PyCallArgs& call, PyInt* int1, PyInt* int2);
    PyResult GetLinkableJumpArrays(PyCallArgs& call);
    PyResult GetAllianceJumpBridges(PyCallArgs& call);
    PyResult GetAllianceBeacons(PyCallArgs& call);
    PyResult GetCurrentSovData(PyCallArgs& call, PyInt* locationID);
    PyResult GetRecentSovActivity(PyCallArgs& call);
    PyResult GetDeadspaceAgentsMap(PyCallArgs& call, PyInt* languageID);
    PyResult GetDeadspaceComplexMap(PyCallArgs& call, PyInt* languageID);
    PyResult GetSystemsInIncursions(PyCallArgs& call);
    PyResult GetSystemsInIncursionsGM(PyCallArgs& call);
    PyResult GetIncursionGlobalReport(PyCallArgs& call);
    PyResult GetVictoryPoints(PyCallArgs& call);
    PyResult GetStuckSystems(PyCallArgs& call);

};

#endif
