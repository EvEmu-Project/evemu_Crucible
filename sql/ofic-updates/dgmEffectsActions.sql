
-- --------------------------------------------------------

--
-- Table structure for table `dgmEffectsActions`
--

DROP TABLE IF EXISTS `dgmEffectsActions`;
CREATE TABLE IF NOT EXISTS `dgmEffectsActions` (
  `effectID` int(11) NOT NULL,
  `effectAppliedInState` int(11) NOT NULL,
  `effectAppliedTo` int(11) NOT NULL,
  `effectApplicationType` int(11) NOT NULL,
  `targetEquipmentType` int(11) NOT NULL,
  `targetGroupIDs` text NOT NULL,
  `stackingPenaltyApplied` int(11) NOT NULL,
  `nullifyOnlineEffect` int(11) NOT NULL,
  `nullifiedOnlineEffectID` int(11) NOT NULL,
  PRIMARY KEY (`effectID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `dgmEffectsActions`
--



