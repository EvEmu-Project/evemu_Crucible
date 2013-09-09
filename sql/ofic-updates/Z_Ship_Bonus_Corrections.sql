
-- For Basilisk, value for bonus to Shield Maintenance Bot Transfer Amount per level is 20% in description, yet 'dgmTypeAttributes' has 100
UPDATE `dgmTypeAttributes` SET valueFloat = 20 WHERE (typeID = 11985 AND attributeID = 1220);

-- For Flycatcher, value for bonus to Light Missile Damage Reduction Factor per level is -3% in description, yet 'dgmTypeAttributes' has 3
UPDATE `dgmTypeAttributes` SET valueFloat = -3 WHERE (typeID = 22464 AND attributeID = 759);

-- For Flycatcher, value for bonus to Armor Maintenance Bot Transfer Amount per level is 20% in description, yet 'dgmTypeAttributes' has 100
UPDATE `dgmTypeAttributes` SET valueFloat = 20 WHERE (typeID = 11987 AND attributeID = 1219);

-- For Hulk, value for Shield Resistance Bonus per level was missing from the database
INSERT INTO `dgmTypeAttributes` (`typeID`, `attributeID`, `valueInt`, `valueFloat`) VALUES (22544, 773, NULL, -7.5);

-- For Ibis, value for bonus to Small Hybrid Turret Range per level is 10% in description, yet 'dgmTypeAttributes' has 5%
UPDATE `dgmTypeAttributes` SET valueFloat = 10 WHERE (typeID = 601 AND attributeID = 463);

-- For Mackinaw, value for Shield Resistance Bonus per level was missing from the database
INSERT INTO `dgmTypeAttributes` (`typeID`, `attributeID`, `valueInt`, `valueFloat`) VALUES (22548, 773, NULL, -7.5);

-- For Mackinaw, value for bonus to Ice Harvester Yield 100% in description, yet 'dgmTypeAttributes' has 10%
UPDATE `dgmTypeAttributes` SET valueFloat = 100 WHERE (typeID = 22548 AND attributeID = 924);

-- For Onerios, value for bonus to Ice Harvester Yield 100% in description, yet 'dgmTypeAttributes' has 10%
UPDATE `dgmTypeAttributes` SET valueFloat = 20 WHERE (typeID = 11989 AND attributeID = 1219);

-- For Scimitar, value for bonus to Shield Maintenance Bot Transfer Amount per level is 20% in description, yet 'dgmTypeAttributes' has 100
UPDATE `dgmTypeAttributes` SET valueFloat = 20 WHERE (typeID = 11978 AND attributeID = 1220);
