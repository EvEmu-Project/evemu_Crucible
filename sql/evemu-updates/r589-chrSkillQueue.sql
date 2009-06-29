ALTER TABLE `chrSkillQueue`
 CHANGE `id` `orderIndex` INT(10) UNSIGNED NOT NULL AFTER `characterID`,
 CHANGE `itemID` `characterID` INT(10) UNSIGNED NOT NULL,
 DROP KEY `id`;
