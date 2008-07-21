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


#ifndef EVE_PRESENTATION_H
#define EVE_PRESENTATION_H

class PyPacket;
class PyRep;
class VersionExchange;

#include "EVETCPConnection.h"

static const uint32 EVESocketMaxNumberOfBytes = 1000000; //this is a limit hard coded in the eve client! (NetClient.dll)

class EVEPresentation {
public:
	EVEPresentation(EVETCPConnection *net);
	virtual ~EVEPresentation();
	
	bool Process();
	
	void QueuePacket(PyPacket *p);
	void FastQueuePacket(PyPacket **p);
	
	bool Connected() const;
	void Disconnect();
	
	PyPacket *PopPacket();

	std::string GetConnectedAddress() const;

	//hack for now:
	void SendHandshake(uint32 macho_version, uint32 user_count, double version_number, uint16 build_version, const char *project_version);
	
protected:
	//virtual void FastQueueRaw(PyRep **rep);
	void _QueueRep(const PyRep *rep);
	virtual void _NetQueuePacket(EVENetPacket **p);
	EVETCPConnection *const net;	//we do not own this, is never NULL

	VersionExchange *m_version;	//only a pointer to obscure the type

#ifdef DRAGON
	typedef enum {
		VersionNotReceived,
		CryptoNotNegotiated,
		CryptoRequestReceived_ChallengeWait,
		CryptoHandshakeSent,	//waiting for handshake result
		CryptoActive
		
	} CryptoState;
	CryptoState m_state;
#endif
};


#endif

