
-- --------------------------------------------------------

--
-- Table structure for table `dgmEffectsActions`
--

-- DROP TABLE IF EXISTS `dgmEffectsActions`;
-- IF NOT EXISTS
CREATE TABLE `dgmEffectsActions` (
  `effectID` int(11) NOT NULL,
  `effectAppliedInState` int(11) NOT NULL,
  `nullifyOnlineEffect` int(11) NOT NULL,
  `nullifiedOnlineEffectID` int(11) NOT NULL,
  PRIMARY KEY (`effectID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `dgmEffectsActions`
--

