/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "EvemuPCH.h"

#ifdef WIN32
	#include <process.h>
#else
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <errno.h>
	#include <fcntl.h>
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
#endif


#define SERVER_LOOP_GRANULARITY 3	//# of ms between checking our socket/queues

BaseTCPServer::BaseTCPServer(int16 in_port) {
	NextID = 1;
	pPort = in_port;
	sock = 0;
	pRunLoop = true;
#ifdef WIN32
	_beginthread(BaseTCPServer::TCPServerLoop, 0, this);
#else
	pthread_t thread;
	pthread_create(&thread, NULL, &BaseTCPServer::TCPServerLoop, this);
#endif
}

BaseTCPServer::~BaseTCPServer() {
	StopLoopAndWait();
}

void BaseTCPServer::StopLoopAndWait() {
	MRunLoop.lock();
	if(pRunLoop) {
		pRunLoop = false;
		MRunLoop.unlock();
		//wait for loop to stop.
		MLoopRunning.lock();
		MLoopRunning.unlock();
	} else {
		MRunLoop.unlock();
	}
}

bool BaseTCPServer::RunLoop() {
	bool ret;
	MRunLoop.lock();
	ret = pRunLoop;
	MRunLoop.unlock();
	return ret;
}

ThreadReturnType BaseTCPServer::TCPServerLoop(void* tmp) {
#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
#endif
	if (tmp == 0) {
//		ThrowError("BaseTCPServerLoop(): tmp = 0!");
		THREAD_RETURN(NULL);
	}
	BaseTCPServer* tcps = (BaseTCPServer*) tmp;
	
#ifndef WIN32
	_log(COMMON__THREADS, "Starting TCPServerLoop with thread ID %d", pthread_self());
#endif
	
	tcps->MLoopRunning.lock();
	while (tcps->RunLoop()) {
//		_CP(BaseTCPServerLoop);
		Sleep(SERVER_LOOP_GRANULARITY);
		tcps->Process();
	}
	tcps->MLoopRunning.unlock();
	
#ifndef WIN32
	_log(COMMON__THREADS, "Ending TCPServerLoop with thread ID %d", pthread_self());
#endif
	
	THREAD_RETURN(NULL);
}

void BaseTCPServer::Process() {
	ListenNewConnections();
}

void BaseTCPServer::ListenNewConnections() {
    SOCKET tmpsock;
    struct sockaddr_in	from;
    struct in_addr	in;
    unsigned int	fromlen;
    unsigned short	port;
    
    from.sin_family = AF_INET;
    fromlen = sizeof(from);
	LockMutex lock(&MSock);
	if (!sock)
		return;

	// Check for pending connects
#ifdef WIN32
	unsigned long nonblocking = 1;
	while ((tmpsock = accept(sock, (struct sockaddr*) &from, (int *) &fromlen)) != INVALID_SOCKET) {
		ioctlsocket (tmpsock, FIONBIO, &nonblocking);
#else
#ifdef __CYGWIN__
	while ((tmpsock = accept(sock, (struct sockaddr *) &from, (int *) &fromlen)) != INVALID_SOCKET) {
#else
	while ((tmpsock = accept(sock, (struct sockaddr*) &from, &fromlen)) != INVALID_SOCKET) {
#endif
		fcntl(tmpsock, F_SETFL, O_NONBLOCK);
#endif
		int bufsize = 64 * 1024; // 64kbyte receive buffer, up from default of 8k
		setsockopt(tmpsock, SOL_SOCKET, SO_RCVBUF, (char*) &bufsize, sizeof(bufsize));
		port = from.sin_port;
		in.s_addr = from.sin_addr.s_addr;
		
		// New TCP connection, this must consume the socket.
		CreateNewConnection(GetNextID(), tmpsock, in.s_addr, ntohs(from.sin_port));
	}
}

bool BaseTCPServer::Open(int16 in_port, char* errbuf) {
	
	if (errbuf != NULL)
	{
		errbuf[0] = 0;
	}

	// mutex lock
	LockMutex lock(&MSock);
	if (sock != 0) 
	{
		if (errbuf != NULL)
		{
			snprintf(errbuf, TCPServer_ErrorBufferSize, "Listening socket already open");
		}
		return false;
	}

	if (in_port != 0) {
		pPort = in_port;
	}

#ifdef WIN32
	SOCKADDR_IN address;
	unsigned long nonblocking = 1;
#else
	struct sockaddr_in address;
#endif
	int reuse_addr = 1;

//	Setup internet address information.  
//	This is used with the bind() call
	memset((char *) &address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(pPort);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

//	Setting up TCP port for new TCP connections
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		if (errbuf)
			snprintf(errbuf, TCPServer_ErrorBufferSize, "socket(): INVALID_SOCKET");
		return false;
	}

// Quag: don't think following is good stuff for TCP, good for UDP
// Mis: SO_REUSEADDR shouldn't be a problem for tcp--allows you to restart
// without waiting for conn's in TIME_WAIT to die
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse_addr, sizeof(reuse_addr));

	if (bind(sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
#ifdef WIN32
		closesocket(sock);
#else
		close(sock);
#endif
		sock = 0;
		if (errbuf)
			sprintf(errbuf, "bind(): <0");
		return false;
	}

	int bufsize = 64 * 1024; // 64kbyte receive buffer, up from default of 8k
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*) &bufsize, sizeof(bufsize));
#ifdef WIN32
	ioctlsocket (sock, FIONBIO, &nonblocking);
#else
	fcntl(sock, F_SETFL, O_NONBLOCK);
#endif

	if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
#ifdef WIN32
		closesocket(sock);
		if (errbuf)
			snprintf(errbuf, TCPServer_ErrorBufferSize, "listen() failed, Error: %d", WSAGetLastError());
#else
		close(sock);
		if (errbuf)
			snprintf(errbuf, TCPServer_ErrorBufferSize, "listen() failed, Error: %s", strerror(errno));
#endif
		sock = 0;
		return false;
	}

	return true;
}

void BaseTCPServer::Close() {
	StopLoopAndWait();
	
	LockMutex lock(&MSock);
	if (sock) {
#ifdef WIN32
		closesocket(sock);
#else
		close(sock);
#endif
	}
	sock = 0;
}

bool BaseTCPServer::IsOpen() {
	MSock.lock();
	bool ret = (bool) (sock != 0);
	MSock.unlock();
	return ret;
}
