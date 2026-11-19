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
assert tractor.index("if (startTractor)") > tractor.index("if (!std::isfinite(range) || range <= 0.0f || distance > range)")
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
assert "SELECT issuerID, status FROM ctrContracts" in contracts
assert "ownerRow.GetUInt(0) != call.client->GetCharacterID()" in contracts
assert "ownerRow.GetInt(1) != 0" in contracts
assert "const bool isPrivate = row.GetBool(11);" in contracts
assert "const uint32 assigneeID = row.GetUInt(12);" in contracts
assert "const uint32 acceptorID = row.GetUInt(11);" in contracts
assert "contractType != 3 || status != 1 || acceptorID != call.client->GetCharacterID()" in contracts

ram = source("src/eve-server/manufacturing/RamProxyService.cpp")
activity = source("src/eve-server/manufacturing/RamMethods.cpp")
assert "if (quantity == 0)\n            continue;" in ram
assert "if (!FactoryDB::CompleteJob(args.jobID," in ram
assert "if (installedItem.get() == nullptr)" in ram
assert "if (!args.cancel && data.activity == EvERam::Activity::Invention)" in ram
# Verify the invention path rejects the job BEFORE payment or installation.
# Test the actual exception, not an implementation comment whose wording can change.
activity_check = activity.split("void RamMethods::ActivityCheck(", 1)[1].split(
    "void RamMethods::JobsCheck(", 1)[0]
invention_case = activity_check.split("case EvERam::Activity::Invention:", 1)[1].split(
    "case EvERam::Activity::ReverseEngineering:", 1)[0]
assert 'throw CustomError("Blueprint invention is not functional on this server yet;' in invention_case
assert 'FactoryDB::InstallJob(' not in invention_case
assert 'AccountService::TransferFunds(' not in invention_case
install = ram.split("PyResult RamProxyService::InstallJob(", 1)[1].split(
    "PyResult RamProxyService::", 1)[0]
assert install.index("sRamMthd.ActivityCheck(call.client, args, bpRef);") < install.index(
    "AccountService::TransferFunds(")
assert install.index("sRamMthd.ActivityCheck(call.client, args, bpRef);") < install.index(
    "FactoryDB::InstallJob(")

drone = source("src/eve-server/npc/DroneAI.cpp")
assert "if (m_pDrone == nullptr)" in drone
assert "IsMiningDrone(m_pDrone) || (pTarget != nullptr && pTarget->IsAsteroidSE())" in drone
assert "asteroid->SetAttribute(AttrQuantity, left);" in drone
assert "ore->MergeTypesInCargo(shipItem.get(), hold);" in drone
mining = drone.split("void DroneAIMgr::ProcessMining()", 1)[1].split("void DroneAIMgr::StopMiningBeam()", 1)[0]
assert mining.index("EvEMining::Orbit(") < mining.index("m_miningTimer.Start(duration);")
assert "if (!flight.inOrbit || MiningSeparation(m_pDrone, target) > range)" in mining
assert "EvEMining::Approach(" in mining and "movement->ApplyMiningDroneStep(" in mining
assert "if (!inRange || (m_minedUnits > 0 && !DeliverMiningOre(ship)))" in mining
assert "movement->Follow(" not in mining
assert "!m_AI->HasMiningFlight()" in source("src/eve-server/npc/Drone.cpp")
drone_snapshot = source("src/eve-server/npc/Drone.cpp").split("void DroneSE::EncodeDestiny", 1)[1]
assert "follow.followRange = m_destiny->GetFollowRange();" in drone_snapshot
assert "data.speedfraction = m_destiny->GetCommandedSpeedFraction();" in drone_snapshot
entity = source("src/eve-server/npc/EntityService.cpp")
assert "pTarget != nullptr && pTarget->IsAsteroidSE()" in entity
assert "return OrderMiningDrones(call, droneIDs, targetID, false);" in entity
assert "return OrderMiningDrones(call, droneIDs, targetID, true);" in entity
assert "ship->TargetMgr()->GetTarget(asteroidID, true) != asteroid" in entity
assert "drone->GetControllerID() == ship->GetID() && drone->GetHomeShip() == ship" in entity
print("Source-level gameplay safety guards passed")
