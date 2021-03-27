/*
 *    File:  MapConnections.h
 *  Author:  Allan
 * Purpose:  See MapConnections.cpp
 *
 * Created on June 29, 2014, 12:40 PM
 *
 *   usage:
 *     MapCon::PopulateConnections();
 */


#ifndef MAPCONN_H
#define MAPCONN_H

class MapCon {
  public:
      static void PopulateConnections();

};

/*
CREATE TABLE IF NOT EXISTS `mapConnections` (
  `ctype` int(10) unsigned NOT NULL,
  `fromreg` int(10) unsigned NOT NULL,
  `fromcon` int(10) unsigned DEFAULT NULL,
  `fromsol` int(10) unsigned DEFAULT NULL,
  `stargateID` int(10) unsigned DEFAULT NULL,
  `celestialID` int(10) unsigned DEFAULT NULL,
  `tosol` int(10) unsigned DEFAULT NULL,
  `tocon` int(10) unsigned DEFAULT NULL,
  `toreg` int(10) unsigned NOT NULL,
  `AI` int(11) NOT NULL AUTO_INCREMENT,
  UNIQUE KEY `AI` (`AI`)
  UNIQUE KEY `stargateID` (`stargateID`),
  UNIQUE KEY `celestialID` (`celestialID`),
  KEY `stargateID_2` (`stargateID`),
  KEY `celestialID_2` (`celestialID`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci ;
*/
#endif