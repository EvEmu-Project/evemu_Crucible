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
    Author:     Bloody.Rabbit
*/

#ifndef __EVE_COMMON_PCH_H__INCL__
#define __EVE_COMMON_PCH_H__INCL__

#include "common.h"

/************************************************************************/
/* dep includes                                                         */
/************************************************************************/
#include <Base64.h>

#include <mysql.h>

#include <utf8.h>

/************************************************************************/
/* common includes                                                      */
/************************************************************************/
#include "database/dbtype.h"

#include "log/logsys.h"
#include "log/LogNew.h"

#include "network/Socket.h"
#include "network/StreamPacketizer.h"
#include "network/TCPConnection.h"
#include "network/TCPServer.h"

#include "utils/Buffer.h"
#include "utils/crc32.h"
#include "utils/Deflate.h"
#include "utils/misc.h"
#include "utils/RefPtr.h"
#include "utils/Singleton.h"
#include "utils/timer.h"
#include "utils/utils_hex.h"
#include "utils/utils_string.h"
#include "utils/utils_time.h"

#endif /* !__EVE_COMMON_PCH_H__INCL__ */
