# NPC ore, ice and mineral orders — 100 million units

**Added 18 November 2026** for the contribution branch behind [upstream draft PR #335](https://github.com/EvEmu-Project/evemu_Crucible/pull/335).

## New order quantities

Each new **NPC buy and sell order** created by the regional EVEDBTool seed, legacy regional/single-station SQL templates, or Trader Joe market bot starts at **100,000,000 units per item type per station** for the following inventory groups:

- Minerals: `invGroups.groupID=18` (e.g. Tritanium, Pyerite).
- Ore types: groups `450–462` and `467–469` (including Arkonor, Mercoxit, etc.).
- Ice: group `465`, included with harvestable ores.

`volEntered` and `volRemaining` start at 100 million; successful trades naturally reduce `volRemaining`. Both NPC bid and ask paths use the same group selection. This is **per order** rather than a regional or type-wide total, and does not create material inventory stacks in players' hangars.

**Other items are unchanged:** regular regional sell seed 550, charge-category sell seed 550,000, rare/meta specials 10; standard NPC buy seed 550/550,000 subject to its existing 10-trillion-ISK pseudo-escrow cap; Trader Joe's non-ore groups retain random quantities and `MaxISKPerOrder` behavior. Prices remain **100% of `invTypes.basePrice`** (100 ISK fallback when nonpositive), not 100 million ISK.

## Quantity and value safeguards

- Existing **10 trillion ISK** NPC buy-order notional cap stays in force for non-mineral/non-ore items.
- NPC ore/mineral/ice bids retain exactly 100 million units when the per-unit reference price is **at most 10,000,000 ISK**. At that ceiling, the non-funded NPC pseudo-escrow is at most **1 quadrillion ISK**, within the tested `DECIMAL(20,2)` column. Exceptionally high-priced entries are skipped rather than secretly downscaled to one unit.
- The dynamic Trader Joe buy and sell placement uses the same 10 million ISK/unit limit. That high-volume exception bypasses its smaller general `MaxISKPerOrder` setting **only** for ore/mineral/ice; unrelated items still obey the configured limit.
- A very large NPC bid is a significant synthetic liquidity injection. This economy balance is appropriate only if the server operator explicitly wants effectively abundant NPC ore and mineral stock.

## Deployment and existing orders

**Code changes do not rewrite live market orders.** With the server and bot stopped and a verified full MariaDB backup:

1. Inspect `evedbtool seed -force -dryrun` and `sql/diagnostics/2026-11-17-npc-buy-orders-100pct-audit.sql` to understand affected regions and historical orders.
2. `evedbtool seed -force` intentionally refreshes both seeded NPC sell and static NPC buy orders in configured regions. It also resets other seed-managed NPC orders, so do **not** run it if you need to preserve currently posted NPC stock or partially filled seed orders.
3. `evedbtool seed -buy-only -force` replaces **only** identifiable static NPC buy bids and leaves existing sell orders untouched. Neither command updates player orders. No automatic resync of existing NPC order volume is performed at startup.
4. Deploy the updated EVEmu binary and restart the market bot for newly generated dynamic buy/sell orders to use 100 million. Previously generated Trader Joe orders keep their current quantity until they expire or are separately removed.

Validate in HeidiSQL (read only):

```sql
SELECT o.orderID, o.stationID, o.typeID, t.typeName, g.groupID,
       o.ownerID, o.bid, o.price, o.volEntered, o.volRemaining
FROM mktOrders AS o
JOIN invTypes AS t ON t.typeID = o.typeID
JOIN invGroups AS g ON g.groupID = t.groupID
WHERE (g.groupID = 18 OR g.groupID BETWEEN 450 AND 462
       OR g.groupID IN (465, 467, 468, 469))
ORDER BY o.stationID, o.typeID, o.bid
LIMIT 100;
```

New seed-managed NPC orders should begin at 100,000,000; old orders, partially filled orders and player orders may legitimately show other quantities. Verify before/after player-order counts, wallet balances, pseudo-escrow and no unintended duplicates.

## CI and test evidence

- `tools/verify_ore_mineral_100m.py` guards all five buy/sell generation paths and non-ore safeguards from source regressions on Linux and Windows.
- MariaDB fixture `tools/tests/npc_buy_seed_unsigned_schema.sql` includes Tritanium, Pyerite, Arkonor, Ice and an intentionally overpriced ore. The workflow tests initial 100M bids, pseudo-escrow, price-cap exclusion, repeatable `-buy-only -force`, the combined `seed -force` sell+buy result and preservation of player orders.
- No migrations or production SQL mutations were performed by submitting this code. A full Crucible client market test is still required.
