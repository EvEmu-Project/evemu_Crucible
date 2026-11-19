#!/usr/bin/env python3
"""Guard 100-million-unit NPC ore/mineral buy and sell order quantities.

This verifies source wiring. The workflow MariaDB job additionally executes the
buy-only and full buy/sell seeds against a test database.
"""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
read = lambda path: (root / path).read_text(encoding="utf-8")

go = read("tools/evedbtool/command_seed.go")
bot = read("src/eve-server/market/MarketBotMgr.cpp")
region = read("sql/seed_and_clean/seed_market.sql")
station = read("sql/seed_and_clean/seed_station_market.sql")
workflow = read(".github/workflows/build-release.yml")
fixture = read("tools/tests/npc_buy_seed_unsigned_schema.sql")

groups = "(ig.groupID = 18 OR ig.groupID BETWEEN 450 AND 462 OR ig.groupID IN (465, 467, 468, 469))"
assert go.count(f"CASE WHEN {groups} THEN 100000000") == 3
assert go.count("WHEN ig.categoryID = 8 THEN 550000 ELSE 550 END") == 3
assert "CASE WHEN priced.standardQuantity = 100000000 THEN 100000000" in go
assert "AND (priced.standardQuantity <> 100000000 OR priced.referencePrice <= 10000000.00)" in go
assert "10000000000000.00 / priced.referencePrice" in go  # non-ores retain cap
assert "eligible.referencePrice * eligible.quantity" in go
assert "eligible.quantity,\n     eligible.quantity," in go

for sql in (region, station):
    expr = r"CASE WHEN \(invGroups\.groupID = 18 OR invGroups\.groupID BETWEEN 450 AND 462 OR invGroups\.groupID IN \(465, 467, 468, 469\)\) THEN 100000000"
    assert len(re.findall(expr, sql)) == 2
    assert sql.count("WHEN invGroups.categoryID = 8 THEN 550000 ELSE 550 END") == 2

assert "ORE_MINERAL_ORDER_QUANTITY = 100000000" in bot
assert "ORE_MINERAL_MAX_UNIT_PRICE = 10000000.0" in bot
assert "groupID == 18" in bot and "groupID >= 450 && groupID <= 462" in bot
assert "groupID == 465 || groupID == 467 || groupID == 468 || groupID == 469" in bot
assert "if (IsOreOrMineralMarketGroup(groupID))" in bot
assert "return ORE_MINERAL_ORDER_QUANTITY;" in bot
assert bot.count("const bool isOreMineral = IsOreOrMineralMarketGroup(type->groupID());") == 2
assert bot.count("if (isOreMineral && (!(price > 0.0) || price > ORE_MINERAL_MAX_UNIT_PRICE))") == 2
assert bot.count("if (!isOreMineral && price * quantity > sMBotConf.main.MaxISKPerOrder)") == 2
assert bot.count("order.volEntered = quantity;") == 2
assert bot.count("order.volRemaining = quantity;") == 2

assert "(18,4),(83,8),(7,7),(450,25),(465,25)" in fixture
assert "Ore Above Safety Ceiling" in fixture
assert workflow.count("verify_ore_mineral_100m.py") == 2
assert 'volEntered=100000000 AND volRemaining=100000000" "4"' in workflow
assert 'typeID=202" "0"' in workflow
assert 'ownerID=90000000" "2"' in workflow

print("PASS: mineral/ore/ice NPC buy and sell orders use 100M units with safe value ceilings")
