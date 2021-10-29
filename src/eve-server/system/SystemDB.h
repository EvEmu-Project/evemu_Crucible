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

#ifndef __SYSTEMDB_H_INCL__
#define __SYSTEMDB_H_INCL__

#include "ServiceDB.h"

class SystemDB
: public ServiceDB
{
public:
    PyObject* ListFactions();
    static PyObject* ListJumps(uint32);
    static PyPackedRow* GetSolarSystem(uint32 ssid);

    static void GetGates(uint32 systemID, std::vector< DBGPointEntity >& gateIDs, uint8& total);

    void GetBelts(uint32 systemID, std::vector< DBGPointEntity >& beltIDs, uint8& total);
    void GetMoons(uint32 systemID, std::vector< DBGPointEntity >& moonIDs, uint8& total);
    void GetPlanets(uint32 systemID, std::vector<DBGPointEntity>& planetIDs, uint8& total);

    static bool GetWrecksToTypes(DBQueryResult& res);

    static void GetLootGroups(DBQueryResult& res);
    static void GetLootGroupTypes(DBQueryResult& res);

    static uint32 GetObjectLocationID( uint32 itemID );

    double GetItemTypeRadius( uint32 typeID );
    double GetCelestialRadius(uint32 itemID);
    static GPoint GetSolarSystemPosition(uint32 systemID);

    static bool LoadSystemStaticEntities(uint32 systemID, std::vector<DBSystemEntity>& into);
    static bool LoadSystemDynamicEntities(uint32 systemID, std::vector<DBSystemDynamicEntity>& into);
    static bool LoadPlayerDynamicEntities(uint32 systemID, std::vector<DBSystemDynamicEntity>& into);
};


#endif