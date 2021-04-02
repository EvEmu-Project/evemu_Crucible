/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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
    Author:        Zhur
    Updates:    Positron96, Allan
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "market/MarketDB.h"

/*
 * MARKET__ERROR
 * MARKET__WARNING
 * MARKET__MESSAGE
 * MARKET__DEBUG
 * MARKET__TRACE
 * MARKET__DB_ERROR
 * MARKET__DB_TRACE
 */

PyRep *MarketDB::GetStationAsks(uint32 stationID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "    typeID, MIN(price) AS price, volRemaining, stationID "
        " FROM mktOrders "
        " WHERE stationID=%u"
        " GROUP BY typeID",
        //" LIMIT %u", sConfig.market.StationOrderLimit
        stationID))
    {
        codelog(MARKET__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    //NOTE: this SHOULD return a crazy dbutil.RowDict object which is
    //made up of packed blue.DBRow objects, but we do not understand
    //the marshalling of those well enough right now, and this object
    //provides the same interface. It is significantly bigger on the wire though.
    return DBResultToIndexRowset(res, "typeID");
}

PyRep *MarketDB::GetSystemAsks(uint32 solarSystemID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "    typeID, MIN(price) AS price, volRemaining, stationID "
        " FROM mktOrders "
        " WHERE solarSystemID=%u"
        " GROUP BY typeID",
        //" LIMIT %u",sConfig.market.SystemOrderLimit
        solarSystemID))
    {
        codelog(MARKET__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    //NOTE: this SHOULD return a crazy dbutil.RowDict object which is
    //made up of packed blue.DBRow objects, but we do not understand
    //the marshalling of those well enough right now, and this object
    //provides the same interface. It is significantly bigger on the wire though.
    return DBResultToIndexRowset(res, "typeID");
}

PyRep *MarketDB::GetRegionBest(uint32 regionID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "    typeID, MIN(price) AS price, volRemaining, stationID "
        " FROM mktOrders "
        " WHERE regionID=%u AND bid=%u"
        " GROUP BY typeID",
        //" LIMIT %u",sConfig.market.RegionOrderLimit
        regionID, Market::Type::Sell))
    {
        codelog(MARKET__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    //NOTE: this SHOULD return a crazy dbutil.RowDict object which is
    //made up of packed blue.DBRow objects, but we do not understand
    //the marshalling of those well enough right now, and this object
    //provides the same interface. It is significantly bigger on the wire though.
    return DBResultToIndexRowset(res, "typeID");
}

PyRep *MarketDB::GetOrders( uint32 regionID, uint16 typeID )
{
    // returns a tuple (sell, buy) of PyObjectEx with data in PyPackedRows

    PyTuple* tup = new PyTuple(2);
    DBQueryResult res;
    //query sell orders
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "    price, volRemaining, typeID, orderRange AS `range`, orderID,"
        "   volEntered, minVolume, bid, issued as issueDate, duration,"
        "   stationID, regionID, solarSystemID, jumps"
        " FROM mktOrders "
        " WHERE regionID=%u AND typeID=%u AND bid=%u", regionID, typeID, Market::Type::Sell))
    {
        codelog( MARKET__DB_ERROR, "Error in query: %s", res.error.c_str() );
        return nullptr;
    }
    _log(MARKET__DB_TRACE, "GetOrders() - Fetched %u sell orders for type %u", res.GetRowCount(), typeID);
    tup->SetItem(0, DBResultToCRowset( res ) );

    //query buy orders
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "    price, volRemaining, typeID, orderRange AS `range`, orderID,"
        "   volEntered, minVolume, bid, issued as issueDate, duration,"
        "   stationID, regionID, solarSystemID, jumps"
        " FROM mktOrders "
        " WHERE regionID=%u AND typeID=%u AND bid=%u", regionID, typeID, Market::Type::Buy))
    {
        codelog( MARKET__DB_ERROR, "Error in query: %s", res.error.c_str() );
        PyDecRef( tup );
        return nullptr;
    }
    _log(MARKET__DB_TRACE, "GetOrders() - Fetched %u buy orders for type %u", res.GetRowCount(), typeID);
    tup->SetItem(1, DBResultToCRowset( res ) );

    if (is_log_enabled(MARKET__DUMP))
        tup->Dump(MARKET__DUMP, "    ");
    return tup;
}

PyRep* MarketDB::GetOrdersForOwner(uint32 ownerID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   orderID, typeID, ownerID AS charID, regionID, stationID,"
        "   orderRange AS `range`, bid, price, volEntered, volRemaining,"
        "   issued as issueDate, minVolume, contraband,"
        "   duration, isCorp, solarSystemID, escrow"
        " FROM mktOrders"
        " WHERE ownerID=%u", ownerID))
    {
        codelog(MARKET__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    _log(MARKET__DB_TRACE, "GetOrdersForOwner() - Fetched %u buy orders for %u", res.GetRowCount(), ownerID);

    return DBResultToRowset(res);
}

PyRep *MarketDB::GetOrderRow(uint32 orderID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   price, volRemaining, typeID, orderRange AS `range`, orderID,"
        "   volEntered, minVolume, bid, issued as issueDate, duration,"
        "   stationID, regionID, solarSystemID, jumps"
        " FROM mktOrders"
        " WHERE orderID=%u", orderID))
    {
        codelog(MARKET__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(MARKET__ERROR, "Order %u not found.", orderID);
        return nullptr;
    }

    return DBRowToPackedRow(row);
}

//NOTE: needs a lot of work to implement orderRange
uint32 MarketDB::FindBuyOrder(Call_PlaceCharOrder &call) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT orderID"
        " FROM mktOrders"
        " WHERE bid=1"
        "  AND typeID=%u"
        "  AND stationID=%u"
        "  AND volRemaining >= %u"
        "  AND price > %.2f"
        " ORDER BY price DESC"
        " LIMIT 1;",
        call.typeID,
        call.stationID,
        call.quantity,
        call.price - 0.01/*, sConfig.market.FindBuyOrder*/))
    {
        codelog(MARKET__DB_ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (res.GetRow(row))
        return row.GetUInt(0);

    return 0;    //no order found.
}

uint32 MarketDB::FindSellOrder(Call_PlaceCharOrder &call)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT orderID"
        " FROM mktOrders"
        " WHERE bid=0"
        "  AND typeID=%u"
        "  AND stationID=%u"
        "  AND volRemaining >= %u"
        "  AND price < %.2f"
        " ORDER BY price ASC"
        " LIMIT 1;",
        call.typeID,
        call.stationID,
        call.quantity,
        call.price + 0.01/*, sConfig.market.FindSellOrder*/))
    {
        codelog(MARKET__DB_ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (res.GetRow(row))
        return row.GetUInt(0);

    return 0;
}

bool MarketDB::GetOrderInfo(uint32 orderID, Market::OrderInfo &oInfo) {
    //orderID, typeID, ownerID, regionID, stationID, orderRange, bid, price, escrow,
    // minVolume, volEntered, volRemaining, issued, contraband, duration, jumps, isCorp, accountKey, memberID
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        " volRemaining,"
        " price,"
        " typeID,"
        " stationID,"
        " regionID,"
        " ownerID,"
        " bid,"
        " isCorp"
        " FROM mktOrders"
        " WHERE orderID=%u",
        orderID))
    {
        _log(MARKET__DB_ERROR, "Error in query: %s.", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(MARKET__WARNING, "Order %u not found.", orderID);
        return false;
    }

    oInfo.quantity   = row.GetUInt(0);
    oInfo.price      = row.GetFloat(1);
    oInfo.typeID     = row.GetUInt(2);
    oInfo.stationID  = row.GetUInt(3);
    oInfo.regionID   = row.GetUInt(4);
    oInfo.ownerID    = row.GetUInt(5);
    oInfo.isBuy      = row.GetBool(6);
    oInfo.isCorp     = row.GetBool(7);

    return true;
}

//NOTE: this logic needs some work if there are multiple concurrent market services running at once.  there wont be.
bool MarketDB::AlterOrderQuantity(uint32 orderID, uint32 new_qty) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE mktOrders SET volRemaining = %u WHERE orderID = %u",  new_qty, orderID)) {
        _log(MARKET__DB_ERROR, "Error in query: %s.", err.c_str());
        return false;
    }
    return true;
}

bool MarketDB::AlterOrderPrice(uint32 orderID, double new_price) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE mktOrders SET price = %.2f WHERE orderID = %u", new_price, orderID)) {
        _log(MARKET__DB_ERROR, "Error in query: %s.", err.c_str());
        return false;
    }
    return true;
}

bool MarketDB::DeleteOrder(uint32 orderID) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "DELETE FROM mktOrders WHERE orderID = %u", orderID)) {
        _log(MARKET__DB_ERROR, "Error in query: %s.", err.c_str());
        return false;
    }
    return true;
}

uint32 MarketDB::StoreOrder(Market::SaveData &data) {
    DBerror err;
    uint32 orderID(0);
    if (!sDatabase.RunQueryLID(err, orderID,
        "INSERT INTO mktOrders ("
        " typeID, ownerID, regionID, stationID, solarSystemID, orderRange,"
        " bid, price, escrow, minVolume, volEntered, volRemaining,"
        " issued, contraband, duration, jumps, isCorp, accountKey, memberID"
        " ) VALUES ("
        "    %u, %u, %u, %u, %u, %u,"
        "    %u, %.2f, %.2f, %u, %u, %u,"
        "    %li, %u, %u, %u, %u, %u, %u"
        " )",
        data.typeID, data.ownerID, data.regionID, data.stationID, data.solarSystemID, data.orderRange,
        data.bid?1:0, data.price, data.escrow, data.minVolume, data.volEntered, data.volRemaining,
        data.issued, data.contraband?1:0, data.duration, data.jumps, data.isCorp?1:0, data.accountKey, data.memberID))
    {
        codelog(MARKET__DB_ERROR, "Error in query: %s", err.c_str());
        return 0;
    }

    return orderID;
}

PyRep *MarketDB::GetTransactions(uint32 clientID, Market::TxData& data) {
    //transactionID, transactionDate, typeID, keyID, quantity, price,
    //  transactionType, clientID, regionID, stationID, corpTransaction, characterID
    std::string typeID = "";
    if (data.typeID) {
        typeID = "AND typeID=";
        typeID += std::to_string(data.typeID);
    }
    std::string buy = "";
    if (data.isBuy > -1) {
        buy = "AND transactionType=";
        buy += std::to_string(data.isBuy);
    }
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   transactionID, transactionDate, typeID, keyID, quantity, price,"
        "   transactionType, clientID, regionID, stationID, corpTransaction, characterID"
        " FROM mktTransactions "
        " WHERE clientID=%u %s AND quantity>=%u AND price>=%.2f AND "
        " transactionDate>=%li %s AND keyID=%u AND characterID=%u",
        clientID, typeID.c_str(), data.quantity, data.price,
        data.time, buy.c_str(), data.accountKey, data.memberID))
    {
        codelog(MARKET__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

bool MarketDB::RecordTransaction(Market::TxData &data) {
    //transactionID, transactionDate, typeID, keyID, quantity, price,
    //  transactionType, clientID, regionID, stationID, corpTransaction, characterID
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO"
        " mktTransactions ("
        "    transactionDate, typeID, keyID, quantity, price,"
        "    transactionType, clientID, regionID, stationID, corpTransaction, characterID"
        " ) VALUES ("
        " %f, %u, %u, %u, %f,"
        " %u, %u, %u, %u, %u, %u)",
        GetFileTimeNow(), data.typeID, data.accountKey, data.quantity, data.price,
        data.isBuy > 0?1:0, data.clientID, data.regionID, data.stationID, data.isCorp?1:0, data.memberID))
    {
        codelog(MARKET__DB_ERROR, "Error in query: %s", err.c_str());
        return false;
    }
    return true;
}

PyRep *MarketDB::GetMarketGroups() {

    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT parentGroupID, marketGroupID, marketGroupName,"
        " description, graphicID, hasTypes, iconID, dataID, marketGroupNameID, descriptionID"
        " FROM invMarketGroups"))  {
        codelog(MARKET__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    DBRowDescriptor *header = new DBRowDescriptor(res);

    _log(MARKET__DB_TRACE, "GetMarketGroups header has %u columns.", header->ColumnCount());

    CFilterRowSet *filterRowset = new CFilterRowSet(&header);
    PyDict *keywords = filterRowset->GetKeywords();
    keywords->SetItemString("allowDuplicateCompoundKeys", PyStatic.NewFalse());
    keywords->SetItemString("indexName", PyStatic.NewNone());
    keywords->SetItemString("columnName", new PyString("parentGroupID"));

    DBResultRow row;
    std::map< int, PyRep* > tt;
    while( res.GetRow(row) ) {
        int parentGroupID(row.IsNull(0) ? -1 : row.GetUInt(0));
        PyRep* pid(nullptr);
        CRowSet*rowset(nullptr);
        if (tt.count(parentGroupID)) {
            pid = tt[parentGroupID];
            rowset = filterRowset->GetRowset(pid);
        } else {
            pid = parentGroupID != -1 ? (PyRep*)new PyInt(parentGroupID) : PyStatic.NewNone();
            tt[parentGroupID] = pid;
            rowset = filterRowset->NewRowset(pid);
        }

        PyPackedRow* pyrow = rowset->NewRow();
        pyrow->SetField((uint32)0, pid); //parentGroupID
        pyrow->SetField(1, new PyInt(row.GetUInt(1))); //marketGroupID
        pyrow->SetField(2, new PyString(row.GetText(2))); //marketGroupName
        pyrow->SetField(3, new PyString(row.GetText(3))); //description
        pyrow->SetField(4, row.IsNull(4) ? PyStatic.NewNone() : new PyInt(row.GetUInt(4))); //graphicID
        pyrow->SetField(5, new PyBool(row.GetBool(5))); //hasTypes
        pyrow->SetField(6, row.IsNull(6) ? PyStatic.NewNone() : new PyInt(row.GetUInt(6))); // iconID
        pyrow->SetField(7, new PyInt(row.GetUInt(7))); //dataID
        pyrow->SetField(8, new PyInt(row.GetUInt(8))); //marketGroupNameID
        pyrow->SetField(9, new PyInt(row.GetUInt(9))); //descriptionID
    }

    _log(MARKET__DB_TRACE, "GetMarketGroups returned %u keys.", filterRowset->GetKeyCount());
    if (is_log_enabled(MARKET__DB_TRACE))
        filterRowset->Dump(MARKET__DB_TRACE, "    ");

    return filterRowset;
}

int64 MarketDB::GetUpdateTime()
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT timeStamp FROM mktUpdates WHERE server = 1"))  {
        codelog(MARKET__DB_ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }
    DBResultRow row;
    if (res.GetRow(row))
        return row.GetInt64(0);

    return 0;
}

void MarketDB::SetUpdateTime(int64 setTime)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE mktUpdates SET timeStamp = %li WHERE server = 1", setTime);
}

/** @todo this needs work for better logic.   may need to pull data from transactions */
void MarketDB::UpdateHistory()
{
    //  'date'  needs to be an actual column to pull data from....
    DBerror err;
    sDatabase.RunQuery(err,
                   "INSERT INTO"
                   "    mktHistory"
                   "     (regionID, typeID, historyDate, lowPrice, highPrice, avgPrice, volume, orders)"
                   " SELECT"
                   "    regionID,"
                   "    typeID,"
                   "    ((UNIX_TIMESTAMP(date) + 11644473600) * 10000000),"
                   "    price,"
                   "    price,"
                   "    price,"
                   "    amtEntered,"
                   "    COUNT(DISTINCT typeID)"
                   " FROM mktData");
}


/*  data retrieval for updating base pricing */

void MarketDB::GetShipIDs(std::map< uint16, Inv::TypeData >& data)
{
    DBQueryResult res;
    DBResultRow row;
    // 178 ships using this query
    sDatabase.RunQuery(res,
                "SELECT t.typeID "
                " FROM invTypes AS t "
                " LEFT JOIN invGroups AS g USING (groupID)"
                " WHERE g.categoryID = %u",
                //" AND g.useBasePrice = 1"
                //" AND t.published = 1",
                EVEDB::invCategories::Ship);

    //sDatabase.RunQuery(res, "SELECT typeID FROM invTypes WHERE groupID = %u AND published = 1", EVEDB::invGroups::Frigate);
    while (res.GetRow(row))
        data[row.GetInt(0)] = Inv::TypeData();
}

void MarketDB::GetManufacturedItems(std::map< uint16, Inv::TypeData >& data)
{
    DBQueryResult res;
    DBResultRow row;
    //6602 items in this query
    if (!sDatabase.RunQuery(res, "SELECT DISTINCT typeID FROM invTypeMaterials"))
        codelog(DATABASE__ERROR, "Error in GetRAMMaterials query: %s", res.error.c_str());

    while (res.GetRow(row))
        data[row.GetInt(0)] = Inv::TypeData();
}

void MarketDB::GetMaterialPrices(std::map< uint16, Market::matlData >& data)
{
    DBQueryResult res;
    DBResultRow row;
    std::map< uint16, Market::matlData >::iterator itr;
    for (itr = data.begin(); itr != data.end(); ++itr) {
        sDatabase.RunQuery(res, "SELECT basePrice FROM invTypes WHERE typeID = %u", itr->first);
        if (res.GetRow(row))
            itr->second.price = (row.GetFloat(0) * 1.05);
    }
}

void MarketDB::GetMineralPrices(std::map< uint16, Market::matlData >& data)
{
    DBQueryResult res;
    DBResultRow row;
    std::map< uint16, Market::matlData >::iterator itr;
    for (itr = data.begin(); itr != data.end(); ++itr) {
        sDatabase.RunQuery(res, "SELECT basePrice FROM invTypes WHERE typeID = %u", itr->first);
        if (res.GetRow(row))
            itr->second.price = (row.GetFloat(0) * 1.15);
    }

    /*  mineral prices in first column from rens 31/5/2010 @ 17:30  logged by me from IGB
     * second price column is from Grismar 16/2/07
     * third price column is from ccp market dump, filtered and averaged for Crucible era from all regions
     * typeID   Name            2010        2007       2012
     * 34      Tritanium        2.70        2.37        2.72315
     * 35      Pyerite          5.80        4.00        4.90957
     * 36      Mexallon        26.90       21.93       29.7163
     * 37      Isogen          49.16       64.06       59.1943
     * 38      Nocxium         99.02       93.76       94.227
     * 39      Zydrine       1315.03     2347.36     1290.39
     * 40      Megacyte      2650.00     3989.06     2707.67
     * 11399   Morphite      5407.68    14291.00     4943.2
     *
     */
}

void MarketDB::UpdateInvPrice(std::map< uint16, Inv::TypeData >& data)
{
    DBerror err;
    for (auto cur : data) {
        if (cur.second.basePrice < 0.01) {
            sLog.Error("     SetBasePrice", "Calculated price for %s(%u) is 0", \
                    cur.second.name.c_str(), cur.first);
        } else {
            sDatabase.RunQuery(err, "UPDATE invTypes SET basePrice=%f WHERE typeID= %u", cur.second.basePrice, cur.first);
        }
    }
}

void MarketDB::UpdateMktPrice(std::map< uint16, Market::matlData >& data)
{
    DBerror err;
    for (auto cur : data)
        sDatabase.RunQuery(err, "UPDATE invTypes SET basePrice=%f WHERE typeID= %u", cur.second.price, cur.first);
}

void MarketDB::GetCruPriceAvg(std::map< uint16, Inv::TypeData >& data)
{
    DBQueryResult res;
    DBResultRow row;
    std::map< uint16, Inv::TypeData>::iterator itr;
    for (itr = data.begin(); itr != data.end(); ++itr) {
        sDatabase.RunQuery(res, "SELECT AVG(avgPrice) FROM CruciblePriceHistory WHERE typeID = %u", itr->first);
        if (res.GetRow(row))
            itr->second.basePrice = (row.IsNull(0) ? 0 : row.GetFloat(0));
    }
}
