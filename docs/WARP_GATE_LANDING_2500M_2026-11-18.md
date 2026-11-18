# Gate warp landing and surface-range navigation — 18 November 2026

## Investigation

The client and server use different concepts of distance:

- A warp command (`CmdWarpTo`) carries a **target center** and a center-based stopping distance.
- Stargate jump and docking permissions use **surface separation**: center distance minus the ship's and target's ball radii. Navigation `Follow()` already uses that surface separation internally.
- Prior manual gate warps moved the destination toward the ship by the target radius, then additionally added a `radius / 3` gate fudge and **two ship radii**, so the final surface gap varied with gate size and hull size.
- Prior autopilot warps sent only the configured surface stand-off (normally 10 km) as a *center-based* warp stop, which could drop large ships inside a gate/station.
- Both `Client::StargateJump()` and pending-jump processing called `Follow(gate, gate->GetRadius() + 2500)` despite `Follow()` already subtracting both radii. Station docking had the same duplicate radius.

## Source changes

- `EVEmuMovement::WarpCenterStopRange()` converts a requested **surface gap** into the center range expected by the Destiny warp protocol, adding target and ship radii exactly once, rounding up to a meter. An invalid value fails conservatively, and the 32-bit packet range is bounded.
- For manual stargate and ordinary small-object warps, preserve the target center and request a stop **2,400m from both collision surfaces** for "warp to zero"; longer requested stand-offs (10/30/50/100 km) are retained. Bookmarks with raw coordinates do not have a target ball radius and no longer gain an artificial ship diameter.
- Autopilot still uses its configured stand-off distance (normally 10 km), now including the gate/station and ship radii in the center stop range. Initial autopilot `Follow()` receives only a *surface* range.
- Queued and immediate gate approaches now call `Follow(gate, 2400)`; jump is still authorized at `<=2500m` surface separation. Station docking uses the same safe follow range and retains its 2,500m authorization threshold.
- Preserve custom planet, moon, station-dock-elevation and wormhole warp-in handling. Existing stationary warp-stop synchronization (`SetPosition`, `Halt`, zero velocity) is unchanged.
- Add an informational `DESTINY__WARP_TRACE` entry showing target and ship radii, chosen center range and predicted surface clearance.

## Verification

- `src/eve-test/utils/MovementFormulaTest.cpp` checks 35m and 2,500m ship radii against a 10km gate radius: surface gap remains 2,400m for both. It also checks autopilot 10km, requested 100km, fractional radii and invalid numeric values.
- Existing Windows and Linux `tools/verify_crucible_range_and_warp.py` CI guards are extended to prevent reintroducing `gate radius/3`, ship-diameter duplication or `gate radius + 2500` follow ranges.
- Pure landing math compiled and passed independently with GCC 14.2 `-std=c++17 -Wall -Wextra -Werror`. **The full Windows/Linux builds and live Crucible client warp behavior must be verified by CI and the operator.**

## Required live test matrix

1. Use a frigate, battleship and a large hull (capital if supported). In the same system, warp **at 0m** to a known stargate. Verify the client completes warp, stops rather than rubber-banding, and reports roughly 2.4 km from the gate's collision surface. Immediately click Jump; it should not require an extended reapproach.
2. Repeat gate warp with **10km, 30km and 100km** range selections. Verify those requested surface gaps are preserved rather than gaining a radius/3 or double-ship-radius margin.
3. Use **autopilot warp to gate**. It should land about the configured `apWarptoDistance` from the collision surface (normally 10km), then approach smoothly to 2.4km and jump.
4. Test `Warp to 0` on a wreck/container/other small object and check for surface overlap or an unexpected ship-size-dependent margin. Test a coordinate bookmark separately: it should honor its exact position and requested range.
5. Warp to a station, queue Dock while warping, and confirm normal post-warp approach reaches the <=2.5km docking perimeter. Verify the existing dock-elevation approach and planet/moon/wormhole warp-in paths still behave correctly.
6. Observe server `DESTINY__WARP_TRACE` around the arrival and collect before/after ship ID, target ID, ship radius, gate radius, actual positions, client-overview distance and any destination/velocity corrections.

If bouncing persists, compare the gate's actual rendered collision mesh and ball radius in the installed Crucible SDE, plus the encoded client ball/radius packets. A non-spherical model or mismatched client radius can require an additional **type-specific** clearance; do not blindly increase every target's distance or disable collision globally.

No SQL migration, player inventory edit or deployed-server update is performed by this code patch.
