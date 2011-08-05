ALTER TABLE `character_`
	ADD COLUMN `deletePrepareDateTime` BIGINT(20) UNSIGNED NULL DEFAULT '0' AFTER `online`;