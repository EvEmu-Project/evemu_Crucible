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
assert "strategicDays=100" in _sovdb
assert "claimStructureID=VALUES(claimStructureID)" in _sovdb
assert "AND claimStructureID=%u" in _sovdb
assert "GetIndexOverrides(overrideRows)" in _sovmanager
assert "SovereigntyDB::DeleteIndexOverride(systemID, oldClaim.claimStructureID)" in _sovmanager
assert "GetEffectiveStrategicDays(sData)" in _sovmanager
assert "GetAllDevelopmentIndicesForClients()" in _indices
assert "GetEffectiveMilitaryPoints(sovData)" in _indices
assert "GetEffectiveIndustrialPoints(sovData)" in _indices
assert "EffectiveStrategicDays" in _sov_header
assert "CREATE TABLE IF NOT EXISTS mapSovDevelopmentIndexOverrides" in _migration
assert "claimStructureID" in _migration
assert "ALTER TABLE mapSystemSovInfo" not in _migration
print("PASS: /sovindex5 role, claim-bound SQL persistence, index views and reset guards")
