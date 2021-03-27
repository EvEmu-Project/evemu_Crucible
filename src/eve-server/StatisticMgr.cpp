
 /**
  * @name StatisticMgr.cpp
  *   server information system for managing and saving trivial ingame statistical data
  *
  * @Author:    Allan
  * @date:      8 March 2018
  *
  */


#include "StatisticMgr.h"
#include "system/cosmicMgrs/ManagerDB.h"


StatisticMgr::StatisticMgr()
: m_counter(3)  // do first update 15m after server starts
{
    m_data = StatisticData();
}

void StatisticMgr::Close()
{
    // save current data to db before exit.
    SaveData();
    sLog.Warning("     StatisticMgr", "Statistics Manager has been closed." );
}

int StatisticMgr::Initialize()
{
    //ClearAll();
    // reset current data for new session
    ManagerDB::UpdateStatisticHistory(m_data);
    sLog.Blue( "     StatisticMgr", "Statistics Manager Initialized." );
    return 1;
}

void StatisticMgr::ClearAll()
{
    m_data = StatisticData();
}

void StatisticMgr::GetInfo()
{
    // return current data?
}

// called every 15m by ConsoleCommands::UpdateStatus() from EntityList::Process()
void StatisticMgr::Process()
{
    SaveData();

    // check timers and manipulate data accordingly...
    // this system is currently inaccurate.  priority level: low
    if (++m_counter > 4) {  // every hour?  provided proc call is 15m
        m_counter = 0;
        // every [increment][time] save stat history data
        CompileData();
        //sEntityList.ResetStartTime();
    }
    /*
     * SELECT timeStamp, timeSpan, pcShots, pcMissiles, ramJobs, shipsSalvaged, pcBounties, npcBounties, oreMined, iskMarket FROM srvStatisticData
     * SELECT month, pcShots, pcMissiles, ramJobs, shipsSalvaged, pcBounties, npcBounties, oreMined, iskMarket FROM srvStatisticHistory
     */
}

void StatisticMgr::SaveData()
{
    m_data.span = sEntityList.GetMinutes();
    ManagerDB::SaveStatisticData(m_data);
}

void StatisticMgr::Add(uint8 key, double value)
{
    m_data.span = sEntityList.GetMinutes();
    switch(key) {
        case Stat::pcBounties:
            m_data.pcBounties += value;
            break;
        case Stat::npcBounties:
            m_data.npcBounties += value;
            break;
        case Stat::oreMined:
            m_data.oreMined += value;
            break;
        case Stat::iskMarket:
            m_data.iskMarket += value;
            break;
        default:
            sLog.Error("StatisticMgr::Add()", "Default reached for key %u.", key );
            break;
    }
}

void StatisticMgr::Increment(uint8 key)
{
    m_data.span = sEntityList.GetMinutes();
    switch(key) {
        case Stat::pcShots:
            ++m_data.pcShots;
            break;
        case Stat::ramJobs:
            ++m_data.ramJobs;
            break;
        case Stat::pcMissiles:
            ++m_data.pcMissiles;
            break;
        case Stat::shipsSalvaged:
            ++m_data.shipsSalvaged;
            break;
        case Stat::sitesScanned:
            ++m_data.sitesScanned;
            break;
        case Stat::probesLaunched:
            ++m_data.probesLaunched;
            break;
        default:
            sLog.Error("StatisticMgr::Increment()", "Default reached for key %u.", key );
            break;
    }
}

void StatisticMgr::PrintInfo()
{
    m_data.span = sEntityList.GetMinutes();
    sLog.Cyan("     StatisticMgr", " Time Span: %u minutes", m_data.span);
    sLog.Cyan("     StatisticMgr", " PC Shots Fired: %u", m_data.pcShots);
    sLog.Cyan("     StatisticMgr", " PC Missiles Fired: %u", m_data.pcMissiles);
    sLog.Cyan("     StatisticMgr", " Scan Probes Launched: %u", m_data.probesLaunched);
    sLog.Cyan("     StatisticMgr", " Cosmic Signals Scanned: %u", m_data.sitesScanned);
    sLog.Cyan("     StatisticMgr", " PC Bounties Paid: %.2f isk", m_data.pcBounties);
    sLog.Cyan("     StatisticMgr", " NPC Bounties Paid: %.2f isk", m_data.npcBounties);
    sLog.Cyan("     StatisticMgr", " Ore Mined: %.2f m3", m_data.oreMined);
    sLog.Cyan("     StatisticMgr", " ISK Spent in Market: %.2f isk", m_data.iskMarket);
    sLog.Cyan("     StatisticMgr", " Ships Salvaged: %u", m_data.shipsSalvaged);
    sLog.Cyan("     StatisticMgr", " R.A.M. Jobs: %u", m_data.ramJobs);
}

void StatisticMgr::CompileData()
{
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;
    ManagerDB::GetStatisticData(*res, sEntityList.GetStartTime());
    if (res->GetRowCount() > 0) {
        StatisticData data = StatisticData();
        while (res->GetRow(row)) {
            //SELECT pcShots, pcMissiles, ramJobs, shipsSalvaged, pcBounties, npcBounties, oreMined, iskMarket, sitesScanned, probesLaunched FROM srvStatisticData
            data.pcShots        += row.GetInt(0);
            data.pcMissiles     += row.GetInt(1);
            data.ramJobs        += row.GetInt(2);
            data.shipsSalvaged  += row.GetInt(3);
            data.pcBounties     += row.GetFloat(4);
            data.npcBounties    += row.GetFloat(5);
            data.oreMined       += row.GetFloat(6);
            data.iskMarket      += row.GetFloat(7);
            data.sitesScanned   += row.GetInt(8);
            data.probesLaunched += row.GetInt(9);
        }

        if (data.pcShots == 0)
            if (data.pcMissiles == 0)
                if (data.iskMarket == 0)
                    if (data.oreMined == 0)
                        if (data.shipsSalvaged == 0)
                            if (data.ramJobs == 0)
                                if (data.pcBounties == 0)
                                    if (data.npcBounties == 0)
                                        if (data.probesLaunched == 0)
                                            if (data.sitesScanned == 0)
                                                return;

        // data has been compiled for this running session.  save to history.
        ManagerDB::UpdateStatisticHistory(data);
    }

    SafeDelete(res);
}


