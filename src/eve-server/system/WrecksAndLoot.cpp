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
    Author:        Aknor Jaden
*/

#include "eve-server.h"

#include "system/WrecksAndLoot.h"
#include "system/SystemDB.h"


// ////////////////////// DGM_Types_to_Wrecks_Table Class ////////////////////////////
DGM_Types_to_Wrecks_Table::DGM_Types_to_Wrecks_Table()
{
    m_WrecksToTypesMap.clear();
}

DGM_Types_to_Wrecks_Table::~DGM_Types_to_Wrecks_Table()
{
}

int DGM_Types_to_Wrecks_Table::Initialize()
{
    _Populate();

    return 1;
}

void DGM_Types_to_Wrecks_Table::_Populate()
{
    uint32 wreckID, typeID;

    //first get list of all effects from dgmEffects table
    DBQueryResult *res = new DBQueryResult();
    SystemDB::GetWrecksToTypes(*res);

    //counter
	uint32 total_wreck_count = 0;
	uint32 error_count = 0;

	//go through and populate each effect
    DBResultRow row;
    while( res->GetRow(row) )
    {
        typeID = row.GetInt(0);
        wreckID = row.GetInt(1);
		m_WrecksToTypesMap.insert(std::pair<uint32, uint32>(typeID,wreckID));

		total_wreck_count++;
    }

	sLog.Log("DGM_Types_to_Wrecks_Table", "..........%u total wreck objects loaded", total_wreck_count);

    //cleanup
    delete res;
    res = NULL;
}

uint32 DGM_Types_to_Wrecks_Table::GetWreckID(uint32 typeID)
{
    std::map<uint32, uint32>::iterator mWrecksMapIterator;

    if( (mWrecksMapIterator = m_WrecksToTypesMap.find(typeID)) == m_WrecksToTypesMap.end() )
        return 0;
    else
    {
        return mWrecksMapIterator->second;
    }
}
