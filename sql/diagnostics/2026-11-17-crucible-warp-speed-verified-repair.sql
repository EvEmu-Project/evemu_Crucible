-- Crucible 2011/2012: REPAIR VERIFIED SHIP WARP VALUES AFTER BACKUP.
-- This is a manual one-time migration, not automatically run by EVEmu.
-- Generated from the user's November 2026 typeID-specific SQL export,
-- cross-checked against CCP Crucible notes and historical ship tables.
-- Do NOT execute the INSERT INTO UnknownTable display exports: they were
-- SELECT result dumps, not the target schema.
--
-- CHANGE SCOPE: 98 published hull typeIDs only; guard each against its
-- observed original raw attribute 600. Any externally changed values are
-- SKIPPED, not overwritten. Existing ship item overrides are not altered.
--
-- CRITICAL: stop server, back up dgmTypeAttributes and entity_attributes.
-- Run the PREVIEW. Only then run this script on your staging DB first.
-- Sources and unresolved typeIDs: docs/CRUCIBLE_RANGE_AND_WARP_AUDIT_2026-11-17.md.
DROP TEMPORARY TABLE IF EXISTS tmp_crucible_warp_fix;
CREATE TEMPORARY TABLE tmp_crucible_warp_fix (
  typeID INT UNSIGNED NOT NULL PRIMARY KEY,
  expected_old DOUBLE NULL,
  corrected_au DOUBLE NOT NULL,
  reference_note VARCHAR(120) NOT NULL
);
INSERT INTO tmp_crucible_warp_fix (typeID,expected_old,corrected_au,reference_note)
VALUES
  (620,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (621,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (622,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (623,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (624,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (625,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (626,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (627,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (628,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (629,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (630,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (631,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (632,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (633,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (634,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (635,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (648,0.75,4.50,'2012-era standard industrial 4.5 AU/s'),
  (649,0.75,4.50,'2012-era standard industrial 4.5 AU/s'),
  (650,0.75,4.50,'2012-era standard industrial 4.5 AU/s'),
  (651,0.75,4.50,'2012-era standard industrial 4.5 AU/s'),
  (652,0.75,4.50,'2012-era standard industrial 4.5 AU/s'),
  (653,0.75,4.50,'2012-era standard industrial 4.5 AU/s'),
  (654,0.75,4.50,'2012-era standard industrial 4.5 AU/s'),
  (655,0.75,4.50,'2012-era standard industrial 4.5 AU/s'),
  (656,0.75,4.50,'2012-era standard industrial 4.5 AU/s'),
  (657,0.75,4.50,'2012-era standard industrial 4.5 AU/s'),
  (1944,0.75,4.50,'2012-era standard industrial 4.5 AU/s'),
  (2006,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (4302,NULL,3.00,'2012-era battlecruiser 3 AU/s'),
  (4306,NULL,3.00,'2012-era battlecruiser 3 AU/s'),
  (4308,NULL,3.00,'2012-era battlecruiser 3 AU/s'),
  (4310,NULL,3.00,'2012-era battlecruiser 3 AU/s'),
  (11011,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (11176,13.5,9.00,'historical combat interceptor 9 AU/s'),
  (11184,13.5,9.00,'historical combat interceptor 9 AU/s'),
  (11196,13.5,9.00,'historical combat interceptor 9 AU/s'),
  (11200,13.5,9.00,'historical combat interceptor 9 AU/s'),
  (11365,13.5,6.00,'historical standard assault frigate 6 AU/s'),
  (11371,13.5,6.00,'historical standard assault frigate 6 AU/s'),
  (11379,13.5,6.00,'historical standard assault frigate 6 AU/s'),
  (11381,13.5,6.00,'historical standard assault frigate 6 AU/s'),
  (11393,13.5,6.00,'historical standard assault frigate 6 AU/s'),
  (11400,13.5,6.00,'historical standard assault frigate 6 AU/s'),
  (11957,1.25,3.75,'contemporary Recon cruiser 3.75 AU/s'),
  (11959,1.25,3.75,'contemporary Recon cruiser 3.75 AU/s'),
  (11961,1.25,3.75,'contemporary Recon cruiser 3.75 AU/s'),
  (11963,1.25,3.75,'contemporary Recon cruiser 3.75 AU/s'),
  (11965,1.25,3.75,'contemporary Recon cruiser 3.75 AU/s'),
  (11969,1.25,3.75,'contemporary Recon cruiser 3.75 AU/s'),
  (11971,1.25,3.75,'contemporary Recon cruiser 3.75 AU/s'),
  (11978,9,3.75,'CCP Crucible Logistics 3.75 AU/s'),
  (11985,9,3.75,'CCP Crucible Logistics 3.75 AU/s'),
  (11987,9,3.75,'CCP Crucible Logistics 3.75 AU/s'),
  (11989,9,3.75,'CCP Crucible Logistics 3.75 AU/s'),
  (11995,9,3.75,'historical Heavy Interdictor 3.75 AU/s'),
  (12013,9,3.75,'historical Heavy Interdictor 3.75 AU/s'),
  (12017,9,3.75,'historical Heavy Interdictor 3.75 AU/s'),
  (12021,9,3.75,'historical Heavy Interdictor 3.75 AU/s'),
  (12042,13.5,6.00,'historical standard assault frigate 6 AU/s'),
  (12044,13.5,6.00,'historical standard assault frigate 6 AU/s'),
  (12729,0.75,9.00,'historical blockade runner 9 AU/s'),
  (12731,0.75,3.00,'historical deep-space transport 3 AU/s'),
  (12733,0.75,9.00,'historical blockade runner 9 AU/s'),
  (12735,0.75,9.00,'historical blockade runner 9 AU/s'),
  (12743,0.75,9.00,'historical blockade runner 9 AU/s'),
  (12745,0.75,3.00,'historical deep-space transport 3 AU/s'),
  (12747,0.75,3.00,'historical deep-space transport 3 AU/s'),
  (12753,0.75,3.00,'historical deep-space transport 3 AU/s'),
  (16227,NULL,3.00,'2012-era battlecruiser 3 AU/s'),
  (16229,NULL,3.00,'2012-era battlecruiser 3 AU/s'),
  (16231,NULL,3.00,'2012-era battlecruiser 3 AU/s'),
  (16233,NULL,3.00,'2012-era battlecruiser 3 AU/s'),
  (16236,NULL,3.00,'2012-era destroyer 3 AU/s'),
  (16238,NULL,3.00,'2012-era destroyer 3 AU/s'),
  (16240,NULL,3.00,'2012-era destroyer 3 AU/s'),
  (16242,NULL,3.00,'2012-era destroyer 3 AU/s'),
  (17634,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (17709,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (17713,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (17715,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (17718,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (17720,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (17722,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (17843,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (17922,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (20125,1.25,3.75,'contemporary Recon cruiser 3.75 AU/s'),
  (24696,NULL,3.00,'2012-era battlecruiser 3 AU/s'),
  (24698,NULL,3.00,'2012-era battlecruiser 3 AU/s'),
  (24700,NULL,3.00,'2012-era battlecruiser 3 AU/s'),
  (24702,NULL,3.00,'2012-era battlecruiser 3 AU/s'),
  (28659,13.5,3.00,'historical Marauder 3 AU/s'),
  (28661,13.5,3.00,'historical Marauder 3 AU/s'),
  (28665,13.5,3.00,'historical Marauder 3 AU/s'),
  (28710,13.5,3.00,'historical Marauder 3 AU/s'),
  (29336,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (29337,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (29340,NULL,3.00,'2012-era T1 cruiser 3 AU/s'),
  (29344,NULL,3.00,'2012-era T1 cruiser 3 AU/s');
-- PREVIEW: expected_old NULL means the attribute must be absent in dgmTypeAttributes.
SELECT t.typeID,t.typeName,g.groupName,v.expected_old,
       COALESCE(a.valueFloat,a.valueInt) AS current_raw_attribute_600,
       v.corrected_au AS requested_crucible_au_per_s,
       v.reference_note,
       CASE
         WHEN a.typeID IS NULL AND v.expected_old IS NULL THEN 'READY_INSERT'
         WHEN a.typeID IS NOT NULL AND
              COALESCE(a.valueFloat,a.valueInt)=v.corrected_au THEN 'ALREADY_CORRECT'
         WHEN a.typeID IS NOT NULL AND v.expected_old IS NOT NULL
              AND COALESCE(a.valueFloat,a.valueInt)=v.expected_old THEN 'READY_UPDATE'
         ELSE 'SKIP_CHANGED_OR_UNEXPECTED'
       END AS action
FROM tmp_crucible_warp_fix v
JOIN invTypes t ON t.typeID=v.typeID AND t.published=1
JOIN invGroups g ON g.groupID=t.groupID AND g.categoryID=6
LEFT JOIN dgmTypeAttributes a ON a.typeID=v.typeID AND a.attributeID=600
ORDER BY g.groupName,t.typeName;
--
-- APPLY. Only pre-reviewed original values, only published ship typeIDs.
-- Both UPDATE and INSERT are in one transaction; idempotent if rerun.
START TRANSACTION;
UPDATE dgmTypeAttributes a
JOIN tmp_crucible_warp_fix v ON v.typeID=a.typeID
JOIN invTypes t ON t.typeID=v.typeID AND t.published=1
JOIN invGroups g ON g.groupID=t.groupID AND g.categoryID=6
SET a.valueInt=NULL, a.valueFloat=v.corrected_au
WHERE a.attributeID=600
  AND v.expected_old IS NOT NULL
  AND COALESCE(a.valueFloat,a.valueInt)=v.expected_old;
SELECT ROW_COUNT() AS verified_warp_attributes_updated;
INSERT INTO dgmTypeAttributes (typeID,attributeID,valueInt,valueFloat)
SELECT v.typeID,600,NULL,v.corrected_au
FROM tmp_crucible_warp_fix v
JOIN invTypes t ON t.typeID=v.typeID AND t.published=1
JOIN invGroups g ON g.groupID=t.groupID AND g.categoryID=6
LEFT JOIN dgmTypeAttributes a ON a.typeID=v.typeID AND a.attributeID=600
WHERE v.expected_old IS NULL AND a.typeID IS NULL;
SELECT ROW_COUNT() AS verified_warp_attributes_inserted;
COMMIT;
-- VERIFY: values still unexpected. Should be zero for the approved 98
-- hull typeIDs, otherwise review rows that were deliberately skipped.
SELECT t.typeID,t.typeName,g.groupName,
       COALESCE(a.valueFloat,a.valueInt) AS current_au_per_second,
       v.corrected_au, v.reference_note
FROM tmp_crucible_warp_fix v
JOIN invTypes t ON t.typeID=v.typeID AND t.published=1
JOIN invGroups g ON g.groupID=t.groupID AND g.categoryID=6
LEFT JOIN dgmTypeAttributes a ON a.typeID=v.typeID AND a.attributeID=600
WHERE a.typeID IS NULL OR COALESCE(a.valueFloat,a.valueInt)<>v.corrected_au
ORDER BY t.typeID;
DROP TEMPORARY TABLE tmp_crucible_warp_fix;
