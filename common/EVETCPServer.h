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

#ifndef EVETCPSERVER_H_
#define EVETCPSERVER_H_

#include "TCPServer.h"

class EVETCPConnection;
struct EVETCPNetPacket_Struct;
class ServerPacket;

class EVETCPServer : public TCPServer<EVETCPConnection> {
public:
	EVETCPServer(int16 iPort = 0);
	virtual ~EVETCPServer();
	
protected:
	//virtual void	Process();
	
	//factory method, really should be redone!
	virtual void CreateNewConnection(int32 ID, SOCKET in_socket, int32 irIP, int16 irPort);
};
#endif /*EVETCPSERVER_H_*/
