-- Adds changes for sov implementation
-- +migrate Up
ALTER TABLE posStructureData
CHANGE COLUMN `moonID` `anchorpointID` int(10);
ALTER TABLE mapSystemSovInfo
MODIFY COLUMN `claimTime` bigint(20) NULL DEFAULT 0 AFTER claimStructureID;
ALTER TABLE mapSystemSovInfo
ADD claimID int NOT NULL AUTO_INCREMENT PRIMARY KEY FIRST;
ALTER TABLE mapSystemSovInfo
MODIFY COLUMN `contested` tinyint(1) NULL DEFAULT 0 AFTER hubID;
-- +migrate Down
ALTER TABLE posStructureData
CHANGE COLUMN `anchorpointID` `moonID` int(10);
ALTER TABLE mapSystemSovInfo
MODIFY COLUMN `claimTime` int(20) NULL AFTER claimStructureID;
ALTER TABLE mapSystemSovInfo
DROP COLUMN claimID;
ALTER TABLE mapSystemSovInfo
MODIFY COLUMN `contested` bit(1) NULL DEFAULT b'0' AFTER hubID;