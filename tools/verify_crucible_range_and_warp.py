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
assert "m_shipWarpSpeed = 3.0f * warpSpeedMultiplier;" in destiny
assert "SetAttribute(AttrWarpSpeedMultiplier, EvilOne, false)" in ship
assert "SurfaceSeparation(" in active
assert "rangeSource->GetPosition().distance(m_targetSE->GetPosition())" in active
assert "rangeSource->GetRadius(), m_targetSE->GetRadius()" in active
assert "SurfaceSeparation(" in mining and "separation > miningRange" in mining
assert "liveShip->GetPosition().distance(m_targetSE->GetPosition())" in mining
assert "GetAttribute(AttrMaxRange).get_double()" in turret
assert "GetAttribute(AttrFalloff).get_double()" in turret
assert "WHERE g.categoryID = 6" in sql and "attributeID = 600" in sql
assert "au_per_second_if_legacy_multiplier" in sql
print("Crucible module ranges guarded; historical warp conversion preserved.")
