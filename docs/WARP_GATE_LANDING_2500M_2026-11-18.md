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
- Manual stargate, station and ordinary small-object warps use the target center and request a stop **2,400m from both collision surfaces** for “warp to zero.” Longer selected ranges are retained. Coordinate bookmarks use their exact point and requested range.
- Autopilot uses its configured stand-off distance (normally 10 km), including the gate or station radius and ship radius in the center stop range. A subsequent approach uses a surface range.
- Queued and immediate gate approaches now call `Follow(gate, 2400)`; jump is still authorized at `<=2500m` surface separation. Station docking uses the same safe follow range and retains its 2,500m authorization threshold.
- On gate and station approaches, `MoveObject()` limits the final one-second step at the first intersection with the stop sphere along the ship's current heading. It halts with a single position, stop and zero-velocity update.
- Station warp and approach geometry uses the same entity center as the dock range check. The special planet, moon and wormhole warp-in paths still select their own destination points.
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
5. Warp to a station at 0m and queue Dock during alignment and again during active warp. Confirm each warp lands within 2.5km and docking starts without another approach. Repeat from a longer stand-off to check the approach path. Exercise planet, moon and wormhole warp-ins separately.
6. Observe server `DESTINY__WARP_TRACE` around the arrival and collect before/after ship ID, target ID, ship radius, gate radius, actual positions, client-overview distance and any destination/velocity corrections.

If bouncing persists, compare the gate's actual rendered collision mesh and ball radius in the installed Crucible SDE, plus the encoded client ball/radius packets. A non-spherical model or mismatched client radius can require an additional **type-specific** clearance; do not blindly increase every target's distance or disable collision globally.

No SQL migration, player inventory edit or deployed-server update is performed by this code patch.

## Movement drift, bump and micro-stutter review — 18 November 2026

The continuing review of PR #43 against the newly merged `master` (including mining-drone PR #42 and DED PR #41) found **three further reproducible source-level risks**:

- In `DestinyManager::ProcessState()`, the STOP state called `Stop()` once per simulation tick even when the ship was already stationary, rebroadcasting `CmdStop` and unnecessarily resetting movement state. Now a stationary STOP does not transmit repeated packets; it only simulates an active speed transition or performs a **single** zero-velocity correction for a stale velocity.
- `MoveObject()` previously clamped the final one-second movement step only for stargates. This now covers both **gates and stations**, with a minimum 2,400m surface stand-off even for a manual **Approach 0m** to these large static objects. On completion it emits one combined `SetBallPosition`, `CmdStop` and zero `SetBallVelocity` update, rather than independent conflicting updates.
- `WarpStop()` used to send its zero-velocity/stop update separately from a following position correction. It now sends a single ordered position/stop/velocity packet batch after synchronizing the server's final position. `GetSpeed()` now reports actual velocity magnitude rather than the acceleration timer fraction, improving moving-target orbit prediction. Invalid movement speed/heading stops instead of propagating NaN positions.

Regression checks cover these source invariants plus C++ final-approach math (full speed, last partial tick, already-in-range, and invalid speed). The prior gate surface 2,400m/activation 2,500m and selected warp stand-offs remain unchanged.

**Scope and limits:** this is a source-path audit, not a complete per-line manual review of every file in the 1,399-entry repository tree. Key inspected files: `BeyonceService.cpp`, `Client.cpp`, `DestinyManager.cpp/.h`, `SystemBubble.cpp`, `BubbleManager.cpp`, `SystemEntity.cpp`, `DroneAI.cpp`, `EVE_MovementFormulas.h`, `gpoint.h`, Destiny state packets and movement regression tests. `CheckBump()` only notifies on player/player close contact: it **does not implement server-side stargate collision physics**. Therefore genuine client-mesh collision and micro-stutter elimination **cannot be certified** from unit tests or builds. If the client still bounces, collect the installed gate's ball radius/model collision shape, 2012-client Destiny stream and server timestamps before trying type-specific geometry adjustments.

**Required follow-up playtest:** frigate/battleship/capital warp at 0m, 10km and 100km; immediate gate jump from a fresh warp; manual Approach 0m to a stargate and station; autopilot approach/jump; queued docking; repeated idle for at least 60 simulation ticks; check client overview surface range, speed overlay, collision/wobble, frame pacing and network packet rates. Also test mining drone flight/return because `master` PR #42 updated the same Destiny subsystem. No database migration.

## Warp entry and arrival follow-up

- A validated warp sends one navigation command. Dock and Jump wait through alignment and active warp. Alignment throttle updates keep the warp order active.
- Warp entry resolves the landing from the ship's current server position. The deceleration curve reaches the endpoint at the warp exit speed, and the final tick publishes a single stopped position.
- Invalid coordinates or radii cannot satisfy a jump or dock check. Heavy hulls retain a positive turn rate and directly opposite headings can begin turning.
- The movement tests cover gate and large station geometry across small and capital hull sizes, off-axis approaches, and short- and long-warp deceleration. Live client movement and distance display still require the playtest above.

