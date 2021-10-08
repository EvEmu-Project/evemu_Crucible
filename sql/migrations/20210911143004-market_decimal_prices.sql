-- Replaces float with decimal price storage for market orders. This migration will truncate the table, losing any data which was there before.
-- +migrate Up
TRUNCATE mktOrders;
DROP TABLE IF EXISTS seed_migrations;
ALTER TABLE mktOrders
  MODIFY `price` decimal(16,2) NOT NULL DEFAULT '0';

-- +migrate Down
TRUNCATE mktOrders;
DROP TABLE IF EXISTS seed_migrations;
ALTER TABLE mktOrders
  MODIFY `price` float NOT NULL DEFAULT '0';
