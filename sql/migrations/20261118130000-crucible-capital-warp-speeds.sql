-- The archived Crucible multipliers use a 3 AU/s baseline. The server
-- database stores the converted AU/s in attribute 600.
-- +migrate Up
CREATE TABLE IF NOT EXISTS crucibleCapitalWarpPrevious (
    typeID INT NOT NULL PRIMARY KEY,
    valueInt INT NULL,
    valueFloat DOUBLE NULL,
    correctedAU DOUBLE NOT NULL
) ENGINE=InnoDB;

-- Record only rows this migration can update, so rollback preserves
-- already-correct values and restores the original integer/float encoding.
INSERT IGNORE INTO crucibleCapitalWarpPrevious (typeID, valueInt, valueFloat, correctedAU)
SELECT a.typeID, a.valueInt, a.valueFloat, v.correctedAU
FROM dgmTypeAttributes AS a
JOIN invTypes AS t ON t.typeID = a.typeID AND t.published = 1
JOIN (
    SELECT 20183 AS typeID, 513 AS groupID, 0.25 AS oldAU, 0.75 AS correctedAU
    UNION ALL SELECT 20185, 513, 0.25, 0.75
    UNION ALL SELECT 20187, 513, 0.25, 0.75
    UNION ALL SELECT 20189, 513, 0.25, 0.75
    UNION ALL SELECT 28352, 883, 1.50, 1.20
    UNION ALL SELECT 28606, 941, 1.50, 2.70
) AS v ON v.typeID = a.typeID AND v.groupID = t.groupID
WHERE a.attributeID = 600 AND COALESCE(a.valueFloat, a.valueInt) = v.oldAU;

UPDATE dgmTypeAttributes AS a
JOIN crucibleCapitalWarpPrevious AS old ON old.typeID = a.typeID
SET a.valueInt = NULL, a.valueFloat = old.correctedAU
WHERE a.attributeID = 600
  AND (a.valueInt <=> old.valueInt)
  AND (a.valueFloat <=> old.valueFloat);

-- +migrate Down
UPDATE dgmTypeAttributes AS a
JOIN crucibleCapitalWarpPrevious AS old ON old.typeID = a.typeID
SET a.valueInt = old.valueInt, a.valueFloat = old.valueFloat
WHERE a.attributeID = 600 AND a.valueInt IS NULL AND a.valueFloat = old.correctedAU;

DROP TABLE crucibleCapitalWarpPrevious;
