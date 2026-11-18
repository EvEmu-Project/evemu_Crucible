#!/usr/bin/env python3
"""Non-destructive CI source guard for audited Crucible module range behaviour."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
def source(path):
    return (ROOT / path).read_text(encoding="utf-8")

destiny = source("src/eve-server/system/DestinyManager.cpp")
ship = source("src/eve-server/ship/Ship.cpp")
active = source("src/eve-server/ship/modules/ActiveModule.cpp")
mining = source("src/eve-server/ship/modules/MiningLaser.cpp")
turret = source("src/eve-server/ship/modules/TurretFormulas.cpp")
sql = source("sql/diagnostics/2026-11-17-crucible-ship-weapon-range-warp-audit.sql")

# Preserve historical conversion while the archive/DB encoding is unverified.
assert "EVEmuMovement::CrucibleWarpSpeedAU(" in destiny
assert "m_shipWarpSpeed = 3.0f * warpSpeedMultiplier;" not in destiny
assert "SetAttribute(AttrWarpSpeedMultiplier, 3.0f, false)" in ship
assert "SurfaceSeparation(" in active
assert "rangeSource->GetPosition().distance(m_targetSE->GetPosition())" in active
assert "rangeSource->GetRadius(), m_targetSE->GetRadius()" in active
assert "SurfaceSeparation(" in mining and "separation > miningRange" in mining
assert "liveShip->GetPosition().distance(m_targetSE->GetPosition())" in mining
assert "GetAttribute(AttrMaxRange).get_double()" in turret
assert "GetAttribute(AttrFalloff).get_double()" in turret
assert "WHERE g.categoryID = 6" in sql and "attributeID = 600" in sql
assert "au_per_second_if_legacy_multiplier" in sql
repair = source("sql/diagnostics/2026-11-17-crucible-warp-speed-verified-repair.sql")
assert "11985,9,3.75" in repair and "11987,9,3.75" in repair
assert "11989,9,3.75" in repair and "11978,9,3.75" in repair
assert "a.attributeID=600" in repair and "v.expected_old IS NULL" in repair
assert "tmp_crucible_warp_fix" in repair
print("Crucible module ranges and direct per-hull warp-speed calculation guarded.")

# Ferox alignment must not use the acceleration timer as a speed check or
# override the real heading/speed gate after the estimated align time.
alignment = destiny[destiny.index("void DestinyManager::ProcessState()"):destiny.index("void DestinyManager::SetSpeedFraction(")]
assert "m_activeSpeedFraction >= 0.75f" in alignment
assert "m_timeFraction > 0.749" not in alignment
assert "warp align/speed is incorrect" not in alignment
assert "headingReady && speedReady" in alignment
# STOP must process real deceleration but never emit a CmdStop every idle
# tick. This prevents repeated network prediction resets and packet spam.
idle_state = alignment.split("case Ball::Mode::STOP:", 1)[1].split(
    "case Ball::Mode::GOTO:", 1)[0]
assert "m_decel || m_accel || m_activeSpeedFraction > 0.0001f" in idle_state
assert "Stop();" not in idle_state
assert "SendSingleDestinyUpdate(&update);" in idle_state
assert "return static_cast<float>(m_velocity.length());" in source("src/eve-server/system/DestinyManager.h")
# Stop() must not fall through to a stale Follow() movement tick.
follow = destiny.split("void DestinyManager::Follow()", 1)[1].split("void DestinyManager::Orbit()", 1)[0]
assert "Stop();" in follow
assert "SetPosition(m_position, true);\n            return;" in follow
assert "EVEmuMovement::TractorTravelStep(" in follow

# Warping to stargates or other spherical targets must use the actual
# surface separation, not the old gate radius/3 fudge and ship diameter.
formulas = source("src/eve-common/EVE_MovementFormulas.h")
beyonce = source("src/eve-server/ship/BeyonceService.cpp")
client = source("src/eve-server/Client.cpp")
movement_test = source("src/eve-test/utils/MovementFormulaTest.cpp")
assert "WarpCenterStopRange(double requestedSurfaceRange" in formulas
assert "GateWarpLandingSurfaceRange = 2400.0" in formulas
assert "GateJumpSurfaceRange = 2500.0" in formulas
assert "surface + std::max(0.0, shipRadius) + std::max(0.0, targetRadius)" in formulas
assert beyonce.count("EVEmuMovement::WarpCenterStopRange(") == 2
assert "distance += (radius / 3)" not in beyonce
assert "hasCenterBasedWarpRange" in beyonce
assert "if (pSE != nullptr && !hasCenterBasedWarpRange)" in beyonce
assert "if (autoPilot && pSE != nullptr)" in destiny
assert "const double surfaceRange = EVEmuMovement::SurfaceSeparation(" in destiny
assert client.count("EVEmuMovement::GateWarpLandingSurfaceRange") == 2
assert client.count("EVEmuMovement::GateJumpSurfaceRange") == 2
assert "gate->GetRadius() + 2500.0" not in client
assert "station->GetRadius() + 2500.0" not in destiny
assert "Follow(station, approachDistance);" in destiny
assert "if (pSE->IsGateSE() || pSE->IsStationSE())" in destiny
assert "distance = std::max<uint32>(distance" in destiny
assert "gateFrigateStop == 12435 && gateCapitalStop == 14900" in movement_test
assert "requestedTenKm == 20150" in movement_test

# A 1-second movement tick must not carry a ship through the gate after
# Follow() calculates a safe 2.4km (or manual 0m) surface stand-off.
move = destiny.split("void DestinyManager::MoveObject()", 1)[1].split(
    "bool DestinyManager::IsTurn()", 1)[0]
assert "m_targetEntity.second->IsGateSE()" in move
assert "m_targetEntity.second->IsStationSE()" in move
assert "const double remaining = std::max(0.0, gap - m_followDistance);" in move
assert "if (remaining <= static_cast<double>(speed))" in move
assert "finalPosition += toTarget * remaining;" in move
assert "SetPosition(finalPosition);\n                Halt();" in move
assert "position.Encode(), stop.Encode(), velocity.Encode()" in move
assert "SendDestinyUpdate(updates);" in move
assert "m_shipHeading.isNaN() || m_shipHeading.isInf()" in move

# Repeated Guristas NPC errors: the original orbit equation can compute a
# center radius inside the physically safe hull-to-hull clearance. Clamp
# recoverable geometry and use a static-data speed fallback for movable NPCs.
npc = source("src/eve-server/npc/NPCAI.cpp")
assert "ResolvedOrbitRadius(double minimumRadius" in formulas
assert "NpcCruiseSpeed(double cruiseSpeed" in formulas
assert "EVEmuMovement::ResolvedOrbitRadius(" in destiny
assert "orbitRadius < safeOrbitRadius" not in destiny
assert "EVEmuMovement::NpcCruiseSpeed(" in destiny
assert "EVEmuMovement::NpcCruiseSpeed(" in npc
assert "sEntityList.GetStamp() % 30u == 0u" in destiny
assert "mySE->IsNPCSE() ? DESTINY__WARNING : DESTINY__ERROR" in destiny
assert "if (m_orbitSpeed == 0)" in npc
assert "fixedGuristasOrbit == 10150.0" in movement_test
assert "fixedLargeOrbit == 20150.0" in movement_test
assert "zeroNpcCruise == 190.0" in movement_test

# A warp launch uses the requested destination plus a range. The final
# authoritative stop uses the already range-adjusted endpoint.
assert "wt.dest_x = m_warpDestination.x;" in destiny
assert "m_stopDistance = npcWarp ? distance : std::max(0, distance);" in destiny
assert "m_stopDistance < 0 ? m_targetPoint : m_warpDestination" in destiny
assert "WarpRemainingAfterAcceleration(m_warpState->total_distance, currentDistance)" in destiny
warp_stop = destiny.split("void DestinyManager::WarpStop(", 1)[1].split("void DestinyManager::EntityRemoved(", 1)[0]
assert warp_stop.index("SetPosition(m_targetPoint);") < warp_stop.index("Halt();")
assert "SetBallVelocity velocity;" in warp_stop
assert "position.Encode(), stop.Encode(), velocity.Encode()" in warp_stop
assert "SetPosition(m_position, true);" not in warp_stop
wreck = source("src/eve-server/system/Container.cpp").split("void WreckSE::EncodeDestiny", 1)[1]
assert "tractored ? Ball::Mode::FOLLOW : Ball::Mode::TROLL" in wreck
invbroker = source("src/eve-server/inventory/InvBrokerService.cpp")
assert "case EVEDB::invGroups::Wreck:" in invbroker
assert "shipSE->GetPosition().distance(containerSE->GetPosition())" in invbroker
assert "shipSE->GetRadius(), containerSE->GetRadius()" in invbroker

remaining = source("sql/diagnostics/2026-11-17-crucible-warp-speed-remaining-14-repair.sql")
assert "tmp_crucible_remaining_hulls" in remaining
for typeID in (22448,22466,22468,22474,22442,22470,22444,22446,29986,29990,29988,29984,2863,2998):
    assert f"({typeID},'" in remaining
assert "WHERE a.typeID IS NULL" in remaining and "t.published=1" in remaining
