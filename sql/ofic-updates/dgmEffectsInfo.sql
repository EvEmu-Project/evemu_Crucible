
-- --------------------------------------------------------

--
-- Table structure for table `dgmEffectsInfo`
--

-- DROP TABLE IF EXISTS `dgmEffectsInfo`;
-- IF NOT EXISTS
CREATE TABLE `dgmEffectsInfo` (
  `effectID` int(11),
  `sourceAttributeID` int(11) NOT NULL,
  `targetAttributeID` int(11) NOT NULL,
  `calculationTypeID` int(11) NOT NULL,
  `description` varchar(200) NOT NULL,
  `reverseCalculationTypeID` int(11) NOT NULL,
  `targetGroupIDs` varchar(200) NOT NULL,
  `stackingPenaltyApplied` int(11) NOT NULL,
  `effectAppliedInState` int(11) NOT NULL,
  `affectingID` int(11) NOT NULL,
  `affectingType` int(11) NOT NULL,
  `affectedType` int(11) NOT NULL
--  `effectAppliedTo` int(11) NOT NULL,
--  `targetEquipmentType` int(11) NOT NULL,
--  `nullifyOnlineEffect` int(11) NOT NULL,
--  `nullifiedOnlineEffectID` int(11) NOT NULL,
--  PRIMARY KEY (`effectID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `dgmEffectsInfo`
--

