/*
 * This SQL batch drops some columns which are no longer needed.
 * 
 * characterName, typeID -> LEFT JOIN entity
 * bloodlineID           -> LEFT JOIN entity, bloodlineTypes
 * raceID                -> LEFT JOIN entity, bloodlineTypes, chrBloodlines
 *
 * I know it may look difficult but we don't want to store (and manage)
 * duplicates, do we?
 */

ALTER TABLE character_
 DROP COLUMN characterName,
 DROP COLUMN typeID,
 DROP COLUMN bloodlineID,
 DROP COLUMN raceID;

/*
 * We don't drop characterStatic's columns characterName
 * and typeID because prime_db.sql uses these columns to
 * properly populate entity and eveStaticOwners tables.
 */  
ALTER TABLE characterStatic
 DROP COLUMN bloodlineID,
 DROP COLUMN raceID;

