SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

CREATE TABLE IF NOT EXISTS `planetlaunches` (
  `characterID` int(11) NOT NULL,
  `launchID` int(11) NOT NULL,
  `launchTime` bigint(20) NOT NULL,
  `solarSystemID` int(11) NOT NULL,
  `planetID` int(11) NOT NULL,
  `x` double NOT NULL,
  `y` double NOT NULL,
  `z` double NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `planetlaunches`
  ADD UNIQUE KEY `launchID` (`launchID`);

ALTER TABLE `planetlaunches`
  MODIFY `launchID` int(11) NOT NULL AUTO_INCREMENT;