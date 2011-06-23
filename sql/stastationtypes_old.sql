/*
Navicat MySQL Data Transfer

Source Server         : evemu
Source Server Version : 50147
Source Host           : localhost:3306
Source Database       : evemu_incursion

Target Server Type    : MYSQL
Target Server Version : 50147
File Encoding         : 65001

Date: 2011-06-13 17:24:16
*/

SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for `stastationtypes_old`
-- ----------------------------
DROP TABLE IF EXISTS `stastationtypes_old`;
CREATE TABLE `stastationtypes_old` (
  `stationTypeID` smallint(6) NOT NULL,
  `dockingBayGraphicID` smallint(6) DEFAULT NULL,
  `hangarGraphicID` smallint(6) DEFAULT NULL,
  `dockEntryX` double DEFAULT NULL,
  `dockEntryY` double DEFAULT NULL,
  `dockEntryZ` double DEFAULT NULL,
  `dockOrientationX` double DEFAULT NULL,
  `dockOrientationY` double DEFAULT NULL,
  `dockOrientationZ` double DEFAULT NULL,
  `operationID` tinyint(3) unsigned DEFAULT NULL,
  `officeSlots` tinyint(4) DEFAULT NULL,
  `reprocessingEfficiency` double DEFAULT NULL,
  `conquerable` tinyint(1) DEFAULT NULL,
  PRIMARY KEY (`stationTypeID`),
  KEY `dockingBayGraphicID` (`dockingBayGraphicID`),
  KEY `hangarGraphicID` (`hangarGraphicID`),
  KEY `operationID` (`operationID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of stastationtypes_old
-- ----------------------------
INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = null WHERE stationTypeID IN (16,17,3873,3874,3875,3876,3877);
INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = 241 WHERE stationTypeID IN (54,58,1529,1530,1531,3871,3872,4023,4024,29323);
INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = 1210 WHERE stationTypeID IN (56,3867,3868,3870);
INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = 244 WHERE stationTypeID IN (57,3865,3866,3869);
INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = 242 WHERE stationTypeID IN (59,2496,2497,2498,2499,2500,2501,2502,9866,9873,9874);
INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = 1075 WHERE stationTypeID IN (1926,1927,1928,1929,1930,1931,1932,3864,9856,9857,10795,22296,22297,22298,29286,29387,29388,29389);

INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = 2303 WHERE stationTypeID IN (9867); ('9867', null, '2303', '-896', '-26555', '4800', '0', '-21214', '0', null, null, null, '0');
INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = 2067 WHERE stationTypeID IN (9868,12242,12294,12295,19757,21642,21644,21645,21646,28075,29390) ; ('9868', null, '2067', '-3152.70874023', '-39840.2539063', '-8244.79785156', '-2.18674836105e-006', '-5.39330758329e-005', '-1', null, null, null, '0');



INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = WHERE stationTypeID = ; ('12242', null, '2067', '0', '5079.0859375', '-5337.78466797', '0', '0.195177391171', '0.980767965317', null, null, null, '1');
INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = WHERE stationTypeID = ; ('12294', null, '2067', '0', '-6001.50634766', '4509.06787109', '0', '-1', '0', null, null, null, '1');
INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = WHERE stationTypeID = ; ('12295', null, '2067', '-1297.71374512', '4860.29541016', '6949.29296875', '0.110589414835', '-0.0197495371103', '0.99366992712', null, null, null, '1');
INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = WHERE stationTypeID = ; ('19757', null, '2067', '3454.38549805', '-103', '1666.26000977', '1', '5.96046518808e-008', '-5.92971076685e-008', null, null, null, '0');
INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = WHERE stationTypeID = ; ('21642', null, '2067', '330.967285156', '-837.971984863', '12616.8886719', '0', '5.96046518808e-008', '1', '48', '8', null, '1');
INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = WHERE stationTypeID = ; ('21644', null, '2067', '-13208.5742188', '88', '237', '-0.999999940395', '5.96046412227e-008', '0', '49', '4', null, '1');
INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = WHERE stationTypeID = ; ('21645', null, '2067', '68.9013824463', '-15087.7041016', '2996.0324707', '0.000366347376257', '0.0782493054867', '0.996933698654', '50', '24', null, '1');
INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = WHERE stationTypeID = ; ('21646', null, '2067', '3454.38549805', '-103', '1666.26000977', '1', '5.96046518808e-008', '-5.92971076685e-008', '51', '6', '0.35', '1');

INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = WHERE stationTypeID = ; ('28075', null, '2067', '-10537', '88', '237', '-8575', '0', '0', '49', null, null, '0');

INSERT INTO `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = WHERE stationTypeID = ; ('29390', null, '2067', '-3078.81445313', '-39928.6796875', '-8394.49511719', '-1.87754687886e-006', '-5.02995826537e-005', '-1', null, null, null, '0');
