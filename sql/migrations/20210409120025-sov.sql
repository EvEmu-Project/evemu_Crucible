-- Adds changes for sov implementation
-- +migrate Up
ALTER TABLE posStructureData
CHANGE COLUMN `moonID` `anchorpointID` int(10);
ALTER TABLE mapSystemSovInfo
MODIFY COLUMN `claimTime` bigint(20) NULL DEFAULT 0 AFTER claimStructureID;
ALTER TABLE mapSystemSovInfo
ADD claimID int NOT NULL AUTO_INCREMENT PRIMARY KEY FIRST;
-- +migrate Down
ALTER TABLE posStructureData
CHANGE COLUMN `anchorpointID` `moonID` int(10);
ALTER TABLE mapSystemSovInfo
MODIFY COLUMN `claimTime` int(20) NULL AFTER claimStructureID;
ALTER TABLE mapSystemSovInfo
DROP COLUMN claimID;
