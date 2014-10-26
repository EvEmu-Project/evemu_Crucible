-- ----------------------------
-- Table structure for `salvageGroupLootType`
-- ----------------------------
DROP TABLE IF EXISTS `salvageGroupLootType`;
CREATE TABLE `salvageGroupLootType` (
  `salvagegroupLootID` INT(11) NOT NULL,
  `typeID` INT(11) NOT NULL,
  `typename` TEXT CHARACTER SET latin1,
  `chance` DOUBLE NULL,
  `minquantity` INT(11) NOT NULL,
  `maxquantity` INT(11) NOT NULL,
  KEY `salvagegroupLootID` (`salvagegroupLootID`,`typeID`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

INSERT INTO `salvageGroupLootType` (`salvagegroupLootID`, `typeID`, `typename`, `chance`, `minquantity`, `maxquantity`) VALUES
-- Test salvage for devs
('1', '15331', 'Metal Scraps S', '1', '1', '2');  -- 100% chance of this on **successful** salvage attempt


-- This table links salvagelootgrouptype to the shipwreck ID (typeID) that will spawn the salvage in the wreck (a separate typeID)
-- Create salvagelootGroup table
DROP TABLE IF EXISTS `salvageLootGroup`;
CREATE TABLE IF NOT EXISTS `salvageLootGroup` (
  `typeID` INT(10) UNSIGNED NOT NULL,
  `salvagelootGroupID` INT(10) UNSIGNED NOT NULL,  
  `dropChance` DECIMAL(6,4) UNSIGNED NOT NULL DEFAULT '0.0',
  KEY `typeID` (`typeID`,`salvagelootGroupID`)
) ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- salvagelootGroup
INSERT INTO salvageLootGroup (typeID,salvagelootGroupID,dropChance) VALUES
-- Small Guristas wreck
('26573', '1', '1.00'); -- 100% chance of salvage on **successful** salvage attempt