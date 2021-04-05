
 /**
  * @name FleetData.h
  *     Fleet enums and Data containers for EVEmu
  *
  * @Author:        Allan
  * @date:          05 August 2014 (original skeleton outline)
  * @update:        21 November 2017 (begin actual implementation)
  *
  */

#ifndef EVEMU_SRC_FLEET_DATA_H_
#define EVEMU_SRC_FLEET_DATA_H_

namespace Fleet {
    namespace Job {
        enum {
            None        = 0,
            Scout       = 1,
            Creator     = 2
        };
    }
    namespace Role {
        enum {
            FleetLeader = 1,
            WingLeader  = 2,
            SquadLeader = 3,
            Member      = 4
        };
    }
    namespace Booster {
        enum {
            No      = 0,
            Fleet   = 1,
            Wing    = 2,
            Squad   = 3
        };
    }
    namespace Invite {
        enum {
            Closed = 0,
            Corp = 1,
            Alliance = 2,
            Militia = 4,
            Public = 8,
            Any = 15
        };
    }
    namespace BCast {
        namespace Scope {
            enum {
                Universe = 0,
                System = 1,
                Bubble = 2
            };
        }
        namespace Group {
            enum {
                None = 0,   // is this needed?
                Down = 1,   // subordinates (or squad if member)
                Up = 2,     // superiors
                All = 3
            };
        }
    }
}

// all bonuses are 2%/lvl
struct BoostData {
    int8 armored;  // armor hit points
    int8 leader;   // targeting speed
    int8 info;     // targeting range
    int8 mining;   // mining yield
    int8 siege;    // shield capacity
    int8 skirmish; // agility
};

class Client;

struct FleetAdvert {
    bool hideInfo :1;
    bool joinNeedsApproval :1;
    uint8 inviteScope;
    int32 fleetID;
    uint32 solarSystemID;
    int64 advertTime;
    int64 dateCreated;
    float local_minSecurity;
    float public_minStanding;
    float local_minStanding;
    float public_minSecurity;
    Client* leader;
    std::string fleetName;
    std::string description;
    std::vector<uint32> public_allowedEntities;
    std::vector<uint32> local_allowedEntities;
};

struct FleetData {
    bool isFreeMove :1;
    bool isRegistered :1;
    bool isVoiceEnabled :1;
    bool isLootLogging :1;
    int8 squads;
    int64 dateCreated;
    Client* creator;
    Client* leader;
    Client* booster;
    std::string name;
    std::string motd;
    std::multimap<uint32, uint32> isMutedByLeader;
    std::multimap<uint32, uint32> isExcludedFromMuting;
};

// fleetID, wing name and squad count (5 per fleet)
struct WingData {
    uint32 fleetID;
    BoostData boost;
    Client* leader;
    Client* booster;
    std::string name;
};

// wingID, squad name and member count (5 per wing)
struct SquadData {
    uint32 fleetID;
    uint32 wingID;
    BoostData boost;
    Client* leader;
    Client* booster;
    std::string name;
    std::map<uint32, Client*> members;
};

struct InviteData {
    int8 role;
    int32 wingID;
    int32 squadID;
    Client* invited;
    Client* inviteBy;
};

#endif  // EVEMU_SRC_FLEET_DATA_H_


//Pilots who are disconnected and reconnect within 2 minutes will automatically rejoin their fleet.

/*Note that the maximum size of a fleet is 256 pilots:
 * 1 Fleet Commander,
 * 5 Wing Commanders,
 * 25 10-man squadrons (5 Squads of 9 Pilots + Squad Commander per Wing Commander).
 *
 * To put it another way the maximum size of:
 *   each squadron is 9 members plus the Squad Commander, (10 total)
 *   each wing is five squadrons plus the Wing Commander, (51 total)
 *   each fleet is 5 wings plus the Fleet Commander.      (256 total)
 */

// client fleet error msgs
/*{'messageKey': 'FleetAlreadyBooster', 'dataID': 17879147, 'suppressable': False, 'bodyID': 257898, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2260}
 * {'messageKey': 'FleetAlreadyLeader', 'dataID': 17881512, 'suppressable': False, 'bodyID': 258800, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1743}
 * {'messageKey': 'FleetAlreadyMoving', 'dataID': 17880064, 'suppressable': False, 'bodyID': 258248, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2102}
 * {'messageKey': 'FleetAlreadyRequestedAccess', 'dataID': 17876294, 'suppressable': False, 'bodyID': 256820, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256819, 'messageID': 3069}
 * {'messageKey': 'FleetApplicationReceived', 'dataID': 17876289, 'suppressable': False, 'bodyID': 256818, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3067}
 * {'messageKey': 'FleetBoosterIllegal', 'dataID': 17879152, 'suppressable': False, 'bodyID': 257900, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257899, 'messageID': 2261}
 * {'messageKey': 'FleetBoosterRoleFull', 'dataID': 17879155, 'suppressable': False, 'bodyID': 257901, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2262}
 * {'messageKey': 'FleetBroadcastScopeChange', 'dataID': 17876906, 'suppressable': False, 'bodyID': 257052, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3099}
 * {'messageKey': 'FleetCONCORDSpamPreventionActConfirmation', 'dataID': 17883823, 'suppressable': False, 'bodyID': 259640, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 259639, 'messageID': 898}
 * {'messageKey': 'FleetCandidateDodgy1', 'dataID': 17883738, 'suppressable': True, 'bodyID': 259609, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 259608, 'messageID': 899}
 * {'messageKey': 'FleetCandidateDodgyN', 'dataID': 17883743, 'suppressable': True, 'bodyID': 259611, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 259610, 'messageID': 900}
 * {'messageKey': 'FleetCandidateNotInvited', 'dataID': 17881515, 'suppressable': False, 'bodyID': 258801, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1744}
 * {'messageKey': 'FleetCandidateOffline', 'dataID': 17881015, 'suppressable': False, 'bodyID': 258615, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258614, 'messageID': 1801}
 * {'messageKey': 'FleetCannotDeleteNonEmptySquad', 'dataID': 17880104, 'suppressable': False, 'bodyID': 258264, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258263, 'messageID': 2112}
 * {'messageKey': 'FleetCannotDeleteNonEmptyWing', 'dataID': 17880094, 'suppressable': False, 'bodyID': 258260, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258259, 'messageID': 2109}
 * {'messageKey': 'FleetCannotDoInStation', 'dataID': 17879185, 'suppressable': False, 'bodyID': 257913, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': 257912, 'messageID': 2432}
 * {'messageKey': 'FleetCannotJoinFleet', 'dataID': 17876319, 'suppressable': False, 'bodyID': 256830, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256829, 'messageID': 3103}
 * {'messageKey': 'FleetCannotSendBroadcastToSelectedGroup', 'dataID': 17880139, 'suppressable': False, 'bodyID': 258278, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258277, 'messageID': 2177}
 * {'messageKey': 'FleetCantKickBoss', 'dataID': 17880134, 'suppressable': False, 'bodyID': 258276, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258275, 'messageID': 2162}
 * {'messageKey': 'FleetConfirmAutoJoinVoice', 'dataID': 17879160, 'suppressable': False, 'bodyID': 257903, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 257902, 'messageID': 2435}
 * {'messageKey': 'FleetConfirmDemoteSelf', 'dataID': 17880114, 'suppressable': False, 'bodyID': 258268, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 258267, 'messageID': 2123}
 * {'messageKey': 'FleetConfirmVoiceEnable', 'dataID': 17879165, 'suppressable': True, 'bodyID': 257905, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 257904, 'messageID': 2437}
 * {'messageKey': 'FleetError', 'dataID': 17880147, 'suppressable': False, 'bodyID': 258281, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2192}
 * {'messageKey': 'FleetExportInfo', 'dataID': 17876911, 'suppressable': True, 'bodyID': 257054, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257053, 'messageID': 3114}
 * {'messageKey': 'FleetInviteAllWithoutStanding', 'dataID': 17876299, 'suppressable': False, 'bodyID': 256822, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256821, 'messageID': 3071}
 * {'messageKey': 'FleetInviteMultipleErrors', 'dataID': 17877676, 'suppressable': False, 'bodyID': 257342, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257341, 'messageID': 2901}
 * {'messageKey': 'FleetJoinFleetFromLinkError', 'dataID': 17876324, 'suppressable': False, 'bodyID': 256832, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256831, 'messageID': 3108}
 * {'messageKey': 'FleetJoinRequestRejected', 'dataID': 17876900, 'suppressable': False, 'bodyID': 257050, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3066}
 * {'messageKey': 'FleetMemberAlreadyInGroup', 'dataID': 17881518, 'suppressable': False, 'bodyID': 258802, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1745}
 * {'messageKey': 'FleetMemberAlreadyInvited', 'dataID': 17881521, 'suppressable': False, 'bodyID': 258803, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1746}
 * {'messageKey': 'FleetMemberHasJoined', 'dataID': 17876903, 'suppressable': False, 'bodyID': 257051, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3068}
 * {'messageKey': 'FleetMemberJoinRequest', 'dataID': 17876897, 'suppressable': False, 'bodyID': 257049, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3065}
 * {'messageKey': 'FleetMemberNotFound', 'dataID': 17880084, 'suppressable': False, 'bodyID': 258256, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258255, 'messageID': 2106}
 * {'messageKey': 'FleetMembersDodgy1', 'dataID': 17883748, 'suppressable': True, 'bodyID': 259613, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 259612, 'messageID': 901}
 * {'messageKey': 'FleetMembersDodgyN', 'dataID': 17883753, 'suppressable': True, 'bodyID': 259615, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 259614, 'messageID': 902}
 * {'messageKey': 'FleetMustBeLeader', 'dataID': 17881524, 'suppressable': False, 'bodyID': 258804, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1747}
 * {'messageKey': 'FleetMustBeLeaderAndBoss', 'dataID': 17880124, 'suppressable': False, 'bodyID': 258272, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258271, 'messageID': 2160}
 * {'messageKey': 'FleetMustBeMember', 'dataID': 17881527, 'suppressable': False, 'bodyID': 258805, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1748}
 * {'messageKey': 'FleetMustSpecifyScope', 'dataID': 17876314, 'suppressable': False, 'bodyID': 256828, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256827, 'messageID': 3097}
 * {'messageKey': 'FleetNoInviteInStation', 'dataID': 17881530, 'suppressable': False, 'bodyID': 258806, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1749}
 * {'messageKey': 'FleetNoPositionFound', 'dataID': 17880144, 'suppressable': False, 'bodyID': 258280, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258279, 'messageID': 2186}
 * {'messageKey': 'FleetNoSuchFleet', 'dataID': 17881533, 'suppressable': False, 'bodyID': 258807, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1750}
 * {'messageKey': 'FleetNobodyHasAccess', 'dataID': 17876304, 'suppressable': False, 'bodyID': 256824, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256823, 'messageID': 3083}
 * {'messageKey': 'FleetNotAMember', 'dataID': 17880948, 'suppressable': False, 'bodyID': 258589, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1751}
 * {'messageKey': 'FleetNotAllowed', 'dataID': 17880109, 'suppressable': False, 'bodyID': 258266, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258265, 'messageID': 2122}
 * {'messageKey': 'FleetNotCommanderOrBoss', 'dataID': 17876651, 'suppressable': False, 'bodyID': 256957, 'messageType': 'hint', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3140}
 * {'messageKey': 'FleetNotCreator', 'dataID': 17880089, 'suppressable': False, 'bodyID': 258258, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258257, 'messageID': 2107}
 * {'messageKey': 'FleetNotFleet', 'dataID': 17880119, 'suppressable': False, 'bodyID': 258270, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258269, 'messageID': 2124}
 * {'messageKey': 'FleetNotFound', 'dataID': 17876309, 'suppressable': False, 'bodyID': 256826, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256825, 'messageID': 3093}
 * {'messageKey': 'FleetNotInFleet', 'dataID': 17880069, 'suppressable': False, 'bodyID': 258250, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258249, 'messageID': 2103}
 * {'messageKey': 'FleetNotInvited', 'dataID': 17880074, 'suppressable': False, 'bodyID': 258252, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258251, 'messageID': 2104}
 * {'messageKey': 'FleetNotLeader', 'dataID': 17880079, 'suppressable': False, 'bodyID': 258254, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258253, 'messageID': 2105}
 * {'messageKey': 'FleetNotMemberOfAnyFleet', 'dataID': 17880951, 'suppressable': False, 'bodyID': 258590, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1752}
 * {'messageKey': 'FleetNotNPCCorp', 'dataID': 17879237, 'suppressable': False, 'bodyID': 257933, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257932, 'messageID': 2466}
 * {'messageKey': 'FleetNotOpenToRegister', 'dataID': 17877428, 'suppressable': False, 'bodyID': 257247, 'messageType': 'hint', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2908}
 * {'messageKey': 'FleetNotSelfInvite', 'dataID': 17879170, 'suppressable': False, 'bodyID': 257907, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257906, 'messageID': 2463}
 * {'messageKey': 'FleetNotWingCommander', 'dataID': 17880099, 'suppressable': False, 'bodyID': 258262, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258261, 'messageID': 2110}
 * {'messageKey': 'FleetNotYourAlliance', 'dataID': 17879180, 'suppressable': False, 'bodyID': 257911, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257910, 'messageID': 2465}
 * {'messageKey': 'FleetNotYourCorp', 'dataID': 17879175, 'suppressable': False, 'bodyID': 257909, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257908, 'messageID': 2464}
 * {'messageKey': 'FleetPositionFilled', 'dataID': 17880229, 'suppressable': False, 'bodyID': 258313, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258312, 'messageID': 2113}
 * {'messageKey': 'FleetPositionFilledInvite', 'dataID': 17880249, 'suppressable': False, 'bodyID': 258321, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258320, 'messageID': 2171}
 * {'messageKey': 'FleetPositionFilledMove', 'dataID': 17880254, 'suppressable': False, 'bodyID': 258323, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258322, 'messageID': 2172}
 * {'messageKey': 'FleetRegroup', 'dataID': 17883756, 'suppressable': False, 'bodyID': 259616, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 903}
 * {'messageKey': 'FleetRemoveFleetFinderAd', 'dataID': 17876707, 'suppressable': True, 'bodyID': 256979, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256978, 'messageID': 3082}
 * {'messageKey': 'FleetTooManyFavorites', 'dataID': 17878295, 'suppressable': False, 'bodyID': 257574, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257573, 'messageID': 2567}
 * {'messageKey': 'FleetTooManyMembers', 'dataID': 17880234, 'suppressable': False, 'bodyID': 258315, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258314, 'messageID': 2158}
 * {'messageKey': 'FleetTooManyMembersConvert', 'dataID': 17880239, 'suppressable': False, 'bodyID': 258317, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258316, 'messageID': 2159}
 * {'messageKey': 'FleetTooManyMembersInSquad', 'dataID': 17880244, 'suppressable': False, 'bodyID': 258319, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258318, 'messageID': 2165}
 * {'messageKey': 'FleetTooManySquads', 'dataID': 17880259, 'suppressable': False, 'bodyID': 258325, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258324, 'messageID': 2111}
 * {'messageKey': 'FleetTooManyWings', 'dataID': 17880224, 'suppressable': False, 'bodyID': 258311, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258310, 'messageID': 2108}
 * {'messageKey': 'FleetUpdateFleetFinderAd', 'dataID': 17876508, 'suppressable': False, 'bodyID': 256902, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256901, 'messageID': 3070}
 * {'messageKey': 'FleetUpdateFleetFinderAd_ChangedCorp', 'dataID': 17875813, 'suppressable': False, 'bodyID': 256645, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256644, 'messageID': 3411}
 * {'messageKey': 'FleetUpdateFleetFinderAd_LastMember', 'dataID': 17876518, 'suppressable': False, 'bodyID': 256906, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256905, 'messageID': 3094}
 * {'messageKey': 'FleetUpdateFleetFinderAd_NewBoss', 'dataID': 17876503, 'suppressable': False, 'bodyID': 256900, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256899, 'messageID': 3095}
 * {'messageKey': 'FleetUpdateFleetFinderAd_Standing', 'dataID': 17876513, 'suppressable': False, 'bodyID': 256904, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256903, 'messageID': 3096}
 * {'messageKey': 'FleetWaitForLSC', 'dataID': 17877506, 'suppressable': False, 'bodyID': 257277, 'messageType': 'warning', 'urlAudio': '', 'urlIcon': '', 'titleID': 257276, 'messageID': 2946}
 * {'messageKey': 'FleetWarp', 'dataID': 17883759, 'suppressable': False, 'bodyID': 259617, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 904}
 * {'messageKey': 'FleetYouAreAlreadyInFleet', 'dataID': 17880129, 'suppressable': False, 'bodyID': 258274, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258273, 'messageID': 2161}
 * {'messageKey': 'FleetsterNotInSystem', 'dataID': 17883682, 'suppressable': False, 'bodyID': 259589, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 905}
 */

/*{'FullPath': u'UI/Messages', 'messageID': 257898, 'label': u'FleetAlreadyBoosterBody'}(u'This fleet member is already a Booster.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257899, 'label': u'FleetBoosterIllegalTitle'}(u'Cannot be a Booster', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257900, 'label': u'FleetBoosterIllegalBody'}(u'This fleet member cannot be converted to the requested Booster role.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257901, 'label': u'FleetBoosterRoleFullBody'}(u'This Booster role is already taken.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257902, 'label': u'FleetConfirmAutoJoinVoiceTitle'}(u'Join Fleet Voice?', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257903, 'label': u'FleetConfirmAutoJoinVoiceBody'}(u"The Fleet Leader wishes the fleet to be voice enabled through EVE Voice. Would you like to automatically join relevant voice channels in this fleet?<br>\r\n<br>\r\nIf you choose 'no' you will not be asked again but you will still be able to join voice manually.", None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257904, 'label': u'FleetConfirmVoiceEnableTitle'}(u'Voice Enable Fleet?', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257905, 'label': u'FleetConfirmVoiceEnableBody'}(u'Do you want to request that all the current and future members of this fleet join the fleet audio channels for voice communication?\r\n<br><br>Note that each fleet member can choose not to join audio if you select this option. Additionally, even though this option is not selected each member can manually join audio channels in the fleet.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257906, 'label': u'FleetNotSelfInviteTitle'}(u'No Self Invites', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257907, 'label': u'FleetNotSelfInviteBody'}(u'This fleet does not allow self-invites.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257908, 'label': u'FleetNotYourCorpTitle'}(u'Not your corporation', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257909, 'label': u'FleetNotYourCorpBody'}(u'You need to be in the same corporation as the boss in order to join this fleet.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257910, 'label': u'FleetNotYourAllianceTitle'}(u'Not your alliance', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257911, 'label': u'FleetNotYourAllianceBody'}(u'You need to be in the same alliance as the boss in order to join this fleet.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257912, 'label': u'FleetCannotDoInStationTitle'}(u'You must be in space', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257913, 'label': u'FleetCannotDoInStationBody'}(u'You cannot perform this Fleet action in a station. You must be in space to do that.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257932, 'label': u'FleetNotNPCCorpTitle'}(u'Not in an NPC Corp', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257933, 'label': u'FleetNotNPCCorpBody'}(u'You cannot set your fleet to "Self-Invite" if you are in an NPC corporation.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258248, 'label': u'FleetAlreadyMovingBody'}(u'One or more of your fleet members are already being shuffled around. Please try again in a few moments.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258249, 'label': u'FleetNotInFleetTitle'}(u'Not in a fleet', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258250, 'label': u'FleetNotInFleetBody'}(u'You do not appear to be in a fleet.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258251, 'label': u'FleetNotInvitedTitle'}(u'Not invited', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258252, 'label': u'FleetNotInvitedBody'}(u'You do not have an outstanding invitation to this fleet. It might have expired in which case you should contact the fleet leader to get a new invitation.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258253, 'label': u'FleetNotLeaderTitle'}(u'Not fleet leader', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258254, 'label': u'FleetNotLeaderBody'}(u'You are not the leader of your fleet. You must have fleet leader role to execute that command.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258255, 'label': u'FleetMemberNotFoundTitle'}(u'Member not found', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258256, 'label': u'FleetMemberNotFoundBody'}(u'The requested member was not found in your fleet. It is possible he or she has left the fleet.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258257, 'label': u'FleetNotCreatorTitle'}(u'Not boss', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258258, 'label': u'FleetNotCreatorBody'}(u'You are not the fleet boss. Only the creator of the fleet or his successor may do that.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258259, 'label': u'FleetCannotDeleteNonEmptyWingTitle'}(u'Wing not empty', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258260, 'label': u'FleetCannotDeleteNonEmptyWingBody'}(u'The wing you are trying to remove contains members. You cannot remove the wing unless it is empty. Please move all the wing members to another wing before trying again.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258261, 'label': u'FleetNotWingCommanderTitle'}(u'You are not Wing Commander', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258262, 'label': u'FleetNotWingCommanderBody'}(u"You are not this wing's Wing Commander and can therefore not execute this command.", None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258263, 'label': u'FleetCannotDeleteNonEmptySquadTitle'}(u'Squad not empty', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258264, 'label': u'FleetCannotDeleteNonEmptySquadBody'}(u'The squad you are trying to remove contains members. You cannot remove the squad unless it is empty. Please move all the squad members to another squad before trying again.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258265, 'label': u'FleetNotAllowedTitle'}(u'Not allowed', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258266, 'label': u'FleetNotAllowedBody'}(u'You do not have permission to execute this command in your fleet.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258267, 'label': u'FleetConfirmDemoteSelfTitle'}(u'Demote yourself?', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258268, 'label': u'FleetConfirmDemoteSelfBody'}(u'Are you sure you want to demote yourself? You will lose command doing so!', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258269, 'label': u'FleetNotFleetTitle'}(u'Not a fleet', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258270, 'label': u'FleetNotFleetBody'}(u'Your gang is required to be in a <b>Fleet</b> configuration to perform this action.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258271, 'label': u'FleetMustBeLeaderAndBossTitle'}(u'Must be Leader and Boss', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258272, 'label': u'FleetMustBeLeaderAndBossBody'}(u'You must have the role Fleet Commander <b>and</b> be the Fleet Boss in order to complete this operation.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258273, 'label': u'FleetYouAreAlreadyInFleetTitle'}(u'Already in a fleet', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258274, 'label': u'FleetYouAreAlreadyInFleetBody'}(u'You appear to be already in a fleet.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258275, 'label': u'FleetCantKickBossTitle'}(u'Cannot kick Boss', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258276, 'label': u'FleetCantKickBossBody'}(u"You cannot kick the Boss in your fleet. It wouldn't be right!", None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258277, 'label': u'FleetCannotSendBroadcastToSelectedGroupTitle'}(u'Cannot broadcast to group', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258278, 'label': u'FleetCannotSendBroadcastToSelectedGroupBody'}(u'Your fleet role prohibits you from sending that broadcast to the selected group of fleet members.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258279, 'label': u'FleetNoPositionFoundTitle'}(u'No Position found', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258280, 'label': u'FleetNoPositionFoundBody'}(u'No position found for the invitee. You might have to create an additional squad to fit this member.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258310, 'label': u'FleetTooManyWingsTitle'}(u'Too many wings', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258311, 'label': u'FleetTooManyWingsBody'}(u'You cannot create a new wing because your fleet already contains the maximum of <b>{num}</b> wings.', None, {u'{num}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'num'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258312, 'label': u'FleetPositionFilledTitle'}(u'Position filled', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258313, 'label': u'FleetPositionFilledBody'}(u'<b>{owner}</b> is already in that fleet position. You will have to remove the member from the position before you can place this member there.', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258314, 'label': u'FleetTooManyMembersTitle'}(u'Too many members', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258315, 'label': u'FleetTooManyMembersBody'}(u'Your fleet can support at most <b>{num}</b> members in the current configuration and you cannot invite any more.', None, {u'{num}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'num'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258316, 'label': u'FleetTooManyMembersConvertTitle'}(u'Too many members', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258317, 'label': u'FleetTooManyMembersConvertBody'}(u'Your gang can support at most <b>{num}</b> members in the configuration you are converting to. You currently have <b>{curr}</b> members in your gang and will have to kick some of them in order to complete this operation.', None, {u'{curr}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'curr'}, u'{num}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'num'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258318, 'label': u'FleetTooManyMembersInSquadTitle'}(u'Too many members in squad', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258319, 'label': u'FleetTooManyMembersInSquadBody'}(u'Squads can support at most <b>{num}</b> members and you cannot add any more.', None, {u'{num}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'num'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258320, 'label': u'FleetPositionFilledInviteTitle'}(u'Position filled', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258321, 'label': u'FleetPositionFilledInviteBody'}(u'There is an outstanding invitation to <b>{owner}</b> for this fleet position.<br>\r\nYou will have to wait to see if the pending invitation is rejected or times out if you wish to place someone else into this position.', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258322, 'label': u'FleetPositionFilledMoveTitle'}(u'Position filled', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258323, 'label': u'FleetPositionFilledMoveBody'}(u'<b>{owner}</b> is being moved into this fleet position.<br><br>\r\nYou cannot move another person into this position unless the pending move operation does not succeed.', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258324, 'label': u'FleetTooManySquadsTitle'}(u'Too many squads', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258325, 'label': u'FleetTooManySquadsBody'}(u'You cannot create a new squad in this wing because it already contains the maximum of <b>{num}</b> squads.', None, {u'{num}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'num'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258589, 'label': u'FleetNotAMemberBody'}(u'This person is either not a member of your fleet or not present in this solar system.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258590, 'label': u'FleetNotMemberOfAnyFleetBody'}(u'You are not a member of any fleet.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258614, 'label': u'FleetCandidateOfflineTitle'}(u'Invitee Offline', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258615, 'label': u'FleetCandidateOfflineBody'}(u'{invitee} is offline and thus of no value to your fleet.', None, {u'{invitee}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'invitee'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258800, 'label': u'FleetAlreadyLeaderBody'}(u'There is already a leader in this fleet.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258801, 'label': u'FleetCandidateNotInvitedBody'}(u'There is no invitation to join the fleet standing.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258802, 'label': u'FleetMemberAlreadyInGroupBody'}(u'This person is already in your fleet.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258803, 'label': u'FleetMemberAlreadyInvitedBody'}(u'This person already has an invitation to your fleet.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258804, 'label': u'FleetMustBeLeaderBody'}(u"You must be the fleet's leader to perform this operation.", None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258805, 'label': u'FleetMustBeMemberBody'}(u'You must be a member of a fleet.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258806, 'label': u'FleetNoInviteInStationBody'}(u'You cannot invite someone to fleet with you while you are in station.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258807, 'label': u'FleetNoSuchFleetBody'}(u'This fleet does not exist.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259589, 'label': u'FleetsterNotInSystemBody'}(u'Gangster not in system', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259608, 'label': u'FleetCandidateDodgy1Title'}(u'Fleet Candidate Dodgy', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259609, 'label': u'FleetCandidateDodgy1Body'}(u'The pilot that you wish to add to your fleet is at war with the corporation <b>{corpName}</b>. Confirming that you want him to join your fleet might make all your fleet members vulnerable to this corporation, however they will not be vulnerable to other members of your fleet that are not at war with this corporation unless they decide to attack you. Are you sure you want to add this pilot to your fleet?', None, {u'{corpName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'corpName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259610, 'label': u'FleetCandidateDodgyNTitle'}(u'Fleet Candidate Dodgy', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259611, 'label': u'FleetCandidateDodgyNBody'}(u'The pilot that you wish to add to your fleet is at war with these corporations <b>{corpNames}</b>. Confirming that you want him to join your fleet might make all your fleet members vulnerable to these corporations, however they will not be vulnerable to other members of your fleet that are not associated with war with these corporations unless they decide to attack you. Are you sure you want to add this pilot to your fleet?', None, {u'{corpNames}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'corpNames'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259612, 'label': u'FleetMembersDodgy1Title'}(u'Join Warring Fleet?', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259613, 'label': u'FleetMembersDodgy1Body'}(u'{name} wants you to join their fleet, do you accept?<br><br>This fleet has members that are at war with the corporation {corpName}. Confirming that you will join this fleet will make you vulnerable to this corporation, however they will not be vulnerable to you unless they decide to attack you. Are you sure you want to join this fleet?<br><br>NOTE: Attacking members of your fleet is not a CONCORD sanctioned activity and may result in security status loss and a police response.', None, {u'{name}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'name'}, u'{corpName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'corpName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259614, 'label': u'FleetMembersDodgyNTitle'}(u'Join Warring Fleet?', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259615, 'label': u'FleetMembersDodgyNBody'}(u'{name} wants you to join their fleet, do you accept?<br><br>This fleet has members that are associated with war with these corporations {corpNames}. Confirming that you will join this fleet will make you vulnerable to these corporations, however they will not be vulnerable to you unless they decide to attack you. Are you sure you want to join this fleet?<br><br>NOTE: Attacking members of your gang is not a CONCORD sanctioned activity and may result in security status loss and a police response.', None, {u'{name}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'name'}, u'{corpNames}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'corpNames'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259616, 'label': u'FleetRegroupBody'}(u'Regrouping to {leader}', None, {u'{leader}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'leader'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259617, 'label': u'FleetWarpBody'}(u'Following {leader} in warp', None, {u'{leader}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'leader'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259639, 'label': u'FleetCONCORDSpamPreventionActConfirmationTitle'}(u'Accept CSPA Service Charge', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259640, 'label': u'FleetCONCORDSpamPreventionActConfirmationBody'}(u'This fleet invitation is subject to a CSPA service charge of {amount} ISK, which you must accept to complete the invitation.<br><br>According to the CONCORD Spam Prevention Act, all communication initiations between parties that have not explicitly whitelisted each other through the use of a central address book shall be subject to a nominal service fee which shall be collected by the fluid comm operator. {char} has not whitelisted you by any such means.', None, {u'{amount}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'amount'}, u'{char}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'char'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256644, 'label': u'FleetUpdateFleetFinderAd_ChangedCorpTitle'}(u'Fleet Finder Update', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 256645, 'label': u'FleetUpdateFleetFinderAd_ChangedCorpBody'}(u"You have changed your corporation and therefore your fleet's advert in the Fleet Finder has been removed.<br> Would you like to enter a new advert?", None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257276, 'label': u'FleetWaitForLSCTitle'}(u'Initiating Fleet Chat', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257277, 'label': u'FleetWaitForLSCBody'}(u'CONCORDs Initiative Against Anti-Social Behavior (CIAASB) stipulates that fleet members must have a chance to greet you before you resign your post.<BR>\r\nPlease wait while we connect you to your fleet channel before trying again.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257574, 'label': u'FleetTooManyFavoritesBody'}(u' Watch lists can support at most <b>{num}</b> members and you cannot add any more.', None, {u'{num}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'num'}})
 */