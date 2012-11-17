
-- --------------------------------------------------------

--
-- Table structure for table `avatar_colors`
--
DROP TABLE IF EXISTS `avatar_colors`;
CREATE TABLE IF NOT EXISTS `avatar_colors` (
	`charID` int(11) NOT NULL,
	`colorID` int(5) NOT NULL,
	`colorNameA` int(5) NOT NULL,
	`colorNameBC` int(5) NOT NULL,
	`weight` float(10) NOT NULL,
	`gloss` float(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
--
-- Dumping data for table `avatar_colors`
--