-- EVEmu Crucible: READ-ONLY NPC buy order seed audit (2026-11-17).
-- Does not add, update, delete, or replace any row.
-- Use MariaDB/HeidiSQL on a staging or backed-up production clone first.
-- Static NPC buy seed identity: ownerID=4000000, bid=1, orderRange=0,
-- duration=250, isCorp=0. Trader Joe's dynamic bids have other scopes.
-- The 100%-of-market-value reference here is invTypes.basePrice, NOT the
-- current regional weighted average or player market price.
SET @npc_owner_id = 4000000;

-- 0. Verify the installed range type and any incomplete legacy run.
SHOW COLUMNS FROM mktOrders LIKE 'orderRange';

SELECT regionID, ownerID, bid, orderRange, duration, COUNT(*) AS order_count
FROM mktOrders
WHERE regionID IN (10000001,10000002,10000033)
  AND ownerID IN (1000125,4000000)
GROUP BY regionID, ownerID, bid, orderRange, duration
ORDER BY regionID, ownerID, bid, orderRange, duration;

-- The table exists if EVEDBTool seed was started at least once.
SELECT id, applied_at FROM seed_migrations
WHERE id LIKE 'SEED_NPC_BUY_100PCT%'
ORDER BY applied_at DESC LIMIT 20;

-- 1. Every eligible published market-listed item type.
SELECT COUNT(*) AS eligible_market_types
FROM invTypes AS it
WHERE it.published = 1
  AND it.marketGroupID IS NOT NULL;

-- 2. Number and distribution of STATIC seeded NPC bids (all regions).
SELECT o.regionID,
       COUNT(*) AS npc_buy_orders,
       COUNT(DISTINCT o.typeID) AS unique_item_types,
       COUNT(DISTINCT o.stationID) AS covered_stations,
       SUM(o.volRemaining) AS total_remaining_units
FROM mktOrders AS o
WHERE o.ownerID = @npc_owner_id
  AND o.bid = 1 AND o.isCorp = 0
  AND o.orderRange = 0 AND o.duration = 250
GROUP BY o.regionID
ORDER BY o.regionID;

-- 3. Any NPC bid not priced at 100% of its reference value or whose
--    remaining units exceed original units / notional escrow is insufficient.
SELECT o.orderID, o.regionID, o.stationID, o.typeID, it.typeName,
       o.price AS buy_price,
       CASE WHEN it.basePrice IS NULL OR it.basePrice <= 0 THEN 100.00
            ELSE GREATEST(0.01, ROUND(it.basePrice, 2)) END AS reference_price,
       o.escrow AS original_notional_escrow,
       o.volEntered, o.volRemaining
FROM mktOrders AS o
JOIN invTypes AS it ON it.typeID = o.typeID
WHERE o.ownerID = @npc_owner_id
  AND o.bid = 1 AND o.isCorp = 0
  AND o.orderRange = 0 AND o.duration = 250
  AND (
    o.price <> CASE WHEN it.basePrice IS NULL OR it.basePrice <= 0 THEN 100.00
                    ELSE GREATEST(0.01, ROUND(it.basePrice, 2)) END
    OR o.volRemaining > o.volEntered
    OR o.escrow < o.price * o.volEntered
  )
ORDER BY o.regionID, o.stationID, o.typeID
LIMIT 100;

-- 4. Unexpected duplicate seed-managed bids at the same station/type.
SELECT o.regionID, o.stationID, o.typeID, COUNT(*) AS duplicates
FROM mktOrders AS o
WHERE o.ownerID = @npc_owner_id
  AND o.bid = 1 AND o.isCorp = 0
  AND o.orderRange = 0 AND o.duration = 250
GROUP BY o.regionID, o.stationID, o.typeID
HAVING COUNT(*) > 1
ORDER BY duplicates DESC
LIMIT 100;

-- 5. Published market-listed types with zero STATIC NPC buy orders anywhere.
--    Note: region settings and saturation intentionally limit coverage.
SELECT it.typeID, it.typeName, it.basePrice
FROM invTypes AS it
LEFT JOIN mktOrders AS o
  ON o.typeID = it.typeID AND o.bid = 1
 AND o.ownerID = @npc_owner_id AND o.isCorp = 0
 AND o.orderRange = 0 AND o.duration = 250
WHERE it.published = 1 AND it.marketGroupID IS NOT NULL
  AND o.orderID IS NULL
ORDER BY it.typeID
LIMIT 100;

-- 6. Player market orders are intentionally outside the NPC seed's scope.
--    Run before/after and verify the count does not change from seeding.
SELECT o.bid, COUNT(*) AS non_npc_order_count
FROM mktOrders AS o
WHERE o.ownerID <> @npc_owner_id
GROUP BY o.bid;

-- 7. Historical pre-fix Trader Joe orders under the unsupported 1000125
--    owner ID. The new buy seed normalizes only these regional-range bids.
--    Run before and after: this should be ZERO in seeded regions afterward.
SELECT o.regionID, COUNT(*) AS legacy_unfillable_npc_buy_orders
FROM mktOrders AS o
JOIN staStations AS st ON st.stationID = o.stationID
WHERE o.ownerID = 1000125 AND o.bid = 1 AND o.isCorp = 0
  AND o.orderRange = 32767
  AND st.regionID = o.regionID
GROUP BY o.regionID ORDER BY o.regionID;
