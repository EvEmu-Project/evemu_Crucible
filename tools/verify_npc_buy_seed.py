#!/usr/bin/env python3
"""Source safety checks for NPC buy orders at 100% static base price.

The operator must still verify SQL against MariaDB 11.8 and a Crucible client.
"""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
go = (root / "tools/evedbtool/command_seed.go").read_text(encoding="utf-8")
bot = (root / "src/eve-server/market/MarketBotMgr.cpp").read_text(encoding="utf-8")
mgr = (root / "src/eve-server/market/MarketMgr.cpp").read_text(encoding="utf-8")
market_types = (root / "src/eve-common/EVE_Market.h").read_text(encoding="utf-8")
market_db = (root / "src/eve-server/market/MarketDB.cpp").read_text(encoding="utf-8")
workflow = (root / ".github/workflows/build-release.yml").read_text(encoding="utf-8")
audit = (root / "sql/diagnostics/2026-11-17-npc-buy-orders-100pct-audit.sql").read_text(encoding="utf-8")

# Standard seed and existing installation migrations must coexist.
assert 'cmdFlags.BoolVar(&buyOnly, "buy-only"' in go
assert '"SEED_MARKET"' in go and '"SEED_NPC_BUY_100PCT"' in go
assert "buyQueries = append(buyQueries, getBuyQuery(regionID, saturation, force))" in go
assert "if !buyOnly {" in go
assert "func getBuyQuery(" in go
buy = go.split("func getBuyQuery(", 1)[1]
assert "o.bid = 1" in buy and "o.ownerID = 4000000" in buy
assert "UPDATE mktOrders AS legacy" in buy
assert "legacy.ownerID = 1000125" in buy
assert "legacy.ownerID = 4000000" in buy
assert "legacy.bid = 1" in buy and "legacy.isCorp = 0" in buy
assert "legacy.orderRange = 32767" in buy
assert "legacy.regionID = @regionid" in buy
assert "legacy.escrow = (CASE" in buy
assert "oldOrder.ownerID = 4000000" in buy
assert "oldOrder.bid = 1" in buy and "oldOrder.isCorp = 0" in buy
assert "oldOrder.orderRange = -1" in buy and "oldOrder.duration = 250" in buy
assert "it.published = 1 AND it.marketGroupID IS NOT NULL" in buy
assert "CASE WHEN it.basePrice IS NULL OR it.basePrice <= 0 THEN 100.00" in buy
assert "ELSE GREATEST(0.01, ROUND(it.basePrice, 2))" in buy
assert "eligible.referencePrice * eligible.quantity" in buy
assert "10000000000000.00 / priced.referencePrice" in buy
assert "existing.stationID IS NULL" in buy
assert "tExistingNPCBuySeed" in buy
assert "AND RAND() < @saturation" in buy
assert "DELETE FROM mktOrders;" not in buy
assert "UPDATE chrCharacters" not in buy
assert "UPDATE crpWallet" not in buy

buyPrice = bot.split("double MarketBotMgr::CalculateBuyPrice(", 1)[1].split("\n}", 1)[0]
assert "order.ownerID = BOT_BUY_OWNER_ID;" in bot
assert "static constexpr uint32 BOT_BUY_OWNER_ID = npcTraderJoe;" in bot
assert "GetRandomFloat" not in buyPrice
assert "type->basePrice()" in buyPrice and "std::max(0.01" in buyPrice
assert ": 100.0" in buyPrice

sale = mgr.split("bool MarketMgr::ExecuteBuyOrder(", 1)[1].split("// Executes a sell order.", 1)[0]
assert "const double executionPrice = (isTraderJoe || isTrader) ? oInfo.price : price" in sale
assert "const uint32 qtySold = std::min(quantity, oInfo.quantity)" in sale
assert "quantity > iRef->quantity()" in sale
assert "if (!orderConsumed)" in sale
assert "data.price          = executionPrice" in sale
assert "oInfo.typeID != typeID" in sale and "oInfo.stationID != stationID" in sale
assert "Check corporation wallet authorization before an inventory transfer" in sale
assert "if (shouldDeleteItem)" in sale
assert market_types.count("double price;") >= 3
assert "double escrow;" in market_types
assert "oInfo.price      = row.GetDouble(1);" in market_db
assert "sellerWalletOwnerID = useCorp ?" in sale

assert workflow.count("verify_npc_buy_seed.py") == 2
assert "SELECT" in audit and "DELETE " not in audit and "UPDATE " not in audit
assert "SET @npc_owner_id = 4000000" in audit
print("PASS: NPC buy migration, idempotence, pricing, settlement, and read-only audit invariants")
