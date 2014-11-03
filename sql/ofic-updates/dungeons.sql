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
('2', 'Gravimetric Site'), -- asteroid mining (1001-1100)
('3', 'Magnetometric Site'),  -- Relic: analyse or salvage (1101-1400)
('4', 'Radar Site'),  -- data: hacking (1401-1700)
('5', 'Ladar Site'), -- gas mining (1701-1800)
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
-- include tutorial missions or anything to do with agents reserves values (1-1000)
-- missions 1-250 are reserved for tutorials (if ever implemented)
-- missions 251-300 are for mining
('251','Asteroid Catastrophe','1','0','0','0','0','0','0','0','0'),
('252','Bountiful Bandine','1','0','0','0','0','0','0','0','0'),
('253','Burnt Traces','1','0','0','0','0','0','0','0','0'),
('254','Mercium Experiments','1','0','0','0','0','0','0','0','0'),
('255','Starting Simple','1','0','0','0','0','0','0','0','0'),
('256','Claimjumpers','1','0','0','0','0','0','0','0','0'),
('257','Data Mining (Amarr)','1','0','0','0','0','0','0','0','0'),
('258','Down and Dirty','1','0','0','0','0','0','0','0','0'),
('259','Mercium Belt','1','0','0','0','0','0','0','0','0'),
('260','Understanding Augmene','1','0','0','0','0','0','0','0','0'),
('261','Unknown Events','1','0','0','0','0','0','0','0','0'),
('262','Better World, A','1','0','0','0','0','0','0','0','0'),
('263','Beware They Live','1','0','0','0','0','0','0','0','0'),
('264',"Coming'Round the Mountain",'1','0','0','0','0','0','0','0','0'),
('265','Drone Distribution','1','0','0','0','0','0','0','0','0'),
('266','Pile of Pithix','1','0','0','0','0','0','0','0','0'),
('267','Persistent Pests','1','0','0','0','0','0','0','0','0'),
('268','Stay Frosty','1','0','0','0','0','0','0','0','0'),
('269','Arisite Envy','1','0','0','0','0','0','0','0','0'),
('270','Cheap Chills','1','0','0','0','0','0','0','0','0'),
('271','Feeding the Giant','1','0','0','0','0','0','0','0','0'),
('272','Gas Injections','1','0','0','0','0','0','0','0','0'),
('273','Geodite and Gemology','1','0','0','0','0','0','0','0','0'),
('274','Ice Installation','1','0','0','0','0','0','0','0','0'),
('275','Like Drones to a Cloud','1','0','0','0','0','0','0','0','0'),
('276','Mother Lode','1','0','0','0','0','0','0','0','0'),
('277','Not Gneiss at All','1','0','0','0','0','0','0','0','0');



-- Missions have many 0 values because they arent created at random times but rather by player request. And generally around where the player accepts the mission (not at any point in the galaxy)
-- asteroid mining anomalies values(1001-1100)
INSERT INTO `dungeons` (`dungeonID`, `dungeonName`, `dungeonTypeID`, `chance`, `baseduration`, `minsec`, `maxsec`, `galaxylimit`, `regionallimit`, `constellationlimit`, `systemlimit`) VALUES
('1001','AverageFrontierDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1002','AverageHedbergite,HemorphiteandJaspetDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1003','AverageHemorphite,JaspetandKerniteDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1004','AverageJaspet,KerniteandOmberDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1005','AverageKerniteandOmberDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1006','AverageOmberDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1007','AverageSpodumain,CrokiteandDarkOchreDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1008','ExceptionalCoreDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1009','InfrequentCoreDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1010','IsolatedCoreDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1011','LargeAsteroidCluster','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1012','LargeHedbergite,HemorphiteandJaspetDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1013','LargeHemorphite,JaspetandKerniteDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1014','LargeJaspet,KerniteandOmberDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1015','LargeKerniteandOmberDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1016','LargeOmberDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1017','ModerateAsteroidCluster','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1018','OrdinaryPerimeterDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1019','RarifiedCoreDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1020','SmallAsteroidCluster','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1021','SmallBistotDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1022','SmallHedbergite,HemorphiteandJaspetDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1023','SmallHemorphite,JaspetandKerniteDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1024','SmallJaspet,KerniteandOmberDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1025','SmallKerniteandOmberDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1026','SmallMercoxit,ArkonorandBistotDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1027','SmallOmberDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1028','UncommonCoreDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1029','UnexceptionalFrontierDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2'),
('1030','UnusualCoreDeposit','2','0.3','3600','-1.0','1.0','100','20','8','2');

-- analyse or salvage (1101-1400)

-- data: hacking (1401-1700)
INSERT INTO `dungeons` (`dungeonID`, `dungeonName`, `dungeonTypeID`, `chance`, `baseduration`, `minsec`, `maxsec`, `galaxylimit`, `regionallimit`, `constellationlimit`, `systemlimit`) VALUES
('1401','LocalAngelMainframe','4','0.3','3600','0.5','1','100','20','8','2'),
('1402','LocalAngelVirusTestSite','4','0.3','3600','0.5','1','100','20','8','2'),
('1403','LocalAngelDataProcessingCenter','4','0.3','3600','0.5','1','100','20','8','2'),
('1404','LocalAngelShatteredLife-SupportUnit','4','0.3','3600','0.5','1','100','20','8','2'),
('1405','LocalAngelDataTerminal','4','0.3','3600','0.5','1','100','20','8','2'),
('1406','LocalAngelProductionInstallation','4','0.3','3600','0.5','1','100','20','8','2'),
('1407','LocalAngelBackupServer','4','0.3','3600','0.5','1','100','20','8','2'),
('1408','LocalAngelMinorShipyard','4','0.3','3600','0.5','1','100','20','8','2'),
('1409','LocalBloodRaiderMainframe','4','0.3','3600','0.5','1','100','20','8','2'),
('1410','LocalBloodRaiderVirusTestSite','4','0.3','3600','0.5','1','100','20','8','2'),
('1411','LocalBloodRaiderDataProcessingCenter','4','0.3','3600','0.5','1','100','20','8','2'),
('1412','LocalBloodRaiderShatteredLife-SupportUnit','4','0.3','3600','0.5','1','100','20','8','2'),
('1413','LocalBloodRaiderDataTerminal','4','0.3','3600','0.5','1','100','20','8','2'),
('1414','LocalBloodRaiderProductionInstallation','4','0.3','3600','0.5','1','100','20','8','2'),
('1415','LocalBloodRaiderBackupServer','4','0.3','3600','0.5','1','100','20','8','2'),
('1416','LocalBloodRaiderMinorShipyard','4','0.3','3600','0.5','1','100','20','8','2'),
('1417','LocalGuristasMainframe','4','0.3','3600','0.5','1','100','20','8','2'),
('1418','LocalGuristasVirusTestSite','4','0.3','3600','0.5','1','100','20','8','2'),
('1419','LocalGuristasDataProcessingCenter','4','0.3','3600','0.5','1','100','20','8','2'),
('1420','LocalGuristasShatteredLife-SupportUnit','4','0.3','3600','0.5','1','100','20','8','2'),
('1421','LocalGuristasDataTerminal','4','0.3','3600','0.5','1','100','20','8','2'),
('1422','LocalGuristasProductionInstallation','4','0.3','3600','0.5','1','100','20','8','2'),
('1423','LocalGuristasBackupServer','4','0.3','3600','0.5','1','100','20','8','2'),
('1424','LocalGuristasMinorShipyard','4','0.3','3600','0.5','1','100','20','8','2'),
('1425','LocalSanshaMainframe','4','0.3','3600','0.5','1','100','20','8','2'),
('1426','LocalSanshaVirusTestSite','4','0.3','3600','0.5','1','100','20','8','2'),
('1427','LocalSanshaDataProcessingCenter','4','0.3','3600','0.5','1','100','20','8','2'),
('1428','LocalSanshaShatteredLife-SupportUnit','4','0.3','3600','0.5','1','100','20','8','2'),
('1429','LocalSanshaDataTerminal','4','0.3','3600','0.5','1','100','20','8','2'),
('1430','LocalSanshaProductionInstallation','4','0.3','3600','0.5','1','100','20','8','2'),
('1431','LocalSanshaBackupServer','4','0.3','3600','0.5','1','100','20','8','2'),
('1432','LocalSanshaMinorShipyard','4','0.3','3600','0.5','1','100','20','8','2'),
('1433','LocalSerpentisMainframe','4','0.3','3600','0.5','1','100','20','8','2'),
('1434','LocalSerpentisVirusTestSite','4','0.3','3600','0.5','1','100','20','8','2'),
('1435','LocalSerpentisDataProcessingCenter','4','0.3','3600','0.5','1','100','20','8','2'),
('1436','LocalSerpentisShatteredLife-SupportUnit','4','0.3','3600','0.5','1','100','20','8','2'),
('1437','LocalSerpentisDataTerminal','4','0.3','3600','0.5','1','100','20','8','2'),
('1438','LocalSerpentisProductionInstallation','4','0.3','3600','0.5','1','100','20','8','2'),
('1439','LocalSerpentisBackupServer','4','0.3','3600','0.5','1','100','20','8','2'),
('1440','LocalSerpentisMinorShipyard','4','0.3','3600','0.5','1','100','20','8','2'),
('1441','RegionalAngelDataFortress','4','0.3','3600','0.5','1','100','20','8','2'),
('1442','RegionalAngelMainframe','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1443','RegionalAngelCommandCenter','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1444','RegionalAngelDataProcessingCenter','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1445','RegionalAngelDataMiningSite','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1446','RegionalAngelDataTerminal','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1447','RegionalAngelBackupServer','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1448','RegionalAngelSecureServer','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1449','RegionalBloodRaiderDataFortress','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1450','RegionalBloodRaiderMainframe','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1451','RegionalBloodRaiderCommandCenter','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1452','RegionalBloodRaiderDataProcessingCenter','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1453','RegionalBloodRaiderDataMiningSite','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1454','RegionalBloodRaiderDataTerminal','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1455','RegionalBloodRaiderBackupServer','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1456','RegionalBloodRaiderSecureServer','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1457','RegionalGuristasDataFortress','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1458','RegionalGuristasMainframe','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1459','RegionalGuristasCommandCenter','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1460','RegionalGuristasDataProcessingCenter','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1461','RegionalGuristasDataMiningSite','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1462','RegionalGuristasDataTerminal','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1463','RegionalGuristasBackupServer','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1464','RegionalGuristasSecureServer','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1465','RegionalSanshaDataFortress','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1466','RegionalSanshaMainframe','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1467','RegionalSanshaCommandCenter','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1468','RegionalSanshaDataProcessingCenter','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1469','RegionalSanshaDataMiningSite','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1470','RegionalSanshaDataTerminal','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1471','RegionalSanshaBackupServer','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1472','RegionalSanshaSecureServer','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1473','RegionalSerpentisDataFortress','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1474','RegionalSerpentisMainframe','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1475','RegionalSerpentisCommandCenter','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1476','RegionalSerpentisDataProcessingCenter','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1477','RegionalSerpentisDataMiningSite','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1478','RegionalSerpentisDataTerminal','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1479','RegionalSerpentisBackupServer','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1480','RegionalSerpentisSecureServer','4','0.3','3600','0.01','0.5','100','20','8','2'),
('1481','CentralAngelSparkingTransmitter','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1482','CentralAngelSurveySite','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1483','CentralAngelCommandCenter','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1484','CentralAngelDataMiningSite','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1485','CentralBloodRaiderSparkingTransmitter','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1486','CentralBloodRaiderSurveySite','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1487','CentralBloodRaiderCommandCenter','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1488','CentralBloodRaiderDataMiningSite','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1489','CentralGuristasSparkingTransmitter','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1490','CentralGuristasSurveySite','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1491','CentralGuristasCommandCenter','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1492','CentralGuristasDataMiningSite','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1493','CentralSanshaSparkingTransmitter','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1494','CentralSanshaSurveySite','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1495','CentralSanshaCommandCenter','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1496','CentralSanshaDataMiningSite','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1497','CentralSerpentisSparkingTransmitter','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1498','CentralSerpentisSurveySite','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1499','CentralSerpentisCommandCenter','4','0.3','3600','-1','-0.01','100','20','8','2'),
('1500','CentralSerpentisDataMiningSite','4','0.3','3600','-1','-0.01','100','20','8','2');
-- gas mining/ladar (1701-1800)
INSERT INTO `dungeons` (`dungeonID`, `dungeonName`, `dungeonTypeID`, `chance`, `baseduration`, `minsec`, `maxsec`, `galaxylimit`, `regionallimit`, `constellationlimit`, `systemlimit`) VALUES
('1701','AngelChemicalLab','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1702','BlackeyeNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1703','BloodRaiderChemicalLab','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1704','BoisterousNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1705','BrightNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1706','CalabashNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1707','CardinalNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1708','CHAINMindfloodDistributionSite','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1709','CobraNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1710','CoreRunnerDropDistribution','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1711','CoreRunnerDropProductionFacility','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1712','CoreRunnerExileDistributionBase','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1713','CrabSpiderNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1714','CrystalNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1715','DiabloNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1716','ElohimX-InstinctProductionFacility','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1717','EmeraldNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1718','FlameNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1719','FlowingNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1720','ForgottenNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1721','GhostNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1722','GlassNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1723','GooseNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1724','GuristasChemicalLab','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1725','H-PACrewBluePillProductionFacility','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1726','H-PAGuristasCrashDeposit','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1727','HelixNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1728','Icynebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1729','MassiveNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1730','PDW-09FXFrentixDistributionBase','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1731','RedDragonflyNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1732','RingNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1733','SaintlyNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1734','SanshaGasProcessingSite','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1735','SerpentisGasProcessingSite','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1736','ShimmeringNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1737','SisterNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1738','SmokingNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1739','SpaciousNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1740','SparklingNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1741','SunsparkNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1742','ThickNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1743','TokenPerimeterReservoir','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1744','VaporousNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1745','WildNebula','5','0.3','3600','-1.0','1.0','100','20','8','2'),
('1746','WispyNebula','5','0.3','3600','-1.0','1.0','100','20','8','2');


-- only should spawn a wormhole (1801-1900) groupid 988
INSERT INTO `dungeons` (`dungeonID`, `dungeonName`, `dungeonTypeID`, `chance`, `baseduration`, `minsec`, `maxsec`, `galaxylimit`, `regionallimit`, `constellationlimit`, `systemlimit`) VALUES
('1801', 'Test wormhole', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1802', 'Wormhole Z971', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1803', 'Wormhole R943', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1804', 'Wormhole X702', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1805', 'Wormhole O128', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1806', 'Wormhole N432', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1807', 'Wormhole M555', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1808', 'Wormhole B041', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1809', 'Wormhole U319', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1810', 'Wormhole B449', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1811', 'Wormhole N944', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1812', 'Wormhole S199', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1813', 'Wormhole A641', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1814', 'Wormhole R051', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1815', 'Wormhole V283', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1816', 'Wormhole H121', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1817', 'Wormhole C125', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1818', 'Wormhole O883', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1819', 'Wormhole M609', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1820', 'Wormhole L614', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1821', 'Wormhole S804', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1822', 'Wormhole N110', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1823', 'Wormhole J244', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1824', 'Wormhole Z060', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1825', 'Wormhole Z647', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1826', 'Wormhole D382', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1827', 'Wormhole O477', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1828', 'Wormhole Y683', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1829', 'Wormhole N062', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1830', 'Wormhole R474', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1831', 'Wormhole B274', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1832', 'Wormhole A239', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1833', 'Wormhole E545', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1834', 'Wormhole V301', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1835', 'Wormhole I182', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1836', 'Wormhole N968', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1837', 'Wormhole T405', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1838', 'Wormhole N770', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1839', 'Wormhole A982', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1840', 'Wormhole S047', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1841', 'Wormhole U210', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1842', 'Wormhole K346', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1843', 'Wormhole P060', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1844', 'Wormhole N766', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1845', 'Wormhole C247', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1846', 'Wormhole X877', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1847', 'Wormhole H900', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1848', 'Wormhole U574', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1849', 'Wormhole D845', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1850', 'Wormhole N290', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1851', 'Wormhole K329', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1852', 'Wormhole Y790', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1853', 'Wormhole D364', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1854', 'Wormhole M267', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1855', 'Wormhole E175', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1856', 'Wormhole H296', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1857', 'Wormhole V753', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1858', 'Wormhole D792', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1859', 'Wormhole C140', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1860', 'Wormhole Z142', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1861', 'Wormhole Q317', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1862', 'Wormhole G024', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1863', 'Wormhole L477', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1864', 'Wormhole Z457', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1865', 'Wormhole V911', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1866', 'Wormhole W237', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1867', 'Wormhole B520', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1868', 'Wormhole C391', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1869', 'Wormhole C248', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'), 
('1870', 'Wormhole K162', '6', '1.00', '3600', '-1.00', '1.00', '2', '1', '1', '1'); 
-- easier to find mostly combat sites 2001-2200

INSERT INTO `dungeons` (`dungeonID`, `dungeonName`, `dungeonTypeID`, `chance`, `baseduration`, `minsec`, `maxsec`, `galaxylimit`, `regionallimit`, `constellationlimit`, `systemlimit`) VALUES
('2001','AngelHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2002','AngelHiddenHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2003','AngelForsakenHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2004','AngelForlornHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2005','AngelBurrow','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2006','AngelRefuge','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2007','AngelDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2008','AngelHiddenDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2009','AngelForsakenDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2010','AngelForlornDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2011','AngelYard','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2012','AngelRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2013','AngelHiddenRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2014','AngelForsakenRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2015','AngelForlornRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2016','AngelPort','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2017','AngelHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2018','AngelHiddenHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2019','AngelForsakenHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2020','AngelForlornHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2021','AngelHaven','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2022','AngelSanctum','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2023','BloodHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2024','BloodRaiderHiddenHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2025','BloodRaiderForsakenHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2026','BloodRaiderForlornHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2027','BloodBurrow','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2028','BloodRefuge','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2029','BloodDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2030','BloodRaiderHiddenDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2031','BloodRaiderForsakenDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2032','BloodRaiderForlornDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2033','BloodYard','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2034','BloodRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2035','BloodRaiderHiddenRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2036','BloodRaiderForsakenRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2037','BloodRaiderForlornRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2038','BloodPort','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2039','BloodHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2040','BloodRaiderHiddenHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2041','BloodRaiderForsakenHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2042','BloodRaiderForlornHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2043','BloodHaven','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2044','BloodSanctum','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2045','GuristasHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2046','GuristasHiddenHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2047','GuristasForsakenHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2048','GuristasForlornHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2049','GuristasBurrow','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2050','GuristasRefuge','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2051','GuristasDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2052','GuristasHiddenDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2053','GuristasForsakenDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2054','GuristasForlornDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2055','GuristasYard','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2056','GuristasRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2057','GuristasHiddenRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2058','GuristasForsakenRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2059','GuristasForlornRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2060','GuristasPort','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2061','GuristasHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2062','GuristasHiddenHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2063','GuristasForsakenHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2064','GuristasForlornHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2065','GuristasHaven','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2066','GuristasSanctum','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2067','SanshaHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2068','SanshaHiddenHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2069','SanshaForsakenHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2070','SanshaForlornHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2071','SanshaBurrow','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2072','SanshaRefuge','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2073','SanshaDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2074','SanshaHiddenDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2075','SanshaForsakenDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2076','SanshaForlornDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2077','SanshaYard','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2078','SanshaRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2079','SanshaHiddenRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2080','SanshaForsakenRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2081','SanshaForlornRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2082','SanshaPort','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2083','SanshaHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2084','SanshaHiddenHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2085','SanshaForsakenHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2086','SanshaForlornHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2087','SanshaHaven','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2088','SanshaSanctum','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2089','SerpentisHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2090','SerpentisHiddenHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2091','SerpentisForsakenHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2092','SerpentisForlornHideaway','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2093','SerpentisBurrow','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2094','SerpentisRefuge','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2095','SerpentisDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2096','SerpentisHiddenDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2097','SerpentisForsakenDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2098','SerpentisForlornDen','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2099','SerpentisYard','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2100','SerpentisRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2101','SerpentisHiddenRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2102','SerpentisForsakenRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2103','SerpentisForlornRallyPoint','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2104','SerpentisPort','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2105','SerpentisHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2106','SerpentisHiddenHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2107','SerpentisForsakenHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2108','SerpentisForlornHub','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2109','SerpentisHaven','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2110','SerpentisSanctum','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2111','DroneCluster','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2112','DroneCollection','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2113','DroneAssembly','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2114','DroneGathering','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2115','DroneSurveillance','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2116','DroneMenagerie','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2117','DroneHerd','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2118','DroneSquad','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2119','DronePatrol','7','0.3','3600','-1.0','1.0','100','20','8','2'),
('2120','DroneHorde','7','0.3','3600','-1.0','1.0','100','20','8','2');
-- Values for minsec and maxsec are WRONG at this time. This will be fixed in a later DB update.



-- defined as Cosmic Signatures combat sites, need to be scanned down 2201-2400
-- When created creates a new entry in the players expedition missions journal tab. 2401-2500
-- For DED complexes 2501-2700
-- Custom complexes should over value 3000

-- regional is a value to be later coded to limit certain dungeons to NOT spawn in certain areas of the galaxy. So WH dungeons don't spawn IN Nullsec OR Sansha IN Caldari territory. VALUE IS TO be LEFT AS 0 FOR 'no limit'.
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
-- script is a placeholder in the DB to refer to any particular activity the spawn may do on initial player entry (move to x,y,z RELATIVE to the location or attack target etc. etc.)
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