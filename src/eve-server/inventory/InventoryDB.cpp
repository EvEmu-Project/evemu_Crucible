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
    Author:     Zhur
    Updates:    Allan
*/

/** @todo  most of this needs to go into static data */

#include "eve-server.h"

#include "Client.h"
#include "PyCallable.h"

/* this is only called by Inventory::LoadContents()
 * it is optimized for specific calling objects, to avoid multiple db hits while loading,
 * and to load only things needed for this object at the time of the call.
 */
bool InventoryDB::GetItemContents(OwnerData &od, std::vector<uint32> &into) {
    std::stringstream query;
    query << "SELECT itemID FROM entity WHERE locationID = ";
    query << od.locID;

    if (sDataMgr.IsSolarSystem(od.locID)) {
        query << " AND ownerID = " << od.ownerID;
    } else if (sDataMgr.IsStation(od.locID)) {
        if (od.ownerID == 1) {
            /* this will get agents in station */
            query << " AND ownerID < " << maxNPCItem;
            query << " AND itemID < " << maxNPCItem;
        } else {
            if (IsPlayerCorp(od.corpID)) {
                // check for items owned by corp in players cargo/hangar
                query << " AND ((ownerID = " << od.ownerID << ") OR (ownerID = " << od.corpID << "))";
            } else {
                query << " AND ownerID = " << od.ownerID;
            }
        }
    } else if (IsCharacterID(od.locID)) {
        if (od.ownerID == 1) {
            // not sure what to do here....
        } else if (IsPlayerCorp(od.corpID)) {
            // check for items owned by corp in players cargo/hangar...this *should* work for cap ships' cargo, also
           query << " AND ((ownerID = " << od.ownerID << ") OR (ownerID = " << od.corpID << "))";
        } else {
            query << " AND ownerID = " << od.ownerID;
        }
    } else if (IsOfficeID(od.locID)) {
        // may not need this, as location is officeID, but items MAY be owned by players in corp hangar.
        //query << " AND ownerID = " << od.ownerID;
    }

    query << " ORDER BY itemID";

    DBQueryResult res;
    if (!sDatabase.RunQuery(res,query.str().c_str() )) {
        codelog(DATABASE__ERROR, "Error in GetItemContents query for locationID %u: %s", od.locID, res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "GetItemContents: '%s' returned %lu items", query.str().c_str(), res.GetRowCount());
    DBResultRow row;
    while( res.GetRow( row ) )
        into.push_back( row.GetUInt( 0 ) );

    return true;
}

/*  not used? */
bool InventoryDB::GetItemContents(uint32 itemID, EVEItemFlags flag, std::vector<uint32> &into)
{
    DBQueryResult res;

    if ( !sDatabase.RunQuery( res,
        "SELECT "
        "  itemID"
        " FROM entity "
        " WHERE locationID=%u"
        "  AND flag=%d",
        itemID, (int)flag ) )
    {
        codelog(DATABASE__ERROR, "Error in GetItemContents query for item %u: %s", itemID, res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "GetItemContents for item %u returned %lu items", itemID, res.GetRowCount());
    DBResultRow row;
    while( res.GetRow( row ) )
        into.push_back( row.GetUInt( 0 ) );

    return true;
}

/*  not used? */
bool InventoryDB::GetItemContents(uint32 itemID, EVEItemFlags flag, uint32 ownerID, std::vector<uint32> &into)
{
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT "
        "  itemID"
        " FROM entity "
        " WHERE locationID=%u"
        "  AND flag=%d"
        "  AND ownerID=%u",
        itemID, (int)flag, ownerID))
    {
        codelog(DATABASE__ERROR, "Error in GetItemContents query for item %u with flag %i: %s", itemID, (int)flag, res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "GetItemContents for item %u with flag %i returned %lu items", itemID, (int)flag, res.GetRowCount());
    DBResultRow row;
    while(res.GetRow(row))
        into.push_back(row.GetUInt(0));

    return true;
}

void InventoryDB::DeleteTrackingCans()
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE customInfo LIKE '%Position Test%'");  // 90.63s on main, 0.037s on dev
    //sDatabase.RunQuery(err, "DELETE FROM entity WHERE itemName LIKE '%Bubble%'");         // 66.75s on main, 0.036s on dev
}
