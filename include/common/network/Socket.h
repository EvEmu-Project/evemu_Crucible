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
	Author:		Bloody.Rabbit
*/

#ifndef __SOCKET_H__INCL__
#define __SOCKET_H__INCL__

/**
 * @brief Simple wrapper for sockets.
 *
 * @author Bloody.Rabbit
 */
class Socket
{
public:
    Socket( int af, int type, int protocol );
    ~Socket();

    int connect( const sockaddr* name, unsigned int namelen );

    unsigned int recv( void* buf, unsigned int len, int flags );
    unsigned int recvfrom( void* buf, unsigned int len, int flags, sockaddr* from, unsigned int* fromlen );
    unsigned int send( const void* buf, unsigned int len, int flags );
    unsigned int sendto( const void* buf, unsigned int len, int flags, const sockaddr* to, unsigned int tolen );

    int bind( const sockaddr* name, unsigned int namelen );
    int listen( int backlog = SOMAXCONN );

    Socket* accept( sockaddr* addr, unsigned int* addrlen );

    int setopt( int level, int optname, const void* optval, unsigned int optlen );
#ifdef WIN32
    int ioctl( long cmd, unsigned long* argp );
#else
    int fcntl( int cmd, long arg );
#endif /* !WIN32 */

protected:
    Socket( SOCKET sock );

    SOCKET mSock;
};

#endif /* !__SOCKET_H__INCL__ */
