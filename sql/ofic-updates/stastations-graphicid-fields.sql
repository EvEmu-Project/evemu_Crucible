# these two fields are no longer in the dump but still required

ALTER TABLE `staStationTypes`
	ADD COLUMN `hangarGraphicID` INT(10) NULL AFTER `conquerable`,
	ADD COLUMN `dockingBayGraphicID` INT(10) NULL AFTER `hangarGraphicID`;

# Populate GraphicsIDs
UPDATE `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = null WHERE stationTypeID IN (16,17,3873,3874,3875,3876,3877);
UPDATE `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = 241 WHERE stationTypeID IN (54,58,1529,1530,1531,3871,3872,4023,4024,29323);
UPDATE `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = 1210 WHERE stationTypeID IN (56,3867,3868,3870);
UPDATE `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = 244 WHERE stationTypeID IN (57,3865,3866,3869);
UPDATE `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = 242 WHERE stationTypeID IN (59,2496,2497,2498,2499,2500,2501,2502,9866,9873,9874);
UPDATE `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = 1075 WHERE stationTypeID IN (1926,1927,1928,1929,1930,1931,1932,3864,9856,9857,10795,22296,22297,22298,29286,29387,29388,29389);
UPDATE `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = 2303 WHERE stationTypeID IN (9867);
UPDATE `staStationTypes` SET dockingBayGraphicID = null, hangarGraphicID = 2067 WHERE stationTypeID IN (9868,12242,12294,12295,19757,21642,21644,21645,21646,28075,29390);

