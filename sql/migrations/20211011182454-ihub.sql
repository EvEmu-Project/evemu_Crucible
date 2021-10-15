-- Implement Infrastructure Hubs
-- +migrate Up
UPDATE dgmTypeAttributes
SET valueFloat=300000 WHERE attributeID=556 AND typeID=32458;
UPDATE dgmTypeAttributes
SET valueFloat=300000 WHERE attributeID=676 AND typeID=32458;

-- +migrate Down
UPDATE dgmTypeAttributes
SET valueFloat=3600000 WHERE attributeID=556 AND typeID=32458;
UPDATE dgmTypeAttributes
SET valueFloat=3600000 WHERE attributeID=676 AND typeID=32458;
