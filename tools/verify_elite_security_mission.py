#!/usr/bin/env python3
"""Guard the independent DED/faction/officer Security Agent mission.

Static/source checks complement actual Windows/Linux builds and client playtests.
"""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
read = lambda path: (root / path).read_text(encoding="utf-8")

mission = read("src/eve-common/EVE_Missions.h")
data = read("src/eve-server/missions/MissionDataMgr.cpp")
agent = read("src/eve-server/agents/Agent.cpp")
bound = read("src/eve-server/agents/AgentBound.cpp")
spawn = read("src/eve-server/system/cosmicMgrs/SpawnMgr.cpp")
npc = read("src/eve-server/npc/NPC.cpp")
mgrh = read("src/eve-server/StaticDataMgr.h")
mgr = read("src/eve-server/StaticDataMgr.cpp")
bubbleh = read("src/eve-server/system/SystemBubble.h")
bubblec = read("src/eve-server/system/SystemBubble.cpp")
client = read("src/eve-server/Client.cpp")
groups = read("src/eve-common/tables/invGroups.h")
lifecycle = read("src/eve-test/utils/MissionLifecycleTest.cpp")

assert "kBlockadeSecurityTestMissionID = 65535" in mission
assert "kBlockadeSecurityTestRequiredKills = 12" in mission
assert "kEliteSecurityMissionID = 65534" in mission
assert "kEliteSecurityRequiredKills = 8" in mission
assert "kEliteSecurityRewardISK = 50000000" in mission
assert "kEliteSecurityDurationMinutes = 120" in mission
assert "#include <random>" in mission
assert "std::uniform_int_distribution<std::size_t>" in mission
assert "RandomCandidateIndex(std::size_t count)" in mission
assert "IsSyntheticSecurityEncounter(uint16 missionID)" in mission
assert "EliteLootMetaGroupForNPCGroup" in mission

# The original Blockade fleet remains entirely intact.
assert "static const BlockadeTestShipClass kAngelCartelBlockadeTestMix[]" in spawn
assert '{551, "Asteroid Angel Cartel Cruiser", 8}' in spawn
assert '{576, "Asteroid Angel Cartel BattleCruiser", 4}' in spawn
assert "if (missionID == Mission::kBlockadeSecurityTestMissionID)" in spawn

# Separate encounter, same Agent division & levels, not assigned to courier.
assert 'eliteMission.name = "DED & Officer Expedition (TEST)";' in data
assert "eliteMission.missionID = Mission::kEliteSecurityMissionID" in data
assert "m_missions.emplace(testLevel, eliteMission)" in data
assert "m_missionsImp.emplace(testLevel, eliteMission)" in data
assert data.count("Mission::IsSyntheticSecurityEncounter(mission.missionID)") == 2
assert "securityAgent && Mission::IsSyntheticSecurityEncounter" in data
assert "Mission::RandomCandidateIndex(candidates.size())" in data
assert "candidates[MakeRandomInt(" not in data
assert "Mission::RandomCandidateIndex(it->second.size())" in mgr
assert "it->second[MakeRandomInt(" not in mgr
assert "securityAgent = IsSecurityMissionDivision" in agent
assert "NormalizeEliteTestOffer" in agent
assert "offer.courierAmount = Mission::kEliteSecurityRequiredKills;" in agent
assert "offer.rewardISK = Mission::kEliteSecurityRewardISK;" in agent

# Retain all 18 real candidate groups, but select only two from each
# of four encounter categories for eight NPCs per new expedition.
roster = spawn.split("static const EliteSecurityShipSpec kEliteSecurityFleet[] = {", 1)[1].split("};", 1)[0]
entries = re.findall(r'\{\s*(\d+)\s*,\s*(faction\w+)\s*,\s*"([^"]+)"\s*\}', roster)
assert len(entries) == 18, entries
ids = [int(group_id) for group_id, faction, tier in entries]
assert len(set(ids)) == 18
assert len([e for e in entries if e[2] == "Deadspace"]) == 5
assert len([e for e in entries if e[2] == "Commander"]) == 5
assert len([e for e in entries if e[2] == "Officer"]) == 5
assert len([e for e in entries if e[2] == "Overseer"]) == 3
assert len({faction for _, faction, _ in entries}) == 5
assert "kEliteSecurityCategoryOffsets[] = {0, 5, 10, 15, 18}" in spawn
assert "kEliteSecurityShipsPerCategory = 2" in spawn
assert "Mission::RandomCandidateIndex(available.size())" in spawn
assert "available.pop_back();" in spawn
assert "for (const EliteSecurityShipSpec* spec : chosenSpecs)" in spawn
assert "Mission::RandomCandidateIndex(candidates.size())" in spawn
assert all(re.search(r'=\s*' + str(i) + r',', groups) for i in ids)
assert all(re.search(r'case ' + str(i) + r':', mission) for i in ids)
assert "group.catID != EVEDB::invCategories::Entity" in spawn
assert "group.name.find(spec->groupNameToken)" in spawn
assert "selectedFleet.size()" not in spawn.split("if (missionID == Mission::kEliteSecurityMissionID)", 1)[1].split("// Mission 65535", 1)[0] or "static_assert" in spawn
assert "pBubble->SetMissionData(characterID, agentID, offerID, beaconID" in spawn
assert "pBubble->SetMissionTypeID(missionID);" in spawn
assert "static_cast<uint16>(npcCount)" in spawn
assert "entry.enabled = false" in spawn

# Real SDE items for all three premium tiers, no invented item IDs.
assert "GetEliteMissionLootType(uint8 metaGroupID, uint16& typeID)" in mgrh
for tier in (4, 5, 6):
    assert f"item.metaLvl != {tier}" in mgr
assert "groupItr->second.catID != EVEDB::invCategories::Module" in mgr
assert "std::map<uint8, std::vector<uint16>> allTaggedModules" in mgr
assert "std::map<uint8, std::vector<uint16>> marketListedModules" in mgr
assert "allTaggedModules[tier].push_back(entry.first)" in mgr
assert "item.published && item.marketGroupID != 0" in mgr
assert "m_eliteMissionLootTypes[tier] = preferred" in mgr
assert "m_eliteMissionLootTypes[tier] = available" in mgr
assert "if (!item.published || item.marketGroupID == 0" not in mgr
assert "m_eliteMissionLootTypes" in mgr
assert "m_eliteMissionLootTypes.clear()" in mgr
assert "GetEliteMissionLootType(tier, previewLoot)" in spawn
assert "has no valid Module-category loot" in spawn
assert "GetMissionTypeID()" in bubbleh and "m_missionTypeID = 0;" in bubblec
assert "m_bubble->GetMissionTypeID() == Mission::kEliteSecurityMissionID" in npc
assert "EliteLootMetaGroupForNPCGroup(m_self->groupID())" in npc
assert "wreckItemRef->AddItem(droppedItem);" in npc
assert "InventoryItemRef droppedItem = sItemFactory.SpawnItem(rareDrop);" in npc
assert "DropLoot(wreckItemRef, m_self->groupID(), killerID)" in npc
assert "LOOT__WARNING" not in npc  # not defined by this EVEmu logtypes.h

# Literal new client briefing and independent kill/turn-in path.
assert "EliteSecurityMissionBriefing(const MissionOffer& offer)" in bound
assert '"- Defeat DED and deadspace ships (2)' in bound
assert '"- Destroy faction commander ships (2)' in bound
assert '"- Eliminate pirate officers (2)' in bound
assert '"- Destroy DED overseers (2)' in bound
assert "offer.stateID == Mission::State::Accepted" in bound
assert bound.count("EliteSecurityMissionBriefing(offer)") >= 5
assert "Mission::kEliteSecurityDurationMinutes" in bound
assert "Mission::IsSyntheticSecurityEncounter(offer.missionID)" in bound
assert "offer.courierAmount = static_cast<uint16>(pBubble->GetMissionKillsRequired())" in client
assert "return (data.courierAmount > 0 && data.courierTypeID >= data.courierAmount)" in client
assert "Mission::kEliteSecurityMissionID != Mission::kBlockadeSecurityTestMissionID" in lifecycle
assert "selectedBlockade && selectedExpedition" in lifecycle

# Repeated mission cleanup/acceptance must not reuse stale bubble metadata.
bubble_mgr = read("src/eve-server/system/BubbleManager.cpp")
spawn_h = read("src/eve-server/system/cosmicMgrs/SpawnMgr.h")
assert "void ClearMissionData()" in bubbleh
assert "m_missionOfferID = 0;" in bubbleh
assert "m_missionBeaconID = 0;" in bubbleh
assert "bool ReleaseMissionBubble(SystemBubble* pBubble, uint32 offerID)" in spawn_h
assert "bool SpawnMgr::ReleaseMissionBubble(SystemBubble* pBubble, uint32 offerID)" in spawn
assert "pBubble->GetMissionOfferID() != offerID" in spawn
assert "m_spawns.erase(bubbleID);" in spawn
assert "pBubble->ClearMissionData();" in spawn
assert "m_bubbles.erase(std::remove(m_bubbles.begin(), m_bubbles.end(), pBubble)" in spawn
assert "m_bubbleIDMap.erase(idItr);" in bubble_mgr
assert "m_sysBubbleMap.erase(itr);" in bubble_mgr
assert "SystemBubble* beaconBubble = pBeacon->SysBubble();" in client
assert "beaconBubble != pBubble" in client
assert "beaconBubble->CountNPCs() != 0" in client
assert "GetMissionOfferID() == offer.offerID" in client
assert "GetMissionBeaconID() == offer.destinationID" in client
assert "ReleaseMissionBubble(pBubble, offer.offerID);" in client
anom_mgr = read("src/eve-server/system/cosmicMgrs/AnomalyMgr.cpp")
assert "SystemBubble* signalBubble = pSE->SysBubble();" in anom_mgr
assert "if (signalBubble == nullptr)" in anom_mgr
assert "sig.bubbleID = signalBubble->GetID();" in anom_mgr
assert "sig.bubbleID = pSE->SysBubble()->GetID();" not in anom_mgr

print("PASS: independent 8-NPC Security Agent DED/officer mission and scoped rare wreck loot")
