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
    Updates:     Allan
*/

#include <boost/algorithm/string.hpp>
#include "eve-server.h"

#include "EntityList.h"
#include "EVEServerConfig.h"
#include "ServiceDB.h"

uint32 ServiceDB::SetClientSeed()
{
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT ClientSeed FROM srvStatus WHERE AI = 1");
    DBResultRow row;
    res.GetRow(row);
    return row.GetInt(0);
}

bool ServiceDB::ValidateAccountName(CryptoChallengePacket& ccp, std::string& failMsg)
{
    if (ccp.user_name.empty()) {
        failMsg = "Account Name is empty.";
        return false;
    }
    if (ccp.user_name.length() < 3) {
        failMsg = "Account Name is too short.";
        return false;
    }
    //if (name.length() < 4)
    //    throw UserError ("CharNameInvalidMinLength");
    if (ccp.user_name.length() > 20) {    //client caps at 24
        failMsg = "Account Name is too long.";
        return false;
    }

    for (const auto cur : badChars)
        if (EvE::icontains(ccp.user_name, cur)) {
            failMsg = "Account Name contains invalid characters.";
            return false;
        }

    return true;
}

bool ServiceDB::GetAccountInformation( CryptoChallengePacket& ccp, AccountData& aData, std::string& failMsg )
{
    //added auto account    -allan 18Jan14   -UD 16Jan18  -ud 15Dec18  -ud failMsgs 15Jun19  -ud type 4Nov20
    std::string eLogin;
    sDatabase.DoEscapeString(eLogin, ccp.user_name);

    DBQueryResult res;
    if ( !sDatabase.RunQuery( res,
        "SELECT accountID, clientID, password, hash, role, type, online, banned, logonCount, lastLogin"
        " FROM account WHERE accountName = '%s'", eLogin.c_str() ) )
    {
        sLog.Error( "ServiceDB", "Error in query: %s.", res.error.c_str() );
        failMsg = "Error in DB Query";
        failMsg += ": Account not found for ";
        failMsg += eLogin;
        //failMsg += res.error.c_str();     // do we wanna sent the db error msg to client?
        return false;
    }

    DBResultRow row;
    if (!res.GetRow( row )) {
        // account not found, create new one if autoAccountRole is not zero (0)
        if (sConfig.account.autoAccountRole > 0) {
            std::string ePass, ePassHash;
            sDatabase.DoEscapeString(ePass, ccp.user_password);
            sDatabase.DoEscapeString(ePassHash, ccp.user_password_hash);
            uint32 accountID = CreateNewAccount( eLogin.c_str(), ePass.c_str(), ePassHash.c_str(), sConfig.account.autoAccountRole);
            if ( accountID > 0 ) {
                // add new account successful, get account info
                return GetAccountInformation(ccp, aData, failMsg);
            } else {
                failMsg = "Failed to create a new account.";
                return false;
            }
        } else {
            failMsg = "That account doesn't exist and AutoAccount is disabled.";
            return false;
        }
    }

    aData.name       = eLogin;
    aData.id         = row.GetInt(0);
    aData.clientID   = row.GetInt(1);
    aData.password   = (row.IsNull(2) ? "" : row.GetText(2));
    aData.hash       = (row.IsNull(3) ? "" : row.GetText(3));
    aData.role       = row.GetInt64(4);
    aData.type       = row.GetUInt(5);
    aData.online     = row.GetInt(6) ? true : false;
    aData.banned     = row.GetInt(7) ? true : false;
    aData.visits     = row.GetInt(8);
    aData.last_login = (row.IsNull(9) ? "" : row.GetText(9));

    return true;
}

bool ServiceDB::UpdateAccountHash( const char* username, std::string & hash )
{
    std::string eLogin, eHash;
    sDatabase.DoEscapeString(eLogin, username);
    sDatabase.DoEscapeString(eHash, hash);

    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE account SET hash='%s' WHERE accountName='%s'", eHash.c_str(), eLogin.c_str())) {
        sLog.Error( "AccountDB", "Unable to update account information for: %s.", username );
        return false;
    }

    return true;
}

bool ServiceDB::IncrementLoginCount( uint32 accountID )
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE account SET lastLogin=now(), logonCount=logonCount+1 WHERE accountID=%u", accountID)) {
        sLog.Error( "AccountDB", "Unable to update account information for accountID %u.", accountID);
        return false;
    }

    return true;
}

uint32 ServiceDB::CreateNewAccount( const char* login, const char* pass, const char* passHash, int64 role )
{
    uint32 accountID(0);
    uint32 clientID(sEntityList.GetClientSeed());

    DBerror err;
    if ( !sDatabase.RunQueryLID( err, accountID,
            "INSERT INTO account ( accountName, password, hash, role, clientID )"
            " VALUES ( '%s', '%s', '%s', %li, %u )",
                    login, pass, passHash, role, clientID ) )
    {
        sLog.Error( "ServiceDB", "Failed to create a new account '%s':'%s': %s.", login, pass, err.c_str() );
        return 0;
    }

    sDatabase.RunQuery(err, "UPDATE srvStatus SET ClientSeed = ClientSeed + 1 WHERE AI = 1");
    return accountID;
}

void ServiceDB::UpdatePassword(uint32 accountID, const char* pass)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE account SET password = '%s' WHERE accountID=%u", pass, accountID);
}

void ServiceDB::SetServerOnlineStatus(bool online) {
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE srvStatus SET Online = %u, Connections = 0, startTime = %s WHERE AI = 1",
        (online ? 1 : 0), (online ? "UNIX_TIMESTAMP(CURRENT_TIMESTAMP)" : "0"));

    //this is only called on startup/shutdown.  reset all char online counts/status'
    sDatabase.RunQuery(err, "UPDATE chrCharacters SET online = 0 WHERE 1");
    sDatabase.RunQuery(err, "UPDATE account SET online = 0 WHERE 1");
    sDatabase.RunQuery( err, "DELETE FROM chrPausedSkillQueue WHERE 1");
}

void ServiceDB::SetAccountOnlineStatus(uint32 accountID, bool online) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE account SET online = %u WHERE accountID= %u ", (online?1:0), accountID)) {
        codelog(DATABASE__ERROR, "Error in query: %s", err.c_str());
    }
}

void ServiceDB::SetAccountBanStatus(uint32 accountID, bool banned) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE account SET banned = %u WHERE accountID = %u", (banned?1:0), accountID)) {
        codelog(DATABASE__ERROR, "Error in query: %s", err.c_str());
    }
}

uint32 ServiceDB::GetStationOwner(uint32 stationID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT corporationID FROM staStations WHERE stationID = %u", stationID)) {
        codelog(DATABASE__ERROR, "Failed to query info for station %u: %s.", stationID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (res.GetRow(row)) {
        return row.GetInt(0);
    } else {
        return 1;
    }
}

bool ServiceDB::GetConstant(const char *name, uint32 &into)
{
    DBQueryResult res;

    std::string escaped;
    sDatabase.DoEscapeString(escaped, name);

    if (!sDatabase.RunQuery(res, "SELECT constantValue FROM eveConstants WHERE constantID='%s'", escaped.c_str() ))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "Unable to find constant %s", name);
        return false;
    }

    into = row.GetUInt(0);

    return true;
}

void ServiceDB::ProcessStringChange(const char* key, const std::string& oldValue, std::string newValue, PyDict* notif, std::vector< std::string >& dbQ)
{
    if (oldValue.compare(newValue) == 0)
        return;
    // add to notification
    PyTuple* val = new PyTuple(2);
    val->items[0] = new PyString(oldValue);
    val->items[1] = new PyString(newValue);
    notif->SetItemString(key, val);

    std::string newEscValue;
    sDatabase.DoEscapeString(newEscValue, newValue);

    std::string qValue = " ";
    qValue += key;
    qValue += " = '";
    qValue += newEscValue.c_str();
    qValue += "' ";
    dbQ.push_back(qValue);
}

void ServiceDB::ProcessRealChange(const char * key, double oldValue, double newValue, PyDict* notif, std::vector<std::string> & dbQ)
{
    if (oldValue == newValue)
        return;
    // add to notification
    PyTuple* val = new PyTuple(2);
    val->items[0] = new PyFloat(oldValue);
    val->items[1] = new PyFloat(newValue);
    notif->SetItemString(key, val);

    int* nullInt(nullptr);
    std::string qValue(key);
    qValue += " = ";
    qValue += fcvt(newValue, 2, nullInt, nullInt);
    dbQ.push_back(qValue);
}

void ServiceDB::ProcessIntChange(const char * key, uint32 oldValue, uint32 newValue, PyDict* notif, std::vector<std::string> & dbQ)
{
    if (oldValue == newValue)
        return;
    // add to notification
    PyTuple* val = new PyTuple(2);
    val->items[0] = new PyInt(oldValue);
    val->items[1] = new PyInt(newValue);
    notif->SetItemString(key, val);

    std::string qValue(key);
    qValue += " = ";
    qValue += std::to_string(newValue);
    dbQ.push_back(qValue);
}

void ServiceDB::ProcessLongChange(const char* key, int64 oldValue, int64 newValue, PyDict* notif, std::vector< std::string >& dbQ)
{
    if (oldValue == newValue)
        return;
    // add to notification
    PyTuple* val = new PyTuple(2);
    val->items[0] = new PyLong(oldValue);
    val->items[1] = new PyLong(newValue);
    notif->SetItemString(key, val);

    std::string qValue(key);
    qValue += " = ";
    qValue += std::to_string(newValue);
    dbQ.push_back(qValue);
}

void ServiceDB::SaveServerStats(double threads, float rss, float vm, float user, float kernel, uint32 items, uint32 bubbles) {
  DBerror err;
  sDatabase.RunQuery(err,
	"UPDATE srvStatus"
	" SET threads = %f,"
	"     rss = %f,"
	"     vm = %f,"
	"     user = %f,"
	"     kernel = %f,"
	"     items = %u,"
    "     bubbles = %u,"
	"     systems = %u,"
    "     npcs = %u,"
    //"     Connections = %u,"
	"     updateTime = UNIX_TIMESTAMP(CURRENT_TIMESTAMP)"
	" WHERE AI = 1",
	    threads, rss, vm, user, kernel, items, bubbles, sEntityList.GetSystemCount(), sEntityList.GetNPCCount()/*, sEntityList.GetConnections()*/);

    _log(DATABASE__INFO, "Server Stats Saved");
}

// lookupService db calls moved here...made no sense in LSCDB file.
PyRep* ServiceDB::LookupChars(const char *match, bool exact) {
    DBQueryResult res;

    std::string matchEsc;
    sDatabase.DoEscapeString(matchEsc, match);
    if (matchEsc == "__ALL__") {
        if (!sDatabase.RunQuery(res,
            "SELECT "
            "   characterID AS ownerID"
            " FROM chrCharacters"
            " WHERE characterID > %u", maxNPCItem))
        {
            _log(DATABASE__ERROR, "Error in LookupChars query: %s", res.error.c_str());
            return nullptr;
        }
    } else {
        if (!sDatabase.RunQuery(res,
            "SELECT "
            "   itemID AS ownerID"
            " FROM entity"
            " WHERE itemName %s '%s'",
            exact?"=":"LIKE", matchEsc.c_str()
        ))
        {
            _log(DATABASE__ERROR, "Error in LookupChars query: %s", res.error.c_str());
            return nullptr;
        }
    }

    return DBResultToRowset(res);
}


PyRep* ServiceDB::LookupOwners(const char *match, bool exact) {
    DBQueryResult res;

    std::string matchEsc;
    sDatabase.DoEscapeString(matchEsc, match);

    // so each row needs "ownerID", "ownerName", and "groupID"
    // ownerID      =  itemID
    // ownerName    =  name
    // groupID      = 1 for character, 2 for corporation, 32 for alliance

    sDatabase.RunQuery(res,
        "SELECT"
        "  characterID AS ownerID,"
        "  characterName AS ownerName,"
        "  1 AS groupID"
        " FROM chrCharacters"
        " WHERE characterName %s '%s'", (exact?"=":"LIKE"), matchEsc.c_str());

    sDatabase.RunQuery(res,
        "SELECT"
        "  corporationID AS ownerID,"
        "  corporationName AS ownerName,"
        "  2 AS groupID"
        " FROM crpCorporation"
        " WHERE corporationName %s '%s'", (exact?"=":"LIKE"), matchEsc.c_str());

    sDatabase.RunQuery(res,
        "SELECT"
        "  corporationID AS ownerID,"
        "  corporationName AS ownerName,"
        "  2 AS groupID"
        " FROM crpCorporation"
        " WHERE tickerName %s '%s'", (exact?"=":"LIKE"), matchEsc.c_str());

    sDatabase.RunQuery(res,
        "SELECT"
        "  allianceID AS ownerID,"
        "  allianceName AS ownerName,"
        "  32 AS groupID"
        " FROM alnAlliance"
        " WHERE allianceName %s '%s'", (exact?"=":"LIKE"), matchEsc.c_str());

    sDatabase.RunQuery(res,
        "SELECT"
        "  allianceID AS ownerID,"
        "  shortName AS ownerName,"
        "  32 AS groupID"
        " FROM alnAlliance"
        " WHERE shortName %s '%s'", (exact?"=":"LIKE"), matchEsc.c_str());

    return DBResultToRowset(res);
}

PyRep* ServiceDB::LookupCorporations(const std::string & search) {
    DBQueryResult res;
    std::string secure;
    sDatabase.DoEscapeString(secure, search);

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   corporationID, corporationName, corporationType "
        " FROM crpCorporation "
        " WHERE corporationName LIKE '%s'", secure.c_str()))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    return DBResultToRowset(res);
}


PyRep* ServiceDB::LookupFactions(const std::string & search) {
    DBQueryResult res;
    std::string secure;
    sDatabase.DoEscapeString(secure, search);

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   factionID, factionName "
        " FROM facFactions "
        " WHERE factionName LIKE '%s'", secure.c_str()))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    return DBResultToRowset(res);
}


PyRep* ServiceDB::LookupCorporationTickers(const std::string & search) {
    DBQueryResult res;
    std::string secure;
    sDatabase.DoEscapeString(secure, search);

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   corporationID, corporationName, tickerName "
        " FROM crpCorporation "
        " WHERE tickerName LIKE '%s'", secure.c_str()))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    return DBResultToRowset(res);
}


PyRep* ServiceDB::LookupStations(const std::string & search) {
    DBQueryResult res;
    std::string secure;
    sDatabase.DoEscapeString(secure, search);

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   stationID, stationName, stationTypeID "
        " FROM staStations "
        " WHERE stationName LIKE '%s'", secure.c_str()))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    return DBResultToRowset(res);
}

//  wtf is this shit?
PyRep* ServiceDB::LookupKnownLocationsByGroup(const std::string & search, uint32 typeID) {
    DBQueryResult res;
    std::string secure;
    sDatabase.DoEscapeString(secure, search);

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   itemID, itemName, typeID "
        " FROM entity "
        " WHERE itemName LIKE '%s' AND typeID = %u", secure.c_str(), typeID))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    return DBResultToRowset(res);
}

/** @todo look into this...may be wrong */
PyRep* ServiceDB::PrimeOwners(std::vector< int32 >& itemIDs)
{
    DBQueryResult res;
    DBResultRow row;
    PyDict* dict = new PyDict();
    for (auto cur : itemIDs) {
        if (IsCharacterID(cur)) {
            sDatabase.RunQuery(res, "SELECT characterID, characterName, typeID FROM chrCharacters WHERE characterID = %u", cur);
        } else if (IsPlayerCorp(cur)) {
            sDatabase.RunQuery(res, "SELECT corporationID, corporationName, typeID FROM crpCorporation WHERE corporationID = %u", cur);
        } else if (IsAlliance(cur)) {
            sDatabase.RunQuery(res, "SELECT allianceID, allianceName, typeID FROM alnAlliance WHERE allianceID = %u", cur);
        } else {
            ; // make error
        }
        if (res.GetRow(row)) {
            PyList* list = new PyList();
                list->AddItem(new PyInt(row.GetInt(0)));
                list->AddItem(new PyString(row.GetText(1)));
                list->AddItem(new PyInt(row.GetInt(2)));
            dict->SetItem(new PyInt(row.GetInt(0)), list);
        }
    }

    return dict;
}

void ServiceDB::GetCorpHangarNames(uint32 corpID, std::map<uint8, std::string> &hangarNames) {
    std::string table = "crpWalletDivisons";
    if (IsNPCCorp(corpID))
        table = "crpNPCWalletDivisons";
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT division1,division2,division3,division4,division5,division6,division7"
        " FROM %s"
        " WHERE corporationID = %u", table.c_str(), corpID))
    {
        codelog(CORP__DB_ERROR, "Error in retrieving corporation's data (%u)", corpID);
        return;
    }

    DBResultRow row;
    if (res.GetRow(row)) {
        hangarNames.emplace(flagHangar, row.GetText(0));
        hangarNames.emplace(flagCorpHangar2, row.GetText(1));
        hangarNames.emplace(flagCorpHangar3, row.GetText(2));
        hangarNames.emplace(flagCorpHangar4, row.GetText(3));
        hangarNames.emplace(flagCorpHangar5, row.GetText(4));
        hangarNames.emplace(flagCorpHangar6, row.GetText(5));
        hangarNames.emplace(flagCorpHangar7, row.GetText(6));
    } else {
        _log(CORP__DB_ERROR, "CorpID %u has no division data.", corpID);
    }
}

void ServiceDB::SaveKillOrLoss(KillData &data) {
    DBerror err;
    sDatabase.RunQuery(err,
            " INSERT INTO chrKillTable (solarSystemID, victimCharacterID, victimCorporationID, victimAllianceID, victimFactionID,"
            "victimShipTypeID, victimDamageTaken, finalCharacterID, finalCorporationID, finalAllianceID, finalFactionID, finalShipTypeID,"
            "finalWeaponTypeID, finalSecurityStatus, finalDamageDone, killBlob, killTime, moonID)"
            " VALUES (%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%f,%u,'%s',%li,%u)",
                data.solarSystemID, data.victimCharacterID, data.victimCorporationID,
                data.victimAllianceID, data.victimFactionID, data.victimShipTypeID, data.victimDamageTaken,
                data.finalCharacterID, data.finalCorporationID, data.finalAllianceID, data.finalFactionID,
                data.finalShipTypeID, data.finalWeaponTypeID, data.finalSecurityStatus, data.finalDamageDone,
                data.killBlob.c_str(), data.killTime, data.moonID);
}
