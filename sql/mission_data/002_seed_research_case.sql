-- Research-only seed. This is NOT a runnable mission definition.
-- No NPC typeIDs, dungeon IDs, coordinates, triggers, rewards or loot are guessed.
-- The Blockade has multiple faction variants; this record is only a research placeholder.

INSERT INTO rmrMissionStage
  (sourceKey, missionName, missionKind, missionLevel, compatibilityStatus,
   sourceVersion, notes)
VALUES
  ('research:blockade:angel-cartel:l1',
   'The Blockade - Angel Cartel (Level 1)',
   'security',
   1,
   'research',
   'community reports; runtime version not yet matched',
   'Research placeholder only. Before adding steps/spawns, map every NPC and room to this Crucible build. Do not promote.')
ON DUPLICATE KEY UPDATE
  missionName=VALUES(missionName),
  missionKind=VALUES(missionKind),
  missionLevel=VALUES(missionLevel),
  compatibilityStatus='research',
  notes=VALUES(notes);

INSERT INTO rmrMissionStageSource
  (stageMissionID, sourceKind, sourceURL, sourceRevision, fieldScope, confidence, attribution, notes)
SELECT stageMissionID, 'eveinfo',
       'https://eveinfo.com/missions/394/the-blockade',
       NULL,
       'mission identity; reported initial NPC group and walkthrough structure',
       'observed',
       'EVE Info - The Blockade',
       'Community reference only; reported bounty/spawn data is not imported as executable rows.'
FROM rmrMissionStage
WHERE sourceKey='research:blockade:angel-cartel:l1'
  AND NOT EXISTS (
    SELECT 1 FROM rmrMissionStageSource s
    WHERE s.stageMissionID=rmrMissionStage.stageMissionID
      AND s.sourceURL='https://eveinfo.com/missions/394/the-blockade'
  );

INSERT INTO rmrMissionStageSource
  (stageMissionID, sourceKind, sourceURL, sourceRevision, fieldScope, confidence, attribution, notes)
SELECT stageMissionID, 'eve_university',
       'https://wiki.eveuniversity.org/The_Blockade_(Angel_Cartel)_(Level_1)',
       NULL,
       'reported reinforcement wave groups and completion observations',
       'observed',
       'EVE University Wiki - The Blockade (Angel Cartel) (Level 1)',
       'Independent community report; keep observations separate until trigger/version compatibility is established.'
FROM rmrMissionStage
WHERE sourceKey='research:blockade:angel-cartel:l1'
  AND NOT EXISTS (
    SELECT 1 FROM rmrMissionStageSource s
    WHERE s.stageMissionID=rmrMissionStage.stageMissionID
      AND s.sourceURL='https://wiki.eveuniversity.org/The_Blockade_(Angel_Cartel)_(Level_1)'
  );
