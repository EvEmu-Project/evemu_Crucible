
 /**
  * @name EVE_Corp.h
  *     Corp enums and Data containers for EVEmu
  *
  * @Author:        Allan
  * @date:          2 December 2017 (corp rewrite began)
  *
  */


#ifndef EVE_CORP_H
#define EVE_CORP_H

namespace Corp {

    namespace BillType {
        enum {
            MarketFine = 1,
            RentalBill = 2,
            BrokerBill = 3,
            WarBill = 4,
            AllianceMaintainanceBill = 5,
            SovereigntyMarker = 6
        };
    }

    namespace BillStatus {
        enum {
            Unpaid = 0,
            Paid = 1,
            Cancelled = 2
        };
    }

    namespace VoteType {
        enum {
            CEO = 0,
            War = 1,
            Shares = 2,
            KickMember = 3,
            General = 4,
            ItemLock = 5,
            ItemUnlock = 6
        };
    }

    namespace EventType {
        enum {
            // these are the only types defined in client.  others are 'logtype'
            CreatedCorporation = 12,
            DeletedCorporation = 13,
            LeftCorporation = 14,
            AppliedForMembershipOfCorporation = 15,
            BecameCEOOfCorporation = 16,
            LeftCEOPositionOfCorporation = 17,
            JoinedCorporation = 44
        };
    }

    namespace AppStatus {
        enum { //          status              corp side   user side
            AppliedByCharacter          = 0, //    new       applied
            RenegotiatedByCharacter     = 1, //   update      reneg
            AcceptedByCharacter         = 2, //  accepted    accepted
            RejectedByCharacter         = 3,
            RejectedByCorporation       = 4, //    error      reject
            RenegotiatedByCorporation   = 5, //    reneg      update
            AcceptedByCorporation       = 6  //    offer      offer
        };
    }

    // not sure about these yet...
    // these are 'remoteActions' from 'self.GetCorpRegistry().ExecuteActions(targetIDs, remoteActions)'
    namespace Actions {
        enum {
            Add = 1,
            Remove = 2,
            Set = 3,
            Give = 4,
            Comms = 5
        };
    }
    namespace RoleLoc {
        enum {
            HQ      = 1,
            Base    = 2,
            Other   = 3
        };
    }

    namespace JoinOp {
        enum {
            None = 0,
            OR = 1,
            AND = 2
        };
    }

    namespace QueryType {
        enum {
            Roles = 0,
            BaseID = 1,
            CharID = 2,
            TitleMask = 3,
            StartDateTime = 4,
            GrantableRoles = 5
        };
    }

    namespace SearchOp {
        enum {
            EQUAL = 1,
            GREATER = 2,
            GREATER_OR_EQUAL = 3,
            LESS = 4,
            LESS_OR_EQUAL = 5,
            NOT_EQUAL = 6,
            HAS_BIT = 7,
            NOT_HAS_BIT = 8,
            STR_CONTAINS = 9,
            STR_LIKE = 10,
            STR_STARTS_WITH = 11,
            STR_ENDS_WITH = 12,
            STR_IS = 13
        };
    }

    namespace ActivityType {
        enum {
            Agriculture     = 1,
            Construction    = 2,
            Mining          = 3,
            Chemical        = 4,
            Military        = 5,
            Biotech         = 6,
            HiTech          = 7,
            Entertainment   = 8,
            Shipyard        = 9,
            Warehouse       = 10,
            Retail          = 11,
            Trading         = 12,
            Bureaucratic    = 13,
            Political       = 14,
            Legal           = 15,
            Security        = 16,
            Financial       = 17,
            Education       = 18,
            Manufacture     = 19,
            Disputed        = 20
        };
    }

    namespace Division {
        enum {
            Accounting          = 1,
            Administration      = 2,
            Advisory            = 3,
            Archives            = 4,
            Astrosurveying      = 5,
            Command             = 6,
            Distribution        = 7,
            Financial           = 8,
            Intelligence        = 9,
            InternalSecurity    = 10,
            Legal               = 11,
            Manufacturing       = 12,
            Marketing           = 13,
            Mining              = 14,
            Personnel           = 15,
            Production          = 16,
            PublicRelations     = 17,
            RnD                 = 18,
            Security            = 19,
            Storage             = 20,
            Surveillance        = 21,
            DistributionNew     = 22,
            MiningNew           = 23,
            SecurityNew         = 24
        };
    }
}


namespace EveAlliance {
/*
 * allianceCreationCost   = 1000000000
 * allianceMembershipCost =    2000000
 */

    namespace AppStatus {
        enum  {
            AppNew = 1,
            AppAccepted = 2,
            AppEffective = 3,
            AppRejected = 4
        };
    }

    namespace Relation {
        enum  {
            NAP = 1,
            Friend = 2,
            Competitor = 3,
            Enemy = 4
        };
    }
}

namespace FacWar {
/*
 * facwarStandingPerVictoryPoint = 0.0015
 *
 * facwarOccupierVictoryPointBonus = 0.1
 *
 * facwarMinStandingsToJoin = 0.5
 *
 * facwarWarningStandingCharacter = 0
 * facwarWarningStandingCorporation = 1
 *
 *
 */

    namespace Relationship {
        enum {
            Unknown = 0,
            YourCorp = 1,
            YourAlliance = 2,
            AtWar = 3,
            AtWarCanFight = 4
        };
    }

    namespace CorpStatus {
        enum  {
            Joining = 0,
            Active = 1,
            Leaving = 2
        };
    }

    namespace StatType {
        enum  {
            Kill = 0,
            Loss = 1
        };
    }

    namespace SysStatus {
        enum {
            None = 0,
            Contested = 1,
            Vulnerable = 2,
            Captured = 3
        };
    }
}

// factions list
typedef enum {
    factionNoFaction     = 0,
    factionCaldari       = 500001,
    factionMinmatar      = 500002,
    factionAmarr         = 500003,
    factionGallente      = 500004,
    factionJove          = 500005,
    factionCONCORD       = 500006,
    factionAmmatar       = 500007,
    factionKhanid        = 500008,
    factionSyndicate     = 500009,
    factionGuristas      = 500010,
    factionAngel         = 500011,
    factionBloodRaider   = 500012,
    factionInterBus      = 500013,
    factionORE           = 500014,
    factionThukker       = 500015,
    factionSistersOfEVE  = 500016,
    factionSociety       = 500017,
    factionMordusLegion  = 500018,
    factionSanshas       = 500019,
    factionSerpentis     = 500020,
    factionUnknown       = 500021,
    factionRogueDrones   = 500022,
    factionSleepers      = 500023
} FactionDef;

// npc corps list
typedef enum {
    corpRogueDrones         = 1000001,
    corpCBD    = 1000002,
    corpPromptDelivery     = 1000003,
    corpYtiri   = 1000004,
    corpHyasyoda    = 1000005,
    corpDeepCoreMining   = 1000006,
    corpPoksuMineralGroup     = 1000007,
    corpMinedrill   = 1000008,
    corpCaldariProvisions  = 1000009,
    corpKaalakiota  = 1000010,
    corpWiyrkomi    = 1000011,
    corpTopDown    = 1000012,
    corpRapidAssembly  = 1000013,
    corpPerkone     = 1000014,
    corpCaldariSteel   = 1000015,
    corpZainou  = 1000016,
    corpNugoeihuvi  = 1000017,
    corpEchelonEntertainment   = 1000018,
    corpIshukone    = 1000019,
    corpLaiDai     = 1000020,
    corpZeroGResearch    = 1000021,
    corpPropelDynamics     = 1000022,
    corpExpertDistribution     = 1000023,
    corpCBDSell   = 1000024,
    corpSukuuvestaa     = 1000025,
    corpCaldariConstructions   = 1000026,
    corpExpertHousing  = 1000027,
    corpCaldariFundsUnlimited     = 1000028,
    corpStateRegionBank   = 1000029,
    corpModernFinances     = 1000030,
    corpChiefExecutivePanel   = 1000031,
    corpMercantileClub     = 1000032,
    corpCaldariBusinessTribunal   = 1000033,
    corpHouseofRecords    = 1000034,
    corpCaldariNavy    = 1000035,
    corpInternalSecurity   = 1000036,
    corpLaiDaiProtection  = 1000037,
    corpIshukoneWatch  = 1000038,
    corpHomeGuard  = 1000039,
    corpPeaceandOrderUnit    = 1000040,
    corpSpacelanePatrol    = 1000041,
    corpWiyrkomiPeaceCorps    = 1000042,
    corpCorporatePoliceForce  = 1000043,
    corpSchoolofAppliedKnowledge     = 1000044,
    corpScienceandTradeInstitute     = 1000045,
    corpSebiestortribe     = 1000046,
    corpKrusualTribe   = 1000047,
    corpVherokiorTribe     = 1000048,
    corpBrutorTribe    = 1000049,
    corpRepublicParliament     = 1000050,
    corpRepublicFleet  = 1000051,
    corpRepublicJustice     = 1000052,
    corpUrbanManagement    = 1000053,
    corpRepublicSecurityServices  = 1000054,
    corpMinmatarMining     = 1000055,
    corpCoreComplexion    = 1000056,
    corpBoundlessCreation  = 1000057,
    corpEifyrandCo   = 1000058,
    corpSixKinDevelopment     = 1000059,
    corpNativeFreshfood    = 1000060,
    corpFreedomExtension   = 1000061,
    corpLeisureGroup   = 1000062,
    corpAmarrConstructions     = 1000063,
    corpCarthumConglomerate    = 1000064,
    corpImperialArmaments  = 1000065,
    corpViziam  = 1000066,
    corpZoarandSons   = 1000067,
    corpNobleAppliances    = 1000068,
    corpDuciaFoundry   = 1000069,
    corpHZORefinery    = 1000070,
    corpInherentImplants   = 1000071,
    corpImperialShipment   = 1000072,
    corpAmarrCertifiedNews    = 1000073,
    corpJointHarvesting    = 1000074,
    corpNurtura     = 1000075,
    corpFurtherFoodstuffs  = 1000076,
    corpRoyalAmarrInstitute   = 1000077,
    corpImperialChancellor     = 1000078,
    corpAmarrCivilService     = 1000079,
    corpMinistryofWar     = 1000080,
    corpMinistryofAssessment  = 1000081,
    corpMinistryofInternalOrder  = 1000082,
    corpAmarrTradeRegistry    = 1000083,
    corpAmarrNavy  = 1000084,
    corpCourtChamberlain   = 1000085,
    corpEmperorFamily  = 1000086,
    corpKadorFamily    = 1000087,
    corpSarumFamily    = 1000088,
    corpKorAzorFamily     = 1000089,
    corpArdishapurFamily   = 1000090,
    corpTashMurkonFamily  = 1000091,
    corpCivicCourt     = 1000092,
    corpTheologyCouncil    = 1000093,
    corpTransStellarShipping   = 1000094,
    corpFederalFreight     = 1000095,
    corpInnerZoneShipping     = 1000096,
    corpMaterialAcquisition    = 1000097,
    corpAstralMining  = 1000098,
    corpCombinedHarvest    = 1000099,
    corpQuafeCompany   = 1000100,
    corpCreoDron    = 1000101,
    corpRodenShipyards     = 1000102,
    corpAllotekIndustries  = 1000103,
    corpPotequePharmaceuticals     = 1000104,
    corpImpetus     = 1000105,
    corpEgonics    = 1000106,
    corpTheScope   = 1000107,
    corpChemalTech     = 1000108,
    corpDuvolleLaboratories    = 1000109,
    corpFedMart     = 1000110,
    corpAliastra    = 1000111,
    corpBankofLuminaire   = 1000112,
    corpPendInsurance  = 1000113,
    corpGarounInvestmentBank  = 1000114,
    corpUniversityofCaille    = 1000115,
    corpPresident   = 1000116,
    corpSenate  = 1000117,
    corpSupremeCourt   = 1000118,
    corpFederalAdministration  = 1000119,
    corpFederationNavy     = 1000120,
    corpFederalIntelligenceOffice     = 1000121,
    corpFederationCustoms  = 1000122,
    corpAmmatarFleet   = 1000123,
    corpArchangels               = 1000124,
    corpCONCORD             = 1000125,
    corpAmmatarConsulate   = 1000126,
    corpGuristas            = 1000127,
    corpMordusLegion        = 1000128,
    corpORE                 = 1000129,
    corpSoE                 = 1000130,
    corpSocietyofCT             = 1000131,
    corpSCC                 = 1000132,
    corpSalvationAngels    = 1000133,
    corpBloodRaider         = 1000134,
    corpSerpentis           = 1000135,
    corpGuardianAngels     = 1000136,
    corpDED                 = 1000137,   //division of concord
    corpDominations     = 1000138,
    corpFoodRelief     = 1000139,
    corpGenolution  = 1000140,
    corpGuristasProduction     = 1000141,
    corpImpro   = 1000142,
    corpInnerCircle    = 1000143,
    corpIntakiBank     = 1000144,
    corpIntakiCommerce     = 1000145,
    corpIntakiSpacePolice     = 1000146,
    corpIntakiSyndicate    = 1000147,
    corpInterbus            = 1000148,
    corpJoveNavy   = 1000149,
    corpJovianDirectorate  = 1000150,
    corpKhanidInnovation    = 1000151,
    corpKhanidTransport     = 1000152,
    corpKhanidWorks         = 1000153,
    corpNefantarMiners  = 1000154,
    corpProsper     = 1000155,
    corpKhanidNavy          = 1000156,
    corpSerpentisInquest   = 1000157,
    corpShapeset    = 1000158,
    corpTheSanctuary   = 1000159,
    corpThukkerMix          = 1000160,
    //corpSanshas             = 1000161,
    corpTrueCreations  = 1000161,
    corpTruePower  = 1000162,
    corpTrustPartners  = 1000163,
    corpXSense     = 1000164,
    corpHedionUniversity   = 1000165,
    corpImperialAcademy    = 1000166,
    corpStateWarAcademy   = 1000167,
    corpFederalNavyAcademy    = 1000168,
    corpCenterforAdvancedStudies     = 1000169,
    corpRepublicMilitarySchool    = 1000170,
    corpRepublicUniversity     = 1000171,
    corpPatorTechSchool   = 1000172,
    corpMaterialInstitute  = 1000177,
    corpAcademyofAggressiveBehaviour     = 1000178,
    corp24ImperialCrusade   = 1000179,
    corpStateProtectorate  = 1000180,
    corpFederalDefenseUnion   = 1000181,
    corpTribalLiberationForce     = 1000182
} corpDef;


#endif  // EVE_CORP_H


/*{'FullPath': u'UI/Messages', 'messageID': 259779, 'label': u'CorpAccountsHintBody'}(u'Corporate Assets Information - Requires Accountant Role', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259780, 'label': u'CorpApplicationsHintBody'}(u'Used for processing applications made by people wanting to join your corporation.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259781, 'label': u'CorpCantSetHQAsNoRealEstateHereTitle'}(u'Relocating HQ', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259782, 'label': u'CorpCantSetHQAsNoRealEstateHereBody'}(u"Your corporation's HQ cannot be relocated to this station as it owns no real estate here. In order to relocate your corporation's HQ here, it must own the station or an office in the station.", None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259783, 'label': u'CorpHQIsAtThisStationTitle'}(u'Relocating HQ', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259784, 'label': u'CorpHQIsAtThisStationBody'}(u"Your corporation's HQ is already located at this station.", None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259785, 'label': u'CorpHasNoMoneyToPayoutDividendsTitle'}(u'No Funds Available', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259786, 'label': u'CorpHasNoMoneyToPayoutDividendsBody'}(u'It is not possible to pay out a dividend now as the corporation has no funds available to do so.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259787, 'label': u'CorpMembersHintBody'}(u'Lists the members of the corporation.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259788, 'label': u'CorpNameInvalidBody'}(u'Corporation name is not valid.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259789, 'label': u'CorpNameInvalidBannedWordBody'}(u'Corporation name contains a banned word.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259790, 'label': u'CorpNameInvalidFirstCharBody'}(u'First character in corporation name is illegal.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259791, 'label': u'CorpNameInvalidLastCharBody'}(u'Last character in corporation name is illegal.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259792, 'label': u'CorpNameInvalidMaxLengthBody'}(u'Maximum length for a corporation name is 24 characters.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259793, 'label': u'CorpNameInvalidMinLengthBody'}(u'Minimum length for a corporation name is 4 characters.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259794, 'label': u'CorpNameInvalidSomeCharBody'}(u'Corporation name contains an illegal character.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259795, 'label': u'CorpNameInvalidTakenBody'}(u'Corporation name is already taken.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259796, 'label': u'CorpSanctionableActionsHintBody'}(u'Lists the votes that have closed where there is a mandate to sanction the action that was the subject of the vote.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259797, 'label': u'CorpSecurityStatusTooLowTitle'}(u'Security Status', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259798, 'label': u'CorpSecurityStatusTooLowBody'}(u'The security status of your corporation is too low for you to do that.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259799, 'label': u'CorpTickerNameInvalidBody'}(u'Corporation ticker name is not valid.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259800, 'label': u'CorpTickerNameInvalidTakenBody'}(u'Corporation ticker name is taken.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259801, 'label': u'CorpVoteCaseClosedTitle'}(u'Voting Period Closed', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259802, 'label': u'CorpVoteCaseClosedBody'}(u'The voting period for the vote you attempted to vote in has closed. Your vote has not been registered.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259803, 'label': u'CorpWarsHintBody'}(u'Displays the status of on going wars. This includes the wars you are directly involved in and all other wars.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259804, 'label': u'CouldNotConnectBody'}(u"Couldn't connect", None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259805, 'label': u'CrpAlreadyVotedTitle'}(u'Already Voted', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259806, 'label': u'CrpAlreadyVotedBody'}(u'You have already cast your vote, you cannot do it again.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259807, 'label': u'CrpApplicationAlreadyExistsTitle'}(u'Not Possible', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259808, 'label': u'CrpApplicationAlreadyExistsBody'}(u'You have already submitted an application to this corporation', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259809, 'label': u'CrpCanNotChangeCorpInSpaceTitle'}(u'Not Possible In Space', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259810, 'label': u'CrpCanNotChangeCorpInSpaceBody'}(u'You can not change your corporation while you are in space.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259811, 'label': u'CrpCanNotGoToWarAgainstTitle'}(u'Illegal War', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259812, 'label': u'CrpCanNotGoToWarAgainstBody'}(u'You can not attempt to declare war against that corporation because they have not killed any of you corporations members within the past 48 hours.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259813, 'label': u'CrpCanNotKickMemberInSpaceTitle'}(u'Not Possible In Space', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259814, 'label': u'CrpCanNotKickMemberInSpaceBody'}(u'You can not kick out this member of your corporation as they are currently in space. Please try again later. You may want to consider removing all roles and grantables from them.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259815, 'label': u'CrpCanNotSanctionWarAgainstOwnerTitle'}(u'Can Not Sanction War', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259816, 'label': u'CrpCanNotSanctionWarAgainstOwnerBody'}(u'The war can not be sanctioned as it is against a member of an alliance. If you want to go to war with them then you will have to declare war against the whole alliance.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259817, 'label': u'CrpCanOnlyAcceptOneApplicationTitle'}(u'Not Possible', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259818, 'label': u'CrpCanOnlyAcceptOneApplicationBody'}(u'You can only accept one application at a time.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259819, 'label': u'CrpCantDeclareWarOnNPCCorpTitle'}(u'You Cannot Declare War Against Them', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259820, 'label': u'CrpCantDeclareWarOnNPCCorpBody'}(u'You cannot declare war against that corporation.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259821, 'label': u'CrpCantLockItemNotYoursTitle'}(u'Locking Operation Failed', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259822, 'label': u'CrpCantLockItemNotYoursBody'}(u'The item can not be locked as it is not yours.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259823, 'label': u'CrpCantUnlockWhatYouDontKnowAboutTitle'}(u'Locking Operation Failed', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259824, 'label': u'CrpCantUnlockWhatYouDontKnowAboutBody'}(u'You can unlock the item as you dont know that it is locked.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259825, 'label': u'CrpCharNotInThisCorpTitle'}(u'Who?', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259826, 'label': u'CrpCharNotInThisCorpBody'}(u'The specified corporation member is not a member of this corporation.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259827, 'label': u'CrpConfirmMutualWarTitle'}(u'Mutual War?', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259828, 'label': u'CrpConfirmMutualWarBody'}(u'Do you want to make this war mutual? Doing so will remove the requirement for bills to be paid by the declarer in order to continue the war.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259829, 'label': u'CrpConfirmRetractWarTitle'}(u'Retract War?', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259830, 'label': u'CrpConfirmRetractWarBody'}(u'Retracting a war can only be performed by the CEO of the corporation that declared the war. Once retracted the war will be considered over after approximately 24 hours. Do you want to retract this war?', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259831, 'label': u'CrpConfirmUnmutualWarTitle'}(u'No longer mutual?', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259832, 'label': u'CrpConfirmUnmutualWarBody'}(u'This war is currently mutual. That means that the declarer does not pay bills for continuing the war. Do you want to end this mutual war state and have the declarer pay bills for continuing the war?', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259833, 'label': u'CrpDataNotOldEnoughForRefreshTitle'}(u'Please Wait', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259834, 'label': u'CrpDataNotOldEnoughForRefreshBody'}(u'The corporation member data you currently have is less than 5 minutes old. You may not refresh the data until it is more than 5 minutes old. If you wish to see the up to date roles of a member right click the member and select the option to view or edit their member details.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259835, 'label': u'CrpHQChangeStillBeingRegisteredTitle'}(u'HQ Registration In Process', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259836, 'label': u'CrpHQChangeStillBeingRegisteredBody'}(u'The HQ of your corporation was changed within the last hour. You must wait for the registration to complete before you can change it again.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259837, 'label': u'CrpJunkContainsLockedItemTitle'}(u'Locked Item Issue', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259838, 'label': u'CrpJunkContainsLockedItemBody'}(u'The impounded items contain at least one locked item. You can not retrieve the impounded items until the locks(s) have been removed. If you have a locked item here, you need to propose a vote to unlock the item before getting the impounded items back.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259839, 'label': u'CrpJunkOnlyAvailableToDirectorTitle'}(u'No Hangar?', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259840, 'label': u'CrpJunkOnlyAvailableToDirectorBody'}(u'The lease on your corporate hangar has expired. Its contents has been impounded. In order to have the contents released either rent another hangar or have a director or your CEO attempt to open the hangar.', None, None)
 *
 * {'FullPath': u'UI/Corporations/AccessRestrictions', 'messageID': 251326, 'label': u'NotAccountantOrBetter'}(u'You need to have the role accountant or better to perform this operation.', None, None)
 * {'FullPath': u'UI/Corporations/AccessRestrictions', 'messageID': 251327, 'label': u'NotJuniorAccountantOrBetter'}(u'You need to have the role accountant or junior accountant or better to perform this operation.', None, None)
 * {'FullPath': u'UI/Corporations/AccessRestrictions', 'messageID': 251337, 'label': u'NotFullAccountant'}(u'You are not an accountant of the corporation.', None, None)
 * {'FullPath': u'UI/Corporations/AccessRestrictions', 'messageID': 251346, 'label': u'NoAccountantOrTrader'}(u'You are not an accountant, a junior accountant or a trader of the corporation,', None, None)
 *
 */
