#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME        "eve-server" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )
SET( TARGET_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )
SET( TARGET_UTILS_DIR   "${PROJECT_SOURCE_DIR}/utils" )

#########
# Files #
#########
SET( INCLUDE
     "${TARGET_INCLUDE_DIR}/eve-server.h"
     "${TARGET_INCLUDE_DIR}/Client.h"
     "${TARGET_INCLUDE_DIR}/ClientSession.h"
     "${TARGET_INCLUDE_DIR}/ConsoleCommands.h"
     "${TARGET_INCLUDE_DIR}/DataClasses.h"
     "${TARGET_INCLUDE_DIR}/EntityList.h"
     "${TARGET_INCLUDE_DIR}/EVEServerConfig.h"
     "${TARGET_INCLUDE_DIR}/LiveUpdateDB.h"
     "${TARGET_INCLUDE_DIR}/NetService.h"
     "${TARGET_INCLUDE_DIR}/POD_containers.h"
     "${TARGET_INCLUDE_DIR}/Profiler.h"
     "${TARGET_INCLUDE_DIR}/ServiceDB.h"
     "${TARGET_INCLUDE_DIR}/StaticDataMgr.h"
     "${TARGET_INCLUDE_DIR}/StatisticMgr.h"
     "${TARGET_INCLUDE_DIR}/DBCleaner.h"
     "${TARGET_INCLUDE_DIR}/Allocators.h")
SET( SOURCE
     "${TARGET_SOURCE_DIR}/eve-server.cpp"
     "${TARGET_SOURCE_DIR}/Client.cpp"
     "${TARGET_SOURCE_DIR}/ClientSession.cpp"
     "${TARGET_SOURCE_DIR}/ConsoleCommands.cpp"
     "${TARGET_SOURCE_DIR}/DataClasses.cpp"
     "${TARGET_SOURCE_DIR}/EntityList.cpp"
     "${TARGET_SOURCE_DIR}/EVEServerConfig.cpp"
     "${TARGET_SOURCE_DIR}/LiveUpdateDB.cpp"
     "${TARGET_SOURCE_DIR}/NetService.cpp"
     "${TARGET_SOURCE_DIR}/Profiler.cpp"
     "${TARGET_SOURCE_DIR}/ServiceDB.cpp"
     "${TARGET_SOURCE_DIR}/StaticDataMgr.cpp"
     "${TARGET_SOURCE_DIR}/StatisticMgr.cpp"
     "${TARGET_SOURCE_DIR}/DBCleaner.cpp")

SET( account_INCLUDE
     "${TARGET_INCLUDE_DIR}/account/AccountDB.h"
     "${TARGET_INCLUDE_DIR}/account/AccountService.h"
     "${TARGET_INCLUDE_DIR}/account/AuthService.h"
     "${TARGET_INCLUDE_DIR}/account/BrowserLockdownSvc.h"
     "${TARGET_INCLUDE_DIR}/account/ClientStatMgrService.h"
     "${TARGET_INCLUDE_DIR}/account/InfoGatheringMgr.h"
     "${TARGET_INCLUDE_DIR}/account/TutorialDB.h"
     "${TARGET_INCLUDE_DIR}/account/TutorialService.h"
     "${TARGET_INCLUDE_DIR}/account/UserService.h" )
SET( account_SOURCE
     "${TARGET_SOURCE_DIR}/account/AccountDB.cpp"
     "${TARGET_SOURCE_DIR}/account/AccountService.cpp"
     "${TARGET_SOURCE_DIR}/account/AuthService.cpp"
     "${TARGET_SOURCE_DIR}/account/BrowserLockdownSvc.cpp"
     "${TARGET_SOURCE_DIR}/account/ClientStatMgrService.cpp"
     "${TARGET_SOURCE_DIR}/account/InfoGatheringMgr.cpp"
     "${TARGET_SOURCE_DIR}/account/TutorialDB.cpp"
     "${TARGET_SOURCE_DIR}/account/TutorialService.cpp"
     "${TARGET_SOURCE_DIR}/account/UserService.cpp" )

SET( admin_INCLUDE
     "${TARGET_INCLUDE_DIR}/admin/AlertService.h"
     "${TARGET_INCLUDE_DIR}/admin/debug_commands.h"
     "${TARGET_INCLUDE_DIR}/admin/AllCommands.h"
     "${TARGET_INCLUDE_DIR}/admin/AllCommandsList.h"
     "${TARGET_INCLUDE_DIR}/admin/ClientStatLogger.h"
     "${TARGET_INCLUDE_DIR}/admin/CommandDB.h"
     "${TARGET_INCLUDE_DIR}/admin/CommandDispatcher.h"
     "${TARGET_INCLUDE_DIR}/admin/DevToolsProviderService.h"
     "${TARGET_INCLUDE_DIR}/admin/DungeonCommands.h"
     "${TARGET_INCLUDE_DIR}/admin/GMCommands.h"
     "${TARGET_INCLUDE_DIR}/admin/PetitionerService.h"
     "${TARGET_INCLUDE_DIR}/admin/SlashService.h"
     "${TARGET_INCLUDE_DIR}/admin/SystemCommands.h"
     "${TARGET_INCLUDE_DIR}/admin/CommandHelper.h" )
SET( admin_SOURCE
     "${TARGET_SOURCE_DIR}/admin/AlertService.cpp"
     "${TARGET_SOURCE_DIR}/admin/debug_commands.cpp"
     "${TARGET_SOURCE_DIR}/admin/AllCommands.cpp"
     "${TARGET_SOURCE_DIR}/admin/ClientStatLogger.cpp"
     "${TARGET_SOURCE_DIR}/admin/CommandDB.cpp"
     "${TARGET_SOURCE_DIR}/admin/CommandDispatcher.cpp"
     "${TARGET_SOURCE_DIR}/admin/DevToolsProviderService.cpp"
     "${TARGET_SOURCE_DIR}/admin/DungeonCommands.cpp"
     "${TARGET_SOURCE_DIR}/admin/GMCommands.cpp"
     "${TARGET_SOURCE_DIR}/admin/PetitionerService.cpp"
     "${TARGET_SOURCE_DIR}/admin/SlashService.cpp"
     "${TARGET_SOURCE_DIR}/admin/SystemCommands.cpp"
     "${TARGET_SOURCE_DIR}/admin/CommandHelper.cpp" )

SET( agent_INCLUDE
     "${TARGET_INCLUDE_DIR}/agents/Agent.h"
     "${TARGET_INCLUDE_DIR}/agents/AgentBound.h"
     "${TARGET_INCLUDE_DIR}/agents/AgentDB.h"
     "${TARGET_INCLUDE_DIR}/agents/AgentMgrService.h")
SET( agent_SOURCE
     "${TARGET_SOURCE_DIR}/agents/Agent.cpp"
     "${TARGET_SOURCE_DIR}/agents/AgentBound.cpp"
     "${TARGET_SOURCE_DIR}/agents/AgentDB.cpp"
     "${TARGET_SOURCE_DIR}/agents/AgentMgrService.cpp")

SET( alliance_INCLUDE
     "${TARGET_INCLUDE_DIR}/alliance/AllianceBound.h"
     "${TARGET_INCLUDE_DIR}/alliance/AllianceRegistry.h"
     "${TARGET_INCLUDE_DIR}/alliance/AllianceDB.h")
SET( alliance_SOURCE
     "${TARGET_SOURCE_DIR}/alliance/AllianceBound.cpp"
     "${TARGET_SOURCE_DIR}/alliance/AllianceRegistry.cpp"
     "${TARGET_SOURCE_DIR}/alliance/AllianceDB.cpp")

SET( cache_INCLUDE
     "${TARGET_INCLUDE_DIR}/cache/BulkDB.h"
     "${TARGET_INCLUDE_DIR}/cache/BulkMgrService.h"
     "${TARGET_INCLUDE_DIR}/cache/ObjCacheDB.h"
     "${TARGET_INCLUDE_DIR}/cache/ObjCacheService.h" )
SET( cache_SOURCE
     "${TARGET_SOURCE_DIR}/cache/BulkDB.cpp"
     "${TARGET_SOURCE_DIR}/cache/BulkMgrService.cpp"
     "${TARGET_SOURCE_DIR}/cache/ObjCacheDB.cpp"
     "${TARGET_SOURCE_DIR}/cache/ObjCacheService.cpp" )

SET( character_INCLUDE
     "${TARGET_INCLUDE_DIR}/character/AggressionMgrService.h"
     "${TARGET_INCLUDE_DIR}/character/CertificateMgrDB.h"
     "${TARGET_INCLUDE_DIR}/character/CertificateMgrService.h"
     "${TARGET_INCLUDE_DIR}/character/Character.h"
     "${TARGET_INCLUDE_DIR}/character/CharacterDB.h"
     "${TARGET_INCLUDE_DIR}/character/CharFittingMgr.h"
     "${TARGET_INCLUDE_DIR}/character/CharMgrService.h"
     "${TARGET_INCLUDE_DIR}/character/CharUnboundMgrService.h"
     "${TARGET_INCLUDE_DIR}/character/PaperDollDB.h"
     "${TARGET_INCLUDE_DIR}/character/PaperDollService.h"
     "${TARGET_INCLUDE_DIR}/character/PhotoUploadService.h"
     "${TARGET_INCLUDE_DIR}/character/Skill.h"
     "${TARGET_INCLUDE_DIR}/character/SkillMgrService.h" )
SET( character_SOURCE
     "${TARGET_SOURCE_DIR}/character/AggressionMgrService.cpp"
     "${TARGET_SOURCE_DIR}/character/CertificateMgrDB.cpp"
     "${TARGET_SOURCE_DIR}/character/CertificateMgrService.cpp"
     "${TARGET_SOURCE_DIR}/character/Character.cpp"
     "${TARGET_SOURCE_DIR}/character/CharacterDB.cpp"
     "${TARGET_SOURCE_DIR}/character/CharFittingMgr.cpp"
     "${TARGET_SOURCE_DIR}/character/CharMgrService.cpp"
     "${TARGET_SOURCE_DIR}/character/CharUnboundMgrService.cpp"
     "${TARGET_SOURCE_DIR}/character/PaperDollDB.cpp"
     "${TARGET_SOURCE_DIR}/character/PaperDollService.cpp"
     "${TARGET_SOURCE_DIR}/character/PhotoUploadService.cpp"
     "${TARGET_SOURCE_DIR}/character/Skill.cpp"
     "${TARGET_SOURCE_DIR}/character/SkillMgrService.cpp" )

SET( chat_INCLUDE
     "${TARGET_INCLUDE_DIR}/chat/LookupService.h"
     "${TARGET_INCLUDE_DIR}/chat/LSCDB.h"
     "${TARGET_INCLUDE_DIR}/chat/LSCChannel.h"
     "${TARGET_INCLUDE_DIR}/chat/LSCService.h"
     "${TARGET_INCLUDE_DIR}/chat/OnlineStatusService.h"
     "${TARGET_INCLUDE_DIR}/chat/VoiceMgrService.h" )
SET( chat_SOURCE
     "${TARGET_SOURCE_DIR}/chat/LookupService.cpp"
     "${TARGET_SOURCE_DIR}/chat/LSCDB.cpp"
     "${TARGET_SOURCE_DIR}/chat/LSCChannel.cpp"
     "${TARGET_SOURCE_DIR}/chat/LSCService.cpp"
     "${TARGET_SOURCE_DIR}/chat/OnlineStatusService.cpp"
     "${TARGET_SOURCE_DIR}/chat/VoiceMgrService.cpp" )

SET( config_INCLUDE
     "${TARGET_INCLUDE_DIR}/config/ConfigDB.h"
     "${TARGET_INCLUDE_DIR}/config/ConfigService.h"
     "${TARGET_INCLUDE_DIR}/config/LanguageService.h"
     "${TARGET_INCLUDE_DIR}/config/LocalizationServerService.h" )
SET( config_SOURCE
     "${TARGET_SOURCE_DIR}/config/ConfigDB.cpp"
     "${TARGET_SOURCE_DIR}/config/ConfigService.cpp"
     "${TARGET_SOURCE_DIR}/config/LanguageService.cpp"
     "${TARGET_SOURCE_DIR}/config/LocalizationServerService.cpp" )

SET( contract_INCLUDE
     "${TARGET_INCLUDE_DIR}/contract/ContractProxy.h"
     "${TARGET_INCLUDE_DIR}/contract/ContractUtils.h" )
SET( contract_SOURCE
     "${TARGET_SOURCE_DIR}/contract/ContractProxy.cpp"
     "${TARGET_SOURCE_DIR}/contract/ContractUtils.cpp" )

SET( corporation_INCLUDE
     "${TARGET_INCLUDE_DIR}/corporation/BillMgr.h"
     "${TARGET_INCLUDE_DIR}/corporation/CorpBookmarkMgr.h"
     "${TARGET_INCLUDE_DIR}/corporation/CorpMgrService.h"
     "${TARGET_INCLUDE_DIR}/corporation/CorpFittingMgr.h"
     "${TARGET_INCLUDE_DIR}/corporation/CorporationDB.h"
     "${TARGET_INCLUDE_DIR}/corporation/CorporationService.h"
     "${TARGET_INCLUDE_DIR}/corporation/CorpRegistryBound.h"
     "${TARGET_INCLUDE_DIR}/corporation/CorpRegistryService.h"
     "${TARGET_INCLUDE_DIR}/corporation/CorpStationMgr.h"
     "${TARGET_INCLUDE_DIR}/corporation/LPService.h"
     "${TARGET_INCLUDE_DIR}/corporation/LPStore.h"
     "${TARGET_INCLUDE_DIR}/corporation/OfficeSparseBound.h" )
SET( corporation_SOURCE
     "${TARGET_SOURCE_DIR}/corporation/BillMgr.cpp"
     "${TARGET_SOURCE_DIR}/corporation/CorpBookmarkMgr.cpp"
     "${TARGET_SOURCE_DIR}/corporation/CorpMgrService.cpp"
     "${TARGET_SOURCE_DIR}/corporation/CorpFittingMgr.cpp"
     "${TARGET_SOURCE_DIR}/corporation/CorporationDB.cpp"
     "${TARGET_SOURCE_DIR}/corporation/CorporationService.cpp"
     "${TARGET_SOURCE_DIR}/corporation/CorpRegistryBound.cpp"
     "${TARGET_SOURCE_DIR}/corporation/CorpRegistryService.cpp"
     "${TARGET_SOURCE_DIR}/corporation/CorpStationMgr.cpp"
     "${TARGET_SOURCE_DIR}/corporation/LPService.cpp"
     "${TARGET_SOURCE_DIR}/corporation/LPStore.cpp"
     "${TARGET_SOURCE_DIR}/corporation/OfficeSparseBound.cpp" )

SET( dogmaim_INCLUDE
     "${TARGET_INCLUDE_DIR}/dogmaim/DogmaIMService.h"
     "${TARGET_INCLUDE_DIR}/dogmaim/DogmaService.h")
SET( dogmaim_SOURCE
     "${TARGET_SOURCE_DIR}/dogmaim/DogmaIMService.cpp"
     "${TARGET_SOURCE_DIR}/dogmaim/DogmaService.cpp" )

SET( dungeon_INCLUDE
     "${TARGET_INCLUDE_DIR}/dungeon/DungeonDB.h"
     "${TARGET_INCLUDE_DIR}/dungeon/DungeonEditor.h"
     "${TARGET_INCLUDE_DIR}/dungeon/DungeonExplorationMgrService.h"
     "${TARGET_INCLUDE_DIR}/dungeon/DungeonService.h")
SET( dungeon_SOURCE
     "${TARGET_SOURCE_DIR}/dungeon/DungeonDB.cpp"
     "${TARGET_SOURCE_DIR}/dungeon/DungeonEditor.cpp"
     "${TARGET_SOURCE_DIR}/dungeon/DungeonExplorationMgrService.cpp"
     "${TARGET_SOURCE_DIR}/dungeon/DungeonService.cpp" )

SET( effects_INCLUDE
     #"${TARGET_INCLUDE_DIR}/effects/EffectsActions.h"
     "${TARGET_INCLUDE_DIR}/effects/EffectsData.h"
     "${TARGET_INCLUDE_DIR}/effects/EffectsDataMgr.h"
     "${TARGET_INCLUDE_DIR}/effects/EffectsProcessor.h"
     "${TARGET_INCLUDE_DIR}/effects/fxData.h")
SET( effects_SOURCE
     #"${TARGET_SOURCE_DIR}/effects/EffectsActions.cpp"
     "${TARGET_SOURCE_DIR}/effects/EffectsDataMgr.cpp"
     "${TARGET_SOURCE_DIR}/effects/EffectsProcessor.cpp" )

SET( explore_INCLUDE
     "${TARGET_INCLUDE_DIR}/exploration/Exploration.h"
     "${TARGET_INCLUDE_DIR}/exploration/Probes.h"
     "${TARGET_INCLUDE_DIR}/exploration/Scan.h"
     "${TARGET_INCLUDE_DIR}/exploration/ScanMgrService.h"
     "${TARGET_INCLUDE_DIR}/exploration/ScanningDB.h")
SET( explore_SOURCE
     "${TARGET_SOURCE_DIR}/exploration/Exploration.cpp"
     "${TARGET_SOURCE_DIR}/exploration/Probes.cpp"
     "${TARGET_SOURCE_DIR}/exploration/Scan.cpp"
     "${TARGET_SOURCE_DIR}/exploration/ScanMgrService.cpp"
     "${TARGET_SOURCE_DIR}/exploration/ScanningDB.cpp")

SET( faction_INCLUDE
     "${TARGET_INCLUDE_DIR}/faction/FactionWarMgrDB.h"
     "${TARGET_INCLUDE_DIR}/faction/FactionWarMgrService.h"
     "${TARGET_INCLUDE_DIR}/faction/WarRegistryService.h" )
SET( faction_SOURCE
     "${TARGET_SOURCE_DIR}/faction/FactionWarMgrDB.cpp"
     "${TARGET_SOURCE_DIR}/faction/FactionWarMgrService.cpp"
     "${TARGET_SOURCE_DIR}/faction/WarRegistryService.cpp")

SET( fleet_INCLUDE
     "${TARGET_INCLUDE_DIR}/fleet/FleetBound.h"
     "${TARGET_INCLUDE_DIR}/fleet/FleetData.h"
     "${TARGET_INCLUDE_DIR}/fleet/FleetManager.h"
     "${TARGET_INCLUDE_DIR}/fleet/FleetObject.h"
     "${TARGET_INCLUDE_DIR}/fleet/FleetProxy.h"
     "${TARGET_INCLUDE_DIR}/fleet/FleetService.h")
SET( fleet_SOURCE
     "${TARGET_SOURCE_DIR}/fleet/FleetBound.cpp"
     "${TARGET_SOURCE_DIR}/fleet/FleetManager.cpp"
     "${TARGET_SOURCE_DIR}/fleet/FleetObject.cpp"
     "${TARGET_SOURCE_DIR}/fleet/FleetProxy.cpp"
     "${TARGET_SOURCE_DIR}/fleet/FleetService.cpp")

SET( imageserver_INCLUDE
     "${TARGET_INCLUDE_DIR}/imageserver/ImageServer.h"
     "${TARGET_INCLUDE_DIR}/imageserver/ImageServerConnection.h"
     "${TARGET_INCLUDE_DIR}/imageserver/ImageServerListener.h" )
SET( imageserver_SOURCE
     "${TARGET_SOURCE_DIR}/imageserver/ImageServer.cpp"
     "${TARGET_SOURCE_DIR}/imageserver/ImageServerConnection.cpp"
     "${TARGET_SOURCE_DIR}/imageserver/ImageServerListener.cpp" )

SET( inventory_INCLUDE
     "${TARGET_INCLUDE_DIR}/inventory/AttributeEnum.h"
     "${TARGET_INCLUDE_DIR}/inventory/AttributeMap.h"
     "${TARGET_INCLUDE_DIR}/inventory/InvBrokerService.h"
     "${TARGET_INCLUDE_DIR}/inventory/Inventory.h"
     "${TARGET_INCLUDE_DIR}/inventory/InventoryBound.h"
     "${TARGET_INCLUDE_DIR}/inventory/InventoryDB.h"
     "${TARGET_INCLUDE_DIR}/inventory/InventoryItem.h"
     "${TARGET_INCLUDE_DIR}/inventory/ItemDB.h"
     "${TARGET_INCLUDE_DIR}/inventory/ItemFactory.h"
     "${TARGET_INCLUDE_DIR}/inventory/ItemRef.h"
     "${TARGET_INCLUDE_DIR}/inventory/ItemType.h"
     "${TARGET_INCLUDE_DIR}/inventory/Voucher.h" )
SET( inventory_SOURCE
     "${TARGET_SOURCE_DIR}/inventory/AttributeMap.cpp"
     "${TARGET_SOURCE_DIR}/inventory/InvBrokerService.cpp"
     "${TARGET_SOURCE_DIR}/inventory/Inventory.cpp"
     "${TARGET_SOURCE_DIR}/inventory/InventoryBound.cpp"
     "${TARGET_SOURCE_DIR}/inventory/InventoryDB.cpp"
     "${TARGET_SOURCE_DIR}/inventory/InventoryItem.cpp"
     "${TARGET_SOURCE_DIR}/inventory/ItemDB.cpp"
     "${TARGET_SOURCE_DIR}/inventory/ItemFactory.cpp"
     "${TARGET_SOURCE_DIR}/inventory/ItemType.cpp"
     "${TARGET_SOURCE_DIR}/inventory/Voucher.cpp" )

SET( mail_INCLUDE
     "${TARGET_INCLUDE_DIR}/mail/MailDB.h"
     "${TARGET_INCLUDE_DIR}/mail/MailingListMgrService.h"
     "${TARGET_INCLUDE_DIR}/mail/MailMgrService.h"
     "${TARGET_INCLUDE_DIR}/mail/NotificationMgrService.h" )
SET( mail_SOURCE
     "${TARGET_SOURCE_DIR}/mail/MailDB.cpp"
     "${TARGET_SOURCE_DIR}/mail/MailingListMgrService.cpp"
     "${TARGET_SOURCE_DIR}/mail/MailMgrService.cpp"
     "${TARGET_SOURCE_DIR}/mail/NotificationMgrService.cpp" )

SET( manufacturing_INCLUDE
     "${TARGET_INCLUDE_DIR}/manufacturing/Blueprint.h"
     "${TARGET_INCLUDE_DIR}/manufacturing/FactoryDB.h"
     "${TARGET_INCLUDE_DIR}/manufacturing/FactoryService.h"
     "${TARGET_INCLUDE_DIR}/manufacturing/RamMethods.h"
     "${TARGET_INCLUDE_DIR}/manufacturing/RamProxyService.h" )
SET( manufacturing_SOURCE
     "${TARGET_SOURCE_DIR}/manufacturing/Blueprint.cpp"
     "${TARGET_SOURCE_DIR}/manufacturing/FactoryDB.cpp"
     "${TARGET_SOURCE_DIR}/manufacturing/FactoryService.cpp"
     "${TARGET_SOURCE_DIR}/manufacturing/RamMethods.cpp"
     "${TARGET_SOURCE_DIR}/manufacturing/RamProxyService.cpp" )

SET( map_INCLUDE
     "${TARGET_INCLUDE_DIR}/map/MapConnections.h"
     "${TARGET_INCLUDE_DIR}/map/MapDB.h"
     "${TARGET_INCLUDE_DIR}/map/MapData.h"
     "${TARGET_INCLUDE_DIR}/map/MapService.h" )
SET( map_SOURCE
     "${TARGET_SOURCE_DIR}/map/MapConnections.cpp"
     "${TARGET_SOURCE_DIR}/map/MapDB.cpp"
     "${TARGET_SOURCE_DIR}/map/MapData.cpp"
     "${TARGET_SOURCE_DIR}/map/MapService.cpp" )

SET( market_INCLUDE
     "${TARGET_INCLUDE_DIR}/market/MarketBotConf.h"
     "${TARGET_INCLUDE_DIR}/market/MarketBotMgr.h"
     "${TARGET_INCLUDE_DIR}/market/MarketMgr.h"
     "${TARGET_INCLUDE_DIR}/market/MarketDB.h"
     "${TARGET_INCLUDE_DIR}/market/MarketProxyService.h" )
     #"${TARGET_INCLUDE_DIR}/market/NPCMarket.h")
SET( market_SOURCE
     "${TARGET_SOURCE_DIR}/market/MarketBotConf.cpp"
     "${TARGET_SOURCE_DIR}/market/MarketBotMgr.cpp"
     "${TARGET_SOURCE_DIR}/market/MarketMgr.cpp"
     "${TARGET_SOURCE_DIR}/market/MarketDB.cpp"
     "${TARGET_SOURCE_DIR}/market/MarketProxyService.cpp" )
     #"${TARGET_SOURCE_DIR}/market/NPCMarket.cpp")

SET( missions_INCLUDE
     "${TARGET_INCLUDE_DIR}/missions/MissionDB.h"
     "${TARGET_INCLUDE_DIR}/missions/MissionDataMgr.h"
     "${TARGET_INCLUDE_DIR}/missions/MissionMgrService.h" )
SET( missions_SOURCE
     "${TARGET_SOURCE_DIR}/missions/MissionDB.cpp"
     "${TARGET_SOURCE_DIR}/missions/MissionDataMgr.cpp"
     "${TARGET_SOURCE_DIR}/missions/MissionMgrService.cpp" )

SET( npc_INCLUDE
     #"${TARGET_INCLUDE_DIR}/npc/Concord.h"
     "${TARGET_INCLUDE_DIR}/npc/Drone.h"
     "${TARGET_INCLUDE_DIR}/npc/DroneAI.h"
     "${TARGET_INCLUDE_DIR}/npc/EntityService.h"
     "${TARGET_INCLUDE_DIR}/npc/NPC.h"
     "${TARGET_INCLUDE_DIR}/npc/NPCAI.h"
     "${TARGET_INCLUDE_DIR}/npc/Sentry.h"
     "${TARGET_INCLUDE_DIR}/npc/SentryAI.h")
SET( npc_SOURCE
     #"${TARGET_SOURCE_DIR}/npc/Concord.cpp"
     "${TARGET_SOURCE_DIR}/npc/Drone.cpp"
     "${TARGET_SOURCE_DIR}/npc/DroneAI.cpp"
     "${TARGET_SOURCE_DIR}/npc/EntityService.cpp"
     "${TARGET_SOURCE_DIR}/npc/NPC.cpp"
     "${TARGET_SOURCE_DIR}/npc/NPCAI.cpp"
     "${TARGET_SOURCE_DIR}/npc/Sentry.cpp"
     "${TARGET_SOURCE_DIR}/npc/SentryAI.cpp" )

SET( planet_INCLUDE
     "${TARGET_INCLUDE_DIR}/planet/Colony.h"
     "${TARGET_INCLUDE_DIR}/planet/CustomsOffice.h"
     "${TARGET_INCLUDE_DIR}/planet/Moon.h"
     "${TARGET_INCLUDE_DIR}/planet/Planet.h"
     "${TARGET_INCLUDE_DIR}/planet/PlanetDB.h"
     "${TARGET_INCLUDE_DIR}/planet/PlanetDataMgr.h"
     "${TARGET_INCLUDE_DIR}/planet/PlanetMgr.h"
     "${TARGET_INCLUDE_DIR}/planet/PlanetMgrBound.h"
     "${TARGET_INCLUDE_DIR}/planet/PlanetORBBound.h")
SET( planet_SOURCE
     "${TARGET_SOURCE_DIR}/planet/Colony.cpp"
     "${TARGET_SOURCE_DIR}/planet/CustomsOffice.cpp"
     "${TARGET_SOURCE_DIR}/planet/Moon.cpp"
     "${TARGET_SOURCE_DIR}/planet/Planet.cpp"
     "${TARGET_SOURCE_DIR}/planet/PlanetDB.cpp"
     "${TARGET_SOURCE_DIR}/planet/PlanetDataMgr.cpp"
     "${TARGET_SOURCE_DIR}/planet/PlanetMgr.cpp"
     "${TARGET_SOURCE_DIR}/planet/PlanetMgrBound.cpp"
     "${TARGET_SOURCE_DIR}/planet/PlanetORBBound.cpp")

SET( pos_INCLUDE
     "${TARGET_INCLUDE_DIR}/pos/Array.h"
     "${TARGET_INCLUDE_DIR}/pos/Battery.h"
     "${TARGET_INCLUDE_DIR}/pos/Module.h"
     "${TARGET_INCLUDE_DIR}/pos/POS_AI.h"
     "${TARGET_INCLUDE_DIR}/pos/PosMgr.h"
     "${TARGET_INCLUDE_DIR}/pos/PosMgrDB.h"
     "${TARGET_INCLUDE_DIR}/pos/Structure.h"
     "${TARGET_INCLUDE_DIR}/pos/Tower.h"
     "${TARGET_INCLUDE_DIR}/pos/Weapon.h"
     "${TARGET_INCLUDE_DIR}/pos/JumpBridge.h"
     "${TARGET_INCLUDE_DIR}/pos/sovStructures/TCU.h"
     "${TARGET_INCLUDE_DIR}/pos/sovStructures/SBU.h"
     "${TARGET_INCLUDE_DIR}/pos/sovStructures/IHub.h")
SET( pos_SOURCE
     "${TARGET_SOURCE_DIR}/pos/Array.cpp"
     "${TARGET_SOURCE_DIR}/pos/Battery.cpp"
     "${TARGET_SOURCE_DIR}/pos/Module.cpp"
     "${TARGET_SOURCE_DIR}/pos/POS_AI.cpp"
     "${TARGET_SOURCE_DIR}/pos/PosMgr.cpp"
     "${TARGET_SOURCE_DIR}/pos/PosMgrDB.cpp"
     "${TARGET_SOURCE_DIR}/pos/Structure.cpp"
     "${TARGET_SOURCE_DIR}/pos/Tower.cpp"
     "${TARGET_SOURCE_DIR}/pos/Weapon.cpp"
     "${TARGET_SOURCE_DIR}/pos/JumpBridge.cpp"
     "${TARGET_SOURCE_DIR}/pos/sovStructures/TCU.cpp"
     "${TARGET_SOURCE_DIR}/pos/sovStructures/SBU.cpp"
     "${TARGET_SOURCE_DIR}/pos/sovStructures/IHub.cpp")

SET( qaTools_INCLUDE
     "${TARGET_INCLUDE_DIR}/qaTools/encounterSpawnServer.h"
     "${TARGET_INCLUDE_DIR}/qaTools/netStateServer.h"
     "${TARGET_INCLUDE_DIR}/qaTools/zActionServer.h")
SET( qaTools_SOURCE
     "${TARGET_SOURCE_DIR}/qaTools/encounterSpawnServer.cpp"
     "${TARGET_SOURCE_DIR}/qaTools/netStateServer.cpp"
     "${TARGET_SOURCE_DIR}/qaTools/zActionServer.cpp")

SET( search_INCLUDE
     "${TARGET_INCLUDE_DIR}/search/Search.h"
     "${TARGET_INCLUDE_DIR}/search/SearchDB.h")
SET( search_SOURCE
     "${TARGET_SOURCE_DIR}/search/Search.cpp"
     "${TARGET_SOURCE_DIR}/search/SearchDB.cpp")

SET( ship_INCLUDE
     "${TARGET_INCLUDE_DIR}/ship/BeyonceService.h"
     "${TARGET_INCLUDE_DIR}/ship/Missile.h"
     "${TARGET_INCLUDE_DIR}/ship/Ship.h"
     "${TARGET_INCLUDE_DIR}/ship/ShipDB.h"
     "${TARGET_INCLUDE_DIR}/ship/ShipService.h")
SET( ship_SOURCE
     "${TARGET_SOURCE_DIR}/ship/BeyonceService.cpp"
     "${TARGET_SOURCE_DIR}/ship/Missile.cpp"
     "${TARGET_SOURCE_DIR}/ship/Ship.cpp"
     "${TARGET_SOURCE_DIR}/ship/ShipDB.cpp"
     "${TARGET_SOURCE_DIR}/ship/ShipService.cpp" )

SET( ship_modules_INCLUDE
     "${TARGET_INCLUDE_DIR}/ship/modules/ActiveModule.h"
     "${TARGET_INCLUDE_DIR}/ship/modules/GenericModule.h"
     "${TARGET_INCLUDE_DIR}/ship/modules/MiningLaser.h"
     "${TARGET_INCLUDE_DIR}/ship/modules/ModuleItem.h"
     "${TARGET_INCLUDE_DIR}/ship/modules/ModuleDefs.h"
     "${TARGET_INCLUDE_DIR}/ship/modules/ModuleFactory.h"
     "${TARGET_INCLUDE_DIR}/ship/modules/ModuleManager.h"
     "${TARGET_INCLUDE_DIR}/ship/modules/Prospector.h"
     "${TARGET_INCLUDE_DIR}/ship/modules/PassiveModule.h"
     "${TARGET_INCLUDE_DIR}/ship/modules/RigModule.h"
     "${TARGET_INCLUDE_DIR}/ship/modules/SubSystemModule.h"
     "${TARGET_INCLUDE_DIR}/ship/modules/TurretFormulas.h"
     "${TARGET_INCLUDE_DIR}/ship/modules/TurretModule.h"
     "${TARGET_INCLUDE_DIR}/ship/modules/CynoModule.h" )
SET( ship_modules_SOURCE
     "${TARGET_SOURCE_DIR}/ship/modules/ActiveModule.cpp"
     "${TARGET_SOURCE_DIR}/ship/modules/GenericModule.cpp"
     "${TARGET_SOURCE_DIR}/ship/modules/MiningLaser.cpp"
     "${TARGET_SOURCE_DIR}/ship/modules/ModuleItem.cpp"
     "${TARGET_SOURCE_DIR}/ship/modules/ModuleManager.cpp"
     "${TARGET_SOURCE_DIR}/ship/modules/Prospector.cpp"
     "${TARGET_SOURCE_DIR}/ship/modules/PassiveModule.cpp"
     "${TARGET_SOURCE_DIR}/ship/modules/RigModule.cpp"
     "${TARGET_SOURCE_DIR}/ship/modules/SubSystemModule.cpp"
     "${TARGET_SOURCE_DIR}/ship/modules/TurretFormulas.cpp"
     "${TARGET_SOURCE_DIR}/ship/modules/TurretModule.cpp"
     "${TARGET_SOURCE_DIR}/ship/modules/CynoModule.cpp" )

SET( standing_INCLUDE
     "${TARGET_INCLUDE_DIR}/standing/Standing.h"
     "${TARGET_INCLUDE_DIR}/standing/StandingDB.h"
     "${TARGET_INCLUDE_DIR}/standing/StandingMgr.h" )
SET( standing_SOURCE
     "${TARGET_SOURCE_DIR}/standing/Standing.cpp"
     "${TARGET_SOURCE_DIR}/standing/StandingDB.cpp"
     "${TARGET_SOURCE_DIR}/standing/StandingMgr.cpp" )

SET( station_INCLUDE
     "${TARGET_INCLUDE_DIR}/station/HoloscreenMgrService.h"
     "${TARGET_INCLUDE_DIR}/station/InsuranceService.h"
     "${TARGET_INCLUDE_DIR}/station/JumpCloneService.h"
     "${TARGET_INCLUDE_DIR}/station/RepairService.h"
     "${TARGET_INCLUDE_DIR}/station/ReprocessingDB.h"
     "${TARGET_INCLUDE_DIR}/station/ReprocessingService.h"
     "${TARGET_INCLUDE_DIR}/station/Station.h"
     "${TARGET_INCLUDE_DIR}/station/StationDataMgr.h"
     "${TARGET_INCLUDE_DIR}/station/StationDB.h"
     "${TARGET_INCLUDE_DIR}/station/StationOffice.h"
     "${TARGET_INCLUDE_DIR}/station/StationService.h"
     "${TARGET_INCLUDE_DIR}/station/StationSvc.h"
     "${TARGET_INCLUDE_DIR}/station/TradeService.h"
     "${TARGET_INCLUDE_DIR}/station/Outpost.h"  )
SET( station_SOURCE
     "${TARGET_SOURCE_DIR}/station/HoloscreenMgrService.cpp"
     "${TARGET_SOURCE_DIR}/station/InsuranceService.cpp"
     "${TARGET_SOURCE_DIR}/station/JumpCloneService.cpp"
     "${TARGET_SOURCE_DIR}/station/RepairService.cpp"
     "${TARGET_SOURCE_DIR}/station/ReprocessingDB.cpp"
     "${TARGET_SOURCE_DIR}/station/ReprocessingService.cpp"
     "${TARGET_SOURCE_DIR}/station/Station.cpp"
     "${TARGET_SOURCE_DIR}/station/StationDataMgr.cpp"
     "${TARGET_SOURCE_DIR}/station/StationDB.cpp"
     "${TARGET_SOURCE_DIR}/station/StationOffice.cpp"
     "${TARGET_SOURCE_DIR}/station/StationService.cpp"
     "${TARGET_SOURCE_DIR}/station/StationSvc.cpp"
     "${TARGET_SOURCE_DIR}/station/TradeService.cpp"
     "${TARGET_SOURCE_DIR}/station/Outpost.cpp" )

SET( system_INCLUDE
     "${TARGET_INCLUDE_DIR}/system/Asteroid.h"
     "${TARGET_INCLUDE_DIR}/system/BookmarkDB.h"
     "${TARGET_INCLUDE_DIR}/system/BookmarkService.h"
     "${TARGET_INCLUDE_DIR}/system/BubbleManager.h"
     "${TARGET_INCLUDE_DIR}/system/CalendarDB.h"
     "${TARGET_INCLUDE_DIR}/system/CalendarProxy.h"
     "${TARGET_INCLUDE_DIR}/system/CalendarMgrService.h"
     "${TARGET_INCLUDE_DIR}/system/Celestial.h"
     "${TARGET_INCLUDE_DIR}/system/CrimeWatch.h"
     "${TARGET_INCLUDE_DIR}/system/Container.h"
     "${TARGET_INCLUDE_DIR}/system/Damage.h"
     "${TARGET_INCLUDE_DIR}/system/DestinyManager.h"
     "${TARGET_INCLUDE_DIR}/system/IndexManager.h"
     "${TARGET_INCLUDE_DIR}/system/KeeperService.h"
     "${TARGET_INCLUDE_DIR}/system/ScenarioService.h"
     "${TARGET_INCLUDE_DIR}/system/SolarSystem.h"
     "${TARGET_INCLUDE_DIR}/system/SystemBubble.h"
     "${TARGET_INCLUDE_DIR}/system/SystemDB.h"
     "${TARGET_INCLUDE_DIR}/system/SystemEntity.h"
     "${TARGET_INCLUDE_DIR}/system/SystemManager.h"
     "${TARGET_INCLUDE_DIR}/system/TargetManager.h"
     "${TARGET_INCLUDE_DIR}/system/WorldSpaceServer.h"
     "${TARGET_INCLUDE_DIR}/system/WormholeSvc.h" )
SET( system_SOURCE
     "${TARGET_SOURCE_DIR}/system/Asteroid.cpp"
     "${TARGET_SOURCE_DIR}/system/BookmarkDB.cpp"
     "${TARGET_SOURCE_DIR}/system/BookmarkService.cpp"
     "${TARGET_SOURCE_DIR}/system/BubbleManager.cpp"
     "${TARGET_SOURCE_DIR}/system/CalendarDB.cpp"
     "${TARGET_SOURCE_DIR}/system/CalendarProxy.cpp"
     "${TARGET_SOURCE_DIR}/system/CalendarMgrService.cpp"
     "${TARGET_SOURCE_DIR}/system/Celestial.cpp"
     "${TARGET_SOURCE_DIR}/system/CrimeWatch.cpp"
     "${TARGET_SOURCE_DIR}/system/Container.cpp"
     "${TARGET_SOURCE_DIR}/system/Damage.cpp"
     "${TARGET_SOURCE_DIR}/system/DestinyManager.cpp"
     "${TARGET_SOURCE_DIR}/system/IndexManager.cpp"
     "${TARGET_SOURCE_DIR}/system/KeeperService.cpp"
     "${TARGET_SOURCE_DIR}/system/ScenarioService.cpp"
     "${TARGET_SOURCE_DIR}/system/SolarSystem.cpp"
     "${TARGET_SOURCE_DIR}/system/SystemBubble.cpp"
     "${TARGET_SOURCE_DIR}/system/SystemDB.cpp"
     "${TARGET_SOURCE_DIR}/system/SystemEntity.cpp"
     "${TARGET_SOURCE_DIR}/system/SystemManager.cpp"
     "${TARGET_SOURCE_DIR}/system/TargetManager.cpp"
     "${TARGET_SOURCE_DIR}/system/WorldSpaceServer.cpp"
     "${TARGET_SOURCE_DIR}/system/WormholeSvc.cpp")

SET( system_sov_INCLUDE
     "${TARGET_INCLUDE_DIR}/system/sov/SovereigntyMgrService.h"
     "${TARGET_INCLUDE_DIR}/system/sov/SovereigntyDB.h"
     "${TARGET_INCLUDE_DIR}/system/sov/SovereigntyDataMgr.h")

SET( system_sov_SOURCE
     "${TARGET_SOURCE_DIR}/system/sov/SovereigntyMgrService.cpp"
     "${TARGET_SOURCE_DIR}/system/sov/SovereigntyDB.cpp"
     "${TARGET_SOURCE_DIR}/system/sov/SovereigntyDataMgr.cpp")

SET( system_managers_INCLUDE
     "${TARGET_INCLUDE_DIR}/system/cosmicMgrs/AnomalyMgr.h"
     "${TARGET_INCLUDE_DIR}/system/cosmicMgrs/BeltMgr.h"
     "${TARGET_INCLUDE_DIR}/system/cosmicMgrs/CivilianMgr.h"
     "${TARGET_INCLUDE_DIR}/system/cosmicMgrs/DungeonMgr.h"
     "${TARGET_INCLUDE_DIR}/system/cosmicMgrs/ManagerDB.h"
     "${TARGET_INCLUDE_DIR}/system/cosmicMgrs/SpawnMgr.h"
     "${TARGET_INCLUDE_DIR}/system/cosmicMgrs/WormholeMgr.h")
SET( system_managers_SOURCE
     "${TARGET_SOURCE_DIR}/system/cosmicMgrs/AnomalyMgr.cpp"
     "${TARGET_SOURCE_DIR}/system/cosmicMgrs/BeltMgr.cpp"
     "${TARGET_SOURCE_DIR}/system/cosmicMgrs/CivilianMgr.cpp"
     "${TARGET_SOURCE_DIR}/system/cosmicMgrs/DungeonMgr.cpp"
     "${TARGET_SOURCE_DIR}/system/cosmicMgrs/ManagerDB.cpp"
     "${TARGET_SOURCE_DIR}/system/cosmicMgrs/SpawnMgr.cpp"
     "${TARGET_SOURCE_DIR}/system/cosmicMgrs/WormholeMgr.cpp")

SET( test_INCLUDE
    "${TARGET_INCLUDE_DIR}/testing/test.h")
SET( test_SOURCE
    "${TARGET_SOURCE_DIR}/testing/test.cpp")

SET( services_INCLUDE
     "${TARGET_INCLUDE_DIR}/services/Service.h"
     "${TARGET_INCLUDE_DIR}/services/Callable.h"
     "${TARGET_INCLUDE_DIR}/services/SparseBound.h"
     "${TARGET_INCLUDE_DIR}/services/BoundService.h"
     "${TARGET_INCLUDE_DIR}/services/ServiceManager.h")
SET( services_SOURCE
     "${TARGET_SOURCE_DIR}/services/Service.cpp"
     "${TARGET_SOURCE_DIR}/services/Callable.cpp"
     "${TARGET_SOURCE_DIR}/services/SparseBound.cpp"
     "${TARGET_SOURCE_DIR}/services/BoundService.cpp"
     "${TARGET_SOURCE_DIR}/services/ServiceManager.cpp")

########################
# Setup the executable #
########################
SOURCE_GROUP( "src"                FILES ${INCLUDE} )
SOURCE_GROUP( "src\\account"       FILES ${account_INCLUDE} )
SOURCE_GROUP( "src\\admin"         FILES ${admin_INCLUDE} )
SOURCE_GROUP( "src\\agent"         FILES ${agent_INCLUDE} )
SOURCE_GROUP( "src\\alliance"      FILES ${alliance_INCLUDE} )
SOURCE_GROUP( "src\\cache"         FILES ${cache_INCLUDE} )
SOURCE_GROUP( "src\\character"     FILES ${character_INCLUDE} )
SOURCE_GROUP( "src\\chat"          FILES ${chat_INCLUDE} )
SOURCE_GROUP( "src\\config"        FILES ${config_INCLUDE} )
SOURCE_GROUP( "src\\contract"      FILES ${contract_INCLUDE} )
SOURCE_GROUP( "src\\corporation"   FILES ${corporation_INCLUDE} )
SOURCE_GROUP( "src\\dogmaim"       FILES ${dogmaim_INCLUDE} )
SOURCE_GROUP( "src\\dungeon"       FILES ${dungeon_INCLUDE} )
SOURCE_GROUP( "src\\effects"       FILES ${effects_INCLUDE} )
SOURCE_GROUP( "src\\exploration"   FILES ${explore_INCLUDE} )
SOURCE_GROUP( "src\\faction"       FILES ${faction_INCLUDE} )
SOURCE_GROUP( "src\\fleet"         FILES ${fleet_INCLUDE} )
SOURCE_GROUP( "src\\imageserver"   FILES ${imageserver_INCLUDE} )
SOURCE_GROUP( "src\\inventory"     FILES ${inventory_INCLUDE} )
SOURCE_GROUP( "src\\mail"          FILES ${mail_INCLUDE} )
SOURCE_GROUP( "src\\manufacturing" FILES ${manufacturing_INCLUDE} )
SOURCE_GROUP( "src\\map"           FILES ${map_INCLUDE} )
SOURCE_GROUP( "src\\market"        FILES ${market_INCLUDE} )
SOURCE_GROUP( "src\\missions"      FILES ${missions_INCLUDE} )
SOURCE_GROUP( "src\\npc"           FILES ${npc_INCLUDE} )
SOURCE_GROUP( "src\\planet"        FILES ${planet_INCLUDE} )
SOURCE_GROUP( "src\\pos"           FILES ${pos_INCLUDE} )
SOURCE_GROUP( "src\\qaTools"       FILES ${qaTools_INCLUDE} )
SOURCE_GROUP( "src\\search"        FILES ${search_INCLUDE} )
SOURCE_GROUP( "src\\services"      FILES ${services_INCLUDE} )
SOURCE_GROUP( "src\\ship"          FILES ${ship_INCLUDE} )
SOURCE_GROUP( "src\\ship\\modules" FILES ${ship_modules_INCLUDE} )
SOURCE_GROUP( "src\\standing"      FILES ${standing_INCLUDE} )
SOURCE_GROUP( "src\\station"       FILES ${station_INCLUDE} )
SOURCE_GROUP( "src\\system"        FILES ${system_INCLUDE} )
SOURCE_GROUP( "src\\system\\sov"   FILES ${system_sov_INCLUDE} )
SOURCE_GROUP( "src\\system\\cosmicMgrs"  FILES ${system_managers_INCLUDE} )
SOURCE_GROUP( "src\\system\\test"  FILES ${test_INCLUDE} )

SOURCE_GROUP( "src"                FILES ${SOURCE} )
SOURCE_GROUP( "src\\account"       FILES ${account_SOURCE} )
SOURCE_GROUP( "src\\admin"         FILES ${admin_SOURCE} )
SOURCE_GROUP( "src\\agent"         FILES ${agent_SOURCE} )
SOURCE_GROUP( "src\\alliance"      FILES ${alliance_SOURCE} )
SOURCE_GROUP( "src\\cache"         FILES ${cache_SOURCE} )
SOURCE_GROUP( "src\\character"     FILES ${character_SOURCE} )
SOURCE_GROUP( "src\\chat"          FILES ${chat_SOURCE} )
SOURCE_GROUP( "src\\config"        FILES ${config_SOURCE} )
SOURCE_GROUP( "src\\contract"      FILES ${contract_SOURCE} )
SOURCE_GROUP( "src\\corporation"   FILES ${corporation_SOURCE} )
SOURCE_GROUP( "src\\dogmaim"       FILES ${dogmaim_SOURCE} )
SOURCE_GROUP( "src\\dungeon"       FILES ${dungeon_SOURCE} )
SOURCE_GROUP( "src\\effects"       FILES ${effects_SOURCE} )
SOURCE_GROUP( "src\\exploration"   FILES ${explore_SOURCE} )
SOURCE_GROUP( "src\\faction"       FILES ${faction_SOURCE} )
SOURCE_GROUP( "src\\fleet"         FILES ${fleet_SOURCE} )
SOURCE_GROUP( "src\\imageserver"   FILES ${imageserver_SOURCE} )
SOURCE_GROUP( "src\\inventory"     FILES ${inventory_SOURCE} )
SOURCE_GROUP( "src\\mail"          FILES ${mail_SOURCE} )
SOURCE_GROUP( "src\\manufacturing" FILES ${manufacturing_SOURCE} )
SOURCE_GROUP( "src\\map"           FILES ${map_SOURCE} )
SOURCE_GROUP( "src\\market"        FILES ${market_SOURCE} )
SOURCE_GROUP( "src\\missions"      FILES ${missions_SOURCE} )
SOURCE_GROUP( "src\\npc"           FILES ${npc_SOURCE} )
SOURCE_GROUP( "src\\planet"        FILES ${planet_SOURCE} )
SOURCE_GROUP( "src\\pos"           FILES ${pos_SOURCE} )
SOURCE_GROUP( "src\\qaTools"       FILES ${qaTools_SOURCE} )
SOURCE_GROUP( "src\\search"        FILES ${search_SOURCE} )
SOURCE_GROUP( "src\\services"      FILES ${services_SOURCE} )
SOURCE_GROUP( "src\\ship"          FILES ${ship_SOURCE} )
SOURCE_GROUP( "src\\ship\\modules" FILES ${ship_modules_SOURCE} )
SOURCE_GROUP( "src\\standing"      FILES ${standing_SOURCE} )
SOURCE_GROUP( "src\\station"       FILES ${station_SOURCE} )
SOURCE_GROUP( "src\\system"        FILES ${system_SOURCE} )
SOURCE_GROUP( "src\\system\\sov"   FILES ${system_sov_SOURCE} )
SOURCE_GROUP( "src\\system\\cosmicMgrs"  FILES ${system_managers_SOURCE} )
SOURCE_GROUP( "src\\system\\test"  FILES ${test_SOURCE} )

ADD_EXECUTABLE( "${TARGET_NAME}"
                ${INCLUDE}                ${SOURCE}
                ${account_INCLUDE}        ${account_SOURCE}
                ${admin_INCLUDE}          ${admin_SOURCE}
                ${agent_INCLUDE}          ${agent_SOURCE}
                ${alliance_INCLUDE}       ${alliance_SOURCE}
                ${cache_INCLUDE}          ${cache_SOURCE}
                ${character_INCLUDE}      ${character_SOURCE}
                ${chat_INCLUDE}           ${chat_SOURCE}
                ${config_INCLUDE}         ${config_SOURCE}
                ${contract_INCLUDE}       ${contract_SOURCE}
                ${corporation_INCLUDE}    ${corporation_SOURCE}
                ${dogmaim_INCLUDE}        ${dogmaim_SOURCE}
                ${dungeon_INCLUDE}        ${dungeon_SOURCE}
                ${effects_INCLUDE}        ${effects_SOURCE}
                ${explore_INCLUDE}        ${explore_SOURCE}
                ${faction_INCLUDE}        ${faction_SOURCE}
                ${fleet_INCLUDE}          ${fleet_SOURCE}
                ${imageserver_INCLUDE}    ${imageserver_SOURCE}
                ${inventory_INCLUDE}      ${inventory_SOURCE}
                ${mail_INCLUDE}           ${mail_SOURCE}
                ${manufacturing_INCLUDE}  ${manufacturing_SOURCE}
                ${map_INCLUDE}            ${map_SOURCE}
                ${market_INCLUDE}         ${market_SOURCE}
                ${missions_INCLUDE}       ${missions_SOURCE}
                ${npc_INCLUDE}            ${npc_SOURCE}
                ${planet_INCLUDE}         ${planet_SOURCE}
                ${pos_INCLUDE}            ${pos_SOURCE}
                ${qaTools_INCLUDE}        ${qaTools_SOURCE}
                ${search_INCLUDE}         ${search_SOURCE}
                ${services_INCLUDE}       ${services_SOURCE}
                ${ship_INCLUDE}           ${ship_SOURCE}
                ${ship_modules_INCLUDE}   ${ship_modules_SOURCE}
                ${standing_INCLUDE}       ${standing_SOURCE}
                ${station_INCLUDE}        ${station_SOURCE}
                ${system_INCLUDE}         ${system_SOURCE}
                ${system_sov_INCLUDE}     ${system_sov_SOURCE}
                ${system_managers_INCLUDE} ${system_managers_SOURCE}
                ${test_INCLUDE}           ${test_SOURCE}
                )

target_precompile_headers( "${TARGET_NAME}" PUBLIC
                  "${TARGET_INCLUDE_DIR}/eve-server.h" )
TARGET_INCLUDE_DIRECTORIES( "${TARGET_NAME}"
                            ${eve-common_INCLUDE_DIRS}
                            "${TARGET_INCLUDE_DIR}" )
TARGET_LINK_LIBRARIES( "${TARGET_NAME}"
                       "eve-common" )

INSTALL( TARGETS "${TARGET_NAME}"
         RUNTIME DESTINATION "bin" )
INSTALL( FILES "${TARGET_UTILS_DIR}/config/devtools.raw"
         DESTINATION "etc" )
