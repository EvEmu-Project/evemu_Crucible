#!/usr/bin/env python3
"""Protect the synthetic Angel Cartel Blockade mission's 12-ship fleet."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
spawn = (ROOT / "src/eve-server/system/cosmicMgrs/SpawnMgr.cpp").read_text(encoding="utf-8")
missions = (ROOT / "src/eve-common/EVE_Missions.h").read_text(encoding="utf-8")
briefings = (ROOT / "src/eve-server/agents/AgentBound.cpp").read_text(encoding="utf-8")
docs = (ROOT / "docs/missions/BLOCKADE_TEST_MODE.md").read_text(encoding="utf-8")

mix = re.search(r"kAngelCartelBlockadeTestMix\[\]\s*=\s*\{(.*?)\};", spawn, re.DOTALL)
assert mix is not None, "Blockade mission fleet composition missing"
ships = re.findall(r'\{(\d+),\s*"([^"]+)",\s*(\d+)\}', mix.group(1))
assert ships == [
    ("551", "Asteroid Angel Cartel Cruiser", "8"),
    ("576", "Asteroid Angel Cartel BattleCruiser", "4"),
], f"Unexpected Blockade ship classes or quantities: {ships!r}"
assert sum(int(qty) for _, _, qty in ships) == 12
assert re.search(r"kBlockadeSecurityTestRequiredKills\s*=\s*12\s*;", missions)
assert "const uint32 fleetSize = Mission::kBlockadeSecurityTestRequiredKills;" in spawn
assert "if (selectedFleet.size() != fleetSize)" in spawn
assert briefings.count('"- 8 cruisers\\n"') == 4
assert briefings.count('"- Destroy 8 cruisers\\n"') == 1
assert briefings.count('"- 4 battlecruisers\\n\\n"') == 5
assert 'Kills confirmed: 0 / 12' in briefings
assert '"- 12 frigates\\n"' not in briefings
assert '"- 4 destroyers\\n"' not in briefings
assert "12 Angel Cartel NPC ships" in docs
print("Blockade 12-ship cruiser/battlecruiser mission guards passed.")
