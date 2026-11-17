-- MANUAL, ONE-TIME CRUCIBLE REPAIR: 14 published hulls still missing attribute 600.
-- Back up dgmTypeAttributes before running. Confirm preview in staging first.
-- Twelve command/strategic hulls have 2017/2019 historical support for 3 AU/s.
-- Primae at 3 AU/s has only present-day corroboration. Noctis 3 AU/s is a
-- PROVISIONAL default requiring 2011/2012 SDE confirmation; skip these two
-- with the clearly indicated NOT IN filter if strict archive fidelity matters.
-- Existing values are NEVER overwritten, even if changed from a prior preview.
DROP TEMPORARY TABLE IF EXISTS tmp_crucible_remaining_hulls;
CREATE TEMPORARY TABLE tmp_crucible_remaining_hulls(
 typeID INT UNSIGNED NOT NULL PRIMARY KEY,
 hull VARCHAR(80) NOT NULL,
 expectedGroup VARCHAR(80) NOT NULL,
 proposedAU DOUBLE NOT NULL,
 evidence VARCHAR(140) NOT NULL
);
INSERT INTO tmp_crucible_remaining_hulls
 (typeID,hull,expectedGroup,proposedAU,evidence)
VALUES
 (22448,'Absolution','Command Ship',3.00,'2019 Command Ship pre-change was 3 AU/s'),
 (22466,'Astarte','Command Ship',3.00,'2019 Command Ship pre-change was 3 AU/s'),
 (22468,'Claymore','Command Ship',3.00,'2019 Command Ship pre-change was 3 AU/s'),
 (22474,'Damnation','Command Ship',3.00,'2019 Command Ship pre-change was 3 AU/s'),
 (22442,'Eos','Command Ship',3.00,'2019 Command Ship pre-change was 3 AU/s'),
 (22470,'Nighthawk','Command Ship',3.00,'2019 Command Ship pre-change was 3 AU/s'),
 (22444,'Sleipnir','Command Ship',3.00,'2019 Command Ship pre-change was 3 AU/s'),
 (22446,'Vulture','Command Ship',3.00,'2019 Command Ship pre-change was 3 AU/s'),
 (29986,'Legion','Strategic Cruiser',3.00,'2017 and 2019 references show 3 AU/s'),
 (29990,'Loki','Strategic Cruiser',3.00,'2017 and 2019 references show 3 AU/s'),
 (29988,'Proteus','Strategic Cruiser',3.00,'2017 and 2019 references show 3 AU/s'),
 (29984,'Tengu','Strategic Cruiser',3.00,'2017 and 2019 references show 3 AU/s'),
 (2863,'Primae','Industrial',3.00,'Current primae 3 AU/s; 2011 SDE not verified'),
 (2998,'Noctis','Industrial',3.00,'PROVISIONAL: needs direct Crucible SDE verification');
-- PREVIEW: verify 14 matches and READY_INSERT; review provisional rows.
SELECT t.typeID,t.typeName,g.groupName,
       COALESCE(a.valueFloat,a.valueInt) AS currentAU,
       x.proposedAU,x.evidence,
       CASE WHEN a.typeID IS NULL THEN 'READY_INSERT'
            WHEN COALESCE(a.valueFloat,a.valueInt)=x.proposedAU THEN 'ALREADY_CORRECT'
            ELSE 'SKIP_EXISTING' END AS action
FROM tmp_crucible_remaining_hulls x
JOIN invTypes t ON t.typeID=x.typeID AND t.published=1
JOIN invGroups g ON g.groupID=t.groupID AND g.categoryID=6 AND g.groupName=x.expectedGroup
LEFT JOIN dgmTypeAttributes a ON a.typeID=x.typeID AND a.attributeID=600
ORDER BY g.groupName,t.typeName;
-- APPLY: will insert only absent 600 rows, not update existing attributes.
-- For the 12 better-supported hulls only, add before the final semicolon:
--   AND x.typeID NOT IN (2863,2998)
START TRANSACTION;
INSERT INTO dgmTypeAttributes (typeID,attributeID,valueInt,valueFloat)
SELECT x.typeID,600,NULL,x.proposedAU
FROM tmp_crucible_remaining_hulls x
JOIN invTypes t ON t.typeID=x.typeID AND t.published=1
JOIN invGroups g ON g.groupID=t.groupID AND g.categoryID=6 AND g.groupName=x.expectedGroup
LEFT JOIN dgmTypeAttributes a ON a.typeID=x.typeID AND a.attributeID=600
WHERE a.typeID IS NULL;
SELECT ROW_COUNT() AS added_remaining_ship_warp_attributes;
COMMIT;
-- VERIFICATION: should be PASS for each inserted row; on skip, CHECK remains.
SELECT x.typeID,x.hull,x.proposedAU,
       COALESCE(a.valueFloat,a.valueInt) AS actualAU,
       CASE WHEN COALESCE(a.valueFloat,a.valueInt)=x.proposedAU
            THEN 'PASS' ELSE 'CHECK' END AS verification
FROM tmp_crucible_remaining_hulls x
LEFT JOIN dgmTypeAttributes a ON a.typeID=x.typeID AND a.attributeID=600
ORDER BY x.hull;
-- Final 600-null coverage (published ships only).
SELECT COUNT(*) AS published_hulls_missing_warp_600
FROM invTypes t
JOIN invGroups g ON g.groupID=t.groupID
LEFT JOIN dgmTypeAttributes a ON a.typeID=t.typeID AND a.attributeID=600
WHERE g.categoryID=6 AND t.published=1 AND a.typeID IS NULL;
DROP TEMPORARY TABLE tmp_crucible_remaining_hulls;
