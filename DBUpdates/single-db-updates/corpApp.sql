CREATE TABLE `chrApplications` (
`corporationID` INT UNSIGNED NOT NULL ,
`characterID` INT UNSIGNED NOT NULL ,
`applicationText` TEXT NOT NULL ,
`roles` BIGINT UNSIGNED NOT NULL ,
`grantableRoles` BIGINT UNSIGNED NOT NULL ,
`status` INT UNSIGNED NOT NULL ,
`applicationDateTime` BIGINT UNSIGNED NOT NULL ,
`deleted` TINYINT UNSIGNED NOT NULL ,
`lastCorpUpdaterID` INT UNSIGNED NOT NULL ,
PRIMARY KEY ( `corporationID` , `characterID` )
) ENGINE = InnoDB;
