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

#include "PyService.h"
#include "map/MapDB.h"

class MapService : public PyService
{
public:
    MapService(PyServiceMgr *mgr);
    virtual ~MapService();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    MapDB m_db;

    PyCallable_DECL_CALL(GetStationExtraInfo);
    PyCallable_DECL_CALL(GetSolarSystemPseudoSecurities);
    PyCallable_DECL_CALL(GetSolarSystemVisits);
    PyCallable_DECL_CALL(GetBeaconCount);
    PyCallable_DECL_CALL(GetStuckSystems);
    PyCallable_DECL_CALL(GetRecentSovActivity);
    PyCallable_DECL_CALL(GetDeadspaceAgentsMap);
    PyCallable_DECL_CALL(GetDeadspaceComplexMap);
    PyCallable_DECL_CALL(GetIncursionGlobalReport);
    PyCallable_DECL_CALL(GetSystemsInIncursions);
    PyCallable_DECL_CALL(GetSystemsInIncursionsGM);
    PyCallable_DECL_CALL(GetStationCount);
    PyCallable_DECL_CALL(GetMapLandmarks);
    PyCallable_DECL_CALL(GetMyExtraMapInfo);
    PyCallable_DECL_CALL(GetMyExtraMapInfoAgents);  //ColorStarsByMyAgents
    PyCallable_DECL_CALL(GetHistory);
    PyCallable_DECL_CALL(GetVictoryPoints);
    PyCallable_DECL_CALL(GetAllianceJumpBridges);
    PyCallable_DECL_CALL(GetAllianceBeacons);
    PyCallable_DECL_CALL(GetLinkableJumpArrays);
    PyCallable_DECL_CALL(GetCurrentSovData);
    // custom call for displaying all items in system
    PyCallable_DECL_CALL(GetCurrentEntities);

};

#endif
