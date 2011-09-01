/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:     Zhur, mmcs
*/

#include "EVEServerPCH.h"
#include "EVEVersion.h"
#include <iostream>

static void SetupSignals();
static void CatchSignal( int sig_num );

static const char* const CONFIG_FILE = EVEMU_ROOT_DIR"etc/eve-server.xml";
static const uint32 MAIN_LOOP_DELAY = 10; // delay 10 ms.

static volatile bool RunLoops = true;
dgmtypeattributemgr * _sDgmTypeAttrMgr;

int main( int argc, char* argv[] )
{
#if defined( MSVC ) && !defined( NDEBUG )
    // Under Visual Studio setup memory leak detection
    _CrtSetDbgFlag( _CRTDBG_LEAK_CHECK_DF | _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG ) );
#endif /* defined( MSVC ) && !defined( NDEBUG ) */

    printf("Copyright (C) 2006-2011 EVEmu Team. http://evemu.org/\n");
    printf("This program is free software; you can redistribute it and/or modify it under\n");
    printf("the terms of the GNU Lesser General Public License as published by the Free \n");
    printf("Software Foundation; either version 2 of the License, or (at your option) any\n");
    printf("later version.\n");
    printf("\n");
    printf("This program is distributed in the hope that it will be useful, but WITHOUT\n");
    printf("ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS\n");
    printf("FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more\n");
    printf("details.\n");
    printf("\n");

    // Load server configuration
    if( !sConfig.ParseFile( CONFIG_FILE ) )
    {
		printf("ERROR: Loading server configuration '%s' failed.", CONFIG_FILE );
        //sLog.Error( "server init", "Loading server configuration '%s' failed.", CONFIG_FILE );
        std::cout << std::endl << "press any key to exit...";  std::cin.get();
        return 1;
    }

	sLog.InitializeLogging(sConfig.files.logDir);
    sLog.Log("server init", "Loading server configuration...");

	sLog.Log("main", "EVEmu "EVEMU_VERSION );
    sLog.Log("server init", "\n"
        "\tSupported Client: %s\n"
        "\tVersion %.2f\n"
        "\tBuild %d\n"
        "\tMachoNet %u",
        EVEProjectVersion,
        EVEVersionNumber,
        EVEBuildVersion,
        MachoNetVersion
    );

    //it is important to do this before doing much of anything, in case they use it.
    Timer::SetCurrentTime();

    // Load server log settings ( will be removed )
    if( load_log_settings( sConfig.files.logSettings.c_str() ) )
        sLog.Success( "server init", "Log settings loaded from %s", sConfig.files.logSettings.c_str() );
    else
        sLog.Warning( "server init", "Unable to read %s (this file is optional)", sConfig.files.logSettings.c_str() );

    // open up the log file if specified ( will be removed )
    if( !sConfig.files.logDir.empty() )
    {
		std::string logFile = sConfig.files.logDir + "eve-server.log";
        if( log_open_logfile( logFile.c_str() ) )
            sLog.Success( "server init", "Found log directory %s", sConfig.files.logDir.c_str() );
        else
            sLog.Warning( "server init", "Unable to find log directory '%s', only logging to the screen now.", sConfig.files.logDir.c_str() );
    }

    //connect to the database...
    DBerror err;
    if( !sDatabase.Open( err,
        sConfig.database.host.c_str(),
        sConfig.database.username.c_str(),
        sConfig.database.password.c_str(),
        sConfig.database.db.c_str(),
        sConfig.database.port ) )
    {
        sLog.Error( "server init", "Unable to connect to the database: %s", err.c_str() );
        std::cout << std::endl << "press any key to exit...";  std::cin.get();
        return 1;
    }
    _sDgmTypeAttrMgr = new dgmtypeattributemgr(); // needs to be after db init as its using it

    _sDgmTypeAttrMgr = new dgmtypeattributemgr(); // needs to be after db init as its using it

    //Start up the TCP server
    EVETCPServer tcps;

    char errbuf[ TCPCONN_ERRBUF_SIZE ];
    if( tcps.Open( sConfig.net.port, errbuf ) )
    {
        sLog.Success( "server init", "TCP listener started on port %u.", sConfig.net.port );
    }
    else
    {
        sLog.Error( "server init", "Failed to start TCP listener on port %u: %s.", sConfig.net.port, errbuf );
        std::cout << std::endl << "press any key to exit...";  std::cin.get();
        return 1;
    }
	//make the item factory
    ItemFactory item_factory( sEntityList );

    //now, the service manager...
    PyServiceMgr services( 888444, sEntityList, item_factory );

    //setup the command dispatcher
    CommandDispatcher command_dispatcher( services );
    RegisterAllCommands( command_dispatcher );

    /*
     * Service creation and registration.
     *
     */
    sLog.Log("server init", "Creating services.");

    services.RegisterService(new ClientStatsMgr(&services));
    services.RegisterService(new AgentMgrService(&services));
    services.RegisterService(new MissionMgrService(&services));
    services.RegisterService(new AccountService(&services));
    services.RegisterService(new UserService(&services));
    services.RegisterService(new AlertService(&services));
    services.RegisterService(new AuthService(&services));
    services.RegisterService(new BillMgrService(&services));
    services.RegisterService(new BookmarkService(&services));
    services.RegisterService(new BrowserLockDownService(&services));
    services.RegisterService(new CertificateMgrService(&services));
    services.RegisterService(new CharacterService(&services));
    services.RegisterService(new CharMgrService(&services));
    services.RegisterService(new ConfigService(&services));
    services.RegisterService(new LanguageService(&services));
    services.RegisterService(new CorpMgrService(&services));
    services.RegisterService(new CorpStationMgrService(&services));
    services.RegisterService(new CorporationService(&services));
    services.RegisterService(new CorpRegistryService(&services));
    services.RegisterService(new ContractProxyService(&services));
    services.RegisterService(new LPService(&services));
    services.RegisterService(new DogmaIMService(&services));
    services.RegisterService(new InvBrokerService(&services));
    services.RegisterService(services.lsc_service = new LSCService(&services, &command_dispatcher));
    services.RegisterService(services.cache_service = new ObjCacheService(&services, sConfig.files.cacheDir.c_str()));
    services.RegisterService(new LookupService(&services));
    services.RegisterService(new VoiceMgrService(&services));
    services.RegisterService(new ShipService(&services));
    services.RegisterService(new InsuranceService(&services));
    services.RegisterService(new BeyonceService(&services));
    services.RegisterService(new MapService(&services));
    services.RegisterService(new OnlineStatusService(&services));
    services.RegisterService(new Standing2Service(&services));
    services.RegisterService(new WarRegistryService(&services));
    services.RegisterService(new FactionWarMgrService(&services));
    services.RegisterService(new StationService(&services));
    services.RegisterService(new StationSvcService(&services));
    services.RegisterService(new JumpCloneService(&services));
    services.RegisterService(new KeeperService(&services));
    services.RegisterService(new DungeonService(&services));
    services.RegisterService(new SkillMgrService(&services));
    services.RegisterService(new TutorialService(&services));
    services.RegisterService(new PetitionerService(&services));
    services.RegisterService(new SlashService(&services, &command_dispatcher));
    services.RegisterService(new MarketProxyService(&services));
    services.RegisterService(new ContractMgrService(&services));
    services.RegisterService(new ReprocessingService(&services));
    services.RegisterService(new FactoryService(&services));
    services.RegisterService(new RamProxyService(&services));
    services.RegisterService(new PosMgrService(&services));
    services.RegisterService(new NetService(&services));
	services.RegisterService(new TradeService(&services));
	services.RegisterService(new CharUnboundMgrService(&services));
	services.RegisterService(new PaperDollService(&services));
	services.RegisterService(new PhotoUploadService(&services));
	services.RegisterService(new AggressionMgrService(&services));
	services.RegisterService(new SovereigntyMgrService(&services));
	services.RegisterService(new PlanetMgrService(&services));
	services.RegisterService(new CharFittingMgrService(&services));
	services.RegisterService(new DungeonExplorationMgrService(&services));
	services.RegisterService(new FleetProxyService(&services));
	services.RegisterService(new MailMgrService(&services));
	services.RegisterService(new MailingListMgrService(&services));
	services.RegisterService(new NotificationMgrService(&services));

    sLog.Log("server init", "Priming cached objects.");
    services.cache_service->PrimeCache();
    sLog.Log("server init", "finished priming");

	// start up the image server
	sImageServer.Run();

	// start up the api server
    sAPIServer.CreateServices( services );
	sAPIServer.Run();

    services.serviceDB().SetServerOnlineStatus(true);

    sLog.Log("server init", "Init done.");

    /*
     * THE MAIN LOOP
     *
     * Everything except IO should happen in this loop, in this thread context.
     *
     */

    /* program events system */
    SetupSignals();

    uint32 start;
    uint32 etime;
    uint32 last_time = GetTickCount();

    EVETCPConnection* tcpc;
    while( RunLoops == true )
    {
        Timer::SetCurrentTime();
        start = GetTickCount();

        //check for timeouts in other threads
        //timeout_manager.CheckTimeouts();
        while( ( tcpc = tcps.PopConnection() ) )
        {
            Client* c = new Client( services, &tcpc );

            sEntityList.Add( &c );
        }

        sEntityList.Process();
        services.Process();

        /* UPDATE */
        last_time = GetTickCount();
        etime = last_time - start;

        // do the stuff for thread sleeping
        if( MAIN_LOOP_DELAY > etime )
            Sleep( MAIN_LOOP_DELAY - etime );
    }

    sLog.Log("server shutdown", "Main loop stopped" );

    tcps.Close();

    sLog.Log("server shutdown", "TCP listener stopped." );

    services.serviceDB().SetServerOnlineStatus(false);

    sLog.Log("server shutdown", "Cleanup db cache" );
    delete _sDgmTypeAttrMgr;

    log_close_logfile();

    // win crap.
//#if defined( MSVC ) && !defined( NDEBUG )
//    _CrtDumpMemoryLeaks();
//#endif /* defined( MSVC ) && !defined( NDEBUG ) */

    std::cout << std::endl << "press any key to exit...";  std::cin.get();

    return 0;
}

static void SetupSignals()
{
    signal( SIGINT, CatchSignal );
    signal( SIGTERM, CatchSignal );
    signal( SIGABRT, CatchSignal );
#ifdef WIN32
    signal( SIGBREAK, CatchSignal );
    signal( SIGABRT_COMPAT, CatchSignal );
#else
    signal( SIGHUP, CatchSignal );
#endif
}

static void CatchSignal( int sig_num )
{
    sLog.Log( "Signal system", "Caught signal: %d", sig_num );

    RunLoops = false;
}
