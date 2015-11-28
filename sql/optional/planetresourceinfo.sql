SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

CREATE TABLE IF NOT EXISTS `planetresourceinfo` (
  `planetID` int(11) NOT NULL,
  `itemID1` int(11) NOT NULL,
  `itemID2` int(11) NOT NULL,
  `itemID3` int(11) NOT NULL,
  `itemID4` int(11) NOT NULL,
  `itemID5` int(11) NOT NULL,
  `quality1` float NOT NULL,
  `quality2` float NOT NULL,
  `quality3` float NOT NULL,
  `quality4` float NOT NULL,
  `quality5` float NOT NULL,
  `data1` varchar(255) NOT NULL,
  `data2` varchar(255) NOT NULL,
  `data3` varchar(255) NOT NULL,
  `data4` varchar(255) NOT NULL,
  `data5` varchar(255) NOT NULL,
  `numBands1` int(11) NOT NULL,
  `numBands2` int(11) NOT NULL,
  `numBands3` int(11) NOT NULL,
  `numBands4` int(11) NOT NULL,
  `numBands5` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `planetresourceinfo`
  ADD UNIQUE KEY `planetID` (`planetID`);

INSERT INTO `planetresourceinfo` SELECT `itemID` AS `planetID`, 2073 AS `itemID1`, 2268 AS `itemID2`, 2287 AS `itemID3`, 2288 AS `itemID4`, 2305 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapdenormalize` WHERE `typeID` = 11 AND `groupID` = 7;
INSERT INTO `planetresourceinfo` SELECT `itemID` AS `planetID`, 2268 AS `itemID1`, 2272 AS `itemID2`, 2073 AS `itemID3`, 2286 AS `itemID4`, 2310 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapdenormalize` WHERE `typeID` = 12 AND `groupID` = 7;
INSERT INTO `planetresourceinfo` SELECT `itemID` AS `planetID`, 2268 AS `itemID1`, 2267 AS `itemID2`, 2309 AS `itemID3`, 2310 AS `itemID4`, 2311 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapdenormalize` WHERE `typeID` = 13 AND `groupID` = 7;
INSERT INTO `planetresourceinfo` SELECT `itemID` AS `planetID`, 2268 AS `itemID1`, 2287 AS `itemID2`, 2073 AS `itemID3`, 2286 AS `itemID4`, 2288 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapdenormalize` WHERE `typeID` = 2014 AND `groupID` = 7;
INSERT INTO `planetresourceinfo` SELECT `itemID` AS `planetID`, 2267 AS `itemID1`, 2272 AS `itemID2`, 2308 AS `itemID3`, 2307 AS `itemID4`, 2306 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapdenormalize` WHERE `typeID` = 2015 AND `groupID` = 7;
INSERT INTO `planetresourceinfo` SELECT `itemID` AS `planetID`, 2268 AS `itemID1`, 2267 AS `itemID2`, 2073 AS `itemID3`, 2270 AS `itemID4`, 2288 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapdenormalize` WHERE `typeID` = 2016 AND `groupID` = 7;
INSERT INTO `planetresourceinfo` SELECT `itemID` AS `planetID`, 2268 AS `itemID1`, 2267 AS `itemID2`, 2308 AS `itemID3`, 2309 AS `itemID4`, 2310 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapdenormalize` WHERE `typeID` = 2017 AND `groupID` = 7;
INSERT INTO `planetresourceinfo` SELECT `itemID` AS `planetID`, 2267 AS `itemID1`, 2272 AS `itemID2`, 2270 AS `itemID3`, 2308 AS `itemID4`, 2306 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapdenormalize` WHERE `typeID` = 2063 AND `groupID` = 7;
