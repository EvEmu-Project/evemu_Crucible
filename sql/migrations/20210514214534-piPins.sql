
-- +migrate Up
ALTER TABLE piPins
ADD COLUMN qtyPerCycle INT(20) AFTER headRadius;

-- +migrate Down
ALTER TABLE piPins
DROP COLUMN qtyPerCycle;


