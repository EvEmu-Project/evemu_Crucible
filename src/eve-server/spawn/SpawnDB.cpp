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

#include "EVEServerPCH.h"

bool SpawnDB::LoadSpawnGroups(uint32 solarSystemID, std::map<uint32, SpawnGroup *> &into) {
    DBQueryResult res;

    //we could avoid this join by doing the `spawns` select first and collecting the
    //spawn group IDs needed... but this is fine for now.

    if(!sDatabase.RunQuery(res,
        "SELECT "
        " spawnGroups.spawnGroupID,"
        " spawnGroups.spawnGroupName,"
        " spawnGroups.formation"
        " FROM spawns "
        "   LEFT JOIN spawnGroups ON spawnGroups.spawnGroupID=spawns.spawnGroupID"
        " WHERE spawns.solarSystemID=%u"
        " GROUP BY spawnGroups.spawnGroupID",
            solarSystemID))
    {
        codelog(SPAWN__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    while(res.GetRow(row)) {
        SpawnGroup *g = new SpawnGroup(
            row.GetUInt(0),
            row.GetText(1),
            row.GetUInt(2)
            );
        into[ g->id ] = g;
    }

    //now collect the IDs of the groups to build a nice where clause (keep the query/join count down)
    std::vector<int32> groupIDs;
    std::map<uint32, SpawnGroup *>::iterator cur, end;
    cur = into.begin();
    end = into.end();
    for(; cur != end; cur++) {
        groupIDs.push_back(cur->first);
    }

    //now select all the spawn group entries.
    std::string INstr;
    ListToINString(groupIDs, INstr, "-1");

    if(!sDatabase.RunQuery(res,
        "SELECT "
        " spawnGroupID,"
        " npcTypeID,"
        " quantity,"
        " probability,"
        " ownerID,"
        " corporationID"
        " FROM spawnGroupEntries "
        " WHERE spawnGroupID IN (%s)",
            INstr.c_str()))
    {
        codelog(SPAWN__ERROR, "Error in query: %s", res.error.c_str());

        //free up all the stuff we allocated.
        cur = into.begin();
        end = into.end();
        for(; cur != end; cur++) {
            delete cur->second;
        }
        into.clear();

        return false;
    }

    while(res.GetRow(row)) {
        SpawnGroup::Entry entry(
            row.GetUInt(0),
            row.GetUInt(1),
            row.GetUInt(2),
            row.GetFloat(3),
            row.GetUInt(4),
            row.GetUInt(5) );
        cur = into.find(entry.spawnGroupID);
        if(cur == into.end()) {
            _log(SPAWN__ERROR, "Loading spawn group entry failed, unable to find group %u for type %u", entry.spawnGroupID, entry.npcTypeID);
            continue;
        }

        cur->second->entries.push_back(entry);
    }

    return true;
}




bool SpawnDB::LoadSpawnEntries(uint32 solarSystemID, const std::map<uint32, SpawnGroup *> &groups, std::map<uint32, SpawnEntry *> &into) {
    DBQueryResult res;

    //we could avoid this join by doing the `spawns` select first and collecting the
    //spawn group IDs needed... but this is fine for now.

    if(!sDatabase.RunQuery(res,
        "SELECT "
        " spawnID,"
        " spawnGroupID,"
        " spawnBoundType,"
        " spawnTimer,"
        " respawnTimeMin,"
        " respawnTimeMax"
        " FROM spawns "
        " WHERE solarSystemID=%u",
            solarSystemID))
    {
        codelog(SPAWN__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    uint32 now = Timer::GetTimeSeconds();

    std::map<uint32, SpawnGroup *>::const_iterator result;
    while(res.GetRow(row)) {
        //process group
        uint32 groupID = row.GetUInt(1);
        result = groups.find(groupID);
        if(result == groups.end()) {
            _log(SPAWN__ERROR, "Error loading spawn entry %u: Unable to find spawn group %u. Skipping.", row.GetUInt(0), row.GetUInt(1));
            continue;
        }

        //process bounds type
        SpawnEntry::SpawnBoundsType boundsType;
        switch(row.GetUInt(2)) {    //enum checking.
            case SpawnEntry::boundsPoint:       boundsType = SpawnEntry::boundsPoint; break;
            case SpawnEntry::boundsLine:        boundsType = SpawnEntry::boundsLine; break;
            //case SpawnEntry::boundsTriangle:  boundsType = SpawnEntry::boundsTriangle; break;
            //case SpawnEntry::boundsSquare:    boundsType = SpawnEntry::boundsSquare; break;
            case SpawnEntry::boundsCube:        boundsType = SpawnEntry::boundsCube; break;
            default:
                _log(SPAWN__ERROR, "Error loading spawn entry %u: Bounds type %u is invalid. Skipping.", row.GetUInt(0), row.GetUInt(2));
                continue;
        }

        //convert stored timer value into a future milliseconds timer.
        uint32 timer_val = row.GetUInt(3);
        if(timer_val <= now)
            timer_val = 1;  //1 is just as good as 0, without the special implications
        else
            timer_val = (timer_val - now) * 1000;

        SpawnEntry *e = new SpawnEntry(
            row.GetUInt(0),
            *result->second,
            row.GetUInt(4),
            row.GetUInt(5),
            timer_val,
            boundsType
        );
        into[ e->GetID() ] = e;
    }

    /*
     * Next, we need to select the bounds of each spawn entry.
     */
    //now collect the IDs of the entries to build a nice where clause (keep the query/join count down)
    std::vector<int32> entryIDs;
    std::map<uint32, SpawnEntry *>::iterator cur, end, tmp;
    cur = into.begin();
    end = into.end();
    for(; cur != end; cur++) {
        entryIDs.push_back(cur->first);
    }

    //now select all the spawn group entries.
    std::string INstr;
    ListToINString(entryIDs, INstr, "-1");

    //we could avoid this order by clause by exposing the `m_bounds` member of SpawnEntry
    if(!sDatabase.RunQuery(res,
        "SELECT "
        " spawnID,"
        " pointIndex,"
        " x, y, z"
        " FROM spawnBounds "
        " WHERE spawnID IN (%s)",
            INstr.c_str()))
    {
        codelog(SPAWN__ERROR, "Error in query: %s", res.error.c_str());

        //free up all the stuff we allocated.
        cur = into.begin();
        end = into.end();
        for(; cur != end; cur++) {
            delete cur->second;
        }
        into.clear();

        return false;
    }

    while(res.GetRow(row)) {
        //this could be more optimized, but I just wanna get it working right now...
        uint32 id = row.GetUInt(0);
        uint32 index = row.GetUInt(1);
        GPoint p(
            row.GetDouble(2),
            row.GetDouble(3),
            row.GetDouble(4) );

        cur = into.find(id);
        if(cur == into.end()) {
            _log(SPAWN__ERROR, "Loading spawn group entry failed, unable to find entry %u for bound point %u,%u", id, id, index);
            continue;
        }

        if(cur->second->bounds.size() <= index)
            cur->second->bounds.resize(index+1);
        cur->second->bounds[index] = p;
    }

    //make sure bounds are loaded properly for each spawn entry...
    cur = into.begin();
    while(cur != into.end()) {
        if(!cur->second->CheckBounds()) {
            _log(SPAWN__ERROR, "Spawn entry %u removed due to invalid bounds.", cur->first);
            delete cur->second;
            tmp = cur;
            cur++;
            into.erase(tmp);
        } else {
            cur++;
        }
    }

    return true;
}























