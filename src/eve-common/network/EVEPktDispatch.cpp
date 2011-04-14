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

#include "EVECommonPCH.h"

#include "network/EVEPktDispatch.h"
#include "packets/AccountPkts.h"
#include "packets/General.h"
#include "python/PyPacket.h"
#include "python/PyVisitor.h"
#include "python/PyRep.h"

bool EVEPacketDispatcher::DispatchPacket( PyPacket* packet )
{
    switch( packet->type )
    {
        case AUTHENTICATION_REQ:
        {
            //check the string part, just for good measure
            if( packet->type_string != "macho.AuthenticationReq" )
            {
				sLog.Error("EVEPacketDispatcher","Received AUTHENTICATION_RSP with invalid type string '%s'", packet->type_string.c_str());
                return false;
            }

            AuthenticationReq req;
            if( !req.Decode( packet->payload ) )
            {
				sLog.Error("EVEPacketDispatcher","Failed to decode AuthenticationReq");
                return false;
            }

            return Handle_AuthenticationReq( packet, req );
        }

        case AUTHENTICATION_RSP:
        {
            //check the string part, just for good measure
            if( packet->type_string != "macho.AuthenticationRsp" )
            {
				sLog.Error("EVEPacketDispatcher","Received AUTHENTICATION_RSP with invalid type string '%s'", packet->type_string.c_str());
                return false;
            }

            AuthenticationRsp rsp;
            if( !rsp.Decode( packet->payload ) )
            {
				sLog.Error("EVEPacketDispatcher","Failed to decode AuthenticationRsp");
                return false;
            }

            return Handle_AuthenticationRsp( packet, rsp );
        }

        case CALL_REQ:
        {
            //check the string part, just for good measure
            if( packet->type_string != "macho.CallReq" )
            {
				sLog.Error("EVEPacketDispatcher","Received CALL_REQ with invalid type string '%s'", packet->type_string.c_str());
                return false;
            }

            PyCallStream call;
            if( !call.Decode( packet->type_string, packet->payload ))
            {
				sLog.Error("EVEPacketDispatcher","Failed to convert packet into a call stream");
                return false;
            }

            return Handle_CallReq( packet, call );
        }

        case CALL_RSP:
        {
            //check the string part, just for good measure
            if( packet->type_string != "macho.CallRsp" )
            {
				sLog.Error("EVEPacketDispatcher","Received CALL_RSP with invalid type string '%s'", packet->type_string.c_str());
                return false;
            }

            //TODO: decode substream in tuple

            return Handle_CallRsp( packet );
        } break;

        case NOTIFICATION:
        {
            //check the string part, just for good measure
            if( packet->type_string != "macho.Notification" )
            {
				sLog.Error("EVEPacketDispatcher","Received NOTIFICATION with invalid type string '%s'", packet->type_string.c_str());
                return false;
            }

            return Handle_Notify( packet );
        }

        case ERRORRESPONSE:
        {
            //check the string part, just for good measure
            if( packet->type_string != "macho.ErrorResponse" )
            {
				sLog.Error("EVEPacketDispatcher","Received ERRORRESPONSE with invalid type string '%s'", packet->type_string.c_str());
                return false;
            }

            ErrorResponse error;
            if( !error.Decode( packet->payload ) )
            {
				sLog.Error("EVEPacketDispatcher","Failed to decode Error Response");
                return false;
            }

            return Handle_ErrorResponse( packet, error );
        }

        case SESSIONCHANGENOTIFICATION:
        {
            //check the string part, just for good measure
            if( packet->type_string != "macho.SessionChangeNotification" )
            {
				sLog.Error("EVEPacketDispatcher","Received SESSIONCHANGENOTIFICATION with invalid type string '%s'", packet->type_string.c_str());
                return false;
            }

            SessionChangeNotification sessionChange;
            if( !sessionChange.Decode( packet->payload ) )
            {
				sLog.Error("EVEPacketDispatcher","Failed to decode session change notification");
                return false;
            }

            return Handle_SessionChange( packet, sessionChange );
        }

        case PING_REQ:
        {
            //check the string part, just for good measure
            if( packet->type_string != "macho.PingReq" )
            {
				sLog.Error("EVEPacketDispatcher","Received PING_REQ with invalid type string '%s'", packet->type_string.c_str());
                return false;
            }

            return Handle_PingReq( packet );
        }

        case PING_RSP:
        {
            //check the string part, just for good measure
            if( packet->type_string != "macho.PingRsp" )
            {
				sLog.Error("EVEPacketDispatcher","Received PING_RSP with invalid type string '%s'", packet->type_string.c_str());
                return false;
            }

            return Handle_PingRsp( packet );
        }

        default:
        {
            return Handle_Other( packet );
        }
    }
}

/* default handlers do nothing but print */
bool EVEPacketDispatcher::Handle_AuthenticationReq( PyPacket* packet, AuthenticationReq& req )
{
	sLog.Error("EVEPacketDispatcher","Unhandled Authentication Request");
    return false;
}
bool EVEPacketDispatcher::Handle_AuthenticationRsp( PyPacket* packet, AuthenticationRsp& rsp )
{
	sLog.Error("EVEPacketDispatcher","Unhandled Authentication Response");
    return false;
}

bool EVEPacketDispatcher::Handle_CallReq( PyPacket* packet, PyCallStream& req )
{
	sLog.Error("EVEPacketDispatcher","Unhandled Call Request");
    return false;
}
bool EVEPacketDispatcher::Handle_CallRsp( PyPacket* packet )
{
	sLog.Error("EVEPacketDispatcher","Unhandled Call Response");
    return false;
}
bool EVEPacketDispatcher::Handle_ErrorResponse( PyPacket* packet, ErrorResponse& body )
{
	sLog.Error("EVEPacketDispatcher","Unhandled Error Response");
    return false;
}

bool EVEPacketDispatcher::Handle_Notify( PyPacket* packet )
{
	sLog.Error("EVEPacketDispatcher","Unhandled Notification");
    return false;
}
bool EVEPacketDispatcher::Handle_SessionChange( PyPacket* packet, SessionChangeNotification& notify )
{
	sLog.Error("EVEPacketDispatcher","Unhandled SessionChange");
    return false;
}

bool EVEPacketDispatcher::Handle_PingReq( PyPacket* packet )
{
	sLog.Error("EVEPacketDispatcher","Unhandled Ping Request.");
    return false;
}
bool EVEPacketDispatcher::Handle_PingRsp( PyPacket* packet )
{
	sLog.Error("EVEPacketDispatcher","Unhandled Ping Response.");
    return false;
}

bool EVEPacketDispatcher::Handle_Other( PyPacket* packet )
{
	sLog.Error("EVEPacketDispatcher","Unhandled Packet of type %s (%d)", MACHONETMSG_TYPE_NAMES[ packet->type ], (int)packet->type);
    return false;
}

















