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
    Rewrite:        Allan
*/


#include "Client.h"
#include "packets/Sovereignty.h"
#include "StaticDataMgr.h"
#include "standing/StandingDB.h"

/*  re-write of standing system  -allan 10Apr15
 *  DB tables have identical field names for ease of implementation
 * Get Fields
 *    toID = me|myCorp|myAlliance
 *    fromID = char|agent|corp|faction|alliance
 *    standing =  fraction representing current value
 * Set Fields
 *    toID = char|agent|corp|faction|alliance
 *    fromID = me|myCorp|myAlliance
 *    standing =  fraction representing current value

 DB tables [fields] (notes)
 repFactions    [fromID, toID, standing] (NPC Faction<-->NPC Faction) {populated, hard-coded  -- cant change}
 repStandings   [fromID, toID, standing] (agent-->char, agent-->PC corp  -- changed by missions status')
 repStandings   [fromID, toID, standing] (corporation<-->alliance, alliance<-->alliance  -- changed thru Corp window)
 repStandings   [fromID, toID, standing] (corporation-->character, corporation<-->corporation  -- changed thru Corp window)
 repStandings   [fromID, toID, standing] (character<-->character, character-->corporation  -- changed thru PnP window)
 repStandings   [fromID, toID, standing] (NPC corp-->char, NPC corp-->PC corp  -- changed by missions and faction kills)

 all change logs will go here.  not sure of all fields yet
 repStandingChanges [fromID, toID, eventID, eventTypeID, eventDateTime, modification, originalFromID, originalToID, int_1, int_2, int_3, msg] ()

 http://www.eveinfo.net/wiki/ind~4067.htm
 */

PyObjectEx* StandingDB::GetFactionStandings() {
    // repFactions table is ONLY faction standings
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT fromID,toID,standing FROM repFactions");
    return DBResultToCRowset(res);
}

PyRep* StandingDB::GetMyStandings(uint32 charID) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT fromID, standing AS rank FROM repStandings WHERE toID = %u", charID);
    return DBResultToCRowset(res);
}

PyRep* StandingDB::GetCharStandings(Client* pClient) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT fromID, toID, standing FROM repStandings WHERE toID = %u OR toID = %u OR fromID = %u", \
                        sDataMgr.GetRaceFaction(pClient->GetChar()->race()), pClient->GetCharacterID(), pClient->GetCharacterID());
    return DBResultToCRowset(res);
}

PyRep* StandingDB::GetCorpStandings(Client* pClient) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT fromID, toID, standing FROM repStandings WHERE toID = %u OR fromID = %u", pClient->GetCorporationID(), pClient->GetCorporationID());
    return DBResultToCRowset(res);
}

PyRep* StandingDB::GetCharNPCStandings(uint32 charID) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT fromID, toID, standing FROM chrNPCStandings WHERE toID = %u", charID );
    return DBResultToCRowset(res);
}

/** @todo not sure about this yet.... wip   ....not used? */
PyRep* StandingDB::PrimeCharStandings(uint32 charID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT "
        " itemID AS ownerID,"
        " itemName AS ownerName,"
        " typeID"
        " FROM entity"
        " WHERE itemID < 0"))
    {
        _log(DATABASE__ERROR, "Error in PrimeCharStandings query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyRep* StandingDB::GetStandingTransactions(Call_GetStandingTransactions& args) {
    //GetStandingTransactions(fromID, toID, direction, eventID, eventType, eventDateTime)
    /** @todo update this for direction */

    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  eventID,"
        "  eventTypeID,"
        "  eventDateTime,"
        "  fromID,"
        "  toID,"
        "  modification,"
        "  originalFromID,"
        "  originalToID,"
        "  int_1,"
        "  int_2,"
        "  int_3,"
        "  msg"
        " FROM repStandingChanges"
        " WHERE toID = %u AND fromID = %u", args.toID, args.fromID )) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }
    return DBResultToRowset(res);
}

float StandingDB::GetStanding(uint32 fromID, uint32 toID) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT standing FROM repStandings WHERE fromID=%u AND toID=%u", fromID, toID);
    DBResultRow row;
    if (res.GetRow(row)) {
        return row.GetFloat(0);
    } else {
        return 0.0f;
    }
}

void StandingDB::SetStanding(uint32 fromID, uint32 toID, float standing) {
    DBerror err;
    sDatabase.RunQuery(err, "INSERT INTO repStandings (fromID, toID, standing) VALUES (%u,%u,%f)", fromID, toID, standing );
}

void StandingDB::UpdateStanding(uint32 fromID, uint32 toID, float standing) {
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO repStandings (fromID, toID, standing)"
        " VALUES (%u,%u,%f)"
        " ON DUPLICATE KEY UPDATE standing = standing + %f", fromID, toID, standing, standing);
}

/** @todo  implement repStandingChanges after standing system is working  */
void StandingDB::SaveStandingChanges(uint32 fromID, uint32 toID, uint16 eventType, float amount, std::string msg) {
    /* eventTypeID,eventDateTime,fromID,toID,modification,originalFromID,originalToID,int_1,int_2,int_3,msg */
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO repStandingChanges (eventTypeID, eventDateTime, fromID, toID, modification, msg)"
        " VALUES (%u, %f, %u, %u, %f, '%s' )",
        eventType, GetFileTimeNow(), fromID, toID, amount, msg.c_str() );
}

PyRep* StandingDB::GetStandingCompositions(uint32 fromID, uint32 toID) {
    // ownerID, standing ...

    return PyStatic.NewNone();
}

PyRep* StandingDB::GetSystemSovInfo(uint32 systemID) {
    /**
     *        sovInfo = sm.RemoteSvc('sovMgr').GetSystemSovereigntyInfo(session.solarsystemid2)
     *        ssAllianceID = sovInfo.allianceID if sovInfo else None
     *
     *        [PyTuple 1 items]
     *            [PySubStream 116 bytes]
     *                [PyObjectData Name: util.KeyVal]
     *                    [PyDict 7 kvp]
     *                        [PyString "contested"]            <<<< interger bool
     *                        [PyInt 0]
     *                        [PyString "corporationID"]
     *                        [PyInt 98049918]
     *                        [PyString "claimTime"]
     *                        [PyIntegerVar 129743663400000000]
     *                        [PyString "claimStructureID"]     <<<<  Territorial Claim Unit
     *                        [PyIntegerVar 1005712174146]
     *                        [PyString "hubID"]                <<<<  iHub (infrastructure hub)
     *                        [PyIntegerVar 1005900797500]
     *                        [PyString "allianceID"]
     *                        [PyInt 99000289]
     *                        [PyString "solarSystemID"]
     *                        [PyInt 30000302]
     */
    //FIXME this needs work.....
    /*
     *    DBQueryResult res;
     *    DBResultRow row;
     *
     *    PyDict *args = new PyDict;
     *
     *    if(!sDatabase.RunQuery(res,
     *        "SELECT"
     *        "   corporationID,"
     *        "   allianceID,"
     *        "   claimStructureID,"
     *        "   claimTime,"
     *        "   hubID,"
     *        "   contested"
     *        " FROM mapSystemSovInfo"
     *        "  WHERE solarSystemID = %u", systemID )) {
     *        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
     *        return NULL;
}

    Rsp_GetSystemSovereigntyInfo rsp;

if (!res.GetRow(row)) {
    sLog.Error("StandingDB::GetSystemSovInfo()", "No Data for systemID %u.  Hacking PyDict.", systemID);
    res.Reset();
    sDatabase.RunQuery(res, "SELECT factionID FROM mapSolarSystems WHERE solarSystemID = %u", systemID );
    uint32 factionID = 0;
    if (res.GetRow(row))
        factionID = row.GetUInt(0);
    args->SetItemString( "contested", new PyInt(0));
    args->SetItemString( "corporationID", new PyInt(0));
    args->SetItemString( "claimTime", new PyLong(0));
    args->SetItemString( "claimStructureID", new PyInt(0));
    args->SetItemString( "hubID", new PyInt(0));
    args->SetItemString( "allianceID", new PyInt(factionID));
    args->SetItemString( "solarSystemID", new PyInt(systemID));
} else {
    args->SetItemString( "contested", new PyInt(row.GetUInt(6)));
    args->SetItemString( "corporationID", new PyInt(row.GetUInt(1)));
    args->SetItemString( "claimTime", new PyLong(row.GetInt64(4)));
    args->SetItemString( "claimStructureID", new PyInt(row.GetUInt(3)));
    args->SetItemString( "hubID", new PyInt(row.GetUInt(5)));
    args->SetItemString( "allianceID", new PyInt(row.GetUInt(2)));
    args->SetItemString( "solarSystemID", new PyInt(row.GetUInt(0)));
}

return new PyObject("util.KeyVal", args);
*/
    return PyStatic.NewNone();
}
