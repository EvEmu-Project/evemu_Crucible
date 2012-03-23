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
    Author:     "everyone" who ever changed this file.
*/

#ifndef __EVE_VERSION_H
#define __EVE_VERSION_H

// supported client is Crucible v1.0.1 build 336683

static const uint16 MachoNetVersion = 315;
static const double EVEVersionNumber = 7.30;
static const int32 EVEBuildVersion = 353549;
static const char *const EVEProjectCodename = "EVE-EVE-TRANQUILITY";
static const char *const EVEProjectRegion = "ccp";
static const char *const EVEProjectVersion = "EVE-EVE-TRANQUILITY@ccp";

static const int32 EVEBirthday = 170472;

// EVEMU_VERSION
// The version of source.
#define EVEMU_SOURCE_LOCATION "http://www.github.com/evemuproject/evemu_crucible"
#define EVEMU_VERSION "0.7.9"


#endif