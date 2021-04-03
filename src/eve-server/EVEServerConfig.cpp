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
    Author:     Zhur, Bloody.Rabbit
    Updates:    Allan
    Version:    10.2
*/


#include "EVEServerConfig.h"

/*************************************************************************/
/* EVEServerConfig                                                       */
/*************************************************************************/
EVEServerConfig::EVEServerConfig()
{
    // register needed parsers
    AddMemberParser( "eve-server", &EVEServerConfig::ProcessEveServer );

    // Set sane defaults

    // items with a "N" behind them are NOT implemented
    // items with a "P" behind them are PARTIALLY implemented

    // items with /*x*/ behind them denote time identifier, with x = (s=seconds, m=minutes, etc)

    // server
    server.TraderJoe = false;//N
    server.maxPlayers = 500;//N
    server.BulkDataOD = false;
    server.NoobShipCheck = true;
    server.ServerSleepTime = 10 /*ms*/;
    server.idleSleepTime = 1000;
    server.DisableIGB = true;
    server.MaxThreadReport = 20;
    server.ModuleAutoOff = false;
    server.ModuleDamageChance = 0.35;
    server.UnloadOnLinkAll = false;
    server.processTic = 1.0;
    server.AllowNonPublished = false;
    server.FleetShareDelayed = false;
    server.BountyPayoutDelayed = false; // this system isnt working yet.  disable by default
    server.BountyPayoutTimer = 20/*m*/;
    server.LoadOldMissions = false;
    server.AsteroidsOnDScan = false;
    server.CargoMassAdditive = false;

    // world
    world.chatLogs = false;//N
    world.globalChat = true;//N
    world.gridUnload = true;
    world.gridUnloadTime = 300 /*s*/; // 5 mins
    world.loginInfo = false;//N
    world.loginMsg = false;//N
    world.saveOnMove = false;
    world.mailDelay = 5;//N
    world.StationDockDelay = 4 /*s*/;
    world.apWarptoDistance = 15000;
    world.shipBoardDistance = 300;

    // rates
    rates.npcBountyMultiply = 1.0;
    rates.secRate = 1.0;
    rates.damageRate = 1.0;
    rates.missileDamage = 1.0;
    rates.missileTime = 1.0;
    rates.missileRoF = 1.0;
    rates.turretDamage = 1.0;
    rates.turretRoF = 1.0;
    rates.corpCost = 1599800;
    rates.medalAwardCost = 5000000;
    rates.medalCreateCost = 5000000;
    rates.WorldDecay = 120 /*m*/;
    rates.NPCDecay = 90 /*m*/;
    rates.DropItem = 1;//N
    rates.DropMoney = 1.0;//N
    rates.DropSalvage = 1;//P
    rates.RepairCost = 1.0;
    rates.ShipRepairModifier = 0.0000075;
    rates.ModuleRepairModifier = 0.00125;
    rates.WebUpdate = 15 /*m*/;
    rates.TaxAmount = 5000;
    rates.TaxedAmount = 75000;
    rates.PlayerCritChance = 0.02;
    rates.NpcCritChance = 0.15;
    rates.SentryCritChance = 0.02;
    rates.DroneCritChance = 0.03;
    rates.ConcordCritChance = 0.05;

    //market
    market.FindBuyOrder = 10;
    market.FindSellOrder = 10;
    market.StationOrderLimit = 10;
    market.SystemOrderLimit = 10;
    market.RegionOrderLimit = 10;
    market.OldPriceLimit = 10;
    market.NewPriceLimit = 10;
    market.HistoryUpdateTime = 6/*h*/;
    market.UseOrderRange = true;//N
    market.DeleteOldTransactions = false;

    // ram
    ram.AutoEvent = false;
    ram.ResME = 1.0;
    ram.ResPE = 1.0;
    ram.MatMod = 1.0;
    ram.WasteMod = 1.0;
    ram.ReTime = 1.0;
    ram.ProdTime = 1.0;
    ram.CopyTime = 1.0;
    ram.InventTime = 1.0;

    // account
    account.autoAccountRole = Acct::Role::STD;
    account.loginMessage = "";

    // character
    character.startBalance = 6666000000.0f;
    character.startAurBalance = 60000.0f;
    character.startStation = 0;
    character.startSecRating = 0.0;
    character.startCorporation = 0;
    character.terminationDelay = 180 /*s*/;
    character.statMultiplier = 1;
    character.allow3edChar = false;

    // npc
    npc.IdleWander = false;
    npc.WarpOut = 600 /*s*/;
    npc.WarpFollowChance = 0.15;
    npc.ThreatRadius = 1.0;//N
    npc.RoamingSpawns = false;
    npc.StaticSpawns = false;
    npc.RoamingTimer = 900 /*s*/;
    npc.StaticTimer = 600 /*s*/;
    npc.RespawnTimer = 480 /*s*/;
    npc.RatFaction = 0;
    npc.TargetPodSec = 0;
    npc.TargetPod = false;
    npc.UseDamageMultiplier = true;
    npc.DefenderMissileChance = 0.0;
    npc.LootDropChance = 0.75;

    // cosmic
    cosmic.PIEnabled = false;
    cosmic.AnomalyEnabled = false;
    cosmic.DungeonEnabled = false;
    cosmic.BeltEnabled = false;
    cosmic.BeltRespawn = 8 /*h*/;
    cosmic.BeltGrowth = 6 /*h*/;
    cosmic.roidRadiusMultiplier = 1.0;
    cosmic.WormHoleEnabled = false;
    cosmic.CiviliansEnabled = false;
    cosmic.BumpEnabled = false;

    // exploring
    exploring.Gravametric = 5;
    exploring.Magnetometric = 3;
    exploring.Ladar = 6;
    exploring.Radar = 6;
    exploring.Unrated = 8;
    exploring.Complex = 4;

    // standings
    //  - mission
    standings.MissionBonus = 1.0;
    standings.MissionFailure = -0.5;
    standings.MissionDeclined = -0.2;
    standings.MissionCompleted = 1.0;
    standings.MissionOfferExpired = -0.1;
    standings.MissionFailedRollback = -0.5;
    standings.ImportantMissionBonus = 3.0;
    standings.BaseMissionMultiplier = 1.0;
    standings.FleetMissionMultiplier = 0.5;
    standings.Agent2CharMissionMultiplier = 1.0;
    standings.ACorp2CharMissionMultiplier = 0.25;
    standings.Agent2PCorpMissionMultiplier = 0.1;
    standings.ACorp2PCorpMissionMultiplier = 0.025;
    standings.AFaction2CharMissionMultiplier = 0.125;
    standings.AFaction2PCorpMissionMultiplier = 0.0125;
    //  - PVP
    //  - ratting/exploring

    // chat
    chat.EnableFleetChat = true;
    chat.EnableWingChat = false;
    chat.EnableSquadChat = false;
    chat.EnableVoiceChat = false;
    chat.EnforceRookieInHelp = false;

    // crime
    crime.Enabled = false; //N
    crime.AggFlagTime = 900 /*s*/;//N
    crime.CrimFlagTime = 900 /*s*/;//N
    crime.CWSessionTime = 60 /*s*/;//N
    crime.KillRightTime = 900 /*s*/;//N
    crime.WeaponFlagTime = 60 /*s*/;//N

    // testing
    testing.EnableDrones = false;
    testing.ShipHeat = false;

    // debug
    debug.BeanCount = false;
    debug.StackTrace = false;
    debug.BubbleTrack = false;
    debug.IsTestServer = true;
    debug.UseProfiling = false;
    debug.PositionHack = false;
    debug.UseShipTracking = false;
    debug.DeleteTrackingCans = true;
    debug.SpawnTest = false;
    debug.AnomalyFaction = 0;
    debug.ProfileTraceTime = 150/*ms*/;

    // database
    database.host = "localhost";
    database.port = 3306;
    database.username = "eve";
    database.password = "eve";
    database.db = "evemu";
    database.compress = false;
    database.ssl = false;
    database.useSocket = false;
    database.autoReconnect = false;
    database.dbTimeout = 2/*s*/;
    database.pingTime = 10/*m*/;

    // files
    files.logDir = "../log/";
    files.logSettings = "../etc/log.ini";
    files.cacheDir = "../server_cache/";
    files.imageDir = "../image_cache/";

    // net
    net.port = 26000;
    net.imageServer = "localhost";
    net.imageServerPort = 26001;

    // threads  -not implemented
    threads.ConsoleThreads = 1;//P
    threads.DatabaseThreads = 2;//N
    threads.ImageServerThreads = 1;//N
    threads.NetworkThreads = 2;//N
    threads.WorldThreads = 2;//N

}

bool EVEServerConfig::ProcessEveServer( const TiXmlElement* ele )
{
    // entering element, extend allowed syntax
    AddMemberParser( "server",      &EVEServerConfig::ProcessServer );
    AddMemberParser( "world",       &EVEServerConfig::ProcessWorld );
    AddMemberParser( "rates",       &EVEServerConfig::ProcessRates );
    AddMemberParser( "market",      &EVEServerConfig::ProcessMarket );
    AddMemberParser( "ram",     &EVEServerConfig::ProcessBPTimes );
    AddMemberParser( "account",     &EVEServerConfig::ProcessAccount );
    AddMemberParser( "character",   &EVEServerConfig::ProcessCharacter );
    AddMemberParser( "npc",         &EVEServerConfig::ProcessNPC );
    AddMemberParser( "cosmic",      &EVEServerConfig::ProcessCosmic );
    AddMemberParser( "exploring",   &EVEServerConfig::ProcessExploring );
    AddMemberParser( "crime",       &EVEServerConfig::ProcessCrime );
    AddMemberParser( "standings",   &EVEServerConfig::ProcessStandings );
    AddMemberParser( "chat",        &EVEServerConfig::ProcessChat );
    AddMemberParser( "debug",       &EVEServerConfig::ProcessDebug );
    AddMemberParser( "database",    &EVEServerConfig::ProcessDatabase );
    AddMemberParser( "files",       &EVEServerConfig::ProcessFiles );
    AddMemberParser( "net",         &EVEServerConfig::ProcessNet );
    AddMemberParser( "testing",     &EVEServerConfig::ProcessTesting );
    AddMemberParser( "threads",     &EVEServerConfig::ProcessThreads );

    // parse the element
    const bool result = ParseElementChildren( ele );

    // leaving element, reduce allowed syntax
    RemoveParser( "server" );
    RemoveParser( "world" );
    RemoveParser( "rates" );
    RemoveParser( "market" );
    RemoveParser( "ram" );
    RemoveParser( "account" );
    RemoveParser( "character" );
    RemoveParser( "npc" );
    RemoveParser( "cosmic" );
    RemoveParser( "exploring" );
    RemoveParser( "crime" );
    RemoveParser( "standings" );
    RemoveParser( "chat" );
    RemoveParser( "debug" );
    RemoveParser( "database" );
    RemoveParser( "files" );
    RemoveParser( "net" );
    RemoveParser( "testing" );
    RemoveParser( "threads" );

    // return status of parsing
    return result;
}

bool EVEServerConfig::ProcessServer( const TiXmlElement* ele )
{
    AddValueParser( "DisableIGB",           server.DisableIGB );
    AddValueParser( "TraderJoe",            server.TraderJoe );
    AddValueParser( "maxPlayers",           server.maxPlayers );
    AddValueParser( "NoobShipCheck",        server.NoobShipCheck );
    AddValueParser( "BulkDataOD",           server.BulkDataOD );
    AddValueParser( "ServerSleepTime",      server.ServerSleepTime );
    AddValueParser( "idleSleepTime",        server.idleSleepTime );
    AddValueParser( "MaxThreadReport",      server.MaxThreadReport );
    AddValueParser( "ModuleAutoOff",        server.ModuleAutoOff );
    AddValueParser( "ModuleDamageChance",   server.ModuleDamageChance );
    AddValueParser( "UnloadOnLinkAll",      server.UnloadOnLinkAll );
    AddValueParser( "processTic",           server.processTic );
    AddValueParser( "AllowNonPublished",    server.AllowNonPublished );
    AddValueParser( "FleetShareDelayed",    server.FleetShareDelayed );
    AddValueParser( "BountyPayoutDelayed",  server.BountyPayoutDelayed );
    AddValueParser( "BountyPayoutTimer",    server.BountyPayoutTimer );
    AddValueParser( "LoadOldMissions",      server.LoadOldMissions );
    AddValueParser( "AsteroidsOnDScan",     server.AsteroidsOnDScan );
    AddValueParser( "CargoMassAdditive",    server.CargoMassAdditive );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "DisableIGB" );
    RemoveParser( "TraderJoe" );
    RemoveParser( "maxPlayers" );
    RemoveParser( "NoobShipCheck" );
    RemoveParser( "BulkDataOD" );
    RemoveParser( "ServerSleepTime" );
    RemoveParser( "idleSleepTime" );
    RemoveParser( "MaxThreadReport" );
    RemoveParser( "ModuleAutoOff" );
    RemoveParser( "ModuleDamageChance" );
    RemoveParser( "processTic" );
    RemoveParser( "AllowNonPublished" );
    RemoveParser( "FleetShareDelayed" );
    RemoveParser( "BountyPayoutDelayed" );
    RemoveParser( "BountyPayoutTimer" );
    RemoveParser( "LoadOldMissions" );
    RemoveParser( "AsteroidsOnDScan" );
    RemoveParser( "CargoMassAdditive" );

    return result;
}

bool EVEServerConfig::ProcessWorld( const TiXmlElement* ele )
{
    AddValueParser( "chatLogs",          world.chatLogs );
    AddValueParser( "globalChat",        world.globalChat );
    AddValueParser( "gridUnload",        world.gridUnload );
    AddValueParser( "gridUnloadTime",    world.gridUnloadTime );
    AddValueParser( "loginInfo",         world.loginInfo );
    AddValueParser( "loginMsg",          world.loginMsg );
    AddValueParser( "saveOnMove",        world.saveOnMove );
    AddValueParser( "saveOnUpdate",      world.saveOnUpdate );
    AddValueParser( "mailDelay",         world.mailDelay );
    AddValueParser( "StationDockDelay",  world.StationDockDelay );
    AddValueParser( "apWarptoDistance",  world.apWarptoDistance );
    AddValueParser( "shipBoardDistance", world.shipBoardDistance );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "chatLogs" );
    RemoveParser( "globalChat" );
    RemoveParser( "gridUnload" );
    RemoveParser( "gridUnloadTime" );
    RemoveParser( "loginInfo" );
    RemoveParser( "loginMsg" );
    RemoveParser( "saveOnMove" );
    RemoveParser( "saveOnUpdate" );
    RemoveParser( "mailDelay" );
    RemoveParser( "StationDockDelay" );
    RemoveParser( "apWarptoDistance" );
    RemoveParser( "shipBoardDistance" );

    return result;
}

bool EVEServerConfig::ProcessRates( const TiXmlElement* ele )
{
    AddValueParser( "secRate",              rates.secRate );
    AddValueParser( "npcBountyMultiply",    rates.npcBountyMultiply );
    AddValueParser( "damageRate",           rates.damageRate );
    AddValueParser( "missileRoF",           rates.missileRoF );
    AddValueParser( "missileDamage",        rates.missileDamage );
    AddValueParser( "missileTime",          rates.missileTime );
    AddValueParser( "turretDamage",         rates.turretDamage );
    AddValueParser( "turretRoF",            rates.turretRoF );
    AddValueParser( "corpCost",             rates.corpCost );
    AddValueParser( "medalAwardCost",       rates.medalAwardCost );
    AddValueParser( "medalCreateCost",      rates.medalCreateCost );
    AddValueParser( "WorldDecay",           rates.WorldDecay );
    AddValueParser( "NPCDecay",             rates.NPCDecay );
    AddValueParser( "DropItem",             rates.DropItem );
    AddValueParser( "DropMoney",            rates.DropMoney );
    AddValueParser( "DropSalvage",          rates.DropSalvage );
    AddValueParser( "RepairCost",           rates.RepairCost );
    AddValueParser( "ShipRepairModifier",   rates.ShipRepairModifier );
    AddValueParser( "ModuleRepairModifier", rates.ModuleRepairModifier );
    AddValueParser( "WebUpdate",            rates.WebUpdate );
    AddValueParser( "TaxAmount",            rates.TaxAmount );
    AddValueParser( "TaxedAmount",          rates.TaxedAmount );
    AddValueParser( "PlayerCritChance",     rates.PlayerCritChance );
    AddValueParser( "NpcCritChance",        rates.NpcCritChance );
    AddValueParser( "SentryCritChance",     rates.SentryCritChance );
    AddValueParser( "DroneCritChance",      rates.DroneCritChance );
    AddValueParser( "ConcordCritChance",    rates.ConcordCritChance );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "secRate" );
    RemoveParser( "npcBountyMultiply" );
    RemoveParser( "damageRate" );
    RemoveParser( "missileDamage" );
    RemoveParser( "missileRoF" );
    RemoveParser( "missileTime" );
    RemoveParser( "turretDamage" );
    RemoveParser( "turretRoF" );
    RemoveParser( "corpCost" );
    RemoveParser( "medalAwardCost" );
    RemoveParser( "medalCreateCost" );
    RemoveParser( "WorldDecay" );
    RemoveParser( "NPCDecay" );
    RemoveParser( "DropItem" );
    RemoveParser( "DropMoney" );
    RemoveParser( "DropSalvage" );
    RemoveParser( "RepairCost" );
    RemoveParser( "ShipRepairModifier" );
    RemoveParser( "ModuleRepairModifier" );
    RemoveParser( "WebUpdate" );
    RemoveParser( "TaxAmount" );
    RemoveParser( "TaxedAmount" );
    RemoveParser( "PlayerCritChance" );
    RemoveParser( "NpcCritChance" );
    RemoveParser( "SentryCritChance" );
    RemoveParser( "DroneCritChance" );
    RemoveParser( "ConcordCritChance" );

    return result;
}

bool EVEServerConfig::ProcessMarket(const TiXmlElement* ele)
{
    AddValueParser( "StationOrderLimit",            market.StationOrderLimit );
    AddValueParser( "SystemOrderLimit",             market.SystemOrderLimit);
    AddValueParser( "RegionOrderLimit",             market.RegionOrderLimit );
    AddValueParser( "FindBuyOrder",                 market.FindBuyOrder );
    AddValueParser( "FindSellOrder",                market.FindSellOrder);
    AddValueParser( "OldPriceLimit",                market.OldPriceLimit );
    AddValueParser( "NewPriceLimit",                market.NewPriceLimit);
    AddValueParser( "UseOrderRange",                market.UseOrderRange);
    AddValueParser( "DeleteOldTransactions",        market.DeleteOldTransactions);
    AddValueParser( "HistoryUpdateTime",     market.HistoryUpdateTime);

    const bool result = ParseElementChildren( ele );

    RemoveParser( "StationOrderLimit" );
    RemoveParser( "SystemOrderLimit" );
    RemoveParser( "RegionOrderLimit" );
    RemoveParser( "FindBuyOrder" );
    RemoveParser( "FindSellOrder" );
    RemoveParser( "OldPriceLimit" );
    RemoveParser( "NewPriceLimit" );
    RemoveParser( "UseOrderRange" );
    RemoveParser( "DeleteOldTransactions" );
    RemoveParser( "HistoryUpdateTime" );

    return result;
}

bool EVEServerConfig::ProcessBPTimes(const TiXmlElement* ele)
{
    AddValueParser( "AutoEvent",        ram.AutoEvent);
    AddValueParser( "ProdTime",         ram.ProdTime);
    AddValueParser( "CopyTime",         ram.CopyTime);
    AddValueParser( "WasteMod",         ram.WasteMod);
    AddValueParser( "MatMod",           ram.MatMod);
    AddValueParser( "ResME",            ram.ResME);
    AddValueParser( "ResPE",            ram.ResPE);
    AddValueParser( "ReTime",           ram.ReTime);
    AddValueParser( "InventTime",       ram.InventTime);

    const bool result = ParseElementChildren( ele );

    RemoveParser( "AutoEvent" );
    RemoveParser( "ProdTime" );
    RemoveParser( "CopyTime" );
    RemoveParser( "WasteMod" );
    RemoveParser( "MatMod" );
    RemoveParser( "ResME" );
    RemoveParser( "ResPE" );
    RemoveParser( "ReTime" );
    RemoveParser( "InventTime" );

    return result;
}

bool EVEServerConfig::ProcessAccount( const TiXmlElement* ele )
{
    AddValueParser( "autoAccountRole",  account.autoAccountRole );
    AddValueParser( "loginMessage",     account.loginMessage );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "autoAccountRole" );
    RemoveParser( "loginMessage" );

    return result;
}

bool EVEServerConfig::ProcessCharacter( const TiXmlElement* ele )
{
    AddValueParser( "startBalance",     character.startBalance );
    AddValueParser( "startAurBalance",  character.startAurBalance );  // added config entry and implemented  -allan 01/10/14
    AddValueParser( "startStation",     character.startStation );
    AddValueParser( "startSecRating",   character.startSecRating );
    AddValueParser( "startCorporation", character.startCorporation );
    AddValueParser( "terminationDelay", character.terminationDelay );
    AddValueParser( "statMultiplier",   character.statMultiplier );
    AddValueParser( "allow3edChar",     character.allow3edChar );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "startBalance" );
    RemoveParser( "startAurBalance" );
    RemoveParser( "startStation" );
    RemoveParser( "startSecRating" );
    RemoveParser( "startCorporation" );
    RemoveParser( "terminationDelay" );
    RemoveParser( "statMultiplier" );
    RemoveParser( "allow3edChar" );

    return result;
}

bool EVEServerConfig::ProcessNPC( const TiXmlElement* ele )
{
    AddValueParser( "IdleWander",               npc.IdleWander );
    AddValueParser( "WarpOut",                  npc.WarpOut );
    AddValueParser( "WarpFollowChance",         npc.WarpFollowChance );
    AddValueParser( "ThreatRadius",             npc.ThreatRadius );
    AddValueParser( "RoamingSpawns",            npc.RoamingSpawns );
    AddValueParser( "StaticSpawns",             npc.StaticSpawns );
    AddValueParser( "RoamingTimer",             npc.RoamingTimer );
    AddValueParser( "StaticTimer",              npc.StaticTimer );
    AddValueParser( "RespawnTimer",             npc.RespawnTimer );
    AddValueParser( "RatFaction",               npc.RatFaction );
    AddValueParser( "TargetPod",                npc.TargetPod );
    AddValueParser( "TargetPodSec",             npc.TargetPodSec );
    AddValueParser( "UseDamageMultiplier",      npc.UseDamageMultiplier );
    AddValueParser( "LootDropChance",           npc.LootDropChance );
    AddValueParser( "DefenderMissileChance",    npc.DefenderMissileChance );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "IdleWander" );
    RemoveParser( "WarpOut" );
    RemoveParser( "WarpFollowChance" );
    RemoveParser( "ThreatRadius" );
    RemoveParser( "RoamingSpawns" );
    RemoveParser( "StaticSpawns" );
    RemoveParser( "RoamingTimer" );
    RemoveParser( "StaticTimer" );
    RemoveParser( "RespawnTimer" );
    RemoveParser( "RatFaction" );
    RemoveParser( "TargetPod" );
    RemoveParser( "TargetPodSec" );
    RemoveParser( "UseDamageMultiplier" );
    RemoveParser( "LootDropChance" );
    RemoveParser( "DefenderMissileChance" );

    return result;
}

bool EVEServerConfig::ProcessDatabase( const TiXmlElement* ele )
{
    AddValueParser( "host",             database.host );
    AddValueParser( "port",             database.port );
    AddValueParser( "username",         database.username );
    AddValueParser( "password",         database.password );
    AddValueParser( "db",               database.db );
    AddValueParser( "compress",         database.compress );
    AddValueParser( "ssl",              database.ssl );
    AddValueParser( "useSocket",        database.useSocket );
    AddValueParser( "autoReconnect",    database.autoReconnect );
    AddValueParser( "dbTimeout",        database.dbTimeout );
    AddValueParser( "pingTime",         database.pingTime );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "host" );
    RemoveParser( "port" );
    RemoveParser( "username" );
    RemoveParser( "password" );
    RemoveParser( "db" );
    RemoveParser( "compress" );
    RemoveParser( "ssl" );
    RemoveParser( "useSocket" );
    RemoveParser( "autoReconnect" );
    RemoveParser( "dbTimeout" );
    RemoveParser( "pingTime" );

    return result;
}

bool EVEServerConfig::ProcessFiles( const TiXmlElement* ele )
{
    AddValueParser( "logDir",           files.logDir );
    AddValueParser( "logSettings",      files.logSettings );
    AddValueParser( "cacheDir",         files.cacheDir );
    AddValueParser( "imageDir",         files.imageDir );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "logDir" );
    RemoveParser( "logSettings" );
    RemoveParser( "cacheDir" );
    RemoveParser( "imageDir" );

    return result;
}

bool EVEServerConfig::ProcessNet( const TiXmlElement* ele )
{
    AddValueParser( "port",             net.port );
    AddValueParser( "imageServerPort",  net.imageServerPort);
    AddValueParser( "imageServer",      net.imageServer);

    const bool result = ParseElementChildren( ele );

    RemoveParser( "port" );
    RemoveParser( "imageServerPort" );
    RemoveParser( "imageServer" );

    return result;
}

bool EVEServerConfig::ProcessThreads( const TiXmlElement* ele )
{
    AddValueParser( "ConsoleThreads",       threads.ConsoleThreads);
    AddValueParser( "DatabaseThreads",      threads.DatabaseThreads);
    AddValueParser( "ImageServerThreads",   threads.ImageServerThreads);
    AddValueParser( "NetworkThreads",       threads.NetworkThreads );
    AddValueParser( "WorldThreads",         threads.WorldThreads);

    const bool result = ParseElementChildren( ele );

    RemoveParser( "ConsoleThreads" );
    RemoveParser( "DatabaseThreads" );
    RemoveParser( "ImageServerThreads" );
    RemoveParser( "NetworkThreads" );
    RemoveParser( "WorldThreads" );

    return result;
}

bool EVEServerConfig::ProcessCosmic( const TiXmlElement* ele )
{
    AddValueParser( "PIEnabled",            cosmic.PIEnabled );
    AddValueParser( "AnomalyEnabled",       cosmic.AnomalyEnabled );
    AddValueParser( "DungeonEnabled",       cosmic.DungeonEnabled );
    AddValueParser( "BeltEnabled",          cosmic.BeltEnabled );
    AddValueParser( "BeltRespawn",          cosmic.BeltRespawn );
    AddValueParser( "BeltGrowth",           cosmic.BeltGrowth );
    AddValueParser( "roidRadiusMultiplier", cosmic.roidRadiusMultiplier );
    AddValueParser( "WormHoleEnabled",      cosmic.WormHoleEnabled );
    AddValueParser( "CiviliansEnabled",     cosmic.CiviliansEnabled);
    AddValueParser( "BumpEnabled",          cosmic.BumpEnabled );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "PIEnabled" );
    RemoveParser( "AnomalyEnabled" );
    RemoveParser( "DungeonEnabled" );
    RemoveParser( "BeltEnabled" );
    RemoveParser( "BeltRespawn" );
    RemoveParser( "BeltGrowth" );
    RemoveParser( "roidRadiusMultiplier" );
    RemoveParser( "WormHoleEnabled" );
    RemoveParser( "CiviliansEnabled" );
    RemoveParser( "BumpEnabled" );

    return result;
}

bool EVEServerConfig::ProcessExploring ( const TiXmlElement* ele ) {
    AddValueParser( "Radar",                exploring.Radar );
    AddValueParser( "Ladar",                exploring.Ladar );
    AddValueParser( "Unrated",              exploring.Unrated );
    AddValueParser( "Complex",              exploring.Complex );
    AddValueParser( "Gravametric",          exploring.Gravametric );
    AddValueParser( "Magnetometric",        exploring.Magnetometric );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "Radar" );
    RemoveParser( "Ladar" );
    RemoveParser( "Unrated" );
    RemoveParser( "Complex" );
    RemoveParser( "Gravametric" );
    RemoveParser( "Magnetometric" );

    return result;
}

bool EVEServerConfig::ProcessChat(const TiXmlElement* ele)
{
    AddValueParser( "EnableFleetChat",      chat.EnableFleetChat );
    AddValueParser( "EnableWingChat",       chat.EnableWingChat );
    AddValueParser( "EnableSquadChat",      chat.EnableSquadChat );
    AddValueParser( "EnableVoiceChat",      chat.EnableVoiceChat );
    AddValueParser( "EnforceRookieInHelp",  chat.EnforceRookieInHelp );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "EnableFleetChat" );
    RemoveParser( "EnableWingChat" );
    RemoveParser( "EnableSquadChat" );
    RemoveParser( "EnableVoiceChat" );
    RemoveParser( "EnforceRookieInHelp" );

    return result;
}

bool EVEServerConfig::ProcessStandings(const TiXmlElement* ele)
{
    AddValueParser( "MissionBonus",                     standings.MissionBonus );
    AddValueParser( "MissionFailure",                   standings.MissionFailure );
    AddValueParser( "MissionDeclined",                  standings.MissionDeclined );
    AddValueParser( "MissionCompleted",                 standings.MissionCompleted );
    AddValueParser( "MissionOfferExpired",              standings.MissionOfferExpired );
    AddValueParser( "MissionFailedRollback",            standings.MissionFailedRollback );
    AddValueParser( "ImportantMissionBonus",            standings.ImportantMissionBonus );
    AddValueParser( "BaseMissionMultiplier",            standings.BaseMissionMultiplier );
    AddValueParser( "FleetMissionMultiplier",           standings.FleetMissionMultiplier );
    AddValueParser( "Agent2CharMissionMultiplier",      standings.Agent2CharMissionMultiplier );
    AddValueParser( "ACorp2CharMissionMultiplier",      standings.ACorp2CharMissionMultiplier );
    AddValueParser( "Agent2PCorpMissionMultiplier",     standings.Agent2PCorpMissionMultiplier );
    AddValueParser( "ACorp2PCorpMissionMultiplier",     standings.ACorp2PCorpMissionMultiplier );
    AddValueParser( "AFaction2CharMissionMultiplier",   standings.AFaction2CharMissionMultiplier );
    AddValueParser( "AFaction2PCorpMissionMultiplier",  standings.AFaction2PCorpMissionMultiplier );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "MissionBonus" );
    RemoveParser( "MissionFailure" );
    RemoveParser( "MissionDeclined" );
    RemoveParser( "MissionCompleted" );
    RemoveParser( "MissionOfferExpired" );
    RemoveParser( "MissionFailedRollback" );
    RemoveParser( "ImportantMissionBonus" );
    RemoveParser( "BaseMissionMultiplier" );
    RemoveParser( "FleetMissionMultiplier" );
    RemoveParser( "Agent2CharMissionMultiplier" );
    RemoveParser( "ACorp2CharMissionMultiplier" );
    RemoveParser( "Agent2PCorpMissionMultiplier" );
    RemoveParser( "ACorp2PCorpMissionMultiplier" );
    RemoveParser( "AFaction2CharMissionMultiplier" );
    RemoveParser( "AFaction2PCorpMissionMultiplier" );

    return result;
}

bool EVEServerConfig::ProcessCrime( const TiXmlElement* ele )
{
    AddValueParser( "Enabled",          crime.Enabled );
    AddValueParser( "AggFlagTime",      crime.AggFlagTime );
    AddValueParser( "CrimFlagTime",     crime.CrimFlagTime );
    AddValueParser( "CWSessionTime",    crime.CWSessionTime );
    AddValueParser( "KillRightTime",    crime.KillRightTime );
    AddValueParser( "WeaponFlagTime",   crime.WeaponFlagTime );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "Enabled" );
    RemoveParser( "AggFlagTime" );
    RemoveParser( "CrimFlagTime" );
    RemoveParser( "CWSessionTime" );
    RemoveParser( "KillRightTime" );
    RemoveParser( "WeaponFlagTime" );

    return result;
}

bool EVEServerConfig::ProcessDebug(const TiXmlElement* ele)
{
    AddValueParser( "UseBeanCount",         debug.BeanCount );
    AddValueParser( "UseStackTrace",        debug.StackTrace );
    AddValueParser( "IsTestServer",         debug.IsTestServer );
    AddValueParser( "UseProfiling",         debug.UseProfiling );
    AddValueParser( "UseShipTracking",      debug.UseShipTracking );
    AddValueParser( "PositionHack",         debug.PositionHack );
    AddValueParser( "AnomalyFaction",       debug.AnomalyFaction );
    AddValueParser( "BubbleTrack",          debug.BubbleTrack );
    AddValueParser( "SpawnTest",            debug.SpawnTest );
    AddValueParser( "DeleteTrackingCans",   debug.DeleteTrackingCans );
    AddValueParser( "ProfileTraceTime",     debug.ProfileTraceTime );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "UseBeanCount" );
    RemoveParser( "UseStackTrace" );
    RemoveParser( "IsTestServer" );
    RemoveParser( "UseProfiling" );
    RemoveParser( "UseShipTracking" );
    RemoveParser( "PositionHack" );
    RemoveParser( "DeleteTrackingCans" );
    RemoveParser( "AnomalyFaction" );
    RemoveParser( "SpawnTest" );
    RemoveParser( "BubbleTrack" );
    RemoveParser( "ProfileTraceTime" );

    return result;
}

bool EVEServerConfig::ProcessTesting(const TiXmlElement* ele)
{
    AddValueParser( "ShipHeat",             testing.ShipHeat );
    AddValueParser( "EnableDrones",         testing.EnableDrones);

    const bool result = ParseElementChildren( ele );

    RemoveParser( "ShipHeat" );
    RemoveParser( "EnableDrones" );

    return result;
}
