SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

CREATE TABLE IF NOT EXISTS `planetsforchar` (
  `characterID` int(11) NOT NULL,
  `planetID` int(11) NOT NULL,
  `solarSystemID` int(11) NOT NULL,
  `typeID` int(11) NOT NULL,
  `numberOfPins` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `planetsforchar`
  ADD PRIMARY KEY (`characterID`,`planetID`);
