/*Table structure for table `accountApi` */

DROP TABLE IF EXISTS `accountApi`;

CREATE TABLE `accountApi` (
  `accountID` int(10) unsigned NOT NULL default '0',
  `userID` int(10) unsigned NOT NULL default '0',
  `fullKey` varchar(64) NOT NULL default '',
  `limitedKey` varchar(64) NOT NULL default '',
  `apiRole` int(10) NOT NULL default '0',
  PRIMARY KEY  (`accountID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

ALTER TABLE accountApi AUTO_INCREMENT=1000000;
