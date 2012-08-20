
-- --------------------------------------------------------

--
-- Table structure for table `dgmEffectsInfo`
--

DROP TABLE IF EXISTS `dgmEffectsInfo`;
CREATE TABLE IF NOT EXISTS `dgmEffectsInfo` (
  `effectID` int(11) NOT NULL,
  `targetAttributeID` int(11) NOT NULL,
  `sourceAttributeID` int(11) NOT NULL,
  `calculationTypeID` int(11) NOT NULL,
  `reverseCalculationTypeID` int(11) NOT NULL,
  `effectAppliedTo` int(11) NOT NULL,
  `effectAppliedBehavior` int(11) NOT NULL,
  `effectApplicationType` int(11) NOT NULL,
  `targetEquipmentType` int(11) NOT NULL,
  `targetGroupIDs` text NOT NULL,
  `stackingPenaltyApplied` int(11) NOT NULL,
  PRIMARY KEY (`effectID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `dgmEffectsInfo`
--



