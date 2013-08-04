-- SQL to create a NEW stargate from Amsen in Lonetrek to Jove Empire system I-R8B0

INSERT INTO `mapdenormalize`(`itemID`, `typeID`, `groupID`, `solarSystemID`, `constellationID`, `regionID`, `orbitID`, `x`, `y`, `z`, `radius`, `itemName`, `security`, `celestialIndex`, `orbitIndex`) VALUES (50020000,29625,10,30001392,20000204,10000016,NULL,390565601280,41190481920,-893183385600,NULL,"Stargate (I-R8B0)",0.0,NULL,NULL);

INSERT INTO `mapdenormalize`(`itemID`, `typeID`, `groupID`, `solarSystemID`, `constellationID`, `regionID`, `orbitID`, `x`, `y`, `z`, `radius`, `itemName`, `security`, `celestialIndex`, `orbitIndex`) VALUES (50020001,3876,10,30001466,20000215,10000017,NULL,390565601280,41190481920,-893183385600,NULL,"Stargate (Amsen)",0.0,NULL,NULL);

INSERT INTO `mapjumps`(`stargateID`, `celestialID`) VALUES (50020000,50020001);

INSERT INTO `mapjumps`(`stargateID`, `celestialID`) VALUES (50020001,50020000);

