/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/



#include "../common/common.h"

#include <signal.h>

#include "../common/EVETCPServer.h"
#include "../common/logsys.h"
#include "../common/EVETCPConnection.h"
#include "../common/EVEPresentation.h"
#include "../common/timer.h"
#include "../common/PyRep.h"
#include "../common/dbcore.h"
#include "../common/EVEmuRevision.h"
#include "../common/EVEVersion.h"
#include "PyServiceMgr.h"
#include "EVEmuServerConfig.h"


#include "EntityList.h"
#include "Client.h"

//services:
#include "account/AccountService.h"
#include "missions/AgentMgrService.h"
#include "missions/MissionMgrService.h"
#include "admin/AlertService.h"
#include "account/AuthService.h"
#include "market/BillMgrService.h"
#include "BookmarkService.h"
#include "character/CharacterService.h"
#include "character/CharMgrService.h"
#include "character/SkillMgrService.h"
#include "config/ConfigService.h"
#include "config/LanguageService.h"
#include "corporation/CorpMgrService.h"
#include "corporation/CorpStationMgrService.h"
#include "corporation/CorporationService.h"
#include "corporation/CorpRegistryService.h"
#include "dogmaim/DogmaIMService.h"
#include "inventory/InvBrokerService.h"
#include "chat/LSCService.h"
#include "chat/LookupService.h"
#include "admin/AllCommands.h"
#include "admin/CommandDispatcher.h"
#include "admin/CommandDB.h"
#include "admin/PetitionerService.h"
#include "admin/SlashService.h"
#include "ship/ShipService.h"
#include "ship/InsuranceService.h"
#include "ship/BeyonceService.h"
#include "map/MapService.h"
#include "cache/ObjCacheService.h"
#include "chat/OnlineStatusService.h"
#include "standing/Standing2Service.h"
#include "station/StationService.h"
#include "station/StationSvcService.h"
#include "station/JumpCloneService.h"
#include "system/KeeperService.h"
#include "system/DungeonService.h"
#include "market/MarketProxyService.h"
#include "tutorial/TutorialService.h"
#include "mining/ReprocessingService.h"
#include "manufacturing/FactoryService.h"
#include "manufacturing/RamProxyService.h"

//int catch_segv;

#define IMPLEMENT_SIGEXCEPT
#include "../common/sigexcept/sigexcept.h"

static void CatchSignal(int sig_num);
static bool InitSignalHandlers();

static volatile bool RunLoops = true;

int main(int argc, char *argv[]) {
	INIT_SIGEXCEPT();
	
	_log(SERVER__INIT, "EVEmu %s", EVEMU_REVISION);
	_log(SERVER__INIT, " Supported Client: %s, Version %f, Build %d, MachoNet %d",
		EVEProjectVersion, EVEVersionNumber, EVEBuildVersion, MachoNetVersion);

	//it is important to do this before doing much of anything, in case they use it.
	Timer::SetCurrentTime();
	
	// Load server configuration
	_log(SERVER__INIT, "Loading server configuration..");
	if (!EVEmuServerConfig::LoadConfig()) {
		_log(SERVER__INIT_ERR, "Loading server configuration failed.");
		return(1);
	}
	const EVEmuServerConfig *Config=EVEmuServerConfig::get();

	if(!load_log_settings(Config->LogSettingsFile.c_str()))
		_log(SERVER__INIT, "Warning: Unable to read %s (this file is optional)", Config->LogSettingsFile.c_str());
	else
		_log(SERVER__INIT, "Log settings loaded from %s", Config->LogSettingsFile.c_str());

	//open up the log file if specified.
	if(!Config->LogFile.empty()) {
		if(log_open_logfile(Config->LogFile.c_str())) {
			_log(SERVER__INIT, "Opened log file %s", Config->LogFile.c_str());
		} else {
			_log(SERVER__INIT_ERR, "Unable to open log file '%s', only logging to the screen now.", Config->LogFile.c_str());
		}
	}
	
	if(!PyRepString::LoadStringFile(Config->StringsFile.c_str())) {
		_log(SERVER__INIT_ERR, "Unable to open %s, i need it to decode string table elements!", Config->StringsFile.c_str());
		return(1);
	}

	//connect to the database...
	DBcore db;
	{
		DBerror err;
		if(!db.Open(err, 
			Config->DatabaseHost.c_str(),
			Config->DatabaseUsername.c_str(),
			Config->DatabasePassword.c_str(),
			Config->DatabaseDB.c_str(),
			Config->DatabasePort)
		) {
			_log(SERVER__INIT_ERR, "Unable to connect to the database: %s", err.c_str());
			return(1);
		}
	}

	
	//Start up the TCP server
	EVETCPServer tcps;
	
	char errbuf[TCPConnection_ErrorBufferSize];
	if (tcps.Open(Config->ServerPort, errbuf)) {
		_log(SERVER__INIT,"TCP listener started on port %d.", Config->ServerPort);
	} else {
		_log(SERVER__INIT_ERR,"Failed to start TCP listener on port %d:", Config->ServerPort);
		_log(SERVER__INIT_ERR,"        %s",errbuf);
		return 1;
	}
	
	EntityList entity_list(&db);
	ItemFactory item_factory(&db, &entity_list);


	//now, the service manager...
	PyServiceMgr services(888444, &db, &entity_list, &item_factory, Config->CacheDirectory);

	//setup the command dispatcher
	CommandDispatcher command_dispatcher(new CommandDB(&db), &services);
	RegisterAllCommands(&command_dispatcher);

	/*                                                                              
     * Service creation and registration.
     *
     */
	_log(SERVER__INIT, "Creating services.");
	services.RegisterService(new AgentMgrService(&services, &db));
	services.RegisterService(new MissionMgrService(&services, &db));
	services.RegisterService(new AccountService(&services, &db));
	services.RegisterService(new AlertService(&services));
	services.RegisterService(new AuthService(&services));
	services.RegisterService(new BillMgrService(&services, &db));
	services.RegisterService(new BookmarkService(&services));
	services.RegisterService(new CharacterService(&services, &db));
	services.RegisterService(new CharMgrService(&services, &db));
	services.RegisterService(new ConfigService(&services, &db));
	services.RegisterService(new LanguageService(&services, &db));
	services.RegisterService(new CorpMgrService(&services, &db));
	services.RegisterService(new CorpStationMgrService(&services, &db));
	services.RegisterService(new CorporationService(&services, &db));
	services.RegisterService(new CorpRegistryService(&services, &db));
	services.RegisterService(new DogmaIMService(&services, &db));
	services.RegisterService(new InvBrokerService(&services, &db));
	services.RegisterService(services.lsc_service = new LSCService(&services, &db, &command_dispatcher));
	services.RegisterService(new LookupService(&services, &db));
	services.RegisterService(new ShipService(&services, &db));
	services.RegisterService(new InsuranceService(&services, &db));
	services.RegisterService(new BeyonceService(&services, &db));
	services.RegisterService(new MapService(&services, &db));
	services.RegisterService(new OnlineStatusService(&services));
	services.RegisterService(new Standing2Service(&services, &db));
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
	services.RegisterService(new ReprocessingService(&services, &db));
	services.RegisterService(new FactoryService(&services, &db));
	services.RegisterService(new RamProxyService(&services, &db));
	
	_log(SERVER__INIT, "Priming cached objects");
//#ifndef WIN32
//#warning CACHABLES DISABLED!
	services.GetCache()->PrimeCache();
//#endif

	// johnsus - serverStartTime mod
	services.GetServiceDB()->SetServerOnlineStatus(true);

	_log(SERVER__INIT, "Init done.");

	while(argc > 1) {
		uint32 sys = strtoul(argv[1], NULL, 0);
		if(sys > 0) {
			_log(SERVER__INIT, "Command line tells us to boot system %lu", sys);
			entity_list.FindOrBootSystem(sys);
		}
		argc--;
		argv++;
	}

	
	/*
	 * THE MAIN LOOP
	 *
	 * Everything except IO should happen in this loop, in this thread context.
	 *
	 */
	if(!InitSignalHandlers())	//do not set these up until the main loop is about to start, else we cannot abort init.
		return(1);
	EVETCPConnection *tcpc;
	while(RunLoops) {
		Timer::SetCurrentTime();
		
		//check for timeouts in other threads
		//timeout_manager.CheckTimeouts();
		
		
		while ((tcpc = tcps.NewQueuePop())) {
			struct in_addr in;
			in.s_addr = tcpc->GetrIP();
			_log(SERVER__CLIENTS, "New TCP connection from %s:%d", inet_ntoa(in),tcpc->GetrPort());
			EVEPresentation *p = new EVEPresentation(tcpc);
			Client *c = new Client(&services, &p);
			
			c->SendHandshake();
			
			entity_list.Add(&c);
		}
		
		entity_list.Process();
		services.Process();
		
		/*if (numclients == 0) {
			Sleep(50);
			continue;
		}*/
		Sleep(3);	//this should be a parameter
	}
	_log(SERVER__SHUTDOWN,"main loop stopped");
	_log(SERVER__SHUTDOWN,"TCP listener stopped.");
	tcps.Close();
	
	// johnsus - serverStartTime mod
	services.GetServiceDB()->SetServerOnlineStatus(false);

	//TODO: properly free physics driver

	return(0);
}

static bool InitSignalHandlers() {
	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		_log(SERVER__INIT_ERR, "Could not set signal handler");
		return(false);
	}
	if (signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		_log(SERVER__INIT_ERR, "Could not set signal handler");
		return(false);
	}
	#ifndef WIN32
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)	{
		_log(SERVER__INIT_ERR, "Could not set signal handler");
		return(false);
	}
	#endif
	return(true);
}

static void CatchSignal(int sig_num) {
	_log(SERVER__SHUTDOWN,"Caught signal %d",sig_num);
	RunLoops = false;
}

