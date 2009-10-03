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

#include "EVECommonPCH.h"

#include "marshal/EVEMarshal.h"
#include "marshal/EVEUnmarshal.h"
#include "network/EVETCPConnection.h"

const uint32 EVE_TCP_CONNECTION_TIMEOUT_MS = 10 * 60 * 1000; // 10 minutes
const uint32 EVE_TCP_CONNECTION_MAX_PACKET_LEN = 1000000;

//client side
EVETCPConnection::EVETCPConnection()
: TCPConnection(),
  mTimeoutTimer(EVE_TCP_CONNECTION_TIMEOUT_MS)
{
}

//server side case
EVETCPConnection::EVETCPConnection(int32 ID, SOCKET in_socket, int32 irIP, int16 irPort)
: TCPConnection(ID, in_socket, irIP, irPort),
  mTimeoutTimer(EVE_TCP_CONNECTION_TIMEOUT_MS)
{
}

EVETCPConnection::~EVETCPConnection()
{
	//the queues free their content right now I believe.
}

void EVETCPConnection::QueueRep( const PyRep* rep )
{
    uint32 length;
    uint8* buf = MarshalDeflate( rep, length );

    if( buf == NULL )
        sLog.Error( "Network", "Failed to marshal new packet." );
    else if( length > EVE_TCP_CONNECTION_MAX_PACKET_LEN )
        sLog.Error( "Network", "Packet length %u exceeds hardcoded packet length limit %u.", length, EVE_TCP_CONNECTION_MAX_PACKET_LEN );
    else
        ServerSendQueuePushEnd( (const uint8 *)&length, sizeof( length ), &buf, length );

	SafeDelete( buf );
}

PyRep* EVETCPConnection::PopRep()
{
	if( !MInQueue.trylock() )
		return NULL;
    StreamPacketizer::Packet* p = InQueue.PopPacket();
	MInQueue.unlock();

    PyRep* res = NULL;
    if( p != NULL )
    {
        if( p->length > EVE_TCP_CONNECTION_MAX_PACKET_LEN )
            sLog.Error( "Network", "Packet length %u exceeds hardcoded packet length limit %u.", p->length, EVE_TCP_CONNECTION_MAX_PACKET_LEN );
        else
            res = InflateUnmarshal( p->data, p->length );
    }

    SafeDelete( p );
	return res;
}

bool EVETCPConnection::ProcessReceivedData(char* errbuf) {
	if (errbuf)
		errbuf[0] = 0;
	if (!recvbuf)
		return true;

	MInQueue.lock();
    {
		mTimeoutTimer.Start();
		
		InQueue.InputBytes(recvbuf, recvbuf_used);
		if(recvbuf_size > 1024) {
			//if the recvbuf grows too large, kill it off and start over
			//this is purely an optimization, it could be killed each time or never...
			SafeDeleteArray(recvbuf);
			recvbuf_size = 0;
		}
		recvbuf_used = 0;
    }
	MInQueue.unlock();
	
	return true;
}

bool EVETCPConnection::RecvData(char* errbuf)
{
	if( !TCPConnection::RecvData( errbuf ) )
        return false;
	
	if( mTimeoutTimer.Check() )
    {
		if( errbuf )
			snprintf( errbuf, TCPConnection_ErrorBufferSize, "EVETCPConnection::RecvData(): Connection timeout" );
		return false;
	}

	return true;
}

void EVETCPConnection::ClearBuffers()
{
	TCPConnection::ClearBuffers();

    MInQueue.lock();
    {
	    mTimeoutTimer.Start();

	    InQueue.ClearBuffers();
    }
	MInQueue.unlock();
}

