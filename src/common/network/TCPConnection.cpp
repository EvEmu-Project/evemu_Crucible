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

#include "CommonPCH.h"

#include "log/logsys.h"
#include "log/LogNew.h"
#include "network/TCPConnection.h"
#include "network/NetUtils.h"
#include "utils/timer.h"

#ifdef FREEBSD //Timothy Whitman - January 7, 2003
#   define MSG_NOSIGNAL 0
#endif

const uint32 TCPCONN_RECVBUF_LIMIT = 524288;
const uint32 TCPCONN_LOOP_GRANULARITY = 5;

#ifdef WIN32
InitWinsock winsock;
#endif

TCPConnection::TCPConnection()
: mSock( NULL ),
  mSockState( STATE_DISCONNECTED ),
  mrIP( 0 ),
  mrPort( 0 ),
  mRecvBuf( NULL ),
  mRecvBufSize( 0 ),
  mRecvBufUsed( 0 ),
  mSendBuf( NULL ),
  mSendBufSize( 0 ),
  mSendBufUsed( 0 )
{
}

TCPConnection::TCPConnection( Socket* socket, uint32 mrIP, uint16 mrPort )
: mSock( socket ),
  mSockState( STATE_CONNECTED ),
  mrIP( mrIP ),
  mrPort( mrPort ),
  mRecvBuf( NULL ),
  mRecvBufSize( 0 ),
  mRecvBufUsed( 0 ),
  mSendBuf( NULL ),
  mSendBufSize( 0 ),
  mSendBufUsed( 0 )
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

TCPConnection::state_t TCPConnection::GetState() const
{
	state_t ret;

	mMSock.lock();
	ret = mSockState;
	mMSock.unlock();

	return ret;
}

bool TCPConnection::Connect( uint32 rIP, uint16 rPort, char* errbuf )
{
	if( errbuf )
		errbuf[0] = 0;

    LockMutex lock( &mMSock );

    state_t oldState = GetState();
    if( oldState == STATE_DISCONNECTED )
    {
        mMSock.unlock();

        // Wait for working thread to stop.
        WaitLoop();

        mMSock.lock();
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

	SetState( STATE_CONNECTED );

    // Start processing thread if necessary
    if( oldState == STATE_DISCONNECTED )
        StartLoop();

	return true;
}

void TCPConnection::AsyncConnect( uint32 rIP, uint16 rPort )
{
    // Changing state; acquire mutex
    LockMutex lock( &mMSock );

    state_t state = GetState();
    if( state == STATE_DISCONNECTED )
    {
        mMSock.unlock();

        // Wait for working thread to stop.
        WaitLoop();

        mMSock.lock();
    }

    state = GetState();
	if( state != STATE_DISCONNECTED )
		return;

    mrIP = rIP;
    mrPort = rPort;

    SetState( STATE_CONNECTING );

    // Start processing thread
    StartLoop();
}

void TCPConnection::Disconnect()
{
    LockMutex lock( &mMSock );

    state_t state = GetState();
    if( state != STATE_CONNECTING && state != STATE_CONNECTED )
        return;

    // Change state
    SetState( STATE_DISCONNECTING );
}

bool TCPConnection::Send( const uint8* data, uint32 size )
{
    LockMutex lock( &mMSock );

    state_t state = GetState();
	if( state != STATE_CONNECTED )
		return false;

    ServerSendQueuePushEnd( data, size );
	return true;
}

void TCPConnection::SetState( state_t state )
{
	mMSock.lock();
    mSockState = state;
	mMSock.unlock();
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
    mMLoopRunning.lock();
    mMLoopRunning.unlock();
}

/* This is always called from an IO thread. Either the server socket's thread, or a 
 * special thread we create when we make an outbound connection. */
bool TCPConnection::Process()
{
	char errbuf[ TCPCONN_ERRBUF_SIZE ];

    LockMutex lock( &mMSock );
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
            bool sent_something = false;
		    if( !SendData( sent_something, errbuf ) )
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
            bool sent_something = false;
		    if( !SendData( sent_something, errbuf ) )
            {
                sLog.Error( "TCPConnection", "%s: %s.", GetAddress().c_str(), errbuf );

                DoDisconnect();
                return false;
		    }

            // We assume SendData emptied send queue

            // Send queue is empty, disconnect
            DoDisconnect();
            return true;
	    }
    }
}

bool TCPConnection::SendData( bool& sent_something, char* errbuf )
{
    sent_something = false;
	if( errbuf )
		errbuf[0] = 0;

    LockMutex lock( &mMSock );

    state_t state = GetState();
    if( state != STATE_CONNECTED && state != STATE_DISCONNECTING )
        return false;

	/************ Get first send packet on queue and send it! ************/
	uint8* data = 0;
	uint32 size = 0;
	if( ServerSendQueuePop( &data, &size ) )
    {
#ifdef WIN32
		int status = mSock->send( data, size, 0 );
#else
		int status = mSock->send( data, size, MSG_NOSIGNAL );
		if( errno == EPIPE )
            status = SOCKET_ERROR;
#endif

		if( status >= 0 )
        {
			if( (uint32)status > size )
            {
                if( errbuf )
			        snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "TCPConnection::SendData(): WTF! status > size." );

				return false;
			}
            else if( (uint32)status < size )
                ServerSendQueuePushFront( &data[ status ], size - status );

			sent_something = ( status > 0 );
		}
		else if( status == SOCKET_ERROR )
        {
#ifdef WIN32
			if( WSAGetLastError() != WSAEWOULDBLOCK )
#else
			if( errno != EWOULDBLOCK )
#endif
			{
				if (errbuf)
#ifdef WIN32
                    snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "TCPConnection::SendData(): send(): Errorcode: %u", WSAGetLastError() );
#else
					snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "TCPConnection::SendData(): send(): Errorcode: %s", strerror( errno ) );
#endif
				
				return false;
			}
		}

		SafeDeleteArray( data );
	}

	return true;
}

bool TCPConnection::RecvData( char* errbuf )
{
	if( errbuf != NULL )
		errbuf[0] = 0;

    LockMutex lock( &mMSock );

    state_t state = GetState();
    if( state != STATE_CONNECTED && state != STATE_DISCONNECTING )
        return false;

	if( mRecvBuf == NULL )
    {
		mRecvBuf = new uint8[ 5120 ];
		mRecvBufSize = 5120;
		mRecvBufUsed = 0;
	}
	else if( ( mRecvBufSize - mRecvBufUsed ) < 2048 )
	{
		uint8* tmpbuf = new uint8[ mRecvBufSize + 5120 ];
		memcpy( tmpbuf, mRecvBuf, mRecvBufUsed );

		SafeDeleteArray( mRecvBuf );
		mRecvBuf = tmpbuf;
		mRecvBufSize += 5120;

		if( mRecvBufSize >= TCPCONN_RECVBUF_LIMIT )
        {
			if( errbuf )
				snprintf( errbuf, TCPCONN_ERRBUF_SIZE, "TCPConnection::RecvData(): mRecvBufSize >= TCPCONN_RECVBUF_LIMIT" );

			return false;
		}
	}

    int status = mSock->recv( &mRecvBuf[ mRecvBufUsed ], ( mRecvBufSize - mRecvBufUsed ), 0 );
    if( status > 0 )
    {
		mRecvBufUsed += status;

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
		if ( WSAGetLastError() != WSAEWOULDBLOCK )
#else
		if ( errno == EWOULDBLOCK )
#endif
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

	return true;
}

void TCPConnection::DoDisconnect()
{
    LockMutex lock( &mMSock );

    state_t state = GetState();
    if( state != STATE_CONNECTED && state != STATE_DISCONNECTING )
        return;

    SafeDelete( mSock );
	mrIP = mrPort = 0;
	ClearBuffers();

    SetState( STATE_DISCONNECTED );
}

void TCPConnection::ClearBuffers()
{
	SafeDeleteArray( mRecvBuf );
    mRecvBufSize = mRecvBufUsed = 0;

	LockMutex lock( &mMSendQueue );
	SafeDeleteArray( mSendBuf );
    mSendBufSize = mSendBufUsed = 0;
}

ThreadReturnType TCPConnection::TCPConnectionLoop( void* arg )
{
	TCPConnection* tcpc = reinterpret_cast<TCPConnection*>( arg );
    assert( tcpc != NULL );

    return tcpc->TCPConnectionLoop();
}

ThreadReturnType TCPConnection::TCPConnectionLoop()
{
#ifdef WIN32
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );
#endif

#ifndef WIN32
	sLog.Log( "Threading", "Starting TCPConnectionLoop with thread ID %d", pthread_self() );
#endif

    mMLoopRunning.lock();

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

	mMLoopRunning.unlock();
	
#ifndef WIN32
	sLog.Log( "Threading", "Ending TCPConnectionLoop with thread ID %d", pthread_self() );
#endif
	
	THREAD_RETURN( NULL );
}

//TODO: this send queue could be rewritten to be an actual queue
//to avoid all these memcpy's
void TCPConnection::ServerSendQueuePushEnd( const uint8* head_data, uint32 head_size, const uint8* data, uint32 data_size )
{
	uint32 size = head_size + data_size;

	mMSendQueue.lock();
	if( mSendBuf == NULL )
    {
		mSendBuf = new uint8[size+128];
		mSendBufSize = size+128;
		mSendBufUsed = 0;
	}
	else if ( ( size + mSendBufUsed ) > mSendBufSize )
    {
		mSendBufSize += size + 1024;
		uint8* tmp = new uint8[mSendBufSize];
		memcpy(tmp, mSendBuf, mSendBufUsed);
		SafeDeleteArray(mSendBuf);
		mSendBuf = tmp;
	}

	memcpy(&mSendBuf[mSendBufUsed], head_data, head_size);
	mSendBufUsed += head_size;

	memcpy(&mSendBuf[mSendBufUsed], data, data_size);
	mSendBufUsed += data_size;

	mMSendQueue.unlock();
}


void TCPConnection::ServerSendQueuePushEnd(const uint8* head_data, uint32 head_size, uint8** data, uint32 data_size) {
	uint32 size = head_size + data_size;
	//This is not taking any advantage of the fact that we consume `data` until we put a real queue in here.
	//
	mMSendQueue.lock();
	if (mSendBuf == NULL)
	{
		mSendBuf = new uint8[size+128];
		mSendBufSize = size+128;
		mSendBufUsed = 0;
	}
	else if (size > (mSendBufSize - mSendBufUsed))
	{
		mSendBufSize += size + 1024;
		uint8* tmp = new uint8[mSendBufSize];
		memcpy(tmp, mSendBuf, mSendBufUsed);
		SafeDeleteArray(mSendBuf);
		mSendBuf = tmp;
	}
	memcpy(&mSendBuf[mSendBufUsed], head_data, head_size);
	mSendBufUsed += head_size;
	memcpy(&mSendBuf[mSendBufUsed], *data, data_size);
	mSendBufUsed += data_size;
	mMSendQueue.unlock();
	SafeDeleteArray(*data);
}

void TCPConnection::ServerSendQueuePushEnd( const uint8* data, uint32 size )
{
    if( size == 0 )
        return;

	mMSendQueue.lock();
	if (mSendBuf == NULL) {
		mSendBuf = new uint8[size+128];
		mSendBufSize = size+128;
		mSendBufUsed = 0;
	}
	else if (size > (mSendBufSize - mSendBufUsed)) {
		mSendBufSize += size + 1024;
		uint8* tmp = new uint8[mSendBufSize];
		memcpy(tmp, mSendBuf, mSendBufUsed);
		SafeDeleteArray(mSendBuf);
		mSendBuf = tmp;
	}
	memcpy(&mSendBuf[mSendBufUsed], data, size);
	mSendBufUsed += size;
	mMSendQueue.unlock();
}

void TCPConnection::ServerSendQueuePushEnd(uint8** data, uint32 size) {
	mMSendQueue.lock();
	if (mSendBuf == NULL) {
		mSendBuf = *data;
		mSendBufSize = size;
		mSendBufUsed = size;
		mMSendQueue.unlock();
		*data = NULL;
		return;
	}
	if (size > (mSendBufSize - mSendBufUsed)) {
		mSendBufSize += size;
		uint8* tmp = new uint8[mSendBufSize];
		memcpy(tmp, mSendBuf, mSendBufUsed);
		SafeDeleteArray(mSendBuf);
		mSendBuf = tmp;
	}
	memcpy(&mSendBuf[mSendBufUsed], *data, size);
	mSendBufUsed += size;
	mMSendQueue.unlock();
	SafeDeleteArray(*data);
}

void TCPConnection::ServerSendQueuePushFront(const uint8* data, uint32 size) {
	mMSendQueue.lock();
	if (mSendBuf == 0) {
		mSendBuf = new uint8[size];
		mSendBufSize = size;
		mSendBufUsed = 0;
	}
	else if (size > (mSendBufSize - mSendBufUsed)) {
		mSendBufSize += size;
		uint8* tmp = new uint8[mSendBufSize];
		memcpy(&tmp[size], mSendBuf, mSendBufUsed);
		SafeDeleteArray(mSendBuf);
		mSendBuf = tmp;
	}
	memcpy(mSendBuf, data, size);
	mSendBufUsed += size;
	mMSendQueue.unlock();
}

bool TCPConnection::ServerSendQueuePop(uint8** data, uint32* size) {
	bool ret;
	if (!mMSendQueue.trylock())
		return false;
	if (mSendBuf) {
		*data = mSendBuf;
		*size = mSendBufUsed;
		mSendBuf = 0;
		ret = true;
	}
	else {
		ret = false;
	}
	mMSendQueue.unlock();
	return ret;
}

bool TCPConnection::ServerSendQueuePopForce(uint8** data, uint32* size) {
	bool ret;
	mMSendQueue.lock();
	if (mSendBuf) {
		*data = mSendBuf;
		*size = mSendBufUsed;
		mSendBuf = 0;
		ret = true;
	}
	else {
		ret = false;
	}
	mMSendQueue.unlock();
	return ret;
}

