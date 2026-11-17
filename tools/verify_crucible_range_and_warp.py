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
