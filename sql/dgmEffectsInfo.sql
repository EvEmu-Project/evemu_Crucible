/*
Navicat MySQL Data Transfer

Source Server         : evemu
Source Server Version : 50156
Source Host           : localhost:3306
Source Database       : evemuincursion

Target Server Type    : MYSQL
Target Server Version : 50156
File Encoding         : 65001

Date: 2011-07-23 15:04:30
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `dgmeffectsinfo`
-- ----------------------------
DROP TABLE IF EXISTS `dgmeffectsinfo`;
CREATE TABLE `dgmeffectsinfo` (
  `effectID` int(11) NOT NULL,
  `targetAttributeID` int(11) NOT NULL,
  `sourceAttributeID` int(11) NOT NULL,
  `calculationTypeID` int(11) NOT NULL,
  PRIMARY KEY (`effectID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of dgmeffectsinfo
-- ----------------------------
