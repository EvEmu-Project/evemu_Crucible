
 /**
  * @name StandingMgr.cpp
  *   memory object caching system for managing and saving standings data
  *   methods and functions relating to manipulation of standings
  *
  * @Author:        Allan
  * @date:      14 Novemeber 2018
  *
  */


#include "StandingMgr.h"

/*
 * STANDING__ERROR
 * STANDING__WARNING
 * STANDING__MESSAGE
 * STANDING__DEBUG
 * STANDING__INFO
 * STANDING__TRACE
 * STANDING__DUMP
 * STANDING__RSPDUMP
 */


StandingMgr::StandingMgr()
: m_factionStandings(nullptr)
{

}

StandingMgr::~StandingMgr()
{

}

void StandingMgr::Clear()
{
    PySafeDecRef(m_factionStandings);
}

int StandingMgr::Initialize()
{
    Populate();
    sLog.Blue("      StandingMgr", "Standings Manager Initialized.");
    return 1;
}

void StandingMgr::GetInfo()
{

}

void StandingMgr::Populate()
{
    m_factionStandings = StandingDB::GetFactionStandings();
    if (m_factionStandings == nullptr)
        sLog.Error("      StandingMgr", "m_factionStandings is null");

}

void StandingMgr::UpdateStandings(uint32 fromID, uint32 toID, uint16 eventType, double amount, std::string msg)
{
    StandingDB::UpdateStanding(fromID, toID, amount);
    StandingDB::SaveStandingChanges(fromID, toID, eventType, amount, msg);
}

