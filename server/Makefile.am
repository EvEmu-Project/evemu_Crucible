INCLUDES+= -I$(srcdir)/../common/

noinst_LIBRARIES = \
	libaccount.a \
	libadmin.a \
	libcache.a \
	libcharacter.a \
	libchat.a \
	libcorporation.a \
	libconfig.a \
	libdogmaim.a \
	libinventory.a \
	libmanufacturing.a \
	libmap.a \
	libmarket.a \
	libmining.a \
	libmissions.a \
	libstanding.a \
	libnpc.a \
	libspawn.a \
	libship.a \
	libstation.a \
	libsystem.a \
	libtutorial.a

bin_PROGRAMS = server

libaccount_a_SOURCES = \
	account/AccountDB.cpp \
	account/AccountDB.h \
	account/AccountService.cpp \
	account/AccountService.h \
	account/AuthService.cpp \
	account/AuthService.h

libadmin_a_SOURCES = \
	admin/AlertService.cpp \
	admin/AlertService.h \
	admin/AllCommands.cpp \
	admin/AllCommands.h \
	admin/AllCommandsList.h \
	admin/CommandDB.cpp \
	admin/CommandDB.h \
	admin/CommandDispatcher.cpp \
	admin/CommandDispatcher.h \
	admin/GMCommands.cpp \
	admin/GMCommands.h \
	admin/PetitionerService.cpp \
	admin/PetitionerService.h \
	admin/SlashService.cpp \
	admin/SlashService.h

libcache_a_SOURCES = \
	cache/CachableList.h \
	cache/ObjCacheDB.cpp \
	cache/ObjCacheDB.h \
	cache/ObjCacheService.cpp \
	cache/ObjCacheService.h

libcharacter_a_SOURCES = \
	character/CharacterDB.cpp \
	character/CharacterDB.h \
	character/CharacterService.cpp \
	character/CharacterService.h \
	character/CharMgrService.cpp \
	character/CharMgrService.h \
	character/SkillMgrService.cpp \
	character/SkillMgrService.h

libchat_a_SOURCES = \
	chat/LookupService.cpp \
	chat/LookupService.h \
	chat/LSCChannel.cpp \
	chat/LSCChannel.h \
	chat/LSCDB.cpp \
	chat/LSCDB.h \
	chat/LSCService.cpp \
	chat/LSCService.h \
	chat/OnlineStatusService.cpp \
	chat/OnlineStatusService.h

libconfig_a_SOURCES = \
	config/ConfigDB.cpp \
	config/ConfigDB.h \
	config/ConfigService.cpp \
	config/ConfigService.h \
	config/LanguageService.cpp \
	config/LanguageService.h

libcorporation_a_SOURCES = \
	corporation/CorpMgrService.cpp \
	corporation/CorpMgrService.h \
	corporation/CorporationCarrier.h \
	corporation/CorporationDB.cpp \
	corporation/CorporationDB.h \
	corporation/CorporationService.cpp \
	corporation/CorporationService.h \
	corporation/CorpRegistryService.cpp \
	corporation/CorpRegistryService.h \
	corporation/CorpStationMgrService.cpp \
	corporation/CorpStationMgrService.h

libdogmaim_a_SOURCES = \
	dogmaim/DogmaIMDB.cpp \
	dogmaim/DogmaIMDB.h \
	dogmaim/DogmaIMService.cpp \
	dogmaim/DogmaIMService.h

libinventory_a_SOURCES = \
	inventory/EVEAttributes.h \
	inventory/InvBrokerService.cpp \
	inventory/InvBrokerService.h \
	inventory/InventoryDB.cpp \
	inventory/InventoryDB.h \
	inventory/InventoryItem.cpp \
	inventory/InventoryItem.h \
	inventory/ItemFactory.cpp \
	inventory/ItemFactory.h
# Seems not to be included
#	inventory/ItemDB.cpp 
#	inventory/ItemDB.h 

libmanufacturing_a_SOURCES = \
	manufacturing/FactoryDB.cpp \
	manufacturing/FactoryDB.h \
	manufacturing/FactoryService.cpp \
	manufacturing/FactoryService.h \
	manufacturing/RamProxyDB.cpp \
	manufacturing/RamProxyDB.h \
	manufacturing/RamProxyService.cpp \
	manufacturing/RamProxyService.h

libmap_a_SOURCES = \
	map/MapDB.cpp \
	map/MapDB.h \
	map/MapService.cpp \
	map/MapService.h

libmarket_a_SOURCES = \
	market/BillMgrService.cpp \
	market/BillMgrService.h \
	market/MarketDB.cpp \
	market/MarketDB.h \
	market/MarketProxyService.cpp \
	market/MarketProxyService.h

libmining_a_SOURCES = \
	mining/AsteroidBeltManager.cpp \
	mining/AsteroidBeltManager.h \
	mining/Asteroid.cpp \
	mining/Asteroid.h \
	mining/MiningCommands.cpp \
	mining/MiningCommands.h \
	mining/MiningDB.cpp \
	mining/MiningDB.h \
	mining/ReprocessingDB.cpp \
	mining/ReprocessingDB.h \
	mining/ReprocessingService.cpp \
	mining/ReprocessingService.h

libmissions_a_SOURCES = \
	missions/Agent.cpp \
	missions/Agent.h \
	missions/AgentMgrService.cpp \
	missions/AgentMgrService.h \
	missions/MissionDB.cpp \
	missions/MissionDB.h \
	missions/MissionMgrService.cpp \
	missions/MissionMgrService.h

libnpc_a_SOURCES = \
	npc/NPCAI.cpp \
	npc/NPCAI.h

# Broken? missing libs/cppstates
#	npc/NPCAI_State.cpp 
#	npc/NPCAI_State.h

libship_a_SOURCES = \
	ship/BeyonceService.cpp \
	ship/BeyonceService.h \
	ship/DestinyManager.cpp \
	ship/DestinyManager.h \
	ship/InsuranceService.cpp \
	ship/InsuranceService.h \
	ship/ModuleManager.cpp \
	ship/ModuleManager.h \
	ship/ShipDB.cpp \
	ship/ShipDB.h \
	ship/ShipService.cpp \
	ship/ShipService.h \
	ship/TargetManager.cpp \
	ship/TargetManager.h

libspawn_a_SOURCES = \
	spawn/SpawnDB.cpp \
	spawn/SpawnDB.h \
	spawn/SpawnManager.cpp \
	spawn/SpawnManager.h

libstanding_a_SOURCES = \
	standing/Standing2Service.cpp \
	standing/Standing2Service.h \
	standing/StandingDB.cpp \
	standing/StandingDB.h

libstation_a_SOURCES = \
	station/JumpCloneService.cpp \
	station/JumpCloneService.h \
	station/StationDB.cpp \
	station/StationDB.h \
	station/StationService.cpp \
	station/StationService.h \
	station/StationSvcService.cpp \
	station/StationSvcService.h

libsystem_a_SOURCES = \
	system/BubbleManager.cpp \
	system/BubbleManager.h \
	system/Damage.cpp \
	system/Damage.h \
	system/DungeonService.cpp \
	system/DungeonService.h \
	system/KeeperService.cpp \
	system/KeeperService.h \
	system/ScenarioService.cpp \
	system/ScenarioService.h \
	system/SystemBubble.cpp \
	system/SystemBubble.h \
	system/SystemDB.cpp \
	system/SystemDB.h \
	system/SystemEntities.cpp \
	system/SystemEntities.h \
	system/SystemEntity.cpp \
	system/SystemEntity.h \
	system/SystemManager.cpp \
	system/SystemManager.h

libtutorial_a_SOURCES = \
	tutorial/TutorialDB.cpp \
	tutorial/TutorialDB.h \
	tutorial/TutorialService.cpp \
	tutorial/TutorialService.h

server_SOURCES = \
	BookmarkService.cpp \
	BookmarkService.h \
	Client.cpp \
	Client.h \
	ClientSession.cpp \
	ClientSession_fields.h \
	ClientSession.h \
	EntityList.cpp \
	EntityList.h \
	EVEmuServerConfig.cpp \
	EVEmuServerConfig_elements.h \
	EVEmuServerConfig.h \
	main.cpp \
	NPC.cpp \
	NPC.h \
	PyBoundObject.cpp \
	PyBoundObject.h \
	PyCallable.cpp \
	PyCallable.h \
	PyServiceCD.h \
	PyService.cpp \
	PyService.h \
	PyServiceMgr.cpp \
	PyServiceMgr.h \
	ServiceDB.cpp \
	ServiceDB.h

server_LDADD = \
  -laccount \
  -ladmin \
  -lcache \
  -lcharacter \
  -lchat \
  -lcorporation \
  -lconfig \
  -ldogmaim \
  -linventory \
  -lmanufacturing \
  -lmap \
  -lmarket \
  -lmining \
  -lmissions \
  -lstanding \
  -lnpc \
  -lspawn \
  -lship \
  -lstation \
  -lsystem \
	-ltutorial \
	-lcommon \
	-ltinyxml \
	-lpackets \
	-lz \
	-lpthread \
	-ldl 

server_LDFLAGS = \
	-L$(srcdir)/../server/ \
	-L$(srcdir)/../common/ \
	-L$(srcdir)/../packets/ \
	@MYSQL_LDFLAGS@

AM_CXXFLAGS+= \
	-pipe \
	-fno-inline \
	-Wall \
	-ansi
