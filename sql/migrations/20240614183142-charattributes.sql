-- Fixed character attributes
-- Removed outdated attributes from chrAncestries and chrBloodlines, adding them to chrAttributes as baseAttributes (PER 20 WIL 20 CHA 19 MEM 20 INT 20)


-- +migrate Up
ALTER TABLE chrAncestries
    DROP COLUMN perception, 
    DROP COLUMN willpower,
    DROP COLUMN charisma,
    DROP COLUMN memory, 
    DROP COLUMN intelligence;

ALTER TABLE chrBloodlines
    DROP COLUMN perception, 
    DROP COLUMN willpower,
    DROP COLUMN charisma,
    DROP COLUMN memory, 
    DROP COLUMN intelligence;

ALTER TABLE chrAttributes
    ADD COLUMN baseAttribute TINYINT(3) UNSIGNED DEFAULT 20 AFTER attributeName;

UPDATE chrAttributes
    SET baseAttribute = 19 
    WHERE attributeName = 'Charisma';

-- +migrate Down

-- Receate chrAncestries
ALTER TABLE chrAttributes   
    DROP COLUMN baseAttribute;

CREATE TABLE tempAncestriesLeft AS
	SELECT ancestryID, ancestryName, bloodlineID, description FROM chrAncestries;
  
CREATE TABLE tempAncestriesRight 
	SELECT ancestryID, shortDescription, iconID, ancestryNameID, descriptionID, dataID FROM chrAncestries; 

CREATE TABLE tempAncestriesCenter(
    ancestryID INT(11) DEFAULT NULL,
    perception TINYINT(3) DEFAULT NULL,
    willpower TINYINT(3) DEFAULT NULL,
    charisma TINYINT(3) DEFAULT NULL,
    memory TINYINT(3) DEFAULT NULL,
    intelligence TINYINT(3) DEFAULT NULL);

INSERT INTO tempAncestriesCenter(ancestryID, perception, willpower, charisma, memory, intelligence) VALUES
    (1,0,1,3,0,0),
    (2,0,0,1,3,0),
    (3,0,4,0,0,0),
    (4,0,0,4,0,0),
    (5,3,0,0,0,1),
    (6,0,4,0,0,0),
    (7,0,0,0,4,0),
    (8,0,4,0,0,0),
    (9,0,2,2,0,0),
    (10,0,0,0,4,0),
    (11,1,0,0,0,3),
    (12,0,4,0,0,0),
    (13,0,0,4,0,0),
    (14,0,0,0,4,0),
    (15,2,2,0,0,0),
    (16,2,0,2,0,0),
    (17,0,0,4,0,0),
    (18,0,0,0,4,0),
    (19,0,0,0,0,4),
    (20,0,0,4,0,0),
    (21,3,1,0,0,0),
    (22,0,0,0,4,0),
    (23,0,3,1,0,0),
    (24,2,2,0,0,0),
    (25,0,3,0,1,0),
    (26,0,0,4,0,0),
    (27,4,0,0,0,0),
    (28,1,0,0,0,3),
    (29,0,0,0,4,0),
    (30,0,4,0,0,0),
    (31,0,0,0,0,4),
    (32,2,2,0,0,0),
    (33,1,0,0,3,0),
    (34,0,3,1,0,0),
    (35,0,0,0,2,2),
    (36,4,0,0,0,0),
    (37,3,0,0,0,1),
    (38,0,0,2,0,2),
    (39,0,2,0,2,0),
    (40,2,0,0,0,2),
    (41,0,3,0,0,1),
    (42,0,0,1,3,0);

DROP TABLE chrAncestries;    

CREATE TABLE chrAncestries AS
SELECT L.*, C.perception, C.willpower, C.charisma, C.memory, C.intelligence, R.shortDescription, R.iconID, R.ancestryNameID, R.descriptionID, R.dataID 
FROM tempAncestriesLeft L
LEFT JOIN tempAncestriesCenter C ON L. ancestryID = C. ancestryID
LEFT JOIN tempAncestriesRight R on L. ancestryID = R. ancestryID;

DROP TABLE tempAncestriesLeft;
DROP TABLE tempAncestriesCenter;
DROP TABLE tempAncestriesRight;

-- Receate chrBloodlines
CREATE TABLE tempBloodlinesLeft AS
	SELECT bloodlineID, bloodlineName, raceID, description, maleDescription, femaleDescription, shipTypeID, corporationID FROM chrBloodlines;
  
CREATE TABLE tempBloodlinesRight 
	SELECT bloodlineID, shortDescription, shortMaleDescription, shortFemaleDescription, iconID, bloodlineNameID, descriptionID, dataID FROM chrBloodlines; 

CREATE TABLE tempBloodlinesCenter(
    bloodlineID INT(11) DEFAULT NULL,
    perception TINYINT(3) DEFAULT NULL,
    willpower TINYINT(3) DEFAULT NULL,
    charisma TINYINT(3) DEFAULT NULL,
    memory TINYINT(3) DEFAULT NULL,
    intelligence TINYINT(3) DEFAULT NULL);

INSERT INTO tempBloodlinesCenter(bloodlineID, perception, willpower, charisma, memory, intelligence) VALUES
    (1,5,5,6,7,7),
    (2,9,6,6,4,5),
    (3,5,6,6,6,7),
    (4,9,7,6,4,4),
    (5,4,10,3,6,7),
    (6,7,4,8,6,5),
    (7,8,4,8,4,6),
    (8,3,6,6,7,8),
    (9,7,8,5,10,10),
    (10,10,9,7,7,7),
    (11,7,6,3,6,8),
    (12,6,7,7,5,5),
    (13,8,8,5,4,5),
    (14,4,3,8,8,7);

DROP TABLE chrBloodlines;    

CREATE TABLE chrBloodlines AS
SELECT L.*, C.perception, C.willpower, C.charisma, C.memory, C.intelligence, R.shortDescription, R.shortMaleDescription, R.shortFemaleDescription, R.iconID, R.bloodlineNameID, R.descriptionID, R.dataID
FROM tempBloodlinesLeft L
LEFT JOIN tempBloodlinesCenter C ON L.bloodlineID = C.bloodlineID
LEFT JOIN tempBloodlinesRight R on L.bloodlineID = R.bloodlineID;

DROP TABLE tempBloodlinesLeft;
DROP TABLE tempBloodlinesCenter;
DROP TABLE tempBloodlinesRight;