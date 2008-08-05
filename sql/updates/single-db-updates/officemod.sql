CREATE TABLE `billsPayable` (
  `billID` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `billTypeID` int(10) UNSIGNED DEFAULT NULL,
  `debtorID` int(10) UNSIGNED DEFAULT NULL,
  `creditorID` int(10) UNSIGNED DEFAULT NULL,
  `amount` text NOT NULL,
  `dueDateTime` text NOT NULL,
  `interest` text NOT NULL,
  `externalID` int(10) UNSIGNED DEFAULT NULL,
  `paid` text NOT NULL,
  `externalID2` text NOT NULL,
  PRIMARY KEY  (`billID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
