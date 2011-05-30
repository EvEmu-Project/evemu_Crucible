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

#ifndef __NETWORK__EVE_TCP_CONNECTION_H__INCL__
#define __NETWORK__EVE_TCP_CONNECTION_H__INCL__

class PyRep;
class EVETCPServer;

/**
 * @brief EVE derivation of TCP connection.
 *
 * @author Zhur, Bloody.Rabbit
 */
class EVETCPConnection
: public TCPConnection
{
    friend class EVETCPServer;

public:
    /// Time (in milliseconds) after which the connection is dropped if no data were received.
    static const uint32 TIMEOUT_MS;
    /// Hardcoded limit of packet size (NetClient.dll).
    static const uint32 PACKET_SIZE_LIMIT;

    /**
     * @brief Creates empty EVE connection.
     */
    EVETCPConnection();

    /**
     * @brief Queues given PyRep into send queue.
     *
     * @param[in] rep PyRep to be queued.
     */
    void QueueRep( const PyRep* rep );

    /**
     * @brief Pops PyRep from receive queue.
     *
     * @return Popped PyRep; NULL if nothing was received.
     */
    PyRep* PopRep();

protected:
    /**
     * @brief Creates new EVE connection from existing socket.
     *
     * @param[in] sock  Socket to be used for connection.
     * @param[in] rIP   Remote IP the socket is connected to.
     * @param[in] rPort Remote TCP port the socket is connected to.
     */
    EVETCPConnection( Socket* sock, uint32 rIP, uint16 rPort );

    bool RecvData( char* errbuf = 0 );
    bool ProcessReceivedData( char* errbuf = 0 );

    void ClearBuffers();

    /// Timer used to implement timeout.
    Timer mTimeoutTimer;

    /// Mutex to protect received data queue.
    Mutex mMInQueue;
    /// Received data queue.
    StreamPacketizer mInQueue;
};

#endif /* !__NETWORK__EVE_TCP_CONNECTION_H__INCL__ */
