-- Adds changes for alliance implementation
-- +migrate Up
ALTER TABLE crpCorporation
ADD COLUMN `chosenExecutorID` int(10) NULL DEFAULT 0 AFTER allianceID;
ALTER TABLE alnAlliance
ADD COLUMN `description` varchar(400) NOT NULL DEFAULT '' AFTER allianceName;
ALTER TABLE alnAlliance
MODIFY `startDate` bigint(20) NOT NULL;
ALTER TABLE alnAlliance AUTO_INCREMENT = 99000000;
ALTER TABLE alnBulletins AUTO_INCREMENT = 100000;
-- +migrate Down
ALTER TABLE crpCorporation
DROP COLUMN `chosenExecutorID`;
ALTER TABLE alnAlliance
DROP COLUMN `description`;
ALTER TABLE alnAlliance
MODIFY `startDate` int(20) NOT NULL;
ALTER TABLE alnAlliance AUTO_INCREMENT = 1;
ALTER TABLE alnBulletins AUTO_INCREMENT = 1;