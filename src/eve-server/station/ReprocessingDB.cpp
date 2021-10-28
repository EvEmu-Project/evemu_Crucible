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

#include "station/ReprocessingDB.h"

bool ReprocessingDB::GetRecoverables(const uint32 typeID, std::vector<Recoverable> &into) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
                "SELECT requiredTypeID, MIN(quantity) FROM ramTypeRequirements"
                " LEFT JOIN invBlueprintTypes ON typeID = blueprintTypeID"
                " WHERE damagePerJob = 1 AND ("
                "   (activityID = 6 AND typeID = %u)"
                "   OR"
                "    (activityID = 1 AND productTypeID = %u))"
                " GROUP BY requiredTypeID",
                typeID, typeID))
    {
        _log(DATABASE__ERROR, "Unable to get recoverables for type ID %u: '%s'", typeID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while (res.GetRow(row)) {
        Recoverable rec = Recoverable();
        rec.typeID = row.GetInt(0);
        rec.amountPerBatch = row.GetInt(1);
        into.push_back(rec);
    }

    //eve-dev says tech 2 items contain both basic materials and advanced materials
    //if(gotFromRamTable) return true;

    if (!sDatabase.RunQuery(res,
                "SELECT materialTypeID, quantity"
                " FROM invTypeMaterials"
                " WHERE typeID=%u",
                typeID))
    {
        _log(DATABASE__ERROR, "Unable to get recoverables for ore ID %u: '%s'", typeID, res.error.c_str());
        return false;
    }

    while (res.GetRow(row)) {
        Recoverable rec = Recoverable();
        rec.typeID = row.GetInt(0);
        rec.amountPerBatch = row.GetInt(1);
        into.push_back(rec);
    }

    return true;
}
