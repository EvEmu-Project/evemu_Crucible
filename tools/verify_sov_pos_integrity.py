#!/usr/bin/env python3
"""Source-level sovereignty/POS and SQL guards."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
def read(path):
    return (ROOT / path).read_text(encoding="utf-8")

sov = read("src/eve-server/system/sov/SovereigntyDB.cpp")
mgr = read("src/eve-server/system/sov/SovereigntyDataMgr.cpp")
sbu = read("src/eve-server/pos/sovStructures/SBU.cpp")
pos = read("src/eve-server/pos/PosMgr.cpp")
db = read("src/eve-server/pos/PosMgrDB.cpp")
cmake = read("src/eve-test/CMakeLists.txt")
migration = read("sql/migrations/20210409120025-sov.sql")

assert "MODIFY COLUMN " in migration and "claimTime" in migration and "bigint(20)" in migration
assert "EffectiveClaimTime(data.claimTime, GetFileTimeNow())" in sov
assert '" VALUES (%u, %u, %u, %u, %lli, %u, %u)"' in sov
assert "bool SovereigntyDB::AddSovereigntyData" in sov
assert "bool SovereigntyDB::RemoveSovereigntyData" in sov
assert "bool SovereigntyDB::GetSovereigntyDataForSystem" in sov
assert "if (!SovereigntyDB::AddSovereigntyData(data, claimID))" in mgr
assert "if (!SovereigntyDB::RemoveSovereigntyData(systemID))" in mgr
assert "if (!SovereigntyDB::GetSovereigntyDataForSystem(*res, systemID))" in mgr
assert "CountDistinctNonzeroGateIDs(activeGateIDs)" in sbu
assert "unit->GetGateSE() != nullptr" in sbu
assert "CanMutatePOS(Client* client" in pos
assert pos.count("GetAuthorizedTowerSE(call.client, pSystem, itemID->value(), m_systemID)") == 5
assert "pTSE->GetOwnerID() != call.client->GetCharID()" in pos
assert "GetSiloCapacityForType(static_cast<uint16>(silo->GetTypeID()))" in pos
assert "if (deployArgs == nullptr || !deployArgs->IsDict()" in pos
assert "if (deploy->size() < 4)" in pos
assert "if (row->GetItem(i) == nullptr || !row->GetItem(i)->IsInt())" in pos
assert "local.toItemID != 0 || remote.toItemID != 0" in pos
assert "INNER JOIN posTowerData AS d USING (itemID)" in db
structure = read("src/eve-server/pos/Structure.cpp")
assert "m_gateSE->GetPosition().distance(pos)" in structure
assert "m_planetSE->GetPosition().distance(pos)" in structure
assert "m_gateSE->GetPosition().distance(m_self->position())" not in structure
assert "m_planetSE->GetPosition().distance(m_self->position())" not in structure
assert '" SET standing=%f, status=%f, statusDrop=%i' in db
assert '" SET anchor=%i, unanchor=%i, online=%i' in db
assert '"INNER JOIN posJumpBridgeData AS local "' in db
assert "remote.toItemID=local.itemID" in db
assert "posDB.DeleteData(itemID());" in structure
assert "void StructureItem::Delete()" in structure
assert "UninstallRemoteBridgeLink(itemID);" in db
assert "SovereigntyPosIntegrityTest.cpp" in cmake
assert 'NAME "SovereigntyPosIntegrityTest"' in cmake
print("PASS: sovereignty FILETIME, cache, SBU, POS permissions and SQL invariants")


# DEV/ADMIN command and client index views must stay in sync. The test
# persists a TCU-bound marker and must never edit the real claimTime.
from pathlib import Path as _Path
_sov_header = read("src/eve-common/EVE_SovIndexOverride.h")
_sovdb = read("src/eve-server/system/sov/SovereigntyDB.cpp")
_sovmanager = read("src/eve-server/system/sov/SovereigntyDataMgr.cpp")
_indices = read("src/eve-server/system/IndexManager.cpp")
_commands = read("src/eve-server/admin/SystemCommands.cpp")
_command_acl = read("src/eve-server/admin/SystemCommands.h")
_migration = read("sql/migrations/20261118070000-sov-development-index-overrides.sql")
assert "COMMAND_ANY( sovindex5, Acct::Role::DEV, Acct::Role::ADMIN" in _command_acl
assert "Command_sovindex5(" in _commands
assert "sysData.factionID" in _commands
assert "!claim.hubID" in _commands
assert "ForceLevelFiveIndices(systemID)" in _commands
assert "ResetIndexOverrides(systemID)" in _commands
assert "mapSovDevelopmentIndexOverrides" in _sovdb
# Upsert uses the named parameterized values; inspect the source constant
# rather than expecting the old hard-coded strategicDays=100 SQL assignment.
assert "strategicDays=VALUES(strategicDays)" in _sovdb
assert "EVESovIndex::LevelFiveStrategicDays" in _sovdb
assert "LevelFiveStrategicDays = 100" in _sov_header
assert "claimStructureID=VALUES(claimStructureID)" in _sovdb
assert "AND claimStructureID=%u" in _sovdb
assert "GetIndexOverrides(overrideRows)" in _sovmanager
assert "SovereigntyDB::DeleteIndexOverride(systemID, oldClaim.claimStructureID)" in _sovmanager
assert "GetEffectiveStrategicDays(sData)" in _sovmanager
assert "GetAllDevelopmentIndicesForClients()" in _indices
assert "GetEffectiveMilitaryPoints(sovData)" in _indices
assert "GetEffectiveIndustrialPoints(sovData)" in _indices
assert "EffectiveStrategicDays" in _sov_header
assert "LevelFiveMilitaryPoints = 1000000000U" in _sov_header
assert "LevelFiveIndustrialPoints = 30000000U" in _sov_header
assert "std::uint32_t militaryPoints;" in _sov_header
assert "std::uint32_t industrialPoints;" in _sov_header
assert 'header->AddColumn("points", DBTYPE_I4)' in _indices
assert 'header->AddColumn("militaryPoints", DBTYPE_I4)' in _sovmanager
assert 'header->AddColumn("industrialPoints", DBTYPE_I4)' in _sovmanager
assert "static_cast<uint16>(overrideRow.GetUInt(2))" not in _sovmanager
assert "militaryPoints=VALUES(militaryPoints)" in _sovdb
assert "industrialPoints=VALUES(industrialPoints)" in _sovdb
_point_width = read("sql/migrations/20261118093000-sov-development-index-point-width.sql")
assert "MODIFY militaryPoints INT(10) UNSIGNED" in _point_width
assert "MODIFY industrialPoints INT(10) UNSIGNED" in _point_width
assert "WHERE militaryPoints = 5 AND industrialPoints = 5" in _point_width
assert "CREATE TABLE IF NOT EXISTS mapSovDevelopmentIndexOverrides" in _migration
assert "claimStructureID" in _migration
assert "ALTER TABLE mapSystemSovInfo" not in _migration
print("PASS: /sovindex5 role, claim-bound SQL persistence, index views and reset guards")

# Ensure the C++ raw-point test keeps its boolean condition chained.
_unit_test = read("src/eve-test/utils/SovIndexOverrideTest.cpp")
assert "&& EVESovIndex::IsActive(140000001, test)" in _unit_test

# Supercapital Construction Facilities must be installable with a true or
# DEV-only simulated Strategic Level 5; older upgrades preserve their rules.
_rules = read("src/eve-common/EVE_IHubUpgradeRules.h")
_inv = read("src/eve-server/inventory/Inventory.cpp")
assert "SupercapitalConstructionFacilities = 2009U" in _rules
assert "case SupercapitalConstructionFacilities: return 100U;" in _rules
assert "effectiveDays >= required" in _rules
assert "GetEffectiveStrategicDays(sovData)" in _inv
assert "EVESovIHub::MeetsStrategicRequirement(iRef->typeID(), effectiveDays)" in _inv
assert "sovData.hubID != m_self->itemID()" in _inv
assert "pClient->GetAllianceID() != sovData.allianceID" in _inv
assert "installed.second->typeID() == iRef->typeID()" in _inv
assert "iRef->categoryID() == EVEDB::invCategories::StructureUpgrade" in _inv
assert "EVEItemFlags::flagStructureActive, true" in _inv
assert 'NAME "IHubUpgradeRulesTest"' in cmake
print("PASS: IHub supercapital Strategic 5 installation, owner and duplicate guards")


# Three IHub upgrade classes, metadata dependencies and tower shield lifecycle.
_fullIHubRules = read("src/eve-common/EVE_IHubUpgradeRules.h")
_fullInv = read("src/eve-server/inventory/Inventory.cpp")
_struct = read("src/eve-server/pos/Structure.cpp")
_tower = read("src/eve-server/pos/Tower.cpp")
_shield = read("src/eve-common/EVE_TowerShieldRules.h")
_sysmgr = read("src/eve-server/system/SystemManager.cpp")
_bubble = read("src/eve-server/system/SystemBubble.cpp")
assert "StrategicGroup = 1016U" in _fullIHubRules
assert "IndustrialGroup = 1020U" in _fullIHubRules
assert "MilitaryGroup = 1021U" in _fullIHubRules
assert "MeetsActivityRequirement" in _fullIHubRules
assert "IsSupportedUpgrade(iRef->typeID(), iRef->groupID())" in _fullInv
assert "AttrsovUpgradeRequiredUpgradeID" in _fullInv
assert "AttrsovUpgradeBlockingUpgradeID" in _fullInv
assert "AttrdevIndexIndustrial" in _fullInv
assert "AttrdevIndexMilitary" in _fullInv
assert "StarbaseConfig" in _fullInv
assert "if (upgType != 0)" in _struct
assert "ContainsTypeQtyByFlag(upgType, flagStructureActive, 1)" in _struct
assert "m_db.UpdateHarmonicAndPassword(m_data.itemID, m_tdata)" in _tower
assert "EVEPOS::ShieldRules::EffectiveRadius(m_tsize, configuredRadius)" in _tower
assert "(!m_tdata.password.empty())" not in _tower
assert "std::isfinite(configuredRadius)" in _shield
assert "!cur.second->IsMissileSE() && !cur.second->IsFieldSE()" in _sysmgr
assert "!cur.second->IsMissileSE() && !cur.second->IsFieldSE()" in _bubble
assert 'NAME "TowerShieldRulesTest"' in cmake
print("PASS: IHub group coverage and tower shield visual lifecycle guards")


# Baseline IHub actual site effect wiring and dungeon creation success.
_anomaly = read("src/eve-server/system/cosmicMgrs/AnomalyMgr.cpp")
_dungeon = read("src/eve-server/system/cosmicMgrs/DungeonMgr.cpp")
assert "sov.hubID && sov.allianceID" in _anomaly
assert "item.second->flag() != flagStructureActive" in _anomaly
assert "EVESovIHub::KindForGroup(item.second->groupID())" in _anomaly
assert "sovMilitarySites < 5" in _anomaly
assert "sovIndustrialSites < 5" in _anomaly
assert "m_typeList.push_back(Dungeon::Type::Anomaly);" in _anomaly
assert "m_typeList.push_back(Dungeon::Type::Gravimetric);" in _anomaly
assert "dData.dungeonID == 0" in _dungeon
assert "m_system->GetBeltMgr()->Create(sig, ore)" in _dungeon
assert "return true; // Previously always returned false" in _dungeon
print("PASS: active IHub upgrade to combat/mineable site wiring and dungeon success result")
