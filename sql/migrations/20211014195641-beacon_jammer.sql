-- Add columns to sovereignty table for cyno generators and jammers
-- +migrate Up
ALTER TABLE mapSystemSovInfo
ADD COLUMN beaconID INT(10) NOT NULL DEFAULT 0 AFTER hubID;
ALTER TABLE mapSystemSovInfo
ADD COLUMN jammerID INT(10) NOT NULL DEFAULT 0 AFTER beaconID;

-- +migrate Down
ALTER TABLE mapSystemSovInfo
DROP COLUMN beaconID;
ALTER TABLE mapSystemSovInfo
DROP COLUMN jammerID;