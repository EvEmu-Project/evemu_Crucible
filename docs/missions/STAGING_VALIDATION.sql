-- Read-only validation queries for rmr_* mission staging.
-- Run only against a disposable/test copy after the staging DDL is installed.
-- These queries do not mutate data.

-- 1) Spawn type IDs absent from this Crucible build's invTypes.
SELECT s.stageSpawnID, s.typeID, m.missionName
FROM rmrMissionStageSpawn s
JOIN rmrMissionStageStep st ON st.stageStepID=s.stageStepID
JOIN rmrMissionStage m ON m.stageMissionID=st.stageMissionID
LEFT JOIN invTypes t ON t.typeID=s.typeID
WHERE t.typeID IS NULL;

-- 2) Spawn types that are not ship/drone groups (categoryID 6 or 18).
SELECT s.stageSpawnID, s.typeID, t.typeName, g.categoryID, m.missionName
FROM rmrMissionStageSpawn s
JOIN rmrMissionStageStep st ON st.stageStepID=s.stageStepID
JOIN rmrMissionStage m ON m.stageMissionID=st.stageMissionID
JOIN invTypes t ON t.typeID=s.typeID
JOIN invGroups g ON g.groupID=t.groupID
WHERE g.categoryID NOT IN (6,18);

-- 3) Reward/loot item type IDs absent from Crucible's invTypes.
SELECT m.stageMissionID, m.missionName, m.rewardTypeID AS missingTypeID, 'mission_reward' AS usage
FROM rmrMissionStage m
LEFT JOIN invTypes t ON t.typeID=m.rewardTypeID
WHERE m.rewardTypeID IS NOT NULL AND t.typeID IS NULL
UNION ALL
SELECT l.stageLootID, m.missionName, l.itemTypeID, 'loot'
FROM rmrMissionStageLoot l
JOIN rmrMissionStageSpawn s ON s.stageSpawnID=l.stageSpawnID
JOIN rmrMissionStageStep st ON st.stageStepID=s.stageStepID
JOIN rmrMissionStage m ON m.stageMissionID=st.stageMissionID
LEFT JOIN invTypes t ON t.typeID=l.itemTypeID
WHERE t.typeID IS NULL;

-- 4) Staged rooms/templates not represented by this build's dungeon templates.
SELECT st.stageStepID, st.roomTemplateID, st.roomID, m.missionName
FROM rmrMissionStageStep st
JOIN rmrMissionStage m ON m.stageMissionID=st.stageMissionID
LEFT JOIN dunTemplates dt ON dt.templateID=st.roomTemplateID
WHERE st.roomTemplateID IS NOT NULL AND dt.templateID IS NULL;

-- 5) Invalid or incomplete spawn definitions.
SELECT s.stageSpawnID, s.typeID, s.quantity, m.missionName
FROM rmrMissionStageSpawn s
JOIN rmrMissionStageStep st ON st.stageStepID=s.stageStepID
JOIN rmrMissionStage m ON m.stageMissionID=st.stageMissionID
WHERE s.quantity=0 OR s.typeID=0;

-- 6) Candidates that are marked approved despite unresolved type/room checks.
-- Any returned row must block promotion.
SELECT DISTINCT m.stageMissionID, m.sourceKey, m.missionName
FROM rmrMissionStage m
LEFT JOIN rmrMissionStageStep st ON st.stageMissionID=m.stageMissionID
LEFT JOIN rmrMissionStageSpawn s ON s.stageStepID=st.stageStepID
LEFT JOIN invTypes t ON t.typeID=s.typeID
LEFT JOIN dunTemplates dt ON dt.templateID=st.roomTemplateID
WHERE m.compatibilityStatus='approved'
  AND (
    (s.stageSpawnID IS NOT NULL AND t.typeID IS NULL)
    OR (st.roomTemplateID IS NOT NULL AND dt.templateID IS NULL)
  );

-- 7) Manual review queue: unknown trigger/placement/reward/loot data.
SELECT m.stageMissionID, m.missionName, st.stepOrder, s.stageSpawnID,
       s.placementVerified, s.triggerVerified, s.bountyISK
FROM rmrMissionStage m
LEFT JOIN rmrMissionStageStep st ON st.stageMissionID=m.stageMissionID
LEFT JOIN rmrMissionStageSpawn s ON s.stageStepID=st.stageStepID
WHERE m.compatibilityStatus IN ('research','unmapped','blocked')
   OR (s.stageSpawnID IS NOT NULL AND (s.placementVerified=0 OR s.triggerVerified=0));
