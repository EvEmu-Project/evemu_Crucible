-- Safe, additive upgrade for an existing rmrMissionStage* staging schema.
-- Run AFTER 001_create_staging_schema.sql, on a TEST database first.
-- This migration never drops tables/columns or changes agtMissions, dun*, invTypes,
-- character/account data, wallets, inventories, or existing mission offers.
-- It adds missing columns only. Existing rows are retained.
-- MySQL/MariaDB: uses information_schema + prepared ALTER statements.

-- rmrMissionStage
SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'sourceKey') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `sourceKey` VARCHAR(190) NOT NULL DEFAULT ''''',
  'SELECT ''rmrMissionStage.sourceKey already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'missionName') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `missionName` VARCHAR(255) NOT NULL DEFAULT ''''',
  'SELECT ''rmrMissionStage.missionName already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'missionKind') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `missionKind` ENUM(''security'',''courier'',''mining'',''storyline'',''epic_arc'',''encounter'',''unknown'') NOT NULL DEFAULT ''unknown''',
  'SELECT ''rmrMissionStage.missionKind already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'missionLevel') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `missionLevel` TINYINT UNSIGNED NULL',
  'SELECT ''rmrMissionStage.missionLevel already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'factionID') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `factionID` INT UNSIGNED NULL',
  'SELECT ''rmrMissionStage.factionID already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'raceID') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `raceID` INT UNSIGNED NULL',
  'SELECT ''rmrMissionStage.raceID already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'sourceMissionID') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `sourceMissionID` INT UNSIGNED NULL',
  'SELECT ''rmrMissionStage.sourceMissionID already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'targetMissionID') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `targetMissionID` INT UNSIGNED NULL',
  'SELECT ''rmrMissionStage.targetMissionID already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'briefingID') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `briefingID` INT UNSIGNED NULL',
  'SELECT ''rmrMissionStage.briefingID already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'objectiveText') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `objectiveText` MEDIUMTEXT NULL',
  'SELECT ''rmrMissionStage.objectiveText already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'rewardISK') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `rewardISK` BIGINT UNSIGNED NULL',
  'SELECT ''rmrMissionStage.rewardISK already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'bonusISK') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `bonusISK` BIGINT UNSIGNED NULL',
  'SELECT ''rmrMissionStage.bonusISK already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'bonusTimeSeconds') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `bonusTimeSeconds` INT UNSIGNED NULL',
  'SELECT ''rmrMissionStage.bonusTimeSeconds already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'rewardTypeID') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `rewardTypeID` INT UNSIGNED NULL',
  'SELECT ''rmrMissionStage.rewardTypeID already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'rewardQuantity') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `rewardQuantity` INT UNSIGNED NULL',
  'SELECT ''rmrMissionStage.rewardQuantity already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'dungeonTemplateID') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `dungeonTemplateID` INT UNSIGNED NULL',
  'SELECT ''rmrMissionStage.dungeonTemplateID already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'compatibilityStatus') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `compatibilityStatus` ENUM(''research'',''unmapped'',''blocked'',''validated'',''approved'',''promoted'') NOT NULL DEFAULT ''research''',
  'SELECT ''rmrMissionStage.compatibilityStatus already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'sourceVersion') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `sourceVersion` VARCHAR(128) NULL',
  'SELECT ''rmrMissionStage.sourceVersion already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'notes') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `notes` TEXT NULL',
  'SELECT ''rmrMissionStage.notes already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'createdAt') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `createdAt` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP',
  'SELECT ''rmrMissionStage.createdAt already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStage'
     AND COLUMN_NAME = 'updatedAt') = 0,
  'ALTER TABLE `rmrMissionStage` ADD COLUMN `updatedAt` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP',
  'SELECT ''rmrMissionStage.updatedAt already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

-- rmrMissionStageSource
SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSource'
     AND COLUMN_NAME = 'stageMissionID') = 0,
  'ALTER TABLE `rmrMissionStageSource` ADD COLUMN `stageMissionID` BIGINT UNSIGNED NOT NULL DEFAULT 0',
  'SELECT ''rmrMissionStageSource.stageMissionID already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSource'
     AND COLUMN_NAME = 'sourceKind') = 0,
  'ALTER TABLE `rmrMissionStageSource` ADD COLUMN `sourceKind` ENUM(''ccp_sde'',''eveinfo'',''eve_university'',''eve_survival'',''npc_reference'',''evemu_static'',''manual_test'') NOT NULL DEFAULT ''manual_test''',
  'SELECT ''rmrMissionStageSource.sourceKind already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSource'
     AND COLUMN_NAME = 'sourceURL') = 0,
  'ALTER TABLE `rmrMissionStageSource` ADD COLUMN `sourceURL` VARCHAR(1024) NOT NULL DEFAULT ''''',
  'SELECT ''rmrMissionStageSource.sourceURL already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSource'
     AND COLUMN_NAME = 'sourceRevision') = 0,
  'ALTER TABLE `rmrMissionStageSource` ADD COLUMN `sourceRevision` VARCHAR(128) NULL',
  'SELECT ''rmrMissionStageSource.sourceRevision already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSource'
     AND COLUMN_NAME = 'retrievedAt') = 0,
  'ALTER TABLE `rmrMissionStageSource` ADD COLUMN `retrievedAt` DATETIME NULL',
  'SELECT ''rmrMissionStageSource.retrievedAt already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSource'
     AND COLUMN_NAME = 'fieldScope') = 0,
  'ALTER TABLE `rmrMissionStageSource` ADD COLUMN `fieldScope` VARCHAR(255) NOT NULL DEFAULT ''unspecified''',
  'SELECT ''rmrMissionStageSource.fieldScope already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSource'
     AND COLUMN_NAME = 'confidence') = 0,
  'ALTER TABLE `rmrMissionStageSource` ADD COLUMN `confidence` ENUM(''authoritative'',''corroborated'',''observed'',''unverified'') NOT NULL DEFAULT ''unverified''',
  'SELECT ''rmrMissionStageSource.confidence already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSource'
     AND COLUMN_NAME = 'attribution') = 0,
  'ALTER TABLE `rmrMissionStageSource` ADD COLUMN `attribution` TEXT NULL',
  'SELECT ''rmrMissionStageSource.attribution already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSource'
     AND COLUMN_NAME = 'notes') = 0,
  'ALTER TABLE `rmrMissionStageSource` ADD COLUMN `notes` TEXT NULL',
  'SELECT ''rmrMissionStageSource.notes already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

-- rmrMissionStageStep
SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageStep'
     AND COLUMN_NAME = 'stageMissionID') = 0,
  'ALTER TABLE `rmrMissionStageStep` ADD COLUMN `stageMissionID` BIGINT UNSIGNED NOT NULL DEFAULT 0',
  'SELECT ''rmrMissionStageStep.stageMissionID already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageStep'
     AND COLUMN_NAME = 'stepOrder') = 0,
  'ALTER TABLE `rmrMissionStageStep` ADD COLUMN `stepOrder` SMALLINT UNSIGNED NOT NULL DEFAULT 0',
  'SELECT ''rmrMissionStageStep.stepOrder already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageStep'
     AND COLUMN_NAME = 'stepName') = 0,
  'ALTER TABLE `rmrMissionStageStep` ADD COLUMN `stepName` VARCHAR(255) NULL',
  'SELECT ''rmrMissionStageStep.stepName already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageStep'
     AND COLUMN_NAME = 'completionRule') = 0,
  'ALTER TABLE `rmrMissionStageStep` ADD COLUMN `completionRule` VARCHAR(255) NULL',
  'SELECT ''rmrMissionStageStep.completionRule already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageStep'
     AND COLUMN_NAME = 'triggerDescription') = 0,
  'ALTER TABLE `rmrMissionStageStep` ADD COLUMN `triggerDescription` TEXT NULL',
  'SELECT ''rmrMissionStageStep.triggerDescription already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageStep'
     AND COLUMN_NAME = 'triggerVerified') = 0,
  'ALTER TABLE `rmrMissionStageStep` ADD COLUMN `triggerVerified` TINYINT(1) NOT NULL DEFAULT 0',
  'SELECT ''rmrMissionStageStep.triggerVerified already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageStep'
     AND COLUMN_NAME = 'roomID') = 0,
  'ALTER TABLE `rmrMissionStageStep` ADD COLUMN `roomID` INT UNSIGNED NULL',
  'SELECT ''rmrMissionStageStep.roomID already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageStep'
     AND COLUMN_NAME = 'roomTemplateID') = 0,
  'ALTER TABLE `rmrMissionStageStep` ADD COLUMN `roomTemplateID` INT UNSIGNED NULL',
  'SELECT ''rmrMissionStageStep.roomTemplateID already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageStep'
     AND COLUMN_NAME = 'sourceStageKey') = 0,
  'ALTER TABLE `rmrMissionStageStep` ADD COLUMN `sourceStageKey` VARCHAR(190) NULL',
  'SELECT ''rmrMissionStageStep.sourceStageKey already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageStep'
     AND COLUMN_NAME = 'notes') = 0,
  'ALTER TABLE `rmrMissionStageStep` ADD COLUMN `notes` TEXT NULL',
  'SELECT ''rmrMissionStageStep.notes already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

-- rmrMissionStageSpawn
SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSpawn'
     AND COLUMN_NAME = 'stageStepID') = 0,
  'ALTER TABLE `rmrMissionStageSpawn` ADD COLUMN `stageStepID` BIGINT UNSIGNED NOT NULL DEFAULT 0',
  'SELECT ''rmrMissionStageSpawn.stageStepID already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSpawn'
     AND COLUMN_NAME = 'spawnGroupKey') = 0,
  'ALTER TABLE `rmrMissionStageSpawn` ADD COLUMN `spawnGroupKey` VARCHAR(190) NOT NULL DEFAULT ''unmapped''',
  'SELECT ''rmrMissionStageSpawn.spawnGroupKey already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSpawn'
     AND COLUMN_NAME = 'spawnOrder') = 0,
  'ALTER TABLE `rmrMissionStageSpawn` ADD COLUMN `spawnOrder` SMALLINT UNSIGNED NOT NULL DEFAULT 0',
  'SELECT ''rmrMissionStageSpawn.spawnOrder already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSpawn'
     AND COLUMN_NAME = 'typeID') = 0,
  'ALTER TABLE `rmrMissionStageSpawn` ADD COLUMN `typeID` INT UNSIGNED NOT NULL DEFAULT 0',
  'SELECT ''rmrMissionStageSpawn.typeID already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSpawn'
     AND COLUMN_NAME = 'quantity') = 0,
  'ALTER TABLE `rmrMissionStageSpawn` ADD COLUMN `quantity` SMALLINT UNSIGNED NOT NULL DEFAULT 0',
  'SELECT ''rmrMissionStageSpawn.quantity already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSpawn'
     AND COLUMN_NAME = 'x') = 0,
  'ALTER TABLE `rmrMissionStageSpawn` ADD COLUMN `x` DOUBLE NULL',
  'SELECT ''rmrMissionStageSpawn.x already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSpawn'
     AND COLUMN_NAME = 'y') = 0,
  'ALTER TABLE `rmrMissionStageSpawn` ADD COLUMN `y` DOUBLE NULL',
  'SELECT ''rmrMissionStageSpawn.y already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSpawn'
     AND COLUMN_NAME = 'z') = 0,
  'ALTER TABLE `rmrMissionStageSpawn` ADD COLUMN `z` DOUBLE NULL',
  'SELECT ''rmrMissionStageSpawn.z already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSpawn'
     AND COLUMN_NAME = 'placementVerified') = 0,
  'ALTER TABLE `rmrMissionStageSpawn` ADD COLUMN `placementVerified` TINYINT(1) NOT NULL DEFAULT 0',
  'SELECT ''rmrMissionStageSpawn.placementVerified already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSpawn'
     AND COLUMN_NAME = 'triggerKey') = 0,
  'ALTER TABLE `rmrMissionStageSpawn` ADD COLUMN `triggerKey` VARCHAR(190) NULL',
  'SELECT ''rmrMissionStageSpawn.triggerKey already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSpawn'
     AND COLUMN_NAME = 'triggerVerified') = 0,
  'ALTER TABLE `rmrMissionStageSpawn` ADD COLUMN `triggerVerified` TINYINT(1) NOT NULL DEFAULT 0',
  'SELECT ''rmrMissionStageSpawn.triggerVerified already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSpawn'
     AND COLUMN_NAME = 'bountyISK') = 0,
  'ALTER TABLE `rmrMissionStageSpawn` ADD COLUMN `bountyISK` BIGINT UNSIGNED NULL',
  'SELECT ''rmrMissionStageSpawn.bountyISK already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageSpawn'
     AND COLUMN_NAME = 'sourceNote') = 0,
  'ALTER TABLE `rmrMissionStageSpawn` ADD COLUMN `sourceNote` VARCHAR(512) NULL',
  'SELECT ''rmrMissionStageSpawn.sourceNote already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

-- rmrMissionStageLoot
SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageLoot'
     AND COLUMN_NAME = 'stageSpawnID') = 0,
  'ALTER TABLE `rmrMissionStageLoot` ADD COLUMN `stageSpawnID` BIGINT UNSIGNED NOT NULL DEFAULT 0',
  'SELECT ''rmrMissionStageLoot.stageSpawnID already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageLoot'
     AND COLUMN_NAME = 'itemTypeID') = 0,
  'ALTER TABLE `rmrMissionStageLoot` ADD COLUMN `itemTypeID` INT UNSIGNED NOT NULL DEFAULT 0',
  'SELECT ''rmrMissionStageLoot.itemTypeID already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageLoot'
     AND COLUMN_NAME = 'quantityMin') = 0,
  'ALTER TABLE `rmrMissionStageLoot` ADD COLUMN `quantityMin` SMALLINT UNSIGNED NULL',
  'SELECT ''rmrMissionStageLoot.quantityMin already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageLoot'
     AND COLUMN_NAME = 'quantityMax') = 0,
  'ALTER TABLE `rmrMissionStageLoot` ADD COLUMN `quantityMax` SMALLINT UNSIGNED NULL',
  'SELECT ''rmrMissionStageLoot.quantityMax already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageLoot'
     AND COLUMN_NAME = 'dropChance') = 0,
  'ALTER TABLE `rmrMissionStageLoot` ADD COLUMN `dropChance` DECIMAL(7,6) NULL',
  'SELECT ''rmrMissionStageLoot.dropChance already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageLoot'
     AND COLUMN_NAME = 'lootKind') = 0,
  'ALTER TABLE `rmrMissionStageLoot` ADD COLUMN `lootKind` ENUM(''npc_drop'',''salvage'',''wreck_content'',''mission_item'',''unknown'') NOT NULL DEFAULT ''unknown''',
  'SELECT ''rmrMissionStageLoot.lootKind already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageLoot'
     AND COLUMN_NAME = 'guaranteed') = 0,
  'ALTER TABLE `rmrMissionStageLoot` ADD COLUMN `guaranteed` TINYINT(1) NOT NULL DEFAULT 0',
  'SELECT ''rmrMissionStageLoot.guaranteed already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

SET @rmr_sql = IF(
  (SELECT COUNT(*) FROM information_schema.COLUMNS
   WHERE TABLE_SCHEMA = DATABASE()
     AND TABLE_NAME = 'rmrMissionStageLoot'
     AND COLUMN_NAME = 'sourceNote') = 0,
  'ALTER TABLE `rmrMissionStageLoot` ADD COLUMN `sourceNote` VARCHAR(512) NULL',
  'SELECT ''rmrMissionStageLoot.sourceNote already present - skipped'' AS migration_status'
);
PREPARE rmr_stmt FROM @rmr_sql;
EXECUTE rmr_stmt;
DEALLOCATE PREPARE rmr_stmt;

-- Report expected staging tables and columns after upgrade.
SELECT TABLE_NAME, COUNT(*) AS column_count
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE()
  AND TABLE_NAME IN ('rmrMissionStage','rmrMissionStageSource',
                     'rmrMissionStageStep','rmrMissionStageSpawn',
                     'rmrMissionStageLoot')
GROUP BY TABLE_NAME
ORDER BY TABLE_NAME;

-- Rows with placeholder IDs/defaults need manual review; this migration does not
-- silently promote or delete them.
