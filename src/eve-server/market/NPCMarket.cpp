/*
 * File:   NPCMarket.cpp
 * Author: RobR
 *
 * Created on June 10, 2014, 5:13 AM
 */

#include "eve-server.h"
#include "NPCMarket.h"

NPCMarket::NPCMarket()
{
    AddMemberParser("npcMarketOrders", &NPCMarket::ProcessNPCOrders);
}

void NPCMarket::CreateNPCMarketFromFile(std::string filename)
{
    sLog.White("NPCMarket", "Regenerating NPC market from file %s", filename.c_str());
    DBerror err;

    // delete all old market orders.
    // TO-DO: make this safe for concurrency.
    sLog.White("NPCMarket", "Deleting old NPC orders.");
    if (!sDatabase.RunQuery(err, "DELETE FROM market_orders WHERE duration > 90"))
    {
        _log(MARKET__ERROR, "Error in query: %s.", err.c_str());
        sLog.White("NPCMarket", "Failed to clear old NPC market.");
        return;
    }

    sLog.White("NPCMarket", "Reading NPC Market xml file %s", filename.c_str());
    // Create a NPCMarket object to process the file.
    NPCMarket npcMarket;
    const char *NPC_MARKET_FILE = (std::string(EVEMU_ROOT) + filename).c_str();
    // Process the file.
    if (!npcMarket.ParseFile(NPC_MARKET_FILE))
    {
        sLog.Warning("Failed to generate NPC market from %s.", NPC_MARKET_FILE);
    }
    sLog.White("NPCMarket", "NPC market created from file %s", filename.c_str());
}

bool NPCMarket::ProcessNPCOrders(const TiXmlElement* ele)
{
    // add parsers for the child properties.
    AddMemberParser("Station", &NPCMarket::ProcessStation);
    AddValueParser("StationID", StationID);
    AddMemberParser("Order", &NPCMarket::ProcessOrder);
    AddValueParser("TypeID", order.TypeID);
    AddValueParser("Qty", order.Qty);
    AddValueParser("Price", order.Price);
    AddValueParser("Bid", order.Bid);
    // parse the <npcMarketOrders> element
    const bool result = ParseElementChildren(ele);
    RemoveParser("TypeID");
    RemoveParser("Qty");
    RemoveParser("Price");
    RemoveParser("Bid");
    RemoveParser("Order");
    RemoveParser("StationID");
    RemoveParser("Station");
    return result;
}

bool NPCMarket::ProcessStation(const TiXmlElement* ele)
{
    static int nStations = 0;
    newOrders.clear();
    StationID = 0;
    // parse the <station> element
    const bool result = ParseElementChildren(ele);
    if(result == false)
      return false;

    // done if there were no orders.
    if (newOrders.size() == 0 || StationID == 0)
        return true;

    // count how many stations have been found.
    nStations++;
    sLog.White("NPCMarket", "%d: Adding %d orders to stationID= %d", nStations, newOrders.size(), StationID);

    DBerror err;

    // get the solar system and region IDs.
    StationData data = StationData();
    if (!stDataMgr.GetStationData(StationID, data)) {
        _log(DATA__ERROR, "Failed to retrieve data for station %u", StationID);
        newOrders.clear();
        // adding these market orders failed but that does not mean the xml is at fault.
        // return a successful processing of the xml.
        return true;
    }

    uint32 orderID;
    std::ostringstream values;
    int64 trnTime = GetFileTimeNow();
    bool first = true;
    // create a single insert command to improve sql performance.
    std::list<stationOrder>::iterator itr = newOrders.begin();
    while (itr != newOrders.end())
    {
        if (!first)
            values << ", ";
        first = false;
        values << "(";
        values << (*itr).TypeID;
        values << ", 0"; // clientID
        values << ", " << data.regionID;
        values << ", " << StationID;
        values << ", 0"; // orderRange
        values << ", " << (*itr).Bid;
        values << ", " << (*itr).Price;
        values << ", " << (*itr).Qty;
        values << ", " << (*itr).Qty;
        values << ", " << trnTime;
        values << ", 1"; // minVolume
        values << ", 0"; // contraband
        values << ", 0"; // accountID
        values << ", 365"; // duration
        values << ", 0"; // isCorp
        values << ", " << data.systemID;
        values << ", 0"; // escrow
        values << ", 1"; // jumps
        values << ")";
        itr++;
    }
    // process the sql command.
    if (!sDatabase.RunQueryLID(err, orderID,
                               "INSERT INTO market_orders ("
                               "    typeID, charID, regionID, stationID,"
                               "    `range`, bid, price, volEntered, volRemaining, issued,"
                               "    minVolume, contraband, accountID, duration,"
                               "    isCorp, solarSystemID, escrow, jumps "
                               " ) VALUES %s",
                               values.str().c_str()
                               ))
    {
        codelog(MARKET__ERROR, "Error in query: %s", err.c_str());
    }
    newOrders.clear();

    return true;
}

bool NPCMarket::ProcessOrder(const TiXmlElement* ele)
{
    order.Price = 0;
    order.Bid = 0;
    order.TypeID = 0;
    order.Qty = 0;
    // parse the <order> element
    const bool result = ParseElementChildren(ele);
    // if all of the order information was found add it to the list of new orders.
    if (order.Price != 0 && order.Qty != 0 && order.TypeID != 0)
        newOrders.push_back(order);
    return result;
}
