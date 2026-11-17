-- EVEmu Crucible sovereignty/POS audit — READ ONLY — 17 November 2026.
-- Run on a backed-up test database or read-only replica.
-- No UPDATE, INSERT, DELETE, migrations or automatic repairs here.
-- Assumes the normal 2021 sovereignty schema with BIGINT claimTime and claimID.

-- SOV-01: Implausible claim timestamps, owner or TCU references.
SELECT claimID, solarSystemID, allianceID, corporationID,
       claimStructureID, claimTime, hubID, contested
FROM mapSystemSovInfo
WHERE claimTime IS NULL OR claimTime < 100000000000000000
   OR claimStructureID = 0 OR allianceID = 0
ORDER BY solarSystemID;

-- SOV-02: Claims whose claimed structure has no inventory entity.
SELECT sov.solarSystemID, sov.claimStructureID, sov.claimTime,
       e.itemID AS actualEntityID
FROM mapSystemSovInfo AS sov
LEFT JOIN entity AS e ON e.itemID = sov.claimStructureID
WHERE sov.claimStructureID > 0 AND e.itemID IS NULL;

-- POS-05: Orphaned tower configuration rows (don't delete automatically).
SELECT t.itemID, t.standing, t.status, t.allowCorp, t.allowAlliance
FROM posTowerData AS t
LEFT JOIN entity AS e ON e.itemID = t.itemID
WHERE e.itemID IS NULL;

-- POS-08: Dangling, asymmetric or cross-alliance bridge links.
SELECT local.itemID, local.systemID, local.allyID,
       local.toItemID, local.toSystemID,
       remote.itemID AS remoteID, remote.systemID AS remoteSystemID,
       remote.allyID AS remoteAllianceID, remote.toItemID AS returnLink
FROM posJumpBridgeData AS local
LEFT JOIN posJumpBridgeData AS remote ON remote.itemID = local.toItemID
WHERE local.toItemID <> 0
  AND (remote.itemID IS NULL OR remote.toItemID <> local.itemID
       OR remote.systemID <> local.toSystemID
       OR remote.allyID <> local.allyID)
ORDER BY local.itemID;

-- POS: Orphaned structure state; requires manual investigation.
SELECT s.itemID, s.towerID, s.anchorpointID, s.state, s.status, s.timestamp
FROM posStructureData AS s
LEFT JOIN entity AS e ON e.itemID = s.itemID
WHERE e.itemID IS NULL;

-- Table row counts before and after any separately authorized repairs.
SELECT 'claims' AS dataset, COUNT(*) AS count_records FROM mapSystemSovInfo
UNION ALL SELECT 'posStructureData', COUNT(*) FROM posStructureData
UNION ALL SELECT 'posTowerData', COUNT(*) FROM posTowerData
UNION ALL SELECT 'posJumpBridgeData', COUNT(*) FROM posJumpBridgeData;
