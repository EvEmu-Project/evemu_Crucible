/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
    Author:     Zhur
*/


#ifndef __CONFIGDB_H_INCL__
#define __CONFIGDB_H_INCL__

#include "ServiceDB.h"

class PyRep;

class ConfigDB
: public ServiceDB
{
public:
    PyRep *GetMultiOwnersEx(const std::vector<int32> &entityIDs);
    PyRep *GetMultiLocationsEx(const std::vector<int32> &entityIDs);
    PyRep *GetMultiAllianceShortNamesEx(const std::vector<int32> &entityIDs);
    PyRep *GetMultiCorpTickerNamesEx(const std::vector<int32> &entityIDs);
    PyRep *GetMultiGraphicsEx(const std::vector<int32> &entityIDs);
    PyRep *GetMultiInvTypesEx(const std::vector<int32> &typeIDs);
    PyObject *GetUnits();
    PyObjectEx *GetMapObjects(uint32 entityID, bool wantRegions, bool wantConstellations, bool wantSystems, bool wantStations);
    PyObject *GetMap(uint32 solarSystemID);
    PyObject *ListLanguages();
    PyRep *GetStationSolarSystemsByOwner(uint32 ownerID);
    PyRep *GetCelestialStatistic(uint32 celestialID);
    PyRep *GetTextsForGroup(const std::string & langID, uint32 textgroup);

protected:
};

#endif



