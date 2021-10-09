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
    Author:     "everyone" who ever changed this file.
*/

#ifndef __EVE_VERSION_H
#define __EVE_VERSION_H

// Client version info
// supported client is Crucible v1.6.5 build 360229
// this is ALL CPP information needed by the client
static const double EVEVersionNumber = 7.31;
static const uint16 MachoNetVersion = 320;
static const int32 EVEBuildVersion = 360229;
static const char* const EVEProjectRegion = "ccp";
static const char* const EVEProjectVersion = "EVE-EVE-TRANQUILITY@ccp";
static const char* const EVEProjectCodename = "EVE-EVE-TRANQUILITY";

static const int32 EVEBirthday = 170472;

/*  Allan's Static Definitions */
static const char* const EVEMU_REVISION = "0.8.4";
static const char* const EVEMU_BUILD_DATE = "08 October 2021";
/* match versions here with stated files for full support */
static const float Config_Version = 10.2; /* eve-server.xml and EveServerConfig.cpp */
static const float Log_Version = 10.2;    /* logtypes.h and log.ini */
/* AI versions for shitz-n-giggles */
static const float Joe_Version = 0.15;   /* MarketBot.xml and MarketBotConf.cpp */
/* these  dont have separate config files ...yet */
static const float NPC_AI_Version = 0.57;
static const float Drone_AI_Version = 0.15;
static const float Mission_Version = 0.31;
/* these arent used yet, but are placeholders for future expansion */
static const float Civilian_AI_Version = 0.10;
static const float Sentry_AI_Version = 0.10;
static const float POS_AI_Version = 0.01;
static const float Scan_Version = 0.43;

#endif
