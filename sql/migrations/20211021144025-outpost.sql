-- Creates tables and columns for Outposts
-- +migrate Up
CREATE TABLE staOutpostServiceConfig (
    stationID INT(10) NOT NULL, 
    serviceID INT(10) NOT NULL, 
    minimumStanding FLOAT NOT NULL DEFAULT 0, 
    minimumCharSecurity FLOAT NOT NULL DEFAULT 0, 
    maximumCharSecurity FLOAT NOT NULL DEFAULT 0, 
    minimumCorpSecurity FLOAT NOT NULL DEFAULT 0, 
    maximumCorpSecurity FLOAT NOT NULL DEFAULT 0, 
    discountPerGoodStandingPoint FLOAT NOT NULL DEFAULT 0, 
    surchargePerBadStandingPoint FLOAT NOT NULL DEFAULT 0);

ALTER TABLE staServices
    ADD COLUMN serviceNameID INT(10) NOT NULL DEFAULT 0 AFTER description;

UPDATE staServices
    SET serviceNameID = 61496 WHERE serviceID = 1; -- Bounty Missions
UPDATE staServices
    SET serviceNameID = 61497 WHERE serviceID = 2; -- Assassination Missions
UPDATE staServices
    SET serviceNameID = 61498 WHERE serviceID = 4; -- Courier Missions
UPDATE staServices
    SET serviceNameID = 61499 WHERE serviceID = 8; -- Interbus
UPDATE staServices
    SET serviceNameID = 61500 WHERE serviceID = 16; -- Reprocessing Plant
UPDATE staServices
    SET serviceNameID = 61501 WHERE serviceID = 32; -- Refinery
UPDATE staServices
    SET serviceNameID = 61502 WHERE serviceID = 64; -- Market
UPDATE staServices
    SET serviceNameID = 61503 WHERE serviceID = 128; -- Black Market
UPDATE staServices
    SET serviceNameID = 61504 WHERE serviceID = 256; -- Stock Exchange
UPDATE staServices
    SET serviceNameID = 61505 WHERE serviceID = 512; -- Cloning
UPDATE staServices
    SET serviceNameID = 61506 WHERE serviceID = 1024; -- Surgery
UPDATE staServices
    SET serviceNameID = 61507 WHERE serviceID = 2048; -- DNA Therapy
UPDATE staServices
    SET serviceNameID = 61508 WHERE serviceID = 4096; -- Repair Facilities
UPDATE staServices
    SET serviceNameID = 61509 WHERE serviceID = 8192; -- Factory
UPDATE staServices
    SET serviceNameID = 61510 WHERE serviceID = 16384; -- Laboratory
UPDATE staServices
    SET serviceNameID = 61511 WHERE serviceID = 32768; -- Gambling
UPDATE staServices
    SET serviceNameID = 61512 WHERE serviceID = 65536; -- Fitting
UPDATE staServices
    SET serviceNameID = 61513 WHERE serviceID = 131072; -- Paintshop
UPDATE staServices
    SET serviceNameID = 61514 WHERE serviceID = 262144; -- News
UPDATE staServices
    SET serviceNameID = 61515 WHERE serviceID = 524288; -- Storage
UPDATE staServices
    SET serviceNameID = 61516 WHERE serviceID = 1048576; -- Insurance
UPDATE staServices
    SET serviceNameID = 61517 WHERE serviceID = 2097152; -- Docking
UPDATE staServices
    SET serviceNameID = 61518 WHERE serviceID = 4194304; -- Office Rental
UPDATE staServices
    SET serviceNameID = 61519 WHERE serviceID = 8388608; -- Jump Clone Facility
UPDATE staServices
    SET serviceNameID = 61520 WHERE serviceID = 16777216; -- Loyalty Point Store
UPDATE staServices
    SET serviceNameID = 61521 WHERE serviceID = 33554432; -- Navy Offices

ALTER TABLE staStations
ADD COLUMN upgradeLevel TINYINT(4) NOT NULL DEFAULT 0 AFTER reprocessingHangarFlag;

CREATE TABLE staImprovements (
    typeID INT(10), 
    requiredUpgradeLevel TINYINT(4), 
    requiredImprovementTypeID INT(10), 
    requiredAssemblyLineTypeID INT(10), 
    raceID INT(10));

INSERT INTO staImprovements
VALUES
    (28076, 3, 28086, 43, 4), -- Amarr Advanced Outpost Factory
    (28077, 3, 28087, 43, 4), -- Amarr Advanced Outpost Plant
    (28078, 3, 28088, 43, 4), -- Amarr Advanced Outpost Laboratory
    (28079, 3, 28089, 43, 4), -- Amarr Advanced Outpost Office
    (28080, 3, 28090, 43, 4), -- Amarr Advanced Outpost Refinery
    (28081, 1, 0, 0, 4), -- Amarr Basic Outpost Factory
    (28082, 1, 0, 0, 4), -- Amarr Basic Outpost Plant
    (28083, 1, 0, 0, 4), -- Amarr Basic Outpost Laboratory
    (28084, 1, 0, 0, 4), -- Amarr Basic Outpost Office
    (28085, 1, 0, 0, 4), -- Amarr Basic Outpost Refinery
    (28086, 2, 28081, 43, 4), -- Amarr Outpost Factory
    (28087, 2, 28082, 43, 4), -- Amarr Outpost Plant
    (28088, 2, 28083, 43, 4), -- Amarr Outpost Laboratory
    (28089, 2, 28084, 43, 4), -- Amarr Outpost Office
    (28090, 2, 28085, 43, 4), -- Amarr Outpost Refinery
    (28091, 3, 28101, 70, 1), -- Caldari Advanced Outpost Factory
    (28092, 3, 28102, 70, 1), -- Caldari Advanced Outpost Laboratory
    (28093, 3, 28103, 70, 1), -- Caldari Advanced Outpost Research Facility
    (28094, 3, 28104, 70, 1), -- Caldari Advanced Outpost Office
    (28095, 3, 28105, 70, 1), -- Caldari Advanced Outpost Refinery
    (28096, 1, 0, 0, 1), -- Caldari Basic Outpost Factory
    (28097, 1, 0, 0, 1), -- Caldari Basic Outpost Laboratory
    (28098, 1, 0, 0, 1), -- Caldari Basic Outpost Research Facility
    (28099, 1, 0, 0, 1), -- Caldari Basic Outpost Office
    (28100, 1, 0, 0, 1), -- Caldari Basic Outpost Refinery
    (28101, 2, 28096, 70, 1), -- Caldari Outpost Factory
    (28102, 2, 28097, 70, 1), -- Caldari Outpost Laboratory
    (28103, 2, 28098, 70, 1), -- Caldari Outpost Research Facility
    (28104, 2, 28099, 70, 1), -- Caldari Outpost Office
    (28105, 2, 28100, 70, 1), -- Caldari Outpost Refinery
    (28106, 3, 28116, 93, 8), -- Gallente Advanced Outpost Factory
    (28107, 3, 28117, 93, 8), -- Gallente Advanced Outpost Plant
    (28108, 3, 28118, 93, 8), -- Gallente Advanced Outpost Laboratory
    (28109, 3, 28119, 93, 8), -- Gallente Advanced Outpost Office
    (28110, 3, 28120, 93, 8), -- Gallente Advanced Outpost Refinery
    (28111, 1, 0, 0, 8), -- Gallente Basic Outpost Factory
    (28112, 1, 0, 0, 8), -- Gallente Basic Outpost Plant
    (28113, 1, 0, 0, 8), -- Gallente Basic Outpost Laboratory
    (28114, 1, 0, 0, 8), -- Gallente Basic Outpost Office
    (28115, 1, 0, 0, 8), -- Gallente Basic Outpost Refinery
    (28116, 2, 28111, 93, 8), -- Gallente Outpost Factory
    (28117, 2, 28112, 93, 8), -- Gallente Outpost Plant
    (28118, 2, 28113, 93, 8), -- Gallente Outpost Laboratory
    (28119, 2, 28114, 93, 8), -- Gallente Outpost Office
    (28120, 2, 28115, 93, 8), -- Gallente Outpost Refinery
    (28121, 3, 28131, 46, 2), -- Minmatar Advanced Outpost Factory
    (28122, 3, 28132, 46, 2), -- Minmatar Advanced Outpost Plant
    (28123, 3, 28133, 46, 2), -- Minmatar Advanced Outpost Laboratory
    (28124, 3, 28134, 46, 2), -- Minmatar Advanced Outpost Office
    (28125, 3, 28135, 46, 2), -- Minmatar Advanced Outpost Refinery
    (28126, 1, 0, 0, 2), -- Minmatar Basic Outpost Factory
    (28127, 1, 0, 0, 2), -- Minmatar Basic Outpost Plant
    (28128, 1, 0, 0, 2), -- Minmatar Basic Outpost Laboratory
    (28129, 1, 0, 0, 2), -- Minmatar Basic Outpost Office
    (28130, 1, 0, 0, 2), -- Minmatar Basic Outpost Refinery
    (28131, 2, 28126, 46, 2), -- Minmatar Outpost Factory
    (28132, 2, 28127, 46, 2), -- Minmatar Outpost Plant
    (28133, 2, 28128, 46, 2), -- Minmatar Outpost Laboratory
    (28134, 2, 28129, 46, 2), -- Minmatar Outpost Office
    (28135, 2, 28130, 46, 2); -- Minmatar Outpost Refinery

CREATE TABLE staImprovementsInstalled (
    stationID INT(10),
    improvementTier1aTypeID INT(10),
    improvementTier2aTypeID INT(10),
    improvementTier3aTypeID INT(10),
    improvementTier1bTypeID INT(10),
    improvementTier2bTypeID INT(10),
    improvementTier1cTypeID INT(10));

-- +migrate Down
DROP TABLE staOutpostServiceConfig;

ALTER TABLE staServices
    DROP COLUMN serviceNameID;

ALTER TABLE staStations
    DROP COLUMN upgradeLevel;

DROP TABLE staImprovements;

DROP TABLE staImprovementsInstalled;