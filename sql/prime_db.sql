-- This file is here to deal with some tables needing to contain both
-- static and dynamic information. The solution for ease of release is
-- to keep all the static info in a seperate copy of the tables such
-- that it can be loaded into the non-static table.

-- switch off the foreign key checks
SET @OLD_FKC = @@FOREIGN_KEY_CHECKS;
SET @@FOREIGN_KEY_CHECKS = 0;

-- Copy over the static character (agent) info
-- optional cleanup instead of the bulk delete: 
-- delete from character_ where characterID > 1 AND (characterID<3008416 OR characterID>100000000);
TRUNCATE TABLE chrEmployment;
TRUNCATE TABLE character_;
INSERT INTO character_ SELECT *, 0 AS Online FROM characterStatic;

-- Copy over the static entities
-- optional cleanup instead of the bulk delete: 
-- delete from entity where itemID > 1 AND (itemID<3008416 OR itemID>100000000);
TRUNCATE TABLE entity_attributes;
TRUNCATE TABLE entity;
INSERT INTO entity SELECT * FROM entityStatic;

-- Copy over the static corporation info
-- optional cleanup instead of the bulk delete: 
-- delete from corporation WHERE corporationID > 2000000;
TRUNCATE TABLE corporation;
INSERT INTO corporation SELECT * FROM corporationStatic;

-- Copy over the static owner info
-- optional cleanup instead of the bulk delete: 
-- delete from eveStaticOwners where ownerID > 1 AND (ownerID<1000000 OR ownerID>100000000);
TRUNCATE TABLE eveStaticOwners;
-- We dont store it in separate table anymore as we can copy it from elsewhere
-- more or less static record .. we might eventually find a new home for it
INSERT INTO eveStaticOwners
 (ownerID, ownerName, typeID)
 VALUES
 (1, 'EVE System', 0);
-- static characters
INSERT INTO eveStaticOwners
 SELECT ch.characterID, ch.characterName, bl.typeID
 FROM characterStatic AS ch
 JOIN bloodlineTypes AS bl USING (bloodlineID);
-- factions
INSERT INTO eveStaticOwners
 SELECT factionID, factionName, 30 AS typeID
 FROM chrFactions;
-- static corporations
INSERT INTO eveStaticOwners
 SELECT corporationID, corporationName, 2 AS typeID
 FROM corporationStatic;

-- Copy over the static channel info
TRUNCATE TABLE channels;
INSERT INTO channels SELECT * FROM channelsStatic;

-- set the auto-increment lower bounds on the shared tables.
ALTER TABLE `corporation`  AUTO_INCREMENT=2000001;
ALTER TABLE `entity` AUTO_INCREMENT=140000000;

-- switch the checks back again
SET @@FOREIGN_KEY_CHECKS=@OLD_FKC;
