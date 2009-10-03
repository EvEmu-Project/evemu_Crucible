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

#ifndef EVETCPCONNECTION_H_
#define EVETCPCONNECTION_H_

class PyRep;

//not sure what we should set this to, I think the client does some sort of keep alive...
extern const uint32 EVE_TCP_CONNECTION_TIMEOUT_MS;
//this is a limit hard coded in the eve client! (NetClient.dll)
extern const uint32 EVE_TCP_CONNECTION_MAX_PACKET_LEN;

class EVETCPConnection : public TCPConnection {
public:
	EVETCPConnection();
	EVETCPConnection(int32 ID, SOCKET iSock, int32 irIP, int16 irPort);
	virtual ~EVETCPConnection();

	//outgoing...
	void    QueueRep( const PyRep* rep );

	//incoming
	PyRep*  PopRep();

protected:
	virtual bool RecvData(char* errbuf = 0);

	//overload incoming
	virtual bool ProcessReceivedData(char* errbuf = 0);

	virtual void ClearBuffers();

	Timer mTimeoutTimer;

    //input queue (received from network)
	Mutex MInQueue;
	StreamPacketizer InQueue;
};

#endif /*EVETCPCONNECTION_H_*/
