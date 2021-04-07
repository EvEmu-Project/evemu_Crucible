/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2008 The EVEmu Team
    For the latest information visit http://evemu.mmoforge.org
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
    Author:     Zhur, Bloody.Rabit
    Updates:    Allan
    Version:    10.2
*/

#ifndef __EVE_SERVER_CONFIG__H__INCL__
#define __EVE_SERVER_CONFIG__H__INCL__

#include "eve-server.h"


/**
 * @brief Class which parses and stores eve-server configuration.
 *
 * @author Zhur, Bloody.Rabbit
 */
class EVEServerConfig
: public XMLParserEx,
  public Singleton< EVEServerConfig >
{
public:
    EVEServerConfig();
    ~EVEServerConfig()                                  { /* do nothing here */}

    // From <server>
    struct {
        bool TraderJoe;
        bool DisableIGB;
        bool BulkDataOD;
        bool NoobShipCheck;
        bool ModuleAutoOff;
        bool UnloadOnLinkAll;
        bool AllowNonPublished;
        bool FleetShareDelayed;
        bool BountyPayoutDelayed;
        bool LoadOldMissions;
        bool AsteroidsOnDScan;
        bool CargoMassAdditive;
        uint8 ServerSleepTime;
        uint8 MaxThreadReport;
        uint8 BountyPayoutTimer;
        uint16 idleSleepTime;
        uint16 maxPlayers;
        float processTic;
        float ModuleDamageChance;
    } server;

    // From <world>
    struct {
        bool chatLogs;
        bool globalChat;
        bool gridUnload;
        bool loginInfo;
        bool loginMsg;
        bool saveOnMove;
        bool saveOnUpdate;
        uint8 mailDelay;
        uint8 StationDockDelay;
        uint16 shipBoardDistance;
        uint16 gridUnloadTime;
        uint16 apWarptoDistance;
    } world;

    // From <rates>
    struct {
        // Decay timer for item deletion (garbage collection)
        uint8 WorldDecay;
        uint8 WebUpdate;
        // min amount of tax accepted by corp.  an amount less than this will not be processed, resulting in no tax for the concerned payment.
        uint32 TaxAmount;
        // min amount to be taxed.  received amounts less than this will not be taxed.
        uint32 TaxedAmount;
        /// Modifier for npc bounties automatically awarded for shooting down npc enemies.
        float npcBountyMultiply;
        /// Modifier for damage from NPCs
        float damageRate;
        /// Modifier for damage from missiles
        float missileDamage;
        /// Modifier for missile flightTime
        float missileTime;
        /// Modifier for missile rate of fire
        float missileRoF;
        /// Modifier for damage from PC turrets
        float turretDamage;
        /// Modifier for turret rate of fire
        float turretRoF;
        // Decay timer for wreck deletion (garbage collection)
        float NPCDecay;
        uint8 DropItem;
        float DropMoney;
        uint8 DropSalvage;
        float RepairCost;
        float ShipRepairModifier;
        float ModuleRepairModifier;
        /// Modifier for security rating changes. Changes how fast it goes up/down based on actions
        double secRate;
        /// Startup Cost to create a corporation.
        uint32 corpCost;
        /// Startup Cost to create an alliance.
        uint32 allyCost;
        uint32 medalAwardCost;
        uint32 medalCreateCost;
        // Crit chance
        float PlayerCritChance;
        float NpcCritChance;
        float SentryCritChance;
        float DroneCritChance;
        float ConcordCritChance;
    } rates;

    // from <market>
    struct {
        bool UseOrderRange;
        bool DeleteOldTransactions;
        uint8 StationOrderLimit;
        uint8 SystemOrderLimit;
        uint8 RegionOrderLimit;
        uint8 FindBuyOrder;
        uint8 FindSellOrder;
        uint8 OldPriceLimit;
        uint8 NewPriceLimit;
        uint8 HistoryUpdateTime;
    } market;

    // From <ram>
    struct {
        bool AutoEvent;
        float ProdTime;
        float CopyTime;
        float ReTime;
        float MatMod;
        float WasteMod;
        float ResPE;
        float ResME;
        float InventTime;
    } ram;

    // From <account>
    struct {
        /// Role to assign to auto created account; set to 0 to disable auto account creation.
        int64 autoAccountRole;
        /// A message shown to every client on login (if enabled in <World><LoginMsg>).
        std::string loginMessage;
    } account;

    // From <character>
    struct {
        bool allow3edChar;
        uint8 statMultiplier;
        /// Starting corp ID for new characters
        uint32 startCorporation;
        /// Delay for terminating a character in seconds
        uint32 terminationDelay;
        /// Starting station ID for new characters
        uint32 startStation;
        /// Money balance of new created characters.
        double startBalance;
        /// Aura balance of new created characters.   -allan 01/10/14
        double startAurBalance;
        /// Starting security rating for new characters.
        double startSecRating;
    } character;

    // From <NPC>
    struct {
        bool IdleWander;
        bool UseDamageMultiplier;
        bool RoamingSpawns;
        bool StaticSpawns;
        bool TargetPod;
        uint16 WarpOut;
        uint16 RoamingTimer;
        uint16 StaticTimer;
        uint16 RespawnTimer;
        uint32 RatFaction;
        float ThreatRadius;
        float WarpFollowChance;
        float TargetPodSec;
        float DefenderMissileChance;
        float LootDropChance;
    } npc;

    // From <database>
    struct {
        bool compress;
        bool ssl;
        bool useSocket;
        bool autoReconnect;
        uint dbTimeout;
        uint8 pingTime;
        /// A port at which the database server listens.
        uint16 port;
        /// Hostname of database server.
        std::string host;
        /// Name of database account to use.
        std::string username;
        /// Password for the database account.
        std::string password;
        /// A database to be used by server.
        std::string db;
    } database;

    // From <files>
    struct {
        /// A directory in which the log files are stored
        std::string logDir;
        /// A log configuration file.
        std::string logSettings;
        /// A directory at which the cache files should be stored.
        std::string cacheDir;
        // used as the base directory for the image server
        std::string imageDir;
    } files;

    // From <net>
    struct {
        /// Port at which the server should listen.
        uint16 port;
        /// Port at which the imageServer should listen.
        uint16 imageServerPort;
        /// the imageServer for char images. should be the evemu server external ip/host
        std::string imageServer;
    } net;

    // From <thread>
    struct {
        uint8 NetworkThreads;
        uint8 DatabaseThreads;
        uint8 WorldThreads;
        uint8 ImageServerThreads;
        uint8 ConsoleThreads;
    } threads;

    // From <cosmic>
    struct {
        bool PIEnabled;
        bool AnomalyEnabled;
        bool DungeonEnabled;
        bool BeltEnabled;
        bool WormHoleEnabled;
        bool CiviliansEnabled;
        bool BumpEnabled;
        uint8 BeltRespawn;
        uint8 BeltGrowth;
        float roidRadiusMultiplier;
    } cosmic;

    // From exploring
    struct {
        int8 Gravametric;
        int8 Magnetometric;
        int8 Ladar;
        int8 Radar;
        int8 Unrated;
        int8 Complex;
    } exploring;

    // From <standings>
    struct {
        float MissionBonus;
        float MissionFailure;
        float MissionDeclined;
        float MissionCompleted;
        float MissionOfferExpired;
        float ImportantMissionBonus;
        float MissionFailedRollback;
        float BaseMissionMultiplier;
        float FleetMissionMultiplier;
        float Agent2CharMissionMultiplier;
        float ACorp2CharMissionMultiplier;
        float Agent2PCorpMissionMultiplier;
        float ACorp2PCorpMissionMultiplier;
        float AFaction2CharMissionMultiplier;
        float AFaction2PCorpMissionMultiplier;
    } standings;

    // From <chat>
    struct {
        bool EnableFleetChat;
        bool EnableWingChat;
        bool EnableSquadChat;
        bool EnableVoiceChat;
        bool EnforceRookieInHelp;
    } chat;

    // From <crime>
    struct {
        bool Enabled;
        uint8 CWSessionTime;
        uint8 WeaponFlagTime;
        uint16 KillRightTime;
        uint16 AggFlagTime;
        uint16 CrimFlagTime;
    } crime;

    // From <testing>
    struct {
        bool ShipHeat;
        bool EnableDrones;
    } testing;

    // From <debug>
    struct {
        bool BeanCount;
        bool StackTrace;
        bool BubbleTrack;
        bool SpawnTest;
        bool IsTestServer;    // to distinguish between live production server or experimental testing server
        bool UseProfiling;
        bool UseShipTracking;
        bool DeleteTrackingCans;
        bool PositionHack;
        uint16 ProfileTraceTime;
        uint32 AnomalyFaction;
    } debug;

protected:
    bool ProcessEveServer( const TiXmlElement* ele );
    bool ProcessServer( const TiXmlElement* ele );
    bool ProcessWorld( const TiXmlElement* ele );
    bool ProcessRates( const TiXmlElement* ele );
    bool ProcessMarket( const TiXmlElement* ele );
    bool ProcessAccount( const TiXmlElement* ele );
    bool ProcessCharacter( const TiXmlElement* ele );
    bool ProcessNPC( const TiXmlElement* ele );
    bool ProcessDatabase( const TiXmlElement* ele );
    bool ProcessFiles( const TiXmlElement* ele );
    bool ProcessNet( const TiXmlElement* ele );
    bool ProcessThreads( const TiXmlElement* ele );
    bool ProcessCosmic( const TiXmlElement* ele );
    bool ProcessExploring( const TiXmlElement* ele );
    bool ProcessStandings( const TiXmlElement* ele );
    bool ProcessChat( const TiXmlElement* ele );
    bool ProcessCrime( const TiXmlElement* ele );
    bool ProcessBPTimes( const TiXmlElement* ele );
    bool ProcessTesting( const TiXmlElement* ele );
    bool ProcessDebug( const TiXmlElement* ele );
};

/// A macro for easier access to the singleton.
#define sConfig \
    ( EVEServerConfig::get() )

#endif /* !__EVE_SERVER_CONFIG__H__INCL__ */
