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
    Author:     Zhur
*/

#include "eve-common.h"

#include "marshal/EVEMarshal.h"
#include "marshal/EVEUnmarshal.h"
#include "network/EVETCPConnection.h"

/*************************************************************************/
/* EVETCPConnection                                                      */
/*************************************************************************/
const uint32 EVETCPConnection::TIMEOUT_MS = 10 * 60 * 1000; // 10 minutes
const uint32 EVETCPConnection::PACKET_SIZE_LIMIT = 1024 * 1024; // 1 megabyte

EVETCPConnection::EVETCPConnection()
: TCPConnection(),
  mTimeoutTimer( TIMEOUT_MS )
{
}

EVETCPConnection::EVETCPConnection( Socket* sock, uint32 rIP, uint16 rPort )
: TCPConnection( sock, rIP, rPort ),
  mTimeoutTimer( TIMEOUT_MS )
{
}

void EVETCPConnection::QueueRep( const PyRep* rep, bool compress/*true*/ )
{
    Buffer* pBuffer = new Buffer();

    // make room for length
    const Buffer::iterator<uint32> bufLen = pBuffer->end<uint32>();
    pBuffer->ResizeAt( bufLen, 1 );

    // 检查缓冲区大小
    size_t bufferSize = pBuffer->size();
    if (bufferSize > PACKET_SIZE_LIMIT) {
        sLog.Error("Network", "Packet length %zu exceeds hardcoded packet length limit %u.", 
                   bufferSize, PACKET_SIZE_LIMIT);
        SafeDelete(pBuffer);
        return;
    }

    bool success(false);
    if (compress) {
        success = MarshalDeflate(rep, *pBuffer);
    } else {
        success = MarshalDeflate(rep, *pBuffer, PACKET_SIZE_LIMIT);
    }

    if (success) {
        // if (is_log_enabled(DEBUG__DEBUG))
       //     DumpBuffer( pBuffer, PACKET_OUTBOUND );
        // 检查最终大小
        bufferSize = pBuffer->size();
        if (bufferSize > static_cast<size_t>(UINT32_MAX)) {
            sLog.Error("Network", "Final packet length %zu exceeds maximum uint32 value", bufferSize);
            SafeDelete(pBuffer);
            return;
        }

        // write length (subtract the size of length field itself)
        *bufLen = static_cast<uint32>(bufferSize - sizeof(uint32));
        Send(&pBuffer);
    } else {
        sLog.Error("Network", "Failed to marshal new packet.");
    }

    PySafeDecRef(rep);
    SafeDelete(pBuffer);
}

PyRep* EVETCPConnection::PopRep()
{
    PyRep* res(nullptr);

    MutexLock lock( mMInQueue );
    Buffer* packet = mInQueue.PopPacket();

    if (packet != nullptr) {
        if ( PACKET_SIZE_LIMIT < packet->size() ) {
            sLog.Error( "Network", "Packet length %lu exceeds hardcoded packet length limit %u.", packet->size(), PACKET_SIZE_LIMIT );
        } else {
           // if (is_log_enabled(DEBUG__DEBUG))
           //     DumpBuffer( packet, PACKET_INBOUND );
            res = InflateUnmarshal( *packet );
        }
    }

    SafeDelete( packet );
    return res;
}

bool EVETCPConnection::ProcessReceivedData( char* errbuf )
{
    if (errbuf != nullptr)
        errbuf[0] = 0;

    MutexLock lock( mMInQueue );

    // put bytes into packetizer
    mInQueue.InputData( *mRecvBuf );
    // process packetizer
    mInQueue.Process();
    mTimeoutTimer.Start();

    return true;
}

bool EVETCPConnection::RecvData( char* errbuf )
{
    if( !TCPConnection::RecvData( errbuf ) )
        return false;

    if( mTimeoutTimer.Check() ) {
        if (errbuf != nullptr)
            snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "Connection timeout" );
        return false;
    }

    return true;
}

void EVETCPConnection::ClearBuffers()
{
    TCPConnection::ClearBuffers();
    mTimeoutTimer.Start();
    MutexLock lock( mMInQueue );
    mInQueue.ClearBuffers();
}

void EVETCPConnection::DumpBuffer( Buffer* buf, packet_direction packet_direction)
{
    /*
    FILE *logpacket;
    char timestamp[16];
    time_t rawtime = time(0);
    tm *now = localtime(&rawtime);
    strftime(timestamp,16,"%y%m%d_%H%M%S",now);

    logpacket = fopen(strcat("c:\\logs\\",strcat(timestamp,".txt")), "w");
    Buffer:: size_type index;
    for(index=0; index<= packet->size(); index++)
    {
        fputs(packet->Get(index), logpacket);
    }
    */

    FILE *logpacket;
    char timestamp[16];
    time_t rawtime = time(0);
    tm *now = localtime(&rawtime);
    strftime(timestamp,16,"%y%m%d_%H%M%S",now);

    std::string path = EVEMU_ROOT "/packet_log/";
    path += timestamp;
    if(packet_direction == PACKET_INBOUND)
        path += "_client_";
    else
        path += "_server_";
    path += ".txt";
    logpacket = fopen(path.c_str(), "w");

    Buffer::iterator<uint8> cur = buf->begin<uint8>();
    for (; cur != buf->end<uint8>(); ++cur) {
        uint8 test = *cur;
        fputc(test, logpacket);
    }
    fclose(logpacket);
}


