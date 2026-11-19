#!/usr/bin/env python3
"""Source-level guards for safe Docker/MariaDB market performance improvements."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
read = lambda name: (root / name).read_text(encoding="utf-8")
migration = read("sql/migrations/20261119113000-market-order-lookup-indexes.sql")
bot = read("src/eve-server/market/MarketBotMgr.cpp")
db = read("src/eve-server/market/MarketDB.cpp")
proxy = read("src/eve-server/market/MarketProxyService.cpp")
compose = read("docker-compose.yml")
workflow = read(".github/workflows/build-release.yml")
up, down = migration.split("-- +migrate Up", 1)[1].split("-- +migrate Down", 1)

indexes = {
    "ix_mkt_region_book_v1": "(regionID, bid, typeID, price, orderID)",
    "ix_mkt_station_asks_v1": "(stationID, bid, typeID, price, orderID)",
    "ix_mkt_system_asks_v1": "(solarSystemID, bid, typeID, price, orderID)",
    "ix_mkt_owner_expiry_v1": "(ownerID, issued)",
}
for name, fields in indexes.items():
    assert f"CREATE INDEX IF NOT EXISTS {name}" in up and fields in up
    assert f"DROP INDEX IF EXISTS {name}" in down
assert not any(dml in up.upper() for dml in ("DELETE FROM", "TRUNCATE", "DROP TABLE"))
for key in ("stationID", "solarSystemID", "regionID"):
    assert f"WHERE {key}=%u AND bid=%u GROUP BY typeID" in db
assert db.count("WHERE regionID=%u AND typeID=%u AND bid=%u") == 2
assert "ObjectCachedMethodID method_id(GetName().c_str(), method_name.c_str());" in proxy
process = bot.split("void MarketBotMgr::Process(bool overrideTimer)", 1)[1].split(
    "void MarketBotMgr::ForceRun", 1)[0]
assert process.count("ExpireOldOrders();") == 1
expiry = bot.split("int MarketBotMgr::ExpireOldOrders()", 1)[1].split(
    "int MarketBotMgr::PlaceBuyOrders", 1)[0]
assert "constexpr size_t kDeleteBatchSize = 128;" in expiry
assert "DELETE FROM mktOrders WHERE orderID IN (%s)" in expiry
assert expiry.count("ownerID IN (%u, %u) AND volEntered != 550") == 2
assert expiry.count("FILETIME_TICKS_PER_DAY, now") == 2
assert "MarketDB::DeleteOrder(orderID)" not in expiry
assert "condition: service_healthy" in compose
assert "healthcheck.sh" in compose and "--innodb_initialized" in compose
assert workflow.count("tools/verify_market_performance.py") == 2
assert "20261119113000-market-order-lookup-indexes.sql" in workflow
print("PASS: composite order indexes, bounded bot cleanup, Docker DB readiness")
