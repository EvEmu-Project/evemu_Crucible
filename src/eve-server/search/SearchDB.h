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
    Author:        Allan
    */


#ifndef EVEMU_SYSTEM_SEARCHDB_H_
#define EVEMU_SYSTEM_SEARCHDB_H_

#include "PyService.h"

//  -allan 5Aug14
enum SearchTypes {
    searchResultAgent           = 1,
    searchResultCharacter       = 2,
    searchResultCorporation     = 3,
    searchResultAlliance        = 4,
    searchResultFaction         = 5,
    searchResultConstellation   = 6,
    searchResultSolarSystem     = 7,
    searchResultRegion          = 8,
    searchResultStation         = 9,
    searchResultInventoryType   = 10,
    //searchResultAllOwners = [1, 2, 3, 4, 5],
    //searchResultAllLocations = [6, 7, 8, 9],
    searchMaxResults            = 500,
    searchMinWildcardLength     = 3
};

class SearchDB
: public ServiceDB {
public:

    PyRep* Query(std::string string, std::vector<int> *searchID, uint32 charID);
    PyRep* QuickQuery(std::string string, std::vector<int> *searchID, uint32 charID, bool hideNPC = false, bool onlyAltName = false);

};


#endif      // EVEMU_SYSTEM_SEARCHDB_H_
