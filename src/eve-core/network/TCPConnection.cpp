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
    Rewrite:    Allan
*/

#include "eve-core.h"

#include "log/logsys.h"
#include "log/LogNew.h"
#include "network/TCPConnection.h"
#include "network/NetUtils.h"
#include "threading/Threading.h"
#include "utils/timer.h"

const uint32 TCPCONN_RECVBUF_SIZE = 0x1000;
const uint32 TCPCONN_LOOP_GRANULARITY = 5;  /* 5ms */

TCPConnection::TCPConnection()
: mSock( nullptr ),
  mSockState( STATE_DISCONNECTED ),
  mrIP( 0 ),
  mrPort( 0 ),
  mRecvBuf( nullptr )
{
}

TCPConnection::TCPConnection( Socket* socket, uint32 mrIP, uint16 mrPort )
: mSock( socket ),
  mSockState( STATE_CONNECTED ),
  mrIP( mrIP ),
  mrPort( mrPort ),
  mRecvBuf( nullptr )
{
    // Start worker thread
    StartLoop();
}

TCPConnection::~TCPConnection()
{
    _log(THREAD__WARNING, "Destroying TCPConnection for thread 0x%X", pthread_self());
    // Make sure we are disconnected
    Disconnect();
    // Wait for loop to stop
    WaitLoop();
    // Clear buffers
    ClearBuffers();
}

std::string TCPConnection::GetAddress()
{
    /* "The Matrix is a system, Neo. That system is our enemy. But when you're inside, you look around, what do you see?" */
    in_addr addr;
    addr.s_addr = GetrIP();

    char address[22];
    int len = snprintf( address, 22, "%s:%u", inet_ntoa( addr ), GetrPort() );

    /* snprintf will return < 0 when a error occurs so return empty string */
    if( len < 0 )
        return std::string();

    return address;
}

bool TCPConnection::Connect( uint32 rIP, uint16 rPort, char* errbuf )
{
    if (errbuf)
        errbuf[0] = 0;

    MutexLock lock( mMSock );

    if (GetState() == STATE_DISCONNECTED) {
        mMSock.Unlock();
        // Wait for working thread to stop.
        WaitLoop();
        mMSock.Lock();
    }

    state_t oldState = GetState();
    if (oldState != STATE_DISCONNECTED && oldState != STATE_CONNECTING)
        return false;

    mSock = new Socket( AF_INET, SOCK_STREAM, 0 );

    sockaddr_in server_sin;
    server_sin.sin_family = AF_INET;
    server_sin.sin_addr.s_addr = rIP;
    server_sin.sin_port = htons( rPort );

    // Establish a connection to the server socket.
    if( mSock->connect( (sockaddr*)&server_sin, sizeof( server_sin ) ) == SOCKET_ERROR )
    {
        if( errbuf )
            snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "%s", strerror( errno ) );

        SafeDelete( mSock );
        return false;
    }

    int bufsize = 64 * 1024; // 64kbyte recieve buffer, up from default of 8k
    mSock->setopt( SOL_SOCKET, SO_RCVBUF, (char*) &bufsize, sizeof( bufsize ) );
    mSock->fcntl( F_SETFL, O_NONBLOCK );
    mrIP = rIP;
    mrPort = rPort;
    mSockState = STATE_CONNECTED;
    // Start processing thread if necessary
    if( oldState == STATE_DISCONNECTED )
        StartLoop();

    return true;
}

void TCPConnection::AsyncConnect( uint32 rIP, uint16 rPort )
{
    // Changing state; acquire mutex
    MutexLock lock( mMSock );

    if (GetState() == STATE_DISCONNECTED) {
        mMSock.Unlock();
        // Wait for working thread to stop.
        WaitLoop();
        mMSock.Lock();
    }

    if (GetState() != STATE_DISCONNECTED)
        return;

    mrIP = rIP;
    mrPort = rPort;
    mSockState = STATE_CONNECTING;
    // Start processing thread
    StartLoop();
}

void TCPConnection::Disconnect()
{
    MutexLock lock( mMSock );

    state_t state = GetState();
    if( state != STATE_CONNECTING && state != STATE_CONNECTED )
        return;

    // Change state
    mSockState = STATE_DISCONNECTING;
}

bool TCPConnection::Send( Buffer** data )
{
    // Invalidate pointer
    Buffer* buf = *data;
    *data = nullptr;

    // Check we are in STATE_CONNECTED
    MutexLock sockLock( mMSock );

    if (GetState() != STATE_CONNECTED) {
        SafeDelete( buf );
        return false;
    }

    // Push buffer to the send queue
    MutexLock queueLock( mMSendQueue );

    mSendQueue.push_back( buf );
    buf = nullptr;

    return true;
}

void TCPConnection::StartLoop()
{
    /** @note  update this to use thread pool instead of creating new threads.
     * check with sThread.XXXX() for avalible thread from current thread pool.
     * if one is avalible, it will be used, and if not, sThread will create a new one
     */
    sThread.CreateThread(TCPConnectionLoop, this);
    /*   ORIGINAL CODE HERE
    // Spawn new thread
    pthread_t thread;
    pthread_create( &thread, nullptr, TCPConnectionLoop, this );
    _log(THREAD__WARNING, "StartLoop() - Created thread ID 0x%X for TCPConnectionLoop", thread);
    sThread.AddThread(thread);*/
}

void TCPConnection::WaitLoop()
{
    // Block calling thread until work thread terminates
    mMLoopRunning.Lock();
    mMLoopRunning.Unlock();
}

/* This is always called from an IO thread. Either the server socket's thread, or a
 * special thread we create when we make an outbound connection. */
bool TCPConnection::Process() {
    char errbuf[ TCPCONN_ERRBUF_SIZE ];
    MutexLock lock( mMSock );
    switch (GetState()) {
        case STATE_CONNECTING: {
            if (!Connect( GetrIP(), GetrPort(), errbuf)) {
                _log(TCP_CLIENT__TRACE, "Process() - Connecting Failed at %s: %s", GetAddress().c_str(), errbuf );
                return false;
            }
            _log(TCP_CLIENT__INFO, "Process() - TCP connectection from %s", GetAddress().c_str() );
            return true;
        }
        case STATE_CONNECTED: {
            if (!RecvData(errbuf)) {
                _log(TCP_CLIENT__TRACE, "Process() - Connected RecvData() Failed at %s: %s", GetAddress().c_str(), errbuf );
                return false;
            }
            if (!SendData(errbuf)) {
                _log(TCP_CLIENT__TRACE, "Process() - Connected SendData() Failed at", "%s: %s", GetAddress().c_str(), errbuf );
                return false;
            }
            return true;
        }
        case STATE_DISCONNECTING: {
            if (!SendData(errbuf)) {
                _log(TCP_CLIENT__TRACE, "Process() - Disconnecting SendData() Failed at", "%s: %s", GetAddress().c_str(), errbuf );
                return false;
            }
            DoDisconnect();
            return true;
        }
        case STATE_DISCONNECTED:
        default: {
            return false;
        }
    }
}

bool TCPConnection::SendData( char* errbuf )
{
    if( errbuf )
    errbuf[0] = 0;

    MutexLock lock( mMSock );

    state_t state = GetState();
    if( state != STATE_CONNECTED && state != STATE_DISCONNECTING )
        return false;

    mMSendQueue.Lock();
    Buffer* buf(nullptr);
    int status(0);
    while (!mSendQueue.empty()) {
        buf = mSendQueue.front();
        mSendQueue.pop_front();
        mMSendQueue.Unlock();
        if (mSendQueue.empty())
            status = mSock->send( &(*buf)[ 0 ], (uint)buf->size(), MSG_NOSIGNAL);
        else
            status = mSock->send( &(*buf)[ 0 ], (uint)buf->size(), (MSG_NOSIGNAL | MSG_MORE) );
        if (status == SOCKET_ERROR) {
            if (errno == EWOULDBLOCK) {
                status = 0;
            } else {
                if( errbuf )
                    snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "%s", strerror( errno ) );
                SafeDelete( buf );
                return false;
            }
        }

        if ((size_t)status > buf->size()) {
            if (errbuf)
                snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "WTF?!?   status > size." );

            SafeDelete( buf );
            return false;
        } else if ((size_t)status < buf->size()) {
            if (status > 0)
                buf->AssignSeq( buf->begin<uint8>() + status, buf->end<uint8>() );
            MutexLock queueLock( mMSendQueue );
            mSendQueue.push_front( buf );
            buf = nullptr;
        } else {
            SafeDelete( buf );
        }
        mMSendQueue.Lock();
    }
    mMSendQueue.Unlock();
    return true;
}

bool TCPConnection::RecvData( char* errbuf )
{
    if (errbuf)
        errbuf[0] = 0;

    MutexLock lock( mMSock );

    state_t state = GetState();
    if ((state != STATE_CONNECTED) && (state != STATE_DISCONNECTING))
        return false;

    int status = 0;
    while (true) {
        if (!mRecvBuf)
            mRecvBuf = new Buffer( TCPCONN_RECVBUF_SIZE );
        else if( mRecvBuf->size() < TCPCONN_RECVBUF_SIZE )
            mRecvBuf->Resize<uint8>( TCPCONN_RECVBUF_SIZE );

        status = mSock->recv( &(*mRecvBuf)[ 0 ], (uint)mRecvBuf->size(), MSG_DONTWAIT);
        if (status == SOCKET_ERROR) {
            if (errno == EWOULDBLOCK)
                return true;
            if (errbuf)
                snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "%s", strerror(errno));
            return false;
        } else if (status == 0) {
            if (errbuf)
                snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "No Data Received.");
            return false;
        } else if (status) {
            mRecvBuf->Resize<uint8>(status);
            if (!ProcessReceivedData(errbuf))
                return false;
        } else {
            if (errbuf)
                snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "recv() returned unknown status");
            _log(TCP_CLIENT__ERROR, "TCPConnection::RecvData(): Error: recv() returned unknown status");
            return false;
        }
    }
    return true;
}

void TCPConnection::DoDisconnect()
{
    MutexLock lock( mMSock );

    state_t state = GetState();
    if ((state != STATE_CONNECTED) && (state != STATE_DISCONNECTING))
        return;

    ClearBuffers();
    mrIP = mrPort = 0;
    SafeDelete( mSock );

    mSockState = STATE_DISCONNECTED;
}

void TCPConnection::ClearBuffers()
{
    MutexLock lock( mMSendQueue );

    Buffer* buf = nullptr;
    while (!mSendQueue.empty()) {
        buf = mSendQueue.front();
        mSendQueue.pop_front();
        SafeDelete(buf);
    }
    SafeDelete(mRecvBuf);
}

void* TCPConnection::TCPConnectionLoop( void* arg )
{
    TCPConnection* tcpc = reinterpret_cast< TCPConnection* >( arg );
    assert( tcpc != nullptr );

    tcpc->TCPConnectionLoop();
    sThread.RemoveThread(pthread_self());

    return nullptr;
}

void TCPConnection::TCPConnectionLoop()
{
    mMLoopRunning.Lock();
    uint32 start = GetTickCount();
    while (Process()) {
        // do the stuff for thread sleeping
        start = GetTickCount() - start;
        if (TCPCONN_LOOP_GRANULARITY > start)
            Sleep(TCPCONN_LOOP_GRANULARITY - start);
        start = GetTickCount();
    }
    DoDisconnect();
    mMLoopRunning.Unlock();
}
