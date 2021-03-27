
/*  EVE_Market.h
 *    enumerators and other defines for Market system
 *
 *  Allan
 *  19 October 2020
 */


#ifndef EVE_MARKET_H
#define EVE_MARKET_H


#include "eve-common.h"

/* market range
 * rangeStation = -1
 * rangeSolarSystem = 0
 * rangeConstellation = 4
 * rangeRegion = 32767
 */


namespace Market {
    namespace Action {
        enum {
            Add     = 1,
            Modify  = 2,
            Expiry  = 3
        };
    }

    namespace Type {
        enum {
            Sell = 0,
            Buy = 1
        };
    }

    namespace Duration {
        enum {
            Immediate = 0,
            Day = 1,
            ThreeDays = 3,
            Week = 7,
            TwoWeeks = 14,      // default
            Month = 30,
            ThreeMonths = 90
        };
    }

    // this is jumps from current system/station
    namespace Range {
        enum {
            Station       = -1,
            System        = 0,
            Constellation = 4,
            Region        = 32767
            //Unreachable   = 1000000
        };
    }

    namespace QtyStatus {
        enum {
            Invalid = 0,
            Complete = 1,
            Over = 2,
            Under = 3
        };

    }
    // used to query/save transaction data
    //sellBuy, typeID, clientID, quantity, fromDate, maxPrice, minPrice, accountKey, memberID
    struct TxData {
        bool isCorp :1;
        int8 isBuy;
        uint16 typeID;
        uint32 clientID;
        uint32 stationID;
        uint32 regionID;
        uint32 quantity;
        uint32 accountKey;   // corp account key (default 1000 (cash))
        uint32 memberID;     // corp member that placed order (0 for char order)
        int64 time;
        float price;
    };

    // used to save order data
    struct SaveData {
        /*
         * orderID, typeID, ownerID, regionID, stationID
         * orderRange, bid, price, escrow, minVolume, volEntered, volRemaining,
         * issued, contraband, duration, jumps, isCorp, accountKey, memberID
         */
        bool bid :1;
        bool isCorp :1;
        bool contraband :1;
        uint8 jumps;
        int16 orderRange;
        uint16 typeID;
        uint16 accountKey;
        uint32 orderID;
        uint32 ownerID;
        uint32 regionID;
        uint32 stationID;
        uint32 solarSystemID;
        uint32 minVolume;
        uint32 volEntered;
        uint32 volRemaining;
        uint32 duration;
        uint32 memberID;
        int64 issued;
        float price;
        float escrow;
    };

    // used to query sell orders when buy is requested
    struct OrderInfo {
        //orderID, typeID, charID, regionID, stationID, orderRange, bid, price, escrow,
        // minVolume, volEntered, volRemaining, issued, contraband, duration, jumps, isCorp, accountKey, memberID
        bool isBuy :1;
        bool isCorp :1;
        uint16 typeID;
        uint32 orderID;
        uint32 ownerID;
        uint32 stationID;
        uint32 regionID;
        uint32 quantity;
        uint32 accountKey;   // corp account key (default 1000 (cash))
        uint32 memberID;     // corp member that placed order (0 for char order)
        float price;
    };

    // POD structure for mineral data used in pricing method
    struct matlData {
        uint16 typeID;
        float price;
        std::string name;
    };

}

/*
            MarketTransaction = 2,     // * transactionID
            MarketEscrow = 42,
            MarketFinePaid = 44,
            Brokerfee = 46,     // * station ownerID
            TransactionTax = 54,     // *
        */


#endif  // EVE_MARKET_H


/*
 * {'FullPath': u'UI/Market/Corporation', 'messageID': 251301, 'label': u'AccessDeniedNeedAccountantOrTraderRole'}(u'You do not have the role <b>Accountant</b> or <b>Trader</b>.', None, None)
 * {'FullPath': u'UI/Market/Corporation', 'messageID': 251303, 'label': u'AccessDeniedNeedAddRights'}(u'This is a delivery bay. You can only pick up items here, which have been bought off the market or returned from the market.', None, None)
 * {'FullPath': u'UI/Market/Corporation', 'messageID': 251307, 'label': u'AccessDeniedNeedJuniorAccountantOrTraderRole'}(u'You do not have the role <b>Accountant</b> or <b>Junior Accountant</b> or <b>Trader</b>.', None, None)
 *
 */



/* this is data displayed on bottom of 'my orders' window
 *
 *    def GetSkillLimits(self):
 *        limits = {}
 *        currentOpen = 0
 *        myskills = sm.GetService('skills').MySkillLevelsByID()
 *        retailLevel = myskills.get(const.typeRetail, 0)
 *        tradeLevel = myskills.get(const.typeTrade, 0)
 *        wholeSaleLevel = myskills.get(const.typeWholesale, 0)
 *        accountingLevel = myskills.get(const.typeAccounting, 0)
 *        brokerLevel = myskills.get(const.typeBrokerRelations, 0)
 *        tycoonLevel = myskills.get(const.typeTycoon, 0)
 *        marginTradingLevel = myskills.get(const.typeMarginTrading, 0)
 *        marketingLevel = myskills.get(const.typeMarketing, 0)
 *        procurementLevel = myskills.get(const.typeProcurement, 0)
 *        visibilityLevel = myskills.get(const.typeVisibility, 0)
 *        daytradingLevel = myskills.get(const.typeDaytrading, 0)
 *        maxOrderCount = 5 + tradeLevel * 4 + retailLevel * 8 + wholeSaleLevel * 16 + tycoonLevel * 32
 *        limits['cnt'] = maxOrderCount
 *        commissionPercentage = const.marketCommissionPercentage / 100.0
 *        commissionPercentage *= 1 - brokerLevel * 0.05
 *        transactionTax = const.mktTransactionTax / 100.0
 *        transactionTax *= 1 - accountingLevel * 0.1
 *        limits['fee'] = commissionPercentage
 *        limits['acc'] = transactionTax
 *        limits['ask'] = jumpsPerSkillLevel[marketingLevel]
 *        limits['bid'] = jumpsPerSkillLevel[procurementLevel]
 *        limits['vis'] = jumpsPerSkillLevel[visibilityLevel]
 *        limits['mod'] = jumpsPerSkillLevel[daytradingLevel]
 *        limits['esc'] = 0.75 ** marginTradingLevel
 *        return limits
 */