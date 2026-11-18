-- +migrate Up
-- Numbering is local to a facility and a line type. Existing station rows
-- keep their IDs and NULL posSlot, so their jobs and scheduling are retained.
ALTER TABLE ramAssemblyLines
    MODIFY assemblyLineID INT(11) NOT NULL AUTO_INCREMENT,
    ADD COLUMN posSlot TINYINT UNSIGNED NULL,
    ADD UNIQUE KEY ramAssemblyLines_posSlot (containerID, assemblyLineTypeID, posSlot);

CREATE VIEW ramOnlinePosLineTypes AS
SELECT e.itemID AS containerID, e.typeID AS containerTypeID,
       e.locationID AS containerLocationID, e.ownerID,
       cfg.assemblyLineTypeID, cfg.quantity, lineType.activityID
FROM posStructureData AS pos
JOIN entity AS e ON e.itemID = pos.itemID
JOIN invTypes AS ty ON ty.typeID = e.typeID
JOIN posStructureData AS towerPos ON towerPos.itemID = pos.towerID
JOIN posTowerData AS towerData ON towerData.itemID = towerPos.itemID
JOIN entity AS tower ON tower.itemID = towerPos.itemID
    AND tower.ownerID = e.ownerID AND tower.locationID = e.locationID
JOIN ramInstallationTypeContents AS cfg ON cfg.installationTypeID = e.typeID
JOIN ramAssemblyLineTypes AS lineType ON lineType.assemblyLineTypeID = cfg.assemblyLineTypeID
WHERE pos.state >= 4 AND towerPos.state >= 4 AND towerData.harmonic = 1
  AND cfg.quantity > 0
  AND ((ty.groupID = 397 AND lineType.activityID = 1)
       OR (ty.groupID = 413 AND lineType.activityID IN (3, 4, 5)));

-- +migrate Down
-- POS rows can be referenced by industry jobs. Preserve them on rollback.
SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'POS industry schema rollback requires a reviewed data migration';
