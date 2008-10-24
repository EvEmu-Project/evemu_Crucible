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

//client side
EVETCPConnection::EVETCPConnection()
: TCPConnection(),
  timeout_timer(EVE_TCP_SERVER_TIMEOUT_MS)
{
	Server = NULL;
}


//server side case
EVETCPConnection::EVETCPConnection(int32 ID, EVETCPServer* iServer, SOCKET in_socket, int32 irIP, int16 irPort)
: TCPConnection(ID, in_socket, irIP, irPort),
  timeout_timer(EVE_TCP_SERVER_TIMEOUT_MS)
{
	Server = NULL;	
}

EVETCPConnection::~EVETCPConnection() {
	//the queues free their content right now I believe.
}

void EVETCPConnection::QueuePacket(EVENetPacket *pack) {
	pack = pack->Clone();
	FastQueuePacket(&pack);
}

//consumes the supplied packet!
void EVETCPConnection::FastQueuePacket(EVENetPacket **pack) {
	uint32 length = (*pack)->length;
	ServerSendQueuePushEnd((const byte *) &length, sizeof(length), &(*pack)->data, (*pack)->length);
	delete *pack;
}

EVENetPacket* EVETCPConnection::PopPacket() {
	EVENetPacket* ret;
	if (!MInQueue.trylock())
		return NULL;
	ret = InQueue.PopPacket();
	MInQueue.unlock();
	return ret;
}

void EVETCPConnection::ClearBuffers() {
	TCPConnection::ClearBuffers();
	
	LockMutex lock2(&MOutQueueLock);
	EVENetPacket* pack = 0;
	while ((pack = OutQueue.pop()))
		safe_delete(pack);
	
	MInQueue.lock();
		timeout_timer.Start();
		
		InQueue.ClearBuffers();
	MInQueue.unlock();
	
}

bool EVETCPConnection::ProcessReceivedData(char* errbuf) {
	if (errbuf)
		errbuf[0] = 0;
	if (!recvbuf)
		return true;
	
	MInQueue.lock();
		timeout_timer.Start();
		
		InQueue.InputBytes(recvbuf, recvbuf_used);
		if(recvbuf_size > 1024) {
			//if the recvbuf grows too large, kill it off and start over
			//this is purely an optimization, it could be killed each time or never...
			safe_delete_array(recvbuf);
			recvbuf_size = 0;
		}
		recvbuf_used = 0;
	MInQueue.unlock();
	
	return(true);
}

bool EVETCPConnection::RecvData(char* errbuf) {
	if(!TCPConnection::RecvData(errbuf)) {
		MInQueue.lock();
		bool data_to_send = (OutQueue.count() > 0);
		MInQueue.unlock();
		
		return(data_to_send);
	}
	
	if (timeout_timer.Check()) {
		if (errbuf)
			snprintf(errbuf, TCPConnection_ErrorBufferSize, "TCPConnection::RecvData(): Connection timeout");
		return false;
	}
	
	return(true);
}
