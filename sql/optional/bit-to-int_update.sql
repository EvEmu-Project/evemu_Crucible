
ALTER TABLE `account` CHANGE `online` `online` TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE `account` CHANGE `banned` `banned` TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE `chrCharacters` CHANGE `gender` `gender` TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE `chrCharacters` CHANGE `online` `online` TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE `mktOrders` CHANGE `bid` `bid` TINYINT(1) NOT NULL DEFAULT '0';