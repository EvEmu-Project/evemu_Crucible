# Crucible 2011 combat / mining / tractor / warp audit — updated with user SQL exports

## Parsed user data
The four attached exports yielded **376 ship hulls, 61 published hulls without attribute 600, 1,312 module rows and 234 missile-charge rows**. The files are query-result text serialized as `INSERT INTO UnknownTable`; they are NOT real database migrations.

### Warp-speed interpretation — now verified from the user's SDE
- Battleships have raw attribute 600 = **3**; T1 frigates = **6**, and interceptors = **13.5**, matching pre-Rubicon units in AU/s. 2013 CCP documentation explicitly states interceptors previously had **13.5 AU/s**.
- Therefore the old server logic `3.0f * warpSpeedMultiplier` applies an incorrect additional threefold scaling. Runtime now reads fitted attribute 600 as AU/s and falls back to historical type value, then 3.0 only if both are missing.
- 4 Logistics hulls incorrectly store **9**; CCP's Crucible official patch changed them to **3.75 AU/s**.
- 47 published T1 cruiser/battlecruiser/destroyer rows are missing attribute 600 and should use **3 AU/s**, consistent with early 2012 player observations.
- Other confirmed problems in the user export include 11 ordinary industrials set to 0.75 (2012 standard **4.5**), 4 Marauders at 13.5 (historical **3.0**), 4 HICs at 9 (historical **3.75**), 8 Recon cruisers at 1.25 (historical **3.75**), and all 8 transport ships at 0.75 (historical deep space **3.0** vs blockade runners **9.0**). Four combat interceptors should be **9.0** rather than 13.5, while the four fleet interceptors remain **13.5**. Standard Assault Frigates were historically **6.0**, not **13.5**; specialty tournament hulls remain excluded.
- These corrections are typeID-scoped and old-value guarded in `sql/diagnostics/2026-11-17-crucible-warp-speed-verified-repair.sql` (98 hull records). **Manual backup, preview and staging run required; PR does not run SQL against your server.**

### Still requiring individual corroboration
Not all 61 absent values should be filled with 3 automatically: the remaining **14 absent published entries** are 8 Command Ships, 4 Strategic Cruisers, and Noctis/Primae. Other outliers, rare tournament hulls, stealth bombers, electronic attack frigates, freighters and capital variants should be checked against pinned *Crucible 1.6 / 2012* SDE by typeID before changing them. Do not use post-Rubicon values.

## Weapon / mining / tractor observations from 1,312 exported module rows
- Fitted energy/hybrid/projectile turret types already have distinct **optimal** (`54`) and **falloff** (`158`) values, so avoid a universal range multiplier; runtime now reads live effective values in double precision.
- All 6 tractor-beam rows contain sensible separate range / velocity values: Small I **20 km/500 m/s**, Small II **24 km/600 m/s**, Medium I **40 km/500 m/s**, Large I **80 km/500 m/s**, Capital I **200 km/3,000 m/s**, Capital II **240 km/3,600 m/s**. Preserve them.
- Mining/strip laser `54` values are present and vary by type. The range check previously omitted the mining ship radius and used the inventory position instead of the live ship entity position; both are corrected in PR.
- Many module rows show `NULL` for `54` because they instead use specialized attributes: energy neutralizers `98`, energy transfers `91`, smartbombs `99`, shield transporters `87`, and others. **NULL in the earlier output alone is not a broken module**. The extended SQL diagnostics include these fields.
- 234 missile types were enumerated. Their effective range is charge velocity × flight time, including live skills, not launcher range. The uploaded values appear internally coherent; no global missile multiplier has been applied.

## Historical references
- CCP Crucible patch: https://www.eveonline.com/news/view/patch-notes-for-crucible-1
- 2012 player warp category breakdown: https://forums-archive.eveonline.com/message/610710
- CCP Rubicon 2013 confirmation of pre-Rubicon 13.5 interceptor AU/s: https://www.eveonline.com/news/view/warp-drive-active
- Older Recon/Transport/Marauder/HIC and interceptor-specific ship tables:
  - https://lib-corp.wikidot.com/reconship
  - https://lib-corp.wikidot.com/transportship
  - https://lib-corp.wikidot.com/marauder
  - https://lib-corp.wikidot.com/heavyinterdictor
  - https://lib-corp.wikidot.com/interceptor
- For a reproducible exact typeID comparison, CCP's Crucible 1.6 archived SDE was identified as `Crucible_1.6_65449_db.zip` in 2012 discussions, but its contents are not yet available locally. https://forums-archive.eveonline.com/message/1064377/

## In-game QA before merging
1. Verify a loaded battleship warps at **3 AU/s** (not 9) and a standard frigate at **6 AU/s** (not 18).
2. Run the SQL repair on a backup staging DB, restart/reload inventory type data, and verify all four Logistics hulls warp at **3.75 AU/s**.
3. Verify representative cruiser, destroyer, BC, HIC, Recon, Marauder, ordinary industrial, blockade runner and deep-space transport at their individual values.
4. Compare special Command/Strategic Cruiser, rare interceptors, capital and freighter speeds to 2011-2012 SDE before approving their outstanding rows.
5. Test lasers/projectiles/hybrids with optimal/falloff and ammo, missile velocity/duration skills, mining/strip/ice/gas lasers at range boundaries and after moving out of range mid-cycle, and all tractor sizes with hull bonuses.
6. Check existing ship `entity_attributes` (persistent live overrides) for old attribute 600 entries. A changed type default does not override persisted ship-specific attributes.

## 17 November 2026: Ferox, wreck approach, and last 14 missing hulls

The user's database verification returned eight PASS values including the Ferox group, Logistics hulls, and four representative cruisers/BCs. Published hulls missing dogma 600 dropped from 61 to 14.

**Ferox log:** `ProcessState()` formerly demanded heading alignment and `m_timeFraction > 0.749`, then forcibly warped after the estimated standstill alignment time +0.3 ticks while logging `warp align/speed is incorrect`. `m_timeFraction` is acceleration-timer progress, not actual speed. Changed the entry criterion to `m_activeSpeedFraction >= 0.75` and aligned heading; removed the premature forced fallback, replacing it with rate-limited WARP_TRACE for prolonged misalignment. **Test in-game** at standstill, moving wrong-way, during a prop-mod effect and on repeated warp orders.

**Approach/loot distance:** `Follow()` called `Stop()` but then continued processing after `Stop()` cleared follow state. It now returns and sends a one-off authoritative position at the stopping point. `InvBrokerBound::GetInventoryFromId()` checked static inventory center-to-center distance against 2,500m, while follow navigation uses surface separation. It now uses live ship/target SystemEntity positions and subtracts BOTH ball radii, for cargo containers and wrecks. Missing in-space entities fail closed. **Test in-game** using wrecks, cans, and oversized ships near 2,500m surface distance; compare client displayed distance and opening errors.

**The remaining 14 hulls:** The new manually executed SQL `sql/diagnostics/2026-11-17-crucible-warp-speed-remaining-14-repair.sql` proposes 3 AU/s for eight Command Ships and four Strategic Cruisers based on pre-2019 category history; it also proposes 3 AU/s for Primae and Noctis, both marked lower-confidence (especially Noctis) pending a pinned Crucible SDE. It inserts only absent attributes 600 for the exact 14 verified type IDs and supports omitting Primae/Noctis for a strictly corroborated 12-record deployment. This is a proposal only, NOT executed on the user's DB. No legacy ship records are overwritten.

Historical context: CCP [October 2019 warp change](https://www.eveonline.com/news/view/warp-drive-active-warp-speed-changes-now-live), [UniWiki Vulture patch history](https://wiki.eveuniversity.org/Vulture), [UniWiki Legion patch history](https://wiki.eveuniversity.org/Legion), and [Primae specifications](https://wiki.eveuniversity.org/Primae). Current or later warp speeds are not Crucible-era proof for the two ORE ships.
