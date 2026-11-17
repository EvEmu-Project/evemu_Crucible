"""Prevent NPC market price markups and destructive broad repricing.

Static/source regression checks; the operator must still run the read-only
preview and NPC-only SQL repair on a backed-up staging MariaDB instance.
"""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
read = lambda name: (root / name).read_text(encoding="utf-8")

go = read("tools/evedbtool/command_seed.go")
# Two sell paths and the new published-market NPC buy path must agree.
assert go.count("ELSE GREATEST(0.01, ROUND(it.basePrice, 2))") == 5
assert "it.basePrice / stations.security" not in go
assert "WHEN stations.security <= 0 THEN it.basePrice" not in go
assert "oldOrder.bid = 0" in go
assert "station.corporationID = oldOrder.ownerID" in go

bot = read("src/eve-server/market/MarketBotMgr.cpp")
sell = bot.split("double MarketBotMgr::CalculateSellPrice(", 1)[1].split("\n}", 1)[0]
assert "GetRandomFloat" not in sell
assert "type->basePrice()" in sell
assert "std::max(0.01" in sell

regional = read("sql/seed_and_clean/seed_market.sql")
station = read("sql/seed_and_clean/seed_station_market.sql")
for sql in (regional, station):
    assert "basePrice / security" not in sql
    assert "basePrice / stations.security" not in sql
    assert "UPDATE mktOrders SET price = 100 WHERE price = 0" not in sql
    assert "GREATEST(0.01, ROUND(basePrice, 2))" in sql or "GREATEST(0.01, ROUND(it.basePrice, 2))" in sql

normalizer = read("sql/diagnostics/2026-11-16-normalize-npc-sell-orders-100-percent.sql")
assert "START TRANSACTION;" in normalizer and "COMMIT;" in normalizer
assert "o.bid = 0" in normalizer
assert "o.ownerID IN (st.corporationID, st.stationID, 1000125)" in normalizer
assert "JOIN invTypes AS t ON t.typeID = o.typeID" in normalizer
assert "o.price <> CASE" in normalizer
assert "DELETE FROM mktOrders" not in normalizer
assert "UPDATE invTypes" not in normalizer
assert "UPDATE mktOrders SET price" not in normalizer

print("NPC 100%-base-price market source checks passed")
