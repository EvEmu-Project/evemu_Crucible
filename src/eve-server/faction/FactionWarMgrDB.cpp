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
*/

#include "eve-server.h"

#include "faction/FactionWarMgrDB.h"
/*
 * FACWAR__DB_ERROR
 * FACWAR__DB_WARNING
 * FACWAR__DB_INFO
 * FACWAR__DB_MESSAGE
 */

PyRep *FactionWarMgrDB::GetWarFactions() {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT factionID, militiaCorporationID"
        " FROM facFactions"
        " WHERE militiaCorporationID IS NOT NULL"))
    {
        _log(DATABASE__ERROR, "Failed to query war factions: %s.", res.error.c_str());
        return nullptr;
    }

    return DBResultToIntIntDict(res);
}

PyRep* FactionWarMgrDB::GetFacWarSystems()
{   /* done  -allan 03May16 */
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT systemID, occupierID, factionID FROM facWarSystems"))
    {
        _log(DATABASE__ERROR, "Failed to query war factions: %s.", res.error.c_str());
        return nullptr;
    }

    PyDict* result = new PyDict();
    PyDict* dict;
    DBResultRow row;
    while (res.GetRow(row)) {
        dict = new PyDict();
        dict->SetItemString("occupierID", new PyInt(row.GetInt(1)));
        dict->SetItemString("factionID", new PyInt(row.GetInt(2)));
        result->SetItem(new PyInt(row.GetInt(0)), dict );
    }
    return result;
}

uint32 FactionWarMgrDB::GetFactionMilitiaCorporation(const uint32 factionID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT militiaCorporationID"
        " FROM facFactions"
        " WHERE factionID=%u",
        factionID))
    {
        _log(DATABASE__ERROR, "Failed to query militia corporation for faction %u: %s.", factionID, res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(FACWAR__DB_WARNING, "Faction %u not found.", factionID);
        return 0;
    }

    return row.GetUInt(0);
}

