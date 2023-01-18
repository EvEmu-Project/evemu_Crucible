
 /**
  * @name DBCleaner.cpp
  *   Database cleaner module
  * @Author: James
  * @date:   10 October 2021
  */

#include "DBCleaner.h"
#include "database/EVEDBUtils.h"

// This module exists to clean up the database from stuff which could persist in the DB due to crashes.

void DBCleaner::Initialize() {
    sLog.Blue("        DBCleaner", "Database Cleaner Initialized.");

    // Add new cleaning jobs below
    CleanEntity(EVEDB::invTypes::CynosuralFieldI); //Clean cynosural fields
    CleanGroupFromSpace(EVEDB::invGroups::Scanner_Probe); //Clean scanner probes floating in space
    CleanGroupFromSpace(EVEDB::invGroups::Survey_Probe); //Clean survey probes floating in space
    CleanOrphanedWormholes(); //Clean up orphaned wormhole entities
    CleanDungeonEntities();

    // Don't add anything below this line
    sLog.Blue("        DBCleaner", "Cleaning complete.");
}

void DBCleaner::CleanEntity(uint32 type) {
    DBerror err;
    // Delete entity attributes associated with removed entities
    sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID IN (SELECT itemID FROM entity WHERE typeID = %u);", type);
    // Delete entities themselves
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE typeID = %u", type); 
}

void DBCleaner::CleanGroupFromSpace(uint32 groupID) {
    DBerror err;
    // Delete entity attributes associated with removed entities
    sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID IN "
    "(SELECT itemID FROM entity "
    "INNER JOIN invTypes USING (typeID) "
    "WHERE groupID = %u)", groupID);
    // Delete entities themselves
    sDatabase.RunQuery(err, "DELETE entity FROM entity "
    "INNER JOIN invTypes USING (typeID) "
    "WHERE locationID >= 30000000 AND locationID <= 32000000 AND groupID = %u", groupID); 
}

void DBCleaner::CleanOrphanedWormholes() {
    DBerror err;
    // Delete wormholes which don't have any reference in the sysSignatures table
    sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID IN "
    "(SELECT itemID FROM entity "
    "INNER JOIN invTypes USING (typeID) "
    "WHERE itemID NOT IN (SELECT sigItemID FROM sysSignatures) "
    "AND groupID = %u)", EVEDB::invGroups::Wormhole);

    sDatabase.RunQuery(err, "DELETE entity FROM entity "
    "INNER JOIN invTypes USING (typeID) "
    "WHERE itemID NOT IN (SELECT sigItemID FROM sysSignatures) "
    "AND locationID >= 30000000 AND locationID <= 32000000 AND groupID = %u", EVEDB::invGroups::Wormhole); 
}

void DBCleaner::CleanDungeonEntities() {
    DBerror err;
    // Delete entities from live dungeons which should not persist across server restarts
    sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID IN "
    "(SELECT itemID FROM entity "
    "WHERE customInfo LIKE '%%livedungeon%%')");

    sDatabase.RunQuery(err, "DELETE FROM entity WHERE "
    "customInfo LIKE '%%livedungeon%%'");
}