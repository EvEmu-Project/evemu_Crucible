
 /**
  * @name StandingMgr.cpp
  *   memory object caching system for managing and saving standings data
  *   methods and functions relating to manipulation of standings
  *
  * @Author:        Allan
  * @date:      14 Novemeber 2018
  *
  */


#ifndef EVE_STANDING_STANDINGMGR_H
#define EVE_STANDING_STANDINGMGR_H

#include "../eve-server.h"

#include "../../eve-common/EVE_Standings.h"

#include "standing/StandingDB.h"


class StandingMgr
: public Singleton< StandingMgr >
{
public:
    StandingMgr();
    ~StandingMgr();

    int                 Initialize();

    void                Clear();
    void                Close()                         { Clear(); }
    void                GetInfo();

    PyObjectEx*         GetFactionStandings()           { PyIncRef(m_factionStandings); return m_factionStandings; }



    void                UpdateStandings(uint32 fromID, uint32 toID, uint16 eventType, double amount, std::string msg);

protected:
    void                Populate();

private:
    PyObjectEx*         m_factionStandings;

};


//Singleton
#define sStandingMgr \
( StandingMgr::get() )



#endif  // EVE_STANDING_STANDINGMGR_H