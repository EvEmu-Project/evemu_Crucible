"""Source-level guard against accidental reintroduction of experimental NPC portraits.

Does not assert that a 2012 Crucible client can load a station: requires a
real deployment, image HTTP check, and client crash dump.
"""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
conn = (root / "src/eve-server/imageserver/ImageServerConnection.cpp").read_text(encoding="utf-8")
helper = (root / "src/eve-common/EVE_AgentHelpers.h").read_text(encoding="utf-8")
image = (root / "src/eve-server/imageserver/ImageServer.cpp").read_text(encoding="utf-8")
get_pos = conn.index("_imageData = sImageServer.GetImage(_category, _id, _size);")
guard = conn.index('if (_category == "Character")')
guard_text = conn[guard:get_pos]
assert guard < get_pos, "Character guard must precede image reads"
assert "NotFound();" in guard_text and "return;" in guard_text, "Guard must return 404"
assert "IsStationAgentPortraitRequest" not in conn + helper
assert "GetOfflineAgentPortrait" not in image
assert "agent_missing_512.jpg" not in image
assert not (root / "tools/agent_portraits").exists(), "Experimental portrait importer must be removed"
assert not (root / "assets/agent_portraits").exists(), "Experimental JPEG asset must be removed"
print("Character HTTP image isolation source checks passed")
