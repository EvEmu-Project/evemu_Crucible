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
class CryptoChallengePacket;
class Client;

class EVETCPConnection;

static const uint32 EVESocketMaxNumberOfBytes = 1000000; //this is a limit hard coded in the eve client! (NetClient.dll)
static const uint32 EVEDeflationBytesLimit = 10000;	//every packet larger than this is deflated

class EVEPresentation {
	friend class Client;	//TODO: get rid of this crap
public:
	EVEPresentation(EVETCPConnection *n, Client *c);
	virtual ~EVEPresentation();
	
	void QueuePacket(PyPacket *p);
	void FastQueuePacket(PyPacket **p);
	
	bool Connected() const;
	void Disconnect();

	PyPacket *PopPacket();

	std::string GetConnectedAddress() const;

	//hack for now:
	void SendHandshake(uint32 user_count) { m_userCount = user_count; _SendHandshake(); }
	
protected:
	void _QueueRep(const PyRep *rep);
	void _NetQueuePacket(EVENetPacket **p);
	PyPacket *_Dispatch(PyRep *r);
	void _SendHandshake();

	EVETCPConnection *const net;	//we do not own this, is never NULL
	Client *const client;	//we do not own this, never NULL; used for low-level login (see state CryptoHandshakeSent)
	CryptoChallengePacket *m_request;
	uint32 m_userCount;

	enum State {
		VersionNotReceived,
		WaitingForCommand,
		CryptoRequestNotReceived,
		CryptoRequestReceived_ChallengeWait,
		CryptoHandshakeSent,	//waiting for handshake result
		AcceptingPackets
	};
	State m_state;
};

#endif

