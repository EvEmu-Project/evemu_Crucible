# EVEmu master: drone, motion and codebase audit — 2026-11-15

This is a **targeted source audit**, not a claim that every translation unit,
migration, gameplay system or path has been comprehensively validated. The
repository contains more than 1,000 files. The fixes below cover reproducible
or directly visible source defects. Runtime behavior still needs testing with
a compatible Crucible-era client and a backed-up MariaDB database.

## Changes in this pass

- Avoid two full `SystemManager::GetEntities()` map copies per drone scan by
  using a read-only entity registry view in DroneAI; leave the snapshot API
  available where mutation during iteration is possible.
- Mining drones (group 101) no longer auto-attack NPCs, join combat focus fire,
  retaliate as combat drones, or fire via the generic weapon formula. **Mining
  ore collection is NOT implemented** in `DroneAI`; mining has not been fixed
  or verified. The explicit combat target path now logs this unsupported state.
- Retaliation no longer overrides `Return()`'s disabled auto-engage setting.
- Make `SystemManager::GetEntityByID()` return null for missing IDs instead of
  dereferencing `end()`.
- Correct the collision-gap calculation to subtract *both* hull radii;
  preserve the contact flag across all nearby ships and ignore null/stale
  player ship pointers. This bump handler reports contacts; it does not yet
  implement an impulse or full general-object collision solver.
- Clamp `Follow()` edge distances at zero instead of casting a negative float
  to `uint32` (huge unsigned wrap). Aim at the stand-off point on the near side
  of the target, not beyond the target center.
- In `Orbit()` avoid resetting `m_position` to a completely unrelated absolute
  timestamp orbit point before applying `MoveObject()`. Steer toward an orbit
  waypoint derived from the current ship angle instead; include the target
  radius when calculating a safe stand-off and guard nonfinite/zero orbital
  geometry and speed. **Warp accel/cruise/decel, warp packets and warp tunnel
  effects are unchanged.**
- Add `MovementFormulaTest` covering overlapping hulls, touching, normal
  surface gap, invalid inputs and requested stand-off radius.

## Existing path review (source, not live validation)

| Path | Status / review |
| --- | --- |
| Combat drone targeting and damage | Existing target lock, control-range, damage and timers reviewed. Mining-group attack and retaliation guards added. Actual damage to NPC hull/shields not reproduced in-game in this audit. |
| Drone orbit | `IdleOrbit()` and `SetEngaged()` reach `DestinyManager::Orbit()`; geometry no longer teleports the ship before the next movement step. In-game client/server synchronization not verified. |
| Return-to-ship and return-to-bay | `DroneAIMgr::Return()` follows the live controller; `DroneSE::Process()` executes `ScoopToBay()` when within 2,500m. Inventory capacity and exception safeguards already exist. Tested by code inspection only. |
| Mining drone mining/haul | **Not implemented**: `DroneAI::State::Mining` is an empty case, with no asteroid-extraction, drone cargo hold, ore transfer or flight cycle. Do not claim mining is functional. |
| Ship/object collisions | Existing `CheckBump()` handles nearby *player ships*, not arbitrary asteroids/stations. Client/server bounce reports need a replay showing target, speed, angle and Destiny debug logs; no real-object collision resolution has been added. |
| Warp tunnel | No warp-code change; should be regression-tested for align, warp entry/exit, destination positions, interrupted warp and tunnel FX. |

## Required runtime test matrix

1. Launch 1 and 5 combat drones; engage a stationary NPC then an orbiting
   NPC; verify health decreases, locking stabilizes and no server crash.
2. Toggle aggressive/passive and focus fire; return during combat and while
   NPCs target the drones; verify no unintended re-engagement.
3. Return to orbit, then return to drone bay with room available, full bay
   and ship moving away; verify drone inventory and space entity counts.
4. Launch a mining drone, command it to mine and inspect server log: **it is
   currently unsupported**, so no ore production should be expected.
5. Approach/stop/orbit large stations, asteroids and another ship at low and
   high sub-warp speeds; capture any continued bouncing or client corrections.
6. Warp to a bookmark, planet and station; validate tunnel, alignment,
   arrival/deceleration and image/ball state updates.

## Additional audit targets

- Mining-drone pipeline: implement actual asteroid targeting, ore extraction,
  temporary drone hold, cycle timer and ore transfer to ship cargo with
  capacity and ownership tests.
- `DestinyManager::CheckBump()` is limited to player ships and sends
  notifications, not a physical collision impulse; any ship/object bounce
  remaining needs measured trajectories before introducing a collision solver.
- More comprehensive source and runtime auditing of missile defenses,
  entity lifecycle, mission scripts, inventory mutations and SQL migrations
  remains outstanding. Avoid making speculative changes without reproductions.

See [PATCH_NOTES.md](../PATCH_NOTES.md) for previous changes and
[STABILITY_WORKLOG.md](../STABILITY_WORKLOG.md) for CI/history.
