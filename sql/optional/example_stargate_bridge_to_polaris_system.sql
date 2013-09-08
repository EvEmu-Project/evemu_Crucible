-- SQL to create a NEW stargate from Amsen in Lonetrek to Jove Empire system I-R8B0

INSERT INTO `mapdenormalize`(`itemID`, `typeID`, `groupID`, `solarSystemID`, `constellationID`, `regionID`, `orbitID`, `x`, `y`, `z`, `radius`, `itemName`, `security`, `celestialIndex`, `orbitIndex`) VALUES (50020002,29625,10,30001392,20000204,10000016,NULL,-190565601280,-41190481920,793183385600,NULL,"Stargate (Polaris)",0.0,NULL,NULL);

INSERT INTO `mapdenormalize`(`itemID`, `typeID`, `groupID`, `solarSystemID`, `constellationID`, `regionID`, `orbitID`, `x`, `y`, `z`, `radius`, `itemName`, `security`, `celestialIndex`, `orbitIndex`) VALUES (50020003,3876,10,30000380,20000054,10000004,NULL,-190565601280,-41190481920,793183385600,NULL,"Stargate (Amsen)",0.0,NULL,NULL);

INSERT INTO `mapjumps`(`stargateID`, `celestialID`) VALUES (50020002,50020003);

INSERT INTO `mapjumps`(`stargateID`, `celestialID`) VALUES (50020003,50020002);

