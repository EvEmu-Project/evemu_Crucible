-- This file exists to record schema things as they were crafted by 
-- a human, instead of relying purely on a mysqldump of the schema,
-- however, this means that these definitions will get out of date
-- as things get updated, so this is meant only for informational
-- purposes at this time.

DROP TABLE account;
CREATE TABLE account
(
	accountID INT UNSIGNED NOT NULL auto_increment,
	accountName VARCHAR(127) NOT NULL,
	password VARCHAR(127) NOT NULL,
	role INT UNSIGNED NOT NULL
) ENGINE=InnoDB;

DROP TABLE character_;
CREATE TABLE character_
(
	characterID INT UNSIGNED NOT NULL,
	characterName VARCHAR(127) NOT NULL,
	accountID INT UNSIGNED NOT NULL,
	title VARCHAR(255) NOT NULL,
	description TEXT NOT NULL,
	
	bounty REAL NOT NULL,
	balance REAL NOT NULL,
	securityRating REAL NOT NULL,
	petitionMessage INT UNSIGNED NOT NULL,
	logonMinutes INT UNSIGNED NOT NULL,
	
	corporationID INT UNSIGNED NOT NULL,
   corporationDateTime BIGINT UNSIGNED NOT NULL,
	stationID INT UNSIGNED NOT NULL,
	solarSystemID INT UNSIGNED NOT NULL,
	constellationID INT UNSIGNED NOT NULL,
	regionID INT UNSIGNED NOT NULL,
	
	startDateTime BIGINT UNSIGNED NOT NULL,
   createDateTime BIGINT UNSIGNED NOT NULL,
   typeID INT UNSIGNED NOT NULL,
   raceID INT UNSIGNED NOT NULL,
	bloodlineID INT UNSIGNED NOT NULL,
	gender TINYINT NOT NULL,
	
	accessoryID INT UNSIGNED NULL,
	beardID INT UNSIGNED NULL,
	costumeID INT UNSIGNED NOT NULL,
	decoID INT UNSIGNED NULL,
	eyebrowsID INT UNSIGNED NOT NULL,
	eyesID INT UNSIGNED NOT NULL,
	hairID INT UNSIGNED NOT NULL,
	lipstickID INT UNSIGNED NULL,
	makeupID INT UNSIGNED NULL,
	skinID INT UNSIGNED NOT NULL,
	backgroundID INT UNSIGNED NOT NULL,
	lightID INT UNSIGNED NOT NULL,
	
	headRotation1 REAL NOT NULL,
	headRotation2 REAL NOT NULL,
	headRotation3 REAL NOT NULL,
	eyeRotation1 REAL NOT NULL,
	eyeRotation2 REAL NOT NULL,
	eyeRotation3 REAL NOT NULL,
	camPos1 REAL NOT NULL,
	camPos2 REAL NOT NULL,
	camPos3 REAL NOT NULL,
	morph1e REAL NULL,
	morph1n REAL NULL,
	morph1s REAL NULL,
	morph1w REAL NULL,
	morph2e REAL NULL,
	morph2n REAL NULL,
	morph2s REAL NULL,
	morph2w REAL NULL,
	morph3e REAL NULL,
	morph3n REAL NULL,
	morph3s REAL NULL,
	morph3w REAL NULL,
	morph4e REAL NULL,
	morph4n REAL NULL,
	morph4s REAL NULL,
	morph4w REAL NULL,

	UNIQUE KEY(characterName),
	PRIMARY KEY(characterID)
) ENGINE=InnoDB;

INSERT INTO character_ (
	characterID, characterName, accountID,
	title, description, bounty, balance,
	securityRating, petitionMessage,
	logonMinutes, corporationID, corporationDateTime,
	stationID, solarSystemID, constellationID,
	regionID, startDateTime, createDateTime,
   typeID, raceID, bloodlineID, gender, costumeID, 
   eyebrowsID, eyesID, hairID, skinID, backgroundID,
	lightID, headRotation1, headRotation2, headRotation3,
	eyeRotation1, eyeRotation2, eyeRotation3,
	camPos1, camPos2, camPos3
) VALUES (
1, 'EVE System',
0, '', '', 0, 0, 0, '', 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, -- type
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
);

INSERT INTO character_ (
	characterID, characterName, accountID,
	title, description, bounty, balance,
	securityRating, petitionMessage,
	logonMinutes, corporationID, corporationDateTime,
	stationID, solarSystemID, constellationID,
	regionID, startDateTime, createDateTime,
   typeID, raceID, bloodlineID, gender, costumeID, 
   eyebrowsID, eyesID, hairID, skinID, backgroundID,
	lightID, headRotation1, headRotation2, headRotation3,
	eyeRotation1, eyeRotation2, eyeRotation3,
	camPos1, camPos2, camPos3
) VALUES (
3008416, 'Hack Agent',
0, '', '', 0, 0, 0, '', 0, 1000044, 0, 0, 0, 0, 0, 0, 0,
1375, -- type
1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
);

CREATE TABLE corporation
(
	corporationID INT UNSIGNED NOT NULL,
   corporationName VARCHAR(100) NOT NULL,
   description MEDIUMTEXT NOT NULL,
	tickerName VARCHAR(8) NOT NULL,
   url MEDIUMTEXT NOT NULL,
   taxRate REAL NOT NULL,
   minimumJoinStanding REAL NOT NULL,
   corporationType INT UNSIGNED NOT NULL,
   hasPlayerPersonnelManager TINYINT UNSIGNED NOT NULL,
   sendCharTerminationMessage TINYINT UNSIGNED NOT NULL DEFAULT '1',
   creatorID INT UNSIGNED NOT NULL,
   ceoID INT UNSIGNED NOT NULL,
   stationID INT UNSIGNED NOT NULL,
   raceID INT UNSIGNED NULL,
   allianceID INT UNSIGNED NOT NULL DEFAULT '0',
   shares BIGINT( 20 ) UNSIGNED NOT NULL DEFAULT '1000',
   memberCount INT UNSIGNED NOT NULL,
   memberLimit INT UNSIGNED NOT NULL DEFAULT '10',
   allowedMemberRaceIDs INT UNSIGNED NOT NULL,
   graphicID INT UNSIGNED NOT NULL,
   shape1 INT UNSIGNED NULL,
   shape2 INT UNSIGNED NULL,
   shape3 INT UNSIGNED NULL,
   color1 INT UNSIGNED NULL,
   color2 INT UNSIGNED NULL,
   color3 INT UNSIGNED NULL,
   typeface VARCHAR(32) NULL,
   division1 VARCHAR(32) NULL,
   division2 VARCHAR(32) NULL,
   division3 VARCHAR(32) NULL,
   division4 VARCHAR(32) NULL,
   division5 VARCHAR(32) NULL,
   division6 VARCHAR(32) NULL,
   division7 VARCHAR(32) NULL,
   deleted TINYINT UNSIGNED NOT NULL DEFAULT '0',
   balance REAL NOT NULL DEFAULT '0',
	PRIMARY KEY(corporationID)
) ENGINE=InnoDB;
INSERT INTO corporation VALUES(
	1000044,
   "School Of Applied Knowledge",
   "The School of Applied Knowledge focuses mainly on technical studies and is considered the best school for aspiring engineers. The school collaborates with many leading Caldari corporations, which benefits the school, the companies and not the least the students.",
	"SAK",
   "http://sak.url",
   0.10,
   0.0,
   1,
   0,
   0,
   1,
   3004181,
   60004420,
   1,
   0,
   100000000,
   61255,
   100000,
   63,
   1622,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   'Advisory',
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   0,
   0);

CREATE TABLE crpOffices
(
   corporationID INT UNSIGNED NOT NULL,
   stationID INT UNSIGNED NOT NULL,
   itemID INT UNSIGNED NOT NULL,
   typeID INT UNSIGNED NOT NULL,
   officeFolderID INT UNSIGNED NOT NULL,
   PRIMARY KEY(corporationID,stationID),
   INDEX(itemID)
) ENGINE=InnoDB;
--INSERT INTO crpOffices (corporationID,stationID,itemID,officeFolderID) VALUES();

CREATE TABLE `chrCorporationRoles` (
  `characterID` int(10) unsigned NOT NULL,
  `corprole` bigint(20) unsigned NOT NULL,
  `rolesAtAll` bigint(20) unsigned NOT NULL,
  `rolesAtBase` bigint(20) unsigned NOT NULL,
  `rolesAtHQ` bigint(20) unsigned NOT NULL,
  `rolesAtOther` bigint(20) unsigned NOT NULL,
  PRIMARY KEY  (`characterID`)
) ENGINE=InnoDB;

DROP TABLE entity;
CREATE TABLE entity
(
	itemID INT UNSIGNED NOT NULL auto_increment,
	itemName VARCHAR(255) NOT NULL,     #duplicated from other tables.
	typeID INT UNSIGNED NOT NULL,
	ownerID INT UNSIGNED NOT NULL,
	locationID INT UNSIGNED NOT NULL,
	flag INT UNSIGNED NOT NULL,
	contraband INT UNSIGNED NOT NULL,
	singleton INT UNSIGNED NOT NULL,
	quantity INT UNSIGNED NOT NULL,
	x REAL NOT NULL DEFAULT 0,    #not sure what table to put these in
	y REAL NOT NULL DEFAULT 0,
	z REAL NOT NULL DEFAULT 0,
	customInfo TEXT NULL,
	PRIMARY KEY(itemID)
) AUTO_INCREMENT=140000000  ENGINE=InnoDB;
INSERT INTO entity VALUES(1, 'EVE System', 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, NULL);
INSERT INTO entity VALUES(30001407, 'Todaki', 5, 1, 20000206, 0, 0, 1, 1, 0, 0, 0, NULL);
--create an entity record from a station table entry:
INSERT INTO entity 
   (itemID,itemName,typeID,ownerID,locationID,flag,
    contraband,singleton,quantity,x,y,z,customInfo)
   SELECT stationID,stationName,stationTypeID,1,solarSystemID,0,
      0,1,1,x,y,z,NULL
   FROM staStations
   WHERE stationID IN (60004420);

CREATE TABLE entity_attributes
(
  itemID INT UNSIGNED NOT NULL,
  attributeID INT UNSIGNED NOT NULL,
  valueInt INT UNSIGNED NULL,
  valueFloat REAL UNSIGNED NULL,
  PRIMARY KEY(itemID, attributeID),
  CONSTRAINT FOREIGN KEY (itemID) 
      REFERENCES entity (itemID),
  CONSTRAINT FOREIGN KEY (attributeID) 
      REFERENCES dgmAttributeTypes (attributeID)
) ENGINE=InnoDB;



-- had TO have an ID field instead OF using (channelID,accessor) AS the pkey since accessor may be NULL
CREATE TABLE channelMods
(
   id INT UNSIGNED NOT NULL auto_increment,
   channelID INT UNSIGNED NOT NULL,
	accessor INT UNSIGNED NULL,
	mode INT UNSIGNED NOT NULL,
	untilWhen BIGINT UNSIGNED NULL,
	originalMode INT UNSIGNED NULL,
	admin VARCHAR(255) NULL,
	reason TEXT NULL,
   PRIMARY KEY(id)
) ENGINE=InnoDB;
INSERT INTO channelMods VALUES(0, 1, NULL, 3, NULL, NULL, NULL, NULL);
INSERT INTO channelMods VALUES(0, 2, NULL, 3, NULL, NULL, NULL, NULL);

-- this table is essentially an unknown right now
CREATE TABLE channelChars
(
   channelID INT UNSIGNED NOT NULL,
	corpID INT UNSIGNED NOT NULL,
	charID INT UNSIGNED NOT NULL,
	allianceID INT UNSIGNED NOT NULL,
	role INT UNSIGNED NOT NULL,
	extra INT UNSIGNED NOT NULL,
   PRIMARY KEY(channelID,charID)
) ENGINE=InnoDB;

CREATE TABLE chrStandings
(
   characterID INT UNSIGNED NOT NULL,
   toID INT UNSIGNED NOT NULL,
   standing REAL NOT NULL,
   PRIMARY KEY(characterID,toID)
) ENGINE=InnoDB;

CREATE TABLE chrNPCStandings
(
   characterID INT UNSIGNED NOT NULL,
   fromID INT UNSIGNED NOT NULL,
   standing REAL NOT NULL,
   PRIMARY KEY(characterID,fromID)
) ENGINE=InnoDB;

-- update imported tables to behave better
ALTER TABLE mapDenormalize ADD INDEX(solarSystemID);
ALTER TABLE mapDenormalize ADD INDEX(groupID);

CREATE TABLE droneState
(
   droneID INT UNSIGNED NOT NULL,
   solarSystemID INT UNSIGNED NOT NULL,
   ownerID INT UNSIGNED NOT NULL,
   controllerID INT UNSIGNED NOT NULL,
   activityState INT UNSIGNED NOT NULL,
   typeID INT UNSIGNED NOT NULL,
   controllerOwnerID INT UNSIGNED NOT NULL,
   PRIMARY KEY(droneID)
) ENGINE=InnoDB;

CREATE TABLE bookmarks
(
   bookmarkID INT UNSIGNED NOT NULL,
   ownerID INT UNSIGNED NOT NULL,
   itemID INT UNSIGNED NOT NULL,
   typeID INT UNSIGNED NOT NULL,
   flag INT UNSIGNED NOT NULL,
   memo VARCHAR(255) NOT NULL,
   created INT UNSIGNED NOT NULL,
   x REAL NOT NULL,
   y REAL NOT NULL,
   z REAL NOT NULL,
   locationID INT UNSIGNED NOT NULL,
   PRIMARY KEY(bookmarkID)
) ENGINE=InnoDB;

CREATE TABLE staStations
(
   stationID INT UNSIGNED NOT NULL,
   security INT UNSIGNED NOT NULL,
   dockingCostPerVolume REAL NOT NULL,
   maxShipVolumeDockable INT UNSIGNED NOT NULL,
   officeRentalCost INT UNSIGNED NOT NULL,
   operationID INT UNSIGNED,
   stationTypeID INT UNSIGNED NOT NULL,
   corporationID INT UNSIGNED NOT NULL,
   solarSystemID INT UNSIGNED NOT NULL,
   constellationID INT UNSIGNED NOT NULL,
   regionID INT UNSIGNED NOT NULL,
   stationName VARCHAR(255) NOT NULL,
   x REAL NOT NULL,
   y REAL NOT NULL,
   z REAL NOT NULL,
   reprocessingEfficiency REAL NOT NULL,
   reprocessingStationsTake REAL NOT NULL,
   reprocessingHangarFlag INT UNSIGNED NOT NULL,
   PRIMARY KEY(stationID),
   INDEX(corporationID),
   INDEX(solarSystemID),
   INDEX(constellationID),
   INDEX(regionID)
) ENGINE=InnoDB;

CREATE TABLE staOperationServices (
   operationID INT UNSIGNED NOT NULL,
   serviceID INT UNSIGNED NOT NULL,
   PRIMARY KEY(operationID,serviceID)
) ENGINE=InnoDB;

CREATE TABLE staServices (
   serviceID INT UNSIGNED NOT NULL,
   serviceName VARCHAR(100) NOT NULL,
   description MEDIUMTEXT NOT NULL,
   PRIMARY KEY(serviceID)
) ENGINE=InnoDB;

CREATE TABLE staOperations
(
   activityID INT UNSIGNED NOT NULL,
   operationID INT UNSIGNED NOT NULL,
   operationName VARCHAR(100) NOT NULL,
   description MEDIUMTEXT NOT NULL,
   fringe INT UNSIGNED NOT NULL,
   corridor INT UNSIGNED NOT NULL,
   hub INT UNSIGNED NOT NULL,
   border INT UNSIGNED NOT NULL,
   ratio INT UNSIGNED NOT NULL,
   caldariStationTypeID INT UNSIGNED NULL,
   minmatarStationTypeID INT UNSIGNED NULL,
   amarrStationTypeID INT UNSIGNED NULL,
   gallenteStationTypeID INT UNSIGNED NULL,
   joveStationTypeID INT UNSIGNED NULL,
   PRIMARY KEY(operationID)
) ENGINE=InnoDB;

CREATE TABLE staStationTypes
(
   stationTypeID INT UNSIGNED NOT NULL,
   dockingBayGraphicID INT UNSIGNED NULL,
   hangarGraphicID INT UNSIGNED NULL,
   dockEntryX REAL NOT NULL,
   dockEntryY REAL NOT NULL,
   dockEntryZ REAL NOT NULL,
   dockOrientationX REAL NOT NULL,
   dockOrientationY REAL NOT NULL,
   dockOrientationZ REAL NOT NULL,
   operationID INT UNSIGNED NULL,
   officeSlots INT UNSIGNED NULL,
   reprocessingEfficiency INT UNSIGNED NULL,
   conquerable TINYINT UNSIGNED NOT NULL,
   PRIMARY KEY(stationTypeID)
) ENGINE=InnoDB;

CREATE TABLE evemail (
  channelID INT UNSIGNED NOT NULL,
  messageID INT UNSIGNED NOT NULL,
  senderID INT UNSIGNED NOT NULL,
  subject VARCHAR(64) NOT NULL,
  created INT UNSIGNED NOT NULL,
  `read` TINYINT UNSIGNED NOT NULL
) ENGINE=InnoDB;

CREATE TABLE evemailDetails (
  attachmentID INT UNSIGNED NOT NULL auto_increment,
  messageID INT UNSIGNED NOT NULL,
  mimeType TEXT NOT NULL,
  attachment TEXT NOT NULL,
  PRIMARY KEY(attachmentID)
) ENGINE=InnoDB;


CREATE TABLE billsReceivable (
  billID INT UNSIGNED NULL,
  billTypeID INT UNSIGNED NULL,
  debtorID INT UNSIGNED NULL,
  creditorID INT UNSIGNED NULL,
  amount TEXT NOT NULL,
  dueDateTime TEXT NOT NULL,
  interest TEXT NOT NULL,
  externalID INT UNSIGNED NULL,
  paid TEXT NOT NULL,
  externalID2 TEXT NOT NULL,
  PRIMARY KEY(billID)
) ENGINE=InnoDB;

CREATE TABLE crpCharShares (
  characterID INT UNSIGNED NOT NULL,
  corporationID INT UNSIGNED NOT NULL,
  shares INT UNSIGNED NOT NULL,
  PRIMARY KEY(characterID,corporationID)
) ENGINE=InnoDB;

CREATE TABLE rentalInfo (
  stationID INT UNSIGNED NOT NULL,
  slotNumber INT UNSIGNED NOT NULL,
  renterID INT UNSIGNED NOT NULL,
  typeID INT UNSIGNED NOT NULL,
  rentPeriodInDays INT UNSIGNED NOT NULL,
  periodCost REAL NOT NULL,
  billID INT UNSIGNED NOT NULL,
  balanceDueDate INT UNSIGNED NOT NULL,
  discontinue TINYINT UNSIGNED NOT NULL,
  publiclyAvailable TINYINT UNSIGNED NOT NULL,
  PRIMARY KEY(stationID,slotNumber)
) ENGINE=InnoDB;

CREATE TABLE market_orders (
  orderID INT UNSIGNED NOT NULL auto_increment,
  typeID INT UNSIGNED NOT NULL,
  charID INT UNSIGNED NOT NULL,
  regionID INT UNSIGNED NOT NULL,
  stationID INT UNSIGNED NOT NULL,
  range INT UNSIGNED NOT NULL,
  bid REAL NOT NULL,
  price REAL NOT NULL,
  volEntered INT UNSIGNED NOT NULL,
  volRemaining INT UNSIGNED NOT NULL,
  issued BIGINT UNSIGNED NOT NULL,
  orderState INT UNSIGNED NOT NULL,
  minVolume INT UNSIGNED NOT NULL,
  contraband TINYINT UNSIGNED NOT NULL,
  accountID INT UNSIGNED NOT NULL,
  duration INT UNSIGNED NOT NULL,
  isCorp TINYINT UNSIGNED NOT NULL,
  solarSystemID INT UNSIGNED NULL,
  escrow TINYINT UNSIGNED NOT NULL,
  PRIMARY KEY(orderID)
) ENGINE=InnoDB;

CREATE TABLE market_transactions (
  transactionID INT UNSIGNED NOT NULL auto_increment,
  transactionDateTime BIGINT UNSIGNED NOT NULL,
  typeID INT UNSIGNED NOT NULL,
  quantity INT UNSIGNED NOT NULL,
  price REAL NOT NULL,
  transactionType INT UNSIGNED NOT NULL,
  clientID INT UNSIGNED NOT NULL,
  regionID INT UNSIGNED NOT NULL,
  stationID INT UNSIGNED NOT NULL,
  PRIMARY KEY(transactionID),
  INDEX(regionID)
) ENGINE=InnoDB;

CREATE TABLE market_history_old (
  regionID INT UNSIGNED NOT NULL,
  typeID INT UNSIGNED NOT NULL,
  historyDate INT UNSIGNED NOT NULL,
  volume INT UNSIGNED NOT NULL,
  orders INT UNSIGNED NOT NULL,
  lowPrice REAL NOT NULL,
  highPrice REAL NOT NULL,
  avgPrice REAL NOT NULL,
  PRIMARY KEY(regionID,typeID,historyDate),
  INDEX(regionID,typeID)
) ENGINE=InnoDB;

CREATE TABLE market_journal (
  characterID INT UNSIGNED NOT NULL,
  refID INT UNSIGNED NOT NULL,
  transDate INT UNSIGNED NOT NULL,
  refTypeID INT UNSIGNED NOT NULL,
  ownerID1 INT UNSIGNED NOT NULL,
  ownerID2 INT UNSIGNED NOT NULL,
  argID1 VARCHAR(64) NULL,
  accountID INT UNSIGNED NOT NULL,
  accountKey INT UNSIGNED NOT NULL,
  amount REAL NOT NULL,
  balance REAL NOT NULL,
  reason VARCHAR(128) NULL,
  PRIMARY KEY(refID)
) ENGINE=InnoDB;

CREATE TABLE `billsPayable` (
  `billID` int(10) unsigned NOT NULL default '0',
  `billTypeID` int(10) unsigned default NULL,
  `debtorID` int(10) unsigned default NULL,
  `creditorID` int(10) unsigned default NULL,
  `amount` text NOT NULL,
  `dueDateTime` text NOT NULL,
  `interest` text NOT NULL,
  `externalID` int(10) unsigned default NULL,
  `paid` text NOT NULL,
  `externalID2` text NOT NULL,
  PRIMARY KEY  (`billID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE courierMissions (
  missionID INT UNSIGNED NULL auto_increment,
  kind TINYINT UNSIGNED NOT NULL,
  state TINYINT UNSIGNED NOT NULL,
  availabilityID INT UNSIGNED NULL,
  inOrder TINYINT UNSIGNED NOT NULL,
  description TEXT NOT NULL,
  issuerID INT UNSIGNED NULL,
  assigneeID INT UNSIGNED NULL,
  acceptFee REAL NOT NULL,
  acceptorID INT UNSIGNED NULL,
  dateIssued INT UNSIGNED NOT NULL,
  dateExpires INT UNSIGNED NOT NULL,
  dateAccepted INT UNSIGNED NOT NULL,
  dateCompleted INT UNSIGNED NOT NULL,
  totalReward REAL NOT NULL,
  tracking TINYINT UNSIGNED NOT NULL,
  pickupStationID INT UNSIGNED NULL,
  craterID INT UNSIGNED NULL,
  dropStationID INT UNSIGNED NULL,
  volume REAL NOT NULL,
  pickupSolarSystemID INT UNSIGNED NULL,
  pickupRegionID INT UNSIGNED NULL,
  dropSolarSystemID INT UNSIGNED NULL,
  dropRegionID INT UNSIGNED NULL,
  PRIMARY KEY(missionID)
) ENGINE=InnoDB;

CREATE TABLE alliance_shortnames (
  allianceID INT UNSIGNED NULL PRIMARY KEY,
  shortName TEXT NOT NULL
) ENGINE=InnoDB;

CREATE TABLE eveEncyclTypes (
  encyclopediaTypeID INT UNSIGNED NULL PRIMARY KEY,
  encyclopediaTypeName TEXT NOT NULL
) ENGINE=InnoDB;




CREATE TABLE spawns (
  spawnID INT UNSIGNED PRIMARY KEY auto_increment,
  solarSystemID INT UNSIGNED NOT NULL,
  spawnGroupID INT UNSIGNED NOT NULL,
  spawnBoundType INT UNSIGNED NOT NULL DEFAULT '0',
  spawnTimer INT UNSIGNED NOT NULL DEFAULT '0',
  respawnTimeMin INT UNSIGNED NOT NULL,
  respawnTimeMax INT UNSIGNED NOT NULL
) ENGINE=InnoDB;

CREATE TABLE spawnBounds (
  spawnID INT UNSIGNED NOT NULL,
  pointIndex TINYINT UNSIGNED NOT NULL,
  x DOUBLE NOT NULL,
  y DOUBLE NOT NULL,
  z DOUBLE NOT NULL,
  PRIMARY KEY(spawnID, pointIndex)
) ENGINE=InnoDB;

CREATE TABLE spawnGroups (
  spawnGroupID INT UNSIGNED PRIMARY KEY auto_increment,
  spawnGroupName VARCHAR(255) NOT NULL,
  formation INT UNSIGNED NOT NULL
) ENGINE=InnoDB;

CREATE TABLE spawnGroupEntries (
  spawnGroupID INT UNSIGNED NOT NULL,
  npcTypeID  INT UNSIGNED NOT NULL,
  quantity TINYINT UNSIGNED NOT NULL DEFAULT '1',
  probability FLOAT NOT NULL DEFAULT '1',
  ownerID INT UNSIGNED NOT NULL,
  corporationID INT UNSIGNED NOT NULL,
  PRIMARY KEY(spawnGroupID, npcTypeID)
) ENGINE=InnoDB;





--things like kill, courier, etc.
CREATE TABLE agtMissionTypes (
  missionTypeID INT UNSIGNED auto_increment,
  missionTypeName VARCHAR(255) NOT NULL,
  PRIMARY KEY(missionTypeID)
) ENGINE=InnoDB;

CREATE TABLE agtMissions (
  missionID INT UNSIGNED auto_increment,
  missionName VARCHAR(255) NOT NULL,
  missionLevel TINYINT UNSIGNED NOT NULL,
  missionTypeID INT UNSIGNED NOT NULL,
  importantMission TINYINT UNSIGNED NOT NULL,
  PRIMARY KEY(missionID),
  INDEX(missionLevel),
  CONSTRAINT FOREIGN KEY (missionTypeID) 
      REFERENCES agtMissionTypes (missionTypeID)
) ENGINE=InnoDB;

--exact key is to be determined
CREATE TABLE agtMissionGroups (
  corporationID INT UNSIGNED,
  divisionID INT UNSIGNED,
  level INT UNSIGNED,
  missionID INT UNSIGNED,
  PRIMARY KEY(corporationID, divisionID, level),
  CONSTRAINT FOREIGN KEY (corporationID)
      REFERENCES agtMissionTypes (missionTypeID),
  CONSTRAINT FOREIGN KEY (divisionID)
      REFERENCES crpNPCDivisions (divisionID),
  CONSTRAINT FOREIGN KEY (missionID)
      REFERENCES agtMissions (missionID)
) ENGINE=InnoDB;

CREATE TABLE chrMissionState (
  characterID INT UNSIGNED NOT NULL,
  missionID INT UNSIGNED NOT NULL,
  missionState TINYINT UNSIGNED NOT NULL,
  expirationTime BIGINT UNSIGNED NOT NULL,
  PRIMARY KEY(characterID,missionID),
  CONSTRAINT FOREIGN KEY (characterID)
      REFERENCES character_ (characterID),
  CONSTRAINT FOREIGN KEY (missionID)
      REFERENCES agtMissions (missionID)
) ENGINE=InnoDB;


CREATE TABLE agtOffers (
  offerID INT UNSIGNED PRIMARY KEY auto_increment,
  offerName VARCHAR(255) NOT NULL,
  offerLevel TINYINT UNSIGNED NOT NULL,
  loyaltyPoints INT UNSIGNED NOT NULL,
  requiredISK INT UNSIGNED NOT NULL,
  rewardISK INT UNSIGNED NOT NULL,
  offerText MEDIUMTEXT NOT NULL,
  offerAcceptedText MEDIUMTEXT NOT NULL,
  INDEX(offerLevel)
) ENGINE=InnoDB;

CREATE TABLE agtOfferRequired (
  offerID INT UNSIGNED NOT NULL,
  typeID INT UNSIGNED NOT NULL,
  quantity INT UNSIGNED NOT NULL,
  PRIMARY KEY(offerID,typeID),
  CONSTRAINT FOREIGN KEY (offerID)
      REFERENCES agtOffers (offerID),
  CONSTRAINT FOREIGN KEY (typeID)
      REFERENCES invTypes (typeID)
) ENGINE=InnoDB;

CREATE TABLE agtOfferReward (
  offerID INT UNSIGNED NOT NULL,
  typeID INT UNSIGNED NOT NULL,
  quantity INT UNSIGNED NOT NULL,
  PRIMARY KEY(offerID,typeID),
  CONSTRAINT FOREIGN KEY (offerID)
      REFERENCES agtOffers (offerID),
  CONSTRAINT FOREIGN KEY (typeID)
      REFERENCES invTypes (typeID)
) ENGINE=InnoDB;

--exact key is to be determined
CREATE TABLE agtOfferGroups (
  corporationID INT UNSIGNED,
  divisionID INT UNSIGNED,
  level INT UNSIGNED,
  offerID INT UNSIGNED,
  PRIMARY KEY(corporationID, divisionID, level),
  CONSTRAINT FOREIGN KEY (corporationID)
      REFERENCES agtMissionTypes (missionTypeID),
  CONSTRAINT FOREIGN KEY (divisionID)
      REFERENCES crpNPCDivisions (divisionID),
  CONSTRAINT FOREIGN KEY (offerID)
      REFERENCES agtOffers (offerID)
) ENGINE=InnoDB;

CREATE TABLE chrOffers (
  characterID INT UNSIGNED NOT NULL,
  offerID INT UNSIGNED NOT NULL,
  expirationTime BIGINT UNSIGNED NOT NULL,
  PRIMARY KEY(characterID,offerID),
  CONSTRAINT FOREIGN KEY (characterID)
      REFERENCES character_ (characterID),
  CONSTRAINT FOREIGN KEY (offerID)
      REFERENCES agtOffers (offerID)
) ENGINE=InnoDB;




create table invOreInfo (
 typeID INT UNSIGNED NOT NULL,
 yieldBonus INT UNSIGNED NOT NULL,
 minToRefine INT UNSIGNED NOT NULL,
 PRIMARY KEY(typeID),
 CONSTRAINT FOREIGN KEY (typeID)
      REFERENCES invTypes (typeID)
) TYPE=InnoDB;

create table invOreYield (
 typeID INT UNSIGNED NOT NULL,
 yieldTypeID INT UNSIGNED NOT NULL,
 yieldPercent INT UNSIGNED NOT NULL,
 PRIMARY KEY(typeID, yieldTypeID),
 CONSTRAINT FOREIGN KEY (typeID)
      REFERENCES invOreInfo (typeID),
 CONSTRAINT FOREIGN KEY (yieldTypeID)
      REFERENCES invTypes (typeID)
) TYPE=InnoDB;

CREATE TABLE chrNotes (
  ownerID INT UNSIGNED NOT NULL,
  itemID INT UNSIGNED NOT NULL,
  note TEXT NOT NULL,
  PRIMARY KEY(ownerID,itemID),
  CONSTRAINT FOREIGN KEY (ownerID)
      REFERENCES character_ (characterID),
  CONSTRAINT FOREIGN KEY (itemID)
      REFERENCES entity (itemID)
) TYPE=InnoDB;

CREATE TABLE `chrEmployment` (
  `characterID` int(10) unsigned NOT NULL,
  `corporationID` int(10) unsigned NOT NULL,
  `startDate` bigint(20) unsigned NOT NULL,
  `deleted` tinyint(4) NOT NULL default '0',
  PRIMARY KEY  (`characterID`,`corporationID`,`startDate`),
  CONSTRAINT FOREIGN KEY (characterID)
      REFERENCES character_ (characterID),
  CONSTRAINT FOREIGN KEY (corporationID)
      REFERENCES corporation (corporationID)
) ENGINE=InnoDB;

CREATE TABLE `chrStandingChanges` (
  `eventID` int(10) unsigned NOT NULL auto_increment,
  `eventTypeID` int(10) unsigned NOT NULL,
  `eventDateTime` bigint(20) unsigned NOT NULL,
  `fromID` int(10) unsigned NOT NULL,
  `toID` int(10) unsigned NOT NULL,
  `modification` double NOT NULL,
  `originalFromID` int(10) unsigned NOT NULL,
  `originalToID` int(10) unsigned NOT NULL,
  `int_1` int(10) unsigned NOT NULL,
  `int_2` int(10) unsigned NOT NULL,
  `int_3` int(10) unsigned NOT NULL,
  `msg` text NOT NULL,
  PRIMARY KEY  (`eventID`)
) ENGINE=InnoDB;

CREATE TABLE `chrOwnerNote` (
`noteID` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
`ownerID` INT UNSIGNED NOT NULL ,
`label` TEXT CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
`note` TEXT CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL ,
PRIMARY KEY ( `noteID` ),
INDEX ( `ownerID` )
) ENGINE = innodb;

CREATE TABLE `tutorial_criteria` (
  `criteriaID` int(10) unsigned NOT NULL default '0',
  `criteriaName` varchar(100) NOT NULL default '',
  `messageText` mediumtext NOT NULL,
  `audioPath` varchar(200) default NULL,
  PRIMARY KEY(criteriaID)
) ENGINE=InnoDB;

CREATE TABLE `tutorial_page_criteria` (
  `pageID` int(10) unsigned NOT NULL default '0',
  `criteriaID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`pageID`,`criteriaID`),
  CONSTRAINT FOREIGN KEY (criteriaID)
      REFERENCES tutorial_criteria (criteriaID)
) ENGINE=InnoDB;

CREATE TABLE `tutorials` (
  `tutorialID` int(10) unsigned NOT NULL default '0',
  `tutorialVName` varchar(100) NOT NULL default '',
  `nextTutorialID` int(10) unsigned default NULL,
  PRIMARY KEY(tutorialID)
) ENGINE=InnoDB;

CREATE TABLE tutorials_criterias (
  `tutorialID` int(10) unsigned NOT NULL,
  `criteriaID` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`tutorialID`,`criteriaID`),
  CONSTRAINT FOREIGN KEY (tutorialID)
      REFERENCES tutorials (tutorialID),
  CONSTRAINT FOREIGN KEY (criteriaID)
      REFERENCES tutorial_criteria (criteriaID)
) ENGINE=InnoDB;
                                  

CREATE TABLE `tutorial_pages` (
  `tutorialID` int(10) unsigned NOT NULL
  `pageID` int(10) unsigned NOT NULL default '0',
  `pageNumber` int(10) unsigned NOT NULL default '0',
  `pageName` varchar(100) NOT NULL default '',
  `text` mediumtext NOT NULL,
  `imagePath` varchar(200) default NULL,
  `audioPath` varchar(200) NOT NULL default '',
  PRIMARY KEY  (`pageID`),
  UNIQUE KEY(tutorialID, pageNumber),
  CONSTRAINT FOREIGN KEY (tutorialID)
      REFERENCES tutorials (tutorialID)
) ENGINE=InnoDB;





-------------------------------------------------------
--Queries which need to run after the data is imported:
-------------------------------------------------------
--set up primary keys and indexes
ALTER TABLE crpNPCCorporations ADD PRIMARY KEY(corporationID);
ALTER TABLE mapSolarSystems ADD PRIMARY KEY(solarSystemID);
ALTER TABLE mapSolarSystems ADD INDEX(regionID);
ALTER TABLE mapSolarSystems ADD INDEX(constellationID);
ALTER TABLE chrSchools ADD PRIMARY KEY(schoolID);
ALTER TABLE TL2MaterialsForTypeWithActivity ADD PRIMARY KEY(typeID,activity,requiredTypeID);
ALTER TABLE dgmAttributeTypes ADD PRIMARY KEY(attributeID);
ALTER TABLE dgmTypeAttributes ADD PRIMARY KEY(typeID,attributeID);
ALTER TABLE agtAgents2 ADD INDEX(stationID);
ALTER TABLE eveNames ADD PRIMARY KEY(itemID);
ALTER TABLE eveNames ADD INDEX(typeID);
ALTER TABLE chrBloodlines ADD PRIMARY KEY(bloodlineID);
--mark columns not null in cases where we assume this to be true.
ALTER TABLE chrBloodlines CHANGE shipTypeID shipTypeID INTEGER NOT NULL;
ALTER TABLE chrBloodlines CHANGE raceID raceID INTEGER NOT NULL;

--fix up the corporation table a bit.
-- copy corporationName into the table since joining eveNames is very slow.
ALTER TABLE crpNPCCorporations ADD corporationName VARCHAR(100) NOT NULL;
ALTER TABLE crpNPCCorporations ADD stationID INT UNSIGNED NOT NULL;
ALTER TABLE crpNPCCorporations ADD ceoID INT UNSIGNED NOT NULL DEFAULT 1;
UPDATE crpNPCCorporations,eveNames
  SET corporationName=itemName
  WHERE corporationID=itemID;
-- im not 100% confident in this query, but it will do.
UPDATE crpNPCCorporations,staStations
  SET crpNPCCorporations.stationID=staStations.stationID
  WHERE crpNPCCorporations.corporationID=staStations.corporationID
   AND crpNPCCorporations.solarSystemID=staStations.solarSystemID;
--todo: extract ceoID from Corporation::GetFactionInfo

--fix up the TL2MaterialsForTypeWithActivity table for ramtyperequirements
ALTER TABLE TL2MaterialsForTypeWithActivity CHANGE typeID typeID INT UNSIGNED NOT NULL;
ALTER TABLE TL2MaterialsForTypeWithActivity CHANGE activity activity TINYINT UNSIGNED NOT NULL;
ALTER TABLE TL2MaterialsForTypeWithActivity CHANGE quantity quantity INT UNSIGNED NOT NULL;
ALTER TABLE TL2MaterialsForTypeWithActivity CHANGE damagePerJob damagePerJob FLOAT NOT NULL;
ALTER TABLE TL2MaterialsForTypeWithActivity ADD recycle TINYINT UNSIGNED NOT NULL DEFAULT 0;
INSERT INTO TL2MaterialsForTypeWithActivity (typeID,activity,requiredTypeID,quantity,damagePerJob,recycle)
 VALUES (34, 6, 242, 3, 1.0, 1),
        (35, 6, 242, 1, 1.0, 1),
        (36, 6, 242, 2, 1.0, 1),
        (37, 6, 242, 9, 1.0, 1),
        (11399, 6, 438, 1, 1.0, 1);

-- corp (temp hack):
INSERT INTO channels VALUES(1000115,1000115, NULL, NULL, NULL, 1, NULL, 1, 2000000000, 0, 1, 1, 1252);
INSERT INTO channels VALUES(1000044,1000044, NULL, NULL, NULL, 1, NULL, 1, 2000000000, 0, 1, 1, 1252);
--solar (temp hack):
INSERT INTO channels VALUES(30001407,30001407, NULL, NULL, NULL, 1, NULL, 1, 2000000000, 0, 1, 1, 1252);

