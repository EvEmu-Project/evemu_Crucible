using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Attributes;
using System.ComponentModel;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Stations services as gotten from the data dumps (staServices table).
    /// </summary>
    [Flags]
    public enum StationServices
    {
        None = 0,

        [Header("Bounty Missions")]
        BountyMissions = 1,

        [Header("Assassination Missions")]
        AssassinationMissions = 0x2,

        [Header("Courier Missions")]
        CourierMissions = 0x4,

        [Header("Interbus")]
        Interbus = 0x8,

        [Header("Reprocessing Plant")]
        ReprocessingPlant = 0x10,

        [Header("Refinery")]
        Refinery = 0x20,

        [Header("Market")]
        Market = 0x40,

        [Header("Black Market")]
        BlackMarket = 0x80,

        [Header("Stock Exchange")]
        StockExchange = 0x100,

        [Header("Cloning")]
        Cloning = 0x200,

        [Header("Surgery")]
        Surgery = 0x400,

        [Header("DNA Therapy")]
        DNATherapy = 0x800,

        [Header("Repair Facilities")]
        RepairFacilities = 0x1000,

        [Header("Factory")]
        Factory = 0x2000,

        [Header("Laboratory")]
        Laboratory = 0x4000,

        [Header("Gambling")]
        Gambling = 0x8000,

        [Header("Fitting")]
        Fitting = 0x10000,

        [Header("Paintshop")]
        Paintshop = 0x20000,

        [Header("News")]
        News = 0x40000,

        [Header("Storage")]
        Storage = 0x80000,

        [Header("Insurance")]
        [Description("Used to buy insurance for ships.")]
        Insurance = 0x100000,

        [Header("Docking")]
        Docking = 0x200000,

        [Header("Office Rental")]
        OfficeRental = 0x400000,

        [Header("Jump Clone Facility")]
        JumpCloneFacility = 0x800000,

        [Header("Loyalty Point Store")]
        LoyaltyPointStore = 0x1000000,

        [Header("Navy Offices")]
        NavyOffices = 0x2000000
    }
}
