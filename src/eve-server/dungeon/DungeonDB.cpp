
/**
 * @name DungeonDB.cpp
 *   Dungeon DataBase methods
 *   Copyright 2018  Alasiya-EVEmu Team
 *
 * @Author:    Allan
 * @date:      13 December 2018
 *
 */


/* Dungeon Logging
 * DUNG__ERROR
 * DUNG__WARNING
 * DUNG__INFO
 * DUNG__MESSAGE
 * DUNG__TRACE
 * DUNG__CALL
 * DUNG__CALL_DUMP
 * DUNG__RSP_DUMP
 * DUNG__DB_ERROR
 * DUNG__DB_WARNING
 * DUNG__DB_INFO
 * DUNG__DB_MESSAGE
 */

#include "eve-server.h"

#include "DungeonDB.h"


void DungeonDB::GetTemplates(DBQueryResult& res, int32 userID)
{
    if (!sDatabase.RunQuery(res, "SELECT dunTemplateID as templateID, dunTemplateName as templateName, "
    "dunTemplateDescription as description, "
    "%u as userID "
    "FROM dunTemplates", userID))
        _log(DATABASE__ERROR, "Error in GetDunTemplates query: %s", res.error.c_str());
}

void DungeonDB::GetFactions(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT factionID FROM facFactions"))
    _log(DATABASE__ERROR, "Error in GetFactions query: %s", res.error.c_str());
}

void DungeonDB::GetRooms(uint32 dungeonID, DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT roomID, roomName "
    "FROM dunRooms "
    "WHERE dungeonID=%u", dungeonID))
    _log(DATABASE__ERROR, "Error in GetFactions query: %s", res.error.c_str());
}

void DungeonDB::GetArchetypes(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT archetypeID, archetypeName FROM dunArchetypes"))
    _log(DATABASE__ERROR, "Error in GetFactions query: %s", res.error.c_str());
}

void DungeonDB::GetDungeons(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT dungeonID, dungeonName, dungeonStatus, NULL as dungeonNameID, factionID, archetypeID "
    "FROM dunDungeons"))
    _log(DATABASE__ERROR, "Error in GetFactions query: %s", res.error.c_str());
}

void DungeonDB::GetDungeons(uint32 dungeonID, DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT dungeonID, dungeonName, dungeonStatus, NULL as dungeonNameID, factionID, archetypeID "
    "FROM dunDungeons "
    "WHERE dungeonID=%u", dungeonID))
    _log(DATABASE__ERROR, "Error in GetFactions query: %s", res.error.c_str());
}

void DungeonDB::GetDungeons(uint32 archetypeID, uint32 factionID, DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT dungeonID, dungeonName, dungeonStatus, NULL as dungeonNameID, factionID, archetypeID "
    "FROM dunDungeons "
    "WHERE archetypeID=%u AND factionID=%u", archetypeID, factionID))
    _log(DATABASE__ERROR, "Error in GetFactions query: %s", res.error.c_str());
}

void DungeonDB::GetGroups(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT groupID, groupName FROM dunGroups"))
    _log(DATABASE__ERROR, "Error in GetFactions query: %s", res.error.c_str());
}

PyRep* DungeonDB::GetRoomGroups(uint32 roomID)
{
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
            "SELECT "
                " groupID AS selectionGroupID,"
                " groupName AS selectionGroupName,"
                " dunGroups.yaw,"
                " dunGroups.pitch,"
                " dunGroups.roll"
            " FROM dunRoomObjects "
            " INNER JOIN dunGroups USING (groupID)"
            " WHERE roomID = %u"
            " GROUP BY groupID"))
    {
        _log(DATABASE__ERROR, "Error in GetRoomGroups query: %s", res.error.c_str());
    }

    return DBResultToCRowset(res);
}

uint32 DungeonDB::GetFirstGroupForRoom(uint32 roomID)
{
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
            "SELECT "
                " groupID"
            " FROM dunRoomObjects "
            " WHERE roomID = %u"
            " GROUP BY groupID"
            " ORDER BY groupID ASC"
            " LIMIT 1"))
    {
        _log(DATABASE__ERROR, "Error in GetFirstGroupForRoom query: %s", res.error.c_str());
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return 0;

    return row.GetUInt(0);
}

void DungeonDB::GetRoomObjects(uint32 roomID, std::vector< Dungeon::RoomObject >& into)
{
    DBQueryResult res;

    if (!sDatabase.RunQuery(res, "SELECT objectID, roomID, typeID, groupID, x, y, z, yaw, pitch, roll, radius "
    "FROM dunRoomObjects "
    "WHERE roomID=%u", roomID))
    _log(DATABASE__ERROR, "Error in GetRoomObjects query: %s", res.error.c_str());

    _log(DATABASE__RESULTS, "GetRoomObjects returned %lu items", res.GetRowCount());
    DBResultRow row;

    while(res.GetRow(row)) {
        Dungeon::RoomObject entry = Dungeon::RoomObject();
            entry.objectID = row.GetInt(0);
            entry.roomID = row.GetInt(1);
            entry.typeID = row.GetInt(2);
            entry.groupID = row.GetInt(3);
            entry.x = row.GetInt(4);
            entry.y = row.GetInt(5);
            entry.z = row.GetInt(6);
            entry.pitch = row.GetDouble(7);
            entry.roll = row.GetDouble(8);
            entry.yaw = row.GetDouble(9);
            entry.radius = row.GetDouble(10);
        into.push_back(entry);
    }
}

void DungeonDB::GetTemplateObjects(uint32 templateID, std::vector< Dungeon::RoomObject >& into)
{
    DBQueryResult res;

    if (!sDatabase.RunQuery(res, "SELECT objectID, roomID, typeID, groupID, x, y, z, yaw, pitch, roll, radius "
    "FROM dunTemplates INNER JOIN dunRoomObjects ON dunTemplates.dunRoomID = dunRoomObjects.roomID "
    "WHERE dunTemplateID=%u", templateID))
    _log(DATABASE__ERROR, "Error in GetTemplateObjects query: %s", res.error.c_str());

    _log(DATABASE__RESULTS, "GetTemplateObjects returned %lu items", res.GetRowCount());
    DBResultRow row;

    while(res.GetRow(row)) {
        Dungeon::RoomObject entry = Dungeon::RoomObject();
            entry.objectID = row.GetInt(0);
            entry.roomID = row.GetInt(1);
            entry.typeID = row.GetInt(2);
            entry.groupID = row.GetInt(3);
            entry.x = row.GetInt(4);
            entry.y = row.GetInt(5);
            entry.z = row.GetInt(6);
            entry.pitch = row.GetDouble(7);
            entry.roll = row.GetDouble(8);
            entry.yaw = row.GetDouble(9);
            entry.radius = row.GetDouble(10);
        into.push_back(entry);
    }
}

void DungeonDB::EditObjectXYZ(uint32 objectID, double x, double y, double z)
{
    DBerror err;

    if (!sDatabase.RunQuery(err, "UPDATE dunRoomObjects SET x = %f, y = %f, z = %f WHERE objectID = %u", x, y, z, objectID))
        _log(DATABASE__ERROR, "Cannot update object's %u coordinates to %f, %f, %f", objectID, x, y, z);
}

void DungeonDB::EditObjectRadius(uint32 objectID, double radius)
{
    DBerror err;

    if (!sDatabase.RunQuery(err, "UPDATE dunRoomObjects SET radius = %f WHERE objectID = %u", radius, objectID))
        _log(DATABASE__ERROR, "Cannot update object's %u radius to %f", objectID, radius);
}

void DungeonDB::EditObjectYawPitchRoll(uint32 objectID, double yaw, double pitch, double roll)
{
    DBerror err;

    if (!sDatabase.RunQuery(err, "UPDATE dunRoomObjects SET yaw = %f, pitch = %f, roll = %f WHERE objectID = %u", yaw, pitch, roll, objectID))
        _log(DATABASE__ERROR, "Cannot update object's %u rotation to %f, %f, %f", objectID, yaw, pitch, roll);
}

uint32 DungeonDB::CreateObject(uint32 roomID, uint32 typeID, uint32 groupID, double x, double y, double z, double yaw, double pitch, double roll, double radius)
{
    DBerror err;

    uint32 objectID;
    if (!sDatabase.RunQueryLID(err, objectID, "INSERT INTO dunRoomObjects (roomID, typeID, groupID, x, y, z, yaw, pitch, roll, radius) VALUES (%u, %u, %u, %f, %f, %f, %f, %f, %f, %f)", roomID, typeID, groupID, x, y, z, yaw, pitch, roll, radius))
        _log(DATABASE__ERROR, "Cannot insert object into room %u", roomID);

    return objectID;
}

void DungeonDB::DeleteObject(uint32 objectID)
{
    DBerror err;

    if (!sDatabase.RunQuery(err, "DELETE FROM dunRoomObjects WHERE objectID = %u", objectID))
        _log(DATABASE__ERROR, "Cannot delete object %u", objectID);
}

void DungeonDB::EditTemplate(uint32 templateID, std::string templateName, std::string templateDescription)
{
    DBerror err;

    std::string templateNameEscaped;
    sDatabase.DoEscapeString(templateNameEscaped, templateName);

    std::string templateDescriptionEscaped;
    sDatabase.DoEscapeString(templateDescriptionEscaped, templateDescription);

    if (!sDatabase.RunQuery(err, "UPDATE dunTemplates SET dunTemplateName = '%s', dunTemplateDescription = '%s' WHERE dunTemplateID = %u", templateNameEscaped.c_str(), templateDescriptionEscaped.c_str(), templateID))
        _log(DATABASE__ERROR, "Cannot update template %u", templateID);
}

void DungeonDB::DeleteTemplate(uint32 templateID)
{
    DBerror err;

    if (!sDatabase.RunQuery(err, "DELETE FROM dunTemplates WHERE dunTemplateID = %u", templateID))
        _log(DATABASE__ERROR, "Cannot delete template %u", templateID);
}

uint32 DungeonDB::CreateTemplate(std::string templateName, std::string templateDescription, uint32 roomID)
{
    DBerror err;

    std::string templateNameEscaped;
    sDatabase.DoEscapeString(templateNameEscaped, templateName);

    std::string templateDescriptionEscaped;
    sDatabase.DoEscapeString(templateDescriptionEscaped, templateDescription);

    uint32 templateID;
    if (!sDatabase.RunQueryLID(err, templateID, "INSERT INTO dunTemplates (dunTemplateName, dunTemplateDescription, dunRoomID) VALUES ('%s', '%s', %u)", templateNameEscaped.c_str(), templateDescriptionEscaped.c_str(), roomID))
        _log(DATABASE__ERROR, "Cannot insert template into room %u", roomID);

    return templateID;
}