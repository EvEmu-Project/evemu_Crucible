

/*
 *
 * LSC stands for Large Scale Chat
 *
 *
 * CHTMODE_CREATOR = 8 + 4 + 2 + 1
 * CHTMODE_OPERATOR = 4 + 2 + 1
 * CHTMODE_CONVERSATIONALIST = 2 + 1
 * CHTMODE_SPEAKER = 2
 * CHTMODE_LISTENER = 1
 * CHTMODE_NOTSPECIFIED = -1
 * CHTMODE_DISALLOWED = -2
 *
 * CHTERR_NOSUCHCHANNEL = -3
 * CHTERR_ACCESSDENIED = -4
 * CHTERR_INCORRECTPASSWORD = -5
 * CHTERR_ALREADYEXISTS = -6
 * CHTERR_TOOMANYCHANNELS = -7
 *
 * CHT_MAX_USERS_PER_IMMEDIATE_CHANNEL = 50
 * CHT_MAX_INPUT = 253
 *
 *
 *  'rookieHelpChannel': 1,
 *  'helpChannelEN': 2,
 *  'helpChannelDE': 40,
 *  'helpChannelRU': 55,
 *  'helpChannelJA': 56
 *
 */

/*
 * service.ROLE_CHTADMINISTRATOR | service.ROLE_GMH
 * CHTMODE_CREATOR = (((8 + 4) + 2) + 1)
 * CHTMODE_OPERATOR = ((4 + 2) + 1)
 * CHTMODE_CONVERSATIONALIST = (2 + 1)
 * CHTMODE_SPEAKER = 2
 * CHTMODE_LISTENER = 1
 * CHTMODE_NOTSPECIFIED = -1
 * CHTMODE_DISALLOWED = -2
 * CHTERR_NOSUCHCHANNEL = -3
 * CHTERR_ACCESSDENIED = -4
 * CHTERR_INCORRECTPASSWORD = -5
 * CHTERR_ALREADYEXISTS = -6
 * CHTERR_TOOMANYCHANNELS = -7
 * CHT_MAX_USERS_PER_IMMEDIATE_CHANNEL = 50
 *
 * CHANNEL_CUSTOM = 0
 * CHANNEL_GANG = 3
 *
 *
 */
namespace LSC {
/*
    for colorkey, color, intCol in [(service.ROLE_QA, '0xff0099ff', LtoI(4278229503L)),
     (service.ROLE_WORLDMOD, '0xffac75ff', LtoI(4289492479L)),
     (service.ROLE_GMH, '0xffee6666', LtoI(4293813862L)),
     (service.ROLE_GML, '0xffffff20', LtoI(4294967072L)),
     (service.ROLE_CENTURION, '0xff00ff00', LtoI(4278255360L)),
     (service.ROLE_LEGIONEER, '0xff00ffcc', LtoI(4278255564L)),
     (service.ROLE_ADMIN, '0xffee6666', LtoI(4293813862L))]:
        if role & colorkey == colorkey:
            return [color, intCol][asInt]

    return ['0xffe0e0e0', LtoI(4292927712L)][asInt]
    */

    // type designations are internal-use only (client works on strings)
    enum Type{
        global          = 1,    // send channelID as tuple(id, desc)  uses full memberlist, never memberless
        corp            = 2,    // send channelID as tuple(id, desc)  uses full memberlist, never memberless
        region          = 3,    // send channelID as tuple(id, desc)  uses full memberlist, never memberless, not used in w-space
        constellation   = 4,    // send channelID as tuple(id, desc)  uses full memberlist, never memberless, not used in w-space
        solarsystem     = 5,    // send channelID as tuple(id, desc)  used in w-space, memberless, changes chat window title from "Local" to "System"
        solarsystem2    = 6,    // send channelID as tuple(id, desc)  uses full memberlist, never memberless, not used in w-space (k-space "Local" channel)
        // end of static channels
        character       = 7,    // for mailing lists using channelID = charID
        // begin dynamic channels
        alliance        = 8,
        fleet           = 9,
        wing            = 10,
        squad           = 11,
        warfaction      = 12,
        incursion       = 13,
        normal          = 14,   //  trial accts arent time buffered (channelID > 2100000000)
        custom          = 15    //  invite only.  channelID < 0
    };

    enum Mode {
        chDisallowed = -2,
        chUnspecified = -1,
        chNone = 0,
        chListener = 1,
        chSpeaker = 2,
        chConversationalist = 3,
        chOperator = 7,
        chCreator = 15
    };

    enum Error {
        errUnspecified = -1,
        errDisallowed = -2,
        errNoSuchChannel = -3,
        errAccessDenied = -4,
        errWrongPass = -5,
        errChannelExists = -6,
        errTooManyChannels = -7
    };

}   // namespace LSC

/* groupMessageIDs and descriptions
 * 1 = Passive
 * 2 = Aggressive
 * 3 = Focus Fire
 *
 * 263235 = Corporate
 * 263238 = Help
 * 263329 = Factions
 * 263240 = Trade
 * 263331 = Science and Industry
 * 263328 = Content
 *
 */

namespace Notifications {
    /*
securityLevelDescriptions = {-10: 'Notifications/SecurityStatus/SecurityDescription_-10',
 -9: 'Notifications/SecurityStatus/SecurityDescription_-9',
 -8: 'Notifications/SecurityStatus/SecurityDescription_-8',
 -7: 'Notifications/SecurityStatus/SecurityDescription_-7',
 -6: 'Notifications/SecurityStatus/SecurityDescription_-6',
 -5: 'Notifications/SecurityStatus/SecurityDescription_-5',
 -4: 'Notifications/SecurityStatus/SecurityDescription_-4',
 -3: 'Notifications/SecurityStatus/SecurityDescription_-3',
 -2: 'Notifications/SecurityStatus/SecurityDescription_-2',
 -1: 'Notifications/SecurityStatus/SecurityDescription_-1',
 0: 'Notifications/SecurityStatus/SecurityDescription_0',
 1: 'Notifications/SecurityStatus/SecurityDescription_1',
 2: 'Notifications/SecurityStatus/SecurityDescription_2',
 3: 'Notifications/SecurityStatus/SecurityDescription_3',
 4: 'Notifications/SecurityStatus/SecurityDescription_4',
 5: 'Notifications/SecurityStatus/SecurityDescription_5',
 6: 'Notifications/SecurityStatus/SecurityDescription_6',
 7: 'Notifications/SecurityStatus/SecurityDescription_7',
 8: 'Notifications/SecurityStatus/SecurityDescription_8',
 9: 'Notifications/SecurityStatus/SecurityDescription_9',
 10: 'Notifications/SecurityStatus/SecurityDescription_10'}
rankLost = {const.factionCaldariState: 'UI/FactionWarfare/Ranks/RankLostCaldari',
 const.factionMinmatarRepublic: 'UI/FactionWarfare/Ranks/RankLostMinmatar',
 const.factionAmarrEmpire: 'UI/FactionWarfare/Ranks/RankLostAmarr',
 const.factionGallenteFederation: 'UI/FactionWarfare/Ranks/RankLostGallente'}
rankGain = {const.factionCaldariState: 'UI/FactionWarfare/Ranks/RankGainCaldari',
 const.factionMinmatarRepublic: 'UI/FactionWarfare/Ranks/RankGainMinmatar',
 const.factionAmarrEmpire: 'UI/FactionWarfare/Ranks/RankGainAmarr',
 const.factionGallenteFederation: 'UI/FactionWarfare/Ranks/RankGainGallente'}
notificationTypes = {'notificationTypeOldLscMessages': 1,
 'notificationTypeCharTerminationMsg': 2,
 'notificationTypeCharMedalMsg': 3,
 'notificationTypeAllMaintenanceBillMsg': 4,
 'notificationTypeAllWarDeclaredMsg': 5,
 'notificationTypeAllWarSurrenderMsg': 6,
 'notificationTypeAllWarRetractedMsg': 7,
 'notificationTypeAllWarInvalidatedMsg': 8,
 'notificationTypeCharBillMsg': 9,
 'notificationTypeCorpAllBillMsg': 10,
 'notificationTypeBillOutOfMoneyMsg': 11,
 'notificationTypeBillPaidCharMsg': 12,
 'notificationTypeBillPaidCorpAllMsg': 13,
 'notificationTypeBountyClaimMsg': 14,
 'notificationTypeCloneActivationMsg': 15,
 'notificationTypeCorpAppNewMsg': 16,
 'notificationTypeCorpAppRejectMsg': 17,
 'notificationTypeCorpAppAcceptMsg': 18,
 'notificationTypeCorpTaxChangeMsg': 19,
 'notificationTypeCorpNewsMsg': 20,
 'notificationTypeCharLeftCorpMsg': 21,
 'notificationTypeCorpNewCEOMsg': 22,
 'notificationTypeCorpDividendMsg': 23,
 'notificationTypeCorpVoteMsg': 25,
 'notificationTypeCorpVoteCEORevokedMsg': 26,
 'notificationTypeCorpWarDeclaredMsg': 27,
 'notificationTypeCorpWarFightingLegalMsg': 28,
 'notificationTypeCorpWarSurrenderMsg': 29,
 'notificationTypeCorpWarRetractedMsg': 30,
 'notificationTypeCorpWarInvalidatedMsg': 31,
 'notificationTypeContainerPasswordMsg': 32,
 'notificationTypeCustomsMsg': 33,
 'notificationTypeInsuranceFirstShipMsg': 34,
 'notificationTypeInsurancePayoutMsg': 35,
 'notificationTypeInsuranceInvalidatedMsg': 36,
 'notificationTypeSovAllClaimFailMsg': 37,
 'notificationTypeSovCorpClaimFailMsg': 38,
 'notificationTypeSovAllBillLateMsg': 39,
 'notificationTypeSovCorpBillLateMsg': 40,
 'notificationTypeSovAllClaimLostMsg': 41,
 'notificationTypeSovCorpClaimLostMsg': 42,
 'notificationTypeSovAllClaimAquiredMsg': 43,
 'notificationTypeSovCorpClaimAquiredMsg': 44,
 'notificationTypeAllAnchoringMsg': 45,
 'notificationTypeAllStructVulnerableMsg': 46,
 'notificationTypeAllStrucInvulnerableMsg': 47,
 'notificationTypeSovDisruptorMsg': 48,
 'notificationTypeCorpStructLostMsg': 49,
 'notificationTypeCorpOfficeExpirationMsg': 50,
 'notificationTypeCloneRevokedMsg1': 51,
 'notificationTypeCloneMovedMsg': 52,
 'notificationTypeCloneRevokedMsg2': 53,
 'notificationTypeInsuranceExpirationMsg': 54,
 'notificationTypeInsuranceIssuedMsg': 55,
 'notificationTypeJumpCloneDeletedMsg1': 56,
 'notificationTypeJumpCloneDeletedMsg2': 57,
 'notificationTypeFWCorpJoinMsg': 58,
 'notificationTypeFWCorpLeaveMsg': 59,
 'notificationTypeFWCorpKickMsg': 60,
 'notificationTypeFWCharKickMsg': 61,
 'notificationTypeFWCorpWarningMsg': 62,
 'notificationTypeFWCharWarningMsg': 63,
 'notificationTypeFWCharRankLossMsg': 64,
 'notificationTypeFWCharRankGainMsg': 65,
 'notificationTypeAgentMoveMsg': 66,
 'notificationTypeTransactionReversalMsg': 67,
 'notificationTypeReimbursementMsg': 68,
 'notificationTypeLocateCharMsg': 69,
 'notificationTypeResearchMissionAvailableMsg': 70,
 'notificationTypeMissionOfferExpirationMsg': 71,
 'notificationTypeMissionTimeoutMsg': 72,
 'notificationTypeStoryLineMissionAvailableMsg': 73,
 'notificationTypeTutorialMsg': 74,
 'notificationTypeTowerAlertMsg': 75,
 'notificationTypeTowerResourceAlertMsg': 76,
 'notificationTypeStationAggressionMsg1': 77,
 'notificationTypeStationStateChangeMsg': 78,
 'notificationTypeStationConquerMsg': 79,
 'notificationTypeStationAggressionMsg2': 80,
 'notificationTypeFacWarCorpJoinRequestMsg': 81,
 'notificationTypeFacWarCorpLeaveRequestMsg': 82,
 'notificationTypeFacWarCorpJoinWithdrawMsg': 83,
 'notificationTypeFacWarCorpLeaveWithdrawMsg': 84,
 'notificationTypeCorpLiquidationMsg': 85,
 'notificationTypeSovereigntyTCUDamageMsg': 86,
 'notificationTypeSovereigntySBUDamageMsg': 87,
 'notificationTypeSovereigntyIHDamageMsg': 88,
 'notificationTypeContactAdd': 89,
 'notificationTypeContactEdit': 90,
 'notificationTypeIncursionCompletedMsg': 91,
 'notificationTypeCorpKicked': 92,
 'notificationTypeOrbitalAttacked': 93,
 'notificationTypeOrbitalReinforced': 94,
 'notificationTypeOwnershipTransferred': 95,
 'notificationTypeFWAllianceWarningMsg': 96,
 'notificationTypeFWAllianceKickMsg': 97}
notifyIDs = util.KeyVal(notificationTypes)
groupUnread = 0
groupAgents = 1
groupBills = 2
groupCorp = 3
groupMisc = 4
groupOld = 5
groupSov = 6
groupStructures = 7
groupWar = 8
groupContacts = 9
groupTypes = {groupAgents: [notifyIDs.notificationTypeAgentMoveMsg,
               notifyIDs.notificationTypeLocateCharMsg,
               notifyIDs.notificationTypeResearchMissionAvailableMsg,
               notifyIDs.notificationTypeMissionOfferExpirationMsg,
               notifyIDs.notificationTypeMissionTimeoutMsg,
               notifyIDs.notificationTypeStoryLineMissionAvailableMsg,
               notifyIDs.notificationTypeTutorialMsg],
 groupBills: [notifyIDs.notificationTypeAllMaintenanceBillMsg,
              notifyIDs.notificationTypeCharBillMsg,
              notifyIDs.notificationTypeCorpAllBillMsg,
              notifyIDs.notificationTypeBillOutOfMoneyMsg,
              notifyIDs.notificationTypeBillPaidCharMsg,
              notifyIDs.notificationTypeBillPaidCorpAllMsg,
              notifyIDs.notificationTypeCorpOfficeExpirationMsg],
 groupContacts: [notifyIDs.notificationTypeContactAdd, notifyIDs.notificationTypeContactEdit],
 groupCorp: [notifyIDs.notificationTypeCharTerminationMsg,
             notifyIDs.notificationTypeCharMedalMsg,
             notifyIDs.notificationTypeCorpAppNewMsg,
             notifyIDs.notificationTypeCorpAppRejectMsg,
             notifyIDs.notificationTypeCorpAppAcceptMsg,
             notifyIDs.notificationTypeCorpTaxChangeMsg,
             notifyIDs.notificationTypeCorpNewsMsg,
             notifyIDs.notificationTypeCharLeftCorpMsg,
             notifyIDs.notificationTypeCorpNewCEOMsg,
             notifyIDs.notificationTypeCorpDividendMsg,
             notifyIDs.notificationTypeCorpVoteMsg,
             notifyIDs.notificationTypeCorpVoteCEORevokedMsg,
             notifyIDs.notificationTypeCorpLiquidationMsg,
             notifyIDs.notificationTypeCorpKicked],
 groupMisc: [notifyIDs.notificationTypeBountyClaimMsg,
             notifyIDs.notificationTypeCloneActivationMsg,
             notifyIDs.notificationTypeContainerPasswordMsg,
             notifyIDs.notificationTypeCustomsMsg,
             notifyIDs.notificationTypeInsuranceFirstShipMsg,
             notifyIDs.notificationTypeInsurancePayoutMsg,
             notifyIDs.notificationTypeInsuranceInvalidatedMsg,
             notifyIDs.notificationTypeCloneRevokedMsg1,
             notifyIDs.notificationTypeCloneMovedMsg,
             notifyIDs.notificationTypeCloneRevokedMsg2,
             notifyIDs.notificationTypeInsuranceExpirationMsg,
             notifyIDs.notificationTypeInsuranceIssuedMsg,
             notifyIDs.notificationTypeJumpCloneDeletedMsg1,
             notifyIDs.notificationTypeJumpCloneDeletedMsg2,
             notifyIDs.notificationTypeTransactionReversalMsg,
             notifyIDs.notificationTypeReimbursementMsg,
             notifyIDs.notificationTypeIncursionCompletedMsg],
 groupOld: [notifyIDs.notificationTypeOldLscMessages],
 groupSov: [notifyIDs.notificationTypeSovAllClaimFailMsg,
            notifyIDs.notificationTypeSovCorpClaimFailMsg,
            notifyIDs.notificationTypeSovAllBillLateMsg,
            notifyIDs.notificationTypeSovCorpBillLateMsg,
            notifyIDs.notificationTypeSovAllClaimLostMsg,
            notifyIDs.notificationTypeSovCorpClaimLostMsg,
            notifyIDs.notificationTypeSovAllClaimAquiredMsg,
            notifyIDs.notificationTypeSovCorpClaimAquiredMsg,
            notifyIDs.notificationTypeSovDisruptorMsg,
            notifyIDs.notificationTypeAllStructVulnerableMsg,
            notifyIDs.notificationTypeAllStrucInvulnerableMsg,
            notifyIDs.notificationTypeSovereigntyTCUDamageMsg,
            notifyIDs.notificationTypeSovereigntySBUDamageMsg,
            notifyIDs.notificationTypeSovereigntyIHDamageMsg],
 groupStructures: [notifyIDs.notificationTypeAllAnchoringMsg,
                   notifyIDs.notificationTypeCorpStructLostMsg,
                   notifyIDs.notificationTypeTowerAlertMsg,
                   notifyIDs.notificationTypeTowerResourceAlertMsg,
                   notifyIDs.notificationTypeStationAggressionMsg1,
                   notifyIDs.notificationTypeStationStateChangeMsg,
                   notifyIDs.notificationTypeStationConquerMsg,
                   notifyIDs.notificationTypeStationAggressionMsg2,
                   notifyIDs.notificationTypeOrbitalAttacked,
                   notifyIDs.notificationTypeOrbitalReinforced,
                   notifyIDs.notificationTypeOwnershipTransferred],
 groupWar: [notifyIDs.notificationTypeAllWarDeclaredMsg,
            notifyIDs.notificationTypeAllWarSurrenderMsg,
            notifyIDs.notificationTypeAllWarRetractedMsg,
            notifyIDs.notificationTypeAllWarInvalidatedMsg,
            notifyIDs.notificationTypeCorpWarDeclaredMsg,
            notifyIDs.notificationTypeCorpWarFightingLegalMsg,
            notifyIDs.notificationTypeCorpWarSurrenderMsg,
            notifyIDs.notificationTypeCorpWarRetractedMsg,
            notifyIDs.notificationTypeCorpWarInvalidatedMsg,
            notifyIDs.notificationTypeFWCorpJoinMsg,
            notifyIDs.notificationTypeFWCorpLeaveMsg,
            notifyIDs.notificationTypeFWCorpKickMsg,
            notifyIDs.notificationTypeFWCharKickMsg,
            notifyIDs.notificationTypeFWCorpWarningMsg,
            notifyIDs.notificationTypeFWCharWarningMsg,
            notifyIDs.notificationTypeFWCharRankLossMsg,
            notifyIDs.notificationTypeFWCharRankGainMsg,
            notifyIDs.notificationTypeFacWarCorpJoinRequestMsg,
            notifyIDs.notificationTypeFacWarCorpLeaveRequestMsg,
            notifyIDs.notificationTypeFacWarCorpJoinWithdrawMsg,
            notifyIDs.notificationTypeFacWarCorpLeaveWithdrawMsg,
            notifyIDs.notificationTypeFWAllianceWarningMsg,
            notifyIDs.notificationTypeFWAllianceKickMsg]}
            */
}   // namespace Notifications