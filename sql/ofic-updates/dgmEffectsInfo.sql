
SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `dgmEffectsInfo`
--



--
-- Table structure for table `dgmEffectsActions`
--

DROP TABLE IF EXISTS `dgmEffectsActions`;
CREATE TABLE IF NOT EXISTS `dgmEffectsActions` (
  `effectID` int(11) NOT NULL,
  `effectAppliedInState` int(11) NOT NULL,
  `effectAppliedTo` int(11) NOT NULL,
  `effectApplicationType` int(11) NOT NULL,
  `stackingPenaltyApplied` int(11) NOT NULL,
  `nullifyOnlineEffect` int(11) NOT NULL,
  `nullifiedOnlineEffectID` int(11) NOT NULL,
  PRIMARY KEY (`effectID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `dgmEffectsActions`
--


/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
