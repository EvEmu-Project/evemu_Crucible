
-- --------------------------------------------------------

--
-- Table structure for table `avatar_sculpts`
--
DROP TABLE IF EXISTS `avatar_sculpts`;
CREATE TABLE IF NOT EXISTS `avatar_sculpts` (
	`charID` int(11) NOT NULL,
	`sculptLocationID` int(5) NOT NULL,
	`weightUpDown` float(5),
	`weightLeftRight` float(5),
	`weightForwardBack` float(5)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
--
-- Dumping data for table `avatar_sculpts`
--