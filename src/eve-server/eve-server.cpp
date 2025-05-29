/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2021 The EVEmu Team
 *    For the latest information visit https://evemu.dev
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:     Zhur, mmcs
 *    Rewrite:    Allan
 */

#include "eve-server.h"
// version
#include "../eve-common/EVEVersion.h"

#include "EVEServerConfig.h"
#include "NetService.h"
// data managers
#include "StaticDataMgr.h"
#include "StatisticMgr.h"
#include "missions/MissionDataMgr.h"
//console commands
#include "ConsoleCommands.h"
// account services
#include "account/AccountService.h"
#include "account/AuthService.h"
#include "account/BrowserLockdownSvc.h"
#include "account/ClientStatMgrService.h"
#include "account/InfoGatheringMgr.h"
#include "account/TutorialService.h"
#include "account/UserService.h"
// admin services
#include "admin/AlertService.h"
#include "admin/AllCommands.h"
#include "admin/ClientStatLogger.h"
#include "admin/CommandDispatcher.h"
#include "admin/DevToolsProviderService.h"
#include "admin/PetitionerService.h"
#include "admin/SlashService.h"
// agent services
#include "agents/Agent.h"
#include "agents/AgentMgrService.h"
// alliance services
#include "alliance/AllianceRegistry.h"
// calendar services
#include "system/CalendarMgrService.h"
#include "system/CalendarProxy.h"
// cache services
#include "cache/BulkDB.h"
#include "cache/BulkMgrService.h"
#include "cache/ObjCacheService.h"
// character services
#include "character/AggressionMgrService.h"
#include "character/CertificateMgrService.h"
#include "character/CharFittingMgr.h"
#include "character/CharMgrService.h"
#include "character/CharUnboundMgrService.h"
#include "character/PaperDollService.h"
#include "character/PhotoUploadService.h"
#include "character/SkillMgrService.h"
// chat services
#include "chat/LookupService.h"
#include "chat/LSCService.h"
#include "chat/OnlineStatusService.h"
#include "chat/VoiceMgrService.h"
// config services
#include "config/ConfigService.h"
#include "config/LanguageService.h"
#include "config/LocalizationServerService.h"
// contract services
#include "contract/ContractProxy.h"
// corporation services
#include "corporation/BillMgr.h"
#include "corporation/CorpBookmarkMgr.h"
#include "corporation/CorpFittingMgr.h"
#include "corporation/CorpMgrService.h"
#include "corporation/CorporationService.h"
#include "corporation/CorpRegistryService.h"
#include "corporation/CorpStationMgr.h"
#include "corporation/LPService.h"
#include "corporation/LPStore.h"
// dogmaim services
#include "dogmaim/DogmaIMService.h"
#include "dogmaim/DogmaService.h"
#include "effects/EffectsDataMgr.h"
// dungeon services
#include "dungeon/DungeonExplorationMgrService.h"
#include "dungeon/DungeonService.h"
// entity service (player drones)
#include "npc/EntityService.h"
// exploration services
#include "exploration/ScanMgrService.h"
// faction services
#include "faction/FactionWarMgrService.h"
#include "faction/WarRegistryService.h"
// fleet services
#include "fleet/FleetObject.h"
#include "fleet/FleetProxy.h"
#include "fleet/FleetService.h"
// imageserver services
#include "imageserver/ImageServer.h"
// development index service
#include "system/IndexManager.h"
// inventory services
#include "inventory/InvBrokerService.h"
#include "inventory/Voucher.h"
// mail services
#include "mail/MailMgrService.h"
#include "mail/MailingListMgrService.h"
#include "mail/NotificationMgrService.h"
// manufacturing services
#include "manufacturing/FactoryService.h"
#include "manufacturing/RamProxyService.h"
// map services
#include "map/MapData.h"
#include "map/MapService.h"
// market services
#include "market/MarketMgr.h"
#include "market/MarketProxyService.h"
#include "market/MarketBotMgr.h"
// missions services
#include "missions/MissionMgrService.h"
// planet services
#include "planet/Planet.h"
#include "planet/PlanetDataMgr.h"
#include "planet/PlanetMgrBound.h"
#include "planet/PlanetORBBound.h"
// pos services
#include "pos/PosMgr.h"
#include "pos/Structure.h"
// qaTools
#include "qaTools/encounterSpawnServer.h"
#include "qaTools/netStateServer.h"
#include "qaTools/zActionServer.h"
// search services
#include "search/Search.h"
// ship services
#include "ship/BeyonceService.h"
#include "ship/ShipService.h"
// standing services
#include "standing/Standing.h"
#include "standing/StandingMgr.h"
// station services
#include "station/HoloscreenMgrService.h"
#include "station/InsuranceService.h"
#include "station/JumpCloneService.h"
#include "station/RepairService.h"
#include "station/ReprocessingService.h"
#include "station/StationDataMgr.h"
#include "station/StationService.h"
#include "station/StationSvc.h"
#include "station/TradeService.h"
// system services
#include "system/BookmarkService.h"
#include "system/BubbleManager.h"
#include "system/KeeperService.h"
#include "system/ScenarioService.h"
#include "system/sov/SovereigntyMgrService.h"
#include "system/sov/SovereigntyDataMgr.h"
#include "system/WormholeSvc.h"
// cosmic managers
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/CivilianMgr.h"
#include "system/cosmicMgrs/DungeonMgr.h"
#include "system/cosmicMgrs/SpawnMgr.h"
#include "system/cosmicMgrs/WormholeMgr.h"
// database cleaner service
#include "DBCleaner.h"

static const char* const SRV_CONFIG_FILE = EVEMU_ROOT "/etc/eve-server.xml";

static void SetupSignals();
static void CatchSignal( int sig_num );

static volatile bool m_run = true;

CommandDispatcher* g_dispatcher = nullptr; // ---commandlist update

int main( int argc, char* argv[] )
{
    double profileStartTime(GetTimeMSeconds());

    /* set current time for timer */
    Timer::SetCurrentTime();

    /* init logging */
    sLog.Initialize();
    /* Load server log settings */
    if (load_log_settings(sConfig.files.logSettings.c_str())) {
        sLog.Green( "       ServerInit", "Log settings loaded from %s", sConfig.files.logSettings.c_str() );
    } else {
        sLog.Warning( "       ServerInit", "Unable to read %s (this file is optional)", sConfig.files.logSettings.c_str() );
    }

    std::printf("\n");     // spacer
    sLog.Green("       ServerInit", "Loading Server Configuration from %s.", SRV_CONFIG_FILE);
    // should i try to load individual config files here?  probably not, but would look cool.  ;)
    /* Load server configuration */
    if (!sConfig.ParseFile(SRV_CONFIG_FILE)) {
        sLog.Error( "       ServerInit", "ERROR: Loading server configuration '%s' failed.", SRV_CONFIG_FILE );
        std::cout << std::endl << "press any key to exit...";  std::cin.get();
        return EXIT_FAILURE;
    }

    std::printf("\n");     // spacer
    /* display server config data */
    sLog.Log(" Supported Client", " %s", EVEProjectVersion);
    sLog.Log("   Client Version", " %.2f", EVEVersionNumber);
    sLog.Log("     Client Build", " %d", EVEBuildVersion);
    sLog.Log("         MachoNet", " %u", MachoNetVersion);
    sLog.Log("  Server Revision", " %s", EVEMU_REVISION );
    sLog.Log("       Build Date", " %s", EVEMU_BUILD_DATE );
    sLog.Log("   Config Version", " %.1f", Config_Version );
    sLog.Log("      Log Version", " %.1f", Log_Version );
    sLog.Log("TraderJoe Version", " %.2f", Joe_Version );
    sLog.Log(" Missions Version", " %.2f", Mission_Version );
    sLog.Log("     Scan Version", " %.2f", Scan_Version );
    sLog.Log("   NPC AI Version", " %.2f", NPC_AI_Version );
    sLog.Log(" Drone AI Version", " %.2f", Drone_AI_Version );
    sLog.Log("   Civ AI Version", " %.2f", Civilian_AI_Version );
    sLog.Log("Sentry AI Version", " %.2f", Sentry_AI_Version );
    sLog.Log("   POS AI Version", " %.2f", POS_AI_Version );
    std::printf("\n");     // spacer

    /* open up the log file if specified */
    if (!sConfig.files.logDir.empty()) {
        //sLog.InitializeLogging(sConfig.files.logDir);
        std::string logFile = sConfig.files.logDir + "eve-server.log";
        if ( log_open_logfile( logFile.c_str() ) ) {
            sLog.Green( "       ServerInit", "Found log directory %s", sConfig.files.logDir.c_str() );
        } else {
            sLog.Warning( "       ServerInit", "Unable to find log directory '%s', only logging to the screen now.", sConfig.files.logDir.c_str() );
        }
    }
    std::printf("\n");     // spacer

    sLog.Green("       ServerInit", "Server Configuration Files Loaded.");
    std::printf("\n");     // spacer

    sLog.Blue("     ServerConfig", "Main Loop Settings");
    uint8 m_sleepTime = sConfig.server.ServerSleepTime; // default 10ms.  max 256ms
    if (m_sleepTime == 10) {
        sLog.Green("  Loop Sleep Time","Default at 10ms.");
    } else {
        sLog.Error("  Loop Sleep Time","**Be Careful With This Setting!**");
        sLog.Warning("  Loop Sleep Time","Changed from default 10ms to %ums.", m_sleepTime);
    }
    /* removed code for this  25 March 2020
    uint16 m_idle = sConfig.server.idleSleepTime;       // default 1s.  max 65.535s
    if (m_idle == 1000)
        sLog.Green("  Idle Sleep Time","Default at 1000ms.");
    else {
        sLog.Error("  Loop Sleep Time","**Be Careful With This Setting!**");
        sLog.Yellow("  Idle Sleep Time","Changed from default 1000ms to %ums.", m_idle);
    } */
    std::printf("\n");     // spacer

    /* Custom config file options
     * current settings displayed on console at start-up
     *   -allan 7June2015
     */
    sLog.Blue("     ServerConfig", "World Switches");
    if (sConfig.world.saveOnMove) {
        sLog.Green("     Save on Move","Enabled.");
    } else {
        sLog.Warning("     Save on Move","Disabled.");
    }
    if (sConfig.world.saveOnUpdate) {
        sLog.Green("   Save on Update","Enabled.");
    } else {
        sLog.Warning("   Save on Update","Disabled.");
    }
    if (sConfig.world.StationDockDelay) {
        sLog.Green("    Docking Delay","Enabled.");
    } else {
        sLog.Warning("    Docking Delay","Disabled.");
    }
    if (sConfig.world.gridUnload) {
        sLog.Green("   Grid Unloading","Enabled.  Grids will unload after %u seconds of inactivity.", sConfig.world.gridUnloadTime);
    } else {
        sLog.Warning("   Grid Unloading","Disabled.");
    }
    std::printf("\n");     // spacer

    sLog.Blue("     ServerConfig", "Rate Modifiers");
    if (sConfig.rates.secRate != 1.0) {
        sLog.Yellow("        SecStatus","Modified at %.0f%%.", (sConfig.rates.secRate *100) );
    } else {
        sLog.Green("        SecStatus","Normal.");
    }
    if (sConfig.rates.npcBountyMultiply != 1.0) {
        sLog.Yellow("          Bountys","Modified at %.0f%%.", (sConfig.rates.npcBountyMultiply *100) );
    } else {
        sLog.Green("          Bountys","Normal.");
    }
    if (sConfig.rates.DropItem != 1) {
        sLog.Yellow("       Item Drops","Modified at %ux.", sConfig.rates.DropItem );
    } else {
        sLog.Green("       Item Drops","Normal.");
    }
    if (sConfig.rates.DropSalvage != 1) {
        sLog.Yellow("    Salvage Drops","Modified at %ux.", sConfig.rates.DropSalvage );
    } else {
        sLog.Green("    Salvage Drops","Normal.");
    }
    if (sConfig.rates.DropMoney != 1.0) {
        sLog.Yellow("      Isk Rewards","Modified at %.0f%%.", (sConfig.rates.DropMoney *100) );
    } else {
        sLog.Green("      Isk Rewards","Normal.");
    }
    if (sConfig.rates.damageRate != 1.0) {
        sLog.Yellow("      All Damages","Modified at %.0f%%.", (sConfig.rates.damageRate *100) );
    } else {
        sLog.Green("      All Damages","Normal.");
    }
    if (sConfig.rates.turretDamage != 1.0) {
        sLog.Yellow("       Turret Dmg","Modified at %.0f%%.", (sConfig.rates.turretDamage *100) );
    } else {
        sLog.Green("       Turret Dmg","Normal.");
    }
    if (sConfig.rates.turretRoF != 1.0) {
        sLog.Yellow("       Turret ROF","Modified at %.0f%%.", (sConfig.rates.turretRoF *100) );
    } else {
        sLog.Green("       Turret ROF","Normal.");
    }
    if (sConfig.rates.missileDamage != 1.0) {
        sLog.Yellow("      Missile Dmg","Modified at %.0f%%.", (sConfig.rates.missileDamage *100) );
    } else {
        sLog.Green("      Missile Dmg","Normal.");
    }
    if (sConfig.rates.missileRoF != 1.0) {
        sLog.Yellow("      Missile ROF","Modified at %.0f%%.", (sConfig.rates.missileRoF *100) );
    } else {
        sLog.Green("      Missile ROF","Normal.");
    }
    if (sConfig.rates.missileTime != 1.0) {
        sLog.Yellow("     Missile Time","Modified at %.0f%%.", (sConfig.rates.missileTime *100) );
    } else {
        sLog.Green("     Missile Time","Normal.");
    }
    std::printf("\n");     // spacer
    sLog.Blue("     ServerConfig", "R.A.M. Rate Modifiers");
    if (sConfig.ram.ResPE != 1.0) {
        sLog.Yellow(" PE Research Time","Modified at %.0f%%.", (sConfig.ram.ResPE *100) );
    } else {
        sLog.Green(" PE Research Time","Normal.");
    }
    if (sConfig.ram.ResME != 1.0) {
        sLog.Yellow(" ME Research Time","Modified at %.0f%%.", (sConfig.ram.ResME *100) );
    } else {
        sLog.Green(" ME Research Time","Normal.");
    }
    if (sConfig.ram.MatMod != 1.0) {
        sLog.Yellow("Material Modifier","Modified at %.0f%%.", (sConfig.ram.MatMod *100) );
    } else {
        sLog.Green("Material Modifier","Normal.");
    }
    if (sConfig.ram.CopyTime != 1.0) {
        sLog.Yellow("        Copy Time","Modified at %.0f%%.", (sConfig.ram.CopyTime *100) );
    } else {
        sLog.Green("        Copy Time","Normal.");
    }
    if (sConfig.ram.ProdTime != 1.0) {
        sLog.Yellow("  Production Time","Modified at %.0f%%.", (sConfig.ram.ProdTime *100) );
    } else {
        sLog.Green("  Production Time","Normal.");
    }
    if (sConfig.ram.InventTime != 1.0) {
        sLog.Yellow("   Invention Time","Modified at %.0f%%.", (sConfig.ram.InventTime *100) );
    } else {
        sLog.Green("   Invention Time","Normal.");
    }
    if (sConfig.ram.ReTime != 1.0) {
        sLog.Yellow("          RE Time","Modified at %.0f%%.", (sConfig.ram.ReTime *100) );
    } else {
        sLog.Green("          RE Time","Normal.");
    }
    if (sConfig.ram.WasteMod != 1.0) {
        sLog.Yellow("   Waste Modifier","Modified at %.0f%%.", (sConfig.ram.WasteMod *100) );
    } else {
        sLog.Green("   Waste Modifier","Normal.");
    }
    std::printf("\n");     // spacer

    sLog.Blue("     ServerConfig","Critical Hit Chances");
    if (sConfig.rates.PlayerCritChance != 0.02f) {
        sLog.Yellow("           Player","Modified at %.1f%%.", (sConfig.rates.PlayerCritChance *100) );
    } else {
        sLog.Green("           Player","Normal at 2%%.");
    }
    if (sConfig.rates.NpcCritChance != 0.015f) {
        sLog.Yellow("              NPC","Modified at %.1f%%.", (sConfig.rates.NpcCritChance *100) );
    } else {
        sLog.Green("              NPC","Normal at 1.5%%.");
    }
    if (sConfig.rates.SentryCritChance != 0.02f) {
        sLog.Yellow("           Sentry","Modified at %.1f%%.", (sConfig.rates.SentryCritChance *100) );
    } else {
        sLog.Green("           Sentry","Normal at 2%%.");
    }
    if (sConfig.rates.DroneCritChance != 0.03f) {
        sLog.Yellow("            Drone","Modified at %.1f%%.", (sConfig.rates.DroneCritChance *100) );
    } else {
        sLog.Green("            Drone","Normal at 3%%.");
    }
    if (sConfig.rates.ConcordCritChance != 0.05f) {
        sLog.Yellow("          Concord","Modified at %.1f%%.", (sConfig.rates.ConcordCritChance *100) );
    } else {
        sLog.Green("          Concord","Normal at 5%%.");
    }
    std::printf("\n");     // spacer

    sLog.Blue("     ServerConfig", "Feature Switches");
    if (sConfig.ram.AutoEvent) {
        sLog.Green("    RAM AutoEvent","Enabled.");
    } else {
        sLog.Warning("    RAM AutoEvent","Disabled.");
    }
    if (sConfig.server.ModuleAutoOff) {
        sLog.Green("  Module Auto-Off","Enabled.");
    } else {
        sLog.Warning("  Module Auto-Off","Disabled.");
    }
    if (sConfig.server.AsteroidsOnDScan) {
        sLog.Green("  DScan Asteroids","Enabled.");
    } else {
        sLog.Warning("  DScan Asteroids","Disabled.");
    }
    if (sConfig.server.CargoMassAdditive) {
        sLog.Green("       Cargo Mass","Enabled.");
    } else {
        sLog.Warning("       Cargo Mass","Disabled.");
    }
    if (sConfig.cosmic.BumpEnabled) {
        sLog.Green("Bumping Mechanics","Enabled.");
    } else {
        sLog.Warning("Bumping Mechanics","Disabled.");
    }
    if (sConfig.server.LoadOldMissions) {
        sLog.Green("Keep Old Missions","Enabled.");
    } else {
        sLog.Warning("Keep Old Missions","Disabled.");
    }
    if (sConfig.testing.EnableDrones) {
        sLog.Green("    Player Drones","Enabled.");
    } else {
        sLog.Warning("    Player Drones","Disabled.");
    }
    if (sConfig.testing.ShipHeat) {
        sLog.Green("        Ship Heat","Enabled.");
    } else {
        sLog.Warning("        Ship Heat","Disabled.");
    }
    if (sConfig.cosmic.PIEnabled) {
        sLog.Green("        PI System","Enabled.");
    } else {
        sLog.Warning("        PI System","Disabled.");
    }
    if (sConfig.cosmic.AnomalyEnabled) {
        sLog.Green("   Anomaly System","Enabled.");
    } else {
        sLog.Warning("   Anomaly System","Disabled.");
    }
    if (sConfig.cosmic.DungeonEnabled) {
        sLog.Green("   Dungeon System","Enabled.");
    } else {
        sLog.Warning("   Dungeon System","Disabled.");
    }
    if (sConfig.cosmic.BeltEnabled) {
        sLog.Green("   Asteroid Belts","Enabled.");
    } else {
        sLog.Warning("   Asteroid Belts","Disabled.");
    }
    if (sConfig.npc.StaticSpawns) {
        sLog.Green("    Static Spawns","Enabled.  Checks every %u minutes", sConfig.npc.StaticTimer /60);
    } else {
        sLog.Warning("    Static Spawns","Disabled.");
    }
    if (sConfig.npc.RoamingSpawns) {
        sLog.Green("   Roaming Spawns","Enabled.  Checks every %u minutes", sConfig.npc.RoamingTimer /60);
    } else {
        sLog.Warning("   Roaming Spawns","Disabled.");
    }
    if (sConfig.npc.RoamingSpawns or sConfig.npc.StaticSpawns)
        sLog.Green("   Spawns Enabled","Respawn timer checks every %u minutes", sConfig.npc.RespawnTimer /60);
    if (sConfig.server.BountyPayoutDelayed) {
        sLog.Green(" Delayed Bounties","Delayed Bounties are Enabled.  Loop runs every %u minutes", sConfig.server.BountyPayoutTimer);
        if (sConfig.server.FleetShareDelayed) {
            sLog.Green(" Delayed Bounties","Delay for Fleet Bounty Sharing is Enabled.");
        } else {
            sLog.Warning(" Delayed Bounties","Delay for Fleet Bounty Sharing is Disabled.  Fleet Sharing of Bounties is immediate.");
        }
    } else {
        sLog.Warning(" Delayed Bounties","Delayed Bounties are Disabled.  Bounty payouts are immediate.");
        if (sConfig.server.FleetShareDelayed) {
            sLog.Warning(" Delayed Bounties","Delayed Bounties are Disabled.  Fleet Sharing of Bounties is immediate.");
        }  else {
            sLog.Warning(" Delayed Bounties","Delay for Fleet Bounty Sharing is Disabled.  Fleet Sharing of Bounties is immediate.");
        }
    }
    std::printf("\n");     // spacer

    sLog.Blue("     ServerConfig", "Misc Switches");
    if (sConfig.server.ModuleDamageChance) {
        sLog.Green("    Module Damage","Enabled.  Set to %i%% chance.", (int8)(sConfig.server.ModuleDamageChance *100));
    } else {
        sLog.Warning("    Module Damage","Disabled.");
    }
    if (sConfig.rates.WorldDecay) {
        sLog.Green("      Decay Timer","Enabled.  Checks every %u minutes", sConfig.rates.WorldDecay);
    }  else {
        sLog.Warning("      Decay Timer","Disabled.");
    }
    if (sConfig.server.TraderJoe) {
        sLog.Green("   Market Bot Mgr", "TraderJoe is Enabled.");
        /* create the MarketBot singleton */
        sLog.Green("       ServerInit", "Starting Market Bot Manager");
        sMktBotMgr.Initialize();
    } else {
        sLog.Warning("   Market Bot Mgr", "TraderJoe is Disabled.");
    }
    std::printf("\n");     // spacer

    sLog.Blue("     ServerConfig", "Debug Switches");
    if (sConfig.debug.IsTestServer) {
        sLog.Error("     ServerConfig", "Test Server Enabled");
    }  else {
        sLog.Error("     ServerConfig", "Live Server Enabled");
    }
    if (sConfig.debug.StackTrace) {
        sLog.Warning("       StackTrace", "Enabled");
    }  else {
        sLog.Warning("       StackTrace", "Disabled");
    }
    if (sConfig.debug.UseProfiling) {
        sLog.Green(" Server Profiling","Enabled.");
        sProfiler.Initialize();
    } else {
        sLog.Warning(" Server Profiling","Disabled.");
    }
    if (sConfig.debug.BeanCount) {
        sLog.Green("     BeanCounting","Enabled.");
    } else {
        sLog.Warning("     BeanCounting","Disabled.");
    }
    if (sConfig.debug.SpawnTest) {
        sLog.Warning("       Spawn Test","Enabled.");
    }  else {
        sLog.Warning("       Spawn Test","Disabled.");
    }
    if (sConfig.debug.BubbleTrack) {
        sLog.Warning("  Bubble Tracking","Enabled.");
    }  else {
        sLog.Warning("  Bubble Tracking","Disabled.");
    }
    if (sConfig.debug.UseShipTracking) {
        sLog.Warning("    Ship Tracking","Enabled.");
    }  else {
        sLog.Warning("    Ship Tracking","Disabled.");
    }
    if (sConfig.debug.PositionHack) {
        sLog.Warning("    Position Hack","Enabled.");
    }  else {
        sLog.Warning("    Position Hack","Disabled.");
    }
    std::printf("\n");     // spacer

    sAllocators.tickAllocator.Init(Allocators::TICK_ALLOCATOR_SIZE, "TickAllocator");

    /* Start up the TCP server */
    EVETCPServer tcps;
    char errbuf[ TCPCONN_ERRBUF_SIZE ];
    sLog.Green( "       ServerInit", "Starting TCP Server");
    if (tcps.Open(sConfig.net.port, errbuf)) {
        sLog.Blue( "    BaseTCPServer", "TCP Server started on port %u.", sConfig.net.port );
    } else {
        sLog.Error( "    BaseTCPServer", "Error starting TCP Server: %s.", errbuf );
        std::cout << std::endl << "press any key to exit...";  std::cin.get();
        return EXIT_FAILURE;
    }
    std::printf("\n");     // spacer
    Sleep(250);

    /* connect to the database */
    sLog.Green("       ServerInit", "Connecting to DataBase");
    sDatabase.Initialize(sConfig.database.host,
                         sConfig.database.username,
                         sConfig.database.password,
                         sConfig.database.db,
                         sConfig.database.compress,
                         sConfig.database.ssl,
                         sConfig.database.port,
                         sConfig.database.useSocket,
                         sConfig.database.autoReconnect,
                         sConfig.debug.UseProfiling
                        );
    if (sDatabase.GetStatus() != DBcore::Connected) {
        // error msg printed in DBcore::Initalize routine
        std::cout << std::endl << "press any key to exit...";  std::cin.get();
        return EXIT_FAILURE;
    }
    std::printf("\n");     // spacer

    // Clean DB upon initialisation
    dbClean.Initialize();
    std::printf("\n");

    // start up the image server
    sLog.Green("       ServerInit", "Starting Image Server");
    sImageServer.Run();
    //  this gives the imageserver's server time to load so the dynamic database msgs are in order
    Sleep(250);

    sThread.Initialize();
    sLog.Green( "        Threading", "Starting Main Loop thread with ID 0x%X", std::this_thread::get_id() );
    //sThread.AddThread(pthread_self());
    std::printf("\n");     // spacer

    // basic shit done.  begin loading server specifics...
    sLog.Green("       ServerInit", "Loading server");
    std::printf("\n");     // spacer

    /* create a single item factory */
    sLog.Green("       ServerInit", "Starting Item Factory");
    sItemFactory.Initialize();
    /* initialize EntityList singleton, clientID seed and start tic timer */
    sLog.Green("       ServerInit", "Starting Entity List");
    sEntityList.Initialize();
    /* create a service manager */
    sLog.Green("       ServerInit", "Starting Service Manager");
    EVEServiceManager newSvcMgr(888444);
    /* create a command dispatcher */
    sLog.Green("       ServerInit", "Starting Command Dispatch Manager");
    CommandDispatcher command_dispatcher(newSvcMgr);
    g_dispatcher = &command_dispatcher; // ---commandlist update
    RegisterAllCommands(command_dispatcher);
    sLog.Blue(" Command Dispatch", "Command Dispatcher Initialized.");
    /* Service creation and registration. */
    sLog.Green("       ServerInit", "Registering Service Managers."); // 90 currently known pyServMgr
    double startTime = GetTimeMSeconds();
    /* Please keep the pyServMgr list clean so it's easier to find things */
    /* 'services' here are systems that respond to client calls */
    // move this into a service Init() function?   will need more work to do...
    newSvcMgr.Register(new RamProxyService());
    newSvcMgr.Register(new PosMgr(newSvcMgr));
    newSvcMgr.Register(new FleetObject(newSvcMgr));
    newSvcMgr.Register(new CorpStationMgr(newSvcMgr));
    newSvcMgr.Register(new ContractProxy());
    newSvcMgr.Register(new CorpBookmarkMgr());
    newSvcMgr.Register(new BookmarkService());
    newSvcMgr.Register(new CharMgrService(newSvcMgr));
    newSvcMgr.Register(new SlashService(&command_dispatcher));
    newSvcMgr.Register(new LSCService(newSvcMgr, &command_dispatcher));
    newSvcMgr.Register(new JumpCloneService(newSvcMgr));
    newSvcMgr.Register(new MailMgrService());
    newSvcMgr.Register(new MailingListMgrService());
    newSvcMgr.Register(new CharFittingMgr());
    newSvcMgr.Register(new FleetManager());
    newSvcMgr.Register(new FactoryService());
    newSvcMgr.Register(new PaperDollService());
    newSvcMgr.Register(new CorpFittingMgr());
    newSvcMgr.Register(new IndexManager());
    newSvcMgr.Register(new InvBrokerService(newSvcMgr));
    newSvcMgr.Register(new KeeperService(newSvcMgr));
    newSvcMgr.Register(new PlanetMgrService(newSvcMgr));
    newSvcMgr.Register(new EntityService(newSvcMgr));
    newSvcMgr.Register(new FleetProxy());
    newSvcMgr.Register(new DogmaService());
    newSvcMgr.Register(new BeyonceService(newSvcMgr));
    newSvcMgr.Register(new DungeonService());
    newSvcMgr.Register(new LookupService());
    newSvcMgr.Register(new MapService());
    newSvcMgr.Register(new CalendarMgrService());
    newSvcMgr.Register(new AgentMgrService(newSvcMgr));
    newSvcMgr.Register(new ScanMgrService(newSvcMgr));
    newSvcMgr.Register(new ShipService(newSvcMgr));
    newSvcMgr.Register(new SkillMgrService(newSvcMgr));
    newSvcMgr.Register(new TradeService(newSvcMgr));
    newSvcMgr.Register(new PlanetORB(newSvcMgr));
    newSvcMgr.Register(new CorpMgrService());
    newSvcMgr.Register(new CalendarProxy());
    newSvcMgr.Register(new AccountService());
    newSvcMgr.Register(new BulkMgrService());
    newSvcMgr.Register(new LPService());
    newSvcMgr.Register(new ConfigService());
    newSvcMgr.Register(new PhotoUploadService());
    newSvcMgr.Register(new AggressionMgrService(newSvcMgr));
    newSvcMgr.Register(new BillMgr());
    newSvcMgr.Register(new MissionMgrService());
    newSvcMgr.Register(new NotificationMgrService());
    newSvcMgr.Register(new DungeonExplorationMgrService());
    newSvcMgr.Register(new DevToolsProviderService());
    newSvcMgr.Register(new ClientStatsMgr());
    newSvcMgr.Register(new ReprocessingService(newSvcMgr));
    newSvcMgr.Register(new HoloscreenMgrService());
    newSvcMgr.Register(new Standing());
    newSvcMgr.Register(new TutorialService());
    newSvcMgr.Register(new VoucherService(newSvcMgr));
    newSvcMgr.Register(new encounterSpawnServer());
    newSvcMgr.Register(new zActionServer());
    newSvcMgr.Register(new RepairService(newSvcMgr));
    newSvcMgr.Register(new CorporationService());
    newSvcMgr.Register(new LanguageService());
    newSvcMgr.Register(new LocalizationServerService());
    newSvcMgr.Register(new LPStore());
    newSvcMgr.Register(new OnlineStatusService());
    newSvcMgr.Register(new Search());
    newSvcMgr.Register(new SovereigntyMgrService());
    newSvcMgr.Register(new VoiceMgrService());
    newSvcMgr.Register(new ObjCacheService(sConfig.files.cacheDir.c_str()));
    newSvcMgr.Register(new NetService(newSvcMgr));
    newSvcMgr.Register(new StationService());
    newSvcMgr.Register(new StationSvc(newSvcMgr));
    newSvcMgr.Register(new WormHoleSvc());
    newSvcMgr.Register(new netStateServer());
    newSvcMgr.Register(new UserService());
    newSvcMgr.Register(new MovementService(&newSvcMgr));
    newSvcMgr.Register(new InfoGatheringMgr());
    newSvcMgr.Register(new PetitionerService());
    newSvcMgr.Register(new ClientStatLogger());
    newSvcMgr.Register(new AlertService());
    newSvcMgr.Register(new BrowserLockdownService());
    newSvcMgr.Register(new AuthService());
    newSvcMgr.Register(new WarRegistryService(newSvcMgr));
    newSvcMgr.Register(new FactionWarMgrService(newSvcMgr));
    newSvcMgr.Register(new CertificateMgrService(newSvcMgr));
    newSvcMgr.Register(new MarketProxyService(newSvcMgr));
    newSvcMgr.Register(new CharUnboundMgrService(newSvcMgr));
    newSvcMgr.Register(new InsuranceService(newSvcMgr));
    newSvcMgr.Register(new AllianceRegistry(newSvcMgr));
    newSvcMgr.Register(new DogmaIMService(newSvcMgr));
    newSvcMgr.Register(new CorpRegistryService(newSvcMgr));

    // keep a reference to cache in the old manager so it still works
    // TODO: REMOVE ONCE THE CHANGES ARE DONE
    sEntityList.SetService(&newSvcMgr);
    std::printf("\n");     // spacer

    sLog.Blue("  Service Manager", "Service Manager Initialized.");
    /* create the BubbleManager singleton */
    sLog.Green("       ServerInit", "Starting Bubble Manager");
    sBubbleMgr.Initialize();
    /* create the StandingMgr singleton */
    sLog.Green("       ServerInit", "Starting Standings Manager");
    sStandingMgr.Initialize();
    /* create the FleetService singleton */
    sLog.Green("       ServerInit", "Starting Fleet Services");
    sFltSvc.Initialize(newSvcMgr);
    /* create the MarketMgr singleton */
    sLog.Green("       ServerInit", "Starting Market Manager");
    sMktMgr.Initialize(newSvcMgr);
    sLog.Green("       ServerInit", "Starting Statistics Manager");
    sStatMgr.Initialize();
    /* create console command interperter singleton */
    sLog.Green("       ServerInit", "Starting Console Manager");
    sConsole.Initialize(&command_dispatcher);
    std::printf("\n");     // spacer

    sLog.Blue("     ServerConfig", "Cosmic Manager Settings");
    if (sConfig.cosmic.CiviliansEnabled) {
        sLog.Green(" Civilian Manager", "Civilian Manager Enabled.");
        /* create the CivilianMgr singleton */
        sLog.Green("       ServerInit", "Starting Civilian Manager");
        sCivMgr.Initialize();
    } else {
        sLog.Warning(" Civilian Manager", "Civilian Manager Disabled.");
    }
    if (sConfig.cosmic.WormHoleEnabled) {
        sLog.Green(" Wormhole Manager", "Wormhole Manager Enabled.");
        /* create the WormholeMgr singleton */
        sLog.Green("       ServerInit", "Starting Wormhole Manager");
        sWHMgr.Initialize();
    } else {
        sLog.Warning(" Wormhole Manager", "Wormhole Manager Disabled.");
    }
    std::printf("\n");     // spacer

    // Create In-Memory Database Objects for Critical and High-Use Systems:
    sLog.Yellow("       ServerInit", "Loading Static Database Table Objects...");
    std::printf("\n");     // spacer
    /** @note  this is NOT used correctly yet...  */
    //sLog.Green("       ServerInit", "Priming cached objects.");
    //pyServMgr.cache_service->PrimeCache();
    sLog.Green("       ServerInit", "Initializing BulkData");
    if (sConfig.server.BulkDataOD) {
        sLog.Yellow("      BulkDataMgr", "PreLoading Disabled. BulkData will load on first call.");
    } else {
        sBulkDB.Initialize();
    }
    std::printf("\n");     // spacer

    sLog.Green("       ServerInit", "Loading Data Sets");
    sDataMgr.Initialize();
    std::printf("\n");     // spacer
    sMissionDataMgr.Initialize();
    std::printf("\n");     // spacer
    sFxDataMgr.Initialize();
    std::printf("\n");     // spacer
    sMapData.Initialize();
    std::printf("\n");     // spacer
    sDunDataMgr.Initialize();
    std::printf("\n");     // spacer
    sPlanetDataMgr.Initialize();
    std::printf("\n");     // spacer
    sPIDataMgr.Initialize();
    std::printf("\n");     // spacer
    stDataMgr.Initialize();
    std::printf("\n");     // spacer
    svDataMgr.Initialize();
    std::printf("\n");     // spacer

    // clear dynamic system data (player counts, etc) on server start
    MapDB::SystemStartup();
    sLog.Green("       ServerInit", "Dynamic System Data Reset.");

    //sLog.Warning("server init", "Adding NPC Market Orders.");
    //NPCMarket::CreateNPCMarketFromFile("/etc/npcMarket.xml");

    /* program events system */
    SetupSignals();

    /*
    #ifndef _WIN32 // Windows

    ///- Handle affinity for multiple processors and process priority
    uint32 affinity = sConfigMgr->GetIntDefault("UseProcessors", 0);
    bool highPriority = sConfigMgr->GetBoolDefault("ProcessPriority", false);

    if (affinity > 0) {
        cpu_set_t mask;
        CPU_ZERO(&mask);

        for (unsigned int i = 0; i < sizeof(affinity) * 8; ++i)
            if (affinity & (1 << i))
                CPU_SET(i, &mask);

        if (sched_setaffinity(0, sizeof(mask), &mask))
            sLog->outError("Can't set used processors (hex): %x, error: %s", affinity, strerror(errno));
        else {
            CPU_ZERO(&mask);
            sched_getaffinity(0, sizeof(mask), &mask);
            sLog->outString("Using processors (bitmask, hex): %lx", *(__cpu_mask*)(&mask));
        }
    }

    if (highPriority) {
        if (setpriority(PRIO_PROCESS, 0, PROCESS_HIGH_PRIORITY))
            sLog->outError("Can't set worldserver process priority class, error: %s", strerror(errno));
        else
            sLog->outString("worldserver process priority class set to %i", getpriority(PRIO_PROCESS, 0));
    }

    #endif
    */

    uint32 start(0);
    EVETCPConnection* tcpc(nullptr);

    if (sConfig.debug.UseProfiling) {
        // display startup data
        sProfiler.PrintStartUpData();
        // clear profile data from server startup
        sProfiler.ClearAll();
        sLog.Green(" Server Profiling","Profile Data Reset.");
    }
    std::printf("\n");     // spacer

    sLog.Blue("       ServerInit", "Server Initialized in %.3f Seconds.", (GetTimeMSeconds() - profileStartTime) / 1000);
    sLog.Error("       ServerInit", "Main Loop Starting.");

    ServiceDB::SetServerOnlineStatus(true);
    sLog.Green("       ServerInit", "EVEmu Server is Online.");

    sLog.Cyan("           Server", "Started on %s", currentDateTime().c_str());

    /////////////////////////////////////////////////////////////////////////////////////
    //     !!!  DO NOT PUT ANY INITIALIZATION CODE OR CALLS BELOW THIS LINE   !!!
    /////////////////////////////////////////////////////////////////////////////////////

    /*
     * THE MAIN LOOP
     * Everything except IO should happen in this loop, in this thread context.
     */
    while (m_run) {
        Timer::SetCurrentTime();
        start = GetTickCount();

        sAllocators.tickAllocator.Reset();

        /* Freeze Detector Code */
        //++m_worldLoopCounter;

        if ((tcpc = tcps.PopConnection()))
            sEntityList.Add(new Client(newSvcMgr, &tcpc));

        sEntityList.Process();

        /*  process console commands, if any, and check for 'exit' command */
        m_run = sConsole.Process();

        /* do the stuff for thread sleeping */
        start = GetTickCount() - start;
        if (m_sleepTime > start)
            std::this_thread::sleep_for(std::chrono::milliseconds(start));
    }

    /*
     * end of main loop
     *  at this point, server has been killed, and these are cleanup methods below here
     */

    /** @todo  update this to have a ShutDown() method, with these items.
     * also look into calling it when a signal is caught, for cleanup.
     * @note  these are order-dependent...
     */
    sLog.Warning("   ServerShutdown", "Main loop has stopped." );
    sLog.Error("   ServerShutdown", "EVEmu Server is Offline." );
    if (!sConsole.IsDbError())
        ServiceDB::SetServerOnlineStatus(false);
    /* stop TCP listener */
    tcps.Close();
    sLog.Warning("   ServerShutdown", "TCP listener stopped." );
    /* stop Image Server */
    sImageServer.Stop();
    sLog.Warning("   ServerShutdown", "Image Server stopped." );
    /* Close the MarketMgr */
    sMktMgr.Close();
    /* Close the bulk data manager */
    sBulkDB.Close();
    /* Close the station data manager */
    stDataMgr.Close();
    /* Close the map data manager */
    sMapData.Close();
    /* Close the static data manager */
    sDataMgr.Close();
    /* Close the statistics manager */
    sStatMgr.Close();
    /* Close the standings manager */
    sStandingMgr.Close();
    sLog.Warning("   ServerShutdown", "Saving Items." );
    if (!sConsole.IsDbError())
        sItemFactory.SaveItems();
    /* Close the entity list */
    sEntityList.Close();
    /* Shut down the Item system */
    sLog.Warning("   ServerShutdown", "Shutting down Item Factory." );
    sItemFactory.Close();
    /* Close the bubble manager */
    sBubbleMgr.clear();
    /* Close the command dispatcher */
    command_dispatcher.Close();
    /* Stop Console Command Interpreter */
    //sConsole.Stop();
    /* close the db handler */
    sLog.Warning("   ServerShutdown", "Closing DataBase Connection." );
    sDatabase.Close();
    /** @todo  the thread system is only implemented for tcp connections at this time. */
    sLog.Warning("   ServerShutdown", "Shutting down Thread Manager." );
    /* join open threads */
    sThread.EndThreads();
    sLog.Warning("   ServerShutdown", "EVEmu is Offline.");
    /* close logfile */
    log_close_logfile();
    exit(EXIT_SUCCESS);
}

static void SetupSignals()
{
    /* setup sigaction to prevent zombies and catch other non-fatal signals */
#ifdef _WIN32

#else
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_NOCLDWAIT;
    if (sigemptyset(&sa.sa_mask) == -1 ) {  /* MT safe */
        perror("SigEmptySet Failure");
        exit(EXIT_FAILURE);     /* NOT MT safe */
    }
    if (sigaction(SIGCHLD, &sa, nullptr) == -1) {  /* MT safe */
        perror("SigAction Failure");
        exit(EXIT_FAILURE);     /* NOT MT safe */
    }
    if (sigaction(SIGPIPE, &sa, nullptr) == -1) {  /* MT safe */
        // ignore broken pipe signal.  db code will auto-recover.
        perror("SigPipe Failure");
        return;
    }
#endif

    //::signal( SIGPIPE, SIG_IGN );
    //::signal( SIGCHLD, SIG_IGN );
    ::signal( SIGINT, CatchSignal );
    ::signal( SIGTERM, CatchSignal );
    ::signal( SIGABRT, CatchSignal );
    //::signal( SIGSEGV, CatchSignal );

    #ifdef SIGABRT_COMPAT
    ::signal( SIGABRT_COMPAT, CatchSignal );
    #endif /* SIGABRT_COMPAT */

    #ifdef SIGBREAK
    ::signal( SIGBREAK, CatchSignal );
    #endif /* SIGBREAK */

    #ifdef SIGHUP
    ::signal( SIGHUP, CatchSignal );
    #endif /* SIGHUP */
}

static void CatchSignal( int sig_num )
{
    sLog.Error( "    Signal System", "Caught signal: %d", sig_num );
    if (sConfig.debug.StackTrace)
        EvE::traceStack();
    //SafeSave();
    m_run = false;
    //CleanUp();
}

static void CleanUp() {
    sLog.Warning("   ServerShutdown", "Main loop has stopped." );
    sLog.Error("   ServerShutdown", "EVEmu Server is Offline." );
    if (!sConsole.IsDbError())
        ServiceDB::SetServerOnlineStatus(false);
    /* stop TCP listener */
    //tcps.Close();
    sLog.Warning("   ServerShutdown", "TCP listener stopped." );
    /* stop Image Server */
    sImageServer.Stop();
    sLog.Warning("   ServerShutdown", "Image Server stopped." );
    /* Close the MarketMgr */
    sLog.Warning("   ServerShutdown", "Shutting down Market Manager." );
    sMktMgr.Close();
    /* Close the bulk data manager */
    sLog.Warning("   ServerShutdown", "Closing the BulkData Manager." );
    sBulkDB.Close();
    /* Close the station data manager */
    sLog.Warning("   ServerShutdown", "Closing the StationData Manager." );
    stDataMgr.Close();
    /* Close the static data manager */
    sLog.Warning("   ServerShutdown", "Closing the StaticData Manager." );
    sDataMgr.Close();
    sStatMgr.Close();
    sStandingMgr.Close();
    sLog.Warning("   ServerShutdown", "Saving Items." );
    if (!sConsole.IsDbError())
        sItemFactory.SaveItems();
    /* Close the entity list */
    sLog.Warning("   ServerShutdown", "Closing the Entity List." );
    sEntityList.Close();
    /* Close the service manager */
    sLog.Warning("   ServerShutdown", "Closing the Services Manager." );
    //pyServMgr.Close();
    /* Shut down the Item system */
    sLog.Warning("   ServerShutdown", "Shutting down Item Factory." );
    sItemFactory.Close();
    sLog.Warning("   ServerShutdown", "Closing the Bubble Manager." );
    sBubbleMgr.clear();
    /* Close the command dispatcher */
    //command_dispatcher.Close();
    /* Stop Console Command Interpreter */
    //sConsole.Stop();
    /* close the db handler */
    sLog.Warning("   ServerShutdown", "Closing DataBase Connection." );
    sDatabase.Close();
    /** @todo  the thread system is only implemented for tcp connections at this time. */
    sLog.Warning("   ServerShutdown", "Shutting down Thread Manager." );
    /* join open threads */
    sThread.EndThreads();
    sLog.Warning("   ServerShutdown", "EVEmu is Offline.");
    /* close logfile */
    log_close_logfile();
}

/*      Freeze Detector Code taken from TrinityCore.  figure out how to implement here (based on seeing occasional freezes on main)  -allan 29Dec15
 * void FreezeDetectorHandler(const boost::system::error_code& error)
 * {
 *    if (!error)
 *    {
 *        uint32 curtime = getMSTime();
 *
 *        uint32 worldLoopCounter = World::m_worldLoopCounter;
 *        if (_worldLoopCounter != worldLoopCounter)
 *        {
 *            _lastChangeMsTime = curtime;
 *            _worldLoopCounter = worldLoopCounter;
 *        }
 *        // possible freeze
 *        else if (getMSTimeDiff(_lastChangeMsTime, curtime) > _maxCoreStuckTimeInMs)
 *        {
 *            TC_LOG_ERROR("server.worldserver", "World Thread hangs, kicking out server!");
 *            ASSERT(false);
 *        }
 *
 *        _freezeCheckTimer.expires_from_now(boost::posix_time::seconds(1));
 *        _freezeCheckTimer.async_wait(FreezeDetectorHandler);
 *    }
 * }  */
