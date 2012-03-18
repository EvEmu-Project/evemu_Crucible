using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Provides constants about the CCP databases.
    /// </summary>
    public static class DBConstants
    {

        #region Collections

        public static readonly int[] RequiredSkillLevelPropertyIDs = new int[] { 277, 278, 279, 1286, 1287, 1288 };
        public static readonly int[] RequiredSkillPropertyIDs = new int[] { 182, 183, 184, 1285, 1289, 1290 };
        public static readonly int[] LauncherGroupIDs = new int[] { 137, 602, 603 };
        public static readonly int[] ChargeGroupIDs = new int[] { 604, 605, 606, 609, 610 };
        public static readonly int[] CanFitShipGroupIDs = new int[] { 1298, 1299, 1300, 1301 };
        public static readonly int[] ModuleShipGroupIDs = new int[] { 666, 667, 668 };
        public static readonly int[] SpecialisationAsteroidGroupIDs = new int[] { 781 };
        public static readonly int[] ReactionGroupIDs = new int[] { 842, 843 };
        public static readonly int[] PosCargobayAcceptGroupIDs = new int[] { 1352 };

        public static readonly int[] CompressionBlueprintsGroupIDs = new int[] { 1042, 1043 };
        public static readonly int[] IndustryModifyingPropertyIDs = new int[] { 440, 451, 452, 453, 468 };

        public static readonly int[] StategicComponentsGroupIDs = new int[] { 1112, 1138 };

        public static readonly int[] SmallToXLargeShipsGroupIDs = new int[]
            { 5, 6, 7, 8, 391, 464, 469, 494, 761, 766, 817, 840, 1047, 1310};

        public static readonly int[] CapitalShipsGroupIDs = new int[] { 761, 766, 812, 817 };

        public static readonly int[] AdvancedSmallToLargeShipsGroupIDs = new int[]
            { 399, 420, 432, 437, 448, 629, 822, 823, 824, 874, 1065, 1070, 1075, 1080, 1089 };

        #endregion


        #region Effect IDs

        public const int LowSlotEffectID = 11;
        public const int MedSlotEffectID = 13;
        public const int HiSlotEffectID = 12;

        #endregion


        #region Attribute types IDs (Properties)

        public const int MetaLevelPropertyID = 633;
        public const int CargoCapacityPropertyID = 38;
        public const int VolumePropertyID = 161;
        public const int MassPropertyID = 4;
        public const int TechLevelPropertyID = 422;
        public const int MetaGroupPropertyID = 1692;

        public const int ImplantSlotPropertyID = 331;
        public const int CharismaModifierPropertyID = 175;
        public const int IntelligenceModifierPropertyID = 176;
        public const int MemoryModifierPropertyID = 177;
        public const int PerceptionModifierPropertyID = 178;
        public const int WillpowerModifierPropertyID = 179;

        public const int ReprocessingSkillPropertyID = 790;

        public const int ShipBonusPirateFactionPropertyID = 793;

        public const int WarpSpeedMultiplierPropertyID = 600;
        public const int ShipWarpSpeedPropertyID = 1281;

        public const int CPUOutputPropertyID = 48;
        public const int PGOutputPropertyID = 11;

        public const int CPUNeedPropertyID = 50;
        public const int PGNeedPropertyID = 30;

        #endregion


        #region Category IDs

        public const int BlueprintCategoryID = 9;
        public const int ImplantCategoryID = 20;
        public const int SkillCategoryID = 16;

        #endregion


        #region Group IDs

        public const int PlanetGroupID = 7;
        public const int CorporationManagementSkillsGroupID = 266;
        public const int SocialSkillsGroupID = 278;
        public const int TradeSkillsGroupID = 274;
        public const int SubsystemsGroupID = 989;
        public const int CyberLearningImplantsGroupID = 745;

        #endregion


        #region Market group IDs

        public const int ShipsGroupID = 4;
        public const int SkillGroupID = 150;
        public const int DronesGroupID = 157;
        public const int BlueprintsGroupID = 2;
        public const int ImplantsGroupID = 27;
        public const int ShipsBlueprintsGroupID = 204;
        public const int ComponentsGroupID = 475;
        public const int ShipEquipmentGroupID = 9;
        public const int AdvancedSubsystemsGroupID = 1112;
        public const int AmmosAndChargesGroupID = 11;
        public const int ShipModificationsGroupID = 955;
        public const int ImplantsAndBoostersGroupID = 24;
        public const int StarbaseStructuresGroupID = 477;

        public const int MiningBargesGroupID = 495;
        public const int ExhumersGroupID = 875;
        public const int IndustrialsGroupID = 1390;
        public const int CapitalIndustrialsGroupID = 1048;

        public const int ProjectileAmmunitionBlueprintsGroupID = 299;
        public const int HybridAmmunitionBlueprintsGroupID = 300;
        public const int MissilesAmmunitionBlueprintsGroupID = 314;
        public const int BombsBlueprintsGroupID = 1016;
        public const int BoostersChargesBlueprintsGroupID = 339;

        public const int SkillHardwiringImplantGroupID = 531;
        public const int AttributeEnhancersImplantsGroupID = 532;

        #endregion


        #region Custom market group IDs

        public const int RootNonMarketGroupID = 11000;

        public const int RootUniqueDesignsGroupID = 10000;
        public const int UniqueDesignBattleshipsGroupID = 10200;
        public const int UniqueDesignShuttlesGroupID = 10900;

        public const int BlueprintRootNonMarketGroupID = 21000;
        public const int BlueprintNonMarketTechIGroupID = 21001;
        public const int BlueprintNonMarketTechIIGroupID = 21002;
        public const int BlueprintNonMarketStorylineGroupID = 21003;
        public const int BlueprintNonMarketFactionGroupID = 21004;
        public const int BlueprintNonMarketOfficerGroupID = 21005;
        public const int BlueprintNonMarketTechIIIGroupID = 21014;

        #endregion


        #region Types IDs

        public const int ScrapMetalProcessingSkillID = 12196;

        public const int TradeSkillID = 3443;
        public const int RetailSkillID = 3444;
        public const int BrokerRelationsSkillID = 3446;
        public const int WholesaleSkillID = 16596;
        public const int AccountingSkillID = 16622;
        public const int TycconSkillID = 18580;
        public const int MarketingSkillID = 16598;
        public const int ProcurementSkillID = 16594;
        public const int DaytradingSkillID = 16595;
        public const int VisibilitySkillID = 3447;

        public const int ScienceSkillID = 3402;
        public const int IndustrySkillID = 3380;
        public const int ResearchSkillID = 3403;
        public const int MetallurgySkillID = 3409;
        public const int ProductionEfficiencySkillID = 3388;

        public const int MassProductionSkillID = 3387;
        public const int AdvancedMassProductionSkillID = 24625;
        public const int SupplyChainManagementSkillID = 24268;

        public const int LaboratoryOperationSkillID = 3406;
        public const int AdvancedLaboratoryOperationSkillID = 24624;
        public const int ScientificNetworkingSkillID = 24270;

        public const int WarpDisruptProbeBlueprintID = 22779;

        #endregion

    }
}
