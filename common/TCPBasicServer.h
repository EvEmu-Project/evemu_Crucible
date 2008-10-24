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

#ifndef TCPBASICSERVER_H_
#define TCPBASICSERVER_H_

/*#include "TCPServer.h"
#include "TCPConnection.h"

class TCPBasicServer : public TCPServer<TCPConnection> {
public:
	inline TCPBasicServer(int16 iPort = 0) : TCPServer<TCPConnection>(iPort) { }
	inline virtual void CreateNewConnection(int32 ID, SOCKET in_socket, int32 irIP, int16 irPort) {
		TCPConnection *conn = new TCPConnection(ID, in_socket, irIP, irPort);
		AddConnection(conn);
	}
};*/

#endif /*TCPBASICSERVER_H_*/

