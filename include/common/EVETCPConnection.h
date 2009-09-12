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

#include "StreamPacketizer.h"
#include "TCPConnection.h"
#include "timer.h"

struct SPackSendQueue;
class EVETCPServer;

typedef StreamPacketizer::Packet EVENetPacket;

//not sure what we should set this to, I think the client does some sort of keep alive...
static const uint32 EVE_TCP_SERVER_TIMEOUT_MS = 600000;

class EVETCPConnection : public TCPConnection {
public:
	EVETCPConnection();
	EVETCPConnection(int32 ID, EVETCPServer* iServer, SOCKET iSock, int32 irIP, int16 irPort);
	virtual ~EVETCPConnection();

	//outgoing...
	virtual void	QueuePacket(EVENetPacket *tnps);
	virtual void	FastQueuePacket(EVENetPacket **tnps);

	//incoming
	EVENetPacket*	PopPacket();

protected:
	void	OutQueuePush(EVENetPacket* pack);

	virtual bool RecvData(char* errbuf = 0);

	//overload incoming
	virtual bool ProcessReceivedData(char* errbuf = 0);

	virtual void ClearBuffers();

	EVETCPServer*		Server;

	//input queue (received from network)
	Mutex MInQueue;
	Timer timeout_timer;
	StreamPacketizer InQueue;

	//output queue... (being sent out)
	MyQueue<EVENetPacket> OutQueue;
	Mutex	MOutQueueLock;
};

#endif /*EVETCPCONNECTION_H_*/
