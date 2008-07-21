-- This file is here to deal with some tables needing to contain both
-- static and dynamic information. The solution for ease of release is
-- to keep all the static info in a seperate copy of the tables such
-- that it can be loaded into the non-static table.

-- Copy over the static character (agent) info
-- optional cleanup instead of the bulk delete: 
-- delete from character_ where characterID > 1 AND (characterID<3008416 OR characterID>100000000);
DELETE FROM character_;
INSERT INTO character_ SELECT * FROM characterStatic;

-- Copy over the static entities
-- optional cleanup instead of the bulk delete: 
-- delete from entity where itemID > 1 AND (itemID<3008416 OR itemID>100000000);
DELETE FROM entity;
INSERT INTO entity SELECT * FROM entityStatic;

-- Copy over the static corporation info
-- optional cleanup instead of the bulk delete: 
-- delete from corporation WHERE corporationID > 2000000;
DELETE FROM corporation;
INSERT INTO corporation SELECT * FROM corporationStatic;

-- Copy over the static owner info
-- optional cleanup instead of the bulk delete: 
-- delete from eveStaticOwners where ownerID > 1 AND (ownerID<1000000 OR ownerID>100000000);
DELETE FROM eveStaticOwners;
INSERT INTO eveStaticOwners SELECT * FROM eveStaticOwnersStatic;

-- Copy over the static channel info
DELETE FROM channels;
INSERT INTO channels SELECT * FROM channelsStatic;

-- set the auto-increment lower bounds on the shared tables.
ALTER TABLE `corporation`  AUTO_INCREMENT=2000001;
ALTER TABLE `entity` AUTO_INCREMENT=140000000;

