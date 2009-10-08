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
class EVETCPServer;

/** Time (in milliseconds) after which the connection is dropped if no data were received. */
extern const uint32 EVETCPCONN_TIMEOUT;
/** Hardcoded limit of packet size (NetClient.dll) */
extern const uint32 EVETCPCONN_PACKET_LIMIT;

/**
 * @brief EVE derivation of TCP connection.
 *
 * @author Zhur, Bloody.Rabbit
 */
class EVETCPConnection : public TCPConnection
{
    friend class EVETCPServer;
public:
    /**
     * @brief Creates empty EVE connection.
     */
	EVETCPConnection();

    /**
     * @brief Queues given PyRep into send queue.
     *
     * @param[in] rep PyRep to be queued.
     */
	void    QueueRep( const PyRep* rep );

    /**
     * @brief Pops PyRep from receive queue.
     *
     * @return Popped PyRep; NULL if nothing was received.
     */
	PyRep*  PopRep();

protected:
    /**
     * @brief Creates new EVE connection from existing socket.
     *
     * @param[in] sock  Socket to be used for connection.
     * @param[in] rIP   Remote IP the socket is connected to.
     * @param[in] rPort Remote TCP port the socket is connected to.
     */
	EVETCPConnection( Socket* sock, uint32 rIP, uint16 rPort );

	virtual bool RecvData( char* errbuf = 0 );
	virtual bool ProcessReceivedData( char* errbuf = 0 );

	virtual void ClearBuffers();

    /** Timer used to implement timeout. */
	Timer mTimeoutTimer;

    /** Mutex to protect received data queue. */
	Mutex mMInQueue;
    /** Received data queue. */
	StreamPacketizer mInQueue;
};

#endif /*EVETCPCONNECTION_H_*/
