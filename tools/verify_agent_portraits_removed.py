"""Check that the experimental station NPC portrait feature was fully removed.

This is a source-level guard, NOT an in-game Crucible station-load test.
"""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
image_cpp = (root / "src/eve-server/imageserver/ImageServer.cpp").read_text(encoding="utf-8")
image_h = (root / "src/eve-server/imageserver/ImageServer.h").read_text(encoding="utf-8")
connection = (root / "src/eve-server/imageserver/ImageServerConnection.cpp").read_text(encoding="utf-8")
agent_helpers = (root / "src/eve-common/EVE_AgentHelpers.h").read_text(encoding="utf-8")
all_image_code = image_cpp + image_h + connection + agent_helpers

assert "IsStationAgentPortraitRequest" not in all_image_code
assert "GetOfflineAgentPortrait" not in all_image_code
assert "agent_missing_512.jpg" not in all_image_code
assert "if (_category == \"Character\")" not in connection, "Do not introduce new Character guards"
assert "_imageData = sImageServer.GetImage(_category, _id, _size);" in connection
assert "EvEAgent::ModernImageUrl(_category, _id, _size)" in connection
assert not (root / "tools/agent_portraits").exists()
assert not (root / "assets/agent_portraits").exists()
print("Experimental agent image code and assets are fully removed")
