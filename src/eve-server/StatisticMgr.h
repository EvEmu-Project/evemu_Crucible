
 /**
  * @name StatisticMgr.h
  *   server information system for managing and saving trivial game data
  *
  * @Author:    Allan
  * @date:      8 March 2018
  *
  */


#ifndef _EVE_SERVER_STAT_MANAGER_H__
#define _EVE_SERVER_STAT_MANAGER_H__


#include "eve-server.h"
#include "POD_containers.h"

// this class tracks misc statistical data based on in-game events.

namespace Stat {
    enum {
        pcShots         = 1,
        pcMissiles      = 2,
        pcBounties      = 3,
        npcBounties     = 4,
        oreMined        = 5,
        iskMarket       = 6,
        shipsSalvaged   = 7,
        probesLaunched  = 8,
        sitesScanned    = 9,
        ramJobs         = 10
    };
}


class StatisticMgr
: public Singleton< StatisticMgr >
{
public:
    StatisticMgr();
    ~StatisticMgr() { /* do nothing here */ }

    int Initialize();

    void Close();
    void GetInfo();
    void Process();

    void PrintInfo();
    void ClearAll();

    void Add(uint8 key, double value);
    void Increment(uint8 key);

protected:
    void SaveData();
    void CompileData();

private:
    StatisticData m_data;

    int8 m_counter;
};


//Singleton
#define sStatMgr \
    ( StatisticMgr::get() )

#endif  // _EVE_SERVER_STAT_MANAGER_H__
