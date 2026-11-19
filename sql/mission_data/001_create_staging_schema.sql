-- EVEmu Crucible mission staging schema
-- Additive only. This file creates isolated rmr_* staging tables.
-- It does NOT modify agtMissions, dun*, invTypes, characters, accounts, or live offers.
-- Target: MySQL/MariaDB. Review against the exact server schema before applying to a TEST copy.

CREATE TABLE IF NOT EXISTS rmrMissionStage (
  stageMissionID BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  sourceKey VARCHAR(190) NOT NULL,
  missionName VARCHAR(255) NOT NULL,
  missionKind ENUM('security','courier','mining','storyline','epic_arc','encounter','unknown') NOT NULL DEFAULT 'unknown',
  missionLevel TINYINT UNSIGNED NULL,
  factionID INT UNSIGNED NULL,
  raceID INT UNSIGNED NULL,
  sourceMissionID INT UNSIGNED NULL,
  targetMissionID INT UNSIGNED NULL,
  briefingID INT UNSIGNED NULL,
  objectiveText MEDIUMTEXT NULL,
  rewardISK BIGINT UNSIGNED NULL,
  bonusISK BIGINT UNSIGNED NULL,
  bonusTimeSeconds INT UNSIGNED NULL,
  rewardTypeID INT UNSIGNED NULL,
  rewardQuantity INT UNSIGNED NULL,
  dungeonTemplateID INT UNSIGNED NULL,
  compatibilityStatus ENUM('research','unmapped','blocked','validated','approved','promoted') NOT NULL DEFAULT 'research',
  sourceVersion VARCHAR(128) NULL,
  notes TEXT NULL,
  createdAt TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updatedAt TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (stageMissionID),
  UNIQUE KEY uq_rmrMissionStage_sourceKey (sourceKey),
  KEY ix_rmrMissionStage_status_level (compatibilityStatus, missionLevel),
  KEY ix_rmrMissionStage_target (targetMissionID)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS rmrMissionStageSource (
  stageSourceID BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  stageMissionID BIGINT UNSIGNED NOT NULL,
  sourceKind ENUM('ccp_sde','eveinfo','eve_university','eve_survival','npc_reference','evemu_static','manual_test') NOT NULL,
  sourceURL VARCHAR(1024) NOT NULL,
  sourceRevision VARCHAR(128) NULL,
  retrievedAt DATETIME NULL,
  fieldScope VARCHAR(255) NOT NULL,
  confidence ENUM('authoritative','corroborated','observed','unverified') NOT NULL DEFAULT 'unverified',
  attribution TEXT NULL,
  notes TEXT NULL,
  PRIMARY KEY (stageSourceID),
  KEY ix_rmrStageSource_mission (stageMissionID),
  CONSTRAINT fk_rmrStageSource_mission FOREIGN KEY (stageMissionID)
    REFERENCES rmrMissionStage(stageMissionID) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS rmrMissionStageStep (
  stageStepID BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  stageMissionID BIGINT UNSIGNED NOT NULL,
  stepOrder SMALLINT UNSIGNED NOT NULL,
  stepName VARCHAR(255) NULL,
  completionRule VARCHAR(255) NULL,
  triggerDescription TEXT NULL,
  triggerVerified TINYINT(1) NOT NULL DEFAULT 0,
  roomID INT UNSIGNED NULL,
  roomTemplateID INT UNSIGNED NULL,
  sourceStageKey VARCHAR(190) NULL,
  notes TEXT NULL,
  PRIMARY KEY (stageStepID),
  UNIQUE KEY uq_rmrStageStep_order (stageMissionID, stepOrder),
  CONSTRAINT fk_rmrStageStep_mission FOREIGN KEY (stageMissionID)
    REFERENCES rmrMissionStage(stageMissionID) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS rmrMissionStageSpawn (
  stageSpawnID BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  stageStepID BIGINT UNSIGNED NOT NULL,
  spawnGroupKey VARCHAR(190) NOT NULL,
  spawnOrder SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  typeID INT UNSIGNED NOT NULL,
  quantity SMALLINT UNSIGNED NOT NULL,
  x DOUBLE NULL,
  y DOUBLE NULL,
  z DOUBLE NULL,
  placementVerified TINYINT(1) NOT NULL DEFAULT 0,
  triggerKey VARCHAR(190) NULL,
  triggerVerified TINYINT(1) NOT NULL DEFAULT 0,
  bountyISK BIGINT UNSIGNED NULL,
  sourceNote VARCHAR(512) NULL,
  PRIMARY KEY (stageSpawnID),
  KEY ix_rmrStageSpawn_step (stageStepID),
  KEY ix_rmrStageSpawn_type (typeID),
  CONSTRAINT fk_rmrStageSpawn_step FOREIGN KEY (stageStepID)
    REFERENCES rmrMissionStageStep(stageStepID) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS rmrMissionStageLoot (
  stageLootID BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  stageSpawnID BIGINT UNSIGNED NOT NULL,
  itemTypeID INT UNSIGNED NOT NULL,
  quantityMin SMALLINT UNSIGNED NULL,
  quantityMax SMALLINT UNSIGNED NULL,
  dropChance DECIMAL(7,6) NULL,
  lootKind ENUM('npc_drop','salvage','wreck_content','mission_item','unknown') NOT NULL DEFAULT 'unknown',
  guaranteed TINYINT(1) NOT NULL DEFAULT 0,
  sourceNote VARCHAR(512) NULL,
  PRIMARY KEY (stageLootID),
  KEY ix_rmrStageLoot_spawn (stageSpawnID),
  KEY ix_rmrStageLoot_type (itemTypeID),
  CONSTRAINT fk_rmrStageLoot_spawn FOREIGN KEY (stageSpawnID)
    REFERENCES rmrMissionStageSpawn(stageSpawnID) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- A staged mission is not runtime data until it has passed every applicable
-- check in docs/missions/STAGING_VALIDATION.sql and been explicitly promoted.
