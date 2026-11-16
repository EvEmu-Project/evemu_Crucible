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
# The removed experimental agent-portrait feature must stay absent, but the
# Crucible client compatibility fix intentionally denies missing Character
# images with HTTP 404. Validate the guard rather than forbidding it.
lookup = connection.index("_imageData = sImageServer.GetImage(_category, _id, _size);")
guard = connection.index('if (_category == "Character") {', lookup)
redirect = connection.index("Redirect();", guard)
guard_code = connection[guard:redirect]
assert lookup < guard < redirect
assert "NotFound();" in guard_code and "return;" in guard_code, "Missing character images must not redirect"
assert "returning 404 without redirect" in guard_code
assert 'if (category == "Character")' in agent_helpers
assert "EvEImage::HasJpegSignature(*ret)" in image_cpp
assert "_imageData = sImageServer.GetImage(_category, _id, _size);" in connection
assert "EvEAgent::ModernImageUrl(_category, _id, _size)" in connection
assert not (root / "tools/agent_portraits").exists()
assert not (root / "assets/agent_portraits").exists()
print("Experimental NPC portrait assets remain removed; missing Character images are denied locally")
