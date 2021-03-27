
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


void DungeonDB::GetDunTemplates(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT dunTemplateID, dunTemplateName, dunEntryID, dunSpawnID, dunRoomID FROM dunTemplates"))
        _log(DATABASE__ERROR, "Error in GetDunTemplates query: %s", res.error.c_str());
}
