# these two fields are no longer in the dump but still required

ALTER TABLE `stastations`
	ADD COLUMN `hangarGraphicID` INT(10) NULL AFTER `reprocessingHangarFlag`,
	ADD COLUMN `dockingBayGraphicID` INT(10) NULL AFTER `hangarGraphicID`;