/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:     Captnoord
*/

#include "eve-common.h"

#include "marshal/EVEMarshalStringTable.h"

/* we made up this list so we have efficient string communication with the client */
/**  @note
 *      this list should only be updated by running the following script within client context
 *      (thanks comet0)
 * 
 * --
 * import blue
 * values = sorted(blue.marshal.stringTable, key=blue.marshal.stringTable.get)
 * for i in values:
 *      print '    "%s", ' % (i)
 * --
 */

const char* const MarshalStringTable::s_mStringTable[] =
{
    "*corpid",
    "*locationid",
    "age",
    "Asteroid",
    "authentication",
    "ballID",
    "beyonce",
    "bloodlineID",
    "capacity",
    "categoryID",
    "character",
    "characterID",
    "characterName",
    "characterType",
    "charID",
    "chatx",
    "clientID",
    "config",
    "contraband",
    "corporationDateTime",
    "corporationID",
    "createDateTime",
    "customInfo",
    "description",
    "divisionID",
    "DoDestinyUpdate",
    "dogmaIM",
    "EVE System",
    "flag",
    "foo.SlimItem",
    "gangID",
    "Gemini",
    "gender",
    "graphicID",
    "groupID",
    "header",
    "idName",
    "invbroker",
    "itemID",
    "items",
    "jumps",
    "line",
    "lines",
    "locationID",
    "locationName",
    "macho.CallReq",
    "macho.CallRsp",
    "macho.MachoAddress",
    "macho.Notification",
    "macho.SessionChangeNotification",
    "modules",
    "name",
    "objectCaching",
    "objectCaching.CachedObject",
    "OnChatJoin",
    "OnChatLeave",
    "OnChatSpeak",
    "OnGodmaShipEffect",
    "OnItemChange",
    "OnModuleAttributeChange",
    "OnMultiEvent",
    "orbitID",
    "ownerID",
    "ownerName",
    "quantity",
    "raceID",
    "RowClass",
    "securityStatus",
    "Sentry Gun",
    "sessionchange",
    "singleton",
    "skillEffect",
    "squadronID",
    "typeID",
    "used",
    "userID",
    "util.CachedObject",
    "util.IndexRowset",
    "util.Moniker",
    "util.Row",
    "util.Rowset",
    "*multicastID",
    "AddBalls",
    "AttackHit3",
    "AttackHit3R",
    "AttackHit4R",
    "DoDestinyUpdates",
    "GetLocationsEx",
    "InvalidateCachedObjects",
    "JoinChannel",
    "LSC",
    "LaunchMissile",
    "LeaveChannel",
    "OID+",
    "OID-",
    "OnAggressionChange",
    "OnCharGangChange",
    "OnCharNoLongerInStation",
    "OnCharNowInStation",
    "OnDamageMessage",
    "OnDamageStateChange",
    "OnEffectHit",
    "OnGangDamageStateChange",
    "OnLSC",
    "OnSpecialFX",
    "OnTarget",
    "RemoveBalls",
    "SendMessage",
    "SetMaxSpeed",
    "SetSpeedFraction",
    "TerminalExplosion",
    "address",
    "alert",
    "allianceID",
    "allianceid",
    "bid",
    "bookmark",
    "bounty",
    "channel",
    "charid",
    "constellationid",
    "corpID",
    "corpid",
    "corprole",
    "damage",
    "duration",
    "effects.Laser",
    "gangid",
    "gangrole",
    "hqID",
    "issued",
    "jit",
    "languageID",
    "locationid",
    "machoVersion",
    "marketProxy",
    "minVolume",
    "orderID",
    "price",
    "range",
    "regionID",
    "regionid",
    "role",
    "rolesAtAll",
    "rolesAtBase",
    "rolesAtHQ",
    "rolesAtOther",
    "shipid",
    "sn",
    "solarSystemID",
    "solarsystemid",
    "solarsystemid2",
    "source",
    "splash",
    "stationID",
    "stationid",
    "target",
    "userType",
    "userid",
    "volEntered",
    "volRemaining",
    "weapon",
    "agent.missionTemplatizedContent_BasicKillMission",
    "agent.missionTemplatizedContent_ResearchKillMission",
    "agent.missionTemplatizedContent_StorylineKillMission",
    "agent.missionTemplatizedContent_GenericStorylineKillMission",
    "agent.missionTemplatizedContent_BasicCourierMission",
    "agent.missionTemplatizedContent_ResearchCourierMission",
    "agent.missionTemplatizedContent_StorylineCourierMission",
    "agent.missionTemplatizedContent_GenericStorylineCourierMission",
    "agent.missionTemplatizedContent_BasicTradeMission",
    "agent.missionTemplatizedContent_ResearchTradeMission",
    "agent.missionTemplatizedContent_StorylineTradeMission",
    "agent.missionTemplatizedContent_GenericStorylineTradeMission",
    "agent.offerTemplatizedContent_BasicExchangeOffer",
    "agent.offerTemplatizedContent_BasicExchangeOffer_ContrabandDemand",
    "agent.offerTemplatizedContent_BasicExchangeOffer_Crafting",
    "agent.LoyaltyPoints",
    "agent.ResearchPoints",
    "agent.Credits",
    "agent.Item",
    "agent.Entity",
    "agent.Objective",
    "agent.FetchObjective",
    "agent.EncounterObjective",
    "agent.DungeonObjective",
    "agent.TransportObjective",
    "agent.Reward",
    "agent.TimeBonusReward",
    "agent.MissionReferral",
    "agent.Location",
    "agent.StandardMissionDetails",
    "agent.OfferDetails",
    "agent.ResearchMissionDetails",
    "agent.StorylineMissionDetails",
};

/*rawstrings = "\n'*fleetid'\n'*multicastID'\n'*solarsystemid&corpid'\n'*stationid&corpid'\n'*stationid&corpid&corprole'
 * \n'+clientID'\n'1 minute'\n'AccessControl'\n'CacheOK'\n'DoDestinyUpdate'\n'EN'\n'EVE-EVE-RELEASE'\n'GetCachableObject'
 * \n'GetExpiryDate'\n'GetLocationsEx'\n'GetNewPriceHistory'\n'GetOldPriceHistory'\n'GetOrders'
 * \n'HANDSHAKE_INCOMPATIBLEREGION'\n'InvalidateCachedObjects'\n'JoinChannel'\n'LSC'\n'LeaveChannel'\n'MarketRegion'
 * \n'Method Call'\n'OID-'\n'OnAccountChange'\n'OnAgentMissionChange'\n'OnAggressionChange'\n'OnCfgDataChanged'
 * \n'OnCharNoLongerInStation'\n'OnCharNowInStation'\n'OnContactLoggedOn'\n'OnCorporationMemberChanged'
 * \n'OnCorporationVoteCaseOptionChanged'\n'OnDockingAccepted'\n'OnFleetActive'\n'OnFleetJoin'\n'OnFleetLeave'
 * \n'OnFleetMemberChanged'\n'OnFleetStateChange'\n'OnItemsChanged'\n'OnJumpQueueUpdate'\n'OnLSC'
 * \n'OnMachoObjectDisconnect'\n'OnMessage'\n'OnMultiEvent'\n'OnNotifyPreload'\n'OnOwnOrderChanged'
 * \n'OnProbeStateChanged'\n'OnProbeWarpEnd'\n'OnProbeWarpStart'\n'OnRemoteMessage'\n'OnSquadActive'
 * \n'OnStandingsModified'\n'OnSystemScanStarted'\n'OnSystemScanStopped'\n'OnTarget'\n'PlaceCharOrder'
 * \n'ProcessGodmaPrimeLocation'\n'SendMessage'\n'__MultiEvent'\n'account'\n'address'\n'allianceid'
 * \n'authentication'\n'autodepth_stencilformat'\n'avgPrice'\n'baseID'\n'beyonce'\n'bid'\n'bloodlineID'\n'boot_build'
 * \n'boot_codename'\n'boot_region'\n'boot_version'\n'build'\n'ccp'\n'challenge_responsehash'\n'channel'\n'character'
 * \n'charid'\n'charmgr'\n'clientID'\n'clock'\n'codename'\n'columns'\n'compressedPart'\n'config'\n'config.Attributes'
 * \n'config.Bloodlines'\n'config.BulkData.allianceshortnames'\n'config.BulkData.billtypes'\n'config.BulkData.bptypes'
 * \n'config.BulkData.categories'\n'config.BulkData.certificaterelationships'\n'config.BulkData.certificates'
 * \n'config.BulkData.dgmattribs'\n'config.BulkData.dgmeffects'\n'config.BulkData.dgmtypeattribs'
 * \n'config.BulkData.dgmtypeeffects'\n'config.BulkData.graphics'\n'config.BulkData.groups'
 * \n'config.BulkData.invmetatypes'\n'config.BulkData.invtypereactions'\n'config.BulkData.locations'
 * \n'config.BulkData.locationwormholeclasses'\n'config.BulkData.mapcelestialdescriptions'\n'config.BulkData.metagroups'
 * \n'config.BulkData.owners'\n'config.BulkData.ramactivities'\n'config.BulkData.ramaltypes'
 * \n'config.BulkData.ramaltypesdetailpercategory'\n'config.BulkData.ramaltypesdetailpergroup'
 * \n'config.BulkData.ramcompletedstatuses'\n'config.BulkData.invtypematerials'\n'config.BulkData.ramtyperequirements'
 * \n'config.BulkData.shiptypes'\n'config.BulkData.tickernames'\n'config.BulkData.types'\n'config.BulkData.units'
 * \n'config.Flags'\n'config.InvContrabandTypes'\n'config.Races'\n'config.StaticLocations'\n'config.StaticOwners'
 * \n'config.Units'\n'constellationid'\n'contractMgr'\n'corpAccountKey'\n'corpRegistry'\n'corpStationMgr'\n'corpid'
 * \n'corporationSvc'\n'corprole'\n'crypting_securityprovidertype'\n'crypting_sessionkeylength'
 * \n'crypting_sessionkeymethod'\n'dogmaIM'\n'duration'\n'entity'\n'explosioneffectssenabled'\n'facWarMgr'\n'fittingMgr'
 * \n'fullscreen_resolution'\n'fleetSvc'\n'fleetObjectHandler'\n'fleetbooster'\n'fleetid'\n'fleetrole'\n'genderID'
 * \n'handshakefunc_output'\n'handshakefunc_result'\n'header'\n'highPrice'\n'historyDate'\n'hqID'\n'inDetention'
 * \n'invbroker'\n'issued'\n'jit'\n'jumps'\n'languageID'\n'locationid'\n'loggedOnUserCount'\n'lowPrice'\n'macho.CallReq'
 * \n'macho.CallRsp'\n'macho.ErrorResponse'\n'macho.MachoAddress'\n'macho.Notification'\n'macho.PingReq'\n'macho.PingRsp'
 * \n'macho.SessionChangeNotification'\n'macho.SessionInitialStateNotification'\n'machoNet'\n'machoNet.serviceInfo'
 * \n'machoVersion'\n'macho_version'\n'map'\n'market'\n'marketProxy'\n'maxSessionTime'\n'minVolume'
 * \n'network_computername'\n'objectCaching'\n'objectCaching.CachedMethodCallResult'\n'objectCaching.CachedObject'
 * \n'orderID'\n'orders'\n'origin'\n'pixel_shader_version'\n'presentation_interval'\n'price'\n'processor_architecture'
 * \n'processor_identifier'\n'raceID'\n'range'\n'reason'\n'reasonArgs'\n'reasonCode'\n'region'\n'regionID'\n'regionid'
 * \n'role'\n'rolesAtAll'\n'rolesAtBase'\n'rolesAtHQ'\n'rolesAtOther'\n'scanMgr'\n'sessionInfo'\n'sessionMgr'
 * \n'sessionchange'\n'shipid'\n'sn'\n'solarSystemID'\n'solarsystemid'\n'solarsystemid2'\n'squadid'\n'standing2'
 * \n'station'\n'stationID'\n'stationSvc'\n'stationid'\n'tutorialSvc'\n'typeID'\n'userType'\n'user_logonqueueposition'
 * \n'userid'\n'utcmidnight'\n'utcmidnight_or_3hours'\n'util.CachedObject'\n'util.KeyVal'\n'util.Rowset'\n'version'
 * \n'versionCheck'\n'vertex_shader_version'\n'volEntered'\n'volRemaining'\n'volume'\n'warRegistry'\n'warfactionid'
 * \n'wingid'\n"
 */

const size_t MarshalStringTable::s_mStringTableSize = sizeof( MarshalStringTable::s_mStringTable ) / sizeof( const char* );

MarshalStringTable::MarshalStringTable()
{
    for( uint8 i = 1; i <= s_mStringTableSize; i++ )
        mStringTableMap.insert( std::make_pair( hash( LookupString( i ) ), i ) );
}

/* lookup a index using a string */
uint8 MarshalStringTable::LookupIndex( const std::string& str )
{
    return LookupIndex( str.c_str() );
}

/* lookup a index using a string */
uint8 MarshalStringTable::LookupIndex( const char* str )
{
    StringTableMapConstItr res = mStringTableMap.find( hash( str ) );
    if( mStringTableMap.end() == res )
        return STRING_TABLE_ERROR;

    return res->second;
}

const char* MarshalStringTable::LookupString( uint8 index )
{
    if( --index < s_mStringTableSize )
        return s_mStringTable[ index ];
    else
        return nullptr;
}
