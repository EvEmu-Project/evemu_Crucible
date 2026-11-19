-- Additive MariaDB market lookup indexes.
-- The 202406 solarSystem index does not cover bid/type/price grouping.
-- On large production order books, schedule this DDL off-peak after a backup.
-- +migrate Up
CREATE INDEX IF NOT EXISTS ix_mkt_region_book_v1 ON mktOrders (regionID, bid, typeID, price, orderID);
CREATE INDEX IF NOT EXISTS ix_mkt_station_asks_v1 ON mktOrders (stationID, bid, typeID, price, orderID);
CREATE INDEX IF NOT EXISTS ix_mkt_system_asks_v1 ON mktOrders (solarSystemID, bid, typeID, price, orderID);
CREATE INDEX IF NOT EXISTS ix_mkt_owner_expiry_v1 ON mktOrders (ownerID, issued);

-- +migrate Down
-- Non-destructive index rollback only; never truncate or reset orders.
DROP INDEX IF EXISTS ix_mkt_owner_expiry_v1 ON mktOrders;
DROP INDEX IF EXISTS ix_mkt_system_asks_v1 ON mktOrders;
DROP INDEX IF EXISTS ix_mkt_station_asks_v1 ON mktOrders;
DROP INDEX IF EXISTS ix_mkt_region_book_v1 ON mktOrders;
