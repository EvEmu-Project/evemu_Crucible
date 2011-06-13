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
INSERT INTO `stastationtypes_old` VALUES ('16', null, null, '101', '0', '-1498', '0', '0', '-1351', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('17', null, null, '0', '0', '-2300', '0', '0', '-1015', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('54', null, '241', '36', '6854.26220703', '-7948.46679688', '0', '5.96046518808e-008', '-1', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('56', null, '1210', '0', '-5000', '4800', '0', '-1', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('57', null, '244', '-4.19215595571e-009', '6143.52929688', '-3100.00708008', '1.07482665044e-005', '0.107010781765', '0.994257867336', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('58', null, '241', '0', '0', '0', '0', '0', '-1', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('59', null, '242', '0', '0', '0', '0', '0', '-1', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('1529', null, '241', '-170', '3217', '-12112.1542969', '0', '5.96046447754e-008', '-0.999999940395', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('1530', null, '241', '11384.4287109', '7499.12792969', '-270.25189209', '0.981180846691', '0.193091094494', '1.37698270564e-008', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('1531', null, '241', '2447.21484375', '-3841', '-2988', '0.95105612278', '0.30901825428', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('1926', null, '1075', '0', '11158.4707031', '0', '0', '0.999999940395', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('1927', null, '1075', '0', '-21987.5253906', '0', '0', '-0.999999940395', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('1928', null, '1075', '0', '-21316.4511719', '0', '0', '-1.00000011921', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('1929', null, '1075', '199.788482666', '-12936.9394531', '51.9939537048', '0', '-1', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('1930', null, '1075', '-10709', '-9721', '-2069', '0', '-1', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('1931', null, '1075', '-13', '-5689.26806641', '0', '0', '-0.999999940395', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('1932', null, '1075', '-1749.89038086', '-9967.82910156', '27', '0', '-1', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('2496', null, '242', '-2909', '-1437.88049316', '20657', '0', '5.96046447754e-008', '1', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('2497', null, '242', '6299', '2654.04418945', '-11165.6542969', '0', '0.00626164721325', '-0.999980449677', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('2498', null, '242', '30416.3066406', '-1172', '3099.96264648', '0.999999940395', '5.96046518808e-008', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('2499', null, '242', '11903.6494141', '8452.42285156', '-9744.41503906', '0.999980449677', '0.00626146793365', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('2500', null, '242', '1390.30151367', '27', '28047.0371094', '0.0313382260501', '0.00616767024621', '0.999489784241', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('2501', null, '242', '17', '3115', '18255.1972656', '0', '5.96046518808e-008', '1', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('2502', null, '242', '17', '10096', '12607.2060547', '0', '5.96046447754e-008', '1', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('3864', null, '1075', '370.617919922', '8566', '-390', '-1', '-1.19209289551e-007', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('3865', null, '244', '-1004.37133789', '4872.74853516', '5903.59863281', '0.350109219551', '6.31068269286e-006', '0.93670886755', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('3866', null, '244', '9264.68457031', '884', '0', '0.999999940395', '5.96046412227e-008', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('3867', null, '1210', '7933.66796875', '208.236450195', '-18212.3496094', '0.280802875757', '-0.00825552735478', '-0.959729969501', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('3868', null, '1210', '9956.19238281', '1058.91113281', '0', '0.999796926975', '0.0201505292207', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('3869', null, '244', '10679.8837891', '1006.77294922', '14350.2353516', '0.999426364899', '0.0338653326035', '8.08496025684e-009', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('3870', null, '1210', '9788.60546875', '938', '0', '0.999713182449', '-0.0120400795713', '0.0207010973245', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('3871', null, '241', '123.39453125', '-4043.97363281', '17010.3242188', '-2.31896729019e-006', '-0.239630699158', '0.970864117146', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('3872', null, '241', '-28557.8710938', '-1158.93261719', '2436.08154297', '-1', '5.96046447754e-008', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('3873', null, null, '-6', '-335', '3201', '0', '0', '1015', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('3874', null, null, '0', '-33', '2923', '0', '-351', '483', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('3875', null, null, '0', '-32', '-276', '0', '-0.587930262089', '0.808911621571', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('3876', null, null, '0', '0', '-47', '0', '-2', '892', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('3877', null, null, '0', '0', '0', '0', '0', '-1948', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('4023', null, '241', '-27844.5488281', '-1354.30688477', '2399', '-1', '5.96046518808e-008', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('4024', null, '241', '-33431.2070313', '-2574', '2477', '-1', '-1.19209332183e-007', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('9856', null, '1075', '-176', '-11789.0810547', '4116.49902344', '0', '-1.19209332183e-007', '1', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('9857', null, '1075', '2804', '8566', '-390', '-5178', '0', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('9866', null, '242', '-1500', '405', '-1800', '-1602', '885', '-2205', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('9867', null, '2303', '-896', '-26555', '4800', '0', '-21214', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('9868', null, '2067', '-3152.70874023', '-39840.2539063', '-8244.79785156', '-2.18674836105e-006', '-5.39330758329e-005', '-1', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('9873', null, '242', '-1449', '-10148', '24', '-1645', '-4313', '-10', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('9874', null, '242', '0', '-9631', '0', '0', '-14338', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('10795', null, '1075', '-1203', '-18300', '5400', '0', '-4616', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('12242', null, '2067', '0', '5079.0859375', '-5337.78466797', '0', '0.195177391171', '0.980767965317', null, null, null, '1');
INSERT INTO `stastationtypes_old` VALUES ('12294', null, '2067', '0', '-6001.50634766', '4509.06787109', '0', '-1', '0', null, null, null, '1');
INSERT INTO `stastationtypes_old` VALUES ('12295', null, '2067', '-1297.71374512', '4860.29541016', '6949.29296875', '0.110589414835', '-0.0197495371103', '0.99366992712', null, null, null, '1');
INSERT INTO `stastationtypes_old` VALUES ('19757', null, '2067', '3454.38549805', '-103', '1666.26000977', '1', '5.96046518808e-008', '-5.92971076685e-008', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('21642', null, '2067', '330.967285156', '-837.971984863', '12616.8886719', '0', '5.96046518808e-008', '1', '48', '8', null, '1');
INSERT INTO `stastationtypes_old` VALUES ('21644', null, '2067', '-13208.5742188', '88', '237', '-0.999999940395', '5.96046412227e-008', '0', '49', '4', null, '1');
INSERT INTO `stastationtypes_old` VALUES ('21645', null, '2067', '68.9013824463', '-15087.7041016', '2996.0324707', '0.000366347376257', '0.0782493054867', '0.996933698654', '50', '24', null, '1');
INSERT INTO `stastationtypes_old` VALUES ('21646', null, '2067', '3454.38549805', '-103', '1666.26000977', '1', '5.96046518808e-008', '-5.92971076685e-008', '51', '6', '0.35', '1');
INSERT INTO `stastationtypes_old` VALUES ('22296', null, '1075', '4674', '14278', '-650', '-3327', '0', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('22297', null, '1075', '64', '-1249', '1261', '0', '0', '-3948', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('22298', null, '1075', '-294', '-20500', '2504', '0', '0', '5255', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('28075', null, '2067', '-10537', '88', '237', '-8575', '0', '0', '49', null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('29286', null, '1075', '370.617919922', '8566', '-390', '-1', '-1.19209289551e-007', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('29323', null, '241', '875.929870605', '4958.72900391', '-1991.92980957', '0.921267271042', '0.12210547924', '0.369265168905', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('29387', null, '1075', '0', '11158.4707031', '0', '0', '0.999999940395', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('29388', null, '1075', '0', '-21946.9589844', '0', '0', '-1', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('29389', null, '1075', '-1749.89038086', '-9967.82910156', '27', '0', '-1', '0', null, null, null, '0');
INSERT INTO `stastationtypes_old` VALUES ('29390', null, '2067', '-3078.81445313', '-39928.6796875', '-8394.49511719', '-1.87754687886e-006', '-5.02995826537e-005', '-1', null, null, null, '0');
