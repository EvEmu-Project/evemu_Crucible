-- EVEmu Crucible: one-time normalization of EXISTING NPC SELL orders.
-- 100% value is invTypes.basePrice, stored in mktOrders.price DECIMAL(16,2).
-- A nonpositive/NULL basePrice has no percentage meaning: fall back to 100 ISK.
-- For positive values smaller than 0.01 ISK use the minimum market tick 0.01.
--
-- IMPORTANT: Review the SELECT preview and make a MariaDB backup first.
-- Run while the EVEmu server/market bot is stopped, then restart the server
-- to clear cached order views. Does not modify player orders, buy orders,
-- quantities, wallets, escrow, historical transactions or invTypes.basePrice.
-- Authorized NPC sellers:
--  1. staStations.corporationID (standard seeded station NPC corporation)
--  2. staStations.stationID (legacy seed_station_market.sql orders)
--  3. 1000125 (hard-coded Trader Joe/CONCORD market-bot owner)
-- This intentionally does NOT match all corporation-owned orders.
--
-- PREVIEW: counts and current markup/discount for NPC sell orders.
SELECT
    COUNT(*) AS npc_sell_orders,
    SUM(o.price > CASE
                    WHEN t.basePrice IS NULL OR t.basePrice <= 0 THEN 100.00
                    ELSE GREATEST(0.01, ROUND(t.basePrice, 2))
                  END) AS above_reference,
    SUM(o.price < CASE
                    WHEN t.basePrice IS NULL OR t.basePrice <= 0 THEN 100.00
                    ELSE GREATEST(0.01, ROUND(t.basePrice, 2))
                  END) AS below_reference,
    SUM(o.price <> CASE
                    WHEN t.basePrice IS NULL OR t.basePrice <= 0 THEN 100.00
                    ELSE GREATEST(0.01, ROUND(t.basePrice, 2))
                  END) AS orders_to_update
FROM mktOrders AS o
JOIN staStations AS st ON st.stationID = o.stationID
JOIN invTypes AS t ON t.typeID = o.typeID
WHERE o.bid = 0
  AND o.ownerID IN (st.corporationID, st.stationID, 1000125);

-- PREVIEW: the 50 greatest markups, grouped by the individual order.
SELECT o.orderID, o.ownerID, o.stationID, o.typeID, t.typeName,
       o.price AS current_price,
       CASE WHEN t.basePrice IS NULL OR t.basePrice <= 0 THEN 100.00
            ELSE GREATEST(0.01, ROUND(t.basePrice, 2))
       END AS reference_sell_price,
       ROUND(o.price / NULLIF(t.basePrice, 0) * 100, 2) AS base_price_percent
FROM mktOrders AS o
JOIN staStations AS st ON st.stationID = o.stationID
JOIN invTypes AS t ON t.typeID = o.typeID
WHERE o.bid = 0
  AND o.ownerID IN (st.corporationID, st.stationID, 1000125)
  AND t.basePrice > 0
ORDER BY base_price_percent DESC
LIMIT 50;

-- APPLY: only NPC sell orders, in all regions, at 100% reference price.
-- Execute only after reviewing the preview, taking a backup and stopping
-- the live server; this updates prices but DOES NOT replace/delete orders.
START TRANSACTION;

UPDATE mktOrders AS o
JOIN staStations AS st ON st.stationID = o.stationID
JOIN invTypes AS t ON t.typeID = o.typeID
SET o.price = CASE
    WHEN t.basePrice IS NULL OR t.basePrice <= 0 THEN 100.00
    ELSE GREATEST(0.01, ROUND(t.basePrice, 2))
END
WHERE o.bid = 0
  AND o.ownerID IN (st.corporationID, st.stationID, 1000125)
  AND o.price <> CASE
    WHEN t.basePrice IS NULL OR t.basePrice <= 0 THEN 100.00
    ELSE GREATEST(0.01, ROUND(t.basePrice, 2))
  END;

SELECT ROW_COUNT() AS npc_sell_order_prices_updated;
COMMIT;

-- VERIFY: same scope as the preview. All prices should now match.
SELECT COUNT(*) AS remaining_mismatched_npc_sells
FROM mktOrders AS o
JOIN staStations AS st ON st.stationID = o.stationID
JOIN invTypes AS t ON t.typeID = o.typeID
WHERE o.bid = 0
  AND o.ownerID IN (st.corporationID, st.stationID, 1000125)
  AND o.price <> CASE
    WHEN t.basePrice IS NULL OR t.basePrice <= 0 THEN 100.00
    ELSE GREATEST(0.01, ROUND(t.basePrice, 2))
  END;
