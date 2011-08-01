#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME        "eve-server" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/${TARGET_NAME}" )
SET( TARGET_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )
SET( TARGET_UTILS_DIR   "${PROJECT_SOURCE_DIR}/utils" )

#########
# Files #
#########
SET( INCLUDE
     "${TARGET_INCLUDE_DIR}/Client.h"
     "${TARGET_INCLUDE_DIR}/ClientSession.h"
     "${TARGET_INCLUDE_DIR}/EntityList.h"
     "${TARGET_INCLUDE_DIR}/EVEServerConfig.h"
     "${TARGET_INCLUDE_DIR}/EVEServerPCH.h"
     "${TARGET_INCLUDE_DIR}/NetService.h"
     "${TARGET_INCLUDE_DIR}/PyBoundObject.h"
     "${TARGET_INCLUDE_DIR}/PyCallable.h"
     "${TARGET_INCLUDE_DIR}/PyService.h"
     "${TARGET_INCLUDE_DIR}/PyServiceCD.h"
     "${TARGET_INCLUDE_DIR}/PyServiceMgr.h"
     "${TARGET_INCLUDE_DIR}/ServiceDB.h"
     "${TARGET_INCLUDE_DIR}/LiveUpdateDB.h" )
SET( SOURCE
     "${TARGET_SOURCE_DIR}/Client.cpp"
     "${TARGET_SOURCE_DIR}/ClientSession.cpp"
     "${TARGET_SOURCE_DIR}/EntityList.cpp"
     "${TARGET_SOURCE_DIR}/EVEServerConfig.cpp"
     "${TARGET_SOURCE_DIR}/main.cpp"
     "${TARGET_SOURCE_DIR}/NetService.cpp"
     "${TARGET_SOURCE_DIR}/PyBoundObject.cpp"
     "${TARGET_SOURCE_DIR}/PyCallable.cpp"
     "${TARGET_SOURCE_DIR}/PyService.cpp"
     "${TARGET_SOURCE_DIR}/PyServiceMgr.cpp"
     "${TARGET_SOURCE_DIR}/ServiceDB.cpp"
     "${TARGET_SOURCE_DIR}/LiveUpdateDB.cpp" )

SET( account_INCLUDE
     "${TARGET_INCLUDE_DIR}/account/AccountDB.h"
     "${TARGET_INCLUDE_DIR}/account/AccountService.h"
     "${TARGET_INCLUDE_DIR}/account/AuthService.h"
     "${TARGET_INCLUDE_DIR}/account/ClientStatMgrService.h"
     "${TARGET_INCLUDE_DIR}/account/UserService.h" )
SET( account_SOURCE
     "${TARGET_SOURCE_DIR}/account/AccountDB.cpp"
     "${TARGET_SOURCE_DIR}/account/AccountService.cpp"
     "${TARGET_SOURCE_DIR}/account/AuthService.cpp"
     "${TARGET_SOURCE_DIR}/account/ClientStatMgrService.cpp"
     "${TARGET_SOURCE_DIR}/account/UserService.cpp" )

SET( admin_INCLUDE
     "${TARGET_INCLUDE_DIR}/admin/AlertService.h"
     "${TARGET_INCLUDE_DIR}/admin/AllCommands.h"
     "${TARGET_INCLUDE_DIR}/admin/AllCommandsList.h"
     "${TARGET_INCLUDE_DIR}/admin/CommandDB.h"
     "${TARGET_INCLUDE_DIR}/admin/CommandDispatcher.h"
     "${TARGET_INCLUDE_DIR}/admin/GMCommands.h"
     "${TARGET_INCLUDE_DIR}/admin/PetitionerService.h"
     "${TARGET_INCLUDE_DIR}/admin/SlashService.h" )
SET( admin_SOURCE
     "${TARGET_SOURCE_DIR}/admin/AlertService.cpp"
     "${TARGET_SOURCE_DIR}/admin/AllCommands.cpp"
     "${TARGET_SOURCE_DIR}/admin/CommandDB.cpp"
     "${TARGET_SOURCE_DIR}/admin/CommandDispatcher.cpp"
     "${TARGET_SOURCE_DIR}/admin/GMCommands.cpp"
     "${TARGET_SOURCE_DIR}/admin/PetitionerService.cpp"
     "${TARGET_SOURCE_DIR}/admin/SlashService.cpp" )

SET( authorisation_INCLUDE
     "${TARGET_INCLUDE_DIR}/authorisation/BinAsciiModule.h"
     "${TARGET_INCLUDE_DIR}/authorisation/PasswordModule.h"
     "${TARGET_INCLUDE_DIR}/authorisation/ShaModule.h" )

SET( authorisation_SOURCE
     "${TARGET_SOURCE_DIR}/authorisation/PasswordModule.cpp" )

SET( browser_INCLUDE
     "${TARGET_INCLUDE_DIR}/browser/browserLockdownSvc.h" )

SET( browser_SOURCE
     "${TARGET_SOURCE_DIR}/browser/browserLockdownSvc.cpp" )

SET( cache_INCLUDE
     "${TARGET_INCLUDE_DIR}/cache/ObjCacheDB.h"
     "${TARGET_INCLUDE_DIR}/cache/ObjCacheService.h" )
SET( cache_SOURCE
     "${TARGET_SOURCE_DIR}/cache/ObjCacheDB.cpp"
     "${TARGET_SOURCE_DIR}/cache/ObjCacheService.cpp" )

SET( config_INCLUDE
     "${TARGET_INCLUDE_DIR}/config/ConfigDB.h"
     "${TARGET_INCLUDE_DIR}/config/ConfigService.h"
     "${TARGET_INCLUDE_DIR}/config/LanguageService.h" )
SET( config_SOURCE
     "${TARGET_SOURCE_DIR}/config/ConfigDB.cpp"
     "${TARGET_SOURCE_DIR}/config/ConfigService.cpp"
     "${TARGET_SOURCE_DIR}/config/LanguageService.cpp" )
	 
SET( contractproxy_INCLUDE
	 "${TARGET_INCLUDE_DIR}/contractproxy/contractProxy.h" )
SET( contractproxy_SOURCE
	 "${TARGET_SOURCE_DIR}/contractproxy/contractProxy.cpp" )
	 
SET( corporation_INCLUDE
     "${TARGET_INCLUDE_DIR}/corporation/CorpMgrService.h"
     "${TARGET_INCLUDE_DIR}/corporation/CorporationCarrier.h"
     "${TARGET_INCLUDE_DIR}/corporation/CorporationDB.h"
     "${TARGET_INCLUDE_DIR}/corporation/CorporationService.h"
     "${TARGET_INCLUDE_DIR}/corporation/CorpRegistryService.h"
     "${TARGET_INCLUDE_DIR}/corporation/CorpStationMgrService.h"
     "${TARGET_INCLUDE_DIR}/corporation/LPService.h" )
SET( corporation_SOURCE
     "${TARGET_SOURCE_DIR}/corporation/CorpMgrService.cpp"
     "${TARGET_SOURCE_DIR}/corporation/CorporationDB.cpp"
     "${TARGET_SOURCE_DIR}/corporation/CorporationService.cpp"
     "${TARGET_SOURCE_DIR}/corporation/CorpRegistryService.cpp"
     "${TARGET_SOURCE_DIR}/corporation/CorpStationMgrService.cpp"
     "${TARGET_SOURCE_DIR}/corporation/LPService.cpp" )

SET( dogmaim_INCLUDE
     "${TARGET_INCLUDE_DIR}/dogmaim/DogmaIMDB.h"
     "${TARGET_INCLUDE_DIR}/dogmaim/DogmaIMService.h" )
SET( dogmaim_SOURCE
     "${TARGET_SOURCE_DIR}/dogmaim/DogmaIMDB.cpp"
     "${TARGET_SOURCE_DIR}/dogmaim/DogmaIMService.cpp" )

SET( character_INCLUDE
     "${TARGET_INCLUDE_DIR}/character/CertificateMgrDB.h"
     "${TARGET_INCLUDE_DIR}/character/CertificateMgrService.h"
     "${TARGET_INCLUDE_DIR}/character/Character.h"
     "${TARGET_INCLUDE_DIR}/character/CharacterAppearance_fields.h"
     "${TARGET_INCLUDE_DIR}/character/CharacterDB.h"
     "${TARGET_INCLUDE_DIR}/character/CharacterService.h"
     "${TARGET_INCLUDE_DIR}/character/CharMgrService.h"
     "${TARGET_INCLUDE_DIR}/character/Skill.h"
     "${TARGET_INCLUDE_DIR}/character/SkillMgrService.h"
     "${TARGET_INCLUDE_DIR}/character/CharUnboundMgrService.h"
     "${TARGET_INCLUDE_DIR}/character/PaperDollService.h"
     "${TARGET_INCLUDE_DIR}/character/PhotoUploadService.h"
     "${TARGET_INCLUDE_DIR}/character/AggressionMgrService.h" )
SET( character_SOURCE
     "${TARGET_SOURCE_DIR}/character/CertificateMgrDB.cpp"
     "${TARGET_SOURCE_DIR}/character/CertificateMgrService.cpp"
     "${TARGET_SOURCE_DIR}/character/Character.cpp"
     "${TARGET_SOURCE_DIR}/character/CharacterDB.cpp"
     "${TARGET_SOURCE_DIR}/character/CharacterService.cpp"
     "${TARGET_SOURCE_DIR}/character/CharMgrService.cpp"
     "${TARGET_SOURCE_DIR}/character/Skill.cpp"
     "${TARGET_SOURCE_DIR}/character/SkillMgrService.cpp"
     "${TARGET_SOURCE_DIR}/character/CharUnboundMgrService.cpp"
     "${TARGET_SOURCE_DIR}/character/PaperDollService.cpp"
     "${TARGET_SOURCE_DIR}/character/PhotoUploadService.cpp"
     "${TARGET_SOURCE_DIR}/character/AggressionMgrService.cpp" )

SET( chat_INCLUDE
	 "${TARGET_INCLUDE_DIR}/chat/kenny.h"
     "${TARGET_INCLUDE_DIR}/chat/LookupService.h"
     "${TARGET_INCLUDE_DIR}/chat/LSCDB.h"
     "${TARGET_INCLUDE_DIR}/chat/LSCChannel.h"
     "${TARGET_INCLUDE_DIR}/chat/LSCService.h"
     "${TARGET_INCLUDE_DIR}/chat/OnlineStatusService.h"
     "${TARGET_INCLUDE_DIR}/chat/VoiceMgrService.h" )
SET( chat_SOURCE
	 "${TARGET_SOURCE_DIR}/chat/kenny.cpp"
     "${TARGET_SOURCE_DIR}/chat/LookupService.cpp"
     "${TARGET_SOURCE_DIR}/chat/LSCDB.cpp"
     "${TARGET_SOURCE_DIR}/chat/LSCChannel.cpp"
     "${TARGET_SOURCE_DIR}/chat/LSCService.cpp"
     "${TARGET_SOURCE_DIR}/chat/OnlineStatusService.cpp"
     "${TARGET_SOURCE_DIR}/chat/VoiceMgrService.cpp" )

SET( inventory_INCLUDE
     "${TARGET_INCLUDE_DIR}/inventory/AttributeEnum.h"
     "${TARGET_INCLUDE_DIR}/inventory/AttributeMgr.h"
     "${TARGET_INCLUDE_DIR}/inventory/EVEAttributeMgr.h"
     "${TARGET_INCLUDE_DIR}/inventory/EVEAttributes.h"
     "${TARGET_INCLUDE_DIR}/inventory/InvBrokerService.h"
     "${TARGET_INCLUDE_DIR}/inventory/Inventory.h"
     "${TARGET_INCLUDE_DIR}/inventory/InventoryBound.h"
     "${TARGET_INCLUDE_DIR}/inventory/InventoryDB.h"
     "${TARGET_INCLUDE_DIR}/inventory/InventoryItem.h"
     "${TARGET_INCLUDE_DIR}/inventory/ItemDB.h"
     "${TARGET_INCLUDE_DIR}/inventory/ItemFactory.h"
     "${TARGET_INCLUDE_DIR}/inventory/ItemRef.h"
     "${TARGET_INCLUDE_DIR}/inventory/ItemType.h"
     "${TARGET_INCLUDE_DIR}/inventory/Owner.h" )
SET( inventory_SOURCE
     "${TARGET_SOURCE_DIR}/inventory/EVEAttributeMgr.cpp"
     "${TARGET_SOURCE_DIR}/inventory/InvBrokerService.cpp"
     "${TARGET_SOURCE_DIR}/inventory/Inventory.cpp"
     "${TARGET_SOURCE_DIR}/inventory/InventoryBound.cpp"
     "${TARGET_SOURCE_DIR}/inventory/InventoryDB.cpp"
     "${TARGET_SOURCE_DIR}/inventory/InventoryItem.cpp"
     "${TARGET_SOURCE_DIR}/inventory/ItemDB.cpp"
     "${TARGET_SOURCE_DIR}/inventory/ItemFactory.cpp"
     "${TARGET_SOURCE_DIR}/inventory/ItemType.cpp"
     "${TARGET_SOURCE_DIR}/inventory/Owner.cpp" )

SET( manufacturing_INCLUDE
     "${TARGET_INCLUDE_DIR}/manufacturing/Blueprint.h"
     "${TARGET_INCLUDE_DIR}/manufacturing/FactoryDB.h"
     "${TARGET_INCLUDE_DIR}/manufacturing/FactoryService.h"
     "${TARGET_INCLUDE_DIR}/manufacturing/RamProxyDB.h"
     "${TARGET_INCLUDE_DIR}/manufacturing/RamProxyService.h" )
SET( manufacturing_SOURCE
     "${TARGET_SOURCE_DIR}/manufacturing/Blueprint.cpp"
     "${TARGET_SOURCE_DIR}/manufacturing/FactoryDB.cpp"
     "${TARGET_SOURCE_DIR}/manufacturing/FactoryService.cpp"
     "${TARGET_SOURCE_DIR}/manufacturing/RamProxyDB.cpp"
     "${TARGET_SOURCE_DIR}/manufacturing/RamProxyService.cpp" )

SET( map_INCLUDE
     "${TARGET_INCLUDE_DIR}/map/MapDB.h"
     "${TARGET_INCLUDE_DIR}/map/MapService.h" )
SET( map_SOURCE
     "${TARGET_SOURCE_DIR}/map/MapDB.cpp"
     "${TARGET_SOURCE_DIR}/map/MapService.cpp" )

SET( market_INCLUDE
     "${TARGET_INCLUDE_DIR}/market/BillMgrService.h"
     "${TARGET_INCLUDE_DIR}/market/ContractMgrService.h"
     "${TARGET_INCLUDE_DIR}/market/MarketDB.h"
     "${TARGET_INCLUDE_DIR}/market/MarketProxyService.h" )
SET( market_SOURCE
     "${TARGET_SOURCE_DIR}/market/BillMgrService.cpp"
     "${TARGET_SOURCE_DIR}/market/ContractMgrService.cpp"
     "${TARGET_SOURCE_DIR}/market/MarketDB.cpp"
     "${TARGET_SOURCE_DIR}/market/MarketProxyService.cpp" )

SET( mining_INCLUDE
     "${TARGET_INCLUDE_DIR}/mining/Asteroid.h"
     "${TARGET_INCLUDE_DIR}/mining/AsteroidBeltManager.h"
     "${TARGET_INCLUDE_DIR}/mining/MiningCommands.h"
     "${TARGET_INCLUDE_DIR}/mining/MiningDB.h"
     "${TARGET_INCLUDE_DIR}/mining/ReprocessingDB.h"
     "${TARGET_INCLUDE_DIR}/mining/ReprocessingService.h" )
SET( mining_SOURCE
     "${TARGET_SOURCE_DIR}/mining/Asteroid.cpp"
     "${TARGET_SOURCE_DIR}/mining/AsteroidBeltManager.cpp"
     "${TARGET_SOURCE_DIR}/mining/MiningCommands.cpp"
     "${TARGET_SOURCE_DIR}/mining/MiningDB.cpp"
     "${TARGET_SOURCE_DIR}/mining/ReprocessingDB.cpp"
     "${TARGET_SOURCE_DIR}/mining/ReprocessingService.cpp" )

SET( missions_INCLUDE
     "${TARGET_INCLUDE_DIR}/missions/Agent.h"
     "${TARGET_INCLUDE_DIR}/missions/AgentMgrService.h"
     "${TARGET_INCLUDE_DIR}/missions/MissionDB.h"
     "${TARGET_INCLUDE_DIR}/missions/MissionMgrService.h" )
SET( missions_SOURCE
     "${TARGET_SOURCE_DIR}/missions/Agent.cpp"
     "${TARGET_SOURCE_DIR}/missions/AgentMgrService.cpp"
     "${TARGET_SOURCE_DIR}/missions/MissionDB.cpp"
     "${TARGET_SOURCE_DIR}/missions/MissionMgrService.cpp" )

SET( npc_INCLUDE
     "${TARGET_INCLUDE_DIR}/npc/NPC.h"
     "${TARGET_INCLUDE_DIR}/npc/NPCAI.h" )
     #"${TARGET_INCLUDE_DIR}/npc/NPCAI_State.h"
SET( npc_SOURCE
     "${TARGET_SOURCE_DIR}/npc/NPC.cpp"
     "${TARGET_SOURCE_DIR}/npc/NPCAI.cpp" )
     #"${TARGET_SOURCE_DIR}/npc/NPCAI_State.cpp"

SET( planetint_INCLUDE
	 "${TARGET_INCLUDE_DIR}/planetint/planetMgr.h" )
SET( planetint_SOURCE
	 "${TARGET_SOURCE_DIR}/planetint/planetMgr.cpp" )
	 
SET( posmgr_INCLUDE
     "${TARGET_INCLUDE_DIR}/posmgr/PosMgrDB.h"
     "${TARGET_INCLUDE_DIR}/posmgr/PosMgrService.h"
     "${TARGET_INCLUDE_DIR}/posmgr/Structure.h" )
SET( posmgr_SOURCE
     "${TARGET_SOURCE_DIR}/posmgr/PosMgrDB.cpp"
     "${TARGET_SOURCE_DIR}/posmgr/PosMgrService.cpp"
     "${TARGET_SOURCE_DIR}/posmgr/Structure.cpp" )

SET( ship_INCLUDE
     "${TARGET_INCLUDE_DIR}/ship/BeyonceService.h"
     "${TARGET_INCLUDE_DIR}/ship/DestinyManager.h"
     "${TARGET_INCLUDE_DIR}/ship/dgmtypeattributeinfo.h"
     "${TARGET_INCLUDE_DIR}/ship/Drone.h"
     "${TARGET_INCLUDE_DIR}/ship/InsuranceService.h"
     "${TARGET_INCLUDE_DIR}/ship/ModuleManager.h"
     "${TARGET_INCLUDE_DIR}/ship/Ship.h"
     "${TARGET_INCLUDE_DIR}/ship/ShipDB.h"
     "${TARGET_INCLUDE_DIR}/ship/ShipService.h"
     "${TARGET_INCLUDE_DIR}/ship/TargetManager.h" )
SET( ship_SOURCE
     "${TARGET_SOURCE_DIR}/ship/BeyonceService.cpp"
     "${TARGET_SOURCE_DIR}/ship/DestinyManager.cpp"
     "${TARGET_SOURCE_DIR}/ship/dgmtypeattributeinfo.cpp"
     "${TARGET_SOURCE_DIR}/ship/Drone.cpp"
     "${TARGET_SOURCE_DIR}/ship/InsuranceService.cpp"
     "${TARGET_SOURCE_DIR}/ship/ModuleManager.cpp"
     "${TARGET_SOURCE_DIR}/ship/Ship.cpp"
     "${TARGET_SOURCE_DIR}/ship/ShipDB.cpp"
     "${TARGET_SOURCE_DIR}/ship/ShipService.cpp"
     "${TARGET_SOURCE_DIR}/ship/TargetManager.cpp" )

SET( spawn_INCLUDE
     "${TARGET_INCLUDE_DIR}/spawn/SpawnDB.h"
     "${TARGET_INCLUDE_DIR}/spawn/SpawnManager.h" )
SET( spawn_SOURCE
     "${TARGET_SOURCE_DIR}/spawn/SpawnDB.cpp"
     "${TARGET_SOURCE_DIR}/spawn/SpawnManager.cpp" )

SET( standing_INCLUDE
     "${TARGET_INCLUDE_DIR}/standing/FactionWarMgrDB.h"
     "${TARGET_INCLUDE_DIR}/standing/FactionWarMgrService.h"
     "${TARGET_INCLUDE_DIR}/standing/Standing2Service.h"
     "${TARGET_INCLUDE_DIR}/standing/StandingDB.h"
     "${TARGET_INCLUDE_DIR}/standing/WarRegistryService.h" )
SET( standing_SOURCE
     "${TARGET_SOURCE_DIR}/standing/FactionWarMgrDB.cpp"
     "${TARGET_SOURCE_DIR}/standing/FactionWarMgrService.cpp"
     "${TARGET_SOURCE_DIR}/standing/Standing2Service.cpp"
     "${TARGET_SOURCE_DIR}/standing/StandingDB.cpp"
     "${TARGET_SOURCE_DIR}/standing/WarRegistryService.cpp" )

SET( sovereignty_INCLUDE
     "${TARGET_INCLUDE_DIR}/sovereignty/SovereigntyMgrService.h")
SET( sovereignty_SOURCE
     "${TARGET_SOURCE_DIR}/sovereignty/SovereigntyMgrService.cpp")

SET( station_INCLUDE
     "${TARGET_INCLUDE_DIR}/station/JumpCloneService.h"
     "${TARGET_INCLUDE_DIR}/station/Station.h"
     "${TARGET_INCLUDE_DIR}/station/StationDB.h"
     "${TARGET_INCLUDE_DIR}/station/StationService.h"
     "${TARGET_INCLUDE_DIR}/station/StationSvcService.h" )
SET( station_SOURCE
     "${TARGET_SOURCE_DIR}/station/JumpCloneService.cpp"
     "${TARGET_SOURCE_DIR}/station/Station.cpp"
     "${TARGET_SOURCE_DIR}/station/StationDB.cpp"
     "${TARGET_SOURCE_DIR}/station/StationService.cpp"
     "${TARGET_SOURCE_DIR}/station/StationSvcService.cpp" )

SET( system_INCLUDE
     "${TARGET_INCLUDE_DIR}/system/BookmarkDB.h"
     "${TARGET_INCLUDE_DIR}/system/BookmarkService.h"
     "${TARGET_INCLUDE_DIR}/system/BubbleManager.h"
     "${TARGET_INCLUDE_DIR}/system/Celestial.h"
     "${TARGET_INCLUDE_DIR}/system/Container.h"
     "${TARGET_INCLUDE_DIR}/system/Damage.h"
     "${TARGET_INCLUDE_DIR}/system/Deployable.h"
     "${TARGET_INCLUDE_DIR}/system/DungeonService.h"
     "${TARGET_INCLUDE_DIR}/system/KeeperService.h"
     "${TARGET_INCLUDE_DIR}/system/ScenarioService.h"
     "${TARGET_INCLUDE_DIR}/system/SolarSystem.h"
     "${TARGET_INCLUDE_DIR}/system/SystemBubble.h"
     "${TARGET_INCLUDE_DIR}/system/SystemDB.h"
     "${TARGET_INCLUDE_DIR}/system/SystemEntities.h"
     "${TARGET_INCLUDE_DIR}/system/SystemEntity.h"
     "${TARGET_INCLUDE_DIR}/system/SystemManager.h" )
SET( system_SOURCE
     "${TARGET_SOURCE_DIR}/system/BookmarkDB.cpp"
     "${TARGET_SOURCE_DIR}/system/BookmarkService.cpp"
     "${TARGET_SOURCE_DIR}/system/BubbleManager.cpp"
     "${TARGET_SOURCE_DIR}/system/Celestial.cpp"
     "${TARGET_SOURCE_DIR}/system/Container.cpp"
     "${TARGET_SOURCE_DIR}/system/Damage.cpp"
     "${TARGET_SOURCE_DIR}/system/Deployable.cpp"
     "${TARGET_SOURCE_DIR}/system/DungeonService.cpp"
     "${TARGET_SOURCE_DIR}/system/KeeperService.cpp"
     "${TARGET_SOURCE_DIR}/system/ScenarioService.cpp"
     "${TARGET_SOURCE_DIR}/system/SolarSystem.cpp"
     "${TARGET_SOURCE_DIR}/system/SystemBubble.cpp"
     "${TARGET_SOURCE_DIR}/system/SystemDB.cpp"
     "${TARGET_SOURCE_DIR}/system/SystemEntities.cpp"
     "${TARGET_SOURCE_DIR}/system/SystemEntity.cpp"
     "${TARGET_SOURCE_DIR}/system/SystemManager.cpp" )

SET( trade_INCLUDE
     "${TARGET_INCLUDE_DIR}/trade/TradeService.h"
     "${TARGET_INCLUDE_DIR}/trade/TradeDB.h" )
SET( trade_SOURCE
     "${TARGET_SOURCE_DIR}/trade/TradeService.cpp" )

SET( tutorial_INCLUDE
     "${TARGET_INCLUDE_DIR}/tutorial/TutorialDB.h"
     "${TARGET_INCLUDE_DIR}/tutorial/TutorialService.h" )
SET( tutorial_SOURCE
     "${TARGET_SOURCE_DIR}/tutorial/TutorialDB.cpp"
     "${TARGET_SOURCE_DIR}/tutorial/TutorialService.cpp" )
     
SET( imageserver_INCLUDE
     "${TARGET_INCLUDE_DIR}/imageserver/ImageServer.h"
     "${TARGET_INCLUDE_DIR}/imageserver/ImageServerListener.h"
     "${TARGET_INCLUDE_DIR}/imageserver/ImageServerConnection.h" )
SET( imageserver_SOURCE
     "${TARGET_SOURCE_DIR}/imageserver/ImageServer.cpp"
     "${TARGET_SOURCE_DIR}/imageserver/ImageServerListener.cpp"
     "${TARGET_SOURCE_DIR}/imageserver/ImageServerConnection.cpp" )
     
SET( apiserver_INCLUDE
     "${TARGET_INCLUDE_DIR}/apiserver/APIServer.h"
     "${TARGET_INCLUDE_DIR}/apiserver/APIServerListener.h"
     "${TARGET_INCLUDE_DIR}/apiserver/APIServerConnection.h" )
SET( apiserver_SOURCE
     "${TARGET_SOURCE_DIR}/apiserver/APIServer.cpp"
     "${TARGET_SOURCE_DIR}/apiserver/APIServerListener.cpp"
     "${TARGET_SOURCE_DIR}/apiserver/APIServerConnection.cpp" )


########################
# Setup the executable #
########################
INCLUDE_DIRECTORIES( "${GANGSTA_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${MYSQL_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${TINYXML_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${UTF8CPP_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${ASIO_INCLUDE_DIR}" )

INCLUDE_DIRECTORIES( "${utils_INCLUDE_DIR}" )

INCLUDE_DIRECTORIES( "${common_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${eve-common_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${eve-common_PACKETS_DIR}" )
INCLUDE_DIRECTORIES( "${TARGET_INCLUDE_DIR}" )

SOURCE_GROUP( "include"                FILES ${INCLUDE} )
SOURCE_GROUP( "include\\account"       FILES ${account_INCLUDE} )
SOURCE_GROUP( "include\\admin"         FILES ${admin_INCLUDE} )
SOURCE_GROUP( "include\\authorisation" FILES ${authorisation_INCLUDE} )
SOURCE_GROUP( "include\\browser"       FILES ${browser_INCLUDE} )
SOURCE_GROUP( "include\\cache"         FILES ${cache_INCLUDE} )
SOURCE_GROUP( "include\\config"        FILES ${config_INCLUDE} )
SOURCE_GROUP( "include\\contractproxy" FILES ${contractproxy_INCLUDE} )
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
SOURCE_GROUP( "include\\planetint"	   FILES ${planetint_INCLUDE} )
SOURCE_GROUP( "include\\posmgr"        FILES ${posmgr_INCLUDE} )
SOURCE_GROUP( "include\\ship"          FILES ${ship_INCLUDE} )
SOURCE_GROUP( "include\\sovereignty"   FILES ${sovereignty_INCLUDE} )
SOURCE_GROUP( "include\\spawn"         FILES ${spawn_INCLUDE} )
SOURCE_GROUP( "include\\standing"      FILES ${standing_INCLUDE} )
SOURCE_GROUP( "include\\station"       FILES ${station_INCLUDE} )
SOURCE_GROUP( "include\\system"        FILES ${system_INCLUDE} )
SOURCE_GROUP( "include\\trade"         FILES ${trade_INCLUDE} )
SOURCE_GROUP( "include\\tutorial"      FILES ${tutorial_INCLUDE} )
SOURCE_GROUP( "include\\imageserver"   FILES ${imageserver_INCLUDE} )
SOURCE_GROUP( "include\\apiserver"     FILES ${apiserver_INCLUDE} )


SOURCE_GROUP( "src"                FILES ${SOURCE} )
SOURCE_GROUP( "src\\account"       FILES ${account_SOURCE} )
SOURCE_GROUP( "src\\admin"         FILES ${admin_SOURCE} )
SOURCE_GROUP( "src\\authorisation" FILES ${authorisation_SOURCE} )
SOURCE_GROUP( "src\\browser"       FILES ${browser_SOURCE} )
SOURCE_GROUP( "src\\cache"         FILES ${cache_SOURCE} )
SOURCE_GROUP( "src\\config"        FILES ${config_SOURCE} )
SOURCE_GROUP( "src\\contractproxy" FILES ${contractproxy_SOURCE} )
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
SOURCE_GROUP( "src\\planetint"     FILES ${planetint_SOURCE} )
SOURCE_GROUP( "src\\posmgr"        FILES ${posmgr_SOURCE} )
SOURCE_GROUP( "src\\ship"          FILES ${ship_SOURCE} )
SOURCE_GROUP( "src\\sovereignty"   FILES ${sovereignty_SOURCE} )
SOURCE_GROUP( "src\\spawn"         FILES ${spawn_SOURCE} )
SOURCE_GROUP( "src\\standing"      FILES ${standing_SOURCE} )
SOURCE_GROUP( "src\\station"       FILES ${station_SOURCE} )
SOURCE_GROUP( "src\\system"        FILES ${system_SOURCE} )
SOURCE_GROUP( "src\\trade"         FILES ${trade_SOURCE} )
SOURCE_GROUP( "src\\tutorial"      FILES ${tutorial_SOURCE} )
SOURCE_GROUP( "src\\imageserver"   FILES ${imageserver_SOURCE} )
SOURCE_GROUP( "src\\apiserver"     FILES ${apiserver_SOURCE} )


ADD_EXECUTABLE( "${TARGET_NAME}"
                ${INCLUDE}                ${SOURCE}
                ${account_INCLUDE}        ${account_SOURCE}
                ${admin_INCLUDE}          ${admin_SOURCE}
                ${authorisation_INCLUDE}  ${authorisation_SOURCE} 
                ${browser_INCLUDE}        ${browser_SOURCE} 
                ${cache_INCLUDE}          ${cache_SOURCE}
                ${config_INCLUDE}         ${config_SOURCE}
				${contractproxy_INCLUDE}  ${contractproxy_SOURCE}
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
				${planetint_INCLUDE}	  ${planetint_SOURCE}
                ${posmgr_INCLUDE}         ${posmgr_SOURCE}
                ${sovereignty_INCLUDE}    ${sovereignty_SOURCE} 
                ${ship_INCLUDE}           ${ship_SOURCE}
                ${spawn_INCLUDE}          ${spawn_SOURCE}
                ${standing_INCLUDE}       ${standing_SOURCE}
                ${station_INCLUDE}        ${station_SOURCE}
                ${system_INCLUDE}         ${system_SOURCE}
                ${trade_INCLUDE}          ${trade_SOURCE}
                ${tutorial_INCLUDE}       ${tutorial_SOURCE}
                ${imageserver_INCLUDE}    ${imageserver_SOURCE}
                ${apiserver_INCLUDE}      ${apiserver_SOURCE} )

TARGET_BUILD_PCH( "${TARGET_NAME}"
                  "EVEServerPCH.h"
                  "main.cpp" )
TARGET_LINK_LIBRARIES( "${TARGET_NAME}"
                       "eve-common" "common"
                       "utils"
                       ${GANGSTA_LIBRARIES} ${TINYXML_LIBRARIES}
                       ${MYSQL_LIBRARIES} ${ZLIB_LIBRARIES}
                       ${PROJECT_STANDARD_LIBRARIES} )

INSTALL( TARGETS "${TARGET_NAME}"
         RUNTIME DESTINATION "bin" )
INSTALL( FILES "${TARGET_UTILS_DIR}/eve-server.xml"
         DESTINATION "etc" )

######################
# Export directories #
######################
SET( ${TARGET_NAME}_INCLUDE_DIR "${TARGET_INCLUDE_DIR}" PARENT_SCOPE )
