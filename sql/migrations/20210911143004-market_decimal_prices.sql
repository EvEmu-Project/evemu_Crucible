
-- +migrate Up
ALTER TABLE mktOrders
  MODIFY `price` decimal(16,2) NOT NULL DEFAULT '0';

-- +migrate Down
ALTER TABLE mktOrders
  MODIFY `price` float NOT NULL DEFAULT '0';
