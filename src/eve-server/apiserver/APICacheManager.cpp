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
    Author:        Aknor Jaden
*/

#include "eve-server.h"

#include "apiserver/APICacheManager.h"

APICacheManager::APICacheManager()
{
}

bool APICacheManager::CacheRetrieve(const std::string * apiDescriptor, std::string * xmlDoc)
{
    // TODO:
    // 1) run apiDescriptor string through SHA1 hash, then search for a file
    //    with the name of the hash with '.apiobj' extension
    // 2) if no file found, then return FALSE
    // 3) if file found, open it and grab first line, inspect this expriation time and compare to current time
    // 4) if expiration has passed, return FALSE
    // 5) if expiration has NOT passed, read rest of file contents and copy to xmlDoc string, then return TRUE
    return false;
}

bool APICacheManager::CacheDeposit(const std::string * apiDescriptor, const std::string * xmlDoc, uint64 win32timeExpiration)
{
    // TODO:
    // 1) run apiDescriptor string through SHA1 hash, then create a file
    //    with the name of the hash with '.apiobj' extension
    // 2) check to see if this filename already exists, if so, return FALSE
    // 3) place the win32timeExpiration value at the first line of the file, this indicates when this cached object expires
    // 4) place the xmlDoc string in the rest of the file, save and close it
    // 5) return TRUE
    return false;
}
