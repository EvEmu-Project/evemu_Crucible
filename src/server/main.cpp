/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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
	Author:		Zhur
*/

#include "EvemuPCH.h"
#include "EVEVersion.h"
#include "../server/account/ClientStatMgrService.h"

#include <signal.h>

//#define USE_RUNTIME_EXCEPTIONS 1
//#define IMPLEMENT_SIGEXCEPT 1
//#include "../common/sigexcept/sigexcept.h"

static void CatchSignal(int sig_num);
static bool InitSignalHandlers();

// global database hook.
DBcore * general_database;

static volatile bool RunLoops = true;

int main(int argc, char *argv[]) {
	//INIT_SIGEXCEPT();

	_log(SERVER__INIT, "EVEmu %s", EVEMU_REVISION);
	_log(SERVER__INIT, " Supported Client: %s, Version %.2f, Build %d, MachoNet %d",
		EVEProjectVersion, EVEVersionNumber, EVEBuildVersion, MachoNetVersion);

	//it is important to do this before doing much of anything, in case they use it.
	Timer::SetCurrentTime();
	
	// Load server configuration
	_log(SERVER__INIT, "Loading server configuration...");
	if (!sConfig.ParseFile("evemuserver.xml", "eve")) {
		_log(SERVER__INIT_ERR, "Loading server configuration failed.");
		return(1);
	}

	if(!load_log_settings(sConfig.files.logSettings.c_str()))
		_log(SERVER__INIT, "Warning: Unable to read %s (this file is optional)", sConfig.files.logSettings.c_str());
	else
		_log(SERVER__INIT, "Log settings loaded from %s", sConfig.files.logSettings.c_str());

	//open up the log file if specified.
	if(!sConfig.files.log.empty()) {
		if(log_open_logfile(sConfig.files.log.c_str())) {
			_log(SERVER__INIT, "Opened log file %s", sConfig.files.log.c_str());
		} else {
			_log(SERVER__INIT_ERR, "Unable to open log file '%s', only logging to the screen now.", sConfig.files.log.c_str());
		}
	}

	if(!PyRepString::LoadStringFile(sConfig.files.strings.c_str())) {
		_log(SERVER__INIT_ERR, "Unable to open %s, i need it to decode string table elements!", sConfig.files.strings.c_str());
		return(1);
	}

	//connect to the database...
	DBcore db;
	{
		DBerror err;
		if(!db.Open(err, 
			sConfig.database.host.c_str(),
			sConfig.database.username.c_str(),
			sConfig.database.password.c_str(),
			sConfig.database.db.c_str(),
			sConfig.database.port)
		) {
			_log(SERVER__INIT_ERR, "Unable to connect to the database: %s", err.c_str());
			return(1);
		}
	}

	general_database = &db;
	
	//Start up the TCP server
	EVETCPServer tcps;
	
	char errbuf[TCPConnection_ErrorBufferSize];
	if (tcps.Open(sConfig.server.port, errbuf)) {
		_log(SERVER__INIT,"TCP listener started on port %d.", sConfig.server.port);
	} else {
		_log(SERVER__INIT_ERR,"Failed to start TCP listener on port %d:", sConfig.server.port);
		_log(SERVER__INIT_ERR,"        %s",errbuf);
		return 1;
	}
	
	EntityList entity_list(db);
	ItemFactory item_factory(db, entity_list);

	//now, the service manager...
	PyServiceMgr services(888444, db, entity_list, item_factory);

	//setup the command dispatcher
	CommandDispatcher command_dispatcher(services, db);
	RegisterAllCommands(command_dispatcher);

	/*                                                                              
	 * Service creation and registration.
	 *
	 */
	_log(SERVER__INIT, "Creating services.");
	
	services.RegisterService(new ClientStatsMgr(&services));
	services.RegisterService(new AgentMgrService(&services, &db));
	services.RegisterService(new MissionMgrService(&services, &db));
	services.RegisterService(new AccountService(&services, &db));
	services.RegisterService(new UserService(&services));
	services.RegisterService(new AlertService(&services));
	services.RegisterService(new AuthService(&services));
	services.RegisterService(new BillMgrService(&services, &db));
	services.RegisterService(new BookmarkService(&services, &db));
	services.RegisterService(new CertificateMgrService(&services, &db));
	services.RegisterService(new CharacterService(&services, &db));
	services.RegisterService(new CharMgrService(&services, &db));
	services.RegisterService(new ConfigService(&services, &db));
	services.RegisterService(new LanguageService(&services, &db));
	services.RegisterService(new CorpMgrService(&services, &db));
	services.RegisterService(new CorpStationMgrService(&services, &db));
	services.RegisterService(new CorporationService(&services, &db));
	services.RegisterService(new CorpRegistryService(&services, &db));
	services.RegisterService(new LPService(&services));
	services.RegisterService(new DogmaIMService(&services, &db));
	services.RegisterService(new InvBrokerService(&services));
	services.RegisterService(services.lsc_service = new LSCService(&services, &db, &command_dispatcher));
	services.RegisterService(services.cache_service = new ObjCacheService(&services, &db, sConfig.files.cacheDir.c_str()));
	services.RegisterService(new LookupService(&services, &db));
	services.RegisterService(new VoiceMgrService(&services));
	services.RegisterService(new ShipService(&services, &db));
	services.RegisterService(new InsuranceService(&services, &db));
	services.RegisterService(new BeyonceService(&services, &db));
	services.RegisterService(new MapService(&services, &db));
	services.RegisterService(new OnlineStatusService(&services));
	services.RegisterService(new Standing2Service(&services, &db));
	services.RegisterService(new WarRegistryService(&services));
	services.RegisterService(new FactionWarMgrService(&services, &db));
	services.RegisterService(new StationService(&services, &db));
	services.RegisterService(new StationSvcService(&services, &db));
	services.RegisterService(new JumpCloneService(&services, &db));
	services.RegisterService(new KeeperService(&services, &db));
	services.RegisterService(new DungeonService(&services, &db));
	services.RegisterService(new SkillMgrService(&services, &db));
	services.RegisterService(new TutorialService(&services, &db));
	services.RegisterService(new PetitionerService(&services));
	services.RegisterService(new SlashService(&services, &command_dispatcher));
	services.RegisterService(new MarketProxyService(&services, &db));
	services.RegisterService(new ContractMgrService(&services));
	services.RegisterService(new ReprocessingService(&services, &db));
	services.RegisterService(new FactoryService(&services, &db));
	services.RegisterService(new RamProxyService(&services, &db));
	services.RegisterService(new PosMgrService(&services, &db));
	services.RegisterService(new NetService(&services));
	
	_log(SERVER__INIT, "Priming cached objects");
	services.cache_service->PrimeCache();

	services.serviceDB().SetServerOnlineStatus(true);

	_log(SERVER__INIT, "Init done.");

	/*
	 * THE MAIN LOOP
	 *
	 * Everything except IO should happen in this loop, in this thread context.
	 *
	 */
	if(InitSignalHandlers() == false)
		return 1;

	uint32 start;
	uint32 etime;
	uint32 last_time = GetTickCount();
	uint32 server_main_loop_delay = 10; // delay 10 ms.

	EVETCPConnection *tcpc;
	while(RunLoops)
	{
		Timer::SetCurrentTime();
		start = GetTickCount();
		
		//check for timeouts in other threads
		//timeout_manager.CheckTimeouts();
		while ((tcpc = tcps.NewQueuePop())) 
		{
			std::string connectionAddress = tcpc->GetAddress();
			_log(SERVER__CLIENTS, "New TCP connection from %s", connectionAddress.c_str());
			Client *c = new Client(services, tcpc);
					
			entity_list.Add(&c);
		}
		
		entity_list.Process();
		services.Process();

		/* UPDATE */
		last_time = GetTickCount();
		etime = last_time - start;

		// do the stuff for thread sleeping
		if( server_main_loop_delay > etime )
			Sleep( server_main_loop_delay - etime );
	}

	_log(SERVER__SHUTDOWN,"main loop stopped");
	_log(SERVER__SHUTDOWN,"TCP listener stopped.");
	tcps.Close();

	services.serviceDB().SetServerOnlineStatus(false);

	//_CrtDumpMemoryLeaks();

	return 0;
}

static bool InitSignalHandlers()
{
	signal( SIGINT, CatchSignal );
	signal( SIGTERM, CatchSignal );
	signal( SIGABRT, CatchSignal );
#ifdef _WIN32
	signal( SIGBREAK, CatchSignal );
#else
	signal( SIGHUP, CatchSignal );
	signal( SIGUSR1, CatchSignal );
#endif
	return true;
}

static void CatchSignal(int sig_num) {
	_log(SERVER__SHUTDOWN,"Caught signal: %d", sig_num);
	RunLoops = false;
}
