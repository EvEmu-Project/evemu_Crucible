
-- --------------------------------------------------------

--
-- Table structure for table `avatar_modifiers`
--
DROP TABLE IF EXISTS `avatar_modifiers`;
CREATE TABLE IF NOT EXISTS `avatar_modifiers` (
	`charID` int(11) NOT NULL,
	`modifierLocationID` int(5) NOT NULL,
	`paperdollResourceID` int(5) NOT NULL,
	`paperdollResourceVariation` int(5)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
--
-- Dumping data for table `avatar_modifiers`
--