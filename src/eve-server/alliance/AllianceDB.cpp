/**
 * @name AllianceDB.cpp
 *      database methods for alliance data
 *
 * @author Allan - Updates: James
 * @date 24 May 2019
 *
 */


#include "alliance/AllianceDB.h"


void AllianceDB::AddBulletin(uint32 allyID, uint32 ownerID, uint32 cCharID, std::string& title, std::string& body)
{
    DBerror err;
    sDatabase.RunQuery(err,
            "INSERT INTO alnBulletins (allianceID, ownerID, createCharacterID, createDateTime, editCharacterID, editDateTime, title, body)"
            " VALUES (%u, %u, %u, %f, %u, %f, '%s', '%s')",
            allyID, ownerID, cCharID, GetFileTimeNow(), cCharID, GetFileTimeNow(), title.c_str(), body.c_str());
}

void AllianceDB::EditBulletin(uint32 bulletinID, uint32 eCharID, int64 eDataTime, std::string& title, std::string& body)
{
    DBerror err;
    sDatabase.RunQuery(err,
            "UPDATE alnBulletins SET editCharacterID = %u, editDateTime = %li, title = '%s', body = '%s'"
            " WHERE bulletinID = %u",
            eCharID, eDataTime, title.c_str(), body.c_str(), bulletinID);
}

PyRep* AllianceDB::GetBulletins(uint32 allyID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT allianceID, bulletinID, ownerID, createCharacterID, createDateTime, editCharacterID, editDateTime, title, body"
        " FROM alnBulletins"
        " WHERE allianceID = %u ", allyID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyObject *AllianceDB::GetAlliance(uint32 allyID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT "
        " a.allianceID, a.allianceName, a.typeID, a.shortName, a.executorCorpID, a.creatorCorpID, "
        " a.creatorCharID, a.startDate, a.memberCount, a.url, a.deleted"
        " FROM alnAlliance AS a"
        " WHERE allianceID = %u", allyID))
    {
        codelog(ALLY__DB_ERROR, "Error in retrieving alliance's data (%u)", allyID);
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(ALLY__DB_WARNING, "Unable to find alliance's data (%u)", allyID);
        return nullptr;
    }

    return DBRowToRow(row);
    //return DBResultToRowset(res);
}

PyRep *AllianceDB::GetMyApplications(uint32 charID) {
    //    header = [applicationID, corporationID, characterID, applicationText, roles, grantableRoles, status, applicationDateTime, deleted, lastCorpUpdaterID]
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT applicationID, corporationID, characterID, applicationText, roles, grantableRoles,"
        " status, applicationDateTime, deleted, lastCorpUpdaterID"
        " FROM alnApplications"
        " WHERE characterID = %u ", charID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    PyObjectEx* obj = DBResultToCRowset(res);
    if (is_log_enabled(CORP__RSP_DUMP))
        obj->Dump(CORP__RSP_DUMP, "");

    return obj;
}

PyRep *AllianceDB::GetApplications(uint32 allyID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT"
        " applicationID, corporationID, characterID, applicationText, roles, grantableRoles, status,"
        " applicationDateTime, deleted, lastCorpUpdaterID"
        " FROM alnApplications"
        " WHERE corporationID = %u AND status NOT IN (%u, %u)",
                            allyID, Corp::AppStatus::AcceptedByCorporation, Corp::AppStatus::RejectedByCorporation))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    PyObjectEx* obj = DBResultToCIndexedRowset(res, "characterID");
    if (is_log_enabled(CORP__RSP_DUMP))
        obj->Dump(CORP__RSP_DUMP, "");

    return obj;
}

bool AllianceDB::GetCurrentApplicationInfo(uint32 allyID, uint32 corpID, Corp::ApplicationInfo & aInfo) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT"
        " applicationID, applicationText, roles, grantableRoles, status,"
        " applicationDateTime, lastCorpUpdaterID, deleted"
        " FROM alnApplications"
        " WHERE characterID = %u AND corporationID = %u",
                            allyID, corpID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", res.error.c_str());
        aInfo.valid = false;
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(ALLY__DB_WARNING, "There's no previous application.");
        aInfo.valid = false;
        return false;
    }

    aInfo.appID = row.GetInt(0);
    aInfo.charID = allyID;
    aInfo.corpID = corpID;
    aInfo.appText = row.GetText(1);
    aInfo.role = row.GetInt64(2);
    aInfo.grantRole = row.GetInt64(3);
    aInfo.status = row.GetInt(4);
    aInfo.appTime = row.GetInt64(5);
    aInfo.lastCID = row.GetInt(6);
    aInfo.deleted = row.GetInt(7);
    aInfo.valid = true;
    return true;
}

bool AllianceDB::InsertApplication(Corp::ApplicationInfo& aInfo) {
    if (!aInfo.valid) {
        _log(ALLY__DB_WARNING, "InsertApplication(): aInfo contains invalid data");
        return false;
    }

    std::string escaped;
    sDatabase.DoEscapeString(escaped, aInfo.appText);
    DBerror err;
    if (!sDatabase.RunQueryLID(err, aInfo.appID,
        " INSERT INTO alnApplications"
        " (corporationID, characterID, applicationText, applicationDateTime)"
        " VALUES (%u, %u, '%s', %li)",
        aInfo.corpID, aInfo.charID, escaped.c_str(), aInfo.appTime))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", err.c_str());
        return false;
    }

    return true;
}

bool AllianceDB::UpdateApplication(const Corp::ApplicationInfo& aInfo) {
    if (!aInfo.valid) {
        _log(ALLY__DB_WARNING, "UpdateApplication(): info contains invalid data");
        return false;
    }

    DBerror err;
    std::string escaped;
    sDatabase.DoEscapeString(escaped, aInfo.appText);
    if (!sDatabase.RunQuery(err,
        " UPDATE alnApplications"
        " SET status = %u, lastCorpUpdaterID = %u, applicationText = '%s'"
        " WHERE applicationID = %u", aInfo.status, aInfo.lastCID, escaped.c_str(), aInfo.appID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", err.c_str());
        return false;
    }
    return true;
}

bool AllianceDB::DeleteApplication(const Corp::ApplicationInfo& aInfo) {
    DBerror err;
    if (!sDatabase.RunQuery(err,
        " DELETE FROM alnApplications"
        " WHERE applicationID = %u", aInfo.appID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", err.c_str());
        return false;
    }
    return true;
}

PyRep* AllianceDB::GetContacts(uint32 allyID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery( res,
        "SELECT contactID, inWatchlist, relationshipID, labelMask"
        " FROM alnContacts WHERE ownerID = %u", allyID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    PyObject* obj = DBResultToIndexRowset(res, "contactID");
    if (is_log_enabled(CORP__RSP_DUMP))
        obj->Dump(CORP__RSP_DUMP, "");

    return obj;
}

void AllianceDB::AddContact(uint32 allyID)
{

}

void AllianceDB::UpdateContact(uint32 allyID)
{

}

PyRep* AllianceDB::GetLabels(uint32 allyID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT labelID, color, name FROM alnLabels WHERE ownerID = %u", allyID)) {
        codelog(DATABASE__ERROR, "Error on query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCIndexedRowset(res, "labelID");
}

void AllianceDB::SetLabel(uint32 allyID, uint32 color, std::string name)
{
    std::string escaped;
    sDatabase.DoEscapeString(escaped, name);

    DBQueryResult res;
    sDatabase.RunQuery(res, "INSERT INTO alnLabels (color, name, ownerID) VALUES (%u, '%s', %u)", color, escaped.c_str(), allyID);
}

void AllianceDB::DeleteLabel(uint32 allyID, uint32 labelID)
{

}

void AllianceDB::EditLabel(uint32 allyID, uint32 labelID, uint32 color, std::string name)
{

}

void AllianceDB::AddEmployment(uint32 allyID, uint32 corpID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO chrEmployment"
        "  (allianceID, corporationID, startDate)"
        " VALUES (%u, %u, %f)", allyID, corpID, GetFileTimeNow()))
    {
        codelog(DATABASE__ERROR, "Error in employment insert query: %s", err.c_str());
    }

    if (!sDatabase.RunQuery(err, "UPDATE alnAlliance SET memberCount = memberCount+1 WHERE allianceID = %u", allyID))
        codelog(ALLY__DB_ERROR, "Error in new corp member increase query: %s", err.c_str());
}

PyRep* AllianceDB::GetEmploymentRecord(uint32 corpID)
{
    DBQueryResult res;
    //do we really need this order by??
    if (!sDatabase.RunQuery(res,
        "SELECT startDate, allianceID, deleted "
        "   FROM crpEmployment "
        "   WHERE corporationID = %u "
        "   ORDER BY startDate DESC", corpID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}
