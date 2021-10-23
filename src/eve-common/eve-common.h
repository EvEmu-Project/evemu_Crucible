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
    Author:     Bloody.Rabbit
    Updates:	Allan
*/

#ifndef __EVE_COMMON_H__INCL__
#define __EVE_COMMON_H__INCL__

/************************************************************************/
/* core includes                                                    */
/************************************************************************/
#include "eve-core.h"

// UTF8-CPP
#include <utf8.h>

/************************************************************************/
/* eve-common includes                                                  */
/************************************************************************/
// database
#include "database/dbcore.h"
#include "database/dbtype.h"
// log
#include "log/LogNew.h"
#include "log/logsys.h"
// math
#include "math/gpoint.h"
#include "math/Trig.h"
// memory
#include "memory/RefPtr.h"
// network
#include "network/Socket.h"
#include "network/StreamPacketizer.h"
#include "network/TCPConnection.h"
#include "network/TCPServer.h"
// threading
#include "threading/Threading.h"
// eve math equations
#include "utils/EvEMath.h"
// utils
#include "utils/Buffer.h"
#include "utils/crc32.h"
#include "utils/Deflate.h"
#include "utils/EvilNumber.h"
#include "utils/misc.h"
#include "utils/Seperator.h"
#include "utils/timer.h"
#include "utils/utils_hex.h"
#include "utils/utils_string.h"
#include "utils/utils_time.h"
// defines
#include "EVE_Consts.h"
#include "EVE_Defines.h"
#include "EVE_Flags.h"
#include "EVE_Roles.h"
#include "EVE_Skills.h"
#include "EVE_Typedefs.h"
#include "EVE_Station.h"

#endif /* !__EVE_COMMON_H__INCL__ */
