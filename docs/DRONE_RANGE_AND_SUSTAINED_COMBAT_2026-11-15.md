# Drone control range and sustained combat regression — 2026-11-15

Player confirmed that light combat drones fire at NPCs, but that their
control range and repeated attacks are incorrect.

## Changes
- Crucible-era command range calculation: 20 km base plus 5 km per
  Scout Drone Operation level and 3 km per Electronic Warfare Drone
  Interfacing level. Retain larger ship/character dogma ranges and flat
  fitted bonuses where represented.
- Command range is measured from the home ship at the moment a new attack
  is requested. An assigned, locked target remains pursuable outside that
  range while in the same bubble (unless dead/cloaked).
- Do not clear a locked target merely because it crosses an arbitrary chase
  threshold. Do not reset the shot timer when temporarily approaching.
- Verify that the specific target remains locked before a shot; recover
  another available locked target after the assigned ID expires.
- Rejected commands no longer cancel a return-to-bay order or enable
  auto engagement.
- Update only explicitly supplied drone aggression and focus-fire settings,
  and copy the latest settings to newly launched drones.
- Avoid a whole-system entity-map copy in CmdEngage recovery; avoid verbose
  per-tick INFO target lookup messages.
- `DroneControlRangeTest` covers 20/45/60 km, fitted and precomputed ranges,
  and malformed attribute values.

## Manual regression matrix

1. Use a compatible Crucible client and verify control range at 20/35/45/60
   km with matching skill combinations. Test fitted control-range modules.
2. Send five light drones against a moving NPC: track target lock, repeated
   weapon effects, repeated health changes and target pursuit for 30–60s.
3. Move the NPC beyond the initial command range: a locked drone should chase;
   a *new* attack command beyond that range should be rejected.
4. Toggle only aggressive/passive or only focus fire: the other state must not
   change. Check drone settings after scooping and relaunching.
5. Return while engaged and issue an invalid attack: return should continue.
6. Verify mining drone harvesting remains unimplemented in this fork.

Do not interpret passing CI compilation and unit tests as validation of the
actual gameplay. Reproduce persistent problems with 30–60 seconds of
DRONE__AI_TRACE, DRONE__MESSAGE, TARGET__TRACE and DESTINY move traces.

This pass reviewed drone, target manager, ship settings, RPC and movement
interfaces, not all 1,313 repository entries. See
[earlier audit](DRONE_AND_MOTION_AUDIT.md).
