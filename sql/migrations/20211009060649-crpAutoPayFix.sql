
-- +migrate Up
-- remove any null values
UPDATE crpAutoPay SET market = false WHERE market IS NULL;
UPDATE crpAutoPay SET rental = false WHERE rental IS NULL;
UPDATE crpAutoPay SET broker = false WHERE broker IS NULL;
UPDATE crpAutoPay SET war = false WHERE war IS NULL;
UPDATE crpAutoPay SET alliance = false WHERE alliance IS NULL;
UPDATE crpAutoPay SET sov = false WHERE sov IS NULL;
-- set the new table structure
ALTER TABLE crpAutoPay
    MODIFY `corporationID` int(10) NOT NULL PRIMARY KEY,
    MODIFY `market` tinyint(1) NOT NULL DEFAULT '0',
    MODIFY `rental` tinyint(1) NOT NULL DEFAULT '0',
    MODIFY `broker` tinyint(1) NOT NULL DEFAULT '0',
    MODIFY `war` tinyint(1) NOT NULL DEFAULT '0',
    MODIFY `alliance` tinyint(1) NOT NULL DEFAULT '0',
    MODIFY `sov` tinyint(1) NOT NULL DEFAULT '0';

-- +migrate Down
-- remove primary key
ALTER TABLE crpAutoPay DROP PRIMARY KEY;
ALTER TABLE crpAutoPay
    MODIFY `corporationID` int(10),
    MODIFY `market` tinyint(1),
    MODIFY `rental` tinyint(1),
    MODIFY `broker` tinyint(1),
    MODIFY `war` tinyint(1),
    MODIFY `alliance` tinyint(1),
    MODIFY `sov` tinyint(1);