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
assert "iRef->Delete();" in split and "qty >= m_data.quantity" in split
alter = items[items.index("bool InventoryItem::AlterQuantity("):items.index("bool InventoryItem::SetQuantity(", items.index("bool InventoryItem::AlterQuantity("))]
assert "if (new_qty < 0)" in alter
assert "Delete();" not in alter, "Invalid quantity must not delete existing stack"
assert "IsLegacyAgentThumbnail50(category, id, size)" in img
assert "Content-Length: " in connection
# The old embedded baseline50 thumbnail was deliberately removed after
# client crashes persisted. Keep the station agent response file-backed.
assert 'const bool localAgent50 = EvEImage::IsLegacyAgentThumbnail50(category, id, size);' in img
assert 'std::string path(GetFilePath(category, id, size));' in img
assert 'FILE * fp = fopen(path.c_str(), "rb");' in img
assert 'if (fp == NULL && localAgent50)' in img
assert '_basePath + "Character/agent_fallback_50.jpg"' in img
assert '&& !localAgent50' in img, "Never substitute a 512px portrait for an NPC 50px request"
assert 'Serving disk JPEG for agent %u at 50px from %s' in img
assert 'kLegacyAgentThumbnail50Bytes' not in img
thumbnail_header = src("src/eve-common/EVE_LegacyAgentThumbnail.h")
assert 'kLegacyAgentThumbnail50[]' not in thumbnail_header
assert 'IsLegacyAgentThumbnail50("Character", 3011918u, 50u)' in image_test
assert 'IsLegacyAgentThumbnail50("Character", 3011918u, 64u)' in image_test
assert 'EvEImage::HasJpegSignature(jpeg)' in image_test
assert 'EvEImage::HasJpegSignature(png)' in image_test
assert "baseline50" not in image_test, "Remove outdated embedded JPEG verification"
assert img.index('FILE * fp = fopen(path.c_str(), "rb");') < img.index('if (fp == NULL && localAgent50)')

# Inventory and assembled-container move crash regression checks.
container_h = src("src/eve-server/system/Container.h")
container_cpp = src("src/eve-server/system/Container.cpp")
inventory = src("src/eve-server/inventory/Inventory.cpp")
inventory_bound = src("src/eve-server/inventory/InventoryBound.cpp")
factory = src("src/eve-server/inventory/ItemFactory.cpp")
assert '&& (type.groupID() != EVEDB::invGroups::Mission_Container)' in container_h, (
    "Mission_Container loading must agree with InventoryItem's dispatch")
assert 'if (!sDataMgr.IsSolarSystem(locationID()))' in container_cpp, (
    "Empty assembled containers in stations or hangars must survive")
assert 'const auto test = mContents.emplace(iRef->itemID(), iRef);' in inventory, (
    "Inventory insertion must not inspect an uninitialized pair")
assert 'return;  // already indexed in mContents and m_contentsByFlag' in inventory
assert 'if (cRef.get() != nullptr && cRef->groupID()' in inventory, (
    "Missing source container must not be dereferenced")
assert 'if (pClient == nullptr || pClient->GetShip().get() == nullptr)' in inventory
assert 'if (iRef->itemID() == m_myID)' in inventory
assert 'if (pClient == nullptr || m_self.get() == nullptr || pInventory == nullptr)' in inventory_bound
assert 'if (!pInventory->ValidateAddItem(toFlag, iRef))' in inventory_bound
assert 'if (itemVec.empty())' in inventory_bound
assert 'if (iRef.get() == nullptr)' in inventory_bound
assert 'return GetItemRefFromID(parentID, load);' in factory, (
    "A missing parent cache entry must never dereference m_items.end()")
assert 'InventoryItemRef parent = GetItemContainerRef(itemID, load);' in factory
assert 'if (iRef->GetMyInventory() == nullptr)' in broker
assert 'if (item->GetMyInventory() == nullptr)' in broker

assert not (root / "tools/agent_portraits").exists()
tcp = src("src/eve-core/network/TCPConnection.cpp")
thread = src("src/eve-core/threading/Threading.cpp")
assert '"Releasing TCPConnection on thread %s"' in tcp
assert '"Destroying TCPConnection for thread 0x%X"' not in tcp
assert 'threadID << thread->get_id();' in thread
assert 'Created thread ID %s' in thread

print("Station agent and inventory safety source invariants passed")
