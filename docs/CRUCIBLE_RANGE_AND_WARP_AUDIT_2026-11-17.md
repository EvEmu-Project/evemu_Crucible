# Crucible-era combat, mining, tractor and ship warp audit

## Source-level corrections
- **All ship hulls:** Destiny uses fitted/type-specific dogma `warpSpeedMultiplier` (attribute 600) as **AU/s directly**, without an erroneous extra 3x scaling. Invalid live values recover to the type default, then to 3 AU/s if the static type lacks one. Individual ship data is preserved; no broad class rewrite.
- **Turrets:** Use effective fitted optimal and falloff at double precision and the live ship position. Beyond optimal is not an automatic miss; standard Crucible probability continues through falloff.
- **Missiles:** Existing range is controlled by modified missile velocity and maximum flight time, not launcher optimal; the projectile still must physically reach the target before expiry.
- **Mining lasers, strip miners, ice/gas harvesting and tractor beams:** Use live effective module attribute 54 for range. Activation is checked from hull surface to target surface (both radii). Mining now also checks range on completing a cycle, preventing remote ore yield after moving beyond the laser's range.

## Verification against historical EVE
- [Crucible patch notes](https://www.eveonline.com/news/view/patch-notes-for-crucible-1): Logistics warp speed increased **3.0 to 3.75 AU/s**.
- [Warp Drive Active (Rubicon)](https://www.eveonline.com/news/view/warp-drive-active): late-2013 warp rebalance occurred after Crucible; do not use later values as 2011 truth.
- [Revelations 2.2 patch](https://www.eveonline.com/news/view/patch-notes-for-revelations-2.2-1): Capital Tractor Beam I base **200 km**, **3000 m/s**.
- [2012 tractor reference table](https://wikiwiki.jp/eveonlinejp/Module/Tractor%20Beam): Small Tractor Beam I **20 km**, **500 m/s**, capital **200 km**, **3000 m/s**.

## Every hull and module static-data audit
Run `sql/diagnostics/2026-11-17-crucible-ship-weapon-range-warp-audit.sql` on a database backup or read-only session. Query 1 enumerates every ship hull with its stored typeID/warp AU/s; queries 2-3 flag missing/invalid values and check 3.75 AU/s Logistics; query 4 lists each weapon/mining/tractor module's base range, falloff and velocity; query 5 computes unbonused missile theoretical ranges.

**Important:** This work fixes runtime calculations but does not claim all static attribute rows have been reconciled against a pinned 2011 Crucible SDE. That requires a trustworthy historical data export and the live database results. It does NOT run the SQL, change static data, run in-game checks, or assign guessed values to ship hulls.

## In-game QA checklist
1. Unmodified cruiser and Logistics hull: show actual attribute 600 in the database and verify their observed AU/s is no longer tripled.
2. Sample ships in every class (frigate, interceptor, destroyer, cruiser, Logistics, battlecruiser, battleship, barge, Orca, industrial, freighter, capital, shuttle and capsule); compare each against the *same typeID* in the historical SDE.
3. Check laser, hybrid and projectile turret damage at optimal, optimal+falloff and optimal+2 falloff while stationary, with ammo changes, skills and tracking.
4. Verify missile skills modify each missile's velocity/time and therefore effective range, for light/heavy/cruise/rockets/torpedoes and applicable faction variants.
5. Verify strip/mining/ice/gas laser cycles at exact effective range, one metre beyond, and when the ship moves beyond range during the cycle. Confirm no out-of-range ore credit.
6. Test Small Tractor Beam I near 20 km and Capital Tractor Beam I near 200 km, with hull bonuses checked separately. Never override boosted effective ranges with static unbonused distances.
7. Check the UI's maximum target range is not confused with module firing range, and that NPC combat and belt rat spawns are unchanged.
