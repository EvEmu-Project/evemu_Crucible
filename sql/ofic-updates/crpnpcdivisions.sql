/*
Navicat MySQL Data Transfer

Source Server         : MySQL (local)
Source Server Version : 50525
Source Host           : localhost:3306
Source Database       : evemu

Target Server Type    : MYSQL
Target Server Version : 50525
File Encoding         : 65001

Date: 2012-12-09 20:32:12
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `crpnpcdivisions`
-- ----------------------------
DROP TABLE IF EXISTS `crpnpcdivisions`;
CREATE TABLE `crpnpcdivisions` (
  `divisionID` tinyint(3) unsigned NOT NULL,
  `divisionName` varchar(100) DEFAULT NULL,
  `divisionNameID` int(11) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL,
  `leaderType` varchar(100) DEFAULT NULL,
  `leaderTypeID` int(11) DEFAULT NULL,
  PRIMARY KEY (`divisionID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of crpnpcdivisions
-- ----------------------------
INSERT INTO `crpnpcdivisions` VALUES ('1', 'Accounting', '1', '', 'CFO', '1');
INSERT INTO `crpnpcdivisions` VALUES ('2', 'Administration', '2', '', 'CFO', '2');
INSERT INTO `crpnpcdivisions` VALUES ('3', 'Advisory', '3', '', 'Chief Advisor', '3');
INSERT INTO `crpnpcdivisions` VALUES ('4', 'Archives', '4', '', 'Chief Archivist', '4');
INSERT INTO `crpnpcdivisions` VALUES ('5', 'Astrosurveying', '5', '', 'Survey Manager', '5');
INSERT INTO `crpnpcdivisions` VALUES ('6', 'Command', '6', '', 'COO', '6');
INSERT INTO `crpnpcdivisions` VALUES ('7', 'Distribution', '7', '', 'Distribution Manager', '7');
INSERT INTO `crpnpcdivisions` VALUES ('8', 'Financial', '8', '', 'CFO', '8');
INSERT INTO `crpnpcdivisions` VALUES ('9', 'Intelligence', '9', '', 'Chief Operative', '9');
INSERT INTO `crpnpcdivisions` VALUES ('10', 'Internal Security', '10', '', 'Commander', '10');
INSERT INTO `crpnpcdivisions` VALUES ('11', 'Legal', '11', '', 'Principal Clerk', '11');
INSERT INTO `crpnpcdivisions` VALUES ('12', 'Manufacturing', '12', '', 'Assembly Manager', '12');
INSERT INTO `crpnpcdivisions` VALUES ('13', 'Marketing', '13', '', 'Market Manager', '13');
INSERT INTO `crpnpcdivisions` VALUES ('14', 'Mining', '14', '', 'Mining Coordinator', '14');
INSERT INTO `crpnpcdivisions` VALUES ('15', 'Personnel', '15', '', 'Chief of Staff', '15');
INSERT INTO `crpnpcdivisions` VALUES ('16', 'Production', '16', '', 'Production Manager', '16');
INSERT INTO `crpnpcdivisions` VALUES ('17', 'Public Relations', '17', '', 'Chief Coordinator', '17');
INSERT INTO `crpnpcdivisions` VALUES ('18', 'R&D', '18', '', 'Chief Researcher', '18');
INSERT INTO `crpnpcdivisions` VALUES ('19', 'Security', '19', '', 'Commander', '19');
INSERT INTO `crpnpcdivisions` VALUES ('20', 'Storage', '20', '', 'Storage Facilitator', '20');
INSERT INTO `crpnpcdivisions` VALUES ('21', 'Surveillance', '21', '', 'Chief Scout', '21');
INSERT INTO `crpnpcdivisions` VALUES ('22', 'Distribution', '22', 'New distribution division', 'Distribution Manager', '22');
INSERT INTO `crpnpcdivisions` VALUES ('23', 'Mining', '23', 'New mining division', 'Mining Coordinator', '23');
INSERT INTO `crpnpcdivisions` VALUES ('24', 'Security', '24', 'New security division', 'Commander', '24');
