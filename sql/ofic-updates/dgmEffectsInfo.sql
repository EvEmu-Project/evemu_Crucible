
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
  `reverseCalculationTypeID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `dgmEffectsInfo`
--



