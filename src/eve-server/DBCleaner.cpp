
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
    sLog.Blue("   DBCleaner", "Database Cleaner Initialized.");

    // Add new cleaning jobs below
    CleanEntity(EVEDB::invTypes::CynosuralFieldI); //Clean cynosural fields

    // Don't add anything below this line
    sLog.Blue("   DBCleaner", "Cleaning complete.");
}

void DBCleaner::CleanEntity(uint32 type) {
    DBerror err;
    // Delete entity attributes associated with removed entities
    sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID IN (SELECT itemID FROM entity WHERE typeID = %u);", type);
    // Delete entities themselves
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE typeID = %u", type); 
}
