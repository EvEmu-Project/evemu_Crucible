"""Source-level invariants for the gameplay systems audit.

These checks identify accidental regression of verified source-level fixes.
They do not certify in-game behavior, historical static data or unimplemented
mining drones, contracts, invention or mailing-list administration.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def source(path):
    return (ROOT / path).read_text(encoding="utf-8")

tractor = source("src/eve-server/ship/modules/ActiveModule.cpp")
assert "pilotAlliance != 0 && m_targetSE->GetAllianceID() == pilotAlliance" in tractor
assert "pilotFaction != 0 && m_targetSE->GetWarFactionID() == pilotFaction" in tractor
assert "pilotFleet != 0 && pilotFleet == m_targetSE->GetFleetID()" in tractor
assert "if (startTractor)" in tractor
assert tractor.index("if (startTractor)") > tractor.index("if (distance > range)")
assert "m_targetSE->DestinyMgr() != nullptr" in tractor

target = source("src/eve-server/system/TargetManager.cpp")
assert "if (sRef.get() == nullptr || tSE == nullptr)" in target
assert "if (scanRes == 0)" in target
assert "MiningLaser* miningModule = module->GetMiningModule();" in target
assert "if (miningModule != nullptr)" in target
assert "targeter == entry.first && targeter != nullptr" in target

agent = source("src/eve-server/agents/AgentBound.cpp")
start = agent.index("PyResult AgentBound::GetMissionObjectiveInfo(PyCallArgs &call,")
end = agent.index("PyResult AgentBound::GetMyJournalDetails", start)
obj = agent[start:end]
assert "if (!m_agent->HasMission(callerID, offer))" in obj
assert "characterID.value()->value() != callerID" in obj

maildb = source("src/eve-server/mail/MailDB.cpp")
mail = source("src/eve-server/mail/MailMgrService.cpp")
assert "bool MailDB::IsMailRecipient(int32 messageID, uint32 characterID) const" in maildb
assert "m_db.IsMailRecipient(messageID->value(), call.client->GetCharacterID())" in mail
assert "DELETE FROM mailStatus WHERE messageID = %u AND characterID = %u" in maildb
assert "UPDATE mailStatus SET statusMask" in maildb
assert 'UPDATE mailMessage SET statusMask' not in maildb
assert "AND messageID IN (" in maildb and "WHERE characterID = " in maildb
assert "if (bit < 0 || bit > 30)" in maildb
assert "return 0; // never create mailStatus rows with an invalid ID" in maildb

contracts = source("src/eve-server/contract/ContractProxy.cpp")
assert "contractID->value()" in contracts
assert "flagArg == call.byname.end()" in contracts
assert "requestedArg == call.byname.end()" in contracts
assert "sDatabase.DoEscapeString(titleEscaped, title->content());" in contracts
assert "if (plasticWrap.get() == nullptr)" in contracts

ram = source("src/eve-server/manufacturing/RamProxyService.cpp")
activity = source("src/eve-server/manufacturing/RamMethods.cpp")
assert "if (quantity == 0)\n            continue;" in ram
assert "if (!FactoryDB::CompleteJob(args.jobID," in ram
assert "if (installedItem.get() == nullptr)" in ram
assert "if (!args.cancel && data.activity == EvERam::Activity::Invention)" in ram
assert "Do not consume datacores, runs or ISK" in activity
assert 'throw UserError("RamActivityInvalid");' in activity

drone = source("src/eve-server/npc/DroneAI.cpp")
assert "if (m_pDrone == nullptr)" in drone
assert "Mining drone extraction is not implemented" in drone, (
    "Do not misrepresent placeholder mining-drone combat as actual extraction"
)
print("Source-level gameplay safety guards passed")
