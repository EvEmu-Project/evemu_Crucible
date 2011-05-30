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

#include "CommonPCH.h"

#include "log/logsys.h"
#include "log/LogNew.h"
#include "network/TCPConnection.h"
#include "network/NetUtils.h"
#include "utils/timer.h"

const uint32 TCPCONN_RECVBUF_SIZE = 0x1000;
const uint32 TCPCONN_LOOP_GRANULARITY = 5;

#ifdef WIN32
static InitWinsock winsock;
#endif

TCPConnection::TCPConnection()
: mSock( NULL ),
  mSockState( STATE_DISCONNECTED ),
  mrIP( 0 ),
  mrPort( 0 ),
  mRecvBuf( NULL )
{
}

TCPConnection::TCPConnection( Socket* socket, uint32 mrIP, uint16 mrPort )
: mSock( socket ),
  mSockState( STATE_CONNECTED ),
  mrIP( mrIP ),
  mrPort( mrPort ),
  mRecvBuf( NULL )
{
    // Start worker thread
    StartLoop();
}

TCPConnection::~TCPConnection()
{
    // Make sure we are disconnected
    Disconnect();

    // Wait for loop to stop
    WaitLoop();

    // Clear buffers
    ClearBuffers();
}

std::string TCPConnection::GetAddress()
{
    /* "The Matrix is a system, 'Neo'. That system is our enemy. But when you're inside, you look around, what do you see?" */
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
    if( errbuf )
        errbuf[0] = 0;

    MutexLock lock( mMSock );

    state_t oldState = GetState();
    if( oldState == STATE_DISCONNECTED )
    {
        mMSock.Unlock();

        // Wait for working thread to stop.
        WaitLoop();

        mMSock.Lock();
    }

    oldState = GetState();
    if( oldState != STATE_DISCONNECTED && oldState != STATE_CONNECTING )
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
#ifdef WIN32
            snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "TCPConnection::Connect(): connect() failed. Error: %i", WSAGetLastError() );
#else
            snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "TCPConnection::Connect(): connect() failed. Error: %s", strerror( errno ) );
#endif

        SafeDelete( mSock );
        return false;
    }

    int bufsize = 64 * 1024; // 64kbyte recieve buffer, up from default of 8k
    mSock->setopt( SOL_SOCKET, SO_RCVBUF, (char*) &bufsize, sizeof( bufsize ) );

#ifdef WIN32
    unsigned long nonblocking = 1;
    mSock->ioctl( FIONBIO, &nonblocking );
#else
    mSock->fcntl( F_SETFL, O_NONBLOCK );
#endif

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

    state_t state = GetState();
    if( state == STATE_DISCONNECTED )
    {
        mMSock.Unlock();

        // Wait for working thread to stop.
        WaitLoop();

        mMSock.Lock();
    }

    state = GetState();
    if( state != STATE_DISCONNECTED )
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
    *data = NULL;

    // Check we are in STATE_CONNECTED
    MutexLock sockLock( mMSock );

    state_t state = GetState();
    if( state != STATE_CONNECTED )
    {
        SafeDelete( buf );

        return false;
    }

    // Push buffer to the send queue
    MutexLock queueLock( mMSendQueue );

    mSendQueue.push_back( buf );
    buf = NULL;

    return true;
}

void TCPConnection::StartLoop()
{
    // Spawn new thread
#ifdef WIN32
    _beginthread( TCPConnectionLoop, 0, this );
#else
    pthread_t thread;
    pthread_create( &thread, NULL, TCPConnectionLoop, this );
#endif
}

void TCPConnection::WaitLoop()
{
    // Block calling thread until work thread terminates
    mMLoopRunning.Lock();
    mMLoopRunning.Unlock();
}

/* This is always called from an IO thread. Either the server socket's thread, or a
 * special thread we create when we make an outbound connection. */
bool TCPConnection::Process()
{
    char errbuf[ TCPCONN_ERRBUF_SIZE ];

    MutexLock lock( mMSock );
    switch( GetState() )
    {
        case STATE_DISCONNECTED:
        default:
        {
            // Nothing to do
            return false;
        }

        case STATE_CONNECTING:
        {
            // Connect
            if( !Connect( GetrIP(), GetrPort(), errbuf ) )
            {
                sLog.Error( "TCPConnection", "%s: %s.", GetAddress().c_str(), errbuf );

                DoDisconnect();
                return false;
            }

            // All we had to do is connect, and it succeeded
            return true;
        }

        case STATE_CONNECTED:
        {
            // Receive data
            if( !RecvData( errbuf ) )
            {
                sLog.Error( "TCPConnection", "%s: %s.", GetAddress().c_str(), errbuf );

                DoDisconnect();
                return false;
            }

            // Send data
            if( !SendData( errbuf ) )
            {
                sLog.Error( "TCPConnection", "%s: %s.", GetAddress().c_str(), errbuf );

                DoDisconnect();
                return false;
            }

            // Both send and recv succeeded.
            return true;
        }

        case STATE_DISCONNECTING:
        {
            // Send anything that may be pending
            if( !SendData( errbuf ) )
            {
                sLog.Error( "TCPConnection", "%s: %s.", GetAddress().c_str(), errbuf );

                DoDisconnect();
                return false;
            }

            // Send queue is empty, disconnect
            DoDisconnect();
            return true;
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
    while( !mSendQueue.empty() )
    {
        Buffer* buf = mSendQueue.front();
        mSendQueue.pop_front();
        mMSendQueue.Unlock();

        int status = mSock->send( &(*buf)[ 0 ], buf->size(), MSG_NOSIGNAL );

        if( status == SOCKET_ERROR )
        {
#ifdef WIN32
            if( WSAGetLastError() == WSAEWOULDBLOCK )
#else
            if( errno == EWOULDBLOCK )
#endif /* !WIN32 */
            {
                // Act like nothing was sent
                status = 0;
            }
            else
            {
                if( errbuf )
#ifdef WIN32
                    snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "TCPConnection::SendData(): send(): Errorcode: %u", WSAGetLastError() );
#else
                    snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "TCPConnection::SendData(): send(): Errorcode: %s", strerror( errno ) );
#endif

                SafeDelete( buf );
                return false;
            }
        }

        if( (size_t)status > buf->size() )
        {
            if( errbuf )
                snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "TCPConnection::SendData(): WTF! status > size." );

            SafeDelete( buf );
            return false;
        }
        else if( (size_t)status < buf->size() )
        {
            if( status > 0 )
                buf->AssignSeq( buf->begin<uint8>() + status, buf->end<uint8>() );

            MutexLock queueLock( mMSendQueue );

            mSendQueue.push_front( buf );
            buf = NULL;
        }
        else
        {
            SafeDelete( buf );
        }

        mMSendQueue.Lock();
    }
    mMSendQueue.Unlock();

    return true;
}

bool TCPConnection::RecvData( char* errbuf )
{
    if( errbuf != NULL )
        errbuf[0] = 0;

    MutexLock lock( mMSock );

    state_t state = GetState();
    if( state != STATE_CONNECTED && state != STATE_DISCONNECTING )
        return false;

    while( true )
    {
        if( mRecvBuf == NULL )
            mRecvBuf = new Buffer( TCPCONN_RECVBUF_SIZE );
        else if( mRecvBuf->size() < TCPCONN_RECVBUF_SIZE )
            mRecvBuf->Resize<uint8>( TCPCONN_RECVBUF_SIZE );

        int status = mSock->recv( &(*mRecvBuf)[ 0 ], mRecvBuf->size(), 0 );

        if( status > 0 )
        {
            mRecvBuf->Resize<uint8>( status );

            if( !ProcessReceivedData( errbuf ) )
                return false;
        }
        else if( status == 0 )
        {
            snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "TCPConnection::RecvData(): Connection closed" );

            return false;
        }
        else if( status == SOCKET_ERROR )
        {
#ifdef WIN32
            if ( WSAGetLastError() == WSAEWOULDBLOCK )
#else
            if ( errno == EWOULDBLOCK )
#endif
            {
                return true;
            }
            else
            {
                if( errbuf )
#ifdef WIN32
                    snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "TCPConnection::RecvData(): Error: %i", WSAGetLastError() );
#else
                    snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "TCPConnection::RecvData(): Error: %s", strerror( errno ) );
#endif

                return false;
            }
        }
    }
}

void TCPConnection::DoDisconnect()
{
    MutexLock lock( mMSock );

    state_t state = GetState();
    if( state != STATE_CONNECTED && state != STATE_DISCONNECTING )
        return;

    SafeDelete( mSock );
    mrIP = mrPort = 0;
    ClearBuffers();

    mSockState = STATE_DISCONNECTED;
}

void TCPConnection::ClearBuffers()
{
    MutexLock lock( mMSendQueue );

    while( !mSendQueue.empty() )
    {
        Buffer* buf = mSendQueue.front();
        mSendQueue.pop_front();

        SafeDelete( buf );
    }

    SafeDelete( mRecvBuf );
}

thread_return_t TCPConnection::TCPConnectionLoop( void* arg )
{
    TCPConnection* tcpc = reinterpret_cast<TCPConnection*>( arg );
    assert( tcpc != NULL );

    THREAD_RETURN( tcpc->TCPConnectionLoop() );
}

thread_return_t TCPConnection::TCPConnectionLoop()
{
#ifdef WIN32
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );
#endif

#ifndef WIN32
    sLog.Log( "Threading", "Starting TCPConnectionLoop with thread ID %d", pthread_self() );
#endif

    mMLoopRunning.Lock();

    uint32 start = GetTickCount();
    uint32 etime;
    uint32 last_time;

    while( Process() )
    {
        /* UPDATE */
        last_time = GetTickCount();
        etime = last_time - start;

        // do the stuff for thread sleeping
        if( TCPCONN_LOOP_GRANULARITY > etime )
            Sleep( TCPCONN_LOOP_GRANULARITY - etime );

        start = GetTickCount();
    }

    mMLoopRunning.Unlock();

#ifndef WIN32
    sLog.Log( "Threading", "Ending TCPConnectionLoop with thread ID %d", pthread_self() );
#endif

    THREAD_RETURN( NULL );
}

