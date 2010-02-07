/*Table structure for table `chrRaceSkills` */

DROP TABLE IF EXISTS `chrRaceSkills`;

CREATE TABLE `chrRaceSkills` (
  `raceID` tinyint(3) unsigned NOT NULL,
  `skillTypeID` smallint(6) NOT NULL,
  `levels` tinyint(4) default NULL,
  PRIMARY KEY  (`raceID`,`skillTypeID`),
  KEY `skillTypeID` (`skillTypeID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `chrRaceSkills` */

insert  into `chrRaceSkills`(`raceID`,`skillTypeID`,`levels`) values (1,3300,2),(1,3301,3),(1,3327,3),(1,3330,2),(1,3386,2),(1,3392,2),(1,3402,3),(1,3413,3),(1,3416,2),(1,3426,3),(1,3449,3),(2,3300,2),(2,3302,3),(2,3327,3),(2,3329,2),(2,3386,2),(2,3392,2),(2,3402,3),(2,3413,3),(2,3416,2),(2,3426,3),(2,3449,3),(4,3300,2),(4,3303,3),(4,3327,3),(4,3331,2),(4,3386,2),(4,3392,2),(4,3402,3),(4,3413,3),(4,3416,2),(4,3426,3),(4,3449,3),(8,3300,2),(8,3301,3),(8,3327,3),(8,3328,2),(8,3386,2),(8,3392,2),(8,3402,3),(8,3413,3),(8,3416,2),(8,3426,3),(8,3449,3);

