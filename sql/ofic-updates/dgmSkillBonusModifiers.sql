
-- --------------------------------------------------------

--
-- Table structure for table `dgmSkillBonusModifiers`
--

-- DROP TABLE IF EXISTS `dgmSkillBonusModifiers`;
-- IF NOT EXISTS
CREATE TABLE `dgmSkillBonusModifiers` (
  `skillID` int(11),
  `effectID` int(11) NOT NULL,
  `sourceAttributeID` int(11) NOT NULL,
  `targetAttributeID` int(11) NOT NULL,
  `calculationTypeID` int(11) NOT NULL,
  `description` varchar(200) NOT NULL,
  `reverseCalculationTypeID` int(11) NOT NULL,
  `targetGroupIDs` varchar(200) NOT NULL,
  `targetChargeSize` int(11) NOT NULL,
  `appliedPerLevel` int(4) NOT NULL,
  `affectingType` int(11) NOT NULL,
  `affectedType` int(11) NOT NULL
--  `effectAppliedTo` int(11) NOT NULL,
--  `targetEquipmentType` int(11) NOT NULL,
--  PRIMARY KEY (`skillID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `dgmSkillBonusModifiers`
--

