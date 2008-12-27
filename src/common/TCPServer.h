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

#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#define TCPServer_ErrorBufferSize	1024

#include "common.h"
#include "Mutex.h"

//this is the non-connection type specific server.
class BaseTCPServer {
public:
	BaseTCPServer(int16 iPort = 0);
	virtual ~BaseTCPServer();

	bool	Open(int16 iPort = 0, char* errbuf = 0);			// opens the port
	void	Close();						// closes the port
	bool	IsOpen();
	inline int16	GetPort()		{ return pPort; }
	inline int32	GetNextID() { return NextID++; }

protected:
	static ThreadReturnType TCPServerLoop(void* tmp);
	
	//factory method:
	virtual void CreateNewConnection(int32 ID, SOCKET in_socket, int32 irIP, int16 irPort) = 0;
	
	
	virtual void	Process();
	bool	RunLoop();
	Mutex	MLoopRunning;
	
	void StopLoopAndWait();
	
	void	ListenNewConnections();

	int32	NextID;

	Mutex	MRunLoop;
	bool	pRunLoop;

	Mutex	MSock;
	SOCKET	sock;
	int16	pPort;

};

template<class T>
class TCPServer : public BaseTCPServer {
protected:
	typedef typename std::vector<T *> vstore;
	typedef typename std::vector<T *>::iterator vitr;
public:
	TCPServer(int16 iPort = 0)
	: BaseTCPServer(iPort) {
	}
	
	virtual ~TCPServer() {
		StopLoopAndWait();
	
		//i'm not sure what the right thing to do here is...
		//we are freeing a connection which somebody likely has a pointer to..
		//but, we really shouldn't ever get called anyhow..
		vitr cur, end;
		cur = m_list.begin();
		end = m_list.end();
		for(; cur != end; cur++) {
			delete *cur;
		}
	}
	
	T * NewQueuePop() {
		T * ret = NULL;
		MNewQueue.lock();
		if(!m_NewQueue.empty()) {
			ret = m_NewQueue.front();
			m_NewQueue.pop();
		}
		MNewQueue.unlock();
		return ret;
	}
	
protected:
	virtual void Process() {
		BaseTCPServer::Process();
		
		vitr cur;
		cur = m_list.begin();
		while(cur != m_list.end()) {
			T *data = *cur;
			if (data->IsFree() && (!data->CheckNetActive())) {
				delete data;
				cur = m_list.erase(cur);
			} else {
				if (!data->Process())
					data->Disconnect();
				cur++;
			}
		}
	}
	
	void AddConnection(T *con) {
		m_list.push_back(con);
		MNewQueue.lock();
		m_NewQueue.push(con);
		MNewQueue.unlock();
	}
	
	//queue of new connections, for the app to pull from
	Mutex	MNewQueue;
	std::queue<T *> m_NewQueue;
	
	vstore m_list;
};


#endif /*TCPSERVER_H_*/
