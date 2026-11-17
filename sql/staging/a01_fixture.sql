-- Synthetic data only. Intended for a unique A01 Docker Compose MariaDB.
-- All tables have an a01_ prefix; NO original EVEmu tables are modified.
-- A01 fixture intentionally exercises the snapshot/diff harness, not actual
-- server RPC behavior or historical Crucible game mechanics.
DROP TABLE IF EXISTS a01_ctrContracts;
DROP TABLE IF EXISTS a01_mktOrders;
DROP TABLE IF EXISTS a01_agtOffers;
DROP TABLE IF EXISTS a01_entity;
DROP TABLE IF EXISTS a01_crpWalletDivisons;
DROP TABLE IF EXISTS a01_chrCharacters;

CREATE TABLE a01_chrCharacters (
  characterID INT PRIMARY KEY, corporationID INT NOT NULL,
  balance DECIMAL(20,2) NOT NULL
) ENGINE=InnoDB;
CREATE TABLE a01_crpWalletDivisons (
  corporationID INT PRIMARY KEY,
  balance1 DECIMAL(20,2) NOT NULL, balance2 DECIMAL(20,2) NOT NULL,
  balance3 DECIMAL(20,2) NOT NULL, balance4 DECIMAL(20,2) NOT NULL,
  balance5 DECIMAL(20,2) NOT NULL, balance6 DECIMAL(20,2) NOT NULL,
  balance7 DECIMAL(20,2) NOT NULL
) ENGINE=InnoDB;
CREATE TABLE a01_entity (
  itemID INT PRIMARY KEY, ownerID INT NOT NULL, locationID INT NOT NULL,
  typeID INT NOT NULL, flag INT NOT NULL, quantity INT NOT NULL
) ENGINE=InnoDB;
CREATE TABLE a01_agtOffers (
  offerID INT PRIMARY KEY, characterID INT NOT NULL, agentID INT NOT NULL,
  missionID INT NOT NULL, stateID INT NOT NULL, rewardISK BIGINT NOT NULL,
  dateCompleted BIGINT NOT NULL DEFAULT 0
) ENGINE=InnoDB;
CREATE TABLE a01_mktOrders (
  orderID INT PRIMARY KEY, ownerID INT NOT NULL, typeID INT NOT NULL,
  price DECIMAL(20,2) NOT NULL, volRemaining INT NOT NULL
) ENGINE=InnoDB;
CREATE TABLE a01_ctrContracts (
  contractId INT PRIMARY KEY, issuerID INT NOT NULL, acceptorID INT NOT NULL,
  status INT NOT NULL, price INT NOT NULL, reward INT NOT NULL,
  collateral INT NOT NULL, crateID INT NOT NULL
) ENGINE=InnoDB;

-- Two characters, separate corporations, separate inventories.
INSERT INTO a01_chrCharacters VALUES
 (90010001,98010001,100000.00), (90010002,98010002,50000.00);
INSERT INTO a01_crpWalletDivisons VALUES
 (98010001,200000.00,0,0,0,0,0,0), (98010002,300000.00,0,0,0,0,0,0);
INSERT INTO a01_entity VALUES
 (140010001,90010001,90010001,34,5,50),
 (140010002,90010001,90010001,35,5,20),
 (140010003,90010002,90010002,34,5,15);
INSERT INTO a01_agtOffers VALUES
 (41010001,90010001,3011918,65535,1,50000000,0),
 (41010002,90010002,3011918,65535,1,50000000,0);
INSERT INTO a01_mktOrders VALUES
 (51010001,90010001,34,10.00,50),
 (51010002,90010002,35,25.00,80);
INSERT INTO a01_ctrContracts VALUES
 (61010001,90010001,0,0,0,5000,0,0);
