#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( COMPONENT_NAME        "eve-server" )
SET( COMPONENT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/${COMPONENT_NAME}" )
SET( COMPONENT_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${COMPONENT_NAME}" )

#########
# Files #
#########
# eve-server
SET( INCLUDE
     "${COMPONENT_INCLUDE_DIR}/Client.h"
     "${COMPONENT_INCLUDE_DIR}/ClientSession.h"
     "${COMPONENT_INCLUDE_DIR}/EntityList.h"
     "${COMPONENT_INCLUDE_DIR}/EVEmuServerConfig.h"
     "${COMPONENT_INCLUDE_DIR}/EVEServerPCH.h"
     "${COMPONENT_INCLUDE_DIR}/NetService.h"
     "${COMPONENT_INCLUDE_DIR}/PyBoundObject.h"
     "${COMPONENT_INCLUDE_DIR}/PyCallable.h"
     "${COMPONENT_INCLUDE_DIR}/PyService.h"
     "${COMPONENT_INCLUDE_DIR}/PyServiceCD.h"
     "${COMPONENT_INCLUDE_DIR}/PyServiceMgr.h"
     "${COMPONENT_INCLUDE_DIR}/ServiceDB.h" )
SET( SOURCE
     "${COMPONENT_SOURCE_DIR}/Client.cpp"
     "${COMPONENT_SOURCE_DIR}/ClientSession.cpp"
     "${COMPONENT_SOURCE_DIR}/EntityList.cpp"
     "${COMPONENT_SOURCE_DIR}/EVEmuServerConfig.cpp"
     "${COMPONENT_SOURCE_DIR}/main.cpp"
     "${COMPONENT_SOURCE_DIR}/NetService.cpp"
     "${COMPONENT_SOURCE_DIR}/PyBoundObject.cpp"
     "${COMPONENT_SOURCE_DIR}/PyCallable.cpp"
     "${COMPONENT_SOURCE_DIR}/PyService.cpp"
     "${COMPONENT_SOURCE_DIR}/PyServiceMgr.cpp"
     "${COMPONENT_SOURCE_DIR}/ServiceDB.cpp" )

# eve-server/account
SET( account_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/account/AccountDB.h"
     "${COMPONENT_INCLUDE_DIR}/account/AccountService.h"
     "${COMPONENT_INCLUDE_DIR}/account/AuthService.h"
     "${COMPONENT_INCLUDE_DIR}/account/ClientStatMgrService.h"
     "${COMPONENT_INCLUDE_DIR}/account/UserService.h" )
SET( account_SOURCE
     "${COMPONENT_SOURCE_DIR}/account/AccountDB.cpp"
     "${COMPONENT_SOURCE_DIR}/account/AccountService.cpp"
     "${COMPONENT_SOURCE_DIR}/account/AuthService.cpp"
     "${COMPONENT_SOURCE_DIR}/account/ClientStatMgrService.cpp"
     "${COMPONENT_SOURCE_DIR}/account/UserService.cpp" )

# eve-server/admin
SET( admin_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/admin/AlertService.h"
     "${COMPONENT_INCLUDE_DIR}/admin/AllCommands.h"
     "${COMPONENT_INCLUDE_DIR}/admin/AllCommandsList.h"
     "${COMPONENT_INCLUDE_DIR}/admin/CommandDB.h"
     "${COMPONENT_INCLUDE_DIR}/admin/CommandDispatcher.h"
     "${COMPONENT_INCLUDE_DIR}/admin/GMCommands.h"
     "${COMPONENT_INCLUDE_DIR}/admin/PetitionerService.h"
     "${COMPONENT_INCLUDE_DIR}/admin/SlashService.h" )
SET( admin_SOURCE
     "${COMPONENT_SOURCE_DIR}/admin/AlertService.cpp"
     "${COMPONENT_SOURCE_DIR}/admin/AllCommands.cpp"
     "${COMPONENT_SOURCE_DIR}/admin/CommandDB.cpp"
     "${COMPONENT_SOURCE_DIR}/admin/CommandDispatcher.cpp"
     "${COMPONENT_SOURCE_DIR}/admin/GMCommands.cpp"
     "${COMPONENT_SOURCE_DIR}/admin/PetitionerService.cpp"
     "${COMPONENT_SOURCE_DIR}/admin/SlashService.cpp" )

# eve-server/cache
SET( cache_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/cache/ObjCacheDB.h"
     "${COMPONENT_INCLUDE_DIR}/cache/ObjCacheService.h" )
SET( cache_SOURCE
     "${COMPONENT_SOURCE_DIR}/cache/ObjCacheDB.cpp"
     "${COMPONENT_SOURCE_DIR}/cache/ObjCacheService.cpp" )

# eve-server/config
SET( config_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/config/ConfigDB.h"
     "${COMPONENT_INCLUDE_DIR}/config/ConfigService.h"
     "${COMPONENT_INCLUDE_DIR}/config/LanguageService.h" )
SET( config_SOURCE
     "${COMPONENT_SOURCE_DIR}/config/ConfigDB.cpp"
     "${COMPONENT_SOURCE_DIR}/config/ConfigService.cpp"
     "${COMPONENT_SOURCE_DIR}/config/LanguageService.cpp" )

# eve-server/corporation
SET( corporation_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/corporation/CorpMgrService.h"
     "${COMPONENT_INCLUDE_DIR}/corporation/CorporationCarrier.h"
     "${COMPONENT_INCLUDE_DIR}/corporation/CorporationDB.h"
     "${COMPONENT_INCLUDE_DIR}/corporation/CorporationService.h"
     "${COMPONENT_INCLUDE_DIR}/corporation/CorpRegistryService.h"
     "${COMPONENT_INCLUDE_DIR}/corporation/CorpStationMgrService.h"
     "${COMPONENT_INCLUDE_DIR}/corporation/LPService.h" )
SET( corporation_SOURCE
     "${COMPONENT_SOURCE_DIR}/corporation/CorpMgrService.cpp"
     "${COMPONENT_SOURCE_DIR}/corporation/CorporationDB.cpp"
     "${COMPONENT_SOURCE_DIR}/corporation/CorporationService.cpp"
     "${COMPONENT_SOURCE_DIR}/corporation/CorpRegistryService.cpp"
     "${COMPONENT_SOURCE_DIR}/corporation/CorpStationMgrService.cpp"
     "${COMPONENT_SOURCE_DIR}/corporation/LPService.cpp" )

# eve-server/dogmaim
SET( dogmaim_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/dogmaim/DogmaIMDB.h"
     "${COMPONENT_INCLUDE_DIR}/dogmaim/DogmaIMService.h" )
SET( dogmaim_SOURCE
     "${COMPONENT_SOURCE_DIR}/dogmaim/DogmaIMDB.cpp"
     "${COMPONENT_SOURCE_DIR}/dogmaim/DogmaIMService.cpp" )

# eve-server/character
SET( character_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/character/CertificateMgrDB.h"
     "${COMPONENT_INCLUDE_DIR}/character/CertificateMgrService.h"
     "${COMPONENT_INCLUDE_DIR}/character/Character.h"
     "${COMPONENT_INCLUDE_DIR}/character/CharacterAppearance_fields.h"
     "${COMPONENT_INCLUDE_DIR}/character/CharacterDB.h"
     "${COMPONENT_INCLUDE_DIR}/character/CharacterService.h"
     "${COMPONENT_INCLUDE_DIR}/character/CharMgrService.h"
     "${COMPONENT_INCLUDE_DIR}/character/Skill.h"
     "${COMPONENT_INCLUDE_DIR}/character/SkillMgrService.h" )
SET( character_SOURCE
     "${COMPONENT_SOURCE_DIR}/character/CertificateMgrDB.cpp"
     "${COMPONENT_SOURCE_DIR}/character/CertificateMgrService.cpp"
     "${COMPONENT_SOURCE_DIR}/character/Character.cpp"
     "${COMPONENT_SOURCE_DIR}/character/CharacterDB.cpp"
     "${COMPONENT_SOURCE_DIR}/character/CharacterService.cpp"
     "${COMPONENT_SOURCE_DIR}/character/CharMgrService.cpp"
     "${COMPONENT_SOURCE_DIR}/character/Skill.cpp"
     "${COMPONENT_SOURCE_DIR}/character/SkillMgrService.cpp" )

# eve-server/chat
SET( chat_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/chat/LookupService.h"
     "${COMPONENT_INCLUDE_DIR}/chat/LSCDB.h"
     "${COMPONENT_INCLUDE_DIR}/chat/LSCChannel.h"
     "${COMPONENT_INCLUDE_DIR}/chat/LSCService.h"
     "${COMPONENT_INCLUDE_DIR}/chat/OnlineStatusService.h"
     "${COMPONENT_INCLUDE_DIR}/chat/VoiceMgrService.h" )
SET( chat_SOURCE
     "${COMPONENT_SOURCE_DIR}/chat/LookupService.cpp"
     "${COMPONENT_SOURCE_DIR}/chat/LSCDB.cpp"
     "${COMPONENT_SOURCE_DIR}/chat/LSCChannel.cpp"
     "${COMPONENT_SOURCE_DIR}/chat/LSCService.cpp"
     "${COMPONENT_SOURCE_DIR}/chat/OnlineStatusService.cpp"
     "${COMPONENT_SOURCE_DIR}/chat/VoiceMgrService.cpp" )

# eve-server/inventory
SET( inventory_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/inventory/AttributeMgr.h"
     "${COMPONENT_INCLUDE_DIR}/inventory/EVEAttributeMgr.h"
     "${COMPONENT_INCLUDE_DIR}/inventory/EVEAttributes.h"
     "${COMPONENT_INCLUDE_DIR}/inventory/InvBrokerService.h"
     "${COMPONENT_INCLUDE_DIR}/inventory/Inventory.h"
     "${COMPONENT_INCLUDE_DIR}/inventory/InventoryBound.h"
     "${COMPONENT_INCLUDE_DIR}/inventory/InventoryDB.h"
     "${COMPONENT_INCLUDE_DIR}/inventory/InventoryItem.h"
     "${COMPONENT_INCLUDE_DIR}/inventory/ItemDB.h"
     "${COMPONENT_INCLUDE_DIR}/inventory/ItemFactory.h"
     "${COMPONENT_INCLUDE_DIR}/inventory/ItemRef.h"
     "${COMPONENT_INCLUDE_DIR}/inventory/ItemType.h"
     "${COMPONENT_INCLUDE_DIR}/inventory/Owner.h" )
SET( inventory_SOURCE
     "${COMPONENT_SOURCE_DIR}/inventory/EVEAttributeMgr.cpp"
     "${COMPONENT_SOURCE_DIR}/inventory/InvBrokerService.cpp"
     "${COMPONENT_SOURCE_DIR}/inventory/Inventory.cpp"
     "${COMPONENT_SOURCE_DIR}/inventory/InventoryBound.cpp"
     "${COMPONENT_SOURCE_DIR}/inventory/InventoryDB.cpp"
     "${COMPONENT_SOURCE_DIR}/inventory/InventoryItem.cpp"
     "${COMPONENT_SOURCE_DIR}/inventory/ItemDB.cpp"
     "${COMPONENT_SOURCE_DIR}/inventory/ItemFactory.cpp"
     "${COMPONENT_SOURCE_DIR}/inventory/ItemType.cpp"
     "${COMPONENT_SOURCE_DIR}/inventory/Owner.cpp" )

# eve-server/manufacturing
SET( manufacturing_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/manufacturing/Blueprint.h"
     "${COMPONENT_INCLUDE_DIR}/manufacturing/FactoryDB.h"
     "${COMPONENT_INCLUDE_DIR}/manufacturing/FactoryService.h"
     "${COMPONENT_INCLUDE_DIR}/manufacturing/RamProxyDB.h"
     "${COMPONENT_INCLUDE_DIR}/manufacturing/RamProxyService.h" )
SET( manufacturing_SOURCE
     "${COMPONENT_SOURCE_DIR}/manufacturing/Blueprint.cpp"
     "${COMPONENT_SOURCE_DIR}/manufacturing/FactoryDB.cpp"
     "${COMPONENT_SOURCE_DIR}/manufacturing/FactoryService.cpp"
     "${COMPONENT_SOURCE_DIR}/manufacturing/RamProxyDB.cpp"
     "${COMPONENT_SOURCE_DIR}/manufacturing/RamProxyService.cpp" )

# eve-server/map
SET( map_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/map/MapDB.h"
     "${COMPONENT_INCLUDE_DIR}/map/MapService.h" )
SET( map_SOURCE
     "${COMPONENT_SOURCE_DIR}/map/MapDB.cpp"
     "${COMPONENT_SOURCE_DIR}/map/MapService.cpp" )

# eve-server/market
SET( market_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/market/BillMgrService.h"
     "${COMPONENT_INCLUDE_DIR}/market/ContractMgrService.h"
     "${COMPONENT_INCLUDE_DIR}/market/MarketDB.h"
     "${COMPONENT_INCLUDE_DIR}/market/MarketProxyService.h" )
SET( market_SOURCE
     "${COMPONENT_SOURCE_DIR}/market/BillMgrService.cpp"
     "${COMPONENT_SOURCE_DIR}/market/ContractMgrService.cpp"
     "${COMPONENT_SOURCE_DIR}/market/MarketDB.cpp"
     "${COMPONENT_SOURCE_DIR}/market/MarketProxyService.cpp" )

# eve-server/mining
SET( mining_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/mining/Asteroid.h"
     "${COMPONENT_INCLUDE_DIR}/mining/AsteroidBeltManager.h"
     "${COMPONENT_INCLUDE_DIR}/mining/MiningCommands.h"
     "${COMPONENT_INCLUDE_DIR}/mining/MiningDB.h"
     "${COMPONENT_INCLUDE_DIR}/mining/ReprocessingDB.h"
     "${COMPONENT_INCLUDE_DIR}/mining/ReprocessingService.h" )
SET( mining_SOURCE
     "${COMPONENT_SOURCE_DIR}/mining/Asteroid.cpp"
     "${COMPONENT_SOURCE_DIR}/mining/AsteroidBeltManager.cpp"
     "${COMPONENT_SOURCE_DIR}/mining/MiningCommands.cpp"
     "${COMPONENT_SOURCE_DIR}/mining/MiningDB.cpp"
     "${COMPONENT_SOURCE_DIR}/mining/ReprocessingDB.cpp"
     "${COMPONENT_SOURCE_DIR}/mining/ReprocessingService.cpp" )

# eve-server/missions
SET( missions_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/missions/Agent.h"
     "${COMPONENT_INCLUDE_DIR}/missions/AgentMgrService.h"
     "${COMPONENT_INCLUDE_DIR}/missions/MissionDB.h"
     "${COMPONENT_INCLUDE_DIR}/missions/MissionMgrService.h" )
SET( missions_SOURCE
     "${COMPONENT_SOURCE_DIR}/missions/Agent.cpp"
     "${COMPONENT_SOURCE_DIR}/missions/AgentMgrService.cpp"
     "${COMPONENT_SOURCE_DIR}/missions/MissionDB.cpp"
     "${COMPONENT_SOURCE_DIR}/missions/MissionMgrService.cpp" )

# eve-server/npc
SET( npc_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/npc/NPC.h"
     "${COMPONENT_INCLUDE_DIR}/npc/NPCAI.h" )
     #"${COMPONENT_INCLUDE_DIR}/npc/NPCAI_State.h"
SET( npc_SOURCE
     "${COMPONENT_SOURCE_DIR}/npc/NPC.cpp"
     "${COMPONENT_SOURCE_DIR}/npc/NPCAI.cpp" )
     #"${COMPONENT_SOURCE_DIR}/npc/NPCAI_State.cpp"

# eve-server/posmgr
SET( posmgr_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/posmgr/PosMgrDB.h"
     "${COMPONENT_INCLUDE_DIR}/posmgr/PosMgrService.h" )
SET( posmgr_SOURCE
     "${COMPONENT_SOURCE_DIR}/posmgr/PosMgrDB.cpp"
     "${COMPONENT_SOURCE_DIR}/posmgr/PosMgrService.cpp" )

# eve-server/ship
SET( ship_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/ship/BeyonceService.h"
     "${COMPONENT_INCLUDE_DIR}/ship/DestinyManager.h"
     "${COMPONENT_INCLUDE_DIR}/ship/InsuranceService.h"
     "${COMPONENT_INCLUDE_DIR}/ship/ModuleManager.h"
     "${COMPONENT_INCLUDE_DIR}/ship/Ship.h"
     "${COMPONENT_INCLUDE_DIR}/ship/ShipDB.h"
     "${COMPONENT_INCLUDE_DIR}/ship/ShipService.h"
     "${COMPONENT_INCLUDE_DIR}/ship/TargetManager.h" )
SET( ship_SOURCE
     "${COMPONENT_SOURCE_DIR}/ship/BeyonceService.cpp"
     "${COMPONENT_SOURCE_DIR}/ship/DestinyManager.cpp"
     "${COMPONENT_SOURCE_DIR}/ship/InsuranceService.cpp"
     "${COMPONENT_SOURCE_DIR}/ship/ModuleManager.cpp"
     "${COMPONENT_SOURCE_DIR}/ship/Ship.cpp"
     "${COMPONENT_SOURCE_DIR}/ship/ShipDB.cpp"
     "${COMPONENT_SOURCE_DIR}/ship/ShipService.cpp"
     "${COMPONENT_SOURCE_DIR}/ship/TargetManager.cpp" )

# eve-server/spawn
SET( spawn_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/spawn/SpawnDB.h"
     "${COMPONENT_INCLUDE_DIR}/spawn/SpawnManager.h" )
SET( spawn_SOURCE
     "${COMPONENT_SOURCE_DIR}/spawn/SpawnDB.cpp"
     "${COMPONENT_SOURCE_DIR}/spawn/SpawnManager.cpp" )

# eve-server/standing
SET( standing_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/standing/FactionWarMgrDB.h"
     "${COMPONENT_INCLUDE_DIR}/standing/FactionWarMgrService.h"
     "${COMPONENT_INCLUDE_DIR}/standing/Standing2Service.h"
     "${COMPONENT_INCLUDE_DIR}/standing/StandingDB.h"
     "${COMPONENT_INCLUDE_DIR}/standing/WarRegistryService.h" )
SET( standing_SOURCE
     "${COMPONENT_SOURCE_DIR}/standing/FactionWarMgrDB.cpp"
     "${COMPONENT_SOURCE_DIR}/standing/FactionWarMgrService.cpp"
     "${COMPONENT_SOURCE_DIR}/standing/Standing2Service.cpp"
     "${COMPONENT_SOURCE_DIR}/standing/StandingDB.cpp"
     "${COMPONENT_SOURCE_DIR}/standing/WarRegistryService.cpp" )

# eve-server/station
SET( station_INCLDUDE
     "${COMPONENT_INCLUDE_DIR}/station/JumpCloneService.h"
     "${COMPONENT_INCLUDE_DIR}/station/Station.h"
     "${COMPONENT_INCLUDE_DIR}/station/StationDB.h"
     "${COMPONENT_INCLUDE_DIR}/station/StationService.h"
     "${COMPONENT_INCLUDE_DIR}/station/StationSvcService.h" )
SET( station_SOURCE
     "${COMPONENT_SOURCE_DIR}/station/JumpCloneService.cpp"
     "${COMPONENT_SOURCE_DIR}/station/Station.cpp"
     "${COMPONENT_SOURCE_DIR}/station/StationDB.cpp"
     "${COMPONENT_SOURCE_DIR}/station/StationService.cpp"
     "${COMPONENT_SOURCE_DIR}/station/StationSvcService.cpp" )

# eve-server/system
SET( system_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/system/BookmarkDB.h"
     "${COMPONENT_INCLUDE_DIR}/system/BookmarkService.h"
     "${COMPONENT_INCLUDE_DIR}/system/BubbleManager.h"
     "${COMPONENT_INCLUDE_DIR}/system/Celestial.h"
     "${COMPONENT_INCLUDE_DIR}/system/Damage.h"
     "${COMPONENT_INCLUDE_DIR}/system/DungeonService.h"
     "${COMPONENT_INCLUDE_DIR}/system/KeeperService.h"
     "${COMPONENT_INCLUDE_DIR}/system/ScenarioService.h"
     "${COMPONENT_INCLUDE_DIR}/system/SolarSystem.h"
     "${COMPONENT_INCLUDE_DIR}/system/SystemBubble.h"
     "${COMPONENT_INCLUDE_DIR}/system/SystemDB.h"
     "${COMPONENT_INCLUDE_DIR}/system/SystemEntities.h"
     "${COMPONENT_INCLUDE_DIR}/system/SystemEntity.h"
     "${COMPONENT_INCLUDE_DIR}/system/SystemManager.h" )
SET( system_SOURCE
     "${COMPONENT_SOURCE_DIR}/system/BookmarkDB.cpp"
     "${COMPONENT_SOURCE_DIR}/system/BookmarkService.cpp"
     "${COMPONENT_SOURCE_DIR}/system/BubbleManager.cpp"
     "${COMPONENT_SOURCE_DIR}/system/Celestial.cpp"
     "${COMPONENT_SOURCE_DIR}/system/Damage.cpp"
     "${COMPONENT_SOURCE_DIR}/system/DungeonService.cpp"
     "${COMPONENT_SOURCE_DIR}/system/KeeperService.cpp"
     "${COMPONENT_SOURCE_DIR}/system/ScenarioService.cpp"
     "${COMPONENT_SOURCE_DIR}/system/SolarSystem.cpp"
     "${COMPONENT_SOURCE_DIR}/system/SystemBubble.cpp"
     "${COMPONENT_SOURCE_DIR}/system/SystemDB.cpp"
     "${COMPONENT_SOURCE_DIR}/system/SystemEntities.cpp"
     "${COMPONENT_SOURCE_DIR}/system/SystemEntity.cpp"
     "${COMPONENT_SOURCE_DIR}/system/SystemManager.cpp" )

# eve-server/tutorial
SET( tutorial_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/tutorial/TutorialDB.h"
     "${COMPONENT_INCLUDE_DIR}/tutorial/TutorialService.h" )
SET( tutorial_SOURCE
     "${COMPONENT_SOURCE_DIR}/tutorial/TutorialDB.cpp"
     "${COMPONENT_SOURCE_DIR}/tutorial/TutorialService.cpp" )

########################
# Setup the executable #
########################
INCLUDE_DIRECTORIES( "${MYSQL_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${dep_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${common_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${eve-common_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${eve-common_PACKETS_DIR}" )
INCLUDE_DIRECTORIES( "${COMPONENT_INCLUDE_DIR}" )

SOURCE_GROUP( "include"                FILES ${INCLUDE} )
SOURCE_GROUP( "include\\account"       FILES ${account_INCLUDE} )
SOURCE_GROUP( "include\\admin"         FILES ${admin_INCLUDE} )
SOURCE_GROUP( "include\\cache"         FILES ${cache_INCLUDE} )
SOURCE_GROUP( "include\\config"        FILES ${config_INCLUDE} )
SOURCE_GROUP( "include\\corporation"   FILES ${corporation_INCLUDE} )
SOURCE_GROUP( "include\\dogmaim"       FILES ${dogmaim_INCLUDE} )
SOURCE_GROUP( "include\\character"     FILES ${character_INCLUDE} )
SOURCE_GROUP( "include\\chat"          FILES ${chat_INCLUDE} )
SOURCE_GROUP( "include\\inventory"     FILES ${inventory_INCLUDE} )
SOURCE_GROUP( "include\\manufacturing" FILES ${manufacturing_INCLUDE} )
SOURCE_GROUP( "include\\map"           FILES ${map_INCLUDE} )
SOURCE_GROUP( "include\\market"        FILES ${market_INCLUDE} )
SOURCE_GROUP( "include\\mining"        FILES ${mining_INCLUDE} )
SOURCE_GROUP( "include\\missions"      FILES ${missions_INCLUDE} )
SOURCE_GROUP( "include\\npc"           FILES ${npc_INCLUDE} )
SOURCE_GROUP( "include\\posmgr"        FILES ${posmgr_INCLUDE} )
SOURCE_GROUP( "include\\ship"          FILES ${ship_INCLUDE} )
SOURCE_GROUP( "include\\spawn"         FILES ${spawn_INCLUDE} )
SOURCE_GROUP( "include\\standing"      FILES ${standing_INCLUDE} )
SOURCE_GROUP( "include\\station"       FILES ${station_INCLUDE} )
SOURCE_GROUP( "include\\system"        FILES ${system_INCLUDE} )
SOURCE_GROUP( "include\\tutorial"      FILES ${tutorial_INCLUDE} )

SOURCE_GROUP( "src"                FILES ${SOURCE} )
SOURCE_GROUP( "src\\account"       FILES ${account_SOURCE} )
SOURCE_GROUP( "src\\admin"         FILES ${admin_SOURCE} )
SOURCE_GROUP( "src\\cache"         FILES ${cache_SOURCE} )
SOURCE_GROUP( "src\\config"        FILES ${config_SOURCE} )
SOURCE_GROUP( "src\\corporation"   FILES ${corporation_SOURCE} )
SOURCE_GROUP( "src\\dogmaim"       FILES ${dogmaim_SOURCE} )
SOURCE_GROUP( "src\\character"     FILES ${character_SOURCE} )
SOURCE_GROUP( "src\\chat"          FILES ${chat_SOURCE} )
SOURCE_GROUP( "src\\inventory"     FILES ${inventory_SOURCE} )
SOURCE_GROUP( "src\\manufacturing" FILES ${manufacturing_SOURCE} )
SOURCE_GROUP( "src\\map"           FILES ${map_SOURCE} )
SOURCE_GROUP( "src\\market"        FILES ${market_SOURCE} )
SOURCE_GROUP( "src\\mining"        FILES ${mining_SOURCE} )
SOURCE_GROUP( "src\\missions"      FILES ${missions_SOURCE} )
SOURCE_GROUP( "src\\npc"           FILES ${npc_SOURCE} )
SOURCE_GROUP( "src\\posmgr"        FILES ${posmgr_SOURCE} )
SOURCE_GROUP( "src\\ship"          FILES ${ship_SOURCE} )
SOURCE_GROUP( "src\\spawn"         FILES ${spawn_SOURCE} )
SOURCE_GROUP( "src\\standing"      FILES ${standing_SOURCE} )
SOURCE_GROUP( "src\\station"       FILES ${station_SOURCE} )
SOURCE_GROUP( "src\\system"        FILES ${system_SOURCE} )
SOURCE_GROUP( "src\\tutorial"      FILES ${tutorial_SOURCE} )

ADD_EXECUTABLE( "${COMPONENT_NAME}"
                ${INCLUDE}                ${SOURCE}
                ${account_INCLUDE}        ${account_SOURCE}
                ${admin_INCLUDE}          ${admin_SOURCE}
                ${cache_INCLUDE}          ${cache_SOURCE}
                ${config_INCLUDE}         ${config_SOURCE}
                ${corporation_INCLUDE}    ${corporation_SOURCE}
                ${dogmaim_INCLUDE}        ${dogmaim_SOURCE}
                ${character_INCLUDE}      ${character_SOURCE}
                ${chat_INCLUDE}           ${chat_SOURCE}
                ${inventory_INCLUDE}      ${inventory_SOURCE}
                ${manufacturing_INCLUDE}  ${manufacturing_SOURCE}
                ${map_INCLUDE}            ${map_SOURCE}
                ${market_INCLUDE}         ${market_SOURCE}
                ${mining_INCLUDE}         ${mining_SOURCE}
                ${missions_INCLUDE}       ${missions_SOURCE}
                ${npc_INCLUDE}            ${npc_SOURCE}
                ${posmgr_INCLUDE}         ${posmgr_SOURCE}
                ${ship_INCLUDE}           ${ship_SOURCE}
                ${spawn_INCLUDE}          ${spawn_SOURCE}
                ${standing_INCLUDE}       ${standing_SOURCE}
                ${station_INCLUDE}        ${station_SOURCE}
                ${system_INCLUDE}         ${system_SOURCE}
                ${tutorial_INCLUDE}       ${tutorial_SOURCE} )

TARGET_BUILD_PCH( "${COMPONENT_NAME}" "EVEServerPCH.h" "main.cpp" )
TARGET_LINK_LIBRARIES( "${COMPONENT_NAME}"
                       "eve-common" "common" "dep"
                       ${MYSQL_LIBRARIES} ${ZLIB_LIBRARIES} )

######################
# Export directories #
######################
SET( ${COMPONENT_NAME}_INCLUDE_DIR "${COMPONENT_INCLUDE_DIR}" PARENT_SCOPE )
