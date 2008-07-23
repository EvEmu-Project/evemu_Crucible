-- MySQL dump 10.9
--
-- Host: 127.0.0.1    Database: eve
-- ------------------------------------------------------
-- Server version	4.1.22-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `TL2MaterialsActivity`
--

DROP TABLE IF EXISTS `TL2MaterialsActivity`;
CREATE TABLE `TL2MaterialsActivity` (
  `activityID` int(11) NOT NULL default '0',
  `activityName` char(40) NOT NULL default '',
  `published` tinyint(4) NOT NULL default '0',
  PRIMARY KEY  (`activityID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `TL2MaterialsForTypeWithActivity`
--

DROP TABLE IF EXISTS `TL2MaterialsForTypeWithActivity`;
CREATE TABLE `TL2MaterialsForTypeWithActivity` (
  `typeID` int(10) unsigned NOT NULL default '0',
  `activity` tinyint(3) unsigned NOT NULL default '0',
  `requiredTypeID` int(11) NOT NULL default '0',
  `quantity` int(10) unsigned NOT NULL default '0',
  `damagePerJob` float NOT NULL default '0',
  `recycle` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`typeID`,`activity`,`requiredTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `account`
--

DROP TABLE IF EXISTS `account`;
CREATE TABLE `account` (
  `accountID` int(10) unsigned NOT NULL auto_increment,
  `accountName` varchar(43) NOT NULL default '',
  `password` varchar(43) NOT NULL default '',
  `role` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`accountID`),
  UNIQUE KEY `accountName` (`accountName`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `agtAgentTypes`
--

DROP TABLE IF EXISTS `agtAgentTypes`;
CREATE TABLE `agtAgentTypes` (
  `agentTypeID` int(10) unsigned NOT NULL default '0',
  `agentType` varchar(17) NOT NULL default '',
  PRIMARY KEY  (`agentTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `agtAgents`
--

DROP TABLE IF EXISTS `agtAgents`;
CREATE TABLE `agtAgents` (
  `agentID` int(10) unsigned NOT NULL default '0',
  `divisionID` int(10) unsigned NOT NULL default '0',
  `corporationID` int(10) unsigned NOT NULL default '0',
  `stationID` int(10) unsigned default NULL,
  `level` tinyint(3) unsigned NOT NULL default '0',
  `quality` int(11) default NULL,
  `agentTypeID` int(10) unsigned NOT NULL default '0',
  `gender` tinyint(3) unsigned NOT NULL default '0',
  `bloodlineID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`agentID`),
  KEY `FK_AGENTS_AGENTTYPES` (`agentTypeID`),
  KEY `stationID` (`stationID`),
  KEY `divisionID` (`divisionID`),
  KEY `FK_AGENTS_CORPORATION` (`corporationID`),
  CONSTRAINT `agtAgents_ibfk_1` FOREIGN KEY (`stationID`) REFERENCES `staStations` (`stationID`),
  CONSTRAINT `agtAgents_ibfk_3` FOREIGN KEY (`divisionID`) REFERENCES `crpNPCDivisions` (`divisionID`),
  CONSTRAINT `FK_AGENTS_AGENTTYPES` FOREIGN KEY (`agentTypeID`) REFERENCES `agtAgentTypes` (`agentTypeID`),
  CONSTRAINT `FK_AGENTS_CORPORATION` FOREIGN KEY (`corporationID`) REFERENCES `corporation` (`corporationID`),
  CONSTRAINT `FK_AGENTS_NPCDIVISIONS` FOREIGN KEY (`divisionID`) REFERENCES `crpNPCDivisions` (`divisionID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `agtConfig`
--

DROP TABLE IF EXISTS `agtConfig`;
CREATE TABLE `agtConfig` (
  `agentID` int(10) unsigned NOT NULL default '0',
  `k` varchar(17) NOT NULL default '',
  `v` varchar(17) NOT NULL default '',
  PRIMARY KEY  (`agentID`,`k`),
  CONSTRAINT `FK_AGTCONFIG_AGTAGENTS` FOREIGN KEY (`agentID`) REFERENCES `agtAgents` (`agentID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `agtMissionGroups`
--

DROP TABLE IF EXISTS `agtMissionGroups`;
CREATE TABLE `agtMissionGroups` (
  `corporationID` int(10) unsigned NOT NULL default '0',
  `divisionID` int(10) unsigned NOT NULL default '0',
  `level` int(10) unsigned NOT NULL default '0',
  `missionID` int(10) unsigned default NULL,
  PRIMARY KEY  (`corporationID`,`divisionID`,`level`),
  KEY `divisionID` (`divisionID`),
  KEY `missionID` (`missionID`),
  CONSTRAINT `agtMissionGroups_ibfk_1` FOREIGN KEY (`corporationID`) REFERENCES `agtMissionTypes` (`missionTypeID`),
  CONSTRAINT `agtMissionGroups_ibfk_2` FOREIGN KEY (`divisionID`) REFERENCES `crpNPCDivisions` (`divisionID`),
  CONSTRAINT `agtMissionGroups_ibfk_3` FOREIGN KEY (`missionID`) REFERENCES `agtMissions` (`missionID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `agtMissionTypes`
--

DROP TABLE IF EXISTS `agtMissionTypes`;
CREATE TABLE `agtMissionTypes` (
  `missionTypeID` int(10) unsigned NOT NULL auto_increment,
  `missionTypeName` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`missionTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `agtMissions`
--

DROP TABLE IF EXISTS `agtMissions`;
CREATE TABLE `agtMissions` (
  `missionID` int(10) unsigned NOT NULL auto_increment,
  `missionName` varchar(255) NOT NULL default '',
  `missionLevel` tinyint(3) unsigned NOT NULL default '0',
  `missionTypeID` int(10) unsigned NOT NULL default '0',
  `importantMission` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`missionID`),
  KEY `missionLevel` (`missionLevel`),
  KEY `missionTypeID` (`missionTypeID`),
  CONSTRAINT `agtMissions_ibfk_1` FOREIGN KEY (`missionTypeID`) REFERENCES `agtMissionTypes` (`missionTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `agtOfferGroups`
--

DROP TABLE IF EXISTS `agtOfferGroups`;
CREATE TABLE `agtOfferGroups` (
  `corporationID` int(10) unsigned NOT NULL default '0',
  `divisionID` int(10) unsigned NOT NULL default '0',
  `level` int(10) unsigned NOT NULL default '0',
  `offerID` int(10) unsigned default NULL,
  PRIMARY KEY  (`corporationID`,`divisionID`,`level`),
  KEY `divisionID` (`divisionID`),
  KEY `offerID` (`offerID`),
  CONSTRAINT `agtOfferGroups_ibfk_1` FOREIGN KEY (`corporationID`) REFERENCES `agtMissionTypes` (`missionTypeID`),
  CONSTRAINT `agtOfferGroups_ibfk_2` FOREIGN KEY (`divisionID`) REFERENCES `crpNPCDivisions` (`divisionID`),
  CONSTRAINT `agtOfferGroups_ibfk_3` FOREIGN KEY (`offerID`) REFERENCES `agtOffers` (`offerID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `agtOfferRequired`
--

DROP TABLE IF EXISTS `agtOfferRequired`;
CREATE TABLE `agtOfferRequired` (
  `offerID` int(10) unsigned NOT NULL default '0',
  `typeID` int(10) unsigned NOT NULL default '0',
  `quantity` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`offerID`,`typeID`),
  KEY `typeID` (`typeID`),
  CONSTRAINT `agtOfferRequired_ibfk_1` FOREIGN KEY (`offerID`) REFERENCES `agtOffers` (`offerID`),
  CONSTRAINT `agtOfferRequired_ibfk_2` FOREIGN KEY (`typeID`) REFERENCES `invTypes` (`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `agtOfferReward`
--

DROP TABLE IF EXISTS `agtOfferReward`;
CREATE TABLE `agtOfferReward` (
  `offerID` int(10) unsigned NOT NULL default '0',
  `typeID` int(10) unsigned NOT NULL default '0',
  `quantity` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`offerID`,`typeID`),
  KEY `typeID` (`typeID`),
  CONSTRAINT `agtOfferReward_ibfk_1` FOREIGN KEY (`offerID`) REFERENCES `agtOffers` (`offerID`),
  CONSTRAINT `agtOfferReward_ibfk_2` FOREIGN KEY (`typeID`) REFERENCES `invTypes` (`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `agtOffers`
--

DROP TABLE IF EXISTS `agtOffers`;
CREATE TABLE `agtOffers` (
  `offerID` int(10) unsigned NOT NULL auto_increment,
  `offerName` varchar(255) NOT NULL default '',
  `offerLevel` tinyint(3) unsigned NOT NULL default '0',
  `loyaltyPoints` int(10) unsigned NOT NULL default '0',
  `requiredISK` int(10) unsigned NOT NULL default '0',
  `rewardISK` int(10) unsigned NOT NULL default '0',
  `offerText` mediumtext NOT NULL,
  `offerAcceptedText` mediumtext NOT NULL,
  PRIMARY KEY  (`offerID`),
  KEY `offerLevel` (`offerLevel`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `alliance_shortnames`
--

DROP TABLE IF EXISTS `alliance_shortnames`;
CREATE TABLE `alliance_shortnames` (
  `allianceID` int(10) unsigned NOT NULL default '0',
  `shortName` text NOT NULL,
  PRIMARY KEY  (`allianceID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `billTypes`
--

DROP TABLE IF EXISTS `billTypes`;
CREATE TABLE `billTypes` (
  `billTypeID` int(10) unsigned NOT NULL default '0',
  `billTypeName` varchar(100) NOT NULL default '',
  `description` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`billTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `billsPayable`
--

DROP TABLE IF EXISTS `billsPayable`;
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

--
-- Table structure for table `billsReceivable`
--

DROP TABLE IF EXISTS `billsReceivable`;
CREATE TABLE `billsReceivable` (
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

--
-- Table structure for table `bloodlineTypes`
--

DROP TABLE IF EXISTS `bloodlineTypes`;
CREATE TABLE `bloodlineTypes` (
  `bloodlineID` int(10) unsigned NOT NULL default '0',
  `typeID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`bloodlineID`),
  KEY `typeID` (`typeID`),
  CONSTRAINT `bloodlineTypes_ibfk_1` FOREIGN KEY (`bloodlineID`) REFERENCES `chrBloodlines` (`bloodlineID`),
  CONSTRAINT `FK_BLOODLINETYPES_TYPES` FOREIGN KEY (`typeID`) REFERENCES `invTypes` (`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `bookmarks`
--

DROP TABLE IF EXISTS `bookmarks`;
CREATE TABLE `bookmarks` (
  `bookmarkID` int(10) unsigned NOT NULL default '0',
  `ownerID` int(10) unsigned NOT NULL default '0',
  `itemID` int(10) unsigned NOT NULL default '0',
  `typeID` int(10) unsigned NOT NULL default '0',
  `flag` int(10) unsigned NOT NULL default '0',
  `memo` varchar(85) NOT NULL default '',
  `created` int(10) unsigned NOT NULL default '0',
  `x` double NOT NULL default '0',
  `y` double NOT NULL default '0',
  `z` double NOT NULL default '0',
  `locationID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`bookmarkID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `cacheLocations`
--

DROP TABLE IF EXISTS `cacheLocations`;
CREATE TABLE `cacheLocations` (
  `locationID` int(10) unsigned NOT NULL default '0',
  `locationName` varchar(100) NOT NULL default '',
  `x` double NOT NULL default '0',
  `y` double NOT NULL default '0',
  `z` double NOT NULL default '0',
  PRIMARY KEY  (`locationID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `cacheOwners`
--

DROP TABLE IF EXISTS `cacheOwners`;
CREATE TABLE `cacheOwners` (
  `ownerID` int(10) unsigned NOT NULL default '0',
  `ownerName` varchar(100) NOT NULL default '',
  `typeID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`ownerID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `channelChars`
--

DROP TABLE IF EXISTS `channelChars`;
CREATE TABLE `channelChars` (
  `channelID` int(10) unsigned NOT NULL default '0',
  `corpID` int(10) unsigned NOT NULL default '0',
  `charID` int(10) unsigned NOT NULL default '0',
  `allianceID` int(10) unsigned NOT NULL default '0',
  `role` int(10) unsigned NOT NULL default '0',
  `extra` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`channelID`,`charID`),
  KEY `FK_CHANNELCHARS_CHARACTER` (`charID`),
  KEY `FK_CHANNELCHARS_CORPORATIONS` (`corpID`),
  KEY `FK_CHANNELCHARS_ALLIANCES` (`allianceID`),
  CONSTRAINT `FK_CHANNELCHARS_ALLIANCES` FOREIGN KEY (`allianceID`) REFERENCES `alliance_shortnames` (`allianceID`),
  CONSTRAINT `FK_CHANNELCHARS_CHANNELS` FOREIGN KEY (`channelID`) REFERENCES `channels` (`channelID`),
  CONSTRAINT `FK_CHANNELCHARS_CHARACTER` FOREIGN KEY (`charID`) REFERENCES `character_` (`characterID`),
  CONSTRAINT `FK_CHANNELCHARS_CORPORATIONS` FOREIGN KEY (`corpID`) REFERENCES `corporation` (`corporationID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `channelMods`
--

DROP TABLE IF EXISTS `channelMods`;
CREATE TABLE `channelMods` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `channelID` int(10) unsigned NOT NULL default '0',
  `accessor` int(10) unsigned default NULL,
  `mode` int(10) unsigned NOT NULL default '0',
  `untilWhen` bigint(20) unsigned default NULL,
  `originalMode` int(10) unsigned default NULL,
  `admin` varchar(85) default NULL,
  `reason` text,
  PRIMARY KEY  (`id`),
  KEY `FK_CHANNELMODS_CHANNELS` (`channelID`),
  CONSTRAINT `FK_CHANNELMODS_CHANNELS` FOREIGN KEY (`channelID`) REFERENCES `channels` (`channelID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `channelTypes`
--

DROP TABLE IF EXISTS `channelTypes`;
CREATE TABLE `channelTypes` (
  `channelTypeID` int(10) unsigned NOT NULL default '0',
  `channelTypeName` varchar(100) NOT NULL default '',
  `defaultName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`channelTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `channels`
--

DROP TABLE IF EXISTS `channels`;
CREATE TABLE `channels` (
  `channelID` int(10) unsigned NOT NULL auto_increment,
  `ownerID` int(10) unsigned NOT NULL default '0',
  `displayName` varchar(85) default NULL,
  `motd` text,
  `comparisonKey` varchar(11) default NULL,
  `memberless` tinyint(4) NOT NULL default '0',
  `password` varchar(100) default NULL,
  `mailingList` tinyint(4) NOT NULL default '0',
  `cspa` tinyint(4) NOT NULL default '0',
  `temporary` tinyint(4) NOT NULL default '0',
  `mode` tinyint(4) NOT NULL default '0',
  `subscribed` tinyint(4) NOT NULL default '0',
  `estimatedMemberCount` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`channelID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `channelsStatic`
--

DROP TABLE IF EXISTS `channelsStatic`;
CREATE TABLE `channelsStatic` (
  `channelID` int(10) unsigned NOT NULL auto_increment,
  `ownerID` int(10) unsigned NOT NULL default '0',
  `displayName` varchar(85) default NULL,
  `motd` text,
  `comparisonKey` varchar(11) default NULL,
  `memberless` tinyint(4) NOT NULL default '0',
  `password` varchar(100) default NULL,
  `mailingList` tinyint(4) NOT NULL default '0',
  `cspa` tinyint(4) NOT NULL default '0',
  `temporary` tinyint(4) NOT NULL default '0',
  `mode` tinyint(4) NOT NULL default '0',
  `subscribed` tinyint(4) NOT NULL default '0',
  `estimatedMemberCount` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`channelID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `characterStatic`
--

DROP TABLE IF EXISTS `characterStatic`;
CREATE TABLE `characterStatic` (
  `characterID` int(10) unsigned NOT NULL default '0',
  `characterName` varchar(43) NOT NULL default '',
  `accountID` int(10) unsigned default NULL,
  `title` varchar(85) NOT NULL default '',
  `description` text NOT NULL,
  `bounty` double NOT NULL default '0',
  `balance` double NOT NULL default '0',
  `securityRating` double NOT NULL default '0',
  `petitionMessage` varchar(85) NOT NULL default '',
  `logonMinutes` int(10) unsigned NOT NULL default '0',
  `corporationID` int(10) unsigned NOT NULL default '0',
  `corporationDateTime` bigint(20) unsigned NOT NULL default '0',
  `startDateTime` bigint(20) unsigned NOT NULL default '0',
  `createDateTime` bigint(20) unsigned NOT NULL default '0',
  `typeID` int(10) unsigned NOT NULL default '0',
  `raceID` int(10) unsigned NOT NULL default '0',
  `bloodlineID` int(10) unsigned NOT NULL default '0',
  `gender` tinyint(4) NOT NULL default '0',
  `accessoryID` int(10) unsigned default NULL,
  `beardID` int(10) unsigned default NULL,
  `costumeID` int(10) unsigned NOT NULL default '0',
  `decoID` int(10) unsigned default NULL,
  `eyebrowsID` int(10) unsigned NOT NULL default '0',
  `eyesID` int(10) unsigned NOT NULL default '0',
  `hairID` int(10) unsigned NOT NULL default '0',
  `lipstickID` int(10) unsigned default NULL,
  `makeupID` int(10) unsigned default NULL,
  `skinID` int(10) unsigned NOT NULL default '0',
  `backgroundID` int(10) unsigned NOT NULL default '0',
  `lightID` int(10) unsigned NOT NULL default '0',
  `headRotation1` double NOT NULL default '0',
  `headRotation2` double NOT NULL default '0',
  `headRotation3` double NOT NULL default '0',
  `eyeRotation1` double NOT NULL default '0',
  `eyeRotation2` double NOT NULL default '0',
  `eyeRotation3` double NOT NULL default '0',
  `camPos1` double NOT NULL default '0',
  `camPos2` double NOT NULL default '0',
  `camPos3` double NOT NULL default '0',
  `morph1e` double default NULL,
  `morph1n` double default NULL,
  `morph1s` double default NULL,
  `morph1w` double default NULL,
  `morph2e` double default NULL,
  `morph2n` double default NULL,
  `morph2s` double default NULL,
  `morph2w` double default NULL,
  `morph3e` double default NULL,
  `morph3n` double default NULL,
  `morph3s` double default NULL,
  `morph3w` double default NULL,
  `morph4e` double default NULL,
  `morph4n` double default NULL,
  `morph4s` double default NULL,
  `morph4w` double default NULL,
  `stationID` int(10) unsigned NOT NULL default '0',
  `solarSystemID` int(10) unsigned NOT NULL default '0',
  `constellationID` int(10) unsigned NOT NULL default '0',
  `regionID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`characterID`),
  UNIQUE KEY `characterName` (`characterName`),
  KEY `accountID` (`accountID`),
  KEY `typeID` (`typeID`),
  KEY `accessoryID` (`accessoryID`),
  KEY `beardID` (`beardID`),
  KEY `costumeID` (`costumeID`),
  KEY `decoID` (`decoID`),
  KEY `eyebrowsID` (`eyebrowsID`),
  KEY `eyesID` (`eyesID`),
  KEY `hairID` (`hairID`),
  KEY `lipstickID` (`lipstickID`),
  KEY `makeupID` (`makeupID`),
  KEY `skinID` (`skinID`),
  KEY `backgroundID` (`backgroundID`),
  KEY `lightID` (`lightID`),
  CONSTRAINT `characterStatic_ibfk_1` FOREIGN KEY (`lightID`) REFERENCES `chrLights` (`lightID`),
  CONSTRAINT `characterStatic_ibfk_10` FOREIGN KEY (`hairID`) REFERENCES `chrHairs` (`hairID`),
  CONSTRAINT `characterStatic_ibfk_11` FOREIGN KEY (`lipstickID`) REFERENCES `chrLipsticks` (`lipstickID`),
  CONSTRAINT `characterStatic_ibfk_12` FOREIGN KEY (`makeupID`) REFERENCES `chrMakeups` (`makeupID`),
  CONSTRAINT `characterStatic_ibfk_13` FOREIGN KEY (`skinID`) REFERENCES `chrSkins` (`skinID`),
  CONSTRAINT `characterStatic_ibfk_14` FOREIGN KEY (`typeID`) REFERENCES `invTypes` (`typeID`),
  CONSTRAINT `characterStatic_ibfk_2` FOREIGN KEY (`accountID`) REFERENCES `account` (`accountID`),
  CONSTRAINT `characterStatic_ibfk_3` FOREIGN KEY (`accessoryID`) REFERENCES `chrAccessories` (`accessoryID`),
  CONSTRAINT `characterStatic_ibfk_4` FOREIGN KEY (`backgroundID`) REFERENCES `chrBackgrounds` (`backgroundID`),
  CONSTRAINT `characterStatic_ibfk_5` FOREIGN KEY (`beardID`) REFERENCES `chrBeards` (`beardID`),
  CONSTRAINT `characterStatic_ibfk_6` FOREIGN KEY (`costumeID`) REFERENCES `chrCostumes` (`costumeID`),
  CONSTRAINT `characterStatic_ibfk_7` FOREIGN KEY (`decoID`) REFERENCES `chrDecos` (`decoID`),
  CONSTRAINT `characterStatic_ibfk_8` FOREIGN KEY (`eyebrowsID`) REFERENCES `chrEyebrows` (`eyebrowsID`),
  CONSTRAINT `characterStatic_ibfk_9` FOREIGN KEY (`eyesID`) REFERENCES `chrEyes` (`eyesID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `character_`
--

DROP TABLE IF EXISTS `character_`;
CREATE TABLE `character_` (
  `characterID` int(10) unsigned NOT NULL default '0',
  `characterName` varchar(43) NOT NULL default '',
  `accountID` int(10) unsigned default NULL,
  `title` varchar(85) NOT NULL default '',
  `description` text NOT NULL,
  `bounty` double NOT NULL default '0',
  `balance` double NOT NULL default '0',
  `securityRating` double NOT NULL default '0',
  `petitionMessage` varchar(85) NOT NULL default '',
  `logonMinutes` int(10) unsigned NOT NULL default '0',
  `corporationID` int(10) unsigned NOT NULL default '0',
  `corporationDateTime` bigint(20) unsigned NOT NULL default '0',
  `startDateTime` bigint(20) unsigned NOT NULL default '0',
  `createDateTime` bigint(20) unsigned NOT NULL default '0',
  `typeID` int(10) unsigned NOT NULL default '0',
  `raceID` int(10) unsigned NOT NULL default '0',
  `bloodlineID` int(10) unsigned NOT NULL default '0',
  `gender` tinyint(4) NOT NULL default '0',
  `accessoryID` int(10) unsigned default NULL,
  `beardID` int(10) unsigned default NULL,
  `costumeID` int(10) unsigned NOT NULL default '0',
  `decoID` int(10) unsigned default NULL,
  `eyebrowsID` int(10) unsigned NOT NULL default '0',
  `eyesID` int(10) unsigned NOT NULL default '0',
  `hairID` int(10) unsigned NOT NULL default '0',
  `lipstickID` int(10) unsigned default NULL,
  `makeupID` int(10) unsigned default NULL,
  `skinID` int(10) unsigned NOT NULL default '0',
  `backgroundID` int(10) unsigned NOT NULL default '0',
  `lightID` int(10) unsigned NOT NULL default '0',
  `headRotation1` double NOT NULL default '0',
  `headRotation2` double NOT NULL default '0',
  `headRotation3` double NOT NULL default '0',
  `eyeRotation1` double NOT NULL default '0',
  `eyeRotation2` double NOT NULL default '0',
  `eyeRotation3` double NOT NULL default '0',
  `camPos1` double NOT NULL default '0',
  `camPos2` double NOT NULL default '0',
  `camPos3` double NOT NULL default '0',
  `morph1e` double default NULL,
  `morph1n` double default NULL,
  `morph1s` double default NULL,
  `morph1w` double default NULL,
  `morph2e` double default NULL,
  `morph2n` double default NULL,
  `morph2s` double default NULL,
  `morph2w` double default NULL,
  `morph3e` double default NULL,
  `morph3n` double default NULL,
  `morph3s` double default NULL,
  `morph3w` double default NULL,
  `morph4e` double default NULL,
  `morph4n` double default NULL,
  `morph4s` double default NULL,
  `morph4w` double default NULL,
  `stationID` int(10) unsigned NOT NULL default '0',
  `solarSystemID` int(10) unsigned NOT NULL default '0',
  `constellationID` int(10) unsigned NOT NULL default '0',
  `regionID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`characterID`),
  UNIQUE KEY `characterName` (`characterName`),
  KEY `FK_CHARACTER__ACCOUNTS` (`accountID`),
  KEY `FK_CHARACTER__INVTYPES` (`typeID`),
  KEY `FK_CHARACTER__CHRACCESSORIES` (`accessoryID`),
  KEY `FK_CHARACTER__CHRBEARDS` (`beardID`),
  KEY `FK_CHARACTER__CHRCOSTUMES` (`costumeID`),
  KEY `FK_CHARACTER__CHRDECOS` (`decoID`),
  KEY `FK_CHARACTER__CHREYEBROWS` (`eyebrowsID`),
  KEY `FK_CHARACTER__CHREYES` (`eyesID`),
  KEY `FK_CHARACTER__CHRHAIRS` (`hairID`),
  KEY `FK_CHARACTER__CHRLIPSTICKS` (`lipstickID`),
  KEY `FK_CHARACTER__CHRMAKEUPS` (`makeupID`),
  KEY `FK_CHARACTER__CHRSKINS` (`skinID`),
  KEY `FK_CHARACTER__CHRBACKGROUNDS` (`backgroundID`),
  KEY `FK_CHARACTER__CHRLIGHTS` (`lightID`),
  CONSTRAINT `FK_CHARACTER__ACCOUNTS` FOREIGN KEY (`accountID`) REFERENCES `account` (`accountID`),
  CONSTRAINT `FK_CHARACTER__CHRACCESSORIES` FOREIGN KEY (`accessoryID`) REFERENCES `chrAccessories` (`accessoryID`),
  CONSTRAINT `FK_CHARACTER__CHRBACKGROUNDS` FOREIGN KEY (`backgroundID`) REFERENCES `chrBackgrounds` (`backgroundID`),
  CONSTRAINT `FK_CHARACTER__CHRBEARDS` FOREIGN KEY (`beardID`) REFERENCES `chrBeards` (`beardID`),
  CONSTRAINT `FK_CHARACTER__CHRCOSTUMES` FOREIGN KEY (`costumeID`) REFERENCES `chrCostumes` (`costumeID`),
  CONSTRAINT `FK_CHARACTER__CHRDECOS` FOREIGN KEY (`decoID`) REFERENCES `chrDecos` (`decoID`),
  CONSTRAINT `FK_CHARACTER__CHREYEBROWS` FOREIGN KEY (`eyebrowsID`) REFERENCES `chrEyebrows` (`eyebrowsID`),
  CONSTRAINT `FK_CHARACTER__CHREYES` FOREIGN KEY (`eyesID`) REFERENCES `chrEyes` (`eyesID`),
  CONSTRAINT `FK_CHARACTER__CHRHAIRS` FOREIGN KEY (`hairID`) REFERENCES `chrHairs` (`hairID`),
  CONSTRAINT `FK_CHARACTER__CHRLIGHTS` FOREIGN KEY (`lightID`) REFERENCES `chrLights` (`lightID`),
  CONSTRAINT `FK_CHARACTER__CHRLIPSTICKS` FOREIGN KEY (`lipstickID`) REFERENCES `chrLipsticks` (`lipstickID`),
  CONSTRAINT `FK_CHARACTER__CHRMAKEUPS` FOREIGN KEY (`makeupID`) REFERENCES `chrMakeups` (`makeupID`),
  CONSTRAINT `FK_CHARACTER__CHRSKINS` FOREIGN KEY (`skinID`) REFERENCES `chrSkins` (`skinID`),
  CONSTRAINT `FK_CHARACTER__INVTYPES` FOREIGN KEY (`typeID`) REFERENCES `invTypes` (`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrAccessories`
--

DROP TABLE IF EXISTS `chrAccessories`;
CREATE TABLE `chrAccessories` (
  `accessoryID` int(10) unsigned NOT NULL default '0',
  `accessoryName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`accessoryID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrAncestries`
--

DROP TABLE IF EXISTS `chrAncestries`;
CREATE TABLE `chrAncestries` (
  `ancestryID` int(10) unsigned NOT NULL default '0',
  `ancestryName` varchar(100) NOT NULL default '',
  `bloodlineID` int(10) unsigned NOT NULL default '0',
  `description` mediumtext NOT NULL,
  `postPerception` int(10) unsigned NOT NULL default '0',
  `postWillpower` int(10) unsigned NOT NULL default '0',
  `postCharisma` int(10) unsigned NOT NULL default '0',
  `postMemory` int(10) unsigned NOT NULL default '0',
  `postIntelligence` int(10) unsigned NOT NULL default '0',
  `perception` int(10) unsigned NOT NULL default '0',
  `willpower` int(10) unsigned NOT NULL default '0',
  `charisma` int(10) unsigned NOT NULL default '0',
  `memory` int(10) unsigned NOT NULL default '0',
  `intelligence` int(10) unsigned NOT NULL default '0',
  `skillTypeID1` int(10) unsigned NOT NULL default '0',
  `skillTypeID2` int(10) unsigned NOT NULL default '0',
  `typeID` int(10) unsigned default NULL,
  `typeQuantity` int(10) unsigned default NULL,
  `typeID2` text,
  `typeQuantity2` int(10) unsigned default NULL,
  `graphicID` int(10) unsigned default NULL,
  PRIMARY KEY  (`ancestryID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrAttributes`
--

DROP TABLE IF EXISTS `chrAttributes`;
CREATE TABLE `chrAttributes` (
  `attributeID` int(10) unsigned NOT NULL default '0',
  `attributeName` varchar(100) NOT NULL default '',
  `description` mediumtext NOT NULL,
  `graphicID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`attributeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrBLAccessories`
--

DROP TABLE IF EXISTS `chrBLAccessories`;
CREATE TABLE `chrBLAccessories` (
  `bloodlineID` int(10) unsigned NOT NULL default '0',
  `gender` int(10) unsigned NOT NULL default '0',
  `accessoryID` int(10) unsigned NOT NULL default '0',
  `npc` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`bloodlineID`,`gender`,`accessoryID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrBLBackgrounds`
--

DROP TABLE IF EXISTS `chrBLBackgrounds`;
CREATE TABLE `chrBLBackgrounds` (
  `backgroundID` int(10) unsigned NOT NULL default '0',
  `backgroundName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`backgroundID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrBLBeards`
--

DROP TABLE IF EXISTS `chrBLBeards`;
CREATE TABLE `chrBLBeards` (
  `bloodlineID` int(10) unsigned NOT NULL default '0',
  `gender` int(10) unsigned NOT NULL default '0',
  `beardID` int(10) unsigned NOT NULL default '0',
  `npc` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`bloodlineID`,`gender`,`beardID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrBLCostumes`
--

DROP TABLE IF EXISTS `chrBLCostumes`;
CREATE TABLE `chrBLCostumes` (
  `bloodlineID` int(10) unsigned NOT NULL default '0',
  `gender` int(10) unsigned NOT NULL default '0',
  `costumeID` int(10) unsigned NOT NULL default '0',
  `npc` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`bloodlineID`,`gender`,`costumeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrBLDecos`
--

DROP TABLE IF EXISTS `chrBLDecos`;
CREATE TABLE `chrBLDecos` (
  `bloodlineID` int(10) unsigned NOT NULL default '0',
  `gender` int(10) unsigned NOT NULL default '0',
  `decoID` int(10) unsigned NOT NULL default '0',
  `npc` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`bloodlineID`,`gender`,`decoID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrBLEyebrows`
--

DROP TABLE IF EXISTS `chrBLEyebrows`;
CREATE TABLE `chrBLEyebrows` (
  `bloodlineID` int(10) unsigned NOT NULL default '0',
  `gender` int(10) unsigned NOT NULL default '0',
  `eyebrowsID` int(10) unsigned NOT NULL default '0',
  `npc` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`bloodlineID`,`gender`,`eyebrowsID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrBLEyes`
--

DROP TABLE IF EXISTS `chrBLEyes`;
CREATE TABLE `chrBLEyes` (
  `bloodlineID` int(10) unsigned NOT NULL default '0',
  `gender` int(10) unsigned NOT NULL default '0',
  `eyesID` int(10) unsigned NOT NULL default '0',
  `npc` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`bloodlineID`,`gender`,`eyesID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrBLHairs`
--

DROP TABLE IF EXISTS `chrBLHairs`;
CREATE TABLE `chrBLHairs` (
  `bloodlineID` int(10) unsigned NOT NULL default '0',
  `gender` int(10) unsigned NOT NULL default '0',
  `hairID` int(10) unsigned NOT NULL default '0',
  `npc` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`bloodlineID`,`gender`,`hairID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrBLLights`
--

DROP TABLE IF EXISTS `chrBLLights`;
CREATE TABLE `chrBLLights` (
  `lightID` int(10) unsigned NOT NULL default '0',
  `lightName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`lightID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrBLLipsticks`
--

DROP TABLE IF EXISTS `chrBLLipsticks`;
CREATE TABLE `chrBLLipsticks` (
  `bloodlineID` int(10) unsigned NOT NULL default '0',
  `gender` int(10) unsigned NOT NULL default '0',
  `lipstickID` int(10) unsigned NOT NULL default '0',
  `npc` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`bloodlineID`,`gender`,`lipstickID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrBLMakeups`
--

DROP TABLE IF EXISTS `chrBLMakeups`;
CREATE TABLE `chrBLMakeups` (
  `bloodlineID` int(10) unsigned NOT NULL default '0',
  `gender` int(10) unsigned NOT NULL default '0',
  `makeupID` int(10) unsigned NOT NULL default '0',
  `npc` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`bloodlineID`,`gender`,`makeupID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrBLSkins`
--

DROP TABLE IF EXISTS `chrBLSkins`;
CREATE TABLE `chrBLSkins` (
  `bloodlineID` int(10) unsigned NOT NULL default '0',
  `gender` int(10) unsigned NOT NULL default '0',
  `skinID` int(10) unsigned NOT NULL default '0',
  `npc` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`bloodlineID`,`gender`,`skinID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrBackgrounds`
--

DROP TABLE IF EXISTS `chrBackgrounds`;
CREATE TABLE `chrBackgrounds` (
  `backgroundID` int(10) unsigned NOT NULL default '0',
  `backgroundName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`backgroundID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrBeards`
--

DROP TABLE IF EXISTS `chrBeards`;
CREATE TABLE `chrBeards` (
  `beardID` int(10) unsigned NOT NULL default '0',
  `beardName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`beardID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrBloodlines`
--

DROP TABLE IF EXISTS `chrBloodlines`;
CREATE TABLE `chrBloodlines` (
  `bloodlineID` int(10) unsigned NOT NULL auto_increment,
  `bloodlineName` varchar(128) NOT NULL default '',
  `raceID` int(11) NOT NULL default '0',
  `description` text,
  `maleDescription` text,
  `femaleDescription` text,
  `shipTypeID` int(11) NOT NULL default '0',
  `corporationID` int(11) default NULL,
  `perception` int(11) default NULL,
  `willpower` int(11) default NULL,
  `charisma` int(11) default NULL,
  `memory` int(11) default NULL,
  `intelligence` int(11) default NULL,
  `bonusTypeID` int(11) default NULL,
  `skillTypeID1` int(11) default NULL,
  `skillTypeID2` int(11) default NULL,
  `graphicID` int(11) default NULL,
  PRIMARY KEY  (`bloodlineID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrCorporationRoles`
--

DROP TABLE IF EXISTS `chrCorporationRoles`;
CREATE TABLE `chrCorporationRoles` (
  `characterID` int(10) unsigned NOT NULL default '0',
  `corprole` bigint(20) unsigned NOT NULL default '0',
  `rolesAtAll` bigint(20) unsigned NOT NULL default '0',
  `rolesAtBase` bigint(20) unsigned NOT NULL default '0',
  `rolesAtHQ` bigint(20) unsigned NOT NULL default '0',
  `rolesAtOther` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`characterID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrCostumes`
--

DROP TABLE IF EXISTS `chrCostumes`;
CREATE TABLE `chrCostumes` (
  `costumeID` int(10) unsigned NOT NULL default '0',
  `costumeName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`costumeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrDecos`
--

DROP TABLE IF EXISTS `chrDecos`;
CREATE TABLE `chrDecos` (
  `decoID` int(10) unsigned NOT NULL default '0',
  `decoName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`decoID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrDepartments`
--

DROP TABLE IF EXISTS `chrDepartments`;
CREATE TABLE `chrDepartments` (
  `schoolID` int(10) unsigned NOT NULL default '0',
  `departmentID` int(10) unsigned NOT NULL default '0',
  `departmentName` varchar(100) NOT NULL default '',
  `description` mediumtext NOT NULL,
  `skillTypeID1` int(10) unsigned NOT NULL default '0',
  `skillTypeID2` int(10) unsigned NOT NULL default '0',
  `skillTypeID3` int(10) unsigned NOT NULL default '0',
  `graphicID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`departmentID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrEmployment`
--

DROP TABLE IF EXISTS `chrEmployment`;
CREATE TABLE `chrEmployment` (
  `characterID` int(10) unsigned NOT NULL default '0',
  `corporationID` int(10) unsigned NOT NULL default '0',
  `startDate` bigint(20) unsigned NOT NULL default '0',
  `deleted` tinyint(4) NOT NULL default '0',
  PRIMARY KEY  (`characterID`,`corporationID`,`startDate`),
  KEY `corporationID` (`corporationID`),
  CONSTRAINT `chrEmployment_ibfk_1` FOREIGN KEY (`characterID`) REFERENCES `character_` (`characterID`),
  CONSTRAINT `chrEmployment_ibfk_2` FOREIGN KEY (`corporationID`) REFERENCES `corporation` (`corporationID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrEyebrows`
--

DROP TABLE IF EXISTS `chrEyebrows`;
CREATE TABLE `chrEyebrows` (
  `eyebrowsID` int(10) unsigned NOT NULL default '0',
  `eyebrowsName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`eyebrowsID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrEyes`
--

DROP TABLE IF EXISTS `chrEyes`;
CREATE TABLE `chrEyes` (
  `eyesID` int(10) unsigned NOT NULL default '0',
  `eyesName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`eyesID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrFactions`
--

DROP TABLE IF EXISTS `chrFactions`;
CREATE TABLE `chrFactions` (
  `factionID` int(10) unsigned NOT NULL default '0',
  `factionName` varchar(128) NOT NULL default '',
  `description` text,
  `raceIDs` int(11) default NULL,
  `solarSystemID` int(11) default NULL,
  `corporationID` int(11) default NULL,
  `sizeFactor` double default NULL,
  `stationCount` int(11) default NULL,
  `stationSystemCount` int(11) default NULL,
  PRIMARY KEY  (`factionID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrFields`
--

DROP TABLE IF EXISTS `chrFields`;
CREATE TABLE `chrFields` (
  `departmentID` int(10) unsigned NOT NULL default '0',
  `fieldID` int(10) unsigned NOT NULL default '0',
  `fieldName` varchar(100) NOT NULL default '',
  `description` mediumtext NOT NULL,
  `skillTypeID1` int(10) unsigned NOT NULL default '0',
  `skillTypeID2` int(10) unsigned NOT NULL default '0',
  `typeID` int(10) unsigned default NULL,
  `typeQuantity` int(10) unsigned NOT NULL default '0',
  `typeID2` text,
  `typeQuantity2` int(10) unsigned NOT NULL default '0',
  `graphicID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`fieldID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrHairs`
--

DROP TABLE IF EXISTS `chrHairs`;
CREATE TABLE `chrHairs` (
  `hairID` int(10) unsigned NOT NULL default '0',
  `hairName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`hairID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrLights`
--

DROP TABLE IF EXISTS `chrLights`;
CREATE TABLE `chrLights` (
  `lightID` int(10) unsigned NOT NULL default '0',
  `lightName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`lightID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrLipsticks`
--

DROP TABLE IF EXISTS `chrLipsticks`;
CREATE TABLE `chrLipsticks` (
  `lipstickID` int(10) unsigned NOT NULL default '0',
  `lipstickName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`lipstickID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrMakeups`
--

DROP TABLE IF EXISTS `chrMakeups`;
CREATE TABLE `chrMakeups` (
  `makeupID` int(10) unsigned NOT NULL default '0',
  `makeupName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`makeupID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrMissionState`
--

DROP TABLE IF EXISTS `chrMissionState`;
CREATE TABLE `chrMissionState` (
  `characterID` int(10) unsigned NOT NULL default '0',
  `missionID` int(10) unsigned NOT NULL default '0',
  `missionState` tinyint(3) unsigned NOT NULL default '0',
  `expirationTime` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`characterID`,`missionID`),
  KEY `missionID` (`missionID`),
  CONSTRAINT `chrMissionState_ibfk_1` FOREIGN KEY (`characterID`) REFERENCES `character_` (`characterID`),
  CONSTRAINT `chrMissionState_ibfk_2` FOREIGN KEY (`missionID`) REFERENCES `agtMissions` (`missionID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrNPCStandings`
--

DROP TABLE IF EXISTS `chrNPCStandings`;
CREATE TABLE `chrNPCStandings` (
  `characterID` int(10) unsigned NOT NULL default '0',
  `fromID` int(10) unsigned NOT NULL default '0',
  `standing` double NOT NULL default '0',
  PRIMARY KEY  (`characterID`,`fromID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrNotes`
--

DROP TABLE IF EXISTS `chrNotes`;
CREATE TABLE `chrNotes` (
  `ownerID` int(10) unsigned NOT NULL default '0',
  `itemID` int(10) unsigned NOT NULL default '0',
  `note` text NOT NULL,
  PRIMARY KEY  (`ownerID`,`itemID`),
  KEY `itemID` (`itemID`),
  CONSTRAINT `chrNotes_ibfk_1` FOREIGN KEY (`ownerID`) REFERENCES `character_` (`characterID`),
  CONSTRAINT `chrNotes_ibfk_2` FOREIGN KEY (`itemID`) REFERENCES `entity` (`itemID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrOffers`
--

DROP TABLE IF EXISTS `chrOffers`;
CREATE TABLE `chrOffers` (
  `characterID` int(10) unsigned NOT NULL default '0',
  `offerID` int(10) unsigned NOT NULL default '0',
  `expirationTime` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`characterID`,`offerID`),
  KEY `offerID` (`offerID`),
  CONSTRAINT `chrOffers_ibfk_1` FOREIGN KEY (`characterID`) REFERENCES `character_` (`characterID`),
  CONSTRAINT `chrOffers_ibfk_2` FOREIGN KEY (`offerID`) REFERENCES `agtOffers` (`offerID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrOwnerNote`
--

DROP TABLE IF EXISTS `chrOwnerNote`;
CREATE TABLE `chrOwnerNote` (
  `noteID` int(10) unsigned NOT NULL auto_increment,
  `ownerID` int(10) unsigned NOT NULL default '0',
  `label` text NOT NULL,
  `note` text NOT NULL,
  PRIMARY KEY  (`noteID`),
  KEY `ownerID` (`ownerID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrRaces`
--

DROP TABLE IF EXISTS `chrRaces`;
CREATE TABLE `chrRaces` (
  `raceID` int(10) unsigned NOT NULL default '0',
  `raceName` varchar(128) NOT NULL default '',
  `description` text,
  `skillTypeID1` int(11) default NULL,
  `typeID` int(11) default NULL,
  `typeQuantity` int(11) default NULL,
  `graphicID` int(11) default NULL,
  PRIMARY KEY  (`raceID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrSchools`
--

DROP TABLE IF EXISTS `chrSchools`;
CREATE TABLE `chrSchools` (
  `raceID` int(11) default NULL,
  `schoolID` int(11) NOT NULL default '0',
  `schoolName` varchar(100) NOT NULL default '',
  `description` text,
  `graphicID` int(11) default NULL,
  `corporationID` int(11) default NULL,
  `agentID` int(11) default NULL,
  `newAgentID` int(11) default NULL,
  PRIMARY KEY  (`schoolID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrSkins`
--

DROP TABLE IF EXISTS `chrSkins`;
CREATE TABLE `chrSkins` (
  `skinID` int(10) unsigned NOT NULL default '0',
  `skinName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`skinID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrSpecialities`
--

DROP TABLE IF EXISTS `chrSpecialities`;
CREATE TABLE `chrSpecialities` (
  `fieldID` int(10) unsigned NOT NULL default '0',
  `specialityID` int(10) unsigned NOT NULL default '0',
  `specialityName` varchar(100) NOT NULL default '',
  `description` mediumtext NOT NULL,
  `skillTypeID1` int(10) unsigned NOT NULL default '0',
  `skillTypeID2` int(10) unsigned default NULL,
  `typeID` int(10) unsigned default NULL,
  `typeQuantity` int(10) unsigned NOT NULL default '0',
  `typeID2` text,
  `typeQuantity2` int(10) unsigned NOT NULL default '0',
  `graphicID` int(10) unsigned default NULL,
  PRIMARY KEY  (`specialityID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrStandingChanges`
--

DROP TABLE IF EXISTS `chrStandingChanges`;
CREATE TABLE `chrStandingChanges` (
  `eventID` int(10) unsigned NOT NULL auto_increment,
  `eventTypeID` int(10) unsigned NOT NULL default '0',
  `eventDateTime` bigint(20) unsigned NOT NULL default '0',
  `fromID` int(10) unsigned NOT NULL default '0',
  `toID` int(10) unsigned NOT NULL default '0',
  `modification` double NOT NULL default '0',
  `originalFromID` int(10) unsigned NOT NULL default '0',
  `originalToID` int(10) unsigned NOT NULL default '0',
  `int_1` int(10) unsigned NOT NULL default '0',
  `int_2` int(10) unsigned NOT NULL default '0',
  `int_3` int(10) unsigned NOT NULL default '0',
  `msg` text NOT NULL,
  PRIMARY KEY  (`eventID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `chrStandings`
--

DROP TABLE IF EXISTS `chrStandings`;
CREATE TABLE `chrStandings` (
  `characterID` int(10) unsigned NOT NULL default '0',
  `toID` int(10) unsigned NOT NULL default '0',
  `standing` double NOT NULL default '0',
  PRIMARY KEY  (`characterID`,`toID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `corporation`
--

DROP TABLE IF EXISTS `corporation`;
CREATE TABLE `corporation` (
  `corporationID` int(10) unsigned NOT NULL auto_increment,
  `corporationName` varchar(100) NOT NULL default '',
  `description` mediumtext NOT NULL,
  `tickerName` varchar(8) NOT NULL default '',
  `url` mediumtext NOT NULL,
  `taxRate` double NOT NULL default '0',
  `minimumJoinStanding` double NOT NULL default '0',
  `corporationType` int(10) unsigned NOT NULL default '0',
  `hasPlayerPersonnelManager` tinyint(3) unsigned NOT NULL default '0',
  `sendCharTerminationMessage` tinyint(3) unsigned NOT NULL default '1',
  `creatorID` int(10) unsigned NOT NULL default '0',
  `ceoID` int(10) unsigned NOT NULL default '0',
  `stationID` int(10) unsigned NOT NULL default '0',
  `raceID` int(10) unsigned default NULL,
  `allianceID` int(10) unsigned NOT NULL default '0',
  `shares` bigint(20) unsigned NOT NULL default '1000',
  `memberCount` int(10) unsigned NOT NULL default '0',
  `memberLimit` int(10) unsigned NOT NULL default '10',
  `allowedMemberRaceIDs` int(10) unsigned NOT NULL default '0',
  `graphicID` int(10) unsigned NOT NULL default '0',
  `shape1` int(10) unsigned default NULL,
  `shape2` int(10) unsigned default NULL,
  `shape3` int(10) unsigned default NULL,
  `color1` int(10) unsigned default NULL,
  `color2` int(10) unsigned default NULL,
  `color3` int(10) unsigned default NULL,
  `typeface` varchar(11) default NULL,
  `division1` varchar(11) default NULL,
  `division2` varchar(11) default NULL,
  `division3` varchar(11) default NULL,
  `division4` varchar(11) default NULL,
  `division5` varchar(11) default NULL,
  `division6` varchar(11) default NULL,
  `division7` varchar(11) default NULL,
  `balance` double NOT NULL default '0',
  `deleted` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`corporationID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `corporationStatic`
--

DROP TABLE IF EXISTS `corporationStatic`;
CREATE TABLE `corporationStatic` (
  `corporationID` int(10) unsigned NOT NULL auto_increment,
  `corporationName` varchar(100) NOT NULL default '',
  `description` mediumtext NOT NULL,
  `tickerName` varchar(8) NOT NULL default '',
  `url` mediumtext NOT NULL,
  `taxRate` double NOT NULL default '0',
  `minimumJoinStanding` double NOT NULL default '0',
  `corporationType` int(10) unsigned NOT NULL default '0',
  `hasPlayerPersonnelManager` tinyint(3) unsigned NOT NULL default '0',
  `sendCharTerminationMessage` tinyint(3) unsigned NOT NULL default '0',
  `creatorID` int(10) unsigned NOT NULL default '0',
  `ceoID` int(10) unsigned NOT NULL default '0',
  `stationID` int(10) unsigned NOT NULL default '0',
  `raceID` int(10) unsigned default NULL,
  `allianceID` int(10) unsigned default NULL,
  `shares` bigint(20) unsigned NOT NULL default '0',
  `memberCount` int(10) unsigned NOT NULL default '0',
  `memberLimit` int(10) unsigned NOT NULL default '0',
  `allowedMemberRaceIDs` int(10) unsigned NOT NULL default '0',
  `graphicID` int(10) unsigned NOT NULL default '0',
  `shape1` int(10) unsigned default NULL,
  `shape2` int(10) unsigned default NULL,
  `shape3` int(10) unsigned default NULL,
  `color1` int(10) unsigned default NULL,
  `color2` int(10) unsigned default NULL,
  `color3` int(10) unsigned default NULL,
  `typeface` varchar(11) default NULL,
  `division1` varchar(11) default NULL,
  `division2` varchar(11) default NULL,
  `division3` varchar(11) default NULL,
  `division4` varchar(11) default NULL,
  `division5` varchar(11) default NULL,
  `division6` varchar(11) default NULL,
  `division7` varchar(11) default NULL,
  PRIMARY KEY  (`corporationID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `courierMissions`
--

DROP TABLE IF EXISTS `courierMissions`;
CREATE TABLE `courierMissions` (
  `missionID` int(10) unsigned NOT NULL auto_increment,
  `kind` tinyint(3) unsigned NOT NULL default '0',
  `state` tinyint(3) unsigned NOT NULL default '0',
  `availabilityID` int(10) unsigned default NULL,
  `inOrder` tinyint(3) unsigned NOT NULL default '0',
  `description` text NOT NULL,
  `issuerID` int(10) unsigned default NULL,
  `assigneeID` int(10) unsigned default NULL,
  `acceptFee` double NOT NULL default '0',
  `acceptorID` int(10) unsigned default NULL,
  `dateIssued` int(10) unsigned NOT NULL default '0',
  `dateExpires` int(10) unsigned NOT NULL default '0',
  `dateAccepted` int(10) unsigned NOT NULL default '0',
  `dateCompleted` int(10) unsigned NOT NULL default '0',
  `totalReward` double NOT NULL default '0',
  `tracking` tinyint(3) unsigned NOT NULL default '0',
  `pickupStationID` int(10) unsigned default NULL,
  `craterID` int(10) unsigned default NULL,
  `dropStationID` int(10) unsigned default NULL,
  `volume` double NOT NULL default '0',
  `pickupSolarSystemID` int(10) unsigned default NULL,
  `pickupRegionID` int(10) unsigned default NULL,
  `dropSolarSystemID` int(10) unsigned default NULL,
  `dropRegionID` int(10) unsigned default NULL,
  PRIMARY KEY  (`missionID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `crpCharShares`
--

DROP TABLE IF EXISTS `crpCharShares`;
CREATE TABLE `crpCharShares` (
  `characterID` int(10) unsigned NOT NULL default '0',
  `corporationID` int(10) unsigned NOT NULL default '0',
  `shares` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`characterID`,`corporationID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `crpNPCCorporationDivisions`
--

DROP TABLE IF EXISTS `crpNPCCorporationDivisions`;
CREATE TABLE `crpNPCCorporationDivisions` (
  `corporationID` int(10) unsigned NOT NULL default '0',
  `divisionID` int(10) unsigned NOT NULL default '0',
  `divisionNumber` tinyint(3) unsigned NOT NULL default '0',
  `size` int(10) unsigned NOT NULL default '0',
  `leaderID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`corporationID`,`divisionNumber`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `crpNPCCorporationResearchFields`
--

DROP TABLE IF EXISTS `crpNPCCorporationResearchFields`;
CREATE TABLE `crpNPCCorporationResearchFields` (
  `skillID` int(11) NOT NULL default '0',
  `corporationID` int(11) NOT NULL default '0',
  `supplierType` int(11) default NULL,
  PRIMARY KEY  (`skillID`,`corporationID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `crpNPCCorporationTrades`
--

DROP TABLE IF EXISTS `crpNPCCorporationTrades`;
CREATE TABLE `crpNPCCorporationTrades` (
  `corporationID` int(11) default NULL,
  `typeID` int(11) default NULL,
  `supplyDemand` double default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `crpNPCCorporations`
--

DROP TABLE IF EXISTS `crpNPCCorporations`;
CREATE TABLE `crpNPCCorporations` (
  `corporationID` int(11) NOT NULL default '0',
  `mainActivityID` int(11) default NULL,
  `secondaryActivityID` int(11) default NULL,
  `size` char(1) default NULL,
  `extent` char(1) default NULL,
  `solarSystemID` int(11) default NULL,
  `investorID1` int(11) default NULL,
  `investorShares1` int(11) default NULL,
  `investorID2` int(11) default NULL,
  `investorShares2` int(11) default NULL,
  `investorID3` int(11) default NULL,
  `investorShares3` int(11) default NULL,
  `investorID4` int(11) default NULL,
  `investorShares4` int(11) default NULL,
  `friendID` int(11) default NULL,
  `enemyID` int(11) default NULL,
  `publicShares` int(11) default NULL,
  `initialPrice` int(11) default NULL,
  `minSecurity` double default NULL,
  `scattered` int(11) default NULL,
  `fringe` int(11) default NULL,
  `corridor` int(11) default NULL,
  `hub` int(11) default NULL,
  `border` int(11) default NULL,
  `factionID` int(11) default NULL,
  `sizeFactor` double default NULL,
  `stationCount` int(11) default NULL,
  `stationSystemCount` int(11) default NULL,
  `corporationName` varchar(100) NOT NULL default '',
  `stationID` int(10) unsigned NOT NULL default '0',
  `ceoID` int(10) unsigned NOT NULL default '1',
  PRIMARY KEY  (`corporationID`),
  KEY `factionID` (`factionID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `crpNPCDivisions`
--

DROP TABLE IF EXISTS `crpNPCDivisions`;
CREATE TABLE `crpNPCDivisions` (
  `divisionID` int(10) unsigned NOT NULL default '0',
  `divisionName` varchar(100) NOT NULL default '',
  `description` text,
  `leaderType` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`divisionID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `crpNPCTickerNames`
--

DROP TABLE IF EXISTS `crpNPCTickerNames`;
CREATE TABLE `crpNPCTickerNames` (
  `corporationID` int(10) unsigned NOT NULL default '0',
  `tickerName` varchar(64) NOT NULL default '',
  `shape1` int(10) unsigned default NULL,
  `shape2` int(10) unsigned default NULL,
  `shape3` int(10) unsigned default NULL,
  `color1` text,
  `color2` text,
  `color3` text,
  PRIMARY KEY  (`corporationID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `crpOffices`
--

DROP TABLE IF EXISTS `crpOffices`;
CREATE TABLE `crpOffices` (
  `corporationID` int(10) unsigned NOT NULL default '0',
  `stationID` int(10) unsigned NOT NULL default '0',
  `itemID` int(10) unsigned NOT NULL default '0',
  `typeID` int(10) unsigned NOT NULL default '0',
  `officeFolderID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`corporationID`,`officeFolderID`),
  KEY `itemID` (`itemID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `dgmAttributeTypes`
--

DROP TABLE IF EXISTS `dgmAttributeTypes`;
CREATE TABLE `dgmAttributeTypes` (
  `attributeID` int(10) unsigned NOT NULL default '0',
  `attributeName` varchar(128) NOT NULL default '',
  `attributeCategory` int(11) default NULL,
  `description` text,
  `maxAttributeID` int(11) default NULL,
  `attributeIdx` int(11) default NULL,
  `graphicID` int(11) default NULL,
  `chargeRechargeTimeID` int(11) default NULL,
  `defaultValue` int(11) default NULL,
  `published` int(11) default NULL,
  `displayName` varchar(128) NOT NULL default '',
  `unitID` int(11) default NULL,
  `stackable` int(11) default NULL,
  `highIsGood` int(11) default NULL,
  PRIMARY KEY  (`attributeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `dgmAttributes`
--

DROP TABLE IF EXISTS `dgmAttributes`;
CREATE TABLE `dgmAttributes` (
  `attributeID` int(10) unsigned NOT NULL default '0',
  `attributeName` varchar(100) NOT NULL default '',
  `attributeCategory` int(10) unsigned NOT NULL default '0',
  `description` mediumtext,
  `maxAttributeID` int(10) unsigned default NULL,
  `attributeIdx` int(10) unsigned default NULL,
  `graphicID` int(10) unsigned default NULL,
  `chargeRechargeTimeID` int(10) unsigned default NULL,
  `defaultValue` int(10) unsigned NOT NULL default '0',
  `published` int(10) unsigned NOT NULL default '0',
  `unitID` int(10) unsigned default NULL,
  `displayName` varchar(100) NOT NULL default '',
  `stackable` int(10) unsigned NOT NULL default '0',
  `highIsGood` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`attributeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `dgmEffects`
--

DROP TABLE IF EXISTS `dgmEffects`;
CREATE TABLE `dgmEffects` (
  `effectID` int(10) unsigned NOT NULL default '0',
  `effectName` mediumtext NOT NULL,
  `effectCategory` int(10) unsigned NOT NULL default '0',
  `preExpression` int(10) unsigned NOT NULL default '0',
  `postExpression` int(10) unsigned NOT NULL default '0',
  `description` mediumtext NOT NULL,
  `guid` varchar(128) default NULL,
  `graphicID` int(10) unsigned default NULL,
  `isOffensive` int(10) unsigned NOT NULL default '0',
  `isAssistance` int(10) unsigned NOT NULL default '0',
  `durationAttributeID` int(10) unsigned default NULL,
  `trackingSpeedAttributeID` int(10) unsigned default NULL,
  `dischargeAttributeID` int(10) unsigned default NULL,
  `rangeAttributeID` int(10) unsigned default NULL,
  `falloffAttributeID` int(10) unsigned default NULL,
  `published` int(10) unsigned NOT NULL default '0',
  `displayName` mediumtext NOT NULL,
  `isWarpSafe` int(10) unsigned NOT NULL default '0',
  `rangeChance` int(10) unsigned NOT NULL default '0',
  `electronicChance` int(10) unsigned NOT NULL default '0',
  `propulsionChance` int(10) unsigned NOT NULL default '0',
  `distribution` int(10) unsigned default NULL,
  `sfxName` varchar(100) default NULL,
  `npcUsageChanceAttributeID` int(10) unsigned default NULL,
  `npcActivationChanceAttributeID` int(10) unsigned default NULL,
  PRIMARY KEY  (`effectID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `dgmTypeAttribs`
--

DROP TABLE IF EXISTS `dgmTypeAttribs`;
CREATE TABLE `dgmTypeAttribs` (
  `typeID` int(10) unsigned NOT NULL default '0',
  `attributeID` int(10) unsigned NOT NULL default '0',
  `value` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`typeID`,`attributeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `dgmTypeAttributes`
--

DROP TABLE IF EXISTS `dgmTypeAttributes`;
CREATE TABLE `dgmTypeAttributes` (
  `typeID` int(11) NOT NULL default '0',
  `attributeID` int(11) NOT NULL default '0',
  `valueInt` int(11) default NULL,
  `valueFloat` double default NULL,
  PRIMARY KEY  (`typeID`,`attributeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `dgmTypeEffects`
--

DROP TABLE IF EXISTS `dgmTypeEffects`;
CREATE TABLE `dgmTypeEffects` (
  `typeID` int(11) NOT NULL default '0',
  `effectID` int(11) NOT NULL default '0',
  `isDefault` int(11) default NULL,
  PRIMARY KEY  (`typeID`,`effectID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `droneState`
--

DROP TABLE IF EXISTS `droneState`;
CREATE TABLE `droneState` (
  `droneID` int(10) unsigned NOT NULL default '0',
  `solarSystemID` int(10) unsigned NOT NULL default '0',
  `ownerID` int(10) unsigned NOT NULL default '0',
  `controllerID` int(10) unsigned NOT NULL default '0',
  `activityState` int(10) unsigned NOT NULL default '0',
  `typeID` int(10) unsigned NOT NULL default '0',
  `controllerOwnerID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`droneID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `entity`
--

DROP TABLE IF EXISTS `entity`;
CREATE TABLE `entity` (
  `itemID` int(10) unsigned NOT NULL auto_increment,
  `itemName` varchar(85) NOT NULL default '',
  `typeID` int(10) unsigned NOT NULL default '0',
  `ownerID` int(10) unsigned NOT NULL default '0',
  `locationID` int(10) unsigned NOT NULL default '0',
  `flag` int(10) unsigned NOT NULL default '0',
  `contraband` int(10) unsigned NOT NULL default '0',
  `singleton` int(10) unsigned NOT NULL default '0',
  `quantity` int(10) unsigned NOT NULL default '0',
  `x` double NOT NULL default '0',
  `y` double NOT NULL default '0',
  `z` double NOT NULL default '0',
  `customInfo` text,
  PRIMARY KEY  (`itemID`),
  KEY `typeID` (`typeID`),
  CONSTRAINT `entity_ibfk_1` FOREIGN KEY (`typeID`) REFERENCES `invTypes` (`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `entityStatic`
--

DROP TABLE IF EXISTS `entityStatic`;
CREATE TABLE `entityStatic` (
  `itemID` int(10) unsigned NOT NULL auto_increment,
  `itemName` varchar(85) NOT NULL default '',
  `typeID` int(10) unsigned NOT NULL default '0',
  `ownerID` int(10) unsigned NOT NULL default '0',
  `locationID` int(10) unsigned NOT NULL default '0',
  `flag` int(10) unsigned NOT NULL default '0',
  `contraband` int(10) unsigned NOT NULL default '0',
  `singleton` int(10) unsigned NOT NULL default '0',
  `quantity` int(10) unsigned NOT NULL default '0',
  `x` double NOT NULL default '0',
  `y` double NOT NULL default '0',
  `z` double NOT NULL default '0',
  `customInfo` text,
  PRIMARY KEY  (`itemID`),
  KEY `typeID` (`typeID`),
  CONSTRAINT `entityStatic_ibfk_1` FOREIGN KEY (`typeID`) REFERENCES `invTypes` (`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `entity_attributes`
--

DROP TABLE IF EXISTS `entity_attributes`;
CREATE TABLE `entity_attributes` (
  `itemID` int(10) unsigned NOT NULL default '0',
  `attributeID` int(10) unsigned NOT NULL default '0',
  `valueInt` int(10) unsigned default NULL,
  `valueFloat` double unsigned default NULL,
  PRIMARY KEY  (`itemID`,`attributeID`),
  KEY `attributeID` (`attributeID`),
  CONSTRAINT `entity_attributes_ibfk_1` FOREIGN KEY (`itemID`) REFERENCES `entity` (`itemID`),
  CONSTRAINT `entity_attributes_ibfk_2` FOREIGN KEY (`attributeID`) REFERENCES `dgmAttributeTypes` (`attributeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `eveCategories`
--

DROP TABLE IF EXISTS `eveCategories`;
CREATE TABLE `eveCategories` (
  `categoryID` int(10) unsigned NOT NULL default '0',
  `categoryName` varchar(100) NOT NULL default '',
  `description` mediumtext NOT NULL,
  `graphicID` int(10) unsigned default NULL,
  PRIMARY KEY  (`categoryID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `eveConstants`
--

DROP TABLE IF EXISTS `eveConstants`;
CREATE TABLE `eveConstants` (
  `constantID` varchar(128) NOT NULL default '',
  `constantValue` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`constantID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `eveEncyclTypes`
--

DROP TABLE IF EXISTS `eveEncyclTypes`;
CREATE TABLE `eveEncyclTypes` (
  `encyclopediaTypeID` int(10) unsigned NOT NULL default '0',
  `encyclopediaTypeName` text NOT NULL,
  PRIMARY KEY  (`encyclopediaTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `eveFlags`
--

DROP TABLE IF EXISTS `eveFlags`;
CREATE TABLE `eveFlags` (
  `flagID` int(10) unsigned NOT NULL default '0',
  `flagName` varchar(100) NOT NULL default '',
  `flagText` varchar(100) NOT NULL default '',
  `flagType` varchar(100) NOT NULL default '',
  `orderID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`flagID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `eveGraphics`
--

DROP TABLE IF EXISTS `eveGraphics`;
CREATE TABLE `eveGraphics` (
  `graphicID` int(11) NOT NULL default '0',
  `url3D` varchar(200) default NULL,
  `urlWeb` varchar(200) default NULL,
  `description` text,
  `published` int(11) default NULL,
  `obsolete` int(11) default NULL,
  `icon` varchar(200) default NULL,
  `urlSound` varchar(200) default NULL,
  `explosionID` int(11) default NULL,
  PRIMARY KEY  (`graphicID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `eveMessages`
--

DROP TABLE IF EXISTS `eveMessages`;
CREATE TABLE `eveMessages` (
  `messageID` varchar(128) NOT NULL default '',
  `messageType` varchar(100) NOT NULL default '',
  `messageText` mediumtext NOT NULL,
  `urlAudio` mediumtext NOT NULL,
  `urlIcon` mediumtext NOT NULL,
  PRIMARY KEY  (`messageID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `eveNames`
--

DROP TABLE IF EXISTS `eveNames`;
CREATE TABLE `eveNames` (
  `itemID` int(10) unsigned NOT NULL default '0',
  `itemName` varchar(100) NOT NULL default '',
  `categoryID` int(10) unsigned NOT NULL default '0',
  `groupID` int(10) unsigned NOT NULL default '0',
  `typeID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`itemID`),
  KEY `typeID` (`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `eveRoles`
--

DROP TABLE IF EXISTS `eveRoles`;
CREATE TABLE `eveRoles` (
  `roleID` int(10) unsigned NOT NULL default '0',
  `roleName` varchar(100) NOT NULL default '',
  `description` mediumtext NOT NULL,
  PRIMARY KEY  (`roleName`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `eveStaticLocations`
--

DROP TABLE IF EXISTS `eveStaticLocations`;
CREATE TABLE `eveStaticLocations` (
  `locationID` int(10) unsigned NOT NULL default '0',
  `locationName` mediumtext NOT NULL,
  `x` double NOT NULL default '0',
  `y` double NOT NULL default '0',
  `z` double NOT NULL default '0',
  PRIMARY KEY  (`locationID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `eveStaticOwners`
--

DROP TABLE IF EXISTS `eveStaticOwners`;
CREATE TABLE `eveStaticOwners` (
  `ownerID` int(10) unsigned NOT NULL default '0',
  `ownerName` varchar(100) NOT NULL default '',
  `typeID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`ownerID`),
  KEY `typeID` (`typeID`),
  CONSTRAINT `eveStaticOwners_ibfk_1` FOREIGN KEY (`typeID`) REFERENCES `invTypes` (`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `eveStaticOwnersStatic`
--

DROP TABLE IF EXISTS `eveStaticOwnersStatic`;
CREATE TABLE `eveStaticOwnersStatic` (
  `ownerID` int(10) unsigned NOT NULL default '0',
  `ownerName` varchar(100) NOT NULL default '',
  `typeID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`ownerID`),
  KEY `typeID` (`typeID`),
  CONSTRAINT `eveStaticOwnersStatic_ibfk_1` FOREIGN KEY (`typeID`) REFERENCES `invTypes` (`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `eveStatistics`
--

DROP TABLE IF EXISTS `eveStatistics`;
CREATE TABLE `eveStatistics` (
  `statisticID` int(10) unsigned NOT NULL default '0',
  `statisticName` varchar(100) NOT NULL default '',
  `statisticType` int(10) unsigned NOT NULL default '0',
  `description` varchar(100) NOT NULL default '',
  `orderID` int(10) unsigned NOT NULL default '0',
  `graphicID` int(10) unsigned default NULL,
  PRIMARY KEY  (`statisticID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `eveUnits`
--

DROP TABLE IF EXISTS `eveUnits`;
CREATE TABLE `eveUnits` (
  `unitID` int(11) NOT NULL default '0',
  `unitName` varchar(100) NOT NULL default '',
  `displayName` varchar(7) default NULL,
  `description` text,
  PRIMARY KEY  (`unitID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `evemail`
--

DROP TABLE IF EXISTS `evemail`;
CREATE TABLE `evemail` (
  `channelID` int(10) unsigned NOT NULL default '0',
  `messageID` int(10) unsigned NOT NULL auto_increment,
  `senderID` int(10) unsigned NOT NULL default '0',
  `subject` varchar(255) NOT NULL default '',
  `created` bigint(20) unsigned NOT NULL default '0',
  `read` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`messageID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `evemailDetails`
--

DROP TABLE IF EXISTS `evemailDetails`;
CREATE TABLE `evemailDetails` (
  `attachmentID` int(10) unsigned NOT NULL auto_increment,
  `messageID` int(10) unsigned NOT NULL default '0',
  `mimeTypeID` int(10) unsigned NOT NULL default '0',
  `attachment` text NOT NULL,
  PRIMARY KEY  (`attachmentID`),
  KEY `messageID` (`messageID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `evemailMimeType`
--

DROP TABLE IF EXISTS `evemailMimeType`;
CREATE TABLE `evemailMimeType` (
  `mimeTypeID` int(10) unsigned NOT NULL auto_increment,
  `mimeType` text NOT NULL,
  `binary` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`mimeTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `factionRaces`
--

DROP TABLE IF EXISTS `factionRaces`;
CREATE TABLE `factionRaces` (
  `factionID` int(10) unsigned NOT NULL default '0',
  `raceID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`factionID`,`raceID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `invBlueprintTypes`
--

DROP TABLE IF EXISTS `invBlueprintTypes`;
CREATE TABLE `invBlueprintTypes` (
  `blueprintTypeID` int(11) NOT NULL default '0',
  `parentBlueprintTypeID` int(11) default NULL,
  `productTypeID` int(11) default NULL,
  `productionTime` int(11) default NULL,
  `techLevel` int(11) default NULL,
  `researchProductivityTime` int(11) default NULL,
  `researchMaterialTime` int(11) default NULL,
  `researchCopyTime` int(11) default NULL,
  `researchTechTime` int(11) default NULL,
  `productivityModifier` int(11) default NULL,
  `materialModifier` int(11) default NULL,
  `wasteFactor` int(11) default NULL,
  `chanceOfReverseEngineering` double default NULL,
  `maxProductionLimit` int(11) default NULL,
  PRIMARY KEY  (`blueprintTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `invCategories`
--

DROP TABLE IF EXISTS `invCategories`;
CREATE TABLE `invCategories` (
  `categoryID` int(10) unsigned NOT NULL default '0',
  `categoryName` varchar(100) NOT NULL default '',
  `description` text,
  `graphicID` int(11) default NULL,
  PRIMARY KEY  (`categoryID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `invContrabandTypes`
--

DROP TABLE IF EXISTS `invContrabandTypes`;
CREATE TABLE `invContrabandTypes` (
  `factionID` int(11) NOT NULL default '0',
  `typeID` int(11) NOT NULL default '0',
  `standingLoss` double default NULL,
  `confiscateMinSec` double default NULL,
  `fineByValue` double default NULL,
  `attackMinSec` double default NULL,
  PRIMARY KEY  (`factionID`,`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `invControlTowerResourcePurposes`
--

DROP TABLE IF EXISTS `invControlTowerResourcePurposes`;
CREATE TABLE `invControlTowerResourcePurposes` (
  `purpose` varchar(100) NOT NULL default '',
  `purposeText` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`purpose`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `invControlTowerResources`
--

DROP TABLE IF EXISTS `invControlTowerResources`;
CREATE TABLE `invControlTowerResources` (
  `controlTowerTypeID` int(11) NOT NULL default '0',
  `resourceTypeID` int(11) NOT NULL default '0',
  `purpose` int(11) default NULL,
  `quantity` int(11) default NULL,
  `minSecurityLevel` double default NULL,
  `factionID` int(11) default NULL,
  PRIMARY KEY  (`controlTowerTypeID`,`resourceTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `invGroups`
--

DROP TABLE IF EXISTS `invGroups`;
CREATE TABLE `invGroups` (
  `groupID` int(10) unsigned NOT NULL default '0',
  `categoryID` int(10) unsigned NOT NULL default '0',
  `groupName` varchar(128) NOT NULL default '',
  `description` text NOT NULL,
  `graphicID` int(11) default NULL,
  `useBasePrice` tinyint(3) unsigned NOT NULL default '0',
  `allowManufacture` tinyint(3) unsigned NOT NULL default '0',
  `allowRecycler` tinyint(3) unsigned NOT NULL default '0',
  `anchored` tinyint(3) unsigned NOT NULL default '0',
  `anchorable` tinyint(3) unsigned NOT NULL default '0',
  `fittableNonSingleton` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`groupID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `invMarketGroups`
--

DROP TABLE IF EXISTS `invMarketGroups`;
CREATE TABLE `invMarketGroups` (
  `marketGroupID` int(10) unsigned NOT NULL default '0',
  `parentGroupID` int(11) default NULL,
  `marketGroupName` varchar(100) NOT NULL default '',
  `description` text,
  `graphicID` int(11) default NULL,
  `hasTypes` int(11) default NULL,
  PRIMARY KEY  (`marketGroupID`),
  KEY `parentGroupID` (`parentGroupID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `invMetaGroups`
--

DROP TABLE IF EXISTS `invMetaGroups`;
CREATE TABLE `invMetaGroups` (
  `metaGroupID` int(10) unsigned NOT NULL default '0',
  `metaGroupName` varchar(100) NOT NULL default '',
  `description` text,
  `graphicID` int(11) default NULL,
  PRIMARY KEY  (`metaGroupID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `invMetaTypes`
--

DROP TABLE IF EXISTS `invMetaTypes`;
CREATE TABLE `invMetaTypes` (
  `typeID` int(11) NOT NULL default '0',
  `parentTypeID` int(11) default NULL,
  `metaGroupID` int(11) default NULL,
  PRIMARY KEY  (`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `invOreInfo`
--

DROP TABLE IF EXISTS `invOreInfo`;
CREATE TABLE `invOreInfo` (
  `typeID` int(10) unsigned NOT NULL default '0',
  `yieldBonus` int(10) unsigned NOT NULL default '0',
  `minToRefine` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`typeID`),
  CONSTRAINT `invOreInfo_ibfk_1` FOREIGN KEY (`typeID`) REFERENCES `invTypes` (`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `invOreReprocessing`
--

DROP TABLE IF EXISTS `invOreReprocessing`;
CREATE TABLE `invOreReprocessing` (
  `oreTypeID` int(11) default NULL,
  `mineralTypeID` int(11) default NULL,
  `amountPerBatch` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `invOreYield`
--

DROP TABLE IF EXISTS `invOreYield`;
CREATE TABLE `invOreYield` (
  `typeID` int(10) unsigned NOT NULL default '0',
  `yieldTypeID` int(10) unsigned NOT NULL default '0',
  `yieldPercent` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`typeID`,`yieldTypeID`),
  KEY `yieldTypeID` (`yieldTypeID`),
  CONSTRAINT `invOreYield_ibfk_1` FOREIGN KEY (`typeID`) REFERENCES `invOreInfo` (`typeID`),
  CONSTRAINT `invOreYield_ibfk_2` FOREIGN KEY (`yieldTypeID`) REFERENCES `invTypes` (`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `invReprocessing`
--

DROP TABLE IF EXISTS `invReprocessing`;
CREATE TABLE `invReprocessing` (
  `itemTypeID` int(11) default NULL,
  `mineralTypeID` int(11) default NULL,
  `amountPerBatch` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `invShipTypes`
--

DROP TABLE IF EXISTS `invShipTypes`;
CREATE TABLE `invShipTypes` (
  `shipTypeID` int(10) unsigned NOT NULL default '0',
  `powerCoreTypeID` int(10) unsigned NOT NULL default '0',
  `capacitorTypeID` int(10) unsigned NOT NULL default '0',
  `shieldTypeID` int(10) unsigned NOT NULL default '0',
  `propulsionTypeID` int(10) unsigned NOT NULL default '0',
  `sensorTypeID` int(10) unsigned NOT NULL default '0',
  `armorTypeID` int(10) unsigned NOT NULL default '0',
  `computerTypeID` int(10) unsigned NOT NULL default '0',
  `weaponTypeID` int(10) unsigned default NULL,
  `miningTypeID` int(10) unsigned default NULL,
  `skillTypeID` int(10) unsigned default NULL,
  PRIMARY KEY  (`shipTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `invTypeReactions`
--

DROP TABLE IF EXISTS `invTypeReactions`;
CREATE TABLE `invTypeReactions` (
  `reactionTypeID` int(10) unsigned NOT NULL default '0',
  `input` int(10) unsigned NOT NULL default '0',
  `typeID` int(10) unsigned NOT NULL default '0',
  `quantity` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`reactionTypeID`,`typeID`,`input`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `invTypes`
--

DROP TABLE IF EXISTS `invTypes`;
CREATE TABLE `invTypes` (
  `typeID` int(10) unsigned NOT NULL default '0',
  `groupID` int(10) unsigned NOT NULL default '0',
  `typeName` varchar(100) NOT NULL default '',
  `description` text NOT NULL,
  `graphicID` int(11) default NULL,
  `radius` double NOT NULL default '0',
  `mass` double NOT NULL default '0',
  `volume` double NOT NULL default '0',
  `capacity` double NOT NULL default '0',
  `portionSize` int(10) unsigned NOT NULL default '0',
  `raceID` int(11) default NULL,
  `basePrice` double NOT NULL default '0',
  `published` tinyint(3) unsigned NOT NULL default '0',
  `marketGroupID` int(10) unsigned NOT NULL default '0',
  `chanceOfDuplicating` double NOT NULL default '0',
  PRIMARY KEY  (`typeID`),
  KEY `marketGroupID` (`marketGroupID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `languages`
--

DROP TABLE IF EXISTS `languages`;
CREATE TABLE `languages` (
  `languageID` varchar(2) NOT NULL default '',
  `languageName` varchar(100) NOT NULL default '',
  `translatedLanguageName` varchar(22) character set utf8 collate utf8_bin NOT NULL default '',
  PRIMARY KEY  (`languageID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `mapCelestialStatistics`
--

DROP TABLE IF EXISTS `mapCelestialStatistics`;
CREATE TABLE `mapCelestialStatistics` (
  `celestialID` int(11) NOT NULL default '0',
  `temperature` double NOT NULL default '0',
  `spectralClass` char(10) NOT NULL default '',
  `luminosity` double NOT NULL default '0',
  `age` double NOT NULL default '0',
  `life` double NOT NULL default '0',
  `orbitRadius` double NOT NULL default '0',
  `eccentricity` double NOT NULL default '0',
  `massDust` double NOT NULL default '0',
  `massGas` double NOT NULL default '0',
  `fragmented` int(11) NOT NULL default '0',
  `density` double NOT NULL default '0',
  `surfaceGravity` double NOT NULL default '0',
  `escapeVelocity` double NOT NULL default '0',
  `orbitPeriod` double NOT NULL default '0',
  `rotationRate` double NOT NULL default '0',
  `locked` int(11) NOT NULL default '0',
  `pressure` double NOT NULL default '0',
  `radius` double NOT NULL default '0',
  `mass` double NOT NULL default '0',
  PRIMARY KEY  (`celestialID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `mapConstellations`
--

DROP TABLE IF EXISTS `mapConstellations`;
CREATE TABLE `mapConstellations` (
  `regionID` int(11) default NULL,
  `constellationID` int(10) unsigned NOT NULL default '0',
  `constellationName` varchar(100) NOT NULL default '',
  `x` double default NULL,
  `y` double default NULL,
  `z` double default NULL,
  `xMin` double default NULL,
  `xMax` double default NULL,
  `yMin` double default NULL,
  `yMax` double default NULL,
  `zMin` double default NULL,
  `zMax` double default NULL,
  `factionID` int(11) default NULL,
  `radius` double default NULL,
  PRIMARY KEY  (`constellationID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `mapDenormalize`
--

DROP TABLE IF EXISTS `mapDenormalize`;
CREATE TABLE `mapDenormalize` (
  `itemID` int(11) NOT NULL default '0',
  `typeID` int(11) NOT NULL default '0',
  `groupID` int(11) NOT NULL default '0',
  `solarSystemID` int(11) default NULL,
  `constellationID` int(11) default NULL,
  `regionID` int(11) default NULL,
  `orbitID` int(11) default NULL,
  `x` double NOT NULL default '0',
  `y` double NOT NULL default '0',
  `z` double NOT NULL default '0',
  `radius` double default NULL,
  `itemName` varchar(100) NOT NULL default '',
  `security` double default NULL,
  PRIMARY KEY  (`itemID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `mapJumps`
--

DROP TABLE IF EXISTS `mapJumps`;
CREATE TABLE `mapJumps` (
  `stargateID` int(11) NOT NULL default '0',
  `celestialID` int(11) default NULL,
  PRIMARY KEY  (`stargateID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `mapRegions`
--

DROP TABLE IF EXISTS `mapRegions`;
CREATE TABLE `mapRegions` (
  `regionID` int(10) unsigned NOT NULL default '0',
  `regionName` varchar(100) NOT NULL default '',
  `x` double default NULL,
  `y` double default NULL,
  `z` double default NULL,
  `xMin` double default NULL,
  `xMax` double default NULL,
  `yMin` double default NULL,
  `yMax` double default NULL,
  `zMin` double default NULL,
  `zMax` double default NULL,
  `factionID` int(11) default NULL,
  `radius` double default NULL,
  PRIMARY KEY  (`regionID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `mapSolarSystemJumps`
--

DROP TABLE IF EXISTS `mapSolarSystemJumps`;
CREATE TABLE `mapSolarSystemJumps` (
  `fromRegionID` int(11) default NULL,
  `fromConstellationID` int(11) default NULL,
  `fromSolarSystemID` int(11) NOT NULL default '0',
  `toSolarSystemID` int(11) NOT NULL default '0',
  `toConstellationID` int(11) default NULL,
  `toRegionID` int(11) default NULL,
  PRIMARY KEY  (`fromSolarSystemID`,`toSolarSystemID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `mapSolarSystemMinerals`
--

DROP TABLE IF EXISTS `mapSolarSystemMinerals`;
CREATE TABLE `mapSolarSystemMinerals` (
  `securityClass` char(2) NOT NULL default '',
  `mineralGroupID` int(11) NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `mapSolarSystems`
--

DROP TABLE IF EXISTS `mapSolarSystems`;
CREATE TABLE `mapSolarSystems` (
  `regionID` int(11) default NULL,
  `constellationID` int(11) default NULL,
  `solarsystemID` int(10) unsigned NOT NULL default '0',
  `solarSystemName` varchar(100) NOT NULL default '',
  `x` double default NULL,
  `y` double default NULL,
  `z` double default NULL,
  `xMin` double default NULL,
  `xMax` double default NULL,
  `yMin` double default NULL,
  `yMax` double default NULL,
  `zMin` double default NULL,
  `zMax` double default NULL,
  `luminosity` double default NULL,
  `border` int(11) default NULL,
  `fringe` int(11) default NULL,
  `corridor` int(11) default NULL,
  `hub` int(11) default NULL,
  `international` int(11) default NULL,
  `regional` int(11) default NULL,
  `constellation` int(11) default NULL,
  `security` double default NULL,
  `factionID` int(11) default NULL,
  `radius` double default NULL,
  `sunTypeID` int(11) default NULL,
  `securityClass` varchar(2) NOT NULL default '',
  `allianceID` int(11) default NULL,
  PRIMARY KEY  (`solarsystemID`),
  KEY `regionID` (`regionID`),
  KEY `constellationID` (`constellationID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `market_history_old`
--

DROP TABLE IF EXISTS `market_history_old`;
CREATE TABLE `market_history_old` (
  `regionID` int(10) unsigned NOT NULL default '0',
  `typeID` int(10) unsigned NOT NULL default '0',
  `historyDate` int(10) unsigned NOT NULL default '0',
  `volume` int(10) unsigned NOT NULL default '0',
  `orders` int(10) unsigned NOT NULL default '0',
  `lowPrice` double NOT NULL default '0',
  `highPrice` double NOT NULL default '0',
  `avgPrice` double NOT NULL default '0',
  PRIMARY KEY  (`regionID`,`typeID`,`historyDate`),
  KEY `regionID` (`regionID`,`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `market_journal`
--

DROP TABLE IF EXISTS `market_journal`;
CREATE TABLE `market_journal` (
  `characterID` int(10) unsigned NOT NULL default '0',
  `refID` int(10) unsigned NOT NULL auto_increment,
  `transDate` bigint(20) default NULL,
  `refTypeID` int(10) unsigned NOT NULL default '0',
  `ownerID1` int(10) unsigned NOT NULL default '0',
  `ownerID2` int(10) unsigned NOT NULL default '0',
  `argID1` varchar(255) default NULL,
  `accountID` int(10) unsigned NOT NULL default '0',
  `amount` double NOT NULL default '0',
  `balance` double NOT NULL default '0',
  `reason` varchar(43) default NULL,
  `accountKey` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`refID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `market_keyMap`
--

DROP TABLE IF EXISTS `market_keyMap`;
CREATE TABLE `market_keyMap` (
  `accountKey` int(10) unsigned NOT NULL default '0',
  `accountType` varchar(100) NOT NULL default '',
  `accountName` varchar(100) NOT NULL default '',
  `description` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`accountKey`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `market_orders`
--

DROP TABLE IF EXISTS `market_orders`;
CREATE TABLE `market_orders` (
  `orderID` int(10) unsigned NOT NULL auto_increment,
  `typeID` int(10) unsigned NOT NULL default '0',
  `charID` int(10) unsigned NOT NULL default '0',
  `regionID` int(10) unsigned NOT NULL default '0',
  `stationID` int(10) unsigned NOT NULL default '0',
  `range` int(10) unsigned NOT NULL default '0',
  `bid` tinyint(3) unsigned NOT NULL default '0',
  `price` double NOT NULL default '0',
  `volEntered` int(10) unsigned NOT NULL default '0',
  `volRemaining` int(10) unsigned NOT NULL default '0',
  `issued` bigint(20) unsigned NOT NULL default '0',
  `orderState` int(10) unsigned NOT NULL default '0',
  `minVolume` int(10) unsigned NOT NULL default '0',
  `contraband` tinyint(3) unsigned NOT NULL default '0',
  `accountID` int(10) unsigned NOT NULL default '0',
  `duration` int(10) unsigned NOT NULL default '0',
  `isCorp` tinyint(3) unsigned NOT NULL default '0',
  `solarSystemID` int(11) NOT NULL default '0',
  `escrow` tinyint(3) unsigned NOT NULL default '0',
  `jumps` tinyint(4) NOT NULL default '1',
  PRIMARY KEY  (`orderID`),
  KEY `typeID` (`typeID`),
  KEY `regionID` (`regionID`),
  KEY `stationID` (`stationID`),
  KEY `solarSystemID` (`solarSystemID`),
  KEY `regionID_2` (`regionID`),
  KEY `typeID_2` (`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `market_refTypes`
--

DROP TABLE IF EXISTS `market_refTypes`;
CREATE TABLE `market_refTypes` (
  `refTypeID` int(10) unsigned NOT NULL default '0',
  `refTypeText` varchar(100) NOT NULL default '',
  `description` mediumtext NOT NULL,
  PRIMARY KEY  (`refTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `market_transactions`
--

DROP TABLE IF EXISTS `market_transactions`;
CREATE TABLE `market_transactions` (
  `transactionID` int(10) unsigned NOT NULL auto_increment,
  `transactionDateTime` bigint(20) unsigned NOT NULL default '0',
  `typeID` int(10) unsigned NOT NULL default '0',
  `quantity` int(10) unsigned NOT NULL default '0',
  `price` double NOT NULL default '0',
  `transactionType` int(10) unsigned NOT NULL default '0',
  `clientID` int(10) unsigned NOT NULL default '0',
  `regionID` int(10) unsigned NOT NULL default '0',
  `stationID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`transactionID`),
  KEY `regionID` (`regionID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `npcStandings`
--

DROP TABLE IF EXISTS `npcStandings`;
CREATE TABLE `npcStandings` (
  `fromID` int(10) unsigned NOT NULL default '0',
  `toID` int(10) unsigned NOT NULL default '0',
  `standing` double NOT NULL default '0',
  PRIMARY KEY  (`fromID`,`toID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `ramALTypeCategory`
--

DROP TABLE IF EXISTS `ramALTypeCategory`;
CREATE TABLE `ramALTypeCategory` (
  `assemblyLineTypeID` int(10) unsigned NOT NULL default '0',
  `activityID` int(10) unsigned NOT NULL default '0',
  `categoryID` int(10) unsigned NOT NULL default '0',
  `timeMultiplier` double NOT NULL default '0',
  `materialMultiplier` double NOT NULL default '0',
  PRIMARY KEY  (`assemblyLineTypeID`,`categoryID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `ramALTypeGroup`
--

DROP TABLE IF EXISTS `ramALTypeGroup`;
CREATE TABLE `ramALTypeGroup` (
  `assemblyLineTypeID` int(10) unsigned NOT NULL default '0',
  `activityID` int(10) unsigned NOT NULL default '0',
  `groupID` int(10) unsigned NOT NULL default '0',
  `timeMultiplier` double NOT NULL default '0',
  `materialMultiplier` double NOT NULL default '0',
  PRIMARY KEY  (`assemblyLineTypeID`,`groupID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `ramALTypes`
--

DROP TABLE IF EXISTS `ramALTypes`;
CREATE TABLE `ramALTypes` (
  `assemblyLineTypeID` int(10) unsigned NOT NULL default '0',
  `assemblyLineTypeName` varchar(100) NOT NULL default '',
  `typeName` varchar(100) NOT NULL default '',
  `description` varchar(100) NOT NULL default '',
  `activityID` int(10) unsigned NOT NULL default '0',
  `baseTimeMultiplier` double NOT NULL default '0',
  `baseMaterialMultiplier` double NOT NULL default '0',
  `volume` double NOT NULL default '0',
  PRIMARY KEY  (`assemblyLineTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `ramActivities`
--

DROP TABLE IF EXISTS `ramActivities`;
CREATE TABLE `ramActivities` (
  `activityID` int(10) unsigned NOT NULL default '0',
  `activityName` varchar(100) NOT NULL default '',
  `iconNo` varchar(200) default NULL,
  `description` mediumtext NOT NULL,
  `published` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`activityID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `ramAssemblyLineStationCostLogs`
--

DROP TABLE IF EXISTS `ramAssemblyLineStationCostLogs`;
CREATE TABLE `ramAssemblyLineStationCostLogs` (
  `assemblyLineTypeID` int(11) NOT NULL default '0',
  `stationID` int(11) NOT NULL default '0',
  `logDateTime` char(20) NOT NULL default '',
  `_usage` double NOT NULL default '0',
  `costPerHour` float NOT NULL default '0',
  PRIMARY KEY  (`assemblyLineTypeID`),
  KEY `stationID` (`stationID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `ramAssemblyLineStations`
--

DROP TABLE IF EXISTS `ramAssemblyLineStations`;
CREATE TABLE `ramAssemblyLineStations` (
  `stationID` int(11) NOT NULL default '0',
  `assemblyLineTypeID` int(11) NOT NULL default '0',
  `quantity` int(11) default NULL,
  `stationTypeID` int(11) default NULL,
  `ownerID` int(11) default NULL,
  `solarSystemID` int(11) default NULL,
  `regionID` int(11) default NULL,
  PRIMARY KEY  (`stationID`,`assemblyLineTypeID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `ramAssemblyLineTypeDetailPerCategory`
--

DROP TABLE IF EXISTS `ramAssemblyLineTypeDetailPerCategory`;
CREATE TABLE `ramAssemblyLineTypeDetailPerCategory` (
  `assemblyLineTypeID` int(11) NOT NULL default '0',
  `categoryID` int(11) NOT NULL default '0',
  `timeMultiplier` double default NULL,
  `materialMultiplier` double default NULL,
  PRIMARY KEY  (`assemblyLineTypeID`,`categoryID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `ramAssemblyLineTypeDetailPerGroup`
--

DROP TABLE IF EXISTS `ramAssemblyLineTypeDetailPerGroup`;
CREATE TABLE `ramAssemblyLineTypeDetailPerGroup` (
  `assemblyLineTypeID` int(11) NOT NULL default '0',
  `groupID` int(11) NOT NULL default '0',
  `timeMultiplier` double default NULL,
  `materialMultiplier` double default NULL,
  PRIMARY KEY  (`assemblyLineTypeID`,`groupID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `ramAssemblyLineTypes`
--

DROP TABLE IF EXISTS `ramAssemblyLineTypes`;
CREATE TABLE `ramAssemblyLineTypes` (
  `assemblyLineTypeID` int(11) NOT NULL default '0',
  `assemblyLineTypeName` varchar(100) default NULL,
  `description` text,
  `baseTimeMultiplier` double default NULL,
  `baseMaterialMultiplier` double default NULL,
  `volume` double default NULL,
  `activityID` int(11) default NULL,
  `minCostPerHour` double default NULL,
  PRIMARY KEY  (`assemblyLineTypeID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `ramAssemblyLines`
--

DROP TABLE IF EXISTS `ramAssemblyLines`;
CREATE TABLE `ramAssemblyLines` (
  `assemblyLineID` int(11) NOT NULL default '0',
  `assemblyLineTypeID` int(11) default NULL,
  `containerID` int(11) default NULL,
  `nextFreeTime` datetime default NULL,
  `UIGroupingID` int(11) default NULL,
  `costInstall` double default NULL,
  `costPerHour` double default NULL,
  `restrictionMask` int(11) default NULL,
  `discountPerGoodStandingPoint` double default NULL,
  `surchargePerBadStandingPoint` double default NULL,
  `minimumStanding` double default NULL,
  `minimumCharSecurity` double default NULL,
  `minimumCorpSecurity` double default NULL,
  `maximumCharSecurity` double default NULL,
  `maximumCorpSecurity` double default NULL,
  `ownerID` int(11) default NULL,
  `oldContainerID` int(11) default NULL,
  `oldOwnerID` int(11) default NULL,
  `activityID` int(11) default NULL,
  PRIMARY KEY  (`assemblyLineID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `ramCompletedStatuses`
--

DROP TABLE IF EXISTS `ramCompletedStatuses`;
CREATE TABLE `ramCompletedStatuses` (
  `completedStatusID` int(10) unsigned NOT NULL default '0',
  `completedStatusName` varchar(100) NOT NULL default '',
  `completedStatusText` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`completedStatusID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `ramInstallationTypeDefaultContents`
--

DROP TABLE IF EXISTS `ramInstallationTypeDefaultContents`;
CREATE TABLE `ramInstallationTypeDefaultContents` (
  `installationTypeID` int(11) NOT NULL default '0',
  `assemblyLineTypeID` int(11) NOT NULL default '0',
  `UIGroupingID` int(11) NOT NULL default '0',
  `quantity` int(11) NOT NULL default '0',
  `costInstall` float NOT NULL default '0',
  `costPerHour` float NOT NULL default '0',
  `restrictionMask` int(11) NOT NULL default '0',
  `discountPerGoodStandingPoint` float NOT NULL default '0',
  `surchargePerBadStandingPoint` float NOT NULL default '0',
  `minimumStanding` float NOT NULL default '0',
  `minimumCharSecurity` float NOT NULL default '0',
  `minimumCorpSecurity` float NOT NULL default '0',
  `maximumCharSecurity` float NOT NULL default '0',
  `maximumCorpSecurity` float NOT NULL default '0',
  PRIMARY KEY  (`installationTypeID`),
  KEY `assemblyLineTypeID` (`assemblyLineTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `ramJobs`
--

DROP TABLE IF EXISTS `ramJobs`;
CREATE TABLE `ramJobs` (
  `jobID` bigint(20) unsigned NOT NULL auto_increment,
  `ownerID` int(10) unsigned NOT NULL default '0',
  `assemblyLineID` bigint(20) unsigned NOT NULL default '0',
  `installedItemID` int(10) unsigned NOT NULL default '0',
  `installTime` bigint(20) unsigned NOT NULL default '0',
  `beginProductionTime` bigint(20) unsigned NOT NULL default '0',
  `endProductionTime` bigint(20) unsigned NOT NULL default '0',
  `description` varchar(250) default 'blah',
  PRIMARY KEY  (`jobID`),
  KEY `FK_RAMJOBS_ASSEMBLYLINES` (`assemblyLineID`),
  CONSTRAINT `FK_RAMJOBS_ASSEMBLYLINES` FOREIGN KEY (`assemblyLineID`) REFERENCES `ramAssemblyLines` (`assemblyLineID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Jobs for Assembly Lines';

--
-- Table structure for table `rentalInfo`
--

DROP TABLE IF EXISTS `rentalInfo`;
CREATE TABLE `rentalInfo` (
  `stationID` int(10) unsigned NOT NULL default '0',
  `slotNumber` int(10) unsigned NOT NULL default '0',
  `renterID` int(10) unsigned NOT NULL default '0',
  `typeID` int(10) unsigned NOT NULL default '0',
  `rentPeriodInDays` int(10) unsigned NOT NULL default '0',
  `periodCost` double NOT NULL default '0',
  `billID` int(10) unsigned NOT NULL default '0',
  `balanceDueDate` int(10) unsigned NOT NULL default '0',
  `discontinue` tinyint(3) unsigned NOT NULL default '0',
  `publiclyAvailable` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`stationID`,`slotNumber`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `spawnBounds`
--

DROP TABLE IF EXISTS `spawnBounds`;
CREATE TABLE `spawnBounds` (
  `spawnID` int(10) unsigned NOT NULL default '0',
  `pointIndex` tinyint(3) unsigned NOT NULL default '0',
  `x` double NOT NULL default '0',
  `y` double NOT NULL default '0',
  `z` double NOT NULL default '0',
  PRIMARY KEY  (`spawnID`,`pointIndex`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `spawnGroupEntries`
--

DROP TABLE IF EXISTS `spawnGroupEntries`;
CREATE TABLE `spawnGroupEntries` (
  `spawnGroupID` int(10) unsigned NOT NULL default '0',
  `npcTypeID` int(10) unsigned NOT NULL default '0',
  `quantity` tinyint(3) unsigned NOT NULL default '1',
  `probability` float NOT NULL default '1',
  `ownerID` int(10) unsigned NOT NULL default '0',
  `corporationID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`spawnGroupID`,`npcTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `spawnGroups`
--

DROP TABLE IF EXISTS `spawnGroups`;
CREATE TABLE `spawnGroups` (
  `spawnGroupID` int(10) unsigned NOT NULL auto_increment,
  `spawnGroupName` varchar(85) NOT NULL default '',
  `formation` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`spawnGroupID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `spawns`
--

DROP TABLE IF EXISTS `spawns`;
CREATE TABLE `spawns` (
  `spawnID` int(10) unsigned NOT NULL auto_increment,
  `solarSystemID` int(10) unsigned NOT NULL default '0',
  `spawnGroupID` int(10) unsigned NOT NULL default '0',
  `spawnBoundType` int(10) unsigned NOT NULL default '0',
  `spawnTimer` int(10) unsigned NOT NULL default '0',
  `respawnTimeMin` int(10) unsigned NOT NULL default '0',
  `respawnTimeMax` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`spawnID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `staOperationServices`
--

DROP TABLE IF EXISTS `staOperationServices`;
CREATE TABLE `staOperationServices` (
  `operationID` int(10) unsigned NOT NULL default '0',
  `serviceID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`operationID`,`serviceID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `staOperations`
--

DROP TABLE IF EXISTS `staOperations`;
CREATE TABLE `staOperations` (
  `activityID` int(10) unsigned NOT NULL default '0',
  `operationID` int(10) unsigned NOT NULL default '0',
  `operationName` varchar(100) NOT NULL default '',
  `description` mediumtext NOT NULL,
  `fringe` int(10) unsigned NOT NULL default '0',
  `corridor` int(10) unsigned NOT NULL default '0',
  `hub` int(10) unsigned NOT NULL default '0',
  `border` int(10) unsigned NOT NULL default '0',
  `ratio` int(10) unsigned NOT NULL default '0',
  `caldariStationTypeID` int(10) unsigned default NULL,
  `minmatarStationTypeID` int(10) unsigned default NULL,
  `amarrStationTypeID` int(10) unsigned default NULL,
  `gallenteStationTypeID` int(10) unsigned default NULL,
  `joveStationTypeID` int(10) unsigned default NULL,
  PRIMARY KEY  (`operationID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `staServices`
--

DROP TABLE IF EXISTS `staServices`;
CREATE TABLE `staServices` (
  `serviceID` int(10) unsigned NOT NULL default '0',
  `serviceName` varchar(100) NOT NULL default '',
  `description` mediumtext NOT NULL,
  PRIMARY KEY  (`serviceID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `staStationTypes`
--

DROP TABLE IF EXISTS `staStationTypes`;
CREATE TABLE `staStationTypes` (
  `stationTypeID` int(10) unsigned NOT NULL default '0',
  `dockingBayGraphicID` int(10) unsigned default NULL,
  `hangarGraphicID` int(10) unsigned default NULL,
  `dockEntryX` double NOT NULL default '0',
  `dockEntryY` double NOT NULL default '0',
  `dockEntryZ` double NOT NULL default '0',
  `dockOrientationX` double NOT NULL default '0',
  `dockOrientationY` double NOT NULL default '0',
  `dockOrientationZ` double NOT NULL default '0',
  `operationID` int(10) unsigned default NULL,
  `officeSlots` int(10) unsigned default NULL,
  `reprocessingEfficiency` int(10) unsigned default NULL,
  `conquerable` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`stationTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `staStations`
--

DROP TABLE IF EXISTS `staStations`;
CREATE TABLE `staStations` (
  `stationID` int(10) unsigned NOT NULL default '0',
  `security` int(10) unsigned NOT NULL default '0',
  `dockingCostPerVolume` double NOT NULL default '0',
  `maxShipVolumeDockable` int(10) unsigned NOT NULL default '0',
  `officeRentalCost` int(10) unsigned NOT NULL default '0',
  `operationID` int(10) unsigned default NULL,
  `stationTypeID` int(10) unsigned NOT NULL default '0',
  `corporationID` int(10) unsigned NOT NULL default '0',
  `solarSystemID` int(10) unsigned NOT NULL default '0',
  `constellationID` int(10) unsigned NOT NULL default '0',
  `regionID` int(10) unsigned NOT NULL default '0',
  `stationName` varchar(85) NOT NULL default '',
  `x` double NOT NULL default '0',
  `y` double NOT NULL default '0',
  `z` double NOT NULL default '0',
  `reprocessingEfficiency` double NOT NULL default '0',
  `reprocessingStationsTake` double NOT NULL default '0',
  `reprocessingHangarFlag` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`stationID`),
  KEY `corporationID` (`corporationID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `tutorial_criteria`
--

DROP TABLE IF EXISTS `tutorial_criteria`;
CREATE TABLE `tutorial_criteria` (
  `criteriaID` int(10) unsigned NOT NULL default '0',
  `criteriaName` varchar(100) NOT NULL default '',
  `messageText` mediumtext NOT NULL,
  `audioPath` varchar(200) default NULL,
  PRIMARY KEY  (`criteriaID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

--
-- Table structure for table `tutorial_page_criteria`
--

DROP TABLE IF EXISTS `tutorial_page_criteria`;
CREATE TABLE `tutorial_page_criteria` (
  `pageID` int(10) unsigned NOT NULL default '0',
  `criteriaID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`pageID`,`criteriaID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

--
-- Table structure for table `tutorial_pages`
--

DROP TABLE IF EXISTS `tutorial_pages`;
CREATE TABLE `tutorial_pages` (
  `pageID` int(10) unsigned NOT NULL default '0',
  `pageNumber` int(10) unsigned NOT NULL default '0',
  `pageName` varchar(100) NOT NULL default '',
  `text` mediumtext NOT NULL,
  `imagePath` varchar(200) default NULL,
  `audioPath` varchar(200) NOT NULL default '',
  `tutorialID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`pageID`),
  UNIQUE KEY `tutorialID` (`tutorialID`,`pageNumber`),
  CONSTRAINT `tutorial_pages_ibfk_1` FOREIGN KEY (`tutorialID`) REFERENCES `tutorials` (`tutorialID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `tutorials`
--

DROP TABLE IF EXISTS `tutorials`;
CREATE TABLE `tutorials` (
  `tutorialID` int(10) unsigned NOT NULL default '0',
  `tutorialVName` varchar(100) NOT NULL default '',
  `nextTutorialID` int(10) unsigned default NULL,
  PRIMARY KEY  (`tutorialID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

--
-- Table structure for table `tutorials_criterias`
--

DROP TABLE IF EXISTS `tutorials_criterias`;
CREATE TABLE `tutorials_criterias` (
  `tutorialID` int(10) unsigned NOT NULL default '0',
  `criteriaID` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`tutorialID`,`criteriaID`),
  KEY `criteriaID` (`criteriaID`),
  CONSTRAINT `tutorials_criterias_ibfk_2` FOREIGN KEY (`criteriaID`) REFERENCES `tutorial_criteria` (`criteriaID`),
  CONSTRAINT `tutorials_criterias_ibfk_1` FOREIGN KEY (`tutorialID`) REFERENCES `tutorials` (`tutorialID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

