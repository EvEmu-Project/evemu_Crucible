-- A deliberately unsigned orderRange reproduces the MariaDB 1264 failure.
-- Test-only schema and data, never run on a production EVEmu database.
CREATE TABLE staStations (
  stationID BIGINT UNSIGNED NOT NULL PRIMARY KEY,
  solarSystemID INT UNSIGNED NOT NULL,
  regionID INT UNSIGNED NOT NULL,
  corporationID INT UNSIGNED NOT NULL,
  security FLOAT NOT NULL DEFAULT 1.0
) ENGINE=InnoDB;

CREATE TABLE invGroups (
  groupID INT UNSIGNED NOT NULL PRIMARY KEY,
  categoryID INT UNSIGNED NOT NULL
) ENGINE=InnoDB;

CREATE TABLE invTypes (
  typeID INT UNSIGNED NOT NULL PRIMARY KEY,
  typeName VARCHAR(100) NOT NULL,
  groupID INT UNSIGNED NOT NULL,
  published TINYINT NOT NULL,
  marketGroupID INT UNSIGNED NULL,
  basePrice DECIMAL(16,2) NULL
) ENGINE=InnoDB;

-- The regional sell seed also inspects these tables for rare meta items.
CREATE TABLE invMetaTypes (
  typeID INT UNSIGNED NOT NULL PRIMARY KEY,
  metaGroupID INT UNSIGNED NOT NULL
) ENGINE=InnoDB;
CREATE TABLE invMetaGroups (
  metaGroupID INT UNSIGNED NOT NULL PRIMARY KEY,
  metaGroupName VARCHAR(100) NOT NULL
) ENGINE=InnoDB;

CREATE TABLE mktOrders (
  orderID BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
  typeID INT UNSIGNED NOT NULL,
  ownerID INT UNSIGNED NOT NULL,
  regionID INT UNSIGNED NOT NULL,
  stationID BIGINT UNSIGNED NOT NULL,
  solarSystemID INT UNSIGNED NOT NULL,
  orderRange SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  bid TINYINT NOT NULL DEFAULT 0,
  price DECIMAL(16,2) NOT NULL DEFAULT 0,
  escrow DECIMAL(20,2) NOT NULL DEFAULT 0,
  minVolume INT UNSIGNED NOT NULL DEFAULT 1,
  volEntered INT UNSIGNED NOT NULL,
  volRemaining INT UNSIGNED NOT NULL,
  issued BIGINT UNSIGNED NOT NULL,
  contraband TINYINT NOT NULL DEFAULT 0,
  duration INT UNSIGNED NOT NULL DEFAULT 250,
  jumps INT UNSIGNED NOT NULL DEFAULT 1,
  isCorp TINYINT NOT NULL DEFAULT 0,
  accountKey INT UNSIGNED NOT NULL DEFAULT 1000,
  memberID INT UNSIGNED NOT NULL DEFAULT 0
) ENGINE=InnoDB;

INSERT INTO staStations
(stationID, solarSystemID, regionID, corporationID) VALUES
(60000001,30000001,10000001,1000125);

INSERT INTO invGroups (groupID,categoryID) VALUES
(18,4),(83,8),(7,7),(450,25),(465,25);

INSERT INTO invTypes
(typeID,typeName,groupID,published,marketGroupID,basePrice) VALUES
(34,'Test Tritanium',18,1,1,6.25),
(35,'Test Pyerite',18,1,1,12.50),
(100,'Test Charge',83,1,1,0.01),
(101,'Test No Base Price',7,1,1,0.00),
(102,'Not Market Listed',7,1,NULL,100.00),
(103,'Unpublished Type',7,0,1,50.00),
(200,'Test Arkonor',450,1,1,420.50),
(201,'Test Ice',465,1,1,650.25),
(202,'Ore Above Safety Ceiling',450,1,1,20000000.00);

-- Legacy bot bid: old NPC corporation owner, regional-range order.
INSERT INTO mktOrders
(typeID,ownerID,regionID,stationID,solarSystemID,orderRange,bid,
 price,escrow,volEntered,volRemaining,issued,duration)
VALUES
(34,1000125,10000001,60000001,30000001,32767,1,
 3.00,12.00,4,4,132478179209572976,5);

-- Player bid and sell order, plus an unrelated NPC sell, must survive.
INSERT INTO mktOrders
(typeID,ownerID,regionID,stationID,solarSystemID,orderRange,bid,
 price,escrow,volEntered,volRemaining,issued,duration)
VALUES
(34,90000000,10000001,60000001,30000001,0,1,
 5.00,50.00,10,10,132478179209572976,250),
(34,90000000,10000001,60000001,30000001,0,0,
 8.00,0.00,10,10,132478179209572976,250),
(34,1000125,10000001,60000001,30000001,32767,0,
 6.25,0.00,10,10,132478179209572976,250);
