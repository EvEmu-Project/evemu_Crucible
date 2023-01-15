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
    Author:     Zhur
    Updates:	Allan  (updated dependency loading order)
*/

#ifndef __EVE_SERVER_H__INCL__
#define __EVE_SERVER_H__INCL__

/** @todo  this file has many specific headers not used by everything in evemu.
 * this should be trimmed down with specific headers in files that need them
 */

/************************************************************************/
/* eve-common includes (also includes eve-core.h)                       */
/************************************************************************/
#include "eve-common.h"

/************************************************************************/
/* eve-server includes                                                  */
/************************************************************************/
// profile
#include "Profiler.h"
// auth
#include "auth/PasswordModule.h"
// cache
#include "cache/CachedObjectMgr.h"
// database
#include "database/EVEDBUtils.h"
// destiny
#include "destiny/DestinyBinDump.h"
//#include "destiny/DestinyStructs.h"
// network
#include "network/EVETCPConnection.h"
#include "network/EVETCPServer.h"
#include "network/EVEPktDispatch.h"
#include "network/EVESession.h"
// marshal
#include "marshal/EVEMarshal.h"
#include "marshal/EVEMarshalOpcodes.h"
#include "marshal/EVEMarshalStringTable.h"
#include "marshal/EVEUnmarshal.h"
// memory
#include "memory/StackAllocator.h"
// packets
// TODO  work on moving these OUT of this file
#include "packets/AccountPkts.h"
//  #include "packets/Bookmarks.h"
//  #include "packets/CorporationPkts.h"
#include "packets/Crypto.h"
#include "packets/Calendar.h"
#include "packets/Character.h"
#include "packets/Destiny.h"
#include "packets/DogmaIM.h"
//  #include "packets/Fleet.h"
#include "packets/General.h"
#include "packets/Inventory.h"
//
//  #include "packets/LSCPkts.h"
//  #include "packets/Manufacturing.h"
//  #include "packets/Market.h"
#include "packets/Mail.h"
//
//  #include "packets/Missions.h"
#include "packets/ObjectCaching.h"
//  #include "packets/Sovereignty.h"
//
//  #include "packets/Scan.h"
//  #include "packets/Trade.h"
#include "packets/Tutorial.h"
#include "packets/Wallet.h"

// python
#include "python/PyVisitor.h"
#include "python/PyRep.h"
#include "python/PyPacket.h"
#include "python/PyDumpVisitor.h"
#include "python/PyLookupDump.h"
#include "python/PyXMLGenerator.h"
#include "python/classes/PyDatabase.h"
#include "python/classes/PyExceptions.h"
#include "python/classes/PyUtils.h"
// Common Enums
#include "inventory/AttributeEnum.h"
// pointer references for all classes
#include "inventory/ItemRef.h"
// tables
#include "tables/invCategories.h"
#include "tables/invGroups.h"
#include "tables/invTypes.h"
// threading
//#include "threading/Mutex.h"
// utils
#include "utils/EVEUtils.h"

#include "Allocators.h"

#endif /* !__EVE_SERVER_H__INCL__ */
