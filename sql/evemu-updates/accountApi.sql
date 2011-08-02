/*Table structure for table `accountApi` */

DROP TABLE IF EXISTS `accountApi`;

CREATE TABLE `accountApi` (
  `userID` int(10) unsigned NOT NULL auto_increment,
  `accountID` int(10) unsigned NOT NULL default '0',
  `fullKey` varchar(64) NOT NULL default '',
  `limitedKey` varchar(64) NOT NULL default '',
  `apiRole` int(10) NOT NULL default '0',
  PRIMARY KEY  (`userID`)
) ENGINE=InnoDB AUTO_INCREMENT=1000000 DEFAULT CHARSET=utf8;
