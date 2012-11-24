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
    Author:     Zhur
*/

#ifndef __EVE_SERVER_H__INCL__
#define __EVE_SERVER_H__INCL__

/************************************************************************/
/* eve-core includes                                                    */
/************************************************************************/
#include "eve-core.h"

// database
#include "database/dbcore.h"
// log
#include "log/LogNew.h"
#include "log/logsys.h"
// network
#include "network/StreamPacketizer.h"
#include "network/TCPConnection.h"
#include "network/TCPServer.h"
// threading
#include "threading/Mutex.h"
// utils
#include "utils/crc32.h"
#include "utils/Deflate.h"
#include "utils/EvilNumber.h"
#include "utils/gpoint.h"
#include "utils/misc.h"
#include "utils/RefPtr.h"
#include "utils/Seperator.h"
#include "utils/timer.h"
#include "utils/utils_time.h"
#include "utils/utils_string.h"
#include "utils/XMLParserEx.h"
#include "utils/Util.h"

/************************************************************************/
/* eve-common includes                                                  */
/************************************************************************/
#include "eve-common.h"

#include "EVEVersion.h"
// auth
#include "auth/PasswordModule.h"
// cache
#include "cache/CachedObjectMgr.h"
// database
#include "database/EVEDBUtils.h"
// destiny
#include "destiny/DestinyBinDump.h"
#include "destiny/DestinyStructs.h"
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
// packets
#include "packets/AccountPkts.h"
#include "packets/CorporationPkts.h"
#include "packets/Crypto.h"
#include "packets/Character.h"
#include "packets/Destiny.h"
#include "packets/DogmaIM.h"
#include "packets/General.h"
#include "packets/Inventory.h"
#include "packets/Wallet.h"
#include "packets/Missions.h"
#include "packets/Language.h"
#include "packets/LSCPkts.h"
#include "packets/Manufacturing.h"
#include "packets/ObjectCaching.h"
#include "packets/Market.h"
#include "packets/Sovereignty.h"
#include "packets/Standing2.h"
#include "packets/Trade.h"
#include "packets/Tutorial.h"
#include "packets/Mail.h"
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
// tables
#include "tables/invCategories.h"
#include "tables/invGroups.h"
// utils
#include "utils/EVEUtils.h"
#include "utils/EvilNumber.h"

/************************************************************************/
/* eve-server includes                                                  */
/************************************************************************/
// Lua
#include <lua.h>

#endif /* !__EVE_SERVER_H__INCL__ */
