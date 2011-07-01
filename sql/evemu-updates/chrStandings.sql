/*Table structure for table `crpStandings` */

DROP TABLE IF EXISTS `chrStandings`;

CREATE TABLE `crpStandings` (
  `corporationID` int(10) unsigned NOT NULL default '0',
  `fromID` int(10) unsigned NOT NULL default '0',
  `standing` double NOT NULL default '0',
  PRIMARY KEY  (`corporationID`,`fromID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
