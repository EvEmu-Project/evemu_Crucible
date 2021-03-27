
CREATE TABLE IF NOT EXISTS `dedTestTable` (
  `typeID` int(11) NOT NULL,
  `scanGroupID` int(11) NOT NULL,
  `groupID` int(10) NOT NULL DEFAULT '0',
  `strengthAttributeID` int(10) NOT NULL DEFAULT '0',
  `dungeonName` varchar(25) COLLATE utf8_unicode_ci NOT NULL,
  `id` varchar(10) COLLATE utf8_unicode_ci NOT NULL,
  `x` double NOT NULL,
  `y` double NOT NULL,
  `z` double NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;


INSERT INTO `dedTestTable` (`typeID`, `scanGroupID`, `groupID`, `strengthAttributeID`, `dungeonName`, `id`, `x`, `y`, `z`)
VALUES
 ('28356', '128', '885', '1136', 'Drone Squad', 'LAN-331', '1231197245226', '-146772910080', '111531171840'),
 ('28356', '128', '885', '1136', 'Drone Menagerie', 'PDR-841', '18444153541', '-130894110720', '139953807360'),
 ('28356', '128', '885', '1136', 'Drone Squad', 'ODR-392', '-1249402429440', '17663139840', '-2266524672000'),
 ('28356', '128', '885', '1136', 'Drone Surveillance', 'JDR-147', '935699372955', '54293429821', '7677568816256'),
 ('28356', '128', '885', '1136', 'Drone Squad', 'QDR-290', '292796375040', '76468101120', '240046448640'),
 ('28356', '128', '885', '1136', 'Drone Squad', 'MDR-494', '1020728401920', '-299353251840', '7678140948480'),
 ('28356', '128', '885', '1136', 'Drone Surveillance', 'LDR-045', '-1237839940878', '15647174617', '-2344853790720'),
 ('28356', '128', '885', '1136', 'Drone Horde', 'MPK-328', '-124526223360', '-183625846026.77', '-228716875404.414'),
 ('28356', '128', '885', '1136', 'Drone Surveillance', 'KDR-596', '1038101912475', '-3795141059', '7614467478656');