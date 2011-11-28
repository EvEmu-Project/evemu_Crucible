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

PyObject *CorporationDB::ListCorpStations(uint32 corp_id) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   stationID, stationTypeID AS typeID"
        " FROM staStations"
        " WHERE corporationID=%u",
            corp_id
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *CorporationDB::ListStationOffices(uint32 station_id) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   corporationID, itemID, officeFolderID"
        " FROM crpOffices"
        " WHERE officeFolderID=%u",
//TODO: new a new DBSequence for this ID
            station_id + 6000000
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *CorporationDB::ListStationCorps(uint32 station_id) {

    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   corporationID,corporationName,description,shares,graphicID,"
        "   memberCount,ceoID,stationID,raceID,corporationType,creatorID,"
        "   hasPlayerPersonnelManager,tickerName,sendCharTerminationMessage,"
        "   shape1,shape2,shape3,color1,color2,color3,typeface,memberLimit,"
        "   allowedMemberRaceIDs,url,taxRate,minimumJoinStanding,division1,"
        "   division2,division3,division4,division5,division6,division7,"
        "   allianceID,deleted"
        " FROM corporation"
//no idea what the criteria should be here...
        " WHERE stationID=%u",
            station_id
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *CorporationDB::ListStationOwners(uint32 station_id) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   itemID AS ownerID, itemName AS ownerName, typeID"
        " FROM corporation"
//no idea what the criteria should be here...
        "   LEFT JOIN eveNames ON (creatorID=itemID OR ceoID=itemID)"
        "WHERE stationID=%u",
            station_id
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyDict *CorporationDB::ListAllCorpInfo() {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   corporationName,"
        "   corporationID,"
        "   size,extent,solarSystemID,investorID1,investorShares1,"
        "   investorID2, investorShares2, investorID3,investorShares3,"
        "   investorID4,investorShares4,"
        "   friendID,enemyID,publicShares,initialPrice,"
        "   minSecurity,scattered,fringe,corridor,hub,border,"
        "   factionID,sizeFactor,stationCount,stationSystemCount,"
        "   stationID,ceoID,entity.itemName AS ceoName"
        " FROM crpNPCCorporations"
        " JOIN corporation USING (corporationID)"
        "   LEFT JOIN entity ON ceoID=entity.itemID"
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return(DBResultToIntRowDict(res, 1));
}

bool CorporationDB::ListAllCorpFactions(std::map<uint32, uint32> &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   corporationID,factionID"
        " FROM crpNPCCorporations"
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultToUIntUIntDict(res, into);
    return true;
}

bool CorporationDB::ListAllFactionStationCounts(std::map<uint32, uint32> &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   factionID, COUNT(DISTINCT staStations.stationID) "
        " FROM crpNPCCorporations"
        " LEFT JOIN staStations USING (corporationID)"
        " GROUP BY factionID"
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultToUIntUIntDict(res, into);
    return true;
}

bool CorporationDB::ListAllFactionSystemCounts(std::map<uint32, uint32> &into) {
    DBQueryResult res;

    //this is not quite right, but its good enough.
    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   factionID, COUNT(solarSystemID) "
        " FROM mapSolarSystems"
        " GROUP BY factionID"
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultToUIntUIntDict(res, into);
    return true;
}

bool CorporationDB::ListAllFactionRegions(std::map<int32, PyRep *> &into) {
    DBQueryResult res;

    //this is not quite right, but its good enough.
    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   factionID,regionID "
        " FROM mapRegions"
        " WHERE factionID IS NOT NULL"
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultToIntIntlistDict(res, into);
    return true;
}

bool CorporationDB::ListAllFactionConstellations(std::map<int32, PyRep *> &into) {
    DBQueryResult res;

    //this is not quite right, but its good enough.
    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   factionID,constellationID "
        " FROM mapConstellations"
        " WHERE factionID IS NOT NULL"
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultToIntIntlistDict(res, into);
    return true;
}

bool CorporationDB::ListAllFactionSolarSystems(std::map<int32, PyRep *> &into) {
    DBQueryResult res;

    //this is not quite right, but its good enough.
    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   factionID,solarSystemID "
        " FROM mapSolarSystems"
        " WHERE factionID IS NOT NULL"
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultToIntIntlistDict(res, into);
    return true;
}

bool CorporationDB::ListAllFactionRaces(std::map<int32, PyRep *> &into) {
    DBQueryResult res;

    //this is not quite right, but its good enough.
    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   factionID,raceID "
        " FROM factionRaces"
        " WHERE factionID IS NOT NULL"
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultToIntIntlistDict(res, into);
    return true;
}

PyObject *CorporationDB::ListNPCDivisions() {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   divisionID, divisionName, description, leaderType"
        " FROM crpNPCDivisions"
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *CorporationDB::GetEmploymentRecord(uint32 charID) {
    DBQueryResult res;

    //do we really need this order by??
    if (!sDatabase.RunQuery(res,
        "SELECT startDate, corporationID, deleted "
        "   FROM chrEmployment "
        "   WHERE characterID = %u "
        "   ORDER BY startDate DESC", charID
        ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return (DBResultToRowset(res));
}

PyObject* CorporationDB::GetMedalsReceived( uint32 charID )
{
    sLog.Debug( "CorporationDB", "Called GetMedalsReceived stub." );

    util_Rowset rs;

    rs.header.push_back( "medalID" );
    rs.header.push_back( "title" );
    rs.header.push_back( "description" );
    rs.header.push_back( "ownerID" );
    rs.header.push_back( "" );    //wtf??
    rs.header.push_back( "issuerID" );
    rs.header.push_back( "date" );
    rs.header.push_back( "reason" );
    rs.header.push_back( "status" );

    return rs.Encode();
}

static std::string _IoN( PyRep* r )
{
    if( !r->IsInt() )
        return "NULL";
    return itoa( r->AsInt()->value() );
}

bool CorporationDB::AddCorporation(Call_AddCorporation & corpInfo, uint32 charID, uint32 stationID, uint32 & corpID) {
    DBerror err;
    corpID = 0;

    std::string cName, cDesc, cTick, cURL;
    sDatabase.DoEscapeString(cName, corpInfo.corpName);
    sDatabase.DoEscapeString(cDesc, corpInfo.description);
    sDatabase.DoEscapeString(cTick, corpInfo.corpTicker);
    sDatabase.DoEscapeString(cURL, corpInfo.url);

    //TODO: we should be able to get our race ID directly from our Client
    //object eventually, instead of pulling it from this join.
    if (!sDatabase.RunQueryLID(err, corpID,
        " INSERT INTO corporation ( "
        "   corporationName, description, tickerName, url, "
        "   taxRate, minimumJoinStanding, corporationType, hasPlayerPersonnelManager, sendCharTerminationMessage, "
        "   creatorID, ceoID, stationID, raceID, allianceID, shares, memberCount, memberLimit, "
        "   allowedMemberRaceIDs, graphicID, color1, color2, color3, shape1, shape2, shape3, "
        "   typeface "
        "   ) "
        " SELECT "
        "       '%s', '%s', '%s', '%s', "
        "       %lf, 0, 2, 0, 1, "
        "       %u, %u, %u, chrBloodlines.raceID, 0, 1000, 0, 10, "
        "       chrBloodlines.raceID, 0, %s, %s, %s, %s, %s, %s, "
        "       NULL "
        "    FROM entity "
        "       LEFT JOIN bloodlineTypes USING (typeID) "
        "       LEFT JOIN chrBloodlines USING (bloodlineID) "
        "    WHERE entity.itemID = %u ",
        cName.c_str(), cDesc.c_str(), cTick.c_str(), cURL.c_str(),
        corpInfo.taxRate,
        charID, charID, stationID,
        _IoN(corpInfo.color1).c_str(),
        _IoN(corpInfo.color2).c_str(),
        _IoN(corpInfo.color3).c_str(),
        _IoN(corpInfo.shape1).c_str(),
        _IoN(corpInfo.shape2).c_str(),
        _IoN(corpInfo.shape3).c_str(),
        charID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
        return false;
    }

    // It has to go into the eveStaticOwners too
    // (well, not exactly there, but it has to be cached, and i don't know how
    // that works clientside...)
    // This is a temp hack to make my life easier
    if (!sDatabase.RunQuery(err,
        " REPLACE INTO eveStaticOwners (ownerID,ownerName,typeID) "
        "   VALUES (%u, '%s', 2)",
        corpID, cName.c_str()))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
        return false;
    }

    // And create a channel too
    if (!sDatabase.RunQuery(err,
        " INSERT INTO channels ("
        "   channelID, ownerID, displayName, motd, comparisonKey, "
        "   memberless, password, mailingList, cspa, temporary, "
        "   mode, subscribed, estimatedMemberCount"
        " ) VALUES ("
        "   %u, %u, '%s', '%s MOTD', '%s', "
        "   1, NULL, 0, 127, 0, "
        "   1, 1, 0"
        " )",
        corpID, corpID, cName.c_str(), cName.c_str(), cTick.c_str()
        ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
        // This is not a serious problem either, but would be good if the channel
        // were working...
    }

    return true;
}

#define _NI(a, b) if (row.IsNull(b)) { cc.a = new PyNone(); } else { cc.a = new PyInt(row.GetUInt(b)); }

bool CorporationDB::CreateCorporationChangePacket(Notify_OnCorporaionChanged & cc, uint32 oldCorpID, uint32 newCorpID) {
    DBQueryResult res;
    DBResultRow row;

    if (!sDatabase.RunQuery(res,
        " SELECT "
        "   corporationID,corporationName,description,tickerName,url,"
        "   taxRate,minimumJoinStanding,corporationType,hasPlayerPersonnelManager,"
        "   sendCharTerminationMessage,creatorID,ceoID,stationID,raceID,"
        "   allianceID,shares,memberCount,memberLimit,allowedMemberRaceIDs,"
        "   graphicID,shape1,shape2,shape3,color1,color2,color3,typeface,"
        "   division1,division2,division3,division4,division5,division6,"
        "   division7,deleted"
        " FROM corporation "
        " WHERE corporationID = %u ", newCorpID
        ))
    {
        codelog(SERVICE__ERROR, "Error in retrieving new corporation's data (%u)", newCorpID);
        return false;
    }

    if(!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Unable to find new corporation's data (%u)", newCorpID);
        return false;
    }

    cc.corporationIDNew = row.GetUInt(0);
    cc.corporationNameNew = row.GetText(1);
    cc.descriptionNew = row.GetText(2);
    cc.tickerNameNew = row.GetText(3);
    cc.urlNew = row.GetText(4);
    cc.taxRateNew = row.GetDouble(5);
    cc.minimumJoinStandingNew = row.GetDouble(6);
    cc.corporationTypeNew = row.GetUInt(7);
    cc.hasPlayerPersonnelManagerNew = row.GetUInt(8);
    cc.sendCharTerminationMessageNew = row.GetUInt(9);
    cc.creatorIDNew = row.GetUInt(10);
    cc.ceoIDNew = row.GetUInt(11);
    cc.stationIDNew = row.GetUInt(12);
    _NI(raceIDNew, 13);
    _NI(allianceIDNew, 14);
    cc.sharesNew = row.GetUInt64(15);
    cc.memberCountNew = row.GetUInt(16);
    cc.memberLimitNew = row.GetUInt(17);
    cc.allowedMemberRaceIDsNew = row.GetUInt(18);
    cc.graphicIDNew = row.GetUInt(19);
    _NI(shape1New, 20);
    _NI(shape2New, 21);
    _NI(shape3New, 22);
    _NI(color1New, 23);
    _NI(color2New, 24);
    _NI(color3New, 25);
    _NI(typefaceNew, 26);
    _NI(division1New, 27);
    _NI(division2New, 28);
    _NI(division3New, 29);
    _NI(division4New, 30);
    _NI(division5New, 31);
    _NI(division6New, 32);
    _NI(division7New, 33);
    cc.deletedNew = row.GetUInt(34);

    if (!sDatabase.RunQuery(res,
        " SELECT "
        "   corporationID,corporationName,description,tickerName,url,"
        "   taxRate,minimumJoinStanding,corporationType,hasPlayerPersonnelManager,"
        "   sendCharTerminationMessage,creatorID,ceoID,stationID,raceID,"
        "   allianceID,shares,memberCount,memberLimit,allowedMemberRaceIDs,"
        "   graphicID,shape1,shape2,shape3,color1,color2,color3,typeface,"
        "   division1,division2,division3,division4,division5,division6,"
        "   division7,deleted"
        " FROM corporation "
        " WHERE corporationID = %u ", oldCorpID
        ))
    {
        codelog(SERVICE__ERROR, "Error in retrieving old corporation's data (%u)", oldCorpID);
        return false;
    }

    if(!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Unable to find old corporation's data (%u)", oldCorpID);
        return false;
    }

    cc.corporationIDOld = new PyInt(row.GetUInt(0));
    cc.corporationNameOld = new PyString(row.GetText(1));
    cc.descriptionOld = new PyString(row.GetText(2));
    cc.tickerNameOld = new PyString(row.GetText(3));
    cc.urlOld = new PyString(row.GetText(4));
    cc.taxRateOld = new PyFloat(row.GetDouble(5));
    cc.minimumJoinStandingOld = new PyFloat(row.GetDouble(6));
    cc.corporationTypeOld = new PyInt(row.GetUInt(7));
    cc.hasPlayerPersonnelManagerOld = new PyInt(row.GetUInt(8));
    cc.sendCharTerminationMessageOld = new PyInt(row.GetUInt(9));
    cc.creatorIDOld = new PyInt(row.GetUInt(10));
    cc.ceoIDOld = new PyInt(row.GetUInt(11));
    cc.stationIDOld = new PyInt(row.GetUInt(12));
    _NI(raceIDOld, 13);
    _NI(allianceIDOld, 14);
    cc.sharesOld = new PyLong(row.GetUInt64(15));
    cc.memberCountOld = new PyInt(row.GetUInt(16));
    cc.memberLimitOld = new PyInt(row.GetUInt(17));
    cc.allowedMemberRaceIDsOld = new PyInt(row.GetUInt(18));
    cc.graphicIDOld = new PyInt(row.GetUInt(19));
    _NI(shape1Old, 20);
    _NI(shape2Old, 21);
    _NI(shape3Old, 22);
    _NI(color1Old, 23);
    _NI(color2Old, 24);
    _NI(color3Old, 25);
    _NI(typefaceOld, 26);
    _NI(division1Old, 27);
    _NI(division2Old, 28);
    _NI(division3Old, 29);
    _NI(division4Old, 30);
    _NI(division5Old, 31);
    _NI(division6Old, 32);
    _NI(division7Old, 33);
    cc.deletedOld = new PyInt(row.GetUInt(34));

    return true;
}


bool CorporationDB::JoinCorporation(uint32 charID, uint32 corpID, uint32 oldCorpID, const CorpMemberInfo &roles) {
    // TODO: check for free member place

    DBerror err;
    // Decrease previous corp's member count
    if (!sDatabase.RunQuery(err,
        "UPDATE corporation "
        "   SET corporation.memberCount = corporation.memberCount-1"
        "   WHERE corporation.corporationID = %u",
            oldCorpID
        ))
    {
        codelog(SERVICE__ERROR, "Error in prev corp member decrease query: %s", err.c_str());
        return false;
    }

    // Set new corp
    if (!sDatabase.RunQuery(err,
        "UPDATE character_ SET "
        "   corporationID = %u, corporationDateTime = "I64u", "
        "   corpRole = "I64u", rolesAtAll = "I64u", rolesAtBase = "I64u", rolesAtHQ = "I64u", rolesAtOther = "I64u" "
        "   WHERE characterID = %u",
            corpID, Win32TimeNow(),
            roles.corpRole, roles.rolesAtAll, roles.rolesAtBase, roles.rolesAtHQ, roles.rolesAtOther,
            charID
        ))
    {
        codelog(SERVICE__ERROR, "Error in char update query: %s", err.c_str());
        //TODO: undo previous member count decrement.
        return false;
    }

    // Increase new corp's member number...
    if (!sDatabase.RunQuery(err,
        "UPDATE corporation "
        "   SET memberCount = memberCount+1"
        "   WHERE corporationID = %u",
            corpID
        ))
    {
        codelog(SERVICE__ERROR, "Error in new corp member decrease query: %s", err.c_str());
        //dont stop now, we are already moved... else we need to undo everything we just did.
    }

    // Add new employment history record
    if (!sDatabase.RunQuery(err,
        "INSERT INTO chrEmployment VALUES (%u, %u, "I64u", 0)",
        charID, corpID, Win32TimeNow()
        ))
    {
        codelog(SERVICE__ERROR, "Error in employment insert query: %s", err.c_str());
        //dont stop now, we are already moved... else we need to undo everything we just did.
    }

    return true;
}

bool CorporationDB::CreateCorporationCreatePacket(Notify_OnCorporaionChanged & cc, uint32 oldCorpID, uint32 newCorpID) {
    DBQueryResult res;
    DBResultRow row;

    if (!sDatabase.RunQuery(res,
        " SELECT "
        "   corporationID,corporationName,description,tickerName,url,"
        "   taxRate,minimumJoinStanding,corporationType,hasPlayerPersonnelManager,"
        "   sendCharTerminationMessage,creatorID,ceoID,stationID,raceID,"
        "   allianceID,shares,memberCount,memberLimit,allowedMemberRaceIDs,"
        "   graphicID,shape1,shape2,shape3,color1,color2,color3,typeface,"
        "   division1,division2,division3,division4,division5,division6,"
        "   division7,deleted"
        " FROM corporation "
        " WHERE corporationID = %u ", newCorpID
        ))
    {
        codelog(SERVICE__ERROR, "Error in retrieving new corporation's data (%u)", newCorpID);
        return false;
    }

    if(!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Unable to find corporation's data (%u)", newCorpID);
        return false;
    }

    cc.allianceIDOld = new PyNone();
    cc.allowedMemberRaceIDsOld = new PyNone();
    cc.ceoIDOld = new PyNone();
    cc.color1Old = new PyNone();
    cc.color2Old = new PyNone();
    cc.color3Old = new PyNone();
    cc.corporationIDOld = new PyNone();
    cc.corporationNameOld = new PyNone();
    cc.corporationTypeOld = new PyNone();
    cc.creatorIDOld = new PyNone();
    cc.deletedOld = new PyNone();
    cc.descriptionOld = new PyNone();
    cc.division1Old = new PyNone();
    cc.division2Old = new PyNone();
    cc.division3Old = new PyNone();
    cc.division4Old = new PyNone();
    cc.division5Old = new PyNone();
    cc.division6Old = new PyNone();
    cc.division7Old = new PyNone();
    cc.graphicIDOld = new PyNone();
    cc.hasPlayerPersonnelManagerOld = new PyNone();
    cc.memberCountOld = new PyNone();
    cc.memberLimitOld = new PyNone();
    cc.minimumJoinStandingOld = new PyNone();
    cc.raceIDOld = new PyNone();
    cc.sendCharTerminationMessageOld = new PyNone();
    cc.shape1Old = new PyNone();
    cc.shape2Old = new PyNone();
    cc.shape3Old = new PyNone();
    cc.sharesOld = new PyNone();
    cc.stationIDOld = new PyNone();
    cc.taxRateOld = new PyNone();
    cc.tickerNameOld = new PyNone();
    cc.typefaceOld = new PyNone();
    cc.urlOld = new PyNone();

    cc.corporationIDNew = row.GetUInt(0);
    cc.corporationNameNew = row.GetText(1);
    cc.descriptionNew = row.GetText(2);
    cc.tickerNameNew = row.GetText(3);
    cc.urlNew = row.GetText(4);
    cc.taxRateNew = row.GetDouble(5);
    cc.minimumJoinStandingNew = row.GetDouble(6);
    cc.corporationTypeNew = row.GetUInt(7);
    cc.hasPlayerPersonnelManagerNew = row.GetUInt(8);
    cc.sendCharTerminationMessageNew = row.GetUInt(9);
    cc.creatorIDNew = row.GetUInt(10);
    cc.ceoIDNew = row.GetUInt(11);
    cc.stationIDNew = row.GetUInt(12);
    _NI(raceIDNew, 13);
    _NI(allianceIDNew, 14);
    cc.sharesNew = row.GetUInt64(15);
    cc.memberCountNew = row.GetUInt(16);
    cc.memberLimitNew = row.GetUInt(17);
    cc.allowedMemberRaceIDsNew = row.GetUInt(18);
    cc.graphicIDNew = row.GetUInt(19);
    _NI(shape1New, 20);
    _NI(shape2New, 21);
    _NI(shape3New, 22);
    _NI(color1New, 23);
    _NI(color2New, 24);
    _NI(color3New, 25);
    _NI(typefaceNew, 26);
    _NI(division1New, 27);
    _NI(division2New, 28);
    _NI(division3New, 29);
    _NI(division4New, 30);
    _NI(division5New, 31);
    _NI(division6New, 32);
    _NI(division7New, 33);
    cc.deletedNew = row.GetUInt(34);

    return true;
}

PyObject *CorporationDB::GetCorporation(uint32 corpID) {
    DBQueryResult res;
    DBResultRow row;

    if (!sDatabase.RunQuery(res,
        " SELECT "
        "   corporationID,corporationName,description,tickerName,url,"
        "   taxRate,minimumJoinStanding,corporationType,hasPlayerPersonnelManager,"
        "   sendCharTerminationMessage,creatorID,ceoID,stationID,raceID,"
        "   allianceID,shares,memberCount,memberLimit,allowedMemberRaceIDs,"
        "   graphicID,shape1,shape2,shape3,color1,color2,color3,typeface,"
        "   division1,division2,division3,division4,division5,division6,"
        "   division7,deleted"
        " FROM corporation "
        " WHERE corporationID = %u", corpID))
    {
        codelog(SERVICE__ERROR, "Error in retrieving corporation's data (%u)", corpID);
        return NULL;
    }

    if(!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Unable to find corporation's data (%u)", corpID);
        return NULL;
    }

    return DBRowToRow(row);
    //return DBResultToRowset(res);
}

PyObject *CorporationDB::GetEveOwners() {
    DBQueryResult res;

    /*if (!sDatabase.RunQuery(res,
        " SELECT * FROM eveStaticOwners "))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }*/
    if( !sDatabase.RunQuery( res,
        "(SELECT"
        " itemID AS ownerID,"
        " itemName AS ownerName,"
        " typeID"
        " FROM entity"
        " WHERE itemID < 140000000"
        " AND itemID NOT IN ( SELECT ownerID from eveStaticOwners ) )"
        " UNION ALL "
        "(SELECT"
        " *"
        " FROM eveStaticOwners)"
        " ORDER BY ownerID" ) )
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *CorporationDB::GetStations(uint32 corpID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT "
        " stationID, stationTypeID as typeID "
        " FROM staStations "
        " WHERE corporationID = %u ", corpID
        ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }
    return DBResultToRowset(res);
}

uint32 CorporationDB::GetOffices(uint32 corpID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT "
        " COUNT(1) AS OfficeNumber "
        " FROM crpOffices "
        " WHERE corporationID = %u ", corpID
        ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return(0);
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Unable to find corporation's data (%u)", corpID);
        return 0;
    }
    return row.GetUInt(0);
}

PyRep *CorporationDB::Fetch(uint32 corpID, uint32 from, uint32 count) {
    DBQueryResult res;
    DBResultRow rr;

    if (!sDatabase.RunQuery(res,
        " SELECT stationID, typeID, itemID, officeFolderID "
        " FROM crpOffices "
        " WHERE corporationID = %u "
        " LIMIT %u, %u ", corpID, from, count
        ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    res.GetRow(rr);

    // Have to send back a list that contains a tuple that contains an int and a list...
    // params probably needs the following stuff: stationID, typeID, officeID, officeFolderID
    Reply_FetchOffice reply;
    reply.params = new PyList;

    reply.params->AddItemInt( rr.GetInt(0) );
    reply.params->AddItemInt( rr.GetInt(1) );
    reply.officeID = rr.GetInt(2);
    reply.params->AddItemInt( reply.officeID );
    reply.params->AddItemInt( rr.GetInt(3) );

    return reply.Encode();
}
uint32 CorporationDB::GetQuoteForRentingAnOffice(uint32 stationID) {
    DBQueryResult res;
    DBResultRow row;

    if (!sDatabase.RunQuery(res,
        " SELECT "
        " officeRentalCost "
        " FROM staStations "
        " WHERE staStations.stationID = %u ", stationID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        // Try to look more clever than we actually are...
        return 10000;
    }

    if (!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Unable to find station data, stationID: %u", stationID);
        // Try to look more clever than we actually are...
        return 10000;
    }

    return row.GetUInt(0);
}
// Need to find out wether there is any kind of limit regarding the offices
uint32 CorporationDB::ReserveOffice(const OfficeInfo & oInfo) {
    // oInfo should at this point contain the station, officeFolder and corporation infos

    // First check if we have a free office at this station at all...
    // Instead, assume that there is, and add one for this corporation
    DBerror err;

    // First add it into the entity table
    uint32 officeID = 0;
    if (!sDatabase.RunQueryLID(err, officeID,
        " INSERT INTO entity ("
        " itemName, typeID, ownerID, locationID, flag, contraband, singleton, "
        " quantity, x, y, z, customInfo "
        " ) VALUES ("
        // office name should be more descriptive
        // corporation owns the office, station locates the office
        // x, y, z should be coords of the station?
        // no extra info
        " 'office', 27, %u, %u, 0, 0, 1, 1, 0, 0, 0, '' "
        " ); ", oInfo.corporationID, oInfo.stationID ))
    {
        codelog(SERVICE__ERROR, "Error in query at ReserveOffice: %s", err.c_str());
        return(0);
    }

    // inserts with the id gotten previously
    if (!sDatabase.RunQuery(err,
        " INSERT INTO crpOffices "
        " (corporationID, stationID, itemID, typeID, officeFolderID) "
        " VALUES "
        " (%u, %u, %u, %u, %u) ",
        oInfo.corporationID, oInfo.stationID, officeID, oInfo.typeID, oInfo.officeFolderID))
    {
        codelog(SERVICE__ERROR, "Error in query at ReserveOffice: %s", err.c_str());
        // Ensure that officeID stays 0, whatever the RunQueryLID done...
        return(0);
    }

    // If insert is successful, oInfo.officeID now contains the rented office's ID
    // Nothing else to do...
    return(officeID);
}

//NOTE: it makes sense to push this up to ServiceDB, since others will likely need this too.
uint32 CorporationDB::GetStationOwner(uint32 stationID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT corporationID "
        " FROM staStations "
        " WHERE stationID = %u ", stationID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Missing stationID: %u", stationID);
        return 0;
    }
    return row.GetUInt(0);
}

PyRep *CorporationDB::GetMyApplications(uint32 charID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT corporationID, characterID, applicationText, roles, grantableRoles, "
        " status, applicationDateTime, deleted, lastCorpUpdaterID "
        " FROM chrApplications "
        " WHERE characterID = %u ", charID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }
    return DBResultToRowset(res);
}
bool CorporationDB::InsertApplication(const ApplicationInfo & aInfo) {
    if (!aInfo.valid) {
        codelog(SERVICE__ERROR, "aInfo contains invalid data");
        return false;
    }

    DBerror err;
    std::string safeMessage;
    sDatabase.DoEscapeString(safeMessage, aInfo.appText);
    if (!sDatabase.RunQuery(err,
        " INSERT INTO chrApplications ("
        " corporationID, characterID, applicationText, roles, grantableRoles, status, "
        " applicationDateTime, deleted, lastCorpUpdaterID "
        " ) VALUES ( "
        " %u, %u, '%s', " I64u ", " I64u ", %u, " I64u ", %u, %u "
        " ) ", aInfo.corpID, aInfo.charID, safeMessage.c_str(), aInfo.role,
               aInfo.grantRole, aInfo.status, aInfo.appTime, aInfo.deleted, aInfo.lastCID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
        return false;
    }

    return true;
}

PyRep *CorporationDB::GetApplications(uint32 corpID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT "
        " corporationID, characterID, applicationText, roles, grantableRoles, status, "
        " applicationDateTime, deleted, lastCorpUpdaterID "
        " FROM chrApplications "
        " WHERE corporationID = %u ", corpID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToIndexRowset(res, "characterID");
}

uint32 CorporationDB::GetStationCorporationCEO(uint32 stationID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT corporation.ceoID "
        " FROM corporation "
        " LEFT JOIN staStations "
        " ON staStations.corporationID = corporation.corporationID "
        " WHERE staStations.stationID = %u ", stationID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }
    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "There's either no such station or the station has no corp owner or the corporation has no ceo. Probably there's no such corporation.");
        return 0;
    }
    return row.GetUInt(0);
}

uint32 CorporationDB::GetCorporationCEO(uint32 corpID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT ceoID "
        " FROM corporation "
        " WHERE corporation.corporationID = %u ", corpID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }
    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "There's either no such corp owner or the corporation has no ceo. Probably a buggy db.");
        return 0;
    }
    return row.GetUInt(0);
}

uint32 CorporationDB::GetCloneTypeCostByID(uint32 cloneTypeID) {
	DBQueryResult res;
	if (!sDatabase.RunQuery(res,
		" SELECT basePrice "
		" FROM invTypes "
		" WHERE typeID = %u ", cloneTypeID))
	{
		sLog.Error("CorporationDB","Failed to retrieve basePrice of typeID = %u",cloneTypeID);
	}
	DBResultRow row;
	if (!res.GetRow(row)) {
		sLog.Error("CorporationDB","Query returned no results");
		return 0;
	}
	return row.GetUInt(0);
}

bool CorporationDB::GetCurrentApplicationInfo(uint32 charID, uint32 corpID, ApplicationInfo & aInfo) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT "
        " status, applicationText, applicationDateTime, roles, grantableRoles, lastCorpUpdaterID, deleted "
        " FROM chrApplications "
        " WHERE characterID = %u AND corporationID = %u ",
        charID, corpID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        aInfo.valid = false;
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "There's no previous application.");
        aInfo.valid = false;
        return false;
    }

    aInfo.charID = charID;
    aInfo.corpID = corpID;
    aInfo.status = row.GetUInt(0);
    aInfo.appText = row.GetText(1);
    aInfo.appTime = row.GetUInt64(2);
    aInfo.role = row.GetUInt64(3);
    aInfo.grantRole = row.GetUInt64(4);
    aInfo.lastCID = row.GetUInt(5);
    aInfo.deleted = row.GetUInt(6);
    aInfo.valid = true;
    return true;
}

bool CorporationDB::UpdateApplication(const ApplicationInfo & info) {
    if (!info.valid) {
        codelog(SERVICE__ERROR, "info contains invalid data");
        return false;
    }

    DBerror err;
    std::string clear;
    sDatabase.DoEscapeString(clear, info.appText);
    if (!sDatabase.RunQuery(err,
        " UPDATE chrApplications "
        " SET status = %u, lastCorpUpdaterID = %u, applicationText = '%s' "
        " WHERE corporationID = %u AND characterID = %u ", info.status, info.lastCID, clear.c_str(), info.corpID, info.charID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
        return false;
    }
    return true;
}

bool CorporationDB::DeleteApplication(const ApplicationInfo & info) {
    DBerror err;
    if (!sDatabase.RunQuery(err,
        " DELETE FROM chrApplications "
        " WHERE corporationID = %u AND characterID = %u ", info.corpID, info.charID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
        return false;
    }
    return true;
}

bool CorporationDB::CreateMemberAttributeUpdate(MemberAttributeUpdate & attrib, uint32 newCorpID, uint32 charID) {
    // What are we doing here exactly?
    // Corporation gets a new member
    // it's new to it

    DBQueryResult res;
    DBResultRow row;
    if (!sDatabase.RunQuery(res,
        " SELECT "
        "   title, corporationDateTime, corporationID, "
        "   corpRole, rolesAtAll, rolesAtBase, "
        "   rolesAtHQ, rolesAtOther "
        " FROM character_ "
        " WHERE character_.characterID = %u ", charID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    if (!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Cannot find character in database");
        return false;
    }

    // this could be stored in the db
#define PRN new PyNone()
#define PRI(i) new PyInt(i)
#define PRL(i) new PyLong(i)
#define PRS(s) new PyString(s)
#define PRNI(i) (row.IsNull(i) ? PRL(0) : PRL(row.GetUInt64(i)))
#define F(name, o, n) \
    attrib.name##Old = o; \
    attrib.name##New = n

    //element                   Old Value               New Value
    F(accountKey,               PRN,                    PRN);
    // i don't even know what this could refer to
    F(baseID,                   PRN,                    PRN);
    F(characterID,              PRN,                    PRI(charID));
    F(corporationID,            PRI(row.GetUInt(2)),    PRI(newCorpID));
    // these also have to be queried from the db
    F(divisionID,               PRN,                    PRN);
    F(roles,                    PRNI(3),                PRI(0));
    F(grantableRoles,           PRNI(4),                PRI(0));
    F(grantableRolesAtBase,     PRNI(5),                PRI(0));
    F(grantableRolesAtHQ,       PRNI(6),                PRI(0));
    F(grantableRolesAtOther,    PRNI(7),                PRI(0));
    F(squadronID,               PRN,                    PRN);
    F(startDateTime,            PRL(row.GetUInt64(1)),  PRL(Win32TimeNow()));
    // another one i have no idea
    F(titleMask,                PRN,                    PRI(0));
    F(baseID,                   PRS(row.GetText(0)),    PRS(""));
#undef F
#undef PRN
#undef PRI
#undef PRS
#undef PRNI

    return true;
}
bool CorporationDB::UpdateDivisionNames(uint32 corpID, const Call_UpdateDivisionNames & divs, PyDict * notif) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT "
        " division1, division2, division3, division4, division5, division6, division7 "
        " FROM corporation "
        " WHERE corporationID = %u ", corpID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Corporation %u doesn't exists.", corpID);
        return false;
    }

    // We are here, so something must have changed...
    std::vector<std::string> dbQ;
    ProcessStringChange("division1", row.GetText(0), divs.div1, notif, dbQ);
    ProcessStringChange("division2", row.GetText(1), divs.div2, notif, dbQ);
    ProcessStringChange("division3", row.GetText(2), divs.div3, notif, dbQ);
    ProcessStringChange("division4", row.GetText(3), divs.div4, notif, dbQ);
    ProcessStringChange("division5", row.GetText(4), divs.div5, notif, dbQ);
    ProcessStringChange("division6", row.GetText(5), divs.div6, notif, dbQ);
    ProcessStringChange("division7", row.GetText(6), divs.div7, notif, dbQ);

    std::string query = " UPDATE corporation SET ";

    int N = dbQ.size();
    for (int i = 0; i < N; i++) {
        query = dbQ[i];
        if (i < N - 1) query += ", ";
    }

    query += " WHERE corporationID = %u";

    if ((N > 0) && (!sDatabase.RunQuery(res.error, query.c_str(), corpID))) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    return true;
}

bool CorporationDB::UpdateCorporation(uint32 corpID, const Call_UpdateCorporation & upd, PyDict * notif) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT description, url, taxRate "
        " FROM corporation "
        " WHERE corporationID = %u ", corpID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Corporation %u doesn't exists.", corpID);
        return false;
    }

    std::vector<std::string> dbQ;
    ProcessStringChange("description", row.GetText(0), upd.description, notif, dbQ);
    ProcessStringChange("url", row.GetText(1), upd.address, notif, dbQ);
    ProcessRealChange("taxRate", row.GetDouble(2), upd.tax, notif, dbQ);

    std::string query = " UPDATE corporation SET ";

    int N = dbQ.size();
    for (int i = 0; i < N; i++) {
        query += dbQ[i];
        if (i < N - 1) query += ", ";
    }

    query += " WHERE corporationID = %u";

    // only update if there is anything to update
    if ((N > 0) && (!sDatabase.RunQuery(res.error, query.c_str(), corpID))) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    return true;

}
#define NI(i) row.IsNull(i) ? 0 : row.GetInt(i)
bool CorporationDB::UpdateLogo(uint32 corpID, const Call_UpdateLogo & upd, PyDict * notif) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT shape1, shape2, shape3, color1, color2, color3, typeface "
        " FROM corporation "
        " WHERE corporationID = %u ", corpID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Corporation %u doesn't exists.", corpID);
        return false;
    }

    std::vector<std::string> dbQ;
    ProcessIntChange("shape1", NI(0), upd.shape1, notif, dbQ);
    ProcessIntChange("shape2", NI(1), upd.shape2, notif, dbQ);
    ProcessIntChange("shape3", NI(2), upd.shape3, notif, dbQ);

    ProcessIntChange("color1", NI(3), upd.color1, notif, dbQ);
    ProcessIntChange("color2", NI(4), upd.color2, notif, dbQ);
    ProcessIntChange("color3", NI(5), upd.color3, notif, dbQ);

    std::string query = " UPDATE corporation SET ";

    int N = dbQ.size();
    for (int i = 0; i < N; i++) {
        query += dbQ[i];
        if (i < N - 1) query += ", ";
    }

    query += " WHERE corporationID = %u ";
    if ((N > 0) && (!sDatabase.RunQuery(res.error, query.c_str(), corpID))) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    return true;
}
#undef NI

//replace all the typeID of the character's clones
bool CorporationDB::ChangeCloneType(uint32 characterID, uint32 typeID) {
    DBQueryResult res;

    if(sDatabase.RunQuery(res,
        "SELECT "
        " typeID, typeName "
        "FROM "
        " invTypes "
        "WHERE typeID = %u",
        typeID))
    {
		_log(DATABASE__ERROR, "Failed to change clone type of char %u: %s.", characterID, res.error.c_str());
		return false;
	}

    DBResultRow row;
    if( !(res.GetRow(row)) )
    {
        sLog.Error( "CorporationDB::ChangeCloneType()", "Could not find Clone typeID = %u in invTypes table.", typeID );
        return false;
    }
    std::string typeNameString = row.GetText(1);

	if(sDatabase.RunQuery(res,
		"UPDATE "
		"entity "
		"SET typeID=%u, itemName='%s' "
		"where ownerID=%u "
		"and flag='400'",
		typeID,
        typeNameString.c_str(),
		characterID))
	{
		_log(DATABASE__ERROR, "Failed to change clone type of char %u: %s.", characterID, res.error.c_str());
		return false;
	}
    sLog.Debug( "CorporationDB", "Clone upgrade successful" );
	return true;
}








