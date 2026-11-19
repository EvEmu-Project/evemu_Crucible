# Docker/MariaDB market loading performance — 19 November 2026

**Code branch:** `contrib/upstream-crucible-maintenance-2026-11-15` (upstream PR #335). **Reported symptom:** a user says the Crucible market window takes a long time to load. No production SQL timings, Docker resource limits, server logs or client packet captures were provided; therefore this is a source-based bottleneck investigation **not a confirmed performance fix in that user's environment**.

## Traced request path

The client market UI calls `MarketProxyService::GetStationAsks`, `GetSystemAsks`, `GetRegionBest` and `GetOrders(typeID)`. The first three delegate to `MarketDB.cpp` best-ask queries, each with nested `MIN(price)` per `typeID`, an additional `MIN(orderID)` tie break and joins over `mktOrders`; this is costly on large seeded regional order books. `GetOrders` performs **separate sell and buy queries** filtered by `regionID,typeID,bid`, then marshals both result sets to Crucible rowsets and caches them by region and type. Those shapes benefit from composite lookups. There is no client pagination or server-side type-independent summary cache; do not change order-list visibility just to reduce query latency.

The existing `202406242143-mktOrderIndex` migration creates only `solarSystemID`. A single-column index cannot cover the additional `bid`, `typeID` and lowest-price grouping. The new **optional/additive** migration `20261119113000-market-order-lookup-indexes.sql` defines:
- `ix_mkt_region_book_v1(regionID,bid,typeID,price,orderID)`
- `ix_mkt_station_asks_v1(stationID,bid,typeID,price,orderID)`
- `ix_mkt_system_asks_v1(solarSystemID,bid,typeID,price,orderID)`
- `ix_mkt_owner_expiry_v1(ownerID,issued)`

All four are `CREATE INDEX IF NOT EXISTS`, with matching `DROP INDEX IF EXISTS` rollback. They make **no changes to orders, prices, volume, player balances, NPC seed quantities or row data**. On large tables, index creation itself can use substantial CPU, disk and I/O; back up first and schedule during low traffic. Avoid creating redundant indexes with different names if your deployed schema already has equivalent composite keys; compare `SHOW INDEX` first.

## Trader Joe cleanup and Docker startup

`MarketBotMgr::Process` used to call `ExpireOldOrders()` twice per refresh. Each pass scanned `mktOrders` and deleted eligible IDs individually. This patch runs it once and batches up to 128 original candidate IDs in one DELETE, **rechecking owner, existing 550-unit exclusion and expiry**. If another actor refreshes an order between SELECT and DELETE it is not deleted. Batch logs report processed candidates, not exact affected row count. No player orders are selected.

`docker-compose.yml` now waits for the official MariaDB 11.8 container's `healthcheck.sh --connect --innodb_initialized` before starting the server. This addresses initialization races, not ongoing database latency. DB files remain in the existing **named persistent volume**. The Dockerfile still uses Debian 12 with the server and patched EVEDBTool; it does not embed a production database in the server image. No unmeasured buffer-pool size, query-cache switch or risky MariaDB tuning was applied.

## Read-only diagnosis on the affected deployment

Use `docker compose ps`, `docker stats`, `docker compose logs --since=15m db server` to inspect DB readiness, CPU, RSS, I/O and connection errors. Run SQL using a **read-only database session** (substitute your DB user/schema as appropriate):

```sql
SELECT VERSION();
SHOW GLOBAL STATUS LIKE 'Threads_running';
SHOW GLOBAL STATUS LIKE 'Slow_queries';
SHOW GLOBAL STATUS LIKE 'Created_tmp_disk_tables';
SHOW GLOBAL STATUS LIKE 'Innodb_buffer_pool_reads';
SHOW GLOBAL VARIABLES LIKE 'innodb_buffer_pool_size';

SELECT regionID, COUNT(*) AS orders, COUNT(DISTINCT typeID) AS types
FROM mktOrders GROUP BY regionID ORDER BY orders DESC LIMIT 12;

SHOW INDEX FROM mktOrders;

EXPLAIN SELECT typeID, MIN(price)
FROM mktOrders
WHERE regionID = 10000002 AND bid = 0
GROUP BY typeID;

EXPLAIN SELECT price, volRemaining, typeID, orderID, stationID
FROM mktOrders
WHERE regionID = 10000002 AND typeID = 34 AND bid = 0;

EXPLAIN SELECT typeID, MIN(price)
FROM mktOrders
WHERE solarSystemID = 30000142 AND bid = 0
GROUP BY typeID;

SHOW FULL PROCESSLIST;
```

Replace region/system/type IDs with the **actual affected market**. `EXPLAIN` is a plan, not elapsed time; compare `rows`, `key`, `Extra` and measured response time before and after applying indexes. Check `SHOW INDEX` and `information_schema.STATISTICS` first. The MariaDB slow query log is best enabled temporarily in staging or a deliberately scoped production maintenance window, then switched off; avoid printing user order history or passwords in public logs.

For reproducible client measurements record: station/system/region, item type, cache-cold vs warm request, number of orders, Docker host CPU/RAM/disk latency, MariaDB `EXPLAIN`, server market request timing and client elapsed seconds. A cold request may still spend time serializing very large order rowsets even with fast SQL; compare both **database query duration** and **client/network serialization** before attributing all delay to MariaDB.

## Deployment and rollback

1. Backup MariaDB and verify restoration in staging; do not wipe the `db` named volume or run `docker compose down -v`.
2. Review indexes already installed and free disk space. Compare query plans on the same dataset.
3. Apply the **Up** section of `sql/migrations/20261119113000-market-order-lookup-indexes.sql` once using the project's usual SQL migration tool. The commit alone **does not apply indexes to a live DB**.
4. Rebuild/redeploy the updated server image and Compose config. The bot cleanup improvement takes effect after restart.
5. Compare slow-query counts, orderbook query latency, bot refresh duration, market window open time, and sample **player/NPC order counts and balances**. Also test ordinary buys, sells and the contribution's **100,000,000-unit** mineral/ore/ice orders.
6. On regression, revert server image. Remove only the named new indexes via the reviewed migration Down section if necessary; **do not roll back data by dropping/truncating orders**. Inspect DBA changes before dropping any pre-existing same-named index.

## Verification scope

The Linux/Windows source guard `tools/verify_market_performance.py` checks index definitions, original lookup predicates and caching, single bot sweep, safe batched delete filters and Docker health readiness. The MariaDB CI job applies the index migration twice to its synthetic seeded fixture, checks the four index names, confirms player-order count, and runs representative `EXPLAIN` statements. This catches structural regressions but **cannot establish a production speedup without real timing data**. The long-running contribution branch has now been added explicitly to the fork's push trigger. [CI #44498635401](https://github.com/xrizonx/evemu_Crucible/actions/runs/44498635401) **passed** Docker, Linux/Windows GCC and CTest, and MariaDB index/seed tests. The test EXPLAIN output selected `ix_mkt_region_book_v1` and `ix_mkt_station_asks_v1`. No real-user market latency has been measured.

See [upstream handover](UPSTREAM_DEVELOPER_HANDOVER_2026-11-19.md) and [market volume policy](NPC_ORE_MINERAL_100M_ORDERS_2026-11-18.md).
