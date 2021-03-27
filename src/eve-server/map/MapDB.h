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
    Updates:    Allan
*/
#ifndef __MAPDB_H_INCL__
#define __MAPDB_H_INCL__

#include "ServiceDB.h"
#include "packets/Map.h"

class PyObject;
class PyRep;

class MapDB
: public ServiceDB
{
public:
    static PyObject* GetPseudoSecurities();
    static PyObject* GetStationExtraInfo();
    static PyObject* GetStationOpServices();
    static PyObject* GetStationServiceInfo();
    static PyObject* GetSolSystemVisits(uint32);

    static void GetStationCount(DBQueryResult& res);

    /* for MapData class */
    static void GetSystemJumps(DBQueryResult& res);

    /* clear system dynamic data on server start */
    static void SystemStartup();

    // for dynamic data handled in system manager
    static void LoadDynamicData(uint32 sysID, SystemKillData& data);

    // dynamic data db methods    -allan
    static PyRep* GetDynamicData(uint8 type, uint8 time);
    static void ManipulateTimeData();
    static void SetSystemActive(uint32 sysID, bool active=false);
    
    static void AddJump(uint32 sysID);      // jumpsHour
    static void AddKill(uint32 sysID); /**killsHour, kills24Hours */
    static void AddPodKill(uint32 sysID);/**podKillsHour, podKills24Hour */
    static void AddFactionKill(uint32 sysID);  /**factionKills, factionKills24Hour*/

    static void UpdateJumps(uint32 sysID, uint16 jumps);    //jumpsHour
    static void UpdateKillData(uint32 sysID, SystemKillData& data);    // ship, faction, pod
    static void UpdatePilotCount(uint32 sysID, uint16 docked=0, uint16 space=0); /**pilotsDocked, pilotsInSpace */
};

#endif
