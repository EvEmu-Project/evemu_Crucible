
CREATE TABLE `factionSalvage` (
  `factionID` int(10) unsigned NOT NULL,
  `itemID` int(10) unsigned NOT NULL,
  `itemName` varchar(45) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;


INSERT INTO `factionSalvage` (`factionID`, `itemID`, `itemName`)
  VALUES
-- angel
(500011,25595,'Alloyed Tritanium Bar'),
(500011,25605,'Armor Plates'),
(500011,25600,'Burned Logic Circuit'),
(500011,25599,'Charred Micro Circuit'),
(500011,25590,'Contaminated Nanite Compound'),
(500011,25601,'Fried Interface Circuit'),
(500011,25589,'Malfunctioning Shield Emitter'),
(500011,25593,'Smashed Trigger Unit'),
(500011,25602,'Thruster Console'),
(500011,25598,'Tripped Power Circuit'),

-- raider
(500012,25605,'Armor Plates'),
(500012,25600,'Burned Logic Circuit'),
(500012,25599,'Charred Micro Circuit'),
(500012,25590,'Contaminated Nanite Compound'),
(500012,25592,'Defective Current Pump'),
(500012,25601,'Fried Interface Circuit'),
(500012,25603,'Melted Capacitor Console'),
(500012,25594,'Tangled Power Conduit'),
(500012,25598,'Tripped Power Circuit'),

-- gurista
(500010,25600,'Burned Logic Circuit'),
(500010,25599,'Charred Micro Circuit'),
(500010,25604,'Conductive Polymer'),
(500010,25597,'Damaged Artificial Neural Network'),
(500010,25601,'Fried Interface Circuit'),
(500010,25589,'Malfunctioning Shield Emitter'),
(500010,25588,'Scorched Telemetry Processor'),
(500010,25598,'Tripped Power Circuit'),
(500010,25606,'Ward Console'),

-- sansha
(500019,25605,'Armor Plates'),
(500019,25600,'Burned Logic Circuit'),
(500019,25599,'Charred Micro Circuit'),
(500019,25590,'Contaminated Nanite Compound'),
(500019,25592,'Defective Current Pump'),
(500019,25601,'Fried Interface Circuit'),
(500019,25603,'Melted Capacitor Console'),
(500019,25594,'Tangled Power Conduit'),
(500019,25598,'Tripped Power Circuit'),

-- serpentis
(500020,25605,'Armor Plates'),
(500020,25596,'Broken Drone Transceiver'),
(500020,25600,'Burned Logic Circuit'),
(500020,25599,'Charred Micro Circuit'),
(500020,25604,'Conductive Polymer'),
(500020,25591,'Contaminated Lorentz Fluid'),
(500020,25590,'Contaminated Nanite Compound'),
(500020,25597,'Damaged Artificial Neural Network'),
(500020,25601,'Fried Interface Circuit'),

-- drone
(500021,25595,'Alloyed Tritanium Bar'),
(500021,25605,'Armor Plates'),
(500021,25596,'Broken Drone Transceiver'),
(500021,25600,'Burned Logic Circuit'),
(500021,25599,'Charred Micro Circuit'),
(500021,25604,'Conductive Polymer'),
(500021,25591,'Contaminated Lorentz Fluid'),
(500021,25590,'Contaminated Nanite Compound'),
(500021,25592,'Defective Current Pump'),
(500021,25601,'Fried Interface Circuit'),
(500021,25589,'Malfunctioning Shield Emitter'),
(500021,25603,'Melted Capacitor Console'),
(500021,25588,'Scorched Telemetry Processor'),
(500021,25593,'Smashed Trigger Unit'),
(500021,25594,'Tangled Power Conduit'),
(500021,25602,'Thruster Console'),
(500021,25598,'Tripped Power Circuit'),
(500021,25606,'Ward Console'),

-- default 'unknown' faction catch-all
(500022,25595,'Alloyed Tritanium Bar'),
(500022,25605,'Armor Plates'),
(500022,25596,'Broken Drone Transceiver'),
(500022,25600,'Burned Logic Circuit'),
(500022,25599,'Charred Micro Circuit'),
(500022,25604,'Conductive Polymer'),
(500022,25591,'Contaminated Lorentz Fluid'),
(500022,25590,'Contaminated Nanite Compound'),
(500022,25597,'Damaged Artificial Neural Network'),
(500022,25592,'Defective Current Pump'),
(500022,25601,'Fried Interface Circuit'),
(500022,25589,'Malfunctioning Shield Emitter'),
(500022,25603,'Melted Capacitor Console'),
(500022,25588,'Scorched Telemetry Processor'),
(500022,25593,'Smashed Trigger Unit'),
(500022,25594,'Tangled Power Conduit'),
(500022,25602,'Thruster Console'),
(500022,25598,'Tripped Power Circuit'),
(500022,25606,'Ward Console');

-- factions todo
/*
    factionCaldari       = 500001,
    factionMinmatar      = 500002,
    factionAmarr         = 500003,
    factionGallente      = 500004,
    factionJove          = 500005,
    factionCONCORD       = 500006,
    factionAmmatar       = 500007,
    factionKhanid        = 500008,
    factionSyndicate     = 500009,
    factionInterBus      = 500013,
    factionORE           = 500014,
    factionThukker       = 500015,
    factionSistersOfEVE  = 500016,
    factionSociety       = 500017,
    factionMordusLegion  = 500018,
*/