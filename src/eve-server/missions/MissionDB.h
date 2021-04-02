
 /**
  * @name MissionDB.cpp
  *   memory object caching system for managing and saving ingame data specific to missions
  *
  * @Author:        Allan
  * @date:      24 June 2018
  *
  */


#ifndef _EVE_SERVER_MISSION_DATABASE_H__
#define _EVE_SERVER_MISSION_DATABASE_H__

#include "../ServiceDB.h"
#include "../eve-server.h"
#include "../../eve-common/EVE_Missions.h"

class MissionDB
{
public:

    static void CreateOfferID(MissionOffer& data);
    static void LoadOpenOffers(DBQueryResult& res);
    static void LoadMiningData(DBQueryResult& res);
    static void LoadMissionData(DBQueryResult& res);
    static void LoadCourierData(DBQueryResult& res);
    static void LoadClosedOffers(DBQueryResult& res);
    static void LoadMissionBookMark(DBQueryResult& res, std::vector<int32>& bmIDs);

    static void UpdateMissionOffer(MissionOffer& data);

    static void DeleteOffer(MissionOffer& data);
    static void RemoveMissionItem(uint32 charID, uint16 typeID, uint32 qty);   // this is for removing mission items from offline clients.

protected:

};



#endif  // _EVE_SERVER_MISSION_DATABASE_H__