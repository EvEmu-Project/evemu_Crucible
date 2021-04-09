-- Adds changes for sov implementation
-- +migrate Up
ALTER TABLE posStructureData
CHANGE COLUMN `moonID` `anchorpointID` int(10);
-- +migrate Down
ALTER TABLE posStructureData
CHANGE COLUMN `anchorpointID` `moonID` int(10);