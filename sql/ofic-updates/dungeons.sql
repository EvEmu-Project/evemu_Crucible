-- ----------------------------
-- Table structure for `dungeonType`
-- ----------------------------
DROP TABLE IF EXISTS `dungeonType`;
CREATE TABLE `dungeonType` (
  `dungeonTypeID` INT(11) NOT NULL,
  `dungeonType` TEXT CHARACTER SET latin1,
  PRIMARY KEY (`dungeonTypeID`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

INSERT INTO `dungeonType` (`dungeonTypeID`, `dungeonType`) VALUES
-- Test salvage for devs
('1', 'Mission'),  -- created when a player accepts a mission will include tutorial missions or anything to do with agents (reserves 1-1000)
('2', 'Gravimetric Site'), -- asteroid mining (1001-1200)
('3', 'Magnetometric Site'),  -- Relic: analyze or salvage (1200-1400)
('4', 'Radar Site'),  -- data: hacking (1401-1600)
('5', 'Ladar Site'), -- gas mining (1601-1800)
('6', 'Wormholes'), -- only should spawn a wormhole (1801-1900) groupid 988
('7', 'Cosmic Anomaly'), -- easier to find mostly combat sites 2000
('8', 'Unrated complexes'), -- defined as Cosmic Signatures combat sites, need to be scanned down
('9', 'Escalation'), -- When created creates a new entry in the players expedition missions journal tab. 
('10', 'DED Complex'); -- For DED complexes

-- ----------------------------
-- Table structure for `dungeons`
-- ----------------------------
DROP TABLE IF EXISTS `dungeons`;
CREATE TABLE `dungeons` (
  `dungeonID` INT(11) NOT NULL,
  `dungeonName` TEXT CHARACTER SET latin1,
  `dungeonTypeID` INT(11) NOT NULL,
  `chance` DOUBLE NULL,
  `baseduration` INT(11) NOT NULL,
  `minsec` INT(11) NOT NULL,
  `maxsec` INT(11) NOT NULL,
  `galaxylimit` INT(11) NOT NULL,
  `regionallimit` INT(11) NOT NULL,
  `constellationlimit` INT(11) NOT NULL,
  `systemlimit` INT(11) NOT NULL,
  PRIMARY KEY (`dungeonID`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

INSERT INTO `dungeons` (`dungeonID`, `dungeonName`, `dungeonTypeID`, `chance`, `baseduration`, `minsec`, `maxsec`, `galaxylimit`, `regionallimit`, `constellationlimit`, `systemlimit`) VALUES
('1801', 'Test wormhole', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1802', 'Wormhole Z971', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1803', 'Wormhole R943', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1804', 'Wormhole X702', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1805', 'Wormhole O128', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1806', 'Wormhole N432', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1807', 'Wormhole M555', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1808', 'Wormhole B041', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1809', 'Wormhole U319', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1810', 'Wormhole B449', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1811', 'Wormhole N944', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1812', 'Wormhole S199', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1813', 'Wormhole A641', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1814', 'Wormhole R051', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1815', 'Wormhole V283', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1816', 'Wormhole H121', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1817', 'Wormhole C125', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1818', 'Wormhole O883', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1819', 'Wormhole M609', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1820', 'Wormhole L614', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1821', 'Wormhole S804', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1822', 'Wormhole N110', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1823', 'Wormhole J244', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1824', 'Wormhole Z060', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1825', 'Wormhole Z647', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1826', 'Wormhole D382', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1827', 'Wormhole O477', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1828', 'Wormhole Y683', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1829', 'Wormhole N062', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1830', 'Wormhole R474', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1831', 'Wormhole B274', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1832', 'Wormhole A239', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1833', 'Wormhole E545', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1834', 'Wormhole V301', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1835', 'Wormhole I182', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1836', 'Wormhole N968', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1837', 'Wormhole T405', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1838', 'Wormhole N770', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1839', 'Wormhole A982', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1840', 'Wormhole S047', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1841', 'Wormhole U210', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1842', 'Wormhole K346', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1843', 'Wormhole P060', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1844', 'Wormhole N766', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1845', 'Wormhole C247', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1846', 'Wormhole X877', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1847', 'Wormhole H900', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1848', 'Wormhole U574', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1849', 'Wormhole D845', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1850', 'Wormhole N290', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1851', 'Wormhole K329', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1852', 'Wormhole Y790', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1853', 'Wormhole D364', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1854', 'Wormhole M267', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1855', 'Wormhole E175', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1856', 'Wormhole H296', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1857', 'Wormhole V753', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1858', 'Wormhole D792', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1859', 'Wormhole C140', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1860', 'Wormhole Z142', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1861', 'Wormhole Q317', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1862', 'Wormhole G024', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1863', 'Wormhole L477', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1864', 'Wormhole Z457', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1865', 'Wormhole V911', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1866', 'Wormhole W237', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1867', 'Wormhole B520', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1868', 'Wormhole C391', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1869', 'Wormhole C248', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'), 
('1870', 'Wormhole K162', '6', '1.00', '3600', '-10.00', '10.00', '2', '1', '1', '1'); 

-- regional is a value to be later coded to limit certain dungeons to NOT spawn in certain areas of the galaxy. So WH dungeons don't spawn in Nullsec or Sansha in Caldari territory. Value is to be left as 0 for 'no limit'.
-- minsec/maxsec are limits on where some sites can/can't spawn based on system security status. -10.00 to 10.00 means it can spawn in any sec status.
-- galaxylimit refers to how many of this dungeon can be spawn galaxy-wide at any point in time (to prevent over spawning) 0 means unlimited. This number is a maximum, there doesn't HAVE to be any of this spawned.
--  constellationlimit same as above but for  constellations. I decided against doing it for regions.
-- gm commands should override this though. this isn't a server check for any instances of a specific wormhole/whatever, it's the check for any instances of the appropriate dungeon kind. there can be multiple kids of the same 'type' of dungeon. in different locations.
-- attribute 1088 is used to determine timers (baseduration) base duration can be completed early by completing the mission or maxing out the wormhole/completing the complex

-- ----------------------------
-- Table structure for `dungeonsspawned`
-- ----------------------------
DROP TABLE IF EXISTS `dungeonsspawned`;
CREATE TABLE `dungeonsspawned` (
  `dungeonspawnedID` INT(11) NOT NULL AUTO_INCREMENT,
  `dungeonID` INT(11) NOT NULL, 
  `dungeonName` TEXT CHARACTER SET latin1,
  `systemID` INT(11) NOT NULL,
  `x` INT(11) NOT NULL,
  `y` INT(11) NOT NULL,
  `z` INT(11) NOT NULL,
  `script` INT(11) NOT NULL,
  
  PRIMARY KEY (`dungeonspawnedID`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

INSERT INTO `dungeonsspawned` (`dungeonspawnedID`, `dungeonID`, `dungeonName`, `systemID`, `x`, `y`, `z`, `script`) VALUES
('1', '1801', 'Test wormhole', '30001409', '10000000', '0', '10000000', '0'); 

-- xyz values refer to point in the system.
-- script is a placeholder in the DB to refer to any particular activity the spawn may do on initial player entry (move to x,y,z RELATIVE to the loaction or attack target etc etc)
-- Possibly needs another entry for WHICH players the dungeon is available to (if applicable)

-- ----------------------------
-- Table structure for `dungeonsspawnsetup`
-- ----------------------------
DROP TABLE IF EXISTS `dungeonsspawnsetup`;
CREATE TABLE `dungeonsspawnsetup` (
  `spawn` INT(11) NOT NULL, 
  `dungeonID` INT(11) NOT NULL, 
  `dungeonName` TEXT CHARACTER SET latin1,
  `typeID` INT(11) NOT NULL,
  `typeIDName` TEXT CHARACTER SET latin1,
  `x` INT(11) NOT NULL,
  `y` INT(11) NOT NULL,
  `z` INT(11) NOT NULL,
  `script` INT(11) NOT NULL,
  
  PRIMARY KEY (`spawn`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

INSERT INTO `dungeonsspawnsetup` (`spawn`, `dungeonID`, `dungeonName`, `typeID`, `typeIDName`, `x`, `y`, `z`, `script`) VALUES
('1', '1801', 'Test wormhole', '30463', 'Test wormhole', '100', '50', '0', '0'); 

-- CODERS: upon entering the location of the dungeon for the first time (the one being entered) the DB should be called to spawn the following in this table. Just at the location being entered. not for all and only for the first time (but would reset after server restart/shutdown). 
-- script is a place-holder in the DB to refer to any particular activity the spawn may do on initial player entry (move to x,y,z RELATIVE to the location or attack target etc etc)
-- location is relative to where the player warps in (which would be 0,0,0 unless scripted elsewhere)
-- there needs to be something linking spawns to their location (dungeonspawnedID) once they have been spawned so they can be removed later. I thought this may have been an entry in entityattributes but I don't think there is a value for that..
-- 'spawn' is there because there are multiple typeids that are the same in many parts of a complex.