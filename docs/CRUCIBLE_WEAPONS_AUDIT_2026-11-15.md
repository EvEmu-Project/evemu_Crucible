# Crucible weapon and ship-combat source audit — 2026-11-15

This report describes **source-level corrections**, not a claim that every
weapon, ship hull, fitted module or Crucible gameplay mechanic has been
validated. Build/test success exercises formula helpers and compilation, not
the full client/server combat simulation.

## Crucible-era references

- Legacy (pre-tracking-stat-consolidation) turret formula:
  [Rifter Guide](https://eve-files.com/media/corp/wensley/the_rifter_guide_solo_pvp.pdf)
- Historical 2011 missile damage formula:
  [EVE Forums, 24 October 2011](https://forums-archive.eveonline.com/message/238271/)
- Historical 2011 Guided Missile Precision exclusions:
  [EVE Forums, 27 October 2011](https://forums-archive.eveonline.com/message/270292/)
- Historical Warhead Upgrades and missile specialization bonuses:
  [EVE Compendium](https://eve-files.com/media/0812/The_Eve_Compendium_Verison_1.5.pdf)
- Modern reference for explaining persistent underlying mechanics (not as a
  replacement for Crucible static data):
  [EVE University turret mechanics](https://wiki.eveuniversity.org/Turret_mechanics)

## Reviewed weapon families and changes

| Family / path | Changes / source review | Still needs in-game proof |
|---|---|---|
| Hybrid, projectile, energy turrets (`TurretModule`, `TurretFormulas`) | Shared pre-2016 tracking/signature/falloff chance, shared random roll for hit quality; eliminate artificial signature-hit shortcut; default damage multiplier safety | Charge effects, heat, group fire, ammo consumption, ship bonuses |
| NPC gunfire (`NPCAI`, `TurretFormulas`) | Uses the same hit and damage-quality calculation as players | NPC aggro, weapon cadence and all NPC ship variants |
| Combat and sentry drones (`DroneAI`, `SentryAI`) | Shared chance-to-hit; remove guaranteed 0.1 minimum drone damage on misses; identical chance/wrecking behavior | Sustained combat and tracking with real drone attributes |
| Missile families: light, heavy, cruise, rockets, heavy assault, torpedoes, XL | Maintain speed and flight-time skills; Warhead Upgrades 2% not 5%; Guided Missile Precision only guided light/heavy/cruise (FoF/T2 included); specialization no longer adds the incorrect 3% missile hit damage | Proper launcher-side specialization rate-of-fire calculation, charge/fitting/ship effects |
| Linked weapon launchers | Missile grouping increases volley damage, not missile flight time | Linked volleys with client effects and damage logs |
| Missile application (`Missile::HitTarget`) | Target signature, target speed, explosion radius and velocity still feed the shared formula; skip zero/invalid impact damage | Individual old SDE AoE DRF/DRS attributes, target flight path |
| Defenders (`ActiveModule`, `Missile`) | Correct milliseconds-to-seconds travel range; speed bonus from Defender Missiles; deduct defender damage from incoming missile hull HP rather than unconditional destruction | Multiple intercepts, launcher/target selection with actual client |
| Smartbombs | AoE radius/ship resistance path preserved; incoming missiles now receive interception damage once the smartbomb damage exceeds their remaining hull HP | Multi-ship smartbombs, allied effects, missile HP, boundary effects |
| Bombs | Existing bubble AoE and per-target resist/application paths reviewed without unverified balance substitutions | **Existing bomb triggers on a target impact**; Crucible timed area detonation/flight and bomb-specific immunities are not yet faithfully implemented |
| Superweapons | Source is a non-functional work-in-progress: factory constructs generic `ActiveModule` for `Super_Weapon`; its `DoCycle` is an empty stub. The separate `SuperWeapon` class is not registered by the factory | **Not working**: specific Crucible doomsday targeting, activation restrictions, timing, fuel and damage |
| Ship damage and defenses (`Damage`, `Ship`) | Missing resonance defaults to neutral (1.0) instead of silently acting like 99% resistance; explicit 0.0 resonance remains 100% immunity; ship module cycle guards 0/NaN capacitor division | Hull uniformity bleed-through tuning, individual resist bonus stacking, subsystem/rig modifiers |
| Ship turret group and effects | Missing turret damage multiplier uses safe 1.0 fallback; initial effect and damage flows reviewed | Grouped guns should be rolled independently; current volley aggregate shares one quality roll |
| Configuration | Default wrecking probability normalized to 1% for players, NPCs, drones and sentries | Explicit user-installed XML overrides are not changed, intentionally |

## Safety, consistency and performance

- Weapon chance, quality, missile flight range, defender HP and damage
  resonance logic now have deterministic `WeaponFormulaTest` cases.
- Target pointers, deleted missile targets and missing damage sources remain
  sensitive lifecycle boundaries. No changes were made to warp calculations
  or movement packet semantics in this pass.
- We retain a **snapshot** of area-of-effect target IDs because
  `ApplyDamage()` can destroy objects and mutate the bubble mid-cycle.
  Removing that snapshot for a micro-optimization would cause crashes.
- Character skills and ship-module bonuses require matching **Crucible-era
  static data**. Do not import current Tranquility weapon stats or apply modern
  tracking normalization to this code.
- Legacy custom configuration can still change damage rates and critical
  chances; a pre-existing `eve-server.xml` is intentionally not overwritten.

## Recommended gameplay regression matrix

1. Fire small/medium/large hybrid, projectile and laser turrets at stationary
   frigates/cruisers/battleships: check optimal, optimal+falloff, transversal
   and radial movement, misses, hits, wrecks, and shield/armor/hull damage.
2. Verify NPC, sentry and five-drone volley tracking; time 30+ seconds of
   sustained firing and cross out of engagement range.
3. Launch light, heavy, cruise, rocket, heavy-assault and torpedo missiles:
   compare same target signatures and speeds, 0–V support skills,
   grouped volleys and short vs maximum flight time.
4. Reproduce Defender Missile interception against small and high-HP
   incoming missiles, verify surviving missile HP across multiple shots.
5. Trigger smartbombs near hostiles, player ships, drones and incoming
   missiles; inspect missile interception and protective resistances.
6. Test a zero-capacitor special hull, missing resonance attributes and
   100%-resistant test entities for correct non-NaN state updates.
7. DO NOT treat bombs or doomsdays as fully supported: their special-purpose
   behavior remains incomplete.

See previous [drone range regression](DRONE_RANGE_AND_SUSTAINED_COMBAT_2026-11-15.md)
and [drone/motion audit](DRONE_AND_MOTION_AUDIT.md).


## Additional NPC, sentry and projectile checks

- Stationary sentries do not always have a DestinyManager. Sentry tracking
  now works with their authoritative SystemEntity position instead of
  returning a miss for every shot when DestinyManager is absent.
- Sentry damage multipliers now retain fractional values (no truncating
  integer cast); missing speed defaults to a safe attack cadence.
  Null-bubble and deleted-target callbacks are also guarded.
- NPC missile launch now checks target registration, missile velocity and
  DestinyManager before dividing by speed or scheduling a hit. NPC missiles
  are recognized by their charge/effect when applying the configured
  missile damage rate, even though their weaponRef is an NPC entity.
- The smartbomb's interception check now uses live remaining missile HP
  after defender hits rather than static inventory HP.
