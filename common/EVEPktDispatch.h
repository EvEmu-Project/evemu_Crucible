/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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
	Author:		Zhur
*/

#ifndef __EVEDISPATCHERS_H_INCL__
#define __EVEDISPATCHERS_H_INCL__

class PyPacket;
class PyRep;
class PyRepTuple;
class AuthenticationReq;
class AuthenticationRsp;
class PyCallStream;
class EVENotificationStream;
class SessionChangeNotification;
class ErrorResponseBody;

class EVEPacketDispatcher {
public:
	EVEPacketDispatcher();
	virtual ~EVEPacketDispatcher();
	
    virtual void DispatchPacket(PyPacket **packet); //consumes the packet
	
protected:
    /* these methods can choose to consume the ** arguments */
	virtual void Handle_AuthenticationReq(const PyPacket *packet, AuthenticationReq **call);
	virtual void Handle_AuthenticationRsp(const PyPacket *packet, AuthenticationRsp **rsp);
	virtual void Handle_CallReq(const PyPacket *packet, PyCallStream **call);
	virtual void Handle_CallRsp(const PyPacket *packet, PyRepTuple **res);
	virtual void Handle_Notify(const PyPacket *packet, EVENotificationStream **notify);
	virtual void Handle_SessionChange(const PyPacket *packet, SessionChangeNotification **notify);
	virtual void Handle_ErrorResponse(const PyPacket *packet, ErrorResponseBody **body);
	virtual void Handle_Other(PyPacket **packet);
};





#endif


