# Crucible-era combat ranges, mining, tractors and hull warp audit

## Source changes ready to test
- Turret damage uses the fitted optimal and falloff values in full precision and the ship's live position. The existing Crucible hit formula remains in effect beyond optimal (no artificial hard cutoff).
- Targeted mining lasers, strip miners and tractor beams check effective dogma range against distance from both hull surfaces. This corrects the old target-only-radius distance check.
- Mining modules also verify actual fitted range when completing a cycle, and do not grant ore for a completed cycle outside range.
- Existing missile range mechanics use the missile item's velocity and flight time including applied skills; no unverified static speed changes are made.

## Historical warp-speed investigation: intentionally not yet changed
- Server `DestinyManager::UpdateShipVariables()` currently calculates `3.0 * warpSpeedMultiplier` for the relevant hull. The fallback value for a missing ship warp attribute is 1.0.
- **Important:** This design suggests that attribute 600 in the 2011 data could be a *relative multiplier*, unlike some later displays of AU/s values. Removing the factor without checking archived data could incorrectly slow the entire ship fleet. A prior proposal to remove the factor was therefore reverted before merging.
- [Official Crucible patch notes](https://www.eveonline.com/news/view/patch-notes-for-crucible-1): all Logistics hulls increased **3.0 to 3.75 AU/s**.
- [January 2012 contemporary player reference](https://forums-archive.eveonline.com/message/610710): Battleships, battlecruisers, T1 cruisers and destroyers 3 AU/s; T2 cruisers 3.75 AU/s; industrials 4.5 AU/s; frigates typically 6 AU/s; interceptor/Covert Ops variants 9-13 AU/s; capitals 0.75-1.5 AU/s. *These are broad contemporary ranges, not a per-type authoritative SDE list.*
- [CCP Rubicon warp rebalance](https://www.eveonline.com/news/view/warp-drive-active) postdates Crucible by ~2 years; do **not** import modern ship speeds.

## Database diagnostics
`sql/diagnostics/2026-11-17-crucible-ship-weapon-range-warp-audit.sql` is **read only** and lists every ship typeID's raw attribute 600, plus the candidate 3x conversion, highlighting missing/odd values; it also lists relevant weapon/laser/tractor base ranges and missile theoretical range. A Logistics row of `1.25` maps to `3.75 AU/s` under the historical multiplier interpretation; a row of `3.75` implies direct AU/s. Export real database rows and compare to a pinned Crucible 2011/2012 SDE for **every hull and module typeID** before writing any static-attribute updates.

## In-game QA (not run by this PR)
- Turrets: lasers, hybrids, projectiles at optimal and 1x/2x falloff with ammo and ship bonuses.
- Launchers: rockets, heavy/light/cruise missiles, torpedoes, defender and capital missiles, including velocity and flight-time skills.
- Mining: 1 m inside/outside actual fitted effective range; ship moves beyond range mid-cycle, no out-of-range ore.
- Tractors: standard around 20 km, capital around 200 km before bonuses; skill/hull bonuses separately.
- Warp: per-hull speed of frigate, destroyer, cruiser, Logistics, battlecruiser, battleship, industrial, barge, Orca, freighter, capsule, capitals, interceptors and covert-ops; check against independent historical SDE `typeID` values.

The PR does not change live databases or claim that every historical hull/module row has been verified. The full per-hull value correction remains blocked by a missing pinned historical data inventory.
