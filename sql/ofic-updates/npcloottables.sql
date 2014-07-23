--  Creates tables relating to NPC loot.

--  Why use typeID, why not use groupID or marketgroupID?
--  GroupID and marketGroupID link all similar modules/items together. This includes faction or T2 items. A small pirate NPC never drops T2 stuff. This is why typeID is used.

--  Why so many sub-groups?
--  On occasion it is possible to have 2 similar items drop. (Two different missile launchers from the same NPC) so smaller groups relating to turrets have been made. This has also been broken down for tags, narcotics and a few other groups. Also makes it possible for unique NPC's.

--  Mission drops?
--  This would require going through mission reports and linking individual items to individual NPCs. It can be implemented on a mission-by-mission basis later. It is simply too time consuming with too little pay-off at this juncture.

--  Table errors?
--  It is very possible "x" doesn't drop from "y" NPC. Or "z"'s drop rate is too high. This would require intensive testing and player feedback. However alterations and additions to the table can be made easily enough.

--  About the chance and quantity of drops. 
--  It may be better to keep the chance minquantity, maxquantity in the lootGroup table. This means there is a statistical possibility that several similar items could drop. 
--  What would happen is a npcGroupLoot would make a chance to produce a drop. If that chance is successful it would roll again from the npcGroupLootType table. If unsuccessful it would move to the next groupID and chance on the list.
--  The sum of all the 'like' lootGroupID's (all of lootGroupID 7) in the chance field is 1 (100%). Alternatively the combined total of all the chances and then a random roll to select from one of them. 

--  Drop rates?
--  Drop rates appear to be low. This is deliberate and will definitely need tweaking as testing becomes possible. This is due to prevent multiple similar drops. 

--  What's up with the npcGroupLootItems?
--  That's for future DB maintenance reference. It would not be used in the server.

--  For testing purposes the Guristas Arrogator (typeID 2382) has a 100% chance to drop 1 corpse, obviously this will be removed at a later time.

--  ----------------------------
--  Table structure for `npcLootItems`
--  ----------------------------
DROP TABLE IF EXISTS `npcLootItems`;
CREATE TABLE `npcLootItems` (
  `groupLootID` INT(11) NOT NULL,
  `groupIDDescription` TEXT CHARACTER SET latin1,
  PRIMARY KEY (`groupLootID`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

INSERT INTO `npcLootItems` (`groupLootID`, `groupIDDescription`) VALUES

-- TEST groupid for devs
('9999', 'Corpse TEST'),
-- Metal Scraps
('1', 'Metal Scraps (small)'),
('2', 'Metal Scraps (medium)'),
('3', 'Metal Scraps (large)'),
('4', 'Metal Scraps (extra large)'),
-- Ammunition & Charges
-- Hybrid Charges
('5', 'Small Hybrid Charges'),
('6', 'Medium Hybrid Charges'),
('7', 'Large Hybrid Charges'),
('8', 'Extra Large Hybrid Charges'),
-- Projectile Ammo
('9', 'Small Projectile Ammo'),
('10', 'Medium Projectile Ammo'),
('11', 'Large Projectile Ammo'),
('12', 'Extra Large Projectile Ammo'),
-- Frequency Crystals
('13', 'Small Frequency Crystal'),
('14', 'Medium Frequency Crystals'),
('15', 'Large Frequency Crystals'),
('16', 'Extra Large Frequency Crystals'),
-- Missiles
('17', 'Auto-Targeting'),
('18', 'Defender'),
('19', 'Rocket'),
('20', 'Light Missile'),
('21', 'Heavy Missile'),
('22', 'Heavy Assault Missile'),
('23', 'Cruise Missile'),
('24', 'Torpedo'),
-- Cap Booster Charges
('25', 'Cap Booster Charges small'),
('26', 'Cap Booster Charges medium'),
('27', 'Cap Booster Charges large'),
('28', 'Cap Booster Charges extra large'),
('29', 'ProbesNA'),
('30', 'Mining CrystalsNA'),
('31', 'BombsNA'),
('32', 'ScriptsNA'),
('33', 'Nanite Repair PasteNA'),
-- Materials
('34', 'Alloys & Compounds'),
('35', 'Salvaged Materials'),
-- Minerals
('36', 'small hauler'),
('37', 'medium hauler'),
('38', 'large hauler'),
('39', 'capital hauler'),
 -- Ore
-- Trade Goods
('40', 'Industrial Goods'),
('41', 'Consumer Products'),
('42', 'Passengers'),
('43', 'Radioactive Goods'),
 -- Narcotics
('44', 'Crash'),
('45', 'Crystal Egg'),
('46', 'Drop'),
('47', 'Exile'),
('48', 'Frentix'),
('49', 'Mindflood'),
('50', 'Nerve Sticks'),
('51', 'Sooth Sayer'),
('52', 'Vitoc'),
('53', 'X-Instinct'),
-- Turrets & Bays
-- Hybrid Turrets
 -- Railguns
 -- Hybrid Turrents small
('54', 'Railguns 75mm'),
('55', 'Railguns 125mm'),
('56', 'Railguns 150mm'),
('57', 'Hybrid Turrets medium '),
('58', 'Dual Railguns 150mm'),
('59', 'Railguns 200mm'),
('60', 'Railguns 250mm'),
 -- Hybrid Turrets large
('61', 'Dual Railguns 250mm'),
('62', 'Railguns 350mm'),
('63', 'Railguns 425mm'),
 -- Hybrid Turrets extra large
 -- Blasters
 -- Blasters small
('64', 'Light Ion Blaster'),
('65', 'Light Neutron Blaster'),
('66', 'Light Electron Blaster'),
 -- Blasters medium
('67', 'Heavy Ion Blaster'),
('68', 'Heavy Neutron Blaster'),
('69', 'Heavy Electron Blaster'),
 -- Blasters large
('70', 'Ion Blaster Cannon'),
('71', 'Neutron Blaster Cannon'),
('72', 'Electron Blaster Cannon'),
-- Blasters extra large
-- Projectile Turrets
-- Autocannons
('73', 'Autocannons small'),
('74', '125mm AutoCannon'),
('75', '150mm AutoCannon'),
('76', '200mm Autocannon'),
-- Autocannons medium
('77', '180mm Dual Autocannon'),
('78', '220mm Autocannon'),
('79', '425mm Autocannon'),
-- Autocannons large
('80', '425mm DualAutocannon'),
('81', '650mm Autocannon'),
('82', '800mm Autocannon'),
 -- Artillery Cannonssmall
('83', '250mm Artillery Cannon'),
('84', '280mm Howitzer Artillery'),
 -- Artillery Cannonsmedium
('85', '650mm Artillery Cannon'),
('86', '720mm Howitzer Artillery'),
 -- Artillery Cannonslarge
('87', '1200mm Artillery Cannon'),
('88', '1400mm Howitzer Artillery'),
('89', '1200mm Heavy Artillery'),
 -- Artillery Cannonsextra large
-- Laser Turrets
 -- Beam Laserssmall
('90', 'smallDual Beam Laser'),
('91', 'smallFocused Beam Laser'),
 -- Beam Lasersmedium'),
('92', 'Heavy Beam Laser'),
('93', 'mediumFocused Beam Laser'),
('94', 'Quad Light Beam Laser'),
 -- Beam Laserslarge'),
('95', 'Dual Heavy Beam Laser'),
('96', 'Mega Beam Laser'),
('97', 'Tachyon Beam Laser'),
 -- Beam Lasersextra large
-- Pulse Lasers
 -- Pulse Laserssmall
('99', 'Small Focused Pulse Laser'),
('100', 'Gatling Pulse Laser'),
('101', 'Dual Light Pulse Laser'),
-- Pulse Lasersmedium
('102', 'Heavy Pulse Laser'),
('103', 'Focused Medium Pulse Laser'),
 -- Pulse Laserslarge
('104', 'Dual Heavy Pulse Laser'),
('105', 'Mega Pulse Laser'),
 -- Pulse Lasersextra large
-- Missile Launchers
('106', 'Rocket Launchers'),
('107', 'Light Missile Launchers'),
('108', 'Rapid Light Missile Launchers'),
('109', 'Heavy Launchers'),
('110', 'Cruise Launchers'),
('111', 'Torpedo Launchers'),
('112', 'Citadel Launchers'),
('113', 'Heavy Assault Launchers'),
('114', 'Rapid Heavy Missile Launchers'),
-- Weapon Upgrades
('115', 'Ballistic Control Systems'),
('116', 'Gyrostabilizers'),
('117', 'Heat Sinks'),
('118', 'Magnetic Field Stabilizers'),
('119', 'Tracking Computers'),
('120', 'Tracking Enhancers'),
('121', 'Remote Tracking Computers'),
('122', 'Siege Modules'),
('123', 'Bomb Launchers'),
-- Doomsday Devices
-- Hull & Armor 
-- Armor Plates
('124', 'Armor Platessmall'),
('125', 'Armor Platesmedium'),
('126', 'Armor Plateslarge'),
-- Armor Repairers
('127', 'Armor Repairerssmall'),
('128', 'Armor Repairersmedium'),
('129', 'Armor Repairerslarge'),
('130', 'Armor Repairerscapital'),
-- Hull Upgrades
('131', 'Reinforced Bulkheads'),
('132', 'Nanofiber Internal Structures'),
('133', 'Expanded Cargoholds'),
-- Armor Hardeners
('134', 'Thermal Armor Hardeners'),
('135', 'Kinetic Armor Hardeners'),
('136', 'Explosive Armor Hardeners'),
('137', 'EM Armor Hardeners'),
-- Remote Armor Repairers
('138', 'Remote Armor Repairerssmall'),
('139', 'Remote Armor Repairersmedium'),
('140', 'Remote Armor Repairerslarge'),
('141', 'Remote Armor Repairerscapital'),
-- Hull Repairers
('142', 'Hull Repairerssmall'),
('143', 'Hull Repairersmedium'),
('144', 'Hull Repairerslarge'),
-- Resistance Plating
('145', 'Thermal Resistance Plating'),
('146', 'Kinetic Resistance Plating'),
('147', 'Explosive Resistance Plating'),
('148', 'EM Resistance Plating'),
('149', 'Adaptive Resistance Plating'),
-- Energized Plating
('150', 'Energized Explosive Plating'),
('151', 'Energized Thermal Plating'),
('152', 'Energized EM Plating'),
('153', 'Energized Kinetic Plating'),
('154', 'Energized Adaptive Plating'),
('155', 'Damage Controls'),
-- Remote Hull Repairers
('156', 'Remote Hull Repairerssmall'),
('157', 'Remote Hull Repairersmedium'),
('158', 'Remote Hull Repairerslarge'),
('159', 'Remote Hull Repairerscapital'),
('160', 'Reactive Armor Hardener'),
('161', 'Layered Plating'),
('162', 'Energized Armor Layering'),
-- Propulsion 
-- Microwarpdrives
('163', 'Microwarpdrivessmall'),
('164', 'Microwarpdrivesmedium'),
('165', 'Microwarpdriveslarge'),
('166', 'Propulsion Upgrades'),
('167', 'Inertia Stabilizers'),
('168', 'Overdrives'),
('169', 'Warp Core Stabilizers'),
-- Afterburners
('170', 'Afterburnerssmall'),
('171', 'Afterburnersmedium'),
('172', 'Afterburnerslarge'),
('173', 'Micro Jump Drives'),
 -- Smartbombs
('174', 'Micro'),
('175', 'Small'),
('176', 'Medium'),
('177', 'Large'),
-- Shield
('178', 'Shield Rechargers'),
-- Remote Shield Boosters
('179', 'MicroRemote Shield Boosters'),
('180', 'SmallRemote Shield Boosters'),
('181', 'MediumRemote Shield Boosters'),
('182', 'LargeRemote Shield Boosters'),
('183', 'CapitalRemote Shield Boosters'),
('184', 'Shield Resistance Amplifiers'),
('185', 'SmallShield Extenders'),
('186', 'MediumShield Extenders'),
('187', 'MicroShield Extenders'),
('188', 'LargeShield Extenders'),
('189', 'SmallShield Boosters'),
('190', 'MediumShield Boosters'),
('191', 'LargeShield Boosters'),
('192', 'Extra LargeShield Boosters'),
('193', 'Boost Amplifiers'),
('194', 'Shield Hardeners'),
('195', 'Shield Flux Coils'),
('196', 'Shield Power Relays'),
-- Engineering Equipment
('197', 'Power Diagnostic Systems'),
('198', 'Reactor Control Units'),
('199', 'Auxiliary Power Controls'),
('200', 'SmallEnergy Destabilizers'),
('201', 'MediumEnergy Destabilizers'),
('202', 'HeavyEnergy Destabilizers'),
('203', 'SmallEnergy Vampires'),
('204', 'MediumEnergy Vampires'),
('205', 'HeavyEnergy Vampires'),
('206', 'SmallRemote Capacitor Transmitters'),
('207', 'MediumRemote Capacitor Transmitters'),
('208', 'LargeRemote Capacitor Transmitters'),
('209', 'MicroCapacitor Batteries'),
('210', 'SmallCapacitor Batteries'),
('211', 'MediumCapacitor Batteries'),
('212', 'LargeCapacitor Batteries'),
('213', 'Capacitor Rechargers'),
('214', 'Capacitor Flux Coils'),
('215', 'Capacitor Power Relays'),
('216', 'MicroCapacitor Boosters'),
('217', 'SmallCapacitor Boosters'),
('218', 'MediumCapacitor Boosters'),
('219', 'HeavyCapacitor Boosters'),
-- Electronics and Sensor Upgrades
('220', 'Signal Amplifiers'),
('221', 'Automated Targeting Systems'),
('222', 'Sensor Boosters'),
('223', 'Passive Targeting Systems'),
('224', 'Remote Sensor Boosters'),
('225', 'Cloaking Devices'),
('226', 'CPU Upgrades'),
('227', 'Tractor Beams'),
-- Electronic Warfare
-- Electronic Counter Measures
('228', 'Magnetometric Jammers'),
('229', 'Ladar Jammers'),
('230', 'Gravimetric Jammers'),
('231', 'Radar Jammers'),
('232', 'Multi Spectrum Jammers'),
('233', 'Signal Distortion Amplifier'),
('234', 'ECM Bursts'),
('235', 'Remote Sensor Dampeners'),
('236', 'Tracking Disruptors'),
-- Sensor Backup Arrays
('237', 'Gravimetric Backup Arrays'),
('238', 'Ladar Backup Arrays'),
('239', 'Radar Backup Arrays'),
('240', 'Magnetometric Backup Arrays'),
('241', 'Multi-Frequency Backup Arrays'),
('242', 'Warp Jammers'),
('243', 'Stasis Webifiers'),
('244', 'ECCM'),
('245', 'Projected ECCM'),
('246', 'Target Painters'),
('247', 'Warp Disruption Field Generators'),
('248', 'Target Breaker'),
('249', 'Civilian Modules'),
-- Fleet Assistance Modules
-- Warfare Links
('250', 'Armored Warfare Links'),
('251', 'Information Warfare Links'),
('252', 'Siege Warfare Links'),
('253', 'Skirmish Warfare Links'),
('254', 'Drone Upgrades'),
('255', 'Scanning Equipment'),
('256', 'Cargo Scanners'),
('257', 'Scan Probe Launchers'),
('258', 'Ship Scanners'),
('259', 'Survey Scanners'),
('260', 'Scanning Upgrades'),
('261', 'Survey Probe Launchers'),
('262', 'Analyzers'),
-- Harvest Equipment
('263', 'Mining Upgrades'),
('264', 'Gas Cloud Harvesters'),
('265', 'Ice Harvesters'),
('266', 'Mining Lasers'),
('267', 'Strip Miners'),
('268', 'Salvagers'),
-- Skillbooks
('269', 'Science books'),
-- Drones
-- Light Scout Drones
('270', 'Caldari'),
('271', 'Gallente'),
('272', 'Minmatar'),
('273', 'Amarr'),
-- Medium Scout Drones
('274', 'Caldari'),
('275', 'Gallente'),
('276', 'Minmatar'),
('277', 'Amarr'),
-- Heavy Attack Drones
('278', 'Caldari'),
('279', 'Gallente'),
('280', 'Minmatar'),
('281', 'Amarr');
-- Blueprints

-- Other entries to be done on an 'as needed' basis


-- ----------------------------
-- Table structure for `npcGroupLootType`
-- ----------------------------
DROP TABLE IF EXISTS `npcGroupLootType`;
CREATE TABLE `npcGroupLootType` (
  `groupLootID` INT(11) NOT NULL,
  `typeID` INT(11) NOT NULL,
  `typename` TEXT CHARACTER SET latin1,
  `chance` DOUBLE NULL,
  `minquantity` INT(11) NOT NULL,
  `maxquantity` INT(11) NOT NULL,
  KEY `groupLootID` (`groupLootID`,`typeID`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

INSERT INTO `npcGroupLootType` (`groupLootID`, `typeID`, `typename`, `chance`, `minquantity`, `maxquantity`) VALUES
-- Test drop for devs
('9999', '25', 'Test Corpse', '1', '1', '1'),

('1', '15331', 'Metal Scraps S', '1', '1', '2'),
('2', '15331', 'Metal Scraps M', '1', '1', '4'),
('3', '15331', 'Metal Scraps L', '1', '2', '6'),
('4', '15331', 'Metal Scraps XL', '1', '3', '15'),

 -- Small Hybrid Charges
('5', '215', 'Iron Charge S', '0.15', '100', '100'),
('5', '216', 'Tungsten Charge S', '0.15', '100', '100'),
('5', '217', 'Iridium Charge S', '0.15', '100', '100'),
('5', '218', 'Lead Charge S', '0.15', '100', '100'),
('5', '219', 'Thorium Charge S', '0.1', '100', '100'),
('5', '220', 'Uranium Charge S', '0.1', '100', '100'),
('5', '221', 'Plutonium Charge S', '0.1', '100', '100'),
('5', '222', 'Antimatter Charge S', '0.1', '100', '100'),

 -- Medium Hybrid Charges
('6', '223', 'Iron Charge M', '0.15', '100', '100'),
('6', '224', 'Tungsten Charge M', '0.15', '100', '100'),
('6', '225', 'Iridium Charge M', '0.15', '100', '100'),
('6', '226', 'Lead Charge M', '0.15', '100', '100'),
('6', '227', 'Thorium Charge M', '0.1', '100', '100'),
('6', '228', 'Uranium Charge M', '0.1', '100', '100'),
('6', '229', 'Plutonium Charge M', '0.1', '100', '100'),
('6', '230', 'Antimatter Charge M', '0.1', '100', '100'),

 -- Large Hybrid Charges
('7', '231', 'Iron Charge L', '0.15', '100', '100'),
('7', '232', 'Tungsten Charge L', '0.15', '100', '100'),
('7', '233', 'Iridium Charge L', '0.15', '100', '100'),
('7', '234', 'Lead Charge L', '0.15', '100', '100'),
('7', '235', 'Thorium Charge L', '0.1', '100', '100'),
('7', '236', 'Uranium Charge L', '0.1', '100', '100'),
('7', '237', 'Plutonium Charge L', '0.1', '100', '100'),
('7', '238', 'Antimatter Charge L', '0.1', '100', '100'),

 -- Small Projectile Ammo
('9', '178', 'Carbonized Lead S', '0.15', '100', '100'),
('9', '179', 'Nuclear S', '0.15', '100', '100'),
('9', '180', 'Proton S', '0.15', '100', '100'),
('9', '181', 'Depleted Uranium S', '0.15', '100', '100'),
('9', '182', 'Titanium Sabot S', '0.1', '100', '100'),
('9', '183', 'Fusion S', '0.1', '100', '100'),
('9', '184', 'Phased Plasma S', '0.1', '100', '100'),
('9', '185', 'EMP S', '0.1', '100', '100'),

 -- Medium Projectile Ammo
('10', '186', 'Carbonized Lead M', '0.15', '100', '100'),
('10', '187', 'Nuclear M', '0.15', '100', '100'),
('10', '188', 'Proton M', '0.15', '100', '100'),
('10', '189', 'Depleted Uranium M', '0.15', '100', '100'),
('10', '190', 'Titanium Sabot M', '0.1', '100', '100'),
('10', '191', 'Fusion M', '0.1', '100', '100'),
('10', '192', 'Phased Plasma M', '0.1', '100', '100'),
('10', '193', 'EMP M', '0.1', '100', '100'),

 -- Large Projectile Ammo
('11', '194', 'Carbonized Lead L', '0.15', '100', '100'),
('11', '195', 'Nuclear L', '0.15', '100', '100'),
('11', '196', 'Proton L', '0.15', '100', '100'),
('11', '197', 'Depleted Uranium L', '0.15', '100', '100'),
('11', '198', 'Titanium Sabot L', '0.1', '100', '100'),
('11', '199', 'Fusion L', '0.1', '100', '100'),
('11', '200', 'Phased Plasma L', '0.1', '100', '100'),
('11', '201', 'EMP L', '0.1', '100', '100'),

 -- Rocket
('19', '2516', 'Phalanx Rocket', '0.25', '100', '100'),
('19', '266', 'Thorn Rocket', '0.25', '100', '100'),
('19', '2514', 'Foxfire Rocket', '0.25', '100', '100'),
('19', '2512', 'Gremlin Rocket', '0.25', '100', '100'),

 -- Light Missile
('20', '213', 'Piranha Light Missile', '0.25', '100', '100'),
('20', '210', 'Bloodclaw Light Missile', '0.25', '100', '100'),
('20', '211', 'Flameburst Light Missile', '0.25', '100', '100'),
('20', '212', 'Sabretooth Light Missile', '0.25', '100', '100'),

 -- Heavy Missile
('21', '206', 'Havoc Heavy Missile', '0.25', '100', '100'),
('21', '209', 'Scourge Heavy Missile', '0.25', '100', '100'),
('21', '208', 'Widowmaker Heavy Missile', '0.25', '100', '100'),
('21', '207', 'Thunderbolt Heavy Missile', '0.25', '100', '100'),

 -- Cruise Missile
('23', '205', 'Devastator Cruise Missile', '0.25', '100', '100'),
('23', '203', 'Wrath Cruise Missile', '0.25', '100', '100'),
('23', '204', 'Cataclysm Cruise Missile', '0.25', '100', '100'),
('23', '202', 'Paradise Cruise Missile', '0.25', '100', '100'),

 -- Torpedo
('24', '2508', 'Bane Torpedo', '0.25', '100', '100'),
('24', '267', 'Juggernaut Torpedo', '0.25', '100', '100'),
('24', '2510', 'Inferno Torpedo', '0.25', '100', '100'),
('24', '2506', 'Mjolnir Torpedo', '0.25', '100', '100'),

 -- Small Railguns
('54', '561', '75mm Gatling Rail I', '0.2', '1', '1'),
('54', '7247', '75mm Prototype Gauss Gun', '0.2', '1', '1'),
('54', '7249', '75mm Scout Accelerator Cannon', '0.2', '1', '1'),
('54', '7251', '75mm Carbide Railgun I', '0.2', '1', '1'),
('54', '7253', '75mm Compressed Coil Gun I', '0.2', '1', '1'),

('55', '10678', '125mm Railgun I', '0.2', '1', '1'),
('55', '10688', '125mm Scout Accelerator Cannon', '0.2', '1', '1'),
('55', '10690', '125mm Carbide Railgun I', '0.2', '1', '1'),
('55', '10692', '125mm Compressed Coil Gun I', '0.2', '1', '1'),
('55', '10694', '125mm Prototype Gauss Gun', '0.2', '1', '1'),

('56', '565', '150mm Railgun I', '0.2', '1', '1'),
('56', '7287', '150mm Prototype Gauss Gun', '0.2', '1', '1'),
('56', '7289', '150mm Scout Accelerator Cannon', '0.2', '1', '1'),
('56', '7291', '150mm Carbide Railgun I', '0.2', '1', '1'),
('56', '7293', '150mm Compressed Coil Gun I', '0.2', '1', '1'),

('58', '567', 'Dual 150mm Railgun I', '0.2', '1', '1'),
('58', '7327', 'Dual 150mm Prototype Gauss Gun', '0.2', '1', '1'),
('58', '7329', 'Dual 150mm Scout Accelerator Cannon', '0.2', '1', '1'),
('58', '7331', 'Dual 150mm Carbide Railgun I', '0.2', '1', '1'),
('58', '7333', 'Dual 150mm Compressed Coil Gun I', '0.2', '1', '1');


-- This table links npclootgrouptype to the ship ID (typeID) that will spawn the loot in the wreck (a separate typeID)
-- Create NPClootGroup table
DROP TABLE IF EXISTS `npcLootGroup`;
CREATE TABLE IF NOT EXISTS `npcLootGroup` (
  `typeID` INT(10) UNSIGNED NOT NULL,
  `lootGroupID` INT(10) UNSIGNED NOT NULL,  
  `dropChance` DECIMAL(6,4) UNSIGNED NOT NULL DEFAULT '0.0',
  KEY `typeID` (`typeID`,`lootGroupID`)
) ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- npclootGroup
INSERT INTO npcLootGroup (typeID,lootGroupID,dropChance) VALUES
-- Guristas Arrogator
('2382', '1', '0.12'),
('2382', '5', '0.08'),
('2382', '19', '0.05'),
('2382', '54', '0.05'),
('2382', '55', '0.04'),
('2382', '64', '0.05'),
('2382', '65', '0.04'),
('2382', '9999', '1');
