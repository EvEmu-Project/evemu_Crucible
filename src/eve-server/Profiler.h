/**
 * @name Profiler.h
 *   lightweight profiling code specifically for timing sections of running EvEmu application
 *   this code is very basic, and very specific.
 * @Author:         Allan
 * @date:   13 April 2015
 */

/**   Allan's EvEmu Profiler
 * simple singleton profiler using std::vector<double> being simple mem object storage.
 * vector object denotes call type, (db, client, map, etc.)
 * double precision float holds elapsed time for that particular call
 * output functions use size() to count calls, and give readouts as
 *     CALL_TYPE: called N times, hi: Xus, lo: Yus, avg: Zus
 *  Times are measured in microseconds via GetTimeUSeconds() from core/utils/utils_time.cpp
 *
 */


#ifndef EVEMU_EVESERVER_PROFILER_H_
#define EVEMU_EVESERVER_PROFILER_H_


#include "eve-common.h"

namespace Profile {
    enum {          // implemented?  (* = yes)
        destiny     = 1,    //*
        map         = 2,    //
        client      = 3,    //*
        npc         = 4,    //*
        bubbles     = 5,    //*
        items       = 6,    //*
        modules     = 7,    //*
        functions   = 8,    //
        db          = 9,    //*
        ship        = 10,   //*
        targets     = 11,   //*
        server      = 12,   //
        missile     = 13,   //*
        system      = 14,   //*
        entityS     = 15,   //*
        loot        = 16,   //*
        salvage     = 17,   //*
        spawn       = 18,   //*
        collision   = 19,   //*
        drone       = 20,   //*
        itemload    = 21,   //*
        concord     = 22,   //
        colony      = 23,   //*
        damage      = 24,   //*
        parseFX     = 25,   //*
        applyFX     = 26,   //*
        onTarg      = 27    //
    };
}

class Profiler
: public Singleton<Profiler>
{
public:
    Profiler() {};
    ~Profiler() {};

    int Initialize();

    void AddTime(uint8 key, double value);
    void PrintProfile();
    void PrintStartUpData();
    void ClearAll();

    void GetSize(size_t cSize, std::string& ret);

    void GetRunTimes(std::vector< double >& container, float& h, float& l, float& a);

protected:
    std::string GetKeyName(uint8& key);

private:
    std::vector<double> m_server;
    std::vector<double> m_functions;
    std::vector<double> m_db;
    std::vector<double> m_client;
    std::vector<double> m_map;
    std::vector<double> m_destiny;
    std::vector<double> m_system;
    std::vector<double> m_entityS;
    std::vector<double> m_npc;
    std::vector<double> m_drone;
    std::vector<double> m_bubbles;
    std::vector<double> m_items;
    std::vector<double> m_itemload;
    std::vector<double> m_modules;
    std::vector<double> m_ship;
    std::vector<double> m_targets;
    std::vector<double> m_ontarget;
    std::vector<double> m_missile;
    std::vector<double> m_loot;
    std::vector<double> m_salvage;
    std::vector<double> m_spawn;
    std::vector<double> m_collision;
    std::vector<double> m_concord;
    std::vector<double> m_colony;
    std::vector<double> m_damage;
    std::vector<double> m_effects1;
    std::vector<double> m_effects2;
};

#define sProfiler \
    ( Profiler::get() )

#endif  // EVEMU_EVESERVER_PROFILER_H_
