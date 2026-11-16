"""Source-level checks for independently confirmed Crucible station safety defects.

This does NOT verify that the actual 2012 client can enter a station.
"""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
def src(rel):
    return (root / rel).read_text(encoding="utf-8")

agent = src("src/eve-server/agents/AgentMgrService.cpp")
bound = src("src/eve-server/agents/AgentBound.cpp")
broker = src("src/eve-server/inventory/InvBrokerService.cpp")
items = src("src/eve-server/inventory/InventoryItem.cpp")
img = src("src/eve-server/imageserver/ImageServer.cpp")
connection = src("src/eve-server/imageserver/ImageServerConnection.cpp")
image_test = src("src/eve-test/utils/ImageCompatibilityTest.cpp")

assert "bindParameters == nullptr || !bindParameters->IsInt()" in agent
assert "cur.bookmarks != nullptr ? cur.bookmarks->Clone() : new PyList()" in agent
assert "offer.bookmarks != nullptr ? offer.bookmarks->Clone() : new PyList()" in bound
assert "if (containerInventory == nullptr)" in broker
split = items[items.index("InventoryItemRef InventoryItem::Split("):items.index("void InventoryItem::Relocate(", items.index("InventoryItemRef InventoryItem::Split("))]
assert split.index("sItemFactory.SpawnItem(idata)") < split.index("AlterQuantity(-qty, notify)")
assert "iRef->Delete();" in split and "qty > m_data.quantity" in split
alter = items[items.index("bool InventoryItem::AlterQuantity("):items.index("bool InventoryItem::SetQuantity(", items.index("bool InventoryItem::AlterQuantity("))]
assert "if (new_qty < 0)" in alter
assert "Delete();" not in alter, "Invalid quantity must not delete existing stack"
assert "IsLegacyAgentThumbnail50(category, id, size)" in img
assert "Content-Length: " in connection
assert "baseline50" in image_test
assert not (root / "tools/agent_portraits").exists()
print("Station agent and inventory safety source invariants passed")
