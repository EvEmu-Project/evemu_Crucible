/**
 * @name Profiler.cpp
 *   lightweight profiling code specifically for timing sections of running EvEmu application
 *   this code is very basic, and very specific.
 * @Author:         Allan
 * @date:   13 April 2015
 */

#include "Profiler.h"
#include "EVEServerConfig.h"
#include "../eve-core/utils/misc.h"

Profiler::Profiler() { }

Profiler::~Profiler() {
    ClearAll();
}

int Profiler::Initialize() {
    ClearAll();
    sLog.Blue("  Profile Manager", "Profiling initialized.");
    return 1;
}

void Profiler::AddTime(uint8 key, double value) {
    if (sConfig.debug.ProfileTraceTime > 0)
        if (value > sConfig.debug.ProfileTraceTime *1000) {
            sLog.Warning("  Profile Manager", "Long Profile Time on key %s, time %.3f.", GetKeyName(key).c_str(), value);
            //EvE::traceStack();
        }
    /*
    Profile::destiny     = 1,    *
    Profile::map         = 2,
    Profile::client      = 3,    *
    Profile::npc         = 4,    *
    Profile::bubbles     = 5,    *
    Profile::items       = 6,
    Profile::modules     = 7,    *
    Profile::functions   = 8,
    Profile::db          = 9,    *
    Profile::ship        = 10,   *
    Profile::targets     = 11,
    Profile::server      = 12,
    Profile::missile     = 13,
    Profile::system      = 14,
    Profile::entityS     = 15,   *
    Profile::loot        = 16,   *
    Profile::salvage     = 17,
    Profile::spawn       = 18,   *
    Profile::collision   = 19,   *
    Profile::drone       = 20,   *
    Profile::itemload    = 21,   *
    Profile::concord     = 22,   *
    Profile::colony      = 23,   *
    Profile::damage      = 24,   *
    Profile::parseFX     = 25,   *
    Profile::applyFX     = 26,   *
    Profile::onTarg      = 27
    */
    switch(key) {
        case 1:
            m_destiny.push_back(value);
            break;
        case 2:
            m_map.push_back(value);
            break;
        case 3:
            m_client.push_back(value);
            break;
        case 4:
            m_npc.push_back(value);
            break;
        case 5:
            m_bubbles.push_back(value);
            break;
        case 6:
            m_items.push_back(value);
            break;
        case 7:
            m_modules.push_back(value);
            break;
        case 8:
            m_functions.push_back(value);
            break;
        case 9:
            m_db.push_back(value);
            break;
        case 10:
            m_ship.push_back(value);
            break;
        case 11:
            m_targets.push_back(value);
            break;
        case 12:
            m_server.push_back(value);
            break;
        case 13:
            m_missile.push_back(value);
            break;
        case 14:
            m_system.push_back(value);
            break;
        case 15:
            m_entityS.push_back(value);
            break;
        case 16:
            m_loot.push_back(value);
            break;
        case 17:
            m_salvage.push_back(value);
            break;
        case 18:
            m_spawn.push_back(value);
            break;
        case 19:
            m_collision.push_back(value);
            break;
        case 20:
            m_drone.push_back(value);
            break;
        case 21:
            m_itemload.push_back(value);
            break;
        case 22:
            m_concord.push_back(value);
            break;
        case 23:
            m_colony.push_back(value);
            break;
        case 24:
            m_damage.push_back(value);
            break;
        case 25:
            m_effects1.push_back(value);
            break;
        case 26:
            m_effects2.push_back(value);
            break;
        case 27:
            m_ontarget.push_back(value);
            break;
        default:
            sLog.Error("Profile::AddTime()", "Default reached on key %u.", key );
            break;
    }
}

void Profiler::ClearAll()
{
    m_server.clear();
    m_destiny.clear();
    m_map.clear();
    m_client.clear();
    m_npc.clear();
    m_bubbles.clear();
    m_items.clear();
    m_modules.clear();
    m_functions.clear();
    m_db.clear();
    m_ship.clear();
    m_targets.clear();
    m_ontarget.clear();
    m_missile.clear();
    m_system.clear();
    m_entityS.clear();
    m_loot.clear();
    m_salvage.clear();
    m_spawn.clear();
    m_collision.clear();
    m_drone.clear();
    m_itemload.clear();
    m_concord.clear();
    m_colony.clear();
    m_damage.clear();
    m_effects1.clear();
    m_effects2.clear();
}

void Profiler::PrintProfile()
{
    /** @todo figure out how to color this based on times....R,Y,G,M,B,W  */

    double startTime = GetTimeUSeconds();
    std::string fSize;
    float h(0.0f), l(0.0f), a(0.0f);
    sLog.Green("   Server Profile", " Current Process Profile times for this run:");
    //std::printf("\n");     // spacer
    std::printf("\t\tLoop Calls\n");
    GetRunTimes(m_entityS, h, l, a);
    GetSize(m_entityS.size(), fSize);
    std::printf("    EntityList   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    GetRunTimes(m_client, h, l, a);
    GetSize(m_client.size(), fSize);
    std::printf("        Client   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    GetRunTimes(m_system, h, l, a);
    GetSize(m_system.size(), fSize);
    std::printf("     SystemMgr   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    GetRunTimes(m_bubbles, h, l, a);
    GetSize(m_bubbles.size(), fSize);
    std::printf("       Bubbles   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    GetRunTimes(m_destiny, h, l, a);
    GetSize(m_destiny.size(), fSize);
    std::printf("       Destiny   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    GetRunTimes(m_npc, h, l, a);
    GetSize(m_npc.size(), fSize);
    std::printf("           NPC   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    GetRunTimes(m_modules, h, l, a);
    GetSize(m_modules.size(), fSize);
    std::printf("       Modules   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    GetRunTimes(m_ship, h, l, a);
    GetSize(m_ship.size(), fSize);
    std::printf("          Ship   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    //GetRunTimes(m_ontarget, h, l, a);
    //GetSize(m_ontarget.size(), fSize);
    //std::printf("      OnTarget   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    GetRunTimes(m_targets, h, l, a);
    GetSize(m_targets.size(), fSize);
    std::printf("    TargetProc   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    GetRunTimes(m_missile, h, l, a);
    GetSize(m_missile.size(), fSize);
    std::printf("       Missile   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    GetRunTimes(m_damage, h, l, a);
    GetSize(m_damage.size(), fSize);
    std::printf("        Damage   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    if (sConfig.npc.RoamingSpawns or sConfig.npc.StaticSpawns) {
        GetRunTimes(m_spawn, h, l, a);
        GetSize(m_spawn.size(), fSize);
        std::printf("        Spawns   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    } else {
        std::printf("        Spawns   Disabled.\n");
    }
    if (sConfig.cosmic.BumpEnabled) {
        GetRunTimes(m_collision, h, l, a);
        GetSize(m_collision.size(), fSize);
        std::printf("    Collisions   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    } else {
        std::printf("    Collisions   Disabled.\n");
    }
    if (sConfig.testing.EnableDrones) {
        GetRunTimes(m_drone, h, l, a);
        GetSize(m_drone.size(), fSize);
        std::printf("        Drones   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    } else {
        std::printf("        Drones   Disabled.\n");
    }

    //std::printf("\n");     // spacer
    std::printf("\t\tPeriodic Calls\n");
    GetRunTimes(m_db, h, l, a);
    GetSize(m_db.size(), fSize);
    std::printf("            DB   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    GetRunTimes(m_effects1, h, l, a);
    GetSize(m_effects1.size(), fSize);
    std::printf(" Parse Effects   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    GetRunTimes(m_effects2, h, l, a);
    GetSize(m_effects2.size(), fSize);
    std::printf(" Apply Effects   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    GetRunTimes(m_itemload, h, l, a);
    GetSize(m_itemload.size(), fSize);
    std::printf("  Item Loading   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    GetRunTimes(m_loot, h, l, a);
    GetSize(m_loot.size(), fSize);
    std::printf("          Loot   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    GetRunTimes(m_salvage, h, l, a);
    GetSize(m_salvage.size(), fSize);
    std::printf("       Salvage   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    if (sConfig.cosmic.PIEnabled) {
        GetRunTimes(m_colony, h, l, a);
        GetSize(m_colony.size(), fSize);
        std::printf("        Colony   %s times.   \tHi: %.4fus   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    } else {
        std::printf("        Colony   Disabled.\n");
    }
    if (sConfig.crime.Enabled) {
        GetRunTimes(m_concord, h, l, a);GetSize(m_concord.size(), fSize);
        std::printf("       Concord   %s times.   \tHi: %.4f   \tLo: %.4fus   \tAvg: %.4fus\n", fSize.c_str(), h, l, a );
    } else {
        std::printf("       Concord   Disabled.\n");
    }

    //std::printf("\n");     // spacer
    std::printf("\t\tUnimplemented Calls\n");
    /* not used yet. */
    GetRunTimes(m_server, h, l, a);
    std::printf("        *Main()  %u times.   \tHi: %.4f   \tLo: %.4fus   \tAvg: %.4fus\n", m_server.size(), h, l, a );
    GetRunTimes(m_map, h, l, a);
    std::printf("          *Map   %u times.   \tHi: %.4f   \tLo: %.4fus   \tAvg: %.4fus\n", m_map.size(), h, l, a );
    GetRunTimes(m_items, h, l, a);
    std::printf("        *Items   %u times.   \tHi: %.4f   \tLo: %.4fus   \tAvg: %.4fus\n", m_items.size(),  h, l, a );
    GetRunTimes(m_functions, h, l, a);
    std::printf("    *Functions   %u times.   \tHi: %.4f   \tLo: %.4fus   \tAvg: %.4fus\n", m_functions.size(), h, l, a );

    std::printf(" Profile Times Compiled in %.4fus\n", (GetTimeUSeconds() -startTime) );
}

void Profiler::GetRunTimes(std::vector< double >& container, float& h, float& l, float& a)
{
    if (container.empty()) {
        h = 0.0f;
        l = 0.0f;
        a = 0.0f;
        return;
    }

    uint32 size = container.size();
    float total(0.0f), lo(0.0f), hi(0.0f);
    for (uint32 i = 0; i < size; ++i) {
        total += container.at(i);
        if ((lo > container.at(i)) or (lo < 0.000001f))
            lo = container.at(i);
        if (hi < container.at(i))
            hi = container.at(i);
    }

    h = hi;
    l = lo;
    a = total / size;
}

void Profiler::GetSize(size_t cSize, std::string& fSize)
{
    if (cSize > 999999) {
        fSize = std::to_string(cSize / 1000000);
        fSize += "m";
    } else if (cSize > 9999) {
        fSize = std::to_string(cSize / 1000);
        fSize += "k";
    } else {
        fSize = std::to_string(cSize);
    }
}

std::string Profiler::GetKeyName(uint8& key)
{
    switch (key) {
        case Profile::destiny:       return "Destiny";   //  1,
        case Profile::map:           return "Map";       //  2,
        case Profile::client:        return "Client";    //  3,
        case Profile::npc:           return "NPC";       //  4,
        case Profile::bubbles:       return "Bubble";    //  5,
        case Profile::items:         return "Item";      //  6,
        case Profile::modules:       return "Module";    //  7,
        case Profile::functions:     return "Function";  //  8,
        case Profile::db:            return "DB";        //  9,
        case Profile::ship:          return "Ship";      //  10,
        case Profile::targets:       return "Target";    //  11,
        case Profile::server:        return "Server";    //  12,
        case Profile::missile:       return "Missile";   //  13,
        case Profile::system:        return "System";    //  14,
        case Profile::entityS:       return "EntityTic"; //  15,
        case Profile::loot:          return "Loot";      //  16,
        case Profile::salvage:       return "Salvage";   //  17,
        case Profile::spawn:         return "Spawn";     //  18,
        case Profile::collision:     return "Collision"; //  19,
        case Profile::drone:         return "Drone";     //  20,
        case Profile::itemload:      return "ItemLoad";  //  21,
        case Profile::concord:       return "Concord";   //  22,
        case Profile::colony:        return "Colony";    //  23,
        case Profile::damage:        return "Damage";    //  24,
        case Profile::parseFX:       return "ParseFX";   //  25,
        case Profile::applyFX:       return "ApplyFX";   //  26
        default:                     return "Invalid Key";
    }
}

/*  color shit....
 *
    ::fputs( COLOR_TABLE[ color ], stdout );
    std::printf(*message*);
    *


    const char* const NewLog::COLOR_TABLE[ COLOR_COUNT ] =
    {
    "\033[" "00"    "m", // COLOR_DEFAULT
    "\033[" "30;22" "m", // COLOR_BLACK
    "\033[" "31;22" "m", // COLOR_RED
    "\033[" "32;22" "m", // COLOR_GREEN
    "\033[" "33;01" "m", // COLOR_YELLOW
    "\033[" "34;01" "m", // COLOR_BLUE
    "\033[" "35;22" "m", // COLOR_MAGENTA
    "\033[" "36;01" "m", // COLOR_CYAN
    "\033[" "37;01" "m"  // COLOR_WHITE
    };

    */

