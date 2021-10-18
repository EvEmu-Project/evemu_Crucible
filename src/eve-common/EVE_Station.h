/*
 *  EVE_Station.h
 *   station-specific data
 *
 */

#ifndef EVE_STATION_H
#define EVE_STATION_H


namespace Station {
    enum {
        BountyMissions = 1,
        AssassinationMissions = 2,
        CourierMissions = 4,
        Interbus = 8,
        ReprocessingPlant = 16,
        Refinery = 32,
        Market = 64,
        BlackMarket = 128,
        StockExchange = 256,
        Cloning = 512,
        Surgery = 1024,
        DNATherapy = 2048,
        RepairFacilities = 4096,
        Factory	= 8192,
        Laboratory = 16384,
        Gambling = 32768,
        Fitting = 65536,
        Paintshop = 131072,
        News = 262144,
        Storage = 524288,
        Insurance = 1048576,
        Docking = 2097152,
        OfficeRental = 4194304,
        JumpCloneFacility = 8388608,
        LoyaltyPointStore = 16777216,
        NavyOffices = 33554432
    };
}


#endif  // EVE_STATION_H
