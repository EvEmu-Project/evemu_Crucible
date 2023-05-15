/**
 * @name AllianceDB.cpp
 *      database methods for alliance data
 *
 * @author Allan
 * @updates James
 * @date 24 May 2019
 *
 */

#include "Client.h"
#include "StaticDataMgr.h"
#include "character/Character.h"
#include "alliance/AllianceDB.h"

void AllianceDB::AddBulletin(uint32 allyID, uint32 ownerID, uint32 cCharID, const std::string &title, const std::string &body)
{
    DBerror err;
    sDatabase.RunQuery(err,
                       "INSERT INTO alnBulletins (allianceID, ownerID, createCharacterID, createDateTime, editCharacterID, editDateTime, title, body)"
                       " VALUES (%u, %u, %u, %f, %u, %f, '%s', '%s')",
                       allyID, ownerID, cCharID, GetFileTimeNow(), cCharID, GetFileTimeNow(), title.c_str(), body.c_str());
}

void AllianceDB::EditBulletin(uint32 bulletinID, uint32 eCharID, int64 eDataTime, std::string &title, std::string &body)
{
    DBerror err;
    sDatabase.RunQuery(err,
                       "UPDATE alnBulletins SET editCharacterID = %u, editDateTime = %lli, title = '%s', body = '%s'"
                       " WHERE bulletinID = %u",
                       eCharID, eDataTime, title.c_str(), body.c_str(), bulletinID);
}

void AllianceDB::DeleteBulletin(uint32 bulletinID)
{
    DBerror err;
    sDatabase.RunQuery(err,
                       "DELETE from alnBulletins "
                       " WHERE bulletinID = %u",
                       bulletinID);
}

PyRep *AllianceDB::GetBulletins(uint32 allyID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                            "SELECT allianceID, bulletinID, ownerID, createCharacterID, createDateTime, editCharacterID, editDateTime, title, body"
                            " FROM alnBulletins"
                            " WHERE allianceID = %u ",
                            allyID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep *AllianceDB::GetAlliance(uint32 allyID)
{
    // called by alliance member
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                            " SELECT "
                            " a.allianceID, a.allianceName, a.description, a.typeID, a.shortName, a.executorCorpID, a.creatorCorpID, "
                            " a.creatorCharID, a.startDate, a.memberCount, a.url, a.deleted, 0 as dictatorial " //Dictatorial is not used in Crucible but must be set
                            " FROM alnAlliance AS a"
                            " WHERE allianceID = %u",
                            allyID))
    {
        codelog(ALLY__DB_ERROR, "Error in retrieving alliance's data (%u)", allyID);
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row))
    {
        codelog(ALLY__DB_WARNING, "Unable to find alliance's data (%u)", allyID);
        return nullptr;
    }

    //return DBRowToRow(row);
    return DBRowToPackedRow(row);
    //return DBResultToRowset(res);
}

PyRep *AllianceDB::GetMyApplications(uint32 corpID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                            " SELECT applicationID, corporationID, allianceID, applicationText, "
                            " state, applicationDateTime, deleted "
                            " FROM alnApplications"
                            " WHERE corporationID = %u ",
                            corpID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    PyObjectEx *obj = DBResultToCIndexedRowset(res, "corporationID");
    if (is_log_enabled(CORP__RSP_DUMP))
        obj->Dump(CORP__RSP_DUMP, "");

    return obj;
}

PyRep *AllianceDB::GetApplications(uint32 allyID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                            " SELECT applicationID, corporationID, allianceID, applicationText, "
                            " state, applicationDateTime, deleted "
                            " FROM alnApplications"
                            " WHERE allianceID = %u AND state NOT IN (%u, %u)",
                            allyID, EveAlliance::AppStatus::AppAccepted, EveAlliance::AppStatus::AppRejected))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    PyObjectEx *obj = DBResultToCIndexedRowset(res, "corporationID");
    if (is_log_enabled(CORP__RSP_DUMP))
        obj->Dump(CORP__RSP_DUMP, "");

    return obj;
}

bool AllianceDB::GetCurrentApplicationInfo(uint32 allyID, uint32 corpID, Alliance::ApplicationInfo &aInfo)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                            " SELECT applicationID, corporationID, allianceID, applicationText, "
                            " state, applicationDateTime, deleted "
                            " FROM alnApplications"
                            " WHERE corporationID = %u AND allianceID = %u",
                            corpID, allyID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", res.error.c_str());
        aInfo.valid = false;
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row))
    {
        codelog(ALLY__DB_WARNING, "There's no previous application.");
        aInfo.valid = false;
        return false;
    }

    aInfo.appID = row.GetInt(0);
    aInfo.allyID = allyID;
    aInfo.corpID = corpID;
    aInfo.appText = row.GetText(3);
    aInfo.state = row.GetInt(4);
    aInfo.appTime = row.GetInt64(5);
    aInfo.deleted = row.GetInt(6);
    aInfo.valid = true;
    return true;
}

bool AllianceDB::InsertApplication(Alliance::ApplicationInfo &aInfo)
{
    if (!aInfo.valid)
    {
        _log(ALLY__DB_WARNING, "InsertApplication(): aInfo contains invalid data");
        return false;
    }

    std::string escaped;
    sDatabase.DoEscapeString(escaped, aInfo.appText);
    DBerror err;
    if (!sDatabase.RunQueryLID(err, aInfo.appID,
                               " INSERT INTO alnApplications"
                               " (corporationID, allianceID, applicationText, state, applicationDateTime)"
                               " VALUES (%u, %u, '%s', %u, %lli)",
                               aInfo.corpID, aInfo.allyID, escaped.c_str(), aInfo.state, GetFileTimeNow()))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", err.c_str());
        return false;
    }

    return true;
}

bool AllianceDB::UpdateApplication(const Alliance::ApplicationInfo &aInfo)
{
    if (!aInfo.valid)
    {
        _log(ALLY__DB_WARNING, "UpdateApplication(): info contains invalid data");
        return false;
    }

    DBerror err;
    std::string escaped;
    sDatabase.DoEscapeString(escaped, aInfo.appText);
    if (!sDatabase.RunQuery(err,
                            " UPDATE alnApplications"
                            " SET state = %u, applicationText = '%s'"
                            " WHERE corporationID = %u and state = 1",
                            aInfo.state, escaped.c_str(), aInfo.corpID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", err.c_str());
        return false;
    }
    return true;
}

bool AllianceDB::DeleteApplication(const Alliance::ApplicationInfo &aInfo)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
                            " DELETE FROM alnApplications"
                            " WHERE corporationID = %u and allianceID = %u ",
                            aInfo.corpID, aInfo.allyID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", err.c_str());
        return false;
    }
    return true;
}

PyRep *AllianceDB::GetContacts(uint32 allyID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                            "SELECT contactID, inWatchlist, relationshipID, labelMask"
                            " FROM alnContacts WHERE ownerID = %u",
                            allyID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    PyObjectEx *obj = DBResultToCIndexedRowset(res, "contactID");
    if (is_log_enabled(CORP__RSP_DUMP))
        obj->Dump(CORP__RSP_DUMP, "");

    return obj;
}

void AllianceDB::AddContact(uint32 ownerID, int32 contactID, int32 relationshipID)
{
    DBerror err;
    sDatabase.RunQuery(err,
                       "INSERT INTO alnContacts (ownerID, contactID, relationshipID, "
                       " inWatchlist, labelMask) VALUES "
                       " (%u, %u, %i, 0, 0) ",
                       ownerID, contactID, relationshipID);
}

void AllianceDB::UpdateContact(int32 relationshipID, uint32 contactID, uint32 ownerID)
{
    DBerror err;
    sDatabase.RunQuery(err,
                       "UPDATE alnContacts SET relationshipID=%i "
                       " WHERE contactID=%u AND ownerID=%u ",
                       relationshipID, contactID, ownerID);
}

void AllianceDB::RemoveContact(uint32 contactID, uint32 ownerID)
{
    DBerror err;
    sDatabase.RunQuery(err,
                       "DELETE from alnContacts "
                       " WHERE contactID=%u AND ownerID=%u ",
                       contactID, ownerID);
}

PyRep *AllianceDB::GetLabels(uint32 allyID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT labelID, color, name FROM alnLabels WHERE ownerID = %u", allyID))
    {
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

bool AllianceDB::AddEmployment(uint32 allyID, uint32 corpID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
                            "INSERT INTO crpEmployment"
                            "  (allianceID, corporationID, startDate)"
                            " VALUES (%u, %u, %f)",
                            allyID, corpID, GetFileTimeNow()))
    {
        codelog(DATABASE__ERROR, "Error in employment insert query: %s", err.c_str());
    }

    if (!sDatabase.RunQuery(err, "UPDATE alnAlliance SET memberCount = memberCount+1 WHERE allianceID = %u", allyID))
        codelog(ALLY__DB_ERROR, "Error in new corp member increase query: %s", err.c_str());
    return true;
}

PyRep *AllianceDB::GetEmploymentRecord(uint32 corpID)
{
    DBQueryResult res;
    //do we really need this order by??
    if (!sDatabase.RunQuery(res,
                            "SELECT startDate, allianceID, deleted "
                            "   FROM crpEmployment "
                            "   WHERE corporationID = %u "
                            "   ORDER BY startDate DESC",
                            corpID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

uint32 AllianceDB::GetExecutorID(uint32 allyID)
{
    uint32 executorID = 0;
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                            "SELECT executorCorpID FROM alnAlliance "
                            " WHERE allianceID = %u",
                            allyID))
    {
        codelog(DATABASE__ERROR, "Error in checking current alliance executorCorpID: %s", res.error.c_str());
    }

    DBResultRow row;
    while (res.GetRow(row))
    {
        executorID = row.GetUInt(0);
    }

    return executorID;
}

bool AllianceDB::UpdateCorpAlliance(uint32 allyID, uint32 corpID)
{
    uint32 executorID = GetExecutorID(allyID);

    DBerror err;
    if (!sDatabase.RunQuery(err,
                            "UPDATE crpCorporation SET "
                            "  allianceID = %u, "
                            "  allianceMemberStartDate = %f, "
                            "  chosenExecutorID = %u "
                            " WHERE corporationID = %u",
                            allyID, GetFileTimeNow(), executorID, corpID))
    {
        codelog(DATABASE__ERROR, "Error in corp alliance update query: %s", err.c_str());
    }
    return true;
}

void AllianceDB::DeleteMember(uint32 allyID, uint32 corpID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
                            "UPDATE crpCorporation SET "
                            "  allianceID = 0, "
                            "  allianceMemberStartDate = 0, "
                            "  chosenExecutorID = 0 "
                            " WHERE corporationID = %u",
                            corpID))
    {
        codelog(DATABASE__ERROR, "Error in deleting corp from alliance: %s", err.c_str());
    }

    if (!sDatabase.RunQuery(err, "UPDATE alnAlliance SET memberCount = memberCount-1 WHERE allianceID = %u", allyID))
        codelog(ALLY__DB_ERROR, "Error in alliance member decrease query: %s", err.c_str());
}

void AllianceDB::DeclareExecutorSupport(uint32 corpID, uint32 chosenExecutor)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
                            "UPDATE crpCorporation SET "
                            "  chosenExecutorID = %u "
                            " WHERE corporationID = %u",
                            chosenExecutor, corpID))
    {
        codelog(DATABASE__ERROR, "Error in setting chosenExecutor: %s", err.c_str());
    }
}

bool AllianceDB::IsShortNameTaken(std::string shortName)
{
    DBQueryResult res;
    sDatabase.RunQuery(res, " SELECT allianceID FROM alnAlliance WHERE shortName = '%s'", shortName.c_str());
    return (res.GetRowCount() != 0);
}

void AllianceDB::UpdateAlliance(uint32 allyID, std::string description, std::string url)
{
    std::string aDesc, aURL;
    sDatabase.DoEscapeString(aDesc, description);
    sDatabase.DoEscapeString(aURL, url);

    DBerror err;

    if (!sDatabase.RunQuery(err,
                            " UPDATE alnAlliance "
                            "  SET description='%s', url='%s' "
                            " WHERE allyID=%u ",
                            aDesc.c_str(), aURL.c_str(), allyID))
    {
        codelog(ALLY__DB_ERROR, "Error in UpdateAlliance query: %s", err.c_str());
    }
}

bool AllianceDB::CreateAlliance(std::string name, std::string shortName, std::string description, std::string url, Client *pClient, uint32 &allyID, uint32 &corpID)
{
    std::string aName, aShort, aDesc, aURL;
    sDatabase.DoEscapeString(aName, name);
    sDatabase.DoEscapeString(aShort, shortName);
    sDatabase.DoEscapeString(aDesc, description);
    sDatabase.DoEscapeString(aURL, url);

    Character *pChar = pClient->GetChar().get();
    uint32 charID = pClient->GetCharacterID();
    corpID = pClient->GetCorporationID();

    DBerror err;

    if (!sDatabase.RunQueryLID(err, allyID,
                               " INSERT INTO alnAlliance ( "
                               "   allianceName, shortName, description, executorCorpID, creatorCorpID, creatorCharID, "
                               "   startDate, memberCount, url )"
                               " VALUES "
                               "   ('%s', '%s', '%s', %u, %u, %u, %f, 0, '%s') ",
                               aName.c_str(), aShort.c_str(), aDesc.c_str(), corpID, corpID, charID, GetFileTimeNow(), aURL.c_str()))
    {
        codelog(ALLY__DB_ERROR, "Error in CreateAlliance query: %s", err.c_str());
        return false;
    }
    // It has to go into the eveStaticOwners too
    sDatabase.RunQuery(err, " INSERT INTO eveStaticOwners (ownerID,ownerName,typeID) VALUES (%u, '%s', 16159)", allyID, aName.c_str());

    return true;
}

PyRep *AllianceDB::GetMembers(uint32 allyID) //to be called by member of alliance
{
    //This function is called to gather all of the corporationIDs associated to a particular alliance
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                            "SELECT corporationID, corporationName, chosenExecutorID "
                            " FROM crpCorporation WHERE allianceID = %u AND deleted = 0 ",
                            allyID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    PyObject *obj = DBResultToIndexRowset(res, "corporationID");
    if (is_log_enabled(CORP__RSP_DUMP))
        obj->Dump(CORP__RSP_DUMP, "");

    return obj;

    //return DBResultToIndexRowset(res);
    //return DBResultToRowset(res);
}

PyRep *AllianceDB::GetAllianceMembers(uint32 allyID) //to be called from show details pane
{
    //This function is called to gather all of the corporationIDs associated to a particular alliance
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                            "SELECT corporationID "
                            " FROM crpCorporation WHERE allianceID = %u AND deleted = 0",
                            allyID))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

// Not sure how alliances but for now this will simply return an ordered list based upon member count
PyRep *AllianceDB::GetRankedAlliances()
{
    //This function is called to gather all of the corporationIDs associated to a particular alliance
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                            "SELECT allianceID,allianceName,executorCorpID, "
                            " description, typeID, shortName, creatorCorpID, "
                            " creatorCharID, startDate, memberCount, url, deleted "
                            " from alnAlliance order by memberCount "))
    {
        codelog(ALLY__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

bool AllianceDB::CreateAllianceChangePacket(OnAllianceChanged &ac, uint32 oldAllyID, uint32 newAllyID)
{
    // New Alliance \/
    if (newAllyID == 0)
    {
        ac.allianceIDNew = PyStatic.NewNone();
        ac.allianceNameNew = PyStatic.NewNone();
        ac.descriptionNew = PyStatic.NewNone();
        ac.typeIDNew = PyStatic.NewNone();
        ac.shortNameNew = PyStatic.NewNone();
        ac.executorCorpIDNew = PyStatic.NewNone();
        ac.creatorCorpIDNew = PyStatic.NewNone();
        ac.creatorCharIDNew = PyStatic.NewNone();
        ac.startDateNew = PyStatic.NewNone();
        ac.memberCountNew = PyStatic.NewNone();
        ac.urlNew = PyStatic.NewNone();
        ac.deletedNew = PyStatic.NewNone();
        ac.dictatorialNew = PyStatic.NewNone();
    }
    else
    {
        DBQueryResult res;
        if (!sDatabase.RunQuery(res,
                                " SELECT "
                                " a.allianceID, a.allianceName, a.description, a.typeID, a.shortName, a.executorCorpID, a.creatorCorpID, "
                                " a.creatorCharID, a.startDate, a.memberCount, a.url, a.deleted, 0 as dictatorial " //Dictatorial is not used in Crucible but must be set
                                " FROM alnAlliance AS a"
                                " WHERE allianceID = %u",
                                newAllyID))
        {
            codelog(ALLY__DB_ERROR, "Error in retrieving new alliance's data (%u)", newAllyID);
            return false;
        }

        DBResultRow row;
        if (!res.GetRow(row))
        {
            codelog(ALLY__DB_WARNING, "Unable to find new alliance's data (%u)", newAllyID);
            return false;
        }

        ac.allianceIDNew = new PyInt(row.GetUInt(0));
        ac.allianceNameNew = new PyString(row.GetText(1));
        ac.descriptionNew = new PyString(row.GetText(2));
        ac.typeIDNew = new PyInt(row.GetUInt(3));
        ac.shortNameNew = new PyString(row.GetText(4));
        ac.executorCorpIDNew = new PyInt(row.GetUInt(5));
        ac.creatorCorpIDNew = new PyInt(row.GetUInt(6));
        ac.creatorCharIDNew = new PyInt(row.GetUInt(7));
        ac.startDateNew = new PyLong(row.GetInt64(8));
        ac.memberCountNew = new PyInt(row.GetUInt(9));
        ac.urlNew = new PyString(row.GetText(10));
        ac.deletedNew = new PyInt(row.GetInt(11));
        ac.dictatorialNew = new PyInt(row.GetInt(12));
    }

    // Old Alliance \/
    if (oldAllyID == 0)
    {
        ac.allianceIDOld = PyStatic.NewNone();
        ac.allianceNameOld = PyStatic.NewNone();
        ac.descriptionOld = PyStatic.NewNone();
        ac.typeIDOld = PyStatic.NewNone();
        ac.shortNameOld = PyStatic.NewNone();
        ac.executorCorpIDOld = PyStatic.NewNone();
        ac.creatorCorpIDOld = PyStatic.NewNone();
        ac.creatorCharIDOld = PyStatic.NewNone();
        ac.startDateOld = PyStatic.NewNone();
        ac.memberCountOld = PyStatic.NewNone();
        ac.urlOld = PyStatic.NewNone();
        ac.deletedOld = PyStatic.NewNone();
        ac.dictatorialOld = PyStatic.NewNone();
    }
    else
    {
        DBQueryResult res;
        if (!sDatabase.RunQuery(res,
                                " SELECT "
                                " a.allianceID, a.allianceName, a.description, a.typeID, a.shortName, a.executorCorpID, a.creatorCorpID, "
                                " a.creatorCharID, a.startDate, a.memberCount, a.url, a.deleted, 0 as dictatorial " //Dictatorial is not used in Crucible but must be set
                                " FROM alnAlliance AS a"
                                " WHERE allianceID = %u",
                                oldAllyID))
        {
            codelog(ALLY__DB_ERROR, "Error in retrieving old alliance's data (%u)", oldAllyID);
            return false;
        }

        DBResultRow row;
        if (!res.GetRow(row))
        {
            codelog(ALLY__DB_WARNING, "Unable to find old alliance's data (%u)", oldAllyID);
            return false;
        }
        ac.allianceIDOld = new PyInt(row.GetUInt(0));
        ac.allianceNameOld = new PyString(row.GetText(1));
        ac.descriptionOld = new PyString(row.GetText(2));
        ac.typeIDOld = new PyInt(row.GetUInt(3));
        ac.shortNameOld = new PyString(row.GetText(4));
        ac.executorCorpIDOld = new PyInt(row.GetUInt(5));
        ac.creatorCorpIDOld = new PyInt(row.GetUInt(6));
        ac.creatorCharIDOld = new PyInt(row.GetUInt(7));
        ac.startDateOld = new PyLong(row.GetInt64(8));
        ac.memberCountOld = new PyInt(row.GetUInt(9));
        ac.urlOld = new PyString(row.GetText(10));
        ac.deletedOld = new PyInt(row.GetInt(11));
        ac.dictatorialOld = new PyInt(row.GetInt(12));
    }

    return true;
}