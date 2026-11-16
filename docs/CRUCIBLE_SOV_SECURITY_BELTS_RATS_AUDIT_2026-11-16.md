# Crucible 1.6.5 security, sovereignty, asteroid belts and NPC rats — source audit

**Date:** 2026-11-16  
**Branch:** `fix/security-sovereignty-belt-rat-spawns-2026-11-16`  
**Scope:** Source fixes for high/low/null security behavior, dynamic belt ores,
NPC groups/timers and alliance sovereignty. No destructive database changes.

## Results of the repository audit

### Asteroids and belts

- `SystemManager::LoadSystemStatics` registers belts via
  `SystemBubble::SetBelt` **before** `BeltMgr::Init`. The old init cleared all
  belt registration/state. Preserve it.
- The periodic ore respawn timer used the **belt itemID** when `SpawnBelt`
  needs the **bubble ID**. Resolve the belt entity and its bubble.
- `BeltMgr::GetAsteroidType` accumulated SQL weights and compared directly
  to `MakeRandomFloat()` in [0,1], assuming they sum to 1. This incorrectly
  returns zero for rounded or percentage-scale distributions. Normalize to
  the positive finite sum. New `AsteroidSpawnTest` CTest checks 0.25/0.75,
  25/75, incomplete totals, invalid entries and endpoint roll.
- A belt whose asteroids all failed creation (or all saved records failed
  loading) used to be marked spawned, leaving a permanently empty belt.
  Only mark it spawned after successfully creating/loading an asteroid.
- When the last asteroid is depleted, the manager now permits a new ore
  population at the configured respawn tick.
- Stored radius is already in metres; remove erroneous multiplication by
  type radius on reload. On save, persist the actual current `AttrQuantity`
  instead of recomputing quantity from radius, which could restore ore already
  mined and mishandle ice.
- Log missing ore/ice distributions and skip invalid weights/quantities rather
  than creating typeID zero or invalid asteroid records.

### NPC rats and true system security

- `GetSecValue()` is **1.1 - trueSec**, but old `PrepSpawn` used
  `1.0 - GetSecValue()`, shifting every spawn security rating by **-0.1**.
  Now use the actual `GetSystemSecurityRating()`.
- Both random NPC type selection and spawn-subtype selection could access the
  element at `vector.size()` (one past the end). Constrain to
  `size()-1`, clamp requested hauler variants and discard invalid groups.
- The respawn timestamp comparison was inverted, skipping **overdue** rats.
  Correct the comparison; protect missing bubble pointers and invalid erased
  vector iterators when a spawn group is killed.
- Gate NPCs were blocked in systems with zero asteroid belts by an early
  guard. Gate spawns now follow `npc.StaticSpawns` independently.
- A missing NPC faction mapping now safely falls back to the existing rogue
  drone faction instead of selecting factionID zero.
- Avoid underflowing belt/gate NPC counters when removing unregistered
  bubbles.

**Important:** The source does **not** have a special mining-laser action
that triggers pirates on every mining cycle. Pirate encounters are scheduled
after a pilot enters a belt grid (via `SystemBubble::Add` /
`SetSpawnTimer` / `SpawnMgr::DoSpawnForBubble`). This is the game's intended
periodic belt-presence behavior, not a claim that a mine-button event is
implemented. With the supplied `utils/config/eve-server.xml`,
`BeltEnabled=true`, `RoamingSpawns=true`, and `RoamingTimer=300` seconds:
an initial group should normally appear roughly **30 to 300 seconds** after
eligible grid entry, subject to the system's security, NPC faction tables,
runtime config, NPC group spawn checks and player presence.
The code intentionally skips belt/gate NPCs when trueSec > 0.90.
`StaticSpawns=false` disables gate spawns, not belt rats.

### Sovereignty / high-sec / low-sec / null-sec

- Sovereignty information for a **solar system** was being read from the
  **region index**, returning an empty result for claimed null-sec systems.
  Use the solar-system index instead.
- `claimedFor` incorrectly held allianceID, not elapsed claim duration.
  Return nonnegative, bounded days since the claim.
- Avoid allocating unused sovereignty responses for unclaimed systems and
  avoid treating unknown IDs as valid claimable systems.
- Clear the sovereignty multi-index cache when repopulating.
- A sovereign blockade unit (SBU) offlining used
  `TCUSE::SetOnline()` to restore invulnerability; that creates an entirely
  new claim and resets its original date. Use `SetInvulnerable()`.
- Prevent SBU divide-by-zero in systems with zero stargates.
- Reject SBU onlining without an existing claim or stargates.
- Reject TCU onlining in NPC-faction systems, without an alliance, or when
  another TCU owns the claim. Stop one TCU from deleting another's claim.
- IHub may online only for its alliance's sovereign claim, and offlining it
  clears only its own linked hub entry.
- Guard TCU/SBU/IHub initialization when the POS base or grid is missing.
- An unclaimed empire/low-sec system now reports strategic index age 0
  instead of counting days since Windows epoch.

**Limitations:** Full CCP Crucible sovereignty warfare timers, actual TCU
anchoring rules, detailed blockade vulnerability mechanics, Faction Warfare
occupancy, special ice distributions, moon resources and historical NPC
officer rarity are not exhaustively implemented or tested by this source
pass. No claim of complete 2012 EVE behavior.

## Database distribution verification

The server reads `roidDistribution(systemSec, roidID, percent)`; the
`systemSec` **string** must match `mapSolarSystems.securityClass`. It uses
`invTypes`/asteroid category 25 to load types. These base data rows live
in compressed SQL dumps (`sql/base/roidDistribution.sql.gz`) and could not
be inspected as plain text through the connected GitHub source interface.
Accordingly, **the correct historic ore/ice list for each individual
high-, low- and null-sec system is not yet independently certified**.

Run the companion **read-only diagnostic file**:

`sql/diagnostics/2026-11-16-sov-security-belt-rat-integrity.sql`

Review results for any security classes without ore rows, malformed
weights/types, missing regional rat factions, and sovereign claims in
NPC-faction systems. Compare the detailed ore list to a **Crucible-era 2012**
reference; modern post-2020 EVE ore distribution rules are not applicable.
Do not overwrite the server's SQL seed data with guessed resources.

## Suggested in-game smoke tests after CI (separate from automated tests)

| System / security | Expected result |
| --- | --- |
| High-sec 0.5–0.8 | Belt ores from configured securityClass; pirates may appear after normal player-presence timer; no alliance TCU claim |
| High-sec 0.9–1.0 | Belt ores still spawn; security guard intentionally prevents pirate belt rats above 0.90 trueSec |
| Low-sec 0.1–0.4 | Belt ores from class; normal NPC faction by region; no alliance TCU claim in faction-controlled systems |
| Null-sec below 0.0 | Belt ores from class; security-scaled NPC faction groups; TCU/IHub/SBU behavior only in eligible claimable systems |
| Sovereign null-sec | Query sovereignty with client; verify claim age, TCU claim, IHub linkage; on/offline SBU without resetting claimTime |
| Ice belts | Validate actual regional ice types and their saved remaining quantities after restart |
| Exhausted belt | Mine final rock; return after configured BeltRespawn interval and verify new population without duplicates |
| NPC killed in belt | Observe respawn only once delay elapses, no crash, no incorrect additional spawn group |

Test on a backup / disposable character and do not mark any test complete
without observing it. CI builds and the new asteroid probability CTest only
verify source compilation and deterministic helper behavior, **not** the
historic contents of the connected server's database or in-game mining.

## Rollback / deployment

No SQL migrations were introduced in this branch. Retain your existing
0.8.7-Rizonx server binary and database backup. Install the new matching
Windows ZIP together with its UCRT64 runtime DLLs, record `BUILD-INFO.txt`
commit SHA, and capture relevant `COSMIC_MGR`, `SPAWN` and `SOV` logs.
