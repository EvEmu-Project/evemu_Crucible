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
    Author:        Bloody.Rabbit
*/

#include "eve-core.h"

#include "network/Socket.h"

Socket::Socket( int af, int type, int protocol )
: mSock( INVALID_SOCKET )
{
    // Create new socket
    mSock = ::socket( af, type, protocol );

    // Verify socket
    assert( mSock != INVALID_SOCKET );
}

Socket::Socket( SOCKET sock )
: mSock( sock )
{
    // Verify socket
    assert( mSock != INVALID_SOCKET );
}

Socket::~Socket()
{
    ::shutdown( mSock, 0x01 );
    ::shutdown( mSock, 0x00 );

#ifdef HAVE_WINSOCK2_H
    ::closesocket( mSock );
#else /* !HAVE_WINSOCK2_H */
    ::close( mSock );
#endif /* !HAVE_WINSOCK2_H */
}

int Socket::connect( const sockaddr* name, unsigned int namelen )
{
    return ::connect( mSock, name, namelen );
}

unsigned int Socket::recv( void* buf, unsigned int len, int flags )
{
    return ::recv( mSock, (char*)buf, len, flags );
}

unsigned int Socket::recvfrom( void* buf, unsigned int len, int flags, sockaddr* from, unsigned int* fromlen )
{
#ifdef HAVE_WINSOCK2_H
    return ::recvfrom( mSock, (char*)buf, len, flags, from, (int*)fromlen );
#else /* !HAVE_WINSOCK2_H */
    return ::recvfrom( mSock, buf, len, flags, from, fromlen );
#endif /* !HAVE_WINSOCK2_H */
}

unsigned int Socket::send( const void* buf, unsigned int len, int flags )
{
    return ::send( mSock, (const char*)buf, len, flags );
}

unsigned int Socket::sendto( const void* buf, unsigned int len, int flags, const sockaddr* to, unsigned int tolen )
{
    return ::sendto( mSock, (const char*)buf, len, flags, to, tolen );
}

int Socket::bind( const sockaddr* name, unsigned int namelen )
{
    return ::bind( mSock, name, namelen );
}

int Socket::listen( int backlog )
{
    return ::listen( mSock, backlog );
}

Socket* Socket::accept( sockaddr* addr, unsigned int* addrlen )
{
#ifdef HAVE_WINSOCK2_H
    SOCKET sock = ::accept( mSock, addr, (int*)addrlen );
#else /* !HAVE_WINSOCK2_H */
    SOCKET sock = ::accept( mSock, addr, addrlen );
#endif /* !HAVE_WINSOCK2_H */

    if( sock != INVALID_SOCKET )
        return new Socket( sock );
    else
        return NULL;
}

int Socket::setopt( int level, int optname, const void* optval, unsigned int optlen )
{
    return ::setsockopt( mSock, level, optname, (const char*)optval, optlen );
}

#ifdef HAVE_WINSOCK2_H
int Socket::ioctl( long cmd, unsigned long* argp )
{
    return ::ioctlsocket( mSock, cmd, argp );
}
#else /* !HAVE_WINSOCK2_H */
int Socket::fcntl( int cmd, long arg )
{
    return ::fcntl( mSock, cmd, arg );
}
#endif /* !HAVE_WINSOCK2_H */
