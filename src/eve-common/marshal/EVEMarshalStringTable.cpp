/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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

#include "EVECommonPCH.h"

#include "marshal/EVEMarshalStringTable.h"

/* we made up this list so we have efficient string communication with the client */
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
const size_t MarshalStringTable::s_mStringTableSize = sizeof( MarshalStringTable::s_mStringTable ) / sizeof( const char* );

MarshalStringTable::MarshalStringTable()
{
    for( uint8 i = 1; i <= s_mStringTableSize; i++ )
        mStringTableMap.insert(
            std::make_pair( hash( LookupString( i ) ), i )
        );
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
        return NULL;
}
