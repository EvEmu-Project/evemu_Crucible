ALTER TABLE `corporation` MODIFY `deleted` tinyint(4) NOT NULL;
ALTER TABLE `corporation` ADD `walletDivision1` varchar(100) DEFAULT '1st wallet division';
ALTER TABLE `corporation` ADD `walletDivision2` varchar(100) DEFAULT '2nd wallet division';
ALTER TABLE `corporation` ADD `walletDivision3` varchar(100) DEFAULT '3rd wallet division';
ALTER TABLE `corporation` ADD `walletDivision4` varchar(100) DEFAULT '4th wallet division';
ALTER TABLE `corporation` ADD `walletDivision5` varchar(100) DEFAULT '5th wallet division';
ALTER TABLE `corporation` ADD `walletDivision6` varchar(100) DEFAULT '6th wallet division';
ALTER TABLE `corporation` ADD `walletDivision7` varchar(100) DEFAULT '7th wallet division';
ALTER TABLE `corporation` ADD `warFactionID` int(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `corporationStatic` ADD `walletDivision1` varchar(100) DEFAULT '1st wallet division';
ALTER TABLE `corporationStatic` ADD `walletDivision2` varchar(100) DEFAULT '2nd wallet division';
ALTER TABLE `corporationStatic` ADD `walletDivision3` varchar(100) DEFAULT '3rd wallet division';
ALTER TABLE `corporationStatic` ADD `walletDivision4` varchar(100) DEFAULT '4th wallet division';
ALTER TABLE `corporationStatic` ADD `walletDivision5` varchar(100) DEFAULT '5th wallet division';
ALTER TABLE `corporationStatic` ADD `walletDivision6` varchar(100) DEFAULT '6th wallet division';
ALTER TABLE `corporationStatic` ADD `walletDivision7` varchar(100) DEFAULT '7th wallet division';
ALTER TABLE `corporationStatic` ADD `warFactionID` int(10) unsigned NOT NULL DEFAULT '0';

