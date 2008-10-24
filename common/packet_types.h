/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef EVE_PACKET_TYPES_H
#define EVE_PACKET_TYPES_H

#include "types.h"

/*                                                                              
 *
SERVICE_STOPPED = 1
SERVICE_START_PENDING = 2
SERVICE_STOP_PENDING = 3
SERVICE_RUNNING = 4
SERVICE_CONTINUE_PENDING = 5
SERVICE_PAUSE_PENDING = 6
SERVICE_PAUSED = 7
SERVICETYPE_NORMAL = 1
SERVICETYPE_BUILTIN = 2
SERVICETYPE_EXPORT_CONSTANTS = 4
SERVICE_CONTROL_STOP = 1
SERVICE_CONTROL_PAUSE = 2
SERVICE_CONTROL_CONTINUE = 3
SERVICE_CONTROL_INTERROGATE = 4
SERVICE_CONTROL_SHUTDOWN = 5
SERVICE_CHECK_NONE = 0
SERVICE_CHECK_CALL = 1
SERVICE_CHECK_INIT = 2
SERVICE_WANT_SESSIONS = 1
ROLE_LOGIN = 1L
ROLE_PLAYER = 2L
ROLE_GDNPC = 4L
ROLE_GML = 8L
ROLE_GMH = 16L
ROLE_ADMIN = 32L
ROLE_SERVICE = 64L
ROLE_HTTP = 128L
ROLE_PETITIONEE = 256L
ROLE_GDL = 512L
ROLE_GDH = 1024L
ROLE_CENTURION = 2048L
ROLE_WORLDMOD = 4096L
ROLE_QA = 8192L
ROLE_EBS = 16384L
ROLE_ROLEADMIN = 32768L
ROLE_PROGRAMMER = 65536L
ROLE_REMOTESERVICE = 131072L
ROLE_LEGIONEER = 262144L
ROLE_TRANSLATION = 524288L
ROLE_CHTINVISIBLE = 1048576L
ROLE_CHTADMINISTRATOR = 2097152L
ROLE_HEALSELF = 4194304L
ROLE_HEALOTHERS = 8388608L
ROLE_NEWSREPORTER = 16777216L
ROLE_HOSTING = 33554432L
ROLE_BROADCAST = 67108864L
ROLE_TRANSLATIONADMIN = 134217728L
ROLE_N00BIE = 268435456L
ROLE_ACCOUNTMANAGEMENT = 536870912L
ROLE_DUNGEONMASTER = 1073741824L
ROLE_IGB = 2147483648L
ROLE_TRANSLATIONEDITOR = 4294967296L
ROLE_SPAWN = 8589934592L
ROLE_ANY = (18446744073709551615L & ~ROLE_IGB)
ROLEMASK_ELEVATEDPLAYER = (ROLE_ANY & ~(((ROLE_LOGIN | ROLE_PLAYER) | ROLE_N00BIE) | ROLE_NEWSREPORTER))
ROLEMASK_VIEW = ((((ROLE_GML | ROLE_ADMIN) | ROLE_GDL) | ROLE_HOSTING) | ROLE_QA)
PRE_NONE = 0
PRE_AUTH = 1
PRE_HASCHAR = 2
PRE_HASSHIP = 4
PRE_INSTATION = 8
PRE_INFLIGHT = 16

*/

typedef enum {
	AUTHENTICATION_REQ = 0,
	AUTHENTICATION_RSP = 1,
	IDENTIFICATION_REQ = 2,
	IDENTIFICATION_RSP = 3,
	__Fake_Invalid_Type,
	CALL_REQ = 6,
	CALL_RSP = 7,
	TRANSPORTCLOSED = 8,
	RESOLVE_REQ = 10,
	RESOLVE_RSP = 11,
	NOTIFICATION = 12,
	ERRORRESPONSE = 15,
	SESSIONCHANGENOTIFICATION = 16,
	SESSIONINITIALSTATENOTIFICATION = 18,
	PING_REQ = 20,
	PING_RSP = 21,
	_MACHONETMSG_TYPE_next
} MACHONETMSG_TYPE;

typedef enum {
	UNMACHODESTINATION = 0,
	UNMACHOCHANNEL = 1,
	WRAPPEDEXCEPTION = 2
} MACHONETERR_TYPE;
//see PyPacket.cpp
extern const char *MACHONETMSG_TYPE_str[_MACHONETMSG_TYPE_next];

//item types which we need to know about:
enum EVEItemType {
	AllianceTypeID = 16159
};

//these came from the 'constants' object:
enum EVEItemChangeType {
	ixItemID = 0,	//also ixLauncherCapacity?
	ixTypeID = 1,	//also ixLauncherUsed = 1,
	ixOwnerID = 2,	//also ixLauncherChargeItem?
	ixLocationID = 3,
	ixFlag = 4,
	ixContraband = 5,
	ixSingleton = 6,
	ixGroupID = 8,
	ixQuantity = 7,
	ixCategoryID = 9,
	ixCustomInfo = 10,
	ixSubitems = 11
};

enum EVEContainerTypes {
	containerWallet 			= 10001,
	containerGlobal 			= 10002,
	containerSolarSystem 		= 10003,
	containerHangar 			= 10004,
	containerScrapHeap			= 10005,
	containerFactory			= 10006,
	containerBank 				= 10007,
	containerRecycler			= 10008,
	containerOffices			= 10009,
	containerCharacter 			= 10011,
	containerStationCharacters	= 10010,
	containerCorpMarket			= 10012
};


#include "tables/invCategories.h"
typedef EVEDB::invCategories::invCategories EVEItemCategories;


typedef enum EVEItemFlags {	//from eveFlags DB table
	flagAutoFit = 0,
	flagWallet = 1,
	flagFactory = 2,
	flagHangar = 4,
	flagCargoHold = 5,
	flagBriefcase = 6,
	flagSkill = 7,
	flagReward = 8,
	flagConnected = 9,	//Character in station connected
	flagDisconnected = 10,	//Character in station offline
	
	//ship fittings:
	flagLowSlot0 = 11,
		flagLowSlot1 = 12,
		flagLowSlot8 = 18,
		flagMedSlot0 = 19,
		flagMedSlot1 = 20,
		flagMedSlot8 = 26,
		flagHiSlot0 = 27,
		flagHiSlot1 = 28,
		flagHiSlot7 = 34,
	flagFixedSlot = 35,	//also 

	//factory stuff:
	flagFactoryBlueprint = 36,
	flagFactoryMinerals = 37,
	flagFactoryOutput = 38,
	flagFactoryActive = 39,
	flagFactory_SlotFirst = 40,
	flagFactory_SlotLast = 55,

	flagCapsule = 56,	//in space??
	flagPilot = 57,
	flagPassenger = 58,
	flagBoardingGate = 59,
	flagCrew = 60,
	flagSkillInTraining = 61,
	flagCorpMarket = 62,	// Corporation Market Deliveries / Returns
	flagLocked = 63,		//Locked item, can not be moved unless unlocked
	flagUnlocked = 64,
	
	flagOfficeSlotFirst = 70,
	flagOfficeSlotLast = 85,

	flagBonus = 86,		//Char bonus/penalty
	flagDroneBay = 87,
	flagBooster = 88,
	flagImplant = 89,
	flagShipHangar = 90,
	flagShipOffline = 91,

	flagResearchFacilitySlot100First = 100,	//no idea what makes this different than the one below
	flagResearchFacilitySlot100Last = 115,

	flagCorpSecurityAccessGroup2 = 116,
	flagCorpSecurityAccessGroup3 = 117,
	flagCorpSecurityAccessGroup4 = 118,
	flagCorpSecurityAccessGroup5 = 119,
	flagCorpSecurityAccessGroup6 = 120,
	flagCorpSecurityAccessGroup7 = 121,
	
	flagFactorySlotFirst = 140,
	flagFactorySlotLast = 195,
	
	flagResearchFacilitySlotFirst = 200,
	flagResearchFacilitySlotLast = 255
} EVEItemFlags;
//some alternative names for entries above.
static const EVEItemFlags flagSlotFirst = flagLowSlot0;	//duplicate values
static const EVEItemFlags flagSlotLast = flagFixedSlot;
static const EVEItemFlags flagAnywhere = flagAutoFit;
static const uint8 MAX_MODULE_COUNT = flagSlotLast - flagSlotFirst + 1;

//TODO: improve/complete this:
typedef enum {
       skillGunnery = 3300,
       skillSmallHybridTurret = 3301,
       skillSpaceshipCommand = 3327,
       skillCaldariFrigate = 3330,
       skillIndustry = 3380,
       skillRefining = 3385,
       skillMining = 3386,
       skillRefineryEfficiency = 3389,
       skillMechanic = 3392,
       skillScience = 3402,
       skillResearch = 3403,
       skillEngineering = 3413,
       skillDrones = 3436,
       skillMiningDroneOperation = 3438,
       skillScrapmetalProcessing = 12196
} EVESkillID;

//List of eve item types which have special purposes in the game. 
//try to keep this list as short as possible, most things should be accomplishable
//by looking at the atributes of an item, not its type.
typedef enum {
	itemTypeCapsule = 670
} EVEItemTypeID;

//raceID as in table 'entity'
enum EVERace {
	raceCaldari = 1,
	raceMinmatar = 2,
	raceAmarr = 4,
	raceGallente = 8,
	raceJove = 16,
	racePirate = 32
};

//eve standing change messages
//If oFromID and oToID != fromID and toID, the following message is added (except for those marked with x):
//This standing change was initiated by a change from _oFromID towards _oToID
typedef enum {	//chrStandingChanges.eventTypeID
	standingGMInterventionReset = 25,		//Reset by a GM.
	standingDecay = 49,						//All standing decays except when user isn't logged in
	standingPlayerSet = 65,					//Set by player him/herself. Reason: _msg
	standingCorpSet = 68,					//Corp stand set by _int1. Reason: _msg
	standingMissionCompleted = 73,			//_msg: name of mission
	standingMissionFailure = 74,			//_msg: name of mission
	standingMissionDeclined = 75,			//_msg: name of mission
	standingCombatAggression = 76,			//Combat - Aggression
	standingCombatShipKill = 77,			//Combat - Ship Kill
	standingCombatPodKill = 78,				//Combat - Pod Kill
	standingDerivedModificationPleased = 82,//_fromID Corp was pleased
	standingDerivedModificationDispleased = 83,	//_fromID Corp was displeased
	standingGMInterventionDirect = 84,		//Mod directly by _int1. Reason: _msg
	standingLawEnforcement = 89,			//Granted by Concord for actions against _int1
	standingMissionOfferExpired = 90,		//Mission Offer Expired - _msg
	standingCombatAssistance = 112,			//Combat - Assistance
	standingPropertyDamage = 154			//Property Damage
	//anything up until 500 is 'Standing Change'
} EVEStandingEventTypeID;


enum {
	ROLE_LOGIN = 1L,
	ROLE_PLAYER = 2L,
	ROLE_GDNPC = 4L,
	ROLE_GML = 8L,
	ROLE_GMH = 16L,
	ROLE_ADMIN = 32L,
	ROLE_SERVICE = 64L,
	ROLE_HTTP = 128L,
	ROLE_PETITIONEE = 256L,
	ROLE_GDL = 512L,
	ROLE_GDH = 1024L,
	ROLE_CENTURION = 2048L,
	ROLE_WORLDMOD = 4096L,
	ROLE_QA = 8192L,
	ROLE_EBS = 16384L,
	ROLE_ROLEADMIN = 32768L,
	ROLE_PROGRAMMER = 65536L,
	ROLE_REMOTESERVICE = 131072L,
	ROLE_LEGIONEER = 262144L,
	ROLE_SLASH = (ROLE_GML | ROLE_LEGIONEER),
	ROLE_TRANSLATION = 524288L,
	ROLE_CHTINVISIBLE = 1048576L,
	ROLE_CHTADMINISTRATOR = 2097152L,
	ROLE_HEALSELF = 4194304L,
	ROLE_HEALOTHERS = 8388608L,
	ROLE_NEWSREPORTER = 16777216L,
	ROLE_HOSTING = 33554432L,
	ROLE_BROADCAST = 67108864L,
	ROLE_TRANSLATIONADMIN = 134217728L,
	ROLE_N00BIE = 268435456L,
	ROLE_ACCOUNTMANAGEMENT = 536870912L,
	ROLE_DUNGEONMASTER = 1073741824L,
	ROLE_IGB = 2147483648UL,
	ROLE_TRANSAM = (ROLE_TRANSLATION | ROLE_TRANSLATIONADMIN),// | ROLE_TRANSLATIONEDITOR)
	//ROLE_TRANSLATIONEDITOR = 4294967296UL
	//ROLE_SPAWN = 8589934592L
};

enum {
	corpRoleLocationTypeHQ = 1LL,
    corpRoleLocationTypeBase = 2LL,
	corpRoleLocationTypeOther = 3LL,
};

typedef uint64 CorpRoleFlags;
static const CorpRoleFlags corpRoleDirector = 1LL;
static const CorpRoleFlags corpRolePersonnelManager = 128LL;
static const CorpRoleFlags corpRoleAccountant = 256LL;
static const CorpRoleFlags corpRoleSecurityOfficer = 512LL;
static const CorpRoleFlags corpRoleFactoryManager = 1024LL;
static const CorpRoleFlags corpRoleStationManager = 2048LL;
static const CorpRoleFlags corpRoleAuditor = 4096LL;
static const CorpRoleFlags corpRoleHangarCanTake1 = 8192LL;
static const CorpRoleFlags corpRoleHangarCanTake2 = 16384LL;
static const CorpRoleFlags corpRoleHangarCanTake3 = 32768LL;
static const CorpRoleFlags corpRoleHangarCanTake4 = 65536LL;
static const CorpRoleFlags corpRoleHangarCanTake5 = 131072LL;
static const CorpRoleFlags corpRoleHangarCanTake6 = 262144LL;
static const CorpRoleFlags corpRoleHangarCanTake7 = 524288LL;
static const CorpRoleFlags corpRoleHangarCanQuery1 = 1048576LL;
static const CorpRoleFlags corpRoleHangarCanQuery2 = 2097152LL;
static const CorpRoleFlags corpRoleHangarCanQuery3 = 4194304LL;
static const CorpRoleFlags corpRoleHangarCanQuery4 = 8388608LL;
static const CorpRoleFlags corpRoleHangarCanQuery5 = 16777216LL;
static const CorpRoleFlags corpRoleHangarCanQuery6 = 33554432LL;
static const CorpRoleFlags corpRoleHangarCanQuery7 = 67108864LL;
static const CorpRoleFlags corpRoleAccountCanTake1 = 134217728LL;
static const CorpRoleFlags corpRoleAccountCanTake2 = 268435456LL;
static const CorpRoleFlags corpRoleAccountCanTake3 = 536870912LL;
static const CorpRoleFlags corpRoleAccountCanTake4 = 1073741824LL;
static const CorpRoleFlags corpRoleAccountCanTake5 = 2147483648LL;
static const CorpRoleFlags corpRoleAccountCanTake6 = 4294967296LL;
static const CorpRoleFlags corpRoleAccountCanTake7 = 8589934592LL;
static const CorpRoleFlags corpRoleAccountCanQuery1 = 17179869184LL;
static const CorpRoleFlags corpRoleAccountCanQuery2 = 34359738368LL;
static const CorpRoleFlags corpRoleAccountCanQuery3 = 68719476736LL;
static const CorpRoleFlags corpRoleAccountCanQuery4 = 137438953472LL;
static const CorpRoleFlags corpRoleAccountCanQuery5 = 274877906944LL;
static const CorpRoleFlags corpRoleAccountCanQuery6 = 549755813888LL;
static const CorpRoleFlags corpRoleAccountCanQuery7 = 1099511627776LL;
static const CorpRoleFlags corpRoleEquipmentConfig = 2199023255552LL;
static const CorpRoleFlags corpRoleContainerCanTake1 = 4398046511104LL;
static const CorpRoleFlags corpRoleContainerCanTake2 = 8796093022208LL;
static const CorpRoleFlags corpRoleContainerCanTake3 = 17592186044416LL;
static const CorpRoleFlags corpRoleContainerCanTake4 = 35184372088832LL;
static const CorpRoleFlags corpRoleContainerCanTake5 = 70368744177664LL;
static const CorpRoleFlags corpRoleContainerCanTake6 = 140737488355328LL;
static const CorpRoleFlags corpRoleContainerCanTake7 = 281474976710656LL;
static const CorpRoleFlags corpRoleCanRentOffice = 562949953421312LL;
static const CorpRoleFlags corpRoleCanRentFactorySlot = 1125899906842624LL;
static const CorpRoleFlags corpRoleCanRentResearchSlot = 2251799813685248LL;
static const CorpRoleFlags corpRoleJuniorAccountant = 4503599627370496LL;
static const CorpRoleFlags corpRoleStarbaseConfig = 9007199254740992LL;
static const CorpRoleFlags corpRoleTrader = 18014398509481984LL;
static const CorpRoleFlags corpRoleChatManager = 36028797018963968LL;
//some combos:
static const CorpRoleFlags corpRoleAllHangar = 
		corpRoleHangarCanTake1|corpRoleHangarCanTake2|
		corpRoleHangarCanTake3|corpRoleHangarCanTake4|
		corpRoleHangarCanTake5|corpRoleHangarCanTake6|
		corpRoleHangarCanTake7|corpRoleHangarCanQuery1|
		corpRoleHangarCanQuery2|corpRoleHangarCanQuery3|
		corpRoleHangarCanQuery4|corpRoleHangarCanQuery5|
		corpRoleHangarCanQuery6|corpRoleHangarCanQuery7;
static const CorpRoleFlags corpRoleAllAccount = corpRoleJuniorAccountant |
		corpRoleAccountCanTake1|corpRoleAccountCanTake2|
		corpRoleAccountCanTake3|corpRoleAccountCanTake4|
		corpRoleAccountCanTake5|corpRoleAccountCanTake6|
		corpRoleAccountCanTake7|corpRoleAccountCanQuery1|
		corpRoleAccountCanQuery2|corpRoleAccountCanQuery3|
		corpRoleAccountCanQuery4|corpRoleAccountCanQuery5|
		corpRoleAccountCanQuery6|corpRoleAccountCanQuery7;
static const CorpRoleFlags corpRoleAllContainer = 
		corpRoleContainerCanTake1|corpRoleContainerCanTake2|
		corpRoleContainerCanTake3|corpRoleContainerCanTake4|
		corpRoleContainerCanTake5|corpRoleContainerCanTake6|
		corpRoleContainerCanTake7;
static const CorpRoleFlags corpRoleAllOffice = corpRoleCanRentOffice|corpRoleCanRentFactorySlot|corpRoleCanRentResearchSlot;
static const CorpRoleFlags corpRoleAll = 
		corpRoleAllHangar | corpRoleAllAccount | corpRoleAllContainer |
		corpRoleAllOffice |
		corpRoleDirector | corpRolePersonnelManager | corpRoleAccountant |
		corpRoleSecurityOfficer | corpRoleFactoryManager |
		corpRoleStationManager | corpRoleAuditor | corpRoleStarbaseConfig |
		corpRoleEquipmentConfig | corpRoleTrader | corpRoleChatManager;


//these come from dgmEffects.
typedef enum {
	effectTargetAttack = 10,	//effects.Laser
	effectLoPower = 11,
	effectHiPower = 12,
	effectMedPower = 13,
	effectOnline = 16,
	effectProjectileFired = 34,
	effectMiningLaser = 67,		//effects.Laser
	effectSkillEffect = 132,
	effectAnchorDrop = 649,
	effectAnchorLift = 650,
	effectOnlineForStructures = 901,
	effectAnchorDropForStructures = 1022,
	effectAnchorLiftForStructures = 1023
} EVEEffectID;

typedef enum JournalRefType {
	RefType_corpAccountWithdrawal = 37,
	RefType_corpBulkPayment = 47,
	RefType_corpDividendPayment = 38,
	RefType_corpLogoChange = 40,
	RefType_corpPayment = 11,
	RefType_corpRegFee = 39,
	RefType_officeRentalFee = 13,
	RefType_playerDonation = 10
} JournalRefType;

//from market_keyMap
typedef enum {
	accountCash = 1000,
	accountProperty = 1100,
	accountEscrow = 1500,
	accountReceivables = 1800,
	accountPayables = 2000,
	accountGold = 2010,
	accountEquity = 2900,
	accountSales = 3000,
	accountPurchases = 4000
} EVEAccountKeys;

//the constants are made up of:
//  prefix     -> cachedObject
//	            config.BulkData.constants
// 	category   -> config.BulkData.categories
// 	group      -> config.BulkData.groups
// 	metaGreoup -> config.BulkData.metagroups
// 	attribute  -> config.BulkData.dgmattribs
// 	effect     -> config.BulkData.dgmeffects
//	billType   -> config.BulkData.billtypes
// 	role       -> config.Roles
// 	flag       -> config.Flags
// 	race       -> config.Races
// 	bloodline  -> config.Bloodlines
// 	statistic  -> config.Statistics
// 	unit       -> config.Units
// 	channelType -> config.ChannelTypes
// 	encyclopediaType -> config.EncyclopediaTypes
// 	activity   -> config.BulkData.ramactivities
// 	completedStatus -> config.BulkData.ramcompletedstatuses
//  
// First letter of `Name` field if capitalized when prefixed.
// see InsertConstantsFromRowset


//message format argument types:
typedef enum {
	fmtMapping_OWNERID2 = 1,	//not used? owner name
	fmtMapping_OWNERID = 2,	//owner name
	fmtMapping_LOCID = 3,	//locations
	fmtMapping_itemTypeName = 4,	//TYPEID: takes the item ID
	fmtMapping_itemTypeDescription = 5,	//TYPEID2: takes the item ID
	fmtMapping_blueprintTypeName = 6,	//from invBlueprints
	fmtMapping_itemGroupName = 7,	//GROUPID: takes the item group ID
	fmtMapping_itemGroupDescription = 8,	//GROUPID2: takes the item group ID
	fmtMapping_itemCategoryName = 9,	//CATID: takes the item category ID
	fmtMapping_itemCategoryDescription = 10,	//CATID2: takes the item category ID
	fmtMapping_DGMATTR = 11,	//not used...
	fmtMapping_DGMFX = 12,		//not used...
	fmtMapping_DGMTYPEFX = 13,	//not used...
	fmtMapping_dateTime = 14,	//DATETIME: formatted date and time
	fmtMapping_date = 15,		//DATE: formatted date
	fmtMapping_time = 16,		//TIME: formatted time
	fmtMapping_shortTime = 17,	//TIMESHRT: formatted time, short format
	fmtMapping_long = 18,		//AMT: regular number format
	fmtMapping_ISK2 = 19,		//AMT2: ISK format
	fmtMapping_ISK3 = 20,		//AMT3: ISK format
	fmtMapping_distance = 21,		//DIST: distance format
	fmtMapping_message = 22,	//MSGARGS: nested message
	fmtMapping_ADD_THE = 22,	//ADD_THE: prefix argument with 'the '
	fmtMapping_ADD_A = 23,		//ADD_A: prefix argument with 'a ' or 'an ' as appropriate
	fmtMapping_typeQuantity = 24,	//TYPEIDANDQUANTITY: human readable representation of the two arguments: typeID and quantity
	fmtMapping_ownerNickname = 25,	//OWNERIDNICK: first part of owner name (before space)
	fmtMapping_station = 26,	//SESSIONSENSITIVESTATIONID: human readable, fully qualified station name (includes system, constellation and region)
	fmtMapping_system = 27,	//SESSIONSENSITIVELOCID: human readable, fully qualified system name (includes constellation and region)
	fmtMapping_ISK = 28,		//ISK: ISK format
	fmtMapping_TYPEIDL = 29
} fmtMappingType;

typedef enum {
	dgmEffPassive = 0,
	dgmEffActivation = 1,
	dgmEffTarget = 2,
	dgmEffArea = 3,
	dgmEffOnline = 4,
} EffectCategories;







#pragma pack(1)

struct PacketHeader {
	uint32 length;
	uint8 type;
};

#pragma pack()
/*
 *0x10 = "DBTYPE_I1"
0x11 = "DBTYPE_UI1"
0x0B = "DBTYPE_BOOL"
0x02 = "DBTYPE_I2"
0x12 = "DBTYPE_UI2"
0x03 = "DBTYPE_I4"
0x13 = "DBTYPE_UI4"
0x04 = "DBTYPE_R4"
0x14 = "DBTYPE_I8"
0x05 = "DBTYPE_R8"
0x15 = "DBTYPE_UI8"
0x06 = "DBTYPE_CY"
0x40 = "DBTYPE_FILETIME"
0x80 = "DBTYPE_BYTES"
0x81 = "DBTYPE_STR"
0x82 = "DBTYPE_WSTR"
*/

typedef enum {
	DBTYPE_I1 = 0x10,
	DBTYPE_UI1 = 0x11,
	DBTYPE_BOOL = 0x0B,
	DBTYPE_I2 = 0x02,
	DBTYPE_UI2 = 0x12,
	DBTYPE_I4 = 0x03,
	DBTYPE_UI4 = 0x13,
	DBTYPE_R4 = 0x04,
	DBTYPE_I8 = 0x14,
	DBTYPE_R8 = 0x05,
	DBTYPE_UI8 = 0x15,
	DBTYPE_CY = 0x06,	//money
	DBTYPE_FILETIME = 0x40,	//64
	DBTYPE_BYTES = 0x80,
	DBTYPE_STR = 0x81,
	DBTYPE_WSTR = 0x82
} DBTYPE;


/*
 *
ENV_IDX_SELF = 0
ENV_IDX_CHAR = 1
ENV_IDX_SHIP = 2
ENV_IDX_TARGET = 3
ENV_IDX_OTHER = 4
ENV_IDX_AREA = 5
ENV_IDX_EFFECT = 6
 *
 *
 *
 *
*/

/*
 service.ROLE_CHTADMINISTRATOR | service.ROLE_GMH
CHTMODE_CREATOR = (((8 + 4) + 2) + 1)
CHTMODE_OPERATOR = ((4 + 2) + 1)
CHTMODE_CONVERSATIONALIST = (2 + 1)
CHTMODE_SPEAKER = 2
CHTMODE_LISTENER = 1
CHTMODE_NOTSPECIFIED = -1
CHTMODE_DISALLOWED = -2
CHTERR_NOSUCHCHANNEL = -3
CHTERR_ACCESSDENIED = -4
CHTERR_INCORRECTPASSWORD = -5
CHTERR_ALREADYEXISTS = -6
CHTERR_TOOMANYCHANNELS = -7
CHT_MAX_USERS_PER_IMMEDIATE_CHANNEL = 50

CHANNEL_CUSTOM = 0
CHANNEL_GANG = 3


*/


/*
 *from sys/cfg.py
 *




def IsNPC(ownerID):
    return ((ownerID < 100000000) and (ownerID > 10000))



def IsSystemOrNPC(ownerID):
    return (ownerID < 100000000)



def IsFaction(ownerID):
    if ((ownerID >= 500000) and (ownerID < 1000000)):
        return 1
    else:
        return 0



def IsCorporation(ownerID):
    if ((ownerID >= 1000000) and (ownerID < 2000000)):
        return 1
    elif (ownerID < 100000000):
        return 0
    elif ((boot.role == 'server') and sm.StartService('standing2').IsKnownToBeAPlayerCorp(ownerID)):
        return 1
    else:
        return cfg.eveowners.Get(ownerID).IsCorporation()



def IsCharacter(ownerID):
    if ((ownerID >= 3000000) and (ownerID < 4000000)):
        return 1
    elif (ownerID < 100000000):
        return 0
    elif ((boot.role == 'server') and sm.StartService('standing2').IsKnownToBeAPlayerCorp(ownerID)):
        return 0
    else:
        return cfg.eveowners.Get(ownerID).IsCharacter()



def IsOwner(ownerID, fetch = 1):
    if (((ownerID >= 500000) and (ownerID < 1000000)) or (((ownerID >= 1000000) and (ownerID < 2000000)) or ((ownerID >= 3000000) and (ownerID < 4000000)))):
        return 1
    if IsNPC(ownerID):
        return 0
    if fetch:
        oi = cfg.eveowners.Get(ownerID)
        if (oi.groupID in (const.groupCharacter,
         const.groupCorporation)):
            return 1
        else:
            return 0
    else:
        return 0



def IsAlliance(ownerID):
    if (ownerID < 100000000):
        return 0
    elif ((boot.role == 'server') and sm.StartService('standing2').IsKnownToBeAPlayerCorp(ownerID)):
        return 0
    else:
        return cfg.eveowners.Get(ownerID).IsAlliance()




def IsJunkLocation(locationID):
    if (locationID >= 2000):
        return 0
    elif (locationID in [6,
     8,
     10,
     23,
     25]):
        return 1
    elif ((locationID > 1000) and (locationID < 2000)):
        return 1
    else:
        return 0


*/

#define IsAgent(itemID) \
	((itemID >= 3008416) && (itemID < 3020000))

#define IsStaticMapItem(itemID) \
	((itemID >= 10000000) && (itemID < 64000000))

#define IsRegion(itemID) \
    ((itemID >= 10000000) && (itemID < 20000000))

#define IsConstellation(itemID) \
    ((itemID >= 20000000) && (itemID < 30000000))

#define IsSolarSystem(itemID) \
    ((itemID >= 30000000) && (itemID < 40000000))

#define IsUniverseCelestial(itemID) \
    ((itemID >= 40000000) && (itemID < 50000000))

#define IsStargate(itemID) \
    ((itemID >= 50000000) && (itemID < 60000000))

#define IsStation(itemID) \
    ((itemID >= 60000000) && (itemID < 64000000))

#define IsTrading(itemID) \
    ((itemID >= 64000000) && (itemID < 66000000))

#define IsOfficeFolder(itemID) \
    ((itemID >= 66000000) && (itemID < 68000000))

#define IsFactoryFolder(itemID) \
    ((itemID >= 68000000) && (itemID < 70000000))

#define IsUniverseAsteroid(itemID) \
    ((itemID >= 70000000) && (itemID < 80000000))

#define IsScenarioItem(itemID) \
    ((itemID >= 90000000) && (itemID < 100000000))



#endif

