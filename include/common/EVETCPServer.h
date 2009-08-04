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

#ifndef EVETCPSERVER_H_
#define EVETCPSERVER_H_

#include "TCPServer.h"

class EVETCPConnection;
struct EVETCPNetPacket_Struct;
class ServerPacket;

class EVETCPServer : public TCPServer<EVETCPConnection> {
public:
	EVETCPServer( int16 iPort = 0 );
	virtual ~EVETCPServer();
	
protected:
	//virtual void	Process();
	
	//factory method, really should be redone!
	virtual void CreateNewConnection(int32 ID, SOCKET in_socket, int32 irIP, int16 irPort);
};
#endif /*EVETCPSERVER_H_*/
