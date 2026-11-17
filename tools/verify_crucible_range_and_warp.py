#!/usr/bin/env python3
"""Crucible-era hull/weapon range source invariants (no post-Rubicon hardcodes)."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
def src(path):
    return (ROOT / path).read_text(encoding="utf-8")

destiny = src("src/eve-server/system/DestinyManager.cpp")
ship = src("src/eve-server/ship/Ship.cpp")
active = src("src/eve-server/ship/modules/ActiveModule.cpp")
mining = src("src/eve-server/ship/modules/MiningLaser.cpp")
turret = src("src/eve-server/ship/modules/TurretFormulas.cpp")
movement = src("src/eve-common/EVE_MovementFormulas.h")
sql = src("sql/diagnostics/2026-11-17-crucible-ship-weapon-range-warp-audit.sql")
assert "ResolveCrucibleWarpSpeedAU(liveWarpSpeed, hullWarpSpeed)" in destiny
assert "m_shipWarpSpeed = 3.0f *" not in destiny
assert "SetAttribute(AttrWarpSpeedMultiplier, 3.0f, false)" in ship
assert "SurfaceSeparation(" in active
assert "m_shipRef->radius(), m_targetSE->GetRadius()" in active
assert "SurfaceSeparation(" in mining and "separation > miningRange" in mining
assert "GetAttribute(AttrMaxRange).get_double()" in turret
assert "GetAttribute(AttrFalloff).get_double()" in turret
assert "ResolveCrucibleWarpSpeedAU" in movement
assert "WHERE g.categoryID = 6" in sql and "attributeID = 600" in sql
assert not any(line.lstrip().upper().startswith(("UPDATE ", "DELETE ", "INSERT ", "DROP ", "TRUNCATE "))
               for line in sql.splitlines())
print("Crucible range/warp source invariants passed.")
