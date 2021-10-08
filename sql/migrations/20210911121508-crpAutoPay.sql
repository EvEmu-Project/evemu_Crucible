
-- +migrate Up
CREATE TABLE crpAutoPay (corporationID int(10), market tinyint(1), rental tinyint(1), broker tinyint(1), war tinyint(1), alliance tinyint(1), sov tinyint(1));

-- +migrate Down
DROP TABLE crpAutoPay;