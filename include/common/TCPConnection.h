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

#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#ifndef WIN32
	#include <pthread.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <errno.h>
	#include <fcntl.h>
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
#endif

#include "types.h"
#include "Mutex.h"
#include "queue.h"

class BaseTCPServer;
class ServerPacket;

#define TCPConnection_ErrorBufferSize	1024
#define MaxTCPReceiveBufferSize			524288


#ifndef DEF_eConnectionType
#define DEF_eConnectionType
enum eConnectionType {Incomming, Outgoing};
#endif


class TCPConnection {
protected:
	typedef enum {
		TCPS_Ready = 0,
		TCPS_Connecting = 1,
		TCPS_Connected = 100,
		TCPS_Disconnecting = 200,	//I do not know the difference between Disconnecting and Closing
		TCPS_Disconnected = 201,
		TCPS_Closing = 250,
		TCPS_Error = 255
	} State_t;
	
public:
	//socket created by a server (incoming)
	TCPConnection(int32 ID, SOCKET iSock, int32 irIP, int16 irPort);
	//socket created to connect to a server (outgoing)
	TCPConnection();	// for outgoing connections
	
	virtual ~TCPConnection();
	
	// Functions for outgoing connections
	bool			Connect(const char* irAddress, int16 irPort, char* errbuf = 0);
	virtual bool	ConnectIP(int32 irIP, int16 irPort, char* errbuf = 0);
	void			AsyncConnect(const char* irAddress, int16 irPort);
	void			AsyncConnect(int32 irIP, int16 irPort);
	virtual void	Disconnect();

	bool			Send(const uint8* data, int32 size);
	
	char*			PopLine();		//returns ownership of allocated byte array
	inline int32	GetrIP()		const	{ return rIP; }
	inline int16	GetrPort()		const	{ return rPort; }
	
	/**
	 * \brief GetAddress returns a string containing the ip and the port number
	 *
	 * @return std::string ip and port address.
	 * @note its kinda slow this way.
	 */
	std::string		GetAddress()
	{
		char address[22];

		/* "The Matrix is a system, 'Neo'. That system is our enemy. But when you're inside, you look around, what do you see?"
		* @note Aim I'm sorry :'( but I don't think this is cross platform compatible.
		*/
		uint8 *addr = (uint8*)&rIP;
		uint16 port = *(uint16*)&rPort;

		int len = snprintf(address, 21, "%u.%u.%u.%u:%d", addr[0], addr[1],addr[2],addr[3], port);

		/* snprintf will return < 0 when a error occurs so return NULL */
		if (len < 0)
			return NULL;
		return address;
	}	

	virtual State_t	GetState() const;
	inline bool		Connected()	const	{ return (GetState() == TCPS_Connected); }
	bool			ConnectReady() const;
	void			Free();		// Inform TCPServer that this connection object is no longer referanced

	inline int32	GetID()	const		{ return id; }

	bool			GetEcho();
	void			SetEcho(bool iValue);
	bool GetSockName(char *host, uint16 *port);
	
	//should only be used by TCPServer<T>:
	bool			CheckNetActive();
	inline bool		IsFree() const { return pFree; }
	virtual bool	Process();

protected:
	friend class BaseTCPServer;
	void			SetState(State_t iState);

	static ThreadReturnType TCPConnectionLoop(void* tmp);
//	SOCKET			sock;
	bool			RunLoop();
	Mutex			MLoopRunning;
	Mutex	MAsyncConnect;
	bool	GetAsyncConnect();
	bool	SetAsyncConnect(bool iValue);
	char*	charAsyncConnect;
	bool	pAsyncConnect;	//this flag should really be turned into a state instead.

	virtual bool ProcessReceivedData(char* errbuf = 0);
	virtual bool SendData(bool &sent_something, char* errbuf = 0);
	virtual bool RecvData(char* errbuf = 0);
	virtual int _doConnect(int fd, struct sockaddr_in *addr, int addr_len);
	
	virtual void ClearBuffers();

	
	bool m_previousLineEnd;

	eConnectionType	ConnectionType;
	Mutex	MRunLoop;
	bool	pRunLoop;

	SOCKET	connection_socket;
	int32	id;
	int32	rIP;
	int16	rPort; // host byte order
	bool	pFree;
	
	mutable Mutex	MState;
	State_t	pState;
	
	//text based line out queue.
	Mutex MLineOutQueue;
	virtual bool	LineOutQueuePush(char* line);	//this is really kinda a hack for the transition to packet mode. Returns true to stop processing the output.
	MyQueue<char> LineOutQueue;
	
	uint8*	recvbuf;
	int32	recvbuf_size;
	int32	recvbuf_used;
	
	int32	recvbuf_echo;
	volatile bool	pEcho;
	
	Mutex	MSendQueue;
	uint8*	sendbuf;
	int32	sendbuf_size;
	int32	sendbuf_used;
	bool	ServerSendQueuePop(uint8** data, int32* size);
	bool	ServerSendQueuePopForce(uint8** data, int32* size);		//does a lock() instead of a trylock()
	
	void	ServerSendQueuePushEnd(const uint8* head_data, int32 head_size, const uint8* data, int32 size);
	void	ServerSendQueuePushEnd(const uint8* data, int32 size);
	void	ServerSendQueuePushEnd(const uint8* head_data, int32 head_size, uint8** data, int32 size);
	void	ServerSendQueuePushEnd(uint8** data, int32 size);
	void	ServerSendQueuePushFront(uint8* data, int32 size);
	
private:
	void FinishDisconnect();
};


#endif



