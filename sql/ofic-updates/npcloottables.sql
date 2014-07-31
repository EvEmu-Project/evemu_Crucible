--  Creates tables relating to NPC loot.

--  Why use typeID, why not use groupID or marketgroupID for loot?
--  GroupID and marketGroupID link all similar modules/items together. This includes faction or T2 items. A small pirate NPC never drops T2 stuff. This is why typeID is used.

--  Why so many sub-groups?
--  On occasion it is possible to have 2 similar items drop. (Two different missile launchers from the same NPC) so smaller groups relating to turrets have been made. This has also been broken down for tags, narcotics and a few other groups. Also makes it possible for unique NPC's.

--  Mission drops?
--  This would require going through mission reports and linking individual items to individual NPCs. It can be implemented on a mission-by-mission basis later. It is simply too time consuming with too little pay-off at this juncture. A seperate (empty table) has been created for special loot.

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

-- What is 'dungeon' in the NPCSpeciallootGroup table?

-- This is for NPC's that would normally drop nothing or something different under different circumstances (radio tower). Depending on the dungeon ID (relates to mission and DED zones etc. Determines the drop of the typeID.
-- The tables for missions and dungeons haven't been created at this time. This is for future development.

--  For testing purposes the Guristas Arrogator (typeID 2382, groupID 562) has a 100% chance to drop 1 corpse, obviously this will be removed at a later time.

--  ----------------------------
--  Table structure for `npcLootItems`
--  ----------------------------
DROP TABLE IF EXISTS `npcLootItems`;
CREATE TABLE `npcLootItems` (
  `LootGroupID` INT(11) NOT NULL,
  `groupIDDescription` TEXT CHARACTER SET latin1,
  PRIMARY KEY (`LootGroupID`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

INSERT INTO `npcLootItems` (`LootGroupID`, `groupIDDescription`) VALUES

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
('17', 'Defender Light'),
('18', 'Defender Heavy'),
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
('270', 'Caldari Light Scout Drones'),
('271', 'Gallente Light Scout Drones'),
('272', 'Minmatar Light Scout Drones'),
('273', 'Amarr Light Scout Drones'),
-- Medium Scout Drones
('274', 'Caldari Medium Scout Drones'),
('275', 'Gallente Medium Scout Drones'),
('276', 'Minmatar Medium Scout Drones'),
('277', 'Amarr Medium Scout Drones'),
-- Heavy Attack Drones
('278', 'Caldari Heavy Attack Drones'),
('279', 'Gallente Heavy Attack Drones'),
('280', 'Minmatar Heavy Attack Drones'),
('281', 'Amarr Heavy Attack Drones'),
-- Auto-Targeting
('282', 'AT-Light'),
('283', 'AT-Heavy'),
('284', 'AT-Cruise');

-- Blueprints

-- Other entries to be done on an 'as needed' basis


-- ----------------------------
-- Table structure for `npcGroupLootType`
-- ----------------------------
DROP TABLE IF EXISTS `npcLootGroupType`;
CREATE TABLE `npcLootGroupType` (
  `LootGroupID` INT(11) NOT NULL,
  `typeID` INT(11) NOT NULL,
  `typename` TEXT CHARACTER SET latin1,
  `chance` DECIMAL(6,4) UNSIGNED NOT NULL DEFAULT '0.0',
  `minquantity` INT(11) NOT NULL,
  `maxquantity` INT(11) NOT NULL,
  `metalevel` INT(11) NOT NULL,
  KEY `LootGroupID` (`LootGroupID`,`typeID`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

INSERT INTO `npcLootGroupType` (`LootGroupID`, `typeID`, `typename`, `chance`, `minquantity`, `maxquantity`, `metalevel`) VALUES
-- Test drop for devs
('9999', '25', 'Test Corpse', '1', '1', '1', '0'),

-- Metal scraps
('1', '15331', 'Metal Scraps', '1', '1', '2', '0'),
('2', '15331', 'Metal Scraps', '1', '1', '4', '0'),
('3', '15331', 'Metal Scraps', '1', '2', '6', '0'),
('4', '15331', 'Metal Scraps', '1', '3', '15', '0'),

 -- Small Hybrid Charges
('5', '215', 'Iron Charge S', '0.15', '100', '100', '0'),
('5', '216', 'Tungsten Charge S', '0.15', '100', '100', '0'),
('5', '217', 'Iridium Charge S', '0.15', '100', '100', '0'),
('5', '218', 'Lead Charge S', '0.15', '100', '100', '0'),
('5', '219', 'Thorium Charge S', '0.1', '100', '100', '0'),
('5', '220', 'Uranium Charge S', '0.1', '100', '100', '0'),
('5', '221', 'Plutonium Charge S', '0.1', '100', '100', '0'),
('5', '222', 'Antimatter Charge S', '0.1', '100', '100', '0'),

 -- Medium Hybrid Charges
('6', '223', 'Iron Charge M', '0.15', '100', '100', '0'),
('6', '224', 'Tungsten Charge M', '0.15', '100', '100', '0'),
('6', '225', 'Iridium Charge M', '0.15', '100', '100', '0'),
('6', '226', 'Lead Charge M', '0.15', '100', '100', '0'),
('6', '227', 'Thorium Charge M', '0.1', '100', '100', '0'),
('6', '228', 'Uranium Charge M', '0.1', '100', '100', '0'),
('6', '229', 'Plutonium Charge M', '0.1', '100', '100', '0'),
('6', '230', 'Antimatter Charge M', '0.1', '100', '100', '0'),

 -- Large Hybrid Charges
('7', '231', 'Iron Charge L', '0.15', '100', '100', '0'),
('7', '232', 'Tungsten Charge L', '0.15', '100', '100', '0'),
('7', '233', 'Iridium Charge L', '0.15', '100', '100', '0'),
('7', '234', 'Lead Charge L', '0.15', '100', '100', '0'),
('7', '235', 'Thorium Charge L', '0.1', '100', '100', '0'),
('7', '236', 'Uranium Charge L', '0.1', '100', '100', '0'),
('7', '237', 'Plutonium Charge L', '0.1', '100', '100', '0'),
('7', '238', 'Antimatter Charge L', '0.1', '100', '100', '0'),

 -- Small Projectile Ammo
('9', '178', 'Carbonized Lead S', '0.15', '100', '100', '0'),
('9', '179', 'Nuclear S', '0.15', '100', '100', '0'),
('9', '180', 'Proton S', '0.15', '100', '100', '0'),
('9', '181', 'Depleted Uranium S', '0.15', '100', '100', '0'),
('9', '182', 'Titanium Sabot S', '0.1', '100', '100', '0'),
('9', '183', 'Fusion S', '0.1', '100', '100', '0'),
('9', '184', 'Phased Plasma S', '0.1', '100', '100', '0'),
('9', '185', 'EMP S', '0.1', '100', '100', '0'),

 -- Medium Projectile Ammo
('10', '186', 'Carbonized Lead M', '0.15', '100', '100', '0'),
('10', '187', 'Nuclear M', '0.15', '100', '100', '0'),
('10', '188', 'Proton M', '0.15', '100', '100', '0'),
('10', '189', 'Depleted Uranium M', '0.15', '100', '100', '0'),
('10', '190', 'Titanium Sabot M', '0.1', '100', '100', '0'),
('10', '191', 'Fusion M', '0.1', '100', '100', '0'),
('10', '192', 'Phased Plasma M', '0.1', '100', '100', '0'),
('10', '193', 'EMP M', '0.1', '100', '100', '0'),

 -- Large Projectile Ammo
('11', '194', 'Carbonized Lead L', '0.15', '100', '100', '0'),
('11', '195', 'Nuclear L', '0.15', '100', '100', '0'),
('11', '196', 'Proton L', '0.15', '100', '100', '0'),
('11', '197', 'Depleted Uranium L', '0.15', '100', '100', '0'),
('11', '198', 'Titanium Sabot L', '0.1', '100', '100', '0'),
('11', '199', 'Fusion L', '0.1', '100', '100', '0'),
('11', '200', 'Phased Plasma L', '0.1', '100', '100', '0'),
('11', '201', 'EMP L', '0.1', '100', '100', '0'),

('13', '239', 'Radio S', '0.125', '1', '1', '0'),
('13', '240', 'Microwave S', '0.125', '1', '1', '0'),
('13', '241', 'Infrared S', '0.125', '1', '1', '0'),
('13', '242', 'Standard S', '0.125', '1', '1', '0'),
('13', '243', 'Ultraviolet S', '0.125', '1', '1', '0'),
('13', '244', 'Xray S', '0.125', '1', '1', '0'),
('13', '245', 'Gamma S', '0.125', '1', '1', '0'),
('13', '246', 'Multifrequency S', '0.125', '1', '1', '0'),

('14', '247', 'Radio M', '0.125', '1', '1', '0'),
('14', '248', 'Microwave M', '0.125', '1', '1', '0'),
('14', '249', 'Infrared M', '0.125', '1', '1', '0'),
('14', '250', 'Standard M', '0.125', '1', '1', '0'),
('14', '251', 'Ultraviolet M', '0.125', '1', '1', '0'),
('14', '252', 'Xray M', '0.125', '1', '1', '0'),
('14', '253', 'Gamma M', '0.125', '1', '1', '0'),
('14', '254', 'Multifrequency M', '0.125', '1', '1', '0'),

('15', '255', 'Radio L', '0.125', '1', '1', '0'),
('15', '256', 'Microwave L', '0.125', '1', '1', '0'),
('15', '257', 'Infrared L', '0.125', '1', '1', '0'),
('15', '258', 'Standard L', '0.125', '1', '1', '0'),
('15', '259', 'Ultraviolet L', '0.125', '1', '1', '0'),
('15', '260', 'Xray L', '0.125', '1', '1', '0'),
('15', '261', 'Gamma L', '0.125', '1', '1', '0'),
('15', '262', 'Multifrequency L', '0.125', '1', '1', '0'),

('18', '265', 'Heavy Defender I', '1', '100', '100', '0'),
('17', '32782', 'Light Defender I', '1', '100', '100', '0'),

('19', '2514', 'Inferno Rocket', '0.25', '100', '100', '0'),
('19', '2512', 'Mjolnir Rocket', '0.25', '100', '100', '0'),
('19', '2516', 'Nova Rocket', '0.25', '100', '100', '0'),
('19', '266', 'Scourge Rocket', '0.25', '100', '100', '0'),

('20', '211', 'Inferno Light Missile', '0.25', '100', '100', '0'),
('20', '212', 'Mjolnir Light Missile', '0.25', '100', '100', '0'),
('20', '213', 'Nova Light Missile', '0.25', '100', '100', '0'),
('20', '210', 'Scourge Light Missile', '0.25', '100', '100', '0'),

('21', '208', 'Inferno Heavy Missile', '0.25', '100', '100', '0'),
('21', '207', 'Mjolnir Heavy Missile', '0.25', '100', '100', '0'),
('21', '206', 'Nova Heavy Missile', '0.25', '100', '100', '0'),
('21', '209', 'Scourge Heavy Missile', '0.25', '100', '100', '0'),

('22', '20308', 'Inferno Heavy Assault Missile', '0.25', '100', '100', '0'),
('22', '20306', 'Mjolnir Heavy Assault Missile', '0.25', '100', '100', '0'),
('22', '21867', 'Nova Heavy Assault Missile', '0.25', '100', '100', '0'),
('22', '20307', 'Scourge Heavy Assault Missile', '0.25', '100', '100', '0'),

('23', '204', 'Inferno Cruise Missile', '0.25', '100', '100', '0'),
('23', '202', 'Mjolnir Cruise Missile', '0.25', '100', '100', '0'),
('23', '205', 'Nova Cruise Missile', '0.25', '100', '100', '0'),
('23', '203', 'Scourge Cruise Missile', '0.25', '100', '100', '0'),

('24', '2510', 'Inferno Torpedo', '0.25', '100', '100', '0'),
('24', '2506', 'Mjolnir Torpedo', '0.25', '100', '100', '0'),
('24', '2508', 'Nova Torpedo', '0.25', '100', '100', '0'),
('24', '267', 'Scourge Torpedo', '0.25', '100', '100', '0'),

('25', '263', 'Cap Booster 25', '0.5', '1', '1', '0'),
('25', '264', 'Cap Booster 50', '0.5', '1', '1', '0'),
('26', '3552', 'Cap Booster 75', '0.5', '1', '1', '0'),
('26', '3554', 'Cap Booster 100', '0.5', '1', '1', '0'),
('27', '11283', 'Cap Booster 150', '0.5', '1', '1', '0'),
('27', '11285', 'Cap Booster 200', '0.5', '1', '1', '0'),
('28', '11287', 'Cap Booster 400', '0.5', '1', '1', '0'),
('28', '11289', 'Cap Booster 800', '0.5', '1', '1', '0'),

 -- Small Railguns
('54', '561', '75mm Gatling Rail I', '0.2', '1', '1', '0'),
('54', '7247', '75mm Prototype Gauss Gun', '0.2', '1', '1', '4'),
('54', '7249', '75mm Scout Accelerator Cannon', '0.2', '1', '1', '2'),
('54', '7251', '75mm Carbide Railgun I', '0.2', '1', '1', '1'),
('54', '7253', '75mm Compressed Coil Gun I', '0.2', '1', '1', '3'),

('55', '10678', '125mm Railgun I', '0.2', '1', '1', '0'),
('55', '10688', '125mm Scout Accelerator Cannon', '0.2', '1', '1', '2'),
('55', '10690', '125mm Carbide Railgun I', '0.2', '1', '1', '1'),
('55', '10692', '125mm Compressed Coil Gun I', '0.2', '1', '1', '3'),
('55', '10694', '125mm Prototype Gauss Gun', '0.2', '1', '1', '4'),

('56', '565', '150mm Railgun I', '0.2', '1', '1', '0'),
('56', '7287', '150mm Prototype Gauss Gun', '0.2', '1', '1', '4'),
('56', '7289', '150mm Scout Accelerator Cannon', '0.2', '1', '1', '2'),
('56', '7291', '150mm Carbide Railgun I', '0.2', '1', '1', '1'),
('56', '7293', '150mm Compressed Coil Gun I', '0.2', '1', '1', '3'),

('58', '567', 'Dual 150mm Railgun I', '0.3', '1', '1', '0'),
('58', '7331', 'Dual 150mm Carbide Railgun I', '0.175', '1', '1', '1'),
('58', '7329', 'Dual 150mm Scout Accelerator Cannon', '0.175', '1', '1', '2'),
('58', '7333', 'Dual 150mm Compressed Coil Gun I', '0.175', '1', '1', '3'),
('58', '7327', 'Dual 150mm Prototype Gauss Gun', '0.175', '1', '1', '4'),

('59', '12344', '200mm Railgun I', '0.3', '1', '1', '0'),
('59', '14272', '200mm Carbide Railgun I', '0.175', '1', '1', '1'),
('59', '14274', '200mm Scout Accelerator Cannon', '0.175', '1', '1', '2'),
('59', '14276', '200mm Compressed Coil Gun I', '0.175', '1', '1', '3'),
('59', '14278', '200mm Prototype Gauss Gun', '0.175', '1', '1', '4'),

('60', '570', '250mm Railgun I', '0.3', '1', '1', '0'),
('60', '7369', '250mm Scout Accelerator Cannon', '0.175', '1', '1', '1'),
('60', '7371', '250mm Carbide Railgun I', '0.175', '1', '1', '2'),
('60', '7373', '250mm Compressed Coil Gun I', '0.175', '1', '1', '3'),
('60', '7367', '250mm Prototype Gauss Gun', '0.175', '1', '1', '4'),

('61', '572', 'Dual 250mm Railgun I', '0.3', '1', '1', '0'),
('61', '7411', 'Dual 250mm Carbide Railgun I', '0.175', '1', '1', '1'),
('61', '7409', 'Dual 250mm Scout Accelerator Cannon', '0.175', '1', '1', '2'),
('61', '7413', 'Dual 250mm Compressed Coil Gun I', '0.175', '1', '1', '3'),
('61', '7407', 'Dual 250mm Prototype Gauss Gun', '0.175', '1', '1', '4'),

('62', '12354', '350mm Railgun I', '0.3', '1', '1', '0'),
('62', '14280', '350mm Carbide Railgun I', '0.175', '1', '1', '1'),
('62', '14282', '350mm Scout Accelerator Cannon', '0.175', '1', '1', '2'),
('62', '14284', '350mm Compressed Coil Gun I', '0.175', '1', '1', '3'),
('62', '14286', '350mm Prototype Gauss Gun', '0.175', '1', '1', '4'),

('63', '574', '425mm Railgun I', '0.3', '1', '1', '0'),
('63', '7451', '425mm Carbide Railgun I', '0.175', '1', '1', '1'),
('63', '7449', '425mm Scout Accelerator Cannon', '0.175', '1', '1', '2'),
('63', '7453', '425mm Compressed Coil Gun I', '0.175', '1', '1', '3'),
('63', '7447', '425mm Prototype Gauss Gun', '0.175', '1', '1', '4'),

('66', '562', 'Light Electron Blaster I', '0.3', '1', '1', '0'),
('66', '7491', 'Regulated Light Electron Phase Cannon I', '0.175', '1', '1', '1'),
('66', '7489', 'Limited Light Electron Blaster I', '0.175', '1', '1', '2'),
('66', '7493', 'Anode Light Electron Particle Cannon I', '0.175', '1', '1', '3'),
('66', '7487', 'Modal Light Electron Particle Accelerator I', '0.175', '1', '1', '4'),

('64', '563', 'Light Ion Blaster I', '0.3', '1', '1', '0'),
('64', '7539', 'Regulated Light Ion Phase Cannon I', '0.175', '1', '1', '1'),
('64', '7537', 'Limited Light Ion Blaster I', '0.175', '1', '1', '2'),
('64', '7541', 'Anode Light Ion Particle Cannon I', '0.175', '1', '1', '3'),
('64', '7535', 'Modal Light Ion Particle Accelerator I', '0.175', '1', '1', '4'),

('65', '564', 'Light Neutron Blaster I', '0.3', '1', '1', '0'),
('65', '7583', 'Regulated Light Neutron Phase Cannon I', '0.175', '1', '1', '1'),
('65', '7581', 'Limited Light Neutron Blaster I', '0.175', '1', '1', '2'),
('65', '7585', 'Anode Light Neutron Particle Cannon I', '0.175', '1', '1', '3'),
('65', '7579', 'Modal Light Neutron Particle Accelerator I', '0.175', '1', '1', '4'),

('67', '569', 'Heavy Ion Blaster I', '0.4', '1', '1', '0'),
('67', '7665', 'Limited Ion Blaster I', '0.3', '1', '1', '2'),
('67', '7669', 'Anode Ion Particle Cannon I', '0.3', '1', '1', '3'),

('68', '568', 'Heavy Neutron Blaster I', '0.4', '1', '1', '0'),
('68', '7621', 'Limited Electron Blaster I', '0.3', '1', '1', '2'),
('68', '7709', 'Anode Neutron Particle Cannon I', '0.3', '1', '1', '3'),

('69', '566', 'Heavy Electron Blaster I', '0.6', '1', '1', '0'),
('69', '7625', 'Anode Electron Particle Cannon I', '0.4', '1', '1', '3'),

('72', '571', 'Electron Blaster Cannon I', '0.3', '1', '1', '0'),
('72', '7747', 'Regulated Mega Electron Phase Cannon I', '0.175', '1', '1', '1'),
('72', '7745', 'Limited Electron Blaster Cannon I', '0.175', '1', '1', '2'),
('72', '7749', 'Anode Mega Electron Particle Cannon I', '0.175', '1', '1', '3'),
('72', '7743', 'Modal Mega Electron Particle Accelerator I', '0.175', '1', '1', '4'),

('70', '575', 'Ion Blaster Cannon I', '0.3', '1', '1', '0'),
('70', '7831', 'Regulated Mega Ion Phase Cannon I', '0.175', '1', '1', '1'),
('70', '7829', 'Limited Mega Ion Blaster I', '0.175', '1', '1', '2'),
('70', '7833', 'Anode Mega Ion Particle Cannon I', '0.175', '1', '1', '3'),
('70', '7827', 'Modal Mega Ion Particle Accelerator I', '0.175', '1', '1', '4'),

('71', '573', 'Neutron Blaster Cannon I', '0.3', '1', '1', '0'),
('71', '7787', 'Regulated Mega Neutron Phase Cannon I', '0.175', '1', '1', '1'),
('71', '7785', 'Limited Mega Neutron Blaster I', '0.175', '1', '1', '2'),
('71', '7789', 'Anode Mega Neutron Particle Cannon I', '0.175', '1', '1', '3'),
('71', '7783', 'Modal Mega Neutron Particle Accelerator I', '0.175', '1', '1', '4'),

('74', '484', '125mm Gatling AutoCannon I', '0.3', '1', '1', '0'),
('74', '8785', '125mm Light Carbine Repeating Cannon I', '0.175', '1', '1', '1'),
('74', '8787', '125mm Light Gallium Machine Gun', '0.175', '1', '1', '2'),
('74', '8789', '125mm Light Prototype Automatic Cannon', '0.175', '1', '1', '3'),
('74', '8759', '125mm Light Scout Autocannon I', '0.175', '1', '1', '4'),

('75', '485', '150mm Light AutoCannon I', '0.3', '1', '1', '0'),
('75', '8817', '150mm Light Carbine Repeating Cannon I', '0.175', '1', '1', '1'),
('75', '8819', '150mm Light Gallium Machine Gun', '0.175', '1', '1', '2'),
('75', '8821', '150mm Light Prototype Automatic Cannon', '0.175', '1', '1', '3'),
('75', '8815', '150mm Light Scout Autocannon I', '0.175', '1', '1', '4'),

('76', '486', '200mm AutoCannon I', '0.3', '1', '1', '0'),
('76', '8865', '200mm Light Carbine Repeating Cannon I', '0.175', '1', '1', '1'),
('76', '8867', '200mm Light Gallium Machine Gun', '0.175', '1', '1', '2'),
('76', '8869', '200mm Light Prototype Automatic Cannon', '0.175', '1', '1', '3'),
('76', '8863', '200mm Light Scout Autocannon I', '0.175', '1', '1', '4'),

('77', '489', 'Dual 180mm AutoCannon I', '0.3', '1', '1', '0'),
('77', '9073', 'Dual 180mm Carbine Repeating Cannon I', '0.175', '1', '1', '1'),
('77', '9091', 'Dual 180mm Gallium Machine Gun', '0.175', '1', '1', '2'),
('77', '9093', 'Dual 180mm Prototype Automatic Cannon', '0.175', '1', '1', '3'),
('77', '9071', 'Dual 180mm Scout Autocannon I', '0.175', '1', '1', '4'),

('78', '490', '220mm Vulcan AutoCannon I', '0.3', '1', '1', '0'),
('78', '9129', '220mm Medium Carbine Repeating Cannon I', '0.175', '1', '1', '1'),
('78', '9131', '220mm Medium Gallium Machine Gun', '0.175', '1', '1', '2'),
('78', '9133', '220mm Medium Prototype Automatic Cannon', '0.175', '1', '1', '3'),
('78', '9127', '220mm Medium Scout Autocannon I', '0.175', '1', '1', '4'),

('79', '491', '425mm AutoCannon I', '0.3', '1', '1', '0'),
('79', '9137', '425mm Medium Carbine Repeating Cannon I', '0.175', '1', '1', '1'),
('79', '9139', '425mm Medium Gallium Machine Gun', '0.175', '1', '1', '2'),
('79', '9141', '425mm Medium Prototype Automatic Cannon', '0.175', '1', '1', '3'),
('79', '9135', '425mm Medium Scout Autocannon I', '0.175', '1', '1', '4'),

('80', '494', 'Dual 425mm AutoCannon I', '0.3', '1', '1', '0'),
('80', '9249', 'Dual 425mm Carbine Repeating Cannon I', '0.175', '1', '1', '1'),
('80', '9251', 'Dual 425mm Gallium Machine Gun', '0.175', '1', '1', '2'),
('80', '9253', 'Dual 425mm Prototype Automatic Cannon', '0.175', '1', '1', '3'),
('80', '9247', 'Dual 425mm Scout Autocannon I', '0.175', '1', '1', '4'),

('81', '495', 'Dual 650mm Repeating Cannon I', '0.3', '1', '1', '0'),
('81', '9289', 'Dual 650mm Carbine Repeating Cannon I', '0.175', '1', '1', '1'),
('81', '9291', 'Dual 650mm Gallium Repeating Cannon', '0.175', '1', '1', '2'),
('81', '9293', 'Dual 650mm Prototype Automatic Cannon', '0.175', '1', '1', '3'),
('81', '9287', 'Dual 650mm Scout Repeating Cannon I', '0.175', '1', '1', '4'),

('82', '496', '800mm Repeating Cannon I', '0.3', '1', '1', '0'),
('82', '9329', '800mm Heavy Carbine Repeating Cannon I', '0.175', '1', '1', '1'),
('82', '9331', '800mm Heavy Gallium Repeating Cannon', '0.175', '1', '1', '2'),
('82', '9333', '800mm Heavy Prototype Automatic Cannon', '0.175', '1', '1', '3'),
('82', '9327', '800mm Heavy Scout Repeating Cannon I', '0.175', '1', '1', '4'),

('83', '487', '250mm Light Artillery Cannon I', '0.3', '1', '1', '0'),
('83', '8905', '250mm Light Carbine Howitzer I', '0.175', '1', '1', '1'),
('83', '8907', '250mm Light Gallium Cannon', '0.175', '1', '1', '2'),
('83', '8909', '250mm Light Prototype Siege Cannon', '0.175', '1', '1', '3'),
('83', '8903', '250mm Light Scout Artillery I', '0.175', '1', '1', '4'),

('84', '488', '280mm Howitzer Artillery I', '0.3', '1', '1', '0'),
('84', '9413', '280mm Carbine Howitzer I', '0.175', '1', '1', '1'),
('84', '9415', '280mm Gallium Cannon', '0.175', '1', '1', '2'),
('84', '9417', '280mm Prototype Siege Cannon', '0.175', '1', '1', '3'),
('84', '9411', '280mm Scout Artillery I', '0.175', '1', '1', '4'),

('85', '492', '650mm Artillery Cannon I', '0.3', '1', '1', '0'),
('85', '9209', '650mm Medium Carbine Howitzer I', '0.175', '1', '1', '1'),
('85', '9211', '650mm Medium Gallium Cannon', '0.175', '1', '1', '2'),
('85', '9213', '650mm Medium Prototype Siege Cannon', '0.175', '1', '1', '3'),
('85', '9207', '650mm Medium Scout Artillery I', '0.175', '1', '1', '4'),

('86', '493', '720mm Howitzer Artillery I', '0.3', '1', '1', '0'),
('86', '9453', '720mm Carbine Howitzer I', '0.175', '1', '1', '1'),
('86', '9455', '720mm Gallium Cannon', '0.175', '1', '1', '2'),
('86', '9457', '720mm Prototype Siege Cannon', '0.175', '1', '1', '3'),
('86', '9451', '720mm Scout Artillery I', '0.175', '1', '1', '4'),

('87', '497', '1200mm Artillery Cannon I', '0.4', '1', '1', '0'),
('87', '9369', '1200mm Heavy Carbine Howitzer I', '0.2', '1', '1', '1'),
('87', '9371', '1200mm Heavy Gallium Cannon', '0.2', '1', '1', '2'),
('87', '9377', '1200mm Heavy Prototype Siege Cannon', '0.2', '1', '1', '3'),

('88', '498', '1400mm Howitzer Artillery I', '0.3', '1', '1', '0'),
('88', '9493', '1400mm Carbine Howitzer I', '0.175', '1', '1', '1'),
('88', '9495', '1400mm Gallium Cannon', '0.175', '1', '1', '2'),
('88', '9497', '1400mm Prototype Siege Cannon', '0.175', '1', '1', '3'),
('88', '9491', '1400mm Scout Artillery I', '0.175', '1', '1', '4'),

('89', '21559', '1200mm Heavy Jolt Artillery I', '0.4', '1', '1', '0'),
('89', '9373', '1200mm Heavy Prototype Artillery', '0.3', '1', '1', '3'),
('89', '9367', '1200mm Heavy Scout Artillery I', '0.3', '1', '1', '4'),

('90', '452', 'Dual Light Beam Laser I', '0.3', '1', '1', '0'),
('90', '6633', 'Dual Afocal Light Maser I', '0.175', '1', '1', '1'),
('90', '6631', 'Dual Modal Light Laser I', '0.175', '1', '1', '2'),
('90', '6637', 'Dual Anode Light Particle Stream I', '0.175', '1', '1', '3'),
('90', '6635', 'Dual Modulated Light Energy Beam I', '0.175', '1', '1', '4'),

('91', '454', 'Small Focused Beam Laser I', '0.3', '1', '1', '0'),
('91', '6717', 'Small Focused Afocal Maser I', '0.175', '1', '1', '1'),
('91', '6715', 'Small Focused Modal Laser I', '0.175', '1', '1', '2'),
('91', '6721', 'Small Focused Anode Particle Stream I', '0.175', '1', '1', '3'),
('91', '6719', 'Small Focused Modulated Energy Beam I', '0.175', '1', '1', '4'),

('93', '457', 'Focused Medium Beam Laser I', '0.3', '1', '1', '0'),
('93', '6861', 'Focused Afocal Medium Maser I', '0.175', '1', '1', '1'),
('93', '6859', 'Focused Modal Medium Laser I', '0.175', '1', '1', '2'),
('93', '6865', 'Focused Anode Medium Particle Stream I', '0.175', '1', '1', '3'),
('93', '6863', 'Focused Modulated Medium Energy Beam I', '0.175', '1', '1', '4'),

('92', '459', 'Heavy Beam Laser I', '0.3', '1', '1', '0'),
('92', '6961', 'Heavy Afocal Maser I', '0.175', '1', '1', '1'),
('92', '6959', 'Heavy Modal Laser I', '0.175', '1', '1', '2'),
('92', '6965', 'Heavy Anode Particle Stream I', '0.175', '1', '1', '3'),
('92', '6963', 'Heavy Modulated Energy Beam I', '0.175', '1', '1', '4'),

('94', '455', 'Quad Light Beam Laser I', '0.3', '1', '1', '0'),
('94', '6759', 'Quad Afocal Light Maser I', '0.175', '1', '1', '1'),
('94', '6757', 'Quad Modal Light Laser I', '0.175', '1', '1', '2'),
('94', '6763', 'Quad Anode Light Particle Stream I', '0.175', '1', '1', '3'),
('94', '6761', 'Quad Modulated Light Energy Beam I', '0.175', '1', '1', '4'),

('95', '461', 'Dual Heavy Beam Laser I', '0.3', '1', '1', '0'),
('95', '7045', 'Dual Afocal Heavy Maser I', '0.175', '1', '1', '1'),
('95', '7043', 'Dual Modal Heavy Laser I', '0.175', '1', '1', '2'),
('95', '7049', 'Dual Anode Heavy Particle Stream I', '0.175', '1', '1', '3'),
('95', '7047', 'Dual Modulated Heavy Energy Beam I', '0.175', '1', '1', '4'),

('96', '463', 'Mega Beam Laser I', '0.3', '1', '1', '0'),
('96', '7125', 'Mega Afocal Maser I', '0.175', '1', '1', '1'),
('96', '7123', 'Mega Modal Laser I', '0.175', '1', '1', '2'),
('96', '7131', 'Mega Anode Particle Stream I', '0.175', '1', '1', '3'),
('96', '7127', 'Mega Modulated Energy Beam I', '0.175', '1', '1', '4'),

('97', '464', 'Tachyon Beam Laser I', '0.3', '1', '1', '0'),
('97', '7169', 'Tachyon Afocal Maser I', '0.175', '1', '1', '1'),
('97', '7167', 'Tachyon Modal Laser I', '0.175', '1', '1', '2'),
('97', '7173', 'Tachyon Anode Particle Stream I', '0.175', '1', '1', '3'),
('97', '7171', 'Tachyon Modulated Energy Beam I', '0.175', '1', '1', '4'),

('99', '453', 'Small Focused Pulse Laser I', '0.3', '1', '1', '0'),
('99', '6673', 'Small Focused Afocal Pulse Maser I', '0.175', '1', '1', '1'),
('99', '6671', 'Small Focused Modal Pulse Laser I', '0.175', '1', '1', '2'),
('99', '6677', 'Small Focused Anode Pulse Particle Stream I', '0.175', '1', '1', '3'),
('99', '6675', 'Small Focused Modulated Pulse Energy Beam I', '0.175', '1', '1', '4'),

('100', '450', 'Gatling Pulse Laser I', '0.3', '1', '1', '0'),
('100', '5177', 'Gatling Afocal Maser I', '0.175', '1', '1', '1'),
('100', '5175', 'Gatling Modal Laser I', '0.175', '1', '1', '2'),
('100', '5181', 'Gatling Anode Particle Stream I', '0.175', '1', '1', '3'),
('100', '5179', 'Gatling Modulated Energy Beam I', '0.175', '1', '1', '4'),

('101', '451', 'Dual Light Pulse Laser I', '0.3', '1', '1', '0'),
('101', '5217', 'Dual Afocal Pulse Maser I', '0.175', '1', '1', '1'),
('101', '5215', 'Dual Modal Pulse Laser I', '0.175', '1', '1', '2'),
('101', '5221', 'Dual Anode Pulse Particle Stream I', '0.175', '1', '1', '3'),
('101', '5219', 'Dual Modulated Pulse Energy Beam I', '0.175', '1', '1', '4'),

('102', '458', 'Heavy Pulse Laser I', '0.3', '1', '1', '0'),
('102', '6921', 'Heavy Afocal Pulse Maser I', '0.175', '1', '1', '1'),
('102', '6919', 'Heavy Modal Pulse Laser I', '0.175', '1', '1', '2'),
('102', '6925', 'Heavy Anode Pulse Particle Stream I', '0.175', '1', '1', '3'),
('102', '6923', 'Heavy Modulated Pulse Energy Beam I', '0.175', '1', '1', '4'),

('103', '456', 'Focused Medium Pulse Laser I', '0.3', '1', '1', '0'),
('103', '6807', 'Focused Afocal Pulse Maser I', '0.175', '1', '1', '1'),
('103', '6805', 'Focused Modal Pulse Laser I', '0.175', '1', '1', '2'),
('103', '6811', 'Focused Anode Pulse Particle Stream I', '0.175', '1', '1', '3'),
('103', '6809', 'Focused Modulated Pulse Energy Beam I', '0.175', '1', '1', '4'),

('104', '460', 'Dual Heavy Pulse Laser I', '0.3', '1', '1', '0'),
('104', '7001', 'Dual Heavy Afocal Pulse Maser I', '0.175', '1', '1', '1'),
('104', '6999', 'Dual Heavy Modal Pulse Laser I', '0.175', '1', '1', '2'),
('104', '7005', 'Dual Heavy Anode Pulse Particle Stream I', '0.175', '1', '1', '3'),
('104', '7003', 'Dual Heavy Modulated Pulse Energy Beam I', '0.175', '1', '1', '4'),

('105', '462', 'Mega Pulse Laser I', '0.3', '1', '1', '0'),
('105', '7085', 'Mega Afocal Pulse Maser I', '0.175', '1', '1', '1'),
('105', '7083', 'Mega Modal Pulse Laser I', '0.175', '1', '1', '2'),
('105', '7089', 'Mega Anode Pulse Particle Stream I', '0.175', '1', '1', '3'),
('105', '7087', 'Mega Modulated Pulse Energy Beam I', '0.175', '1', '1', '4'),
('105', '23846', 'Halberd Mega Pulse Laser I', '0.175', '1', '1', '0'),

('106', '10629', 'Rocket Launcher I', '0.3', '1', '1', '0'),
('106', '16521', 'Malkuth Rocket Launcher I', '0.175', '1', '1', '1'),
('106', '16523', 'Limos Rocket Launcher I', '0.175', '1', '1', '2'),
('106', '16525', 'OE-5200 Rocket Launcher', '0.175', '1', '1', '3'),
('106', '16527', 'Arbalest Rocket Launcher I', '0.175', '1', '1', '4'),

('107', '499', 'Light Missile Launcher I', '0.3', '1', '1', '0'),
('107', '8089', 'Upgraded Malkuth Light Missile Launcher', '0.175', '1', '1', '1'),
('107', '8091', 'Limited Limos Light Missile Launcher', '0.175', '1', '1', '2'),
('107', '7993', 'Experimental TE-2100 Light Missile Launcher', '0.175', '1', '1', '3'),
('107', '8093', 'Prototype Arbalest Light Missile Launcher', '0.175', '1', '1', '4'),

('108', '1875', 'Rapid Light Missile Launcher I', '0.3', '1', '1', '0'),
('108', '8023', 'Upgraded Malkuth Rapid Light Missile Launcher', '0.175', '1', '1', '1'),
('108', '8025', 'Limited Limos Rapid Light Missile Launcher', '0.175', '1', '1', '2'),
('108', '8007', 'Experimental SV-2000 Rapid Light Missile Launcher', '0.175', '1', '1', '3'),
('108', '8027', 'Prototype Arbalest Rapid Light Missile Launcher', '0.175', '1', '1', '4'),

('109', '501', 'Heavy Missile Launcher I', '0.3', '1', '1', '0'),
('109', '8101', 'Malkuth Heavy Missile Launcher I', '0.175', '1', '1', '1'),
('109', '8103', 'Advanced Limos Heavy Missile Bay I', '0.175', '1', '1', '2'),
('109', '7997', 'XR-3200 Heavy Missile Bay', '0.175', '1', '1', '3'),
('109', '8105', 'Arbalest Heavy Missile Launcher', '0.175', '1', '1', '4'),

('110', '13320', 'Cruise Missile Launcher I', '0.3', '1', '1', '0'),
('110', '16513', 'Malkuth Cruise Launcher I', '0.175', '1', '1', '1'),
('110', '16515', 'Limos Cruise Launcher I', '0.175', '1', '1', '2'),
('110', '16517', 'XT-9000 Cruise Launcher', '0.175', '1', '1', '3'),
('110', '16519', 'Arbalest Cruise Launcher I', '0.175', '1', '1', '4'),

('111', '503', 'Torpedo Launcher I', '0.3', '1', '1', '0'),
('111', '8113', 'Upgraded Malkuth Torpedo Launcher', '0.175', '1', '1', '1'),
('111', '8115', 'Limited Limos Torpedo Launcher', '0.175', '1', '1', '2'),
('111', '8001', 'Experimental ZW-4100 Torpedo Launcher', '0.175', '1', '1', '3'),
('111', '8117', 'Prototype Arbalest Torpedo Launcher', '0.175', '1', '1', '4'),

('112', '32444', 'Citadel Cruise Launcher I', '0.4', '1', '1', '0'),
('112', '20539', 'Citadel Torpedo Launcher I', '0.2', '1', '1', '0'),
('112', '3563', 'Limos Citadel Cruise Launcher I', '0.2', '1', '1', '2'),
('112', '3565', 'Shock Limos Citadel Torpedo Bay I', '0.2', '1', '1', '2'),

('113', '20138', 'Heavy Assault Missile Launcher I', '0.3', '1', '1', '0'),
('113', '25707', 'Arbalest Heavy Assault Missile Launcher I', '0.175', '1', '1', '4'),
('113', '25709', 'Malkuth Heavy Assault Missile Launcher I', '0.175', '1', '1', '1'),
('113', '25711', 'Compact Limos Heavy Assault Missile Launcher I', '0.175', '1', '1', '2'),
('113', '25713', 'XT-2800 Heavy Assault Missile Launcher I', '0.175', '1', '1', '3'),

('114', '33441', 'Limos Rapid Heavy Missile Launcher I', '0.12', '1', '1', '0'),
('114', '33448', 'Rapid Heavy Missile Launcher I', '0.3', '1', '1', '0'),
('114', '33461', 'Shaqils Modified Rapid Heavy Missile Launcher', '0.12', '1', '1', '0'),
('114', '33442', 'Malkuth Rapid Heavy Missile Launcher I', '0.12', '1', '1', '1'),
('114', '33466', 'YO-5000 Rapid Heavy Missile Launcher', '0.12', '1', '1', '3'),
('114', '33440', 'Arbalest Rapid Heavy Missile Launcher I', '0.12', '1', '1', '4');


-- This table links npclootgrouptype to the ship ID (groupID) that will spawn the loot in the wreck (a separate entity)
-- Create NPClootGroup table
DROP TABLE IF EXISTS `npcLootGroup`;
CREATE TABLE IF NOT EXISTS `npcLootGroup` (
  `groupID` INT(10) UNSIGNED NOT NULL,
  `lootGroupID` INT(10) UNSIGNED NOT NULL,  
  `dropChance` DECIMAL(6,4) UNSIGNED NOT NULL DEFAULT '0.0',
  KEY `groupID` (`groupID`,`lootGroupID`)
) ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- npclootGroup
INSERT INTO npcLootGroup (groupID,lootGroupID,dropChance) VALUES
-- Asteroid Guristas Frigate 
('562', '1', '0.12'),
('562', '5', '0.08'),
('562', '19', '0.05'),
('562', '54', '0.05'),
('562', '55', '0.04'),
('562', '64', '0.05'),
('562', '65', '0.04'),
('562', '9999', '1'), -- test drop to be removed later

-- Asteroid Guristas Destroyer  
('579', '1', '0.12'),
('579', '5', '0.08'),
('579', '19', '0.05'),
('579', '54', '0.05'),
('579', '55', '0.04'),
('579', '64', '0.05'),
('579', '65', '0.04'),
('579', '9999', '1'); -- test drop to be removed later

-- This table links npclootgrouptype to the ship ID (typeID) that will spawn the loot in the wreck (a separate typeID) used for special occasions
-- Create NPCSpeciallootGroup table
DROP TABLE IF EXISTS `npcSpecialLootGroup`;
CREATE TABLE IF NOT EXISTS `npcSpecialLootGroup` (
  `typeID` INT(10) UNSIGNED NOT NULL,
  `lootGroupID` INT(10) UNSIGNED NOT NULL,  
  `dropChance` DECIMAL(6,4) UNSIGNED NOT NULL DEFAULT '0.0',
  `dungeon` INT(10) UNSIGNED  NULL, 
  KEY `typeID` (`typeID`,`lootGroupID`)
) ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

/* -- npcSpeciallootGroup
INSERT INTO npcSpecialLootGroup (typeID,LootGroupID,dropChance,dungeon) VALUES
-- Asteroid Guristas Frigate 
('238', '1', '0.12', '0'),
('562', '5', '0.08', '0'),
('562', '19', '0.05', '0'),
('562', '54', '0.05', '0'),
('562', '55', '0.04', '0'),
('562', '64', '0.05', '0'),
('562', '65', '0.04', '0'),
('562', '9999', '1', '0'); */
