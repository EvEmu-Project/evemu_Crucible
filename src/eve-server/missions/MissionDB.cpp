
 /**
  * @name MissionDB.cpp
  *   memory object caching system for managing and saving ingame data specific to missions
  *
  * @Author:        Allan
  * @date:      24 June 2018
  *
  */


#include "missions/MissionDB.h"
#include "database/EVEDBUtils.h"


void MissionDB::LoadMissionData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT id, briefingID, name, level, typeID, important, storyline, raceID, constellationID, corporationID, dungeonID,"
        " rewardISK, rewardItemID, rewardISK, rewardItemQty, bonusISK, bonusTime FROM agtMissions WHERE briefingID > 0 AND rewardISK > 0"))
        codelog(DATABASE__ERROR, "Error in LoadMissionData query: %s", res.error.c_str());
}

void MissionDB::LoadCourierData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT q.id, q.briefingID, q.name, q.level, q.typeID, q.important, q.storyline, q.itemTypeID, q.itemQty, it.volume, q.rewardISK, q.rewardItemID,"
        " q.rewardItemQty, q.bonusISK, q.bonusTime, q.sysRange, q.raceID"
        " FROM qstCourier AS q LEFT JOIN invTypes AS it ON it.typeID = itemTypeID WHERE briefingID > 0 AND itemTypeID > 0 AND rewardISK > 0"))
        codelog(DATABASE__ERROR, "Error in LoadCourierData query: %s", res.error.c_str());
}

void MissionDB::LoadMiningData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT q.id, q.briefingID, q.name, q.level, q.typeID, q.important, q.storyline, q.itemTypeID, q.itemQty, it.volume, q.rewardISK, q.rewardItemID,"
        " q.rewardItemQty, q.bonusISK, q.bonusTime, q.sysRange, q.raceID"
        " FROM qstMining AS q LEFT JOIN invTypes AS it ON it.typeID = itemTypeID WHERE briefingID > 0 AND itemTypeID > 0 AND rewardISK > 0"))
        codelog(DATABASE__ERROR, "Error in LoadMiningData query: %s", res.error.c_str());
}

void MissionDB::CreateOfferID(MissionOffer& data)
{
    DBerror err;
    uint32 uid = 0;
    if (!sDatabase.RunQueryLID(err, uid,
        "INSERT INTO agtOffers(acceptFee, agentID, characterID, courierAmount, courierTypeID, courierVolume, dateAccepted, dateIssued, destinationID, destinationTypeID, "
        " destinationOwnerID, destinationSystemID, expiryTime, important, storyline, missionID, briefingID, name, offerID, originID, originOwnerID, originSystemID,"
        " remoteCompletable, remoteOfferable, rewardISK, rewardItemID, rewardItemQty, rewardLP, bonusISK, bonusTime, stateID, typeID, dungeonLocationID, dungeonSolarSystemID)"
        " VALUES ("
        " %u, %u, %u, %u, %u, %f, %f, %f, %u,"
        " %u, %u, %u, %f, %i, %u,"
        " %u, %u, '%s', %u, %u, %u, %u, %i,"
        " %i, %u, %u, %u, %u, %u, %u, %u,"
        " %u, %u, %u)",
            data.acceptFee, data.agentID, data.characterID, data.courierAmount, data.courierTypeID, data.courierItemVolume, data.dateAccepted, data.dateIssued, data.destinationID,
            data.destinationTypeID, data.destinationOwnerID, data.destinationSystemID, data.expiryTime, (data.important?1:0), data.storyline,
            data.missionID, data.briefingID, data.name.c_str(), data.offerID, data.originID, data.originOwnerID, data.originSystemID,
            (data.remoteCompletable?1:0), (data.remoteOfferable?1:0), data.rewardISK, data.rewardItemID, data.rewardItemQty,data.rewardLP, data.bonusISK, data.bonusTime,
            data.stateID, data.typeID, data.dungeonLocationID, data.dungeonSolarSystemID))
    {
        codelog(DATABASE__ERROR, "Failed to insert new MissionOffer: %s", err.c_str());
        return;
    }

    data.offerID = uid;
}

void MissionDB::DeleteOffer(MissionOffer& data)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM agtOffers WHERE offerID = %u", data.offerID);
}


void MissionDB::UpdateMissionOffer(MissionOffer& data)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE agtOffers SET stateID = %u, dateAccepted = %f, dateCompleted = %f, expiryTime = %f WHERE offerID = %u",
        data.stateID, data.dateAccepted, data.dateCompleted, data.expiryTime, data.offerID))
    {
        codelog(DATABASE__ERROR, "Failed to update MissionOffer: %s", err.c_str());
    }
}

void MissionDB::LoadOpenOffers(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT acceptFee, agentID, characterID, courierAmount, courierTypeID, courierVolume, dateAccepted, dateIssued, destinationID, destinationTypeID, destinationOwnerID, destinationSystemID,"
        " expiryTime, important, storyline, missionID, briefingID, name, offerID, originID, originOwnerID, originSystemID, remoteCompletable, remoteOfferable, "
        " rewardISK, rewardItemID, rewardItemQty, rewardLP, bonusISK, bonusTime, stateID, typeID, dungeonLocationID, dungeonSolarSystemID "
        " FROM agtOffers WHERE dateCompleted = 0 AND stateID < 3"))
        codelog(DATABASE__ERROR, "Error in LoadOpenOffers query: %s", res.error.c_str());
}


void MissionDB::LoadClosedOffers(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT agentID, characterID, courierAmount, courierTypeID, dateAccepted, dateCompleted, dateIssued, destinationID, expiryTime, important, storyline, missionID, name,"
        " offerID, originID, rewardISK, rewardItemID, rewardItemQty, rewardLP, stateID, typeID FROM agtOffers WHERE dateCompleted > 0 OR expiryTime > %f OR stateID > 2", GetFileTimeNow()))
        codelog(DATABASE__ERROR, "Error in LoadClosedOffers query: %s", res.error.c_str());
}

void MissionDB::LoadMissionBookMark(DBQueryResult& res, std::vector<int32>& bmIDs)
{
    std::string ids = "";
    ListToINString(bmIDs, ids);
    if (!sDatabase.RunQuery(res,
        "SELECT bookmarkID, ownerID, itemID, typeID, memo, created, x, y, z, locationID, note, creatorID, folderID"
        " FROM bookmarks WHERE bookmarkID IN (%s)", ids.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
    }
}

void MissionDB::RemoveMissionItem(uint32 charID, uint16 typeID, uint32 qty)
{
    //  this may get a bit complicated if the items are split.
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT itemID, quantity FROM entity WHERE typeID = %u AND ownerID = %u", typeID, charID);

    DBResultRow row;
    std::map<uint32, uint16> map;
    while (res.GetRow(row)) {
        // make map of all items of 'typeID'
        map.emplace(row.GetInt(0), row.GetInt(1));
    }

    DBerror err;
    for (auto cur : map) {
        if (qty < 1)
            break;
        if (cur.second <= qty) {
            qty -= cur.second;
            sDatabase.RunQuery(err, "DELETE FROM entity WHERE itemID = %u", cur.first);
        } else if (cur.second > qty) {
            sDatabase.RunQuery(err, "UPDATE entity SET quantity = %u WHERE itemID = %u", qty, cur.first);
            qty = 0;
        }
    }

}

