/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2021 The EVEmu Team
 *    For the latest information visit https://github.com/evemuproject/evemu_server
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:        Zhur
 *    Updates:       Allan
 */

/** @todo  this file really isnt named correctly.  move like items into new files named accordingly. */
//  wip  -allan 29May15

#ifndef EVE_PACKET_TYPES_H
#define EVE_PACKET_TYPES_H

#include "../EVE_Corp.h"
#include "../EVE_Dungeon.h"
#include "../EVE_Effects.h"
#include "../EVE_Inventory.h"
#include "../EVE_Planet.h"
#include "../EVE_Wallet.h"


/*
 *
 * SERVICE_STOPPED = 1
 * SERVICE_START_PENDING = 2
 * SERVICE_STOP_PENDING = 3
 * SERVICE_RUNNING = 4
 * SERVICE_CONTINUE_PENDING = 5
 * SERVICE_PAUSE_PENDING = 6
 * SERVICE_PAUSED = 7
 * SERVICETYPE_NORMAL = 1
 * SERVICETYPE_BUILTIN = 2
 * SERVICETYPE_EXPORT_CONSTANTS = 4
 * SERVICE_CONTROL_STOP = 1
 * SERVICE_CONTROL_PAUSE = 2
 * SERVICE_CONTROL_CONTINUE = 3
 * SERVICE_CONTROL_INTERROGATE = 4
 * SERVICE_CONTROL_SHUTDOWN = 5
 * SERVICE_CHECK_NONE = 0
 * SERVICE_CHECK_CALL = 1
 * SERVICE_CHECK_INIT = 2
 * SERVICE_WANT_SESSIONS = 1
 * PRE_NONE = 0
 * PRE_AUTH = 1
 * PRE_HASCHAR = 2
 * PRE_HASSHIP = 4
 * PRE_INSTATION = 8
 * PRE_INFLIGHT = 16
 */

enum MACHONETMSG_TYPE
{
    AUTHENTICATION_REQ              = 0,
    AUTHENTICATION_RSP              = 1,
    IDENTIFICATION_REQ              = 2,
    IDENTIFICATION_RSP              = 3,
    __Fake_Invalid_Type             = 4,
    CALL_REQ                        = 6,
    CALL_RSP                        = 7,
    TRANSPORTCLOSED                 = 8,
    RESOLVE_REQ                     = 10,
    RESOLVE_RSP                     = 11,
    NOTIFICATION                    = 12,
    ERRORRESPONSE                   = 15,
    SESSIONCHANGENOTIFICATION       = 16,
    SESSIONINITIALSTATENOTIFICATION = 18,
    PING_REQ                        = 20,
    PING_RSP                        = 21,
    MOVEMENTNOTIFICATION            = 100,
    MACHONETMSG_TYPE_COUNT
};

enum MACHONETERR_TYPE
{
    UNMACHODESTINATION  = 0,
    UNMACHOCHANNEL      = 1,
    WRAPPEDEXCEPTION    = 2
};
//see PyPacket.cpp
extern const char* MACHONETMSG_TYPE_NAMES[MACHONETMSG_TYPE_COUNT];


enum SESSION_TYPE
{
    SESSION_TYPE_INVALID = 0,
    SESSION_TYPE_EXECUTIONCONTEXT = 1,
    SESSION_TYPE_SERVICE = 2,
    SESSION_TYPE_CREST = 3,
    SESSION_TYPE_ESP = 4,
    SESSION_TYPE_GAME = 5
};

//  packet_types *should* end here...at the end of the....wait for it...PACKET TYPES!!

/*
 * mailingListBlocked = 0
 * mailingListAllowed = 1
 * mailingListMemberMuted = 0
 * mailingListMemberDefault = 1
 * mailingListMemberOperator = 2
 * mailingListMemberOwner = 3
 * ALLIANCE_SERVICE_MOD = 200
 * CHARNODE_MOD = 64
 * PLANETARYMGR_MOD = 128
 * mailTypeMail = 1
 * mailTypeNotifications = 2
 * mailStatusMaskRead = 1
 * mailStatusMaskReplied = 2
 * mailStatusMaskForwarded = 4
 * mailStatusMaskTrashed = 8
 * mailStatusMaskDraft = 16
 * mailStatusMaskAutomated = 32
 * mailLabelInbox = 1
 * mailLabelSent = 2
 * mailLabelCorporation = 4
 * mailLabelAlliance = 8
 * mailLabelsSystem = mailLabelInbox + mailLabelSent + mailLabelCorporation + mailLabelAlliance
 * mailMaxRecipients = 50
 * mailMaxGroups = 1
 * mailMaxSubjectSize = 150
 * mailMaxBodySize = 8000
 * mailMaxTaggedBodySize = 10000
 * mailMaxLabelSize = 40
 * mailMaxNumLabels = 25
 * mailMaxPerPage = 100
 * mailTrialAccountTimer = 1
 * mailMaxMessagePerMinute = 5
 * mailinglistMaxMembers = 3000
 * mailinglistMaxMembersUpdated = 1000
 * mailingListMaxNameSize = 60
 */

/** misc costs
 * costCloneContract = 5600
 * costJumpClone = 100000
 */


#endif  //EVE_PACKET_TYPES_H