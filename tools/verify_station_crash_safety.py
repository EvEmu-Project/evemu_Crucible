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
assert "if (new_qty < 0 || new_qty > std::numeric_limits<int32>::max())" in alter
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
assert 'if (iRef->itemID() == m_myID || WouldCreateContainmentCycle(iRef))' in inventory
assert 'if (pClient == nullptr || m_self.get() == nullptr || pInventory == nullptr)' in inventory_bound
assert 'if (!pInventory->ValidateAddItem(toFlag, iRef))' in inventory_bound
assert 'if (itemVec.empty())' in inventory_bound
assert 'if (iRef.get() == nullptr)' in inventory_bound
assert 'return GetItemRefFromID(parentID, load);' in factory, (
    "A missing parent cache entry must never dereference m_items.end()")
assert 'InventoryItemRef parent = GetItemContainerRef(itemID, load);' in factory
assert 'if (iRef->GetMyInventory() == nullptr)' in broker
assert 'if (item->GetMyInventory() == nullptr)' in broker

# Assembled CargoContainer inherits StructureItem, but has no POS system
# entity while in a station. Both inherited AddItem and Rename previously
# dereferenced an uninitialized StructureItem::mySE and could terminate the
# Windows process on drag/drop or rename. Keep these cases covered in CI.
structure = src("src/eve-server/pos/Structure.cpp")
assert 'mySE(nullptr)' in structure.split('StructureItem::~StructureItem', 1)[0]
assert 'mySE != nullptr && mySE->IsCOSE()' in structure
assert 'if (mySE == nullptr || mySE->GetPOSSE() == nullptr)' in structure
assert 'virtual void Rename(std::string name) override;' in container_h
assert 'void CargoContainer::Rename(std::string name)' in container_cpp
cargo_rename = container_cpp.split('void CargoContainer::Rename(std::string name)', 1)[1].split('CargoContainerRef CargoContainer::Load(', 1)[0]
assert 'InventoryItem::Rename(name);' in cargo_rename
assert 'if (sDataMgr.IsSolarSystem(locationID()) && !m_isAnchored)' in cargo_rename
cargo_ctor = container_cpp.split('CargoContainer::CargoContainer(', 1)[1].split('CargoContainer::~CargoContainer', 1)[0]
assert 'pInventory = new Inventory(' not in cargo_ctor, (
    "StructureItem already allocates the inventory; do not leak a second one")

# Cross-cutting inventory, POS, and persistence safety after the full transfer audit.
inventory_h = src("src/eve-server/inventory/Inventory.h")
pos_h = src("src/eve-server/pos/Structure.h")
pos_mgr = src("src/eve-server/pos/PosMgr.cpp")
tower_h = src("src/eve-server/pos/Tower.h")
tower_cpp = src("src/eve-server/pos/Tower.cpp")
item_db = src("src/eve-server/inventory/ItemDB.cpp")
assert 'bool WouldCreateContainmentCycle(InventoryItemRef item) const;' in inventory_h
assert 'bool Inventory::WouldCreateContainmentCycle(InventoryItemRef item) const' in inventory
assert 'std::set<uint32> visited;' in inventory
assert 'if (pInventory->WouldCreateContainmentCycle(iRef))' in inventory_bound
assert 'ScopedInventoryClient factoryClient(pClient);' in inventory_bound
assert 'ScopedInventoryClient factoryClient(call.client);' in inventory_bound
assert 'if (qty < 1 || qty > srcItem->quantity())' in inventory_bound
assert 'if (ship && (pShip == nullptr || m_self->GetShipItem() == nullptr))' in inventory_bound
assert 'if (!pInventory->ValidateAddItem(toFlag, iRef))' in inventory_bound
assert 'iRef->Move(m_itemID, toFlag, true);' in inventory_bound, "Customs must persist location changes"
assert 'Inventory* parentInventory = iRef->GetMyInventory();' in items
assert 'if (parentInventory != nullptr)' in items
assert 'to_merge.get() == this' in items
assert 'qty > std::numeric_limits<int32>::max() - m_data.quantity' in items
assert 'const int64 new_qty = static_cast<int64>(m_data.quantity) + qty;' in items
assert 'if (qty < 0)' in items
assert 'if (pClient->IsDocked() || pClient->GetShipSE() == nullptr' in items
assert 'if (mySE == nullptr || mySE->GetPOSSE() == nullptr)' in structure
assert 'if (!InitData())' in structure
assert 'm_towerSE == nullptr' in structure
assert 'bool StructureSE::InitData()' in structure
assert 'virtual bool                InitData();' in pos_h
assert 'virtual bool                InitData();' in tower_h
assert 'bool TowerSE::InitData()' in tower_cpp
assert 'IsPOSInitialized()' in tower_cpp
assert 'GetCheckedTowerSE(SystemManager* system' in pos_mgr
assert 'GetCheckedStructureSE(SystemManager* system' in pos_mgr
assert 'sDatabase.DoEscapeString(customInfoEsc, cur.customInfo);' in item_db
sql_audit = src("sql/diagnostics/2026-11-16-container-pos-integrity.sql")
assert 'SELECT itemID, itemName, typeID' in sql_audit
assert 'SHOW INDEX FROM entity;' in sql_audit
assert 'DELETE FROM ENTITY' not in sql_audit.upper(), "SQL audit must be read-only"

# Server branding is independent of the 2012 Crucible wire protocol.
server_version = src("src/eve-common/EVEVersion.h")
assert 'REVISION_STRING = std::string("0.8.7-Rizonx");' in server_version
assert 'static const double EVEVersionNumber = 7.31;' in server_version
assert 'static const uint16 MachoNetVersion = 320;' in server_version
assert 'static const int32 EVEBuildVersion = 360229;' in server_version

assert not (root / "tools/agent_portraits").exists()
tcp = src("src/eve-core/network/TCPConnection.cpp")
thread = src("src/eve-core/threading/Threading.cpp")
assert '"Releasing TCPConnection on thread %s"' in tcp
assert '"Destroying TCPConnection for thread 0x%X"' not in tcp
assert 'threadID << thread->get_id();' in thread
assert 'Created thread ID %s' in thread

# Regression guards for MariaDB #1064 truncated bulk entity INSERTs and
# corporation tax updates (the Windows server log ended at customInfo '').
dbcore = src("src/eve-core/database/dbcore.cpp")
escape = dbcore.split("void DBcore::DoEscapeString(std::string &to, const std::string &from)", 1)[1].split(
    "bool DBcore::IsSafeString", 1)[0]
assert "to.resize(static_cast<size_t>(len) * 2 + 1);" in escape
assert "to.resize(esc_len);" in escape
assert "to.resize(esc_len + 1)" not in escape
item_db = src("src/eve-server/inventory/ItemDB.cpp")
assert item_db.count('sDatabase.RunQuery(err, "%s", Inserts.str().c_str())') == 2
assert item_db.count('Inserts << " ON DUPLICATE KEY UPDATE "') == 2
assert "if (data.empty())" in item_db
service_db = src("src/eve-server/ServiceDB.cpp")
assert "std::setprecision(6) << newValue;" in service_db
corp_db = src("src/eve-server/corporation/CorporationDB.cpp")
corp_update = corp_db.split("bool CorporationDB::UpdateCorporation(", 1)[1].split(
    "bool CorporationDB::UpdateLogo(", 1)[0]
assert 'sDatabase.RunQuery(res.error, "%s", query.c_str())' in corp_update
assert 'query += std::to_string(corpID);' in corp_update
corp_bound = src("src/eve-server/corporation/CorpRegistryBound.cpp")
corp_edit = corp_bound.split("PyResult CorpRegistryBound::UpdateCorporation(", 1)[1].split(
    "PyResult CorpRegistryBound::UpdateLogo(", 1)[0]
assert "call.client->GetCorporationID() != m_corpID" in corp_edit
assert "m_db.GetCorporationCEO(m_corpID)" in corp_edit
assert "Corp::Role::Director" in corp_edit
assert "PyRep::FloatValue(tax)" in corp_edit
assert "taxRate < 0.0 || taxRate > 1.0" in corp_edit
assert "UpdateCorpTaxRate(" in corp_edit
assert "sEntityList.CorpNotify(" in corp_edit
assert "sEntityList.Multicast(" not in corp_edit

print("Station agent and inventory safety source invariants passed")
