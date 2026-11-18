# NPC buy orders — standard seed at 100% reference value

## 18 November 2026 — Ore/mineral/ice volume update

The historical **550/550,000 unit** description below remains correct for non-ore items. Newly seeded NPC buy and sell orders for minerals (group 18) and ore/ice (450–462, 465, 467–469) now start at **100,000,000 units each**, with a 10M ISK/unit upper bound for high-volume NPC bid pseudo-escrow. Dynamic Trader Joe orders use the same quantity. For migration/backup and existing order refresh instructions, see [the 100M-unit guide](NPC_ORE_MINERAL_100M_ORDERS_2026-11-18.md). No player order is modified by the source patch.


**Prepared:** 17 November 2026 · **Repository:** `xrizonx/evemu_Crucible` · **Scope:** Windows/Linux patched EVEDBTool and market settlement.

## Goal and reference-value definition

The standard `evedbtool seed` now seeds NPC **buy and sell** orders in configured regions. NPC buy prices are exactly **100% of the Crucible database's `invTypes.basePrice`** (rounded to two decimal places, minimum 0.01 ISK); this is **not** the moving player market average. Types with missing, zero or negative `basePrice` receive a transparent **100 ISK** fallback. No random market-bot discount/markup or security-dependent adjustment applies.

"All items" means **every published, market-listed inventory type** in the installed Crucible SDE. The client cannot normally list unpublished, internal/test or types without a `marketGroupID` for market trading, so these are not seeded. The buy catalogue includes published implants, blueprints and rare faction/deadspace/officer/storyline types when their market group is defined, without a hard-coded category whitelist. The seed can create one buy order per eligible item per selected station. Large installs may add millions of orders; check MariaDB space/performance first.

## Implementation details

- Seed NPC buyer identity `ownerID = 4000000` (reserved Trader Joe pseudo-character ID recognized by `IsTraderJoe`), `bid=1`, `orderRange=0` (database-compatible system-range marker; server instant-sell matching remains **station-specific**), `duration=250`, `isCorp=0`. This combination marks **seed-managed** bids; dynamic random bot orders are not overwritten.
- Orders initially offer **550,000** charge-category items or **550** of other types, capped to keep each initial order's synthetic notional escrow within **10 trillion ISK**. Types whose single-unit reference price is above that cap are skipped; the audit lists gaps. No player wallet is charged to create NPC orders.
- A one-time migration `SEED_NPC_BUY_100PCT` is separate from legacy `SEED_MARKET`. Once applied, a regular `seed` command leaves seeded bids unchanged. Repeatable `seed -buy-only -force` deliberately replaces only marker-matched NPC bids in configured regions.
- Trader Joe's future randomly created buys use 100% reference prices. The one-time seed also **repairs legacy regional-range NPC bids** under the old unsupported NPC corporation owner `1000125`: it changes only these NPC buy orders to owner `4000000`, recalculates their reference price and their **nonfunded NPC pseudo-escrow**. Player-owned bids, NPC sells, actual wallets, player escrow, and historical transactions are not modified. This scoped change appears in `-dryrun`; review the legacy-bid preview query before applying.
- NPC matching uses the **stored bid price** for actual seller payout, not an arbitrary lower price entered by the selling client. Market order and transaction internal price fields have been widened from float to double so expensive ship/module bids do not lose material precision when read from `mktOrders`. Order type/station and requested stack quantity are validated before inventory mutation; full-stack and partial-stack sales are differentiated. Other market transfers remain non-transactional in the existing engine and must still be tested in the client.

## MariaDB Error 1264 (17 November 2026): upgrade and recovery

The initial version of PR #35 attempted to insert `orderRange=-1`, the EVE station sentinel, but existing `mktOrders.orderRange` rejects negative values. The corrected seed inserts **`orderRange=0`**, which the deployed column can represent; static-order matching and force-refresh use `0` consistently. No destructive `ALTER TABLE` is needed.

If you previously ran `seed -buy-only -force` and it failed: **do not assume the whole run rolled back**. A prior `DELETE` (for static NPC bids) or legacy Trader Joe NPC `UPDATE` may have happened before the failing `INSERT`. Stop the game server, take a fresh backup for diagnosis, and compare the database with the pre-run backup before applying anything else. Do not restore the backup over fresh player progress without a conscious plan. Use the updated read-only audit below to count rows, inspect `seed_migrations` and check legacy owner IDs.

After the corrected EVEDBTool build is ready and the backup is verified, review `seed -buy-only -dryrun`. For an **interrupted seed that has no successful migration marker**, running `seed -buy-only` will install the new bid seed. Use `-force` **only if you deliberately want to replace existing seed-managed NPC buy orders**; it deletes/recreates these NPC bids. The source change does not touch existing player characters, inventory, wallets or player-owned orders.

## Existing database rollout (recommended)

1. Stop EVEmu and the market bot. Back up the **entire MariaDB database** and current config. Record before counts: `SELECT bid, COUNT(*) FROM mktOrders GROUP BY bid;`. Do not run on a live multiuser server.
2. Set `seed-regions` and `seed-saturation` in `sql/evedb.yaml`. To seed every NPC station in the configured regions, choose **100**. A few regions with 100% coverage can create very many orders; begin with one test region.
3. From the new version's `share/evemu/sql` directory on Windows:

   ```powershell
   .\evedbtool.exe status
   .\evedbtool.exe seed -buy-only -dryrun
   # After backup and review, with server stopped:
   .\evedbtool.exe seed -buy-only
   ```

   Do **not** run `seed -force` merely to add NPC buys; without `-buy-only` it also rebuilds existing NPC sell orders. The normal `seed` option is fine for first-time installs. No SQL schema migration or static-data override is required.

4. Run `sql/diagnostics/2026-11-17-npc-buy-orders-100pct-audit.sql` **read-only** in HeidiSQL before and after. Check buy-order price/reference mismatches **zero**, duplicate marker-managed bids **zero**, and the **legacy old-owner bid count** in seeded regions is zero. Inspect item type coverage and confirm existing player orders unchanged.
5. Restart EVEmu to refresh market caches. In the Crucible 1.6.5 client select a test station and sell a **small, cheap, published item** (e.g. 1 Tritanium) against an NPC bid. Confirm the NPC buy price shown matches `invTypes.basePrice`, the wallet receives that price less the configured sales tax, the item stack decreases only by the requested quantity, and the NPC bid's remaining volume decreases once. Test a second sale and restart persistence.

## Acceptance checklist

- [ ] Linux GCC + CTest and patched Go EVEDBTool build/dryrun pass.
- [ ] Windows UCRT64 GCC + CTest and patched Go EVEDBTool build/dryrun pass.
- [ ] Docker image build passes.
- [ ] Existing `SEED_MARKET` database accepts `seed -buy-only` without reseeding sells.
- [ ] Repeat `seed -buy-only` creates **no additional orders**.
- [ ] `seed -buy-only -force` replaces only seed-managed `ownerID=4000000` station-local bids; existing player orders and Trader Joe dynamic bids unchanged.
- [ ] Database audit shows zero mispriced or duplicate seed-managed NPC buy orders, and zero legacy old-owner Trader Joe bids in seeded regions.
- [ ] UI displays bids for published market-listed item types (mineral, charges, module, ship, container, implant, skill, blueprint, faction/deadspace).
- [ ] One-item and partial-stack sales pay exactly **stored NPC bid price** (less configured tax) and consume no more than the requested quantity.
- [ ] Wallet/journal/market transaction records and filled/remaining quantities persist after server restart.
- [ ] If anything fails, stop further tests; restore the backed-up database rather than editing player wallets/orders in place.

**Do not treat source checks or CI as a completed live-client pass.** Record the Windows `BUILD-INFO.txt` SHA, seed preview, SQL audit results and one before/after seller inventory/wallet example.

## Known limitations

- NPC bids are inserted with range `0` because the deployed `mktOrders.orderRange` schema rejects negative `-1`. The Crucible UI may label range `0` as **system**, but the existing `MarketDB::FindBuyOrder` logic still requires the **same station ID** for an immediate sale. Test from the same station; neither cross-station nor cross-region remote delivery is implemented.
- `basePrice` may not reflect a desirable supply-and-demand economy price, particularly for rare items; there is no reliable universal 100% player-market average for items with zero trades.
- Existing player and corporation order settlement has other transaction/rollback gaps (see the repair checklist). This change does not make the entire market escrow system atomic.
- The order seeder does not rerun automatically on every login or respawn exhausted orders. Repeat `seed -buy-only -force` only when intentionally refreshing backed-up NPC stock.
