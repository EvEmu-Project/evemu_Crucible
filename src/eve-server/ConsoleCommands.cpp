/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:	Allan
    Thanks to:	avianrr  for the idea
*/

#include <ios>
#include <iostream>
#include <fstream>

#include "ConsoleCommands.h"
#include "../eve-common/EVEVersion.h"
#include "StatisticMgr.h"
#include "effects/EffectsProcessor.h"
#include "inventory/ItemDB.h"
#include "market/MarketDB.h"
#include "market/MarketMgr.h"
#include "missions/MissionDataMgr.h"
#include "threading/Threading.h"
// #include "testing/test.h"


ConsoleCommand::ConsoleCommand()
 :plscc(nullptr),
  pBubbles(nullptr),
  pSystems(nullptr),
  pCommand(nullptr),
  buf(nullptr),
  tv(timeval()),
  m_haltServer(false),
  m_dbError(false)
{
}

void ConsoleCommand::Initialize(CommandDispatcher* cd)
{
    pCommand = cd;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    UpdateStatus();	//initial status setting
    sLog.Blue( "   ConsoleCommand", "Console Commands Initialized." );
    sLog.Yellow( "   ConsoleCommand", "Enter 'h' for current list of supported commands." );

    m_inputThread = new std::thread([&] {
        std::string temp;
        while (true) {
            std::getline(std::cin, temp);
            std::lock_guard<std::mutex> lock(m_inputMutex);
            m_input.push_back(std::move(temp));
            m_inputCondition.notify_one();
        }
    });
}

bool ConsoleCommand::Process() {
    if (m_haltServer) {
        if (!m_dbError)
            sEntityList.Shutdown();
        return false;
    }

    {
        // Critical section
        std::unique_lock<std::mutex> lock(m_inputMutex);
        if (m_inputCondition.wait_for(lock, std::chrono::seconds(0), [&] { return !m_input.empty(); })) {
            // Get a new batch of inputs to process
            std::swap(m_input, m_inputToProcess);
        }
    }

    bool continueRunning = true;
    for (auto&& input : m_inputToProcess)
    {
        continueRunning &= HandleCommand(input.c_str());
    }
    m_inputToProcess.clear();
    
    return continueRunning;
}

bool ConsoleCommand::HandleCommand(const char* buf)
{
    if (strncmp(buf, "x", 1) == 0) {
        sLog.Warning("  EVEmu", "EXIT called.  Breaking out of Main Loop.");
        m_haltServer = true;
        return false;
    }
    else if (strncmp(buf, "h", 1) == 0) {
        sLog.Green("  EVEmu", "Current Console Commands and Descriptions: ");
        sLog.Warning("", "");
        sLog.Warning("           (h)elp", " Displays this dialog.");
        sLog.Warning("          h(e)llo", " Displays the 'Hello World' message.");
        sLog.Warning("           e(x)it", " Exits the server, saving all loaded items and loging out all connected clients.");
        sLog.Warning("        (c)lients", " Displays connected clients, showing account, character, and ip.");
        sLog.Warning("         (s)tatus", " Displays Server's System Status, showing threads, memory, and cpu time.");
        sLog.Warning("        (v)ersion", " Displays server version.");
        sLog.Warning("    (i)nformation", " Displays server information: version, memory, uptime, clients, items, systems, bubbles.");
        sLog.Warning("           s(a)ve", " Immediatly saves all loaded items.  *Broken*");
        sLog.Warning("      (b)roadcast", " Broadcasts a message to all clients thru the LocalChat window.  *Not Implemented*");
        sLog.Warning("           (n)ote", " Broadcasts a message to all clients thru a notification window.");
        sLog.Warning("        (m)essage", " Broadcasts a message to all clients thru a message window.");
        sLog.Warning("        (p)rofile", " Prints a profile of current server runtimes.  *Incomplete*");
        sLog.Warning("          r(o)les", " Prints a list of common roles and their values.");
        sLog.Warning("       c(o)mmands", " Prints a list of currently loaded Commands and their required role. (long list)");
        sLog.Warning("           (t)est", " Prints the current test object *varies*");
        sLog.Warning("        e(f)fects", " Compiles and prints all item effects.");
        sLog.Warning("        threa(d)s", " Prints a list of current threads.");
        sLog.Warning("    reload (l)ogs", " Reloads log.ini to change values without restarting server.");
        sLog.Warning("(q)uery stat data", " Prints current statistic data.");
        sLog.Warning("       hea(r) all", " Echo all chat msgs to console. *Not Implemented*");
    }
    else if (strncmp(buf, "e", 1) == 0) {
        sLog.Green("  EVEmu", "Server Hello:");
        sLog.Magenta("      Server Says", " Hello World!");
    }
    else if (strncmp(buf, "c", 1) == 0) {
        sLog.Green("  EVEmu", "Client Connection Information");
        uint8 clients = sEntityList.GetClientCount();
        sLog.Warning("      Connections", " %u Current Clients Online", clients);
        sLog.Warning("      Connections", " %u Clients Connected since startup.", sEntityList.GetConnections());
        if (clients) {
            std::vector<Client*> list;
            sEntityList.GetClients(list);
            for (auto cur : list) {
                if (cur->GetChar().get() == nullptr) {
                    sLog.Magenta("      Connections", " Account %u Connected, but no character selected yet.", cur->GetUserID());
                    continue;
                }
                else
                    sLog.Warning("      Connections", " [(%u)%u] %s in %s(%u).  %u Minutes Online.", \
                        cur->GetUserID(), cur->GetCharacterID(), cur->GetName(), \
                        cur->GetSystemName().c_str(), cur->GetLocationID(), cur->GetChar()->OnlineTime());
            }
        }
    }
    else if (strncmp(buf, "s", 1) == 0) {
        std::string state = "";
        int64 threads(0);
        uint8 aThreads = std::thread::hardware_concurrency();
        float vm(0.0f), rss(0.0f), user(0.0f), kernel(0.0f);
        Status(state, threads, vm, rss, user, kernel);
        sLog.Warning("    Server Status", "  S: %s | T: %d(%u) | RSS: %.3fMb | VM: %.3fMb | U: %.2f | K: %.2f", \
            state.data(), threads, aThreads, rss, vm, user, kernel);
        sLog.Warning("      Connections", " %u Current Clients Online.", sEntityList.GetClientCount());
        sLog.Warning("      Connections", " %u Clients Connected since startup.", sEntityList.GetConnections());
        if (sConfig.debug.UseProfiling) {
            sLog.Green(" Server Profiling", "Enabled.");
        }
        else {
            sLog.Warning(" Server Profiling", "Disabled.");
        }
        if (sConfig.npc.StaticSpawns) {
            sLog.Green("    Static Spawns", "Enabled.");
        }
        else {
            sLog.Warning("    Static Spawns", "Disabled.");
        }
        if (sConfig.npc.RoamingSpawns) {
            sLog.Green("   Roaming Spawns", "Enabled.");
        }
        else {
            sLog.Warning("   Roaming Spawns", "Disabled.");
        }
        if (sConfig.rates.secRate != 1.0) {
            sLog.Green("        SecStatus", "Enabled at %.0f%%.", (sConfig.rates.secRate * 100));
        }
        else {
            sLog.Warning("        SecStatus", "Normal.");
        }
        if (sConfig.rates.npcBountyMultiply != 1.0) {
            sLog.Green("          Bountys", "Enabled at %.0f%%.", (sConfig.rates.npcBountyMultiply * 100));
        }
        else {
            sLog.Warning("          Bountys", "Normal.");
        }
        if (sConfig.rates.damageRate != 1.0) {
            sLog.Green("      All Damages", "Enabled at %.0f%%.", (sConfig.rates.damageRate * 100));
        }
        else {
            sLog.Warning("      All Damages", "Normal.");
        }
        if (sConfig.rates.missileRoF != 1.0) {
            sLog.Green("      Missile Dmg", "Enabled at %.0f%%.", (sConfig.rates.missileRoF * 100));
        }
        else {
            sLog.Warning("      Missile Dmg", "Normal.");
        }
        if (sConfig.rates.turretRoF != 1.0) {
            sLog.Green("       Turret Dmg", "Enabled at %.0f%%.", (sConfig.rates.turretRoF * 100));
        }
        else {
            sLog.Warning("       Turret Dmg", "Normal.");
        }
    }
    else if (strncmp(buf, "v", 1) == 0) {
        sLog.Green("  EVEmu", "Server Version:");
        sLog.Warning("  Server Revision", " %s", EVEMU_REVISION);
        sLog.Warning("       Build Date", " %s", EVEMU_BUILD_DATE);
    }
    else if (strncmp(buf, "i", 1) == 0) {
        sLog.Green("  EVEmu", "Server Information:");
        sLog.Warning("  Server Revision", " %s", EVEMU_REVISION);
        sLog.Warning("       Build Date", " %s", EVEMU_BUILD_DATE);
        //  memory
        std::string state = "";
        int64 threads(0);
        uint8 aThreads = std::thread::hardware_concurrency();
        float vm(0.0f), rss(0.0f), user(0.0f), kernel(0.0f);
        Status(state, threads, vm, rss, user, kernel);
        sLog.Warning("     Memory Usage", " RSS: %.3fMb  VM: %.3fMb", rss, vm);
        sLog.Warning("    Server Status", "  S: %s | T: %lli(%u) | U: %.2f | K: %.2f", \
            state.data(), threads, aThreads, user, kernel);
        std::string uptime;
        sEntityList.GetUpTime(uptime);
        sLog.Warning("    Server UpTime", " %s", uptime.c_str());
        //  loaded items
        sLog.Warning("     Loaded Items", " %u", sItemFactory.Count());
        //  loaded NPCs
        sLog.Warning("      Loaded NPCs", " %u", sEntityList.GetNPCCount());
        //  loaded systems
        sLog.Warning("   Active Systems", " %u", sEntityList.GetSystemCount());
        //  loaded stations
        sLog.Warning("  Active Stations", " %u", sEntityList.GetStationCount());
        //  loaded bubbles
        sLog.Warning("   Active Bubbles", " %u", sBubbleMgr.Count());
        //  current clients
        sLog.Warning("      Connections", " %u Current Clients Online.", sEntityList.GetClientCount());
        sLog.Warning("      Connections", " %u Clients Connected since startup.", sEntityList.GetConnections());
    }
    else if (strncmp(buf, "a", 1) == 0) {
        sLog.Green("  EVEmu", "Server SaveAll:");
        //sLog.Error("      Server Save", " Not Available Yet." );
        sItemFactory.SaveItems();
    }
    else if (strncmp(buf, "b", 1) == 0) {
        sLog.Green("  EVEmu", "Server Broadcast:");
        sLog.Error(" Server Broadcast", " Not Available Yet.");
        /** @todo  make this show in general chat channel */
        //const char* buff = buf + 2;
        //SendMessage(buff);
    }
    else if (strncmp(buf, "n", 1) == 0) {
        sLog.Green("  EVEmu", "Server Notify:");
        const char* buff = buf + 2;
        std::vector<Client*> list;
        sEntityList.GetClients(list);
        for (auto cur : list)
            cur->SendNotifyMsg(buff);
        sLog.Warning("  Console Command", " Notification sent to all online clients.");
    }
    else if (strncmp(buf, "m", 1) == 0) {
        sLog.Green("  EVEmu", "Server Modal Message:");
        const char* buff = buf + 2;
        std::vector<Client*> list;
        sEntityList.GetClients(list);
        for (auto cur : list)
            cur->SendInfoModalMsg(buff);
        sLog.Warning("  Console Command", " Modal Message sent to all online clients.");
    }
    else if (strncmp(buf, "p", 1) == 0) {
        sLog.Green("  EVEmu", "Server Profile:");
        if (sConfig.debug.UseProfiling) {
            sLog.Warning("    Current Stamp", " %u", sEntityList.GetStamp());
            std::string uptime;
            sEntityList.GetUpTime(uptime);
            sLog.Warning("    Server UpTime", " %s", uptime.c_str());
            sLog.Warning("      Connections", " %u Current Clients Online.", sEntityList.GetClientCount());
            sLog.Warning("      Connections", " %u Clients Connected since startup.", sEntityList.GetConnections());
            sProfiler.PrintProfile();
        }
        else {
            sLog.Error("   Server Profile", "Profiling is turned off.");
        }
    }
    else if (strncmp(buf, "r", 1) == 0) {
        // enable console chat echo
    }
    else if (strncmp(buf, "o", 1) == 0) {
        sLog.Green("  EVEmu", "Common Account Roles:");
        sLog.Warning("     Acct::Role::DEV", " %lli(%p)", Acct::Role::DEV, Acct::Role::DEV);
        sLog.Warning("     Acct::Role::STD", " %lli(%p)", Acct::Role::STD, Acct::Role::STD);
        sLog.Warning("     Acct::Role::VIP", " %lli(%p)", Acct::Role::VIP, Acct::Role::VIP);
        sLog.Warning("    Acct::Role::VIP+", " %lli(%p)", Acct::Role::EPLAYER, Acct::Role::EPLAYER);
        sLog.Warning("    Acct::Role::VIEW", " %lli(%p)", Acct::Role::VIEW, Acct::Role::VIEW);
        sLog.Warning("    Acct::Role::BOSS", " %lli(%p)", Acct::Role::BOSS, Acct::Role::BOSS);
        sLog.Warning("   Acct::Role::SLASH", " %lli(%p)", Acct::Role::SLASH, Acct::Role::SLASH);
        sLog.Warning(" Acct::Role::CREATOR", " %lli(%p)", Acct::Role::CREATOR, Acct::Role::CREATOR);
        std::printf("\n");     // spacer
        sLog.Green("  EVEmu", "Common Corp Roles:");
        sLog.Warning("     Corp::Role::All", " %lli(%p)", Corp::Role::All, Corp::Role::All);
        sLog.Warning("    Corp::Role::Cont", " %lli(%p)", Corp::Role::AllContainer, Corp::Role::AllContainer);
        sLog.Warning("   Corp::Role::Admin", " %lli(%p)", Corp::Role::Admin, Corp::Role::Admin);
        sLog.Warning("  Corp::Role::Hangar", " %lli(%p)", Corp::Role::AllHangar, Corp::Role::AllHangar);
        sLog.Warning(" Corp::Role::Account", " %lli(%p)", Corp::Role::AllAccount, Corp::Role::AllAccount);
        sLog.Warning("Corp::Role::Starbase", " %lli(%p)", Corp::Role::AllStarbase, Corp::Role::AllStarbase);
    }
    else if (strncmp(buf, "o", 1) == 0) {
        pCommand->ListCommands();
    }
    else if (strncmp(buf, "t", 1) == 0) {
        Test();
    }
    else if (strncmp(buf, "f", 1) == 0) {
        std::string args(buf);
        char& num(args.at(1));
        FxProc(atoi(&num));
    }
    else if (strncmp(buf, "d", 1) == 0) {
        uint8 maxCount = sConfig.server.MaxThreadReport;
        uint8 count = sThread.Count();
        sLog.Blue("   Active Threads", "There are %u active threads running in the server.", count);
        if (count > maxCount) {
            sLog.Warning("   Active Threads", "Individual thread IDs are not displayed for more than %u active threads.", maxCount);
        }
        else {
            sThread.ListThreads();
        }
    }
    else if (strncmp(buf, "l", 1) == 0) {
        /*
        sLog.~NewLog();
        sLog.InitializeLogging(sConfig.files.logDir);
        */
        if (load_log_settings(sConfig.files.logSettings.c_str())) {
            sLog.Green("  EVEmu", "Log settings reloaded from %s", sConfig.files.logSettings.c_str());
            // reset config switches based on log settings
            sConfig.debug.StackTrace = is_log_enabled(SERVER__STACKTRACE);
            sConfig.debug.BeanCount = is_log_enabled(SERVER__BEANCOUNT);
            sConfig.debug.IsTestServer = is_log_enabled(SERVER__TESTSERVER);
        }
        else {
            sLog.Warning("  EVEmu", "Unable to reload settings from %s", sConfig.files.logSettings.c_str());
        }
    }
    else if (strncmp(buf, "q", 1) == 0) {
        sLog.Green("  EVEmu", "Server Statistic Data:");
        sStatMgr.PrintInfo();
    }
    else {
        sLog.Error("  EVEmu", "Command not recognized: %s", buf);
    }

    return true;
}

void ConsoleCommand::SendMessage(const char* msg) {
    // LSCChannel::SendMessage(Client * c, const char * message, bool self)
    //  this isnt how it works....need more info
}

void ConsoleCommand::Status(std::string& state, int64& threads, float& vm_usage, float& resident_set, float& user, float& kernel)
{
#if _WIN32
    // Pursche TODO: Figure out how to report this stuff
    state = "R";
    threads = 1;
    vm_usage = 0.0f;
    resident_set = 0.0f;
    user = 0.0f;
    kernel = 0.0f;
#else
    // the fields we want
    std::string ignore = "", run_state = "";
    int64 num_threads(0);  //this is saved from OS as long decimal....*sigh*  gotta allocate long int for it or weird shit happens.
    int64 vsize(0);      //in bytes
    int64 rss(0);			//in pages
    float utime(0.0f), stime(0.0f);

    // stat seems to give the most reliable results
    std::ifstream ifs ("/proc/self/stat", std::ios_base::in);
    ifs >> ignore >> ignore >> run_state >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
        >> ignore >> ignore >> ignore >> utime >> stime >> ignore >> ignore >> ignore >> ignore >> num_threads
        >> ignore >> ignore >> vsize >> rss;
    ifs.close();

    if (sConfig.debug.IsTestServer)
        _log(SERVER__INFO, "ConsoleCommand::Status() proc/self/stat returns RSS: %lli, VM: %lli", rss, vsize);

    /*  state = One character from the string "RSDZTW" where
              R is running,
              S is sleeping in an interruptible wait,
              D is waiting in uninterruptible disk sleep,
              Z is zombie,
              T is traced or stopped (on a signal),
              W is paging
    */
    state = run_state;
    threads = num_threads;
    user = utime / sysconf(_SC_CLK_TCK) / 100.0f;
    kernel = stime / sysconf(_SC_CLK_TCK) / 100.0f;
    vm_usage     = ((vsize / sysconf(_SC_PAGE_SIZE)) / 1024.0f / 6);
    //rss (in pages) * page_size(in bytes, converted to k), then convert to Mb.
    resident_set = (rss * (sysconf(_SC_PAGE_SIZE) / 1024.0f) / 1024.0f);
#endif
}

void ConsoleCommand::Test()
{
    sLog.Green("   EVEmu", "Running Server Test Code:");
    //sLog.Warning(" Server Test Code", "No Test Code at this time.");

    //sMktMgr.GetCruPrices();
    //sLog.Blue("       MktPricing", "Base material Prices Updated");

    sMktMgr.SetBasePrice();

    // execute code to begin filling missing data in mission db.
    //  first step:  get courier missionIDs
    //sMissionDataMgr.UpdateMissionData();
    //MarketDB::UpdateHistory();
}

void ConsoleCommand::UpdateStatus() {
	std::string state = "";
	int64 threads(0);
	float vm(0.0f), rss(0.0f), user(0.0f), kernel(0.0f);
	Status(state, threads, vm, rss, user, kernel);
    if (sConfig.debug.IsTestServer)
        _log(SERVER__INFO, "Current Mem usage - RSS: %f, VM: %f", rss, vm);
    ServiceDB::SaveServerStats(threads + sThread.Count(), rss, vm, user, kernel, sItemFactory.Count(), sBubbleMgr.Count());
    sStatMgr.Process();
}

void ConsoleCommand::FxProc(uint8 idx/*0*/) {
    double start = GetTimeMSeconds();
    sLog.Green("  EVEmu", "FxProcess(%u)", idx);

    // load items from db
    std::map<uint16, std::string> typeIDs;
    switch(idx) {
        case 1: {
            sLog.Green("        FxProcess", "Parsing Effects for Module Items.");
            std::printf("\n");     // spacer
            ItemDB::GetItems(EVEDB::invCategories::Module, typeIDs);
        } break;
        case 2: {
            sLog.Green("        FxProcess", "Parsing Effects for Charge Items.");
            std::printf("\n");     // spacer
            ItemDB::GetItems(EVEDB::invCategories::Charge, typeIDs);
        } break;
        case 3: {
            sLog.Green("        FxProcess", "Parsing Effects for Subsystem Items.");
            std::printf("\n");     // spacer
            ItemDB::GetItems(EVEDB::invCategories::Subsystem, typeIDs);
        } break;
        case 4: {
            sLog.Green("        FxProcess", "Parsing Effects for Skill Items.");
            std::printf("\n");     // spacer
            ItemDB::GetItems(EVEDB::invCategories::Skill, typeIDs);
        } break;
        case 5: {
            sLog.Green("        FxProcess", "Parsing Effects for Implant Items.");
            std::printf("\n");     // spacer
            ItemDB::GetItems(EVEDB::invCategories::Implant, typeIDs); // this is implants and boosters
        } break;
        case 6: {
            sLog.Green("        FxProcess", "Parsing Effects for Ship Items.");
            std::printf("\n");     // spacer
            ItemDB::GetItems(EVEDB::invCategories::Ship, typeIDs);
        } break;
        case 9: {
            sLog.Green("        FxProcess", "Parsing Effects for All Possible Items.");
            std::printf("\n");     // spacer
            ItemDB::GetItems(EVEDB::invCategories::Module, typeIDs);
            ItemDB::GetItems(EVEDB::invCategories::Charge, typeIDs);
            ItemDB::GetItems(EVEDB::invCategories::Subsystem, typeIDs);
            ItemDB::GetItems(EVEDB::invCategories::Skill, typeIDs);
            ItemDB::GetItems(EVEDB::invCategories::Implant, typeIDs); // this is implants and boosters
            ItemDB::GetItems(EVEDB::invCategories::Ship, typeIDs);
        } break;
        default: {
            sLog.Green("        FxProcess", "Usage of Item Fx process reporting.");
            sLog.Green("        FxProcess", "fx where 'x' is a number corresponding to the item category you wish to process.");
            sLog.Green("        FxProcess", "1=Modules, 2=Charges, 3=Subsystems, 4=Skills, 5=Implants/Boosters, 6=Ships, 9=all");
            return;
        }
    }

    for (auto cur : typeIDs) {
        sLog.Yellow("CC::FxProc", "Processing %s (%u)", cur.second.c_str(), cur.first);
        // proc and print skill fx
        std::vector< TypeEffects > typeEffMap;
        sFxDataMgr.GetTypeEffect(cur.first, typeEffMap);
        for (auto cur2: typeEffMap)
            sFxProc.DecodeEffects(cur2.effectID);
    }

    sLog.Magenta("CC::FxProc", "- effects processed in %.3fms", (GetTimeMSeconds() - start));
}
