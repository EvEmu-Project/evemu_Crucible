# Upstream PR #335 — exact changed-file inventory (19 November 2026 snapshot)

**Source:** [upstream draft PR #335](https://github.com/EvEmu-Project/evemu_Crucible/pull/335) at contribution head `9f8929de3387c9ae471e768556eefc8657c62f5f`; target upstream `master` commit `f55b488a006e3090470d3b481aee47a1d7a0dade`.
**Snapshot:** 303 changed paths; 22,836 added and 3,701 removed textual lines; 811 cumulative commits reported by GitHub at the snapshot.
**Important:** This list was generated **before** committing the 19 November handover/patch-note documents, so the final live PR file count may increase. GitHub's [live Files changed view](https://github.com/EvEmu-Project/evemu_Crucible/pull/335/files) is authoritative for subsequent changes.

The file scope is cumulative against upstream's older `master`, not a claim that all paths were touched during the most recent integration. Each group lists every path exactly once. `added/removed` are GitHub diff counts, not binary byte sizes. The full developer explanation is [here](UPSTREAM_DEVELOPER_HANDOVER_2026-11-19.md); the [cumulative patch notes](UPSTREAM_PATCH_NOTES_2026-11-19.md) cover integration and testing.

## Build, packaging and root (8)

| File | Status | + | − |
| --- | --- | ---: | ---: |
| `.dockerignore` | modified | 25 | 9 |
| `.gitignore` | modified | 7 | 1 |
| `CMakeLists.txt` | modified | 29 | 9 |
| `Dockerfile` | modified | 22 | 11 |
| `ISSUE_TRIAGE.md` | added | 380 | 0 |
| `PATCH_NOTES.md` | added | 680 | 0 |
| `README.md` | modified | 386 | 67 |
| `STABILITY_WORKLOG.md` | added | 217 | 0 |

## GitHub workflows and CMake (3)

| File | Status | + | − |
| --- | --- | ---: | ---: |
| `.github/workflows/build-release.yml` | added | 534 | 0 |
| `.github/workflows/cmake.yml` | removed | 0 | 33 |
| `cmake/FindUTF8CPP.cmake` | modified | 1 | 0 |

## Documentation (47)

| File | Status | + | − |
| --- | --- | ---: | ---: |
| `docs/A01_STAGING_HARNESS.md` | added | 21 | 0 |
| `docs/AGENT_LOCAL_PORTRAITS_2026-11-15.md` | added | 76 | 0 |
| `docs/AGENT_PORTRAIT_FULL_REMOVAL_2026-11-16.md` | added | 41 | 0 |
| `docs/AGENT_STATION_IMAGE_AUDIT_2026-11-15.md` | added | 44 | 0 |
| `docs/AGENT_STATION_PORTRAIT_ROLLBACK_2026-11-16.md` | added | 33 | 0 |
| `docs/assets/readme/database-tools-banner.jpg` | added | 0 | 0 |
| `docs/assets/readme/hero-banner.jpg` | added | 0 | 0 |
| `docs/assets/readme/windows-setup-banner.jpg` | added | 0 | 0 |
| `docs/BLOCKADE_REWARD_AND_CONTAINER_MARKET_SEED_2026-11-15.md` | added | 48 | 0 |
| `docs/CONTAINER_ASSEMBLY_AND_AGENT_FALLBACK_FIX_2026-11-16.md` | added | 65 | 0 |
| `docs/CRUCIBLE_1_6_STATIC_ATTRIBUTE_AUDIT.md` | added | 17 | 0 |
| `docs/CRUCIBLE_CLIENT_GAMEPLAY_VERIFICATION_CHECKLIST_2026-11-16.md` | added | 170 | 0 |
| `docs/CRUCIBLE_IMAGE_REDIRECT_CRASH_TRIAGE_2026-11-16.md` | added | 47 | 0 |
| `docs/CRUCIBLE_ITEM_SQL_CORP_TAX_FIX_2026-11-16.md` | added | 161 | 0 |
| `docs/CRUCIBLE_LOCAL_AGENT_PORTRAIT_TEST_2026-11-16.md` | added | 67 | 0 |
| `docs/CRUCIBLE_RANGE_AND_WARP_AUDIT_2026-11-17.md` | added | 69 | 0 |
| `docs/CRUCIBLE_SOV_SECURITY_BELTS_RATS_AUDIT_2026-11-16.md` | added | 173 | 0 |
| `docs/CRUCIBLE_STATION_AGENT_50PX_CLIENT_CRASH_TRIAGE_2026-11-16.md` | added | 63 | 0 |
| `docs/CRUCIBLE_STATION_CONTAINER_TRANSFER_CRASH_TRIAGE_2026-11-16.md` | added | 113 | 0 |
| `docs/CRUCIBLE_WEAPONS_AUDIT_2026-11-15.md` | added | 125 | 0 |
| `docs/DRONE_AND_MOTION_AUDIT.md` | added | 76 | 0 |
| `docs/DRONE_RANGE_AND_SUSTAINED_COMBAT_2026-11-15.md` | added | 46 | 0 |
| `docs/EVEMU_CONTAINER_POS_INVENTORY_AUDIT_2026-11-16.md` | added | 122 | 0 |
| `docs/GAMEPLAY_SUBSYSTEMS_AUDIT_TEST_MATRIX_2026-11-16.md` | added | 241 | 0 |
| `docs/INDUSTRY_RESEARCH_COPY_INVENTION_AUDIT_2026-11-18.md` | added | 76 | 0 |
| `docs/MISSION_EXPIRY_AND_OFFLINE_STACK_AUDIT_2026-11-15.md` | added | 32 | 0 |
| `docs/missions/BLOCKADE_TEST_MODE.md` | added | 15 | 0 |
| `docs/missions/DED_OFFICER_EXPEDITION_TEST_MODE.md` | added | 112 | 0 |
| `docs/missions/MIGRATION_GUIDE.md` | added | 68 | 0 |
| `docs/missions/STAGING_MIGRATION_GUIDE.md` | added | 77 | 0 |
| `docs/missions/STAGING_VALIDATION.sql` | added | 71 | 0 |
| `docs/NPC_BUY_MARKET_100_PERCENT_2026-11-17.md` | added | 71 | 0 |
| `docs/NPC_MARKET_100_PERCENT_REFERENCE_PRICE_2026-11-16.md` | added | 93 | 0 |
| `docs/NPC_ORE_MINERAL_100M_ORDERS_2026-11-18.md` | added | 53 | 0 |
| `docs/PR25_INGAME_VALIDATION_CHECKLIST.md` | added | 153 | 0 |
| `docs/REPAIR_CHECKLIST.md` | added | 27 | 0 |
| `docs/SERVER_LOG_TRIAGE_2026-11-15.md` | added | 86 | 0 |
| `docs/SOV_INDEX_LEVEL5_GM_TEST_2026-11-18.md` | added | 141 | 0 |
| `docs/SOVEREIGNTY_POS_REVIEW_2026-11-17.md` | added | 55 | 0 |
| `docs/STATION_LOGIN_OFFICE_CRASH_AUDIT_2026-11-16.md` | added | 94 | 0 |
| `docs/UPSTREAM_ISSUE_AUDIT_2026-11-17.md` | added | 196 | 0 |
| `docs/UPSTREAM_PATCH_NOTES_HISTORY_2026-11-15.md` | added | 33 | 0 |
| `docs/UPSTREAM_PR_335_DESCRIPTION_2026-11-18.md` | added | 37 | 0 |
| `docs/UPSTREAM_PULL_REQUEST_REVIEW_2026-11-15.md` | added | 294 | 0 |
| `docs/UPSTREAM_PULL_REQUEST_REVIEW_2026-11-17.md` | added | 240 | 0 |
| `docs/UPSTREAM_PULL_REQUEST_REVIEW_2026-11-18.md` | added | 148 | 0 |
| `docs/WARP_GATE_LANDING_2500M_2026-11-18.md` | added | 70 | 0 |

## SQL and migrations (25)

| File | Status | + | − |
| --- | --- | ---: | ---: |
| `sql/diagnostics/2026-11-16-container-pos-integrity.sql` | added | 83 | 0 |
| `sql/diagnostics/2026-11-16-normalize-npc-sell-orders-100-percent.sql` | added | 85 | 0 |
| `sql/diagnostics/2026-11-16-sov-security-belt-rat-integrity.sql` | added | 193 | 0 |
| `sql/diagnostics/2026-11-17-crucible-ship-weapon-range-warp-audit.sql` | added | 106 | 0 |
| `sql/diagnostics/2026-11-17-crucible-warp-speed-remaining-14-repair.sql` | added | 72 | 0 |
| `sql/diagnostics/2026-11-17-crucible-warp-speed-verified-repair.sql` | added | 173 | 0 |
| `sql/diagnostics/2026-11-17-npc-buy-orders-100pct-audit.sql` | added | 106 | 0 |
| `sql/diagnostics/2026-11-17-sov-pos-readonly-audit.sql` | added | 92 | 0 |
| `sql/evedb.yaml.example` | added | 14 | 0 |
| `sql/get_evedbtool.bat` | modified | 48 | 18 |
| `sql/get_evedbtool.sh` | modified | 31 | 29 |
| `sql/migrations/20261118070000-sov-development-index-overrides.sql` | added | 14 | 0 |
| `sql/migrations/20261118093000-sov-development-index-point-width.sql` | added | 18 | 0 |
| `sql/migrations/20261118120000-pos-industry-lines.sql` | added | 29 | 0 |
| `sql/migrations/20261118123000-crucible-static-attribute-corrections.sql` | added | 517 | 0 |
| `sql/migrations/20261118130000-crucible-capital-warp-speeds.sql` | added | 40 | 0 |
| `sql/mission_data/001_create_staging_schema.sql` | added | 110 | 0 |
| `sql/mission_data/002_seed_research_case.sql` | added | 55 | 0 |
| `sql/mission_data/003_upgrade_existing_staging_schema.sql` | added | 756 | 0 |
| `sql/mission_data/research/the_blockade_level1_angel_cartel.json` | added | 189 | 0 |
| `sql/mission_data/source_manifest.json` | added | 70 | 0 |
| `sql/README.md` | modified | 97 | 37 |
| `sql/seed_and_clean/seed_market.sql` | modified | 56 | 6 |
| `sql/seed_and_clean/seed_special_modules.sql` | added | 85 | 0 |
| `sql/seed_and_clean/seed_station_market.sql` | modified | 14 | 3 |

## Shared/common types, protocol and math (31)

| File | Status | + | − |
| --- | --- | ---: | ---: |
| `src/eve-common/database/EVEDBUtils.cpp` | modified | 4 | 0 |
| `src/eve-common/destiny/DestinyBinDump.cpp` | modified | 2 | 2 |
| `src/eve-common/destiny/DestinyStructs.h` | modified | 9 | 7 |
| `src/eve-common/EVE_AgentHelpers.h` | added | 73 | 0 |
| `src/eve-common/EVE_AsteroidSpawnHelpers.h` | added | 51 | 0 |
| `src/eve-common/EVE_CommandPermissions.h` | added | 18 | 0 |
| `src/eve-common/EVE_ContainerHelpers.h` | added | 35 | 0 |
| `src/eve-common/EVE_DroneControlRange.h` | added | 27 | 0 |
| `src/eve-common/EVE_IHubUpgradeRules.h` | added | 73 | 0 |
| `src/eve-common/EVE_ImageHelpers.h` | added | 19 | 0 |
| `src/eve-common/EVE_IndustryFormulas.h` | added | 39 | 0 |
| `src/eve-common/EVE_LegacyAgentThumbnail.h` | added | 18 | 0 |
| `src/eve-common/EVE_Market.h` | modified | 4 | 4 |
| `src/eve-common/EVE_MiningFlight.h` | added | 63 | 0 |
| `src/eve-common/EVE_MiningFormulas.h` | added | 32 | 0 |
| `src/eve-common/EVE_Missions.h` | modified | 70 | 0 |
| `src/eve-common/EVE_MovementFormulas.h` | added | 159 | 0 |
| `src/eve-common/EVE_RAM.h` | modified | 2 | 0 |
| `src/eve-common/EVE_ScanningMath.h` | added | 43 | 0 |
| `src/eve-common/EVE_SovIndexOverride.h` | added | 46 | 0 |
| `src/eve-common/EVE_SovPosIntegrity.h` | added | 41 | 0 |
| `src/eve-common/EVE_StationOfficeHelpers.h` | added | 27 | 0 |
| `src/eve-common/EVE_TowerShieldRules.h` | added | 31 | 0 |
| `src/eve-common/EVE_WeaponFormulas.h` | added | 142 | 0 |
| `src/eve-common/EVEVersion.h` | modified | 8 | 2 |
| `src/eve-common/network/EVETCPConnection.cpp` | modified | 13 | 7 |
| `src/eve-common/python/classes/PyExceptions.cpp` | modified | 7 | 6 |
| `src/eve-common/python/PyPacket.cpp` | modified | 3 | 3 |
| `src/eve-common/python/PyVisitor.cpp` | modified | 4 | 3 |
| `src/eve-common/utils/EvEMath.cpp` | modified | 23 | 8 |
| `src/eve-common/utils/EvEMath.h` | modified | 1 | 1 |

## Core runtime and database (9)

| File | Status | + | − |
| --- | --- | ---: | ---: |
| `src/eve-core/database/dbcore.cpp` | modified | 11 | 6 |
| `src/eve-core/database/dbcore.h` | modified | 5 | 1 |
| `src/eve-core/network/TCPConnection.cpp` | modified | 6 | 1 |
| `src/eve-core/threading/Threading.cpp` | modified | 11 | 6 |
| `src/eve-core/utils/Buffer.h` | modified | 20 | 27 |
| `src/eve-core/utils/CallStack.cpp` | modified | 2 | 104 |
| `src/eve-core/utils/misc.cpp` | modified | 6 | 2 |
| `src/eve-core/utils/utils_string.cpp` | modified | 1 | 1 |
| `src/eve-core/utils/utils_time.cpp` | modified | 1 | 0 |

## Agent missions and AI (7)

| File | Status | + | − |
| --- | --- | ---: | ---: |
| `src/eve-server/agents/Agent.cpp` | modified | 170 | 82 |
| `src/eve-server/agents/Agent.h` | modified | 1 | 1 |
| `src/eve-server/agents/AgentBound.cpp` | modified | 421 | 62 |
| `src/eve-server/agents/AgentBound.h` | modified | 2 | 2 |
| `src/eve-server/agents/AgentDB.cpp` | modified | 21 | 15 |
| `src/eve-server/agents/AgentDB.h` | modified | 1 | 1 |
| `src/eve-server/agents/AgentMgrService.cpp` | modified | 4 | 3 |

## Market and finance (3)

| File | Status | + | − |
| --- | --- | ---: | ---: |
| `src/eve-server/market/MarketBotMgr.cpp` | modified | 52 | 17 |
| `src/eve-server/market/MarketDB.cpp` | modified | 37 | 16 |
| `src/eve-server/market/MarketMgr.cpp` | modified | 60 | 101 |

## NPC and drones (11)

| File | Status | + | − |
| --- | --- | ---: | ---: |
| `src/eve-server/npc/Drone.cpp` | modified | 242 | 180 |
| `src/eve-server/npc/Drone.h` | modified | 9 | 7 |
| `src/eve-server/npc/DroneAI.cpp` | modified | 782 | 67 |
| `src/eve-server/npc/DroneAI.h` | modified | 22 | 2 |
| `src/eve-server/npc/EntityService.cpp` | modified | 262 | 65 |
| `src/eve-server/npc/EntityService.h` | modified | 4 | 3 |
| `src/eve-server/npc/NPC.cpp` | modified | 54 | 18 |
| `src/eve-server/npc/NPCAI.cpp` | modified | 51 | 10 |
| `src/eve-server/npc/NPCAI.h` | modified | 2 | 2 |
| `src/eve-server/npc/SentryAI.cpp` | modified | 18 | 2 |
| `src/eve-server/npc/SentryAI.h` | modified | 1 | 1 |

## Ships, weapons and Destiny (15)

| File | Status | + | − |
| --- | --- | ---: | ---: |
| `src/eve-server/ship/BeyonceService.cpp` | modified | 91 | 54 |
| `src/eve-server/ship/Missile.cpp` | modified | 185 | 53 |
| `src/eve-server/ship/Missile.h` | modified | 8 | 1 |
| `src/eve-server/ship/modules/ActiveModule.cpp` | modified | 263 | 66 |
| `src/eve-server/ship/modules/ActiveModule.h` | modified | 5 | 2 |
| `src/eve-server/ship/modules/GenericModule.cpp` | modified | 6 | 0 |
| `src/eve-server/ship/modules/MiningLaser.cpp` | modified | 129 | 144 |
| `src/eve-server/ship/modules/MiningLaser.h` | modified | 1 | 1 |
| `src/eve-server/ship/modules/ModuleManager.cpp` | modified | 32 | 14 |
| `src/eve-server/ship/modules/TurretFormulas.cpp` | modified | 141 | 119 |
| `src/eve-server/ship/modules/TurretModule.cpp` | modified | 36 | 13 |
| `src/eve-server/ship/Ship.cpp` | modified | 77 | 50 |
| `src/eve-server/ship/ShipService.cpp` | modified | 106 | 62 |
| `src/eve-server/system/DestinyManager.cpp` | modified | 791 | 501 |
| `src/eve-server/system/DestinyManager.h` | modified | 24 | 9 |

## Sovereignty, POS and structures (17)

| File | Status | + | − |
| --- | --- | ---: | ---: |
| `src/eve-server/pos/JumpBridge.cpp` | modified | 14 | 4 |
| `src/eve-server/pos/JumpBridge.h` | modified | 1 | 1 |
| `src/eve-server/pos/Module.cpp` | modified | 8 | 2 |
| `src/eve-server/pos/Module.h` | modified | 1 | 1 |
| `src/eve-server/pos/PosMgr.cpp` | modified | 184 | 52 |
| `src/eve-server/pos/PosMgrDB.cpp` | modified | 14 | 8 |
| `src/eve-server/pos/sovStructures/IHub.cpp` | modified | 15 | 5 |
| `src/eve-server/pos/sovStructures/SBU.cpp` | modified | 40 | 21 |
| `src/eve-server/pos/sovStructures/TCU.cpp` | modified | 31 | 3 |
| `src/eve-server/pos/Structure.cpp` | modified | 122 | 34 |
| `src/eve-server/pos/Structure.h` | modified | 5 | 1 |
| `src/eve-server/pos/Tower.cpp` | modified | 53 | 47 |
| `src/eve-server/pos/Tower.h` | modified | 1 | 1 |
| `src/eve-server/system/sov/SovereigntyDataMgr.cpp` | modified | 172 | 39 |
| `src/eve-server/system/sov/SovereigntyDataMgr.h` | modified | 11 | 0 |
| `src/eve-server/system/sov/SovereigntyDB.cpp` | modified | 73 | 8 |
| `src/eve-server/system/sov/SovereigntyDB.h` | modified | 8 | 3 |

## Cosmic, scanning and bubbles (24)

| File | Status | + | − |
| --- | --- | ---: | ---: |
| `src/eve-server/system/Asteroid.cpp` | modified | 5 | 2 |
| `src/eve-server/system/Asteroid.h` | modified | 2 | 2 |
| `src/eve-server/system/BubbleManager.cpp` | modified | 30 | 38 |
| `src/eve-server/system/Container.cpp` | modified | 37 | 11 |
| `src/eve-server/system/Container.h` | modified | 3 | 1 |
| `src/eve-server/system/cosmicMgrs/AnomalyMgr.cpp` | modified | 57 | 2 |
| `src/eve-server/system/cosmicMgrs/BeltMgr.cpp` | modified | 71 | 36 |
| `src/eve-server/system/cosmicMgrs/BeltMgr.h` | modified | 1 | 1 |
| `src/eve-server/system/cosmicMgrs/DungeonMgr.cpp` | modified | 207 | 10 |
| `src/eve-server/system/cosmicMgrs/DungeonMgr.h` | modified | 2 | 0 |
| `src/eve-server/system/cosmicMgrs/ManagerDB.cpp` | modified | 10 | 4 |
| `src/eve-server/system/cosmicMgrs/SpawnMgr.cpp` | modified | 798 | 34 |
| `src/eve-server/system/cosmicMgrs/SpawnMgr.h` | modified | 7 | 3 |
| `src/eve-server/system/Damage.cpp` | modified | 35 | 15 |
| `src/eve-server/system/Damage.h` | modified | 6 | 4 |
| `src/eve-server/system/IndexManager.cpp` | modified | 10 | 6 |
| `src/eve-server/system/SystemBubble.cpp` | modified | 60 | 25 |
| `src/eve-server/system/SystemBubble.h` | modified | 49 | 0 |
| `src/eve-server/system/SystemEntity.cpp` | modified | 13 | 6 |
| `src/eve-server/system/SystemEntity.h` | modified | 1 | 0 |
| `src/eve-server/system/SystemManager.cpp` | modified | 70 | 43 |
| `src/eve-server/system/SystemManager.h` | modified | 8 | 2 |
| `src/eve-server/system/TargetManager.cpp` | modified | 356 | 85 |
| `src/eve-server/system/TargetManager.h` | modified | 8 | 4 |

## Inventory, industry, client and other server (68)

| File | Status | + | − |
| --- | --- | ---: | ---: |
| `src/eve-server/admin/AllCommands.cpp` | modified | 2 | 0 |
| `src/eve-server/admin/AllCommands.h` | modified | 2 | 0 |
| `src/eve-server/admin/AllCommandsList.h` | modified | 1 | 0 |
| `src/eve-server/admin/CommandDispatcher.cpp` | modified | 27 | 7 |
| `src/eve-server/admin/CommandDispatcher.h` | modified | 7 | 3 |
| `src/eve-server/admin/debug_commands.cpp` | modified | 9 | 7 |
| `src/eve-server/admin/DevToolsProviderService.cpp` | modified | 22 | 11 |
| `src/eve-server/admin/DungeonCommands.cpp` | modified | 1 | 1 |
| `src/eve-server/admin/GMCommands.cpp` | modified | 20 | 6 |
| `src/eve-server/admin/GMCommands.h` | modified | 4 | 3 |
| `src/eve-server/admin/SystemCommands.cpp` | modified | 45 | 1 |
| `src/eve-server/admin/SystemCommands.h` | modified | 2 | 0 |
| `src/eve-server/character/Character.cpp` | modified | 16 | 4 |
| `src/eve-server/character/Character.h` | modified | 3 | 0 |
| `src/eve-server/character/CharacterDB.cpp` | modified | 1 | 1 |
| `src/eve-server/chat/LSCService.cpp` | modified | 3 | 3 |
| `src/eve-server/Client.cpp` | modified | 429 | 71 |
| `src/eve-server/Client.h` | modified | 11 | 2 |
| `src/eve-server/CMakeLists.txt` | modified | 1 | 1 |
| `src/eve-server/ConsoleCommands.cpp` | modified | 16 | 4 |
| `src/eve-server/contract/ContractProxy.cpp` | modified | 95 | 17 |
| `src/eve-server/corporation/CorporationDB.cpp` | modified | 12 | 6 |
| `src/eve-server/corporation/CorporationDB.h` | modified | 1 | 1 |
| `src/eve-server/corporation/CorpRegistryBound.cpp` | modified | 57 | 12 |
| `src/eve-server/corporation/CorpRegistryBound.h` | modified | 1 | 1 |
| `src/eve-server/dungeon/DungeonDB.cpp` | modified | 32 | 12 |
| `src/eve-server/dungeon/DungeonDB.h` | modified | 1 | 0 |
| `src/eve-server/dungeon/DungeonService.cpp` | modified | 2 | 1 |
| `src/eve-server/dungeon/DungeonService.h` | modified | 1 | 1 |
| `src/eve-server/effects/EffectsProcessor.cpp` | modified | 32 | 13 |
| `src/eve-server/EntityList.cpp` | modified | 32 | 10 |
| `src/eve-server/EVEServerConfig.cpp` | modified | 5 | 5 |
| `src/eve-server/exploration/Scan.cpp` | modified | 85 | 39 |
| `src/eve-server/exploration/Scan.h` | modified | 3 | 0 |
| `src/eve-server/exploration/ScanMgrService.cpp` | modified | 11 | 0 |
| `src/eve-server/imageserver/ImageServer.cpp` | modified | 98 | 63 |
| `src/eve-server/imageserver/ImageServerConnection.cpp` | modified | 57 | 12 |
| `src/eve-server/imageserver/ImageServerConnection.h` | modified | 6 | 4 |
| `src/eve-server/inventory/InvBrokerService.cpp` | modified | 106 | 35 |
| `src/eve-server/inventory/Inventory.cpp` | modified | 148 | 48 |
| `src/eve-server/inventory/Inventory.h` | modified | 2 | 0 |
| `src/eve-server/inventory/InventoryBound.cpp` | modified | 169 | 31 |
| `src/eve-server/inventory/InventoryBound.h` | modified | 4 | 2 |
| `src/eve-server/inventory/InventoryItem.cpp` | modified | 60 | 25 |
| `src/eve-server/inventory/ItemDB.cpp` | modified | 16 | 7 |
| `src/eve-server/inventory/ItemFactory.cpp` | modified | 11 | 37 |
| `src/eve-server/mail/MailDB.cpp` | modified | 162 | 238 |
| `src/eve-server/mail/MailDB.h` | modified | 10 | 9 |
| `src/eve-server/mail/MailMgrService.cpp` | modified | 11 | 7 |
| `src/eve-server/manufacturing/Blueprint.h` | modified | 3 | 1 |
| `src/eve-server/manufacturing/FactoryDB.cpp` | modified | 74 | 10 |
| `src/eve-server/manufacturing/FactoryDB.h` | modified | 1 | 1 |
| `src/eve-server/manufacturing/RamMethods.cpp` | modified | 89 | 53 |
| `src/eve-server/manufacturing/RamProxyService.cpp` | modified | 55 | 23 |
| `src/eve-server/missions/MissionDataMgr.cpp` | modified | 252 | 48 |
| `src/eve-server/missions/MissionDataMgr.h` | modified | 3 | 1 |
| `src/eve-server/missions/MissionDB.cpp` | modified | 90 | 20 |
| `src/eve-server/missions/MissionDB.h` | modified | 3 | 0 |
| `src/eve-server/planet/Colony.cpp` | modified | 2 | 2 |
| `src/eve-server/Profiler.cpp` | modified | 8 | 8 |
| `src/eve-server/search/SearchDB.cpp` | modified | 10 | 10 |
| `src/eve-server/ServiceDB.cpp` | modified | 13 | 2 |
| `src/eve-server/services/BoundService.h` | modified | 1 | 1 |
| `src/eve-server/StaticDataMgr.cpp` | modified | 61 | 1 |
| `src/eve-server/StaticDataMgr.h` | modified | 5 | 0 |
| `src/eve-server/station/Station.cpp` | modified | 20 | 7 |
| `src/eve-server/station/Station.h` | modified | 6 | 1 |
| `src/eve-server/station/StationDB.cpp` | modified | 4 | 1 |

## Unit and regression tests (19)

| File | Status | + | − |
| --- | --- | ---: | ---: |
| `src/eve-test/CMakeLists.txt` | modified | 59 | 4 |
| `src/eve-test/ProfilerTestStub.cpp` | added | 10 | 0 |
| `src/eve-test/utils/AgentHelpersTest.cpp` | added | 38 | 0 |
| `src/eve-test/utils/AsteroidSpawnTest.cpp` | added | 56 | 0 |
| `src/eve-test/utils/CommandPermissionsTest.cpp` | added | 35 | 0 |
| `src/eve-test/utils/ContainerAssemblyTest.cpp` | added | 49 | 0 |
| `src/eve-test/utils/DroneControlRangeTest.cpp` | added | 33 | 0 |
| `src/eve-test/utils/IHubUpgradeRulesTest.cpp` | added | 46 | 0 |
| `src/eve-test/utils/ImageCompatibilityTest.cpp` | added | 46 | 0 |
| `src/eve-test/utils/IndustryFormulaTest.cpp` | added | 56 | 0 |
| `src/eve-test/utils/MiningFormulaTest.cpp` | added | 78 | 0 |
| `src/eve-test/utils/MissionLifecycleTest.cpp` | added | 83 | 0 |
| `src/eve-test/utils/MovementFormulaTest.cpp` | added | 189 | 0 |
| `src/eve-test/utils/ScanDirectionTest.cpp` | added | 36 | 0 |
| `src/eve-test/utils/SovereigntyPosIntegrityTest.cpp` | added | 34 | 0 |
| `src/eve-test/utils/SovIndexOverrideTest.cpp` | added | 47 | 0 |
| `src/eve-test/utils/StationOfficeIntegrityTest.cpp` | added | 39 | 0 |
| `src/eve-test/utils/TowerShieldRulesTest.cpp` | added | 27 | 0 |
| `src/eve-test/utils/WeaponFormulaTest.cpp` | added | 84 | 0 |

## Developer tools and fixtures (16)

| File | Status | + | − |
| --- | --- | ---: | ---: |
| `tools/evedbtool/command_seed.go` | added | 591 | 0 |
| `tools/prepare-crucible-agent-portrait.ps1` | added | 65 | 0 |
| `tools/tests/npc_buy_seed_unsigned_schema.sql` | added | 95 | 0 |
| `tools/tests/verify_pos_industry_lines.py` | added | 76 | 0 |
| `tools/verify_agent_portraits_removed.py` | added | 33 | 0 |
| `tools/verify_blockade_12_ship_fleet.py` | added | 31 | 0 |
| `tools/verify_crucible_range_and_warp.py` | added | 201 | 0 |
| `tools/verify_elite_security_mission.py` | added | 176 | 0 |
| `tools/verify_industry_workflows.py` | added | 74 | 0 |
| `tools/verify_npc_buy_seed.py` | added | 82 | 0 |
| `tools/verify_npc_market_baseprice.py` | added | 43 | 0 |
| `tools/verify_ore_mineral_100m.py` | added | 53 | 0 |
| `tools/verify_sov_pos_integrity.py` | added | 169 | 0 |
| `tools/verify_station_crash_safety.py` | added | 187 | 0 |
| `tools/verify_subsystems_audit.py` | added | 104 | 0 |
| `utils/config/eve-server.xml` | modified | 5 | 22 |

## Contributor-only market delta preserved during sync

The contribution branch previously diverged from fork `master`. The imported 19 November merge intentionally preserves its additional 100M mineral/ore/ice market seed and Trader Joe code, MariaDB integration tests and legacy upstream review snapshots. Only `.github/workflows/build-release.yml` and `PATCH_NOTES.md` were edited in both parent lines; these were merged to keep both sets of checks and notes. All other fork-master source changes were imported unchanged except intentional contribution-market overlays.

**Verification limitations:** An inventory and successful parent-master CI do not prove in-client gameplay or combined PR-head CI. Review source, API compatibility, migration/seed policy and exact contribution-head CI separately before upstream merge.
