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

static const byte handshakeFunc[] = {
	0x74, 0x04, 0x00, 0x00, 0x00, 0x4E, 0x6F, 0x6E, 0x65	//marshaled Python string "None"
};

EVEPresentation::EVEPresentation(
	EVETCPConnection *n,
	Client *c)
: net(n),
  client(c),
  m_state(VersionNotReceived),
  m_request(NULL),
  m_userCount(0)
{
	//do not touch client here, its not fully constructed
}

EVEPresentation::~EVEPresentation() {
	delete m_request;
	//let the server thread know we are done with this connection
	//net->Free();
}

bool EVEPresentation::Connected() const {
	return(net->Connected());
}

void EVEPresentation::Disconnect() {
	net->Disconnect();
}

PyPacket *EVEPresentation::PopPacket() {
	EVENetPacket *netp = net->PopPacket();
	if(netp == NULL)
		return(NULL);	//nothing to get yet.

	if(netp->length > EVESocketMaxNumberOfBytes) {
		delete netp;
		_log(NET__PRES_ERROR, "%s: Received invalid version exchange!", GetConnectedAddress().c_str());
		return(NULL);
	}
	
	//take the raw packet and turn it into a PyRep
	PyRep *r = InflateAndUnmarshal(netp->data, netp->length);
	delete netp;
	if(r == NULL)
		return(NULL);	//failed to inflate or unmarshal the packet

	if(is_log_enabled(NET__PRES_REP)) {
		_log(NET__PRES_REP, "%s: Raw Rep Dump:", GetConnectedAddress().c_str());
		r->Dump(NET__PRES_REP, "    ");
	}

	try {
		return(_Dispatch(r));
	} catch(PyException &e) {
		//send it raw
		PyRep *payload = e.ssException.hijack();
		_QueueRep(payload);
		delete payload;
		//do recurse
		return(PopPacket());
	}
}


void EVEPresentation::QueuePacket(PyPacket *p) {
	if(p == NULL)
		return;
	p = p->Clone();
	FastQueuePacket(&p);
}

void EVEPresentation::FastQueuePacket(PyPacket **p) {
	if(p == NULL || *p == NULL)
		return;
	
	PyRep *r = (*p)->Encode();
	if(r == NULL) {
		_log(NET__PRES_ERROR, "%s: Failed to encode???", GetConnectedAddress().c_str());
		delete *p;
		*p = NULL;
		return;
	}
	delete *p;
	*p = NULL;

	_QueueRep(r);
	delete r;
}

void EVEPresentation::_QueueRep(const PyRep *rep) {
	if(is_log_enabled(NET__PRES_REP_OUT)) {
		_log(NET__PRES_REP_OUT, "%s: Outbound Rep:", GetConnectedAddress().c_str());
		PyLogsysDump dv(NET__PRES_REP_OUT);
		rep->visit(&dv);
	}
	
	EVENetPacket *packet = new EVENetPacket;
	packet->data = Marshal(rep, packet->length);
	if(packet->data == NULL) {
		_log(NET__PRES_ERROR, "%s: Error marshaling packet!", GetConnectedAddress().c_str());
		return;
	}

	if(packet->length > EVEDeflationBytesLimit) {
		//packet large enough, deflate it
		uint32 deflen = packet->length;
		byte *deflated = DeflatePacket(packet->data, deflen);

		if(deflen != 0 && deflated != NULL) {
			//deflation successfull
			_log(NET__PRES_TRACE, "%s: Successfully deflated packet from length %lu to length %lu (%.02f).", GetConnectedAddress().c_str(), packet->length, deflen, double(deflen)/double(packet->length));

			delete[] packet->data;
			packet->length = deflen;
			packet->data = deflated;
		} else
			_log(NET__PRES_ERROR, "%s: Failed to deflate packet of length %lu, sending uncompressed.", GetConnectedAddress().c_str(), packet->length);
	}

	_NetQueuePacket(&packet);
}

void EVEPresentation::_NetQueuePacket(EVENetPacket **p) {
	if((*p)->length > EVESocketMaxNumberOfBytes) {
		_log(NET__PRES_ERROR, "%s: tried to queue a packet which is too large! %lu exceeds the hard coded limit of %lu bytes!", GetConnectedAddress().c_str(), (*p)->length, EVESocketMaxNumberOfBytes);
		delete *p;
		*p = NULL;
		return;
	}
	
	_log(NET__PRES_RAW_OUT, "Outbound Raw Packet:");
	_hex(NET__PRES_RAW_OUT, (*p)->data, (*p)->length);
	
	net->FastQueuePacket(p);
}

PyPacket *EVEPresentation::_Dispatch(PyRep *r) {
	switch(m_state) {
		case VersionNotReceived: {
			//we are waiting for their version information...
			VersionExchange ve;
			if(!ve.Decode(&r)) {
				_log(NET__PRES_ERROR, "%s: Received invalid version exchange!", GetConnectedAddress().c_str());
				break;
			}

			_log(NET__PRES_REP, "%s: Received Low Level Version Exchange:\n", GetConnectedAddress().c_str());
			ve.Dump(NET__PRES_REP, "    ");

			if(ve.birthday != EVEBirthday) {
				_log(NET__PRES_ERROR, "%s: Client's birthday does not match ours!", GetConnectedAddress().c_str());
			}
			if(ve.macho_version != MachoNetVersion) {
				_log(NET__PRES_ERROR, "%s: Client's macho_version not match ours!", GetConnectedAddress().c_str());
			}
			if(ve.version_number != EVEVersionNumber) {
				_log(NET__PRES_ERROR, "%s: Client's version_number not match ours!", GetConnectedAddress().c_str());
			}
			if(ve.build_version != EVEBuildVersion) {
				_log(NET__PRES_ERROR, "%s: Client's build_version not match ours!", GetConnectedAddress().c_str());
			}
			if(ve.project_version != EVEProjectVersion) {
				_log(NET__PRES_ERROR, "%s: Client's project_version not match ours!", GetConnectedAddress().c_str());
			}

			m_state = WaitingForCommand;
		}	break;

		case WaitingForCommand: {
			//check if it actually is tuple
			if(!r->CheckType(PyRep::Tuple)) {
				_log(NET__PRES_ERROR, "%s: Invalid packet during waiting for command (tuple expected).", GetConnectedAddress().c_str());
				break;
			}
			PyRepTuple *t = (PyRepTuple *)r;
			//decode
			if(t->items.size() == 2) {
				//QC = Queue Check
				NetCommand_QC cmd;
				if(!cmd.Decode(&t)) {
					_log(NET__PRES_ERROR, "%s: Failed to decode 2-arg command.", GetConnectedAddress().c_str());
					break;
				}
				if(cmd.command != "QC") {
					_log(NET__PRES_ERROR, "%s: Unknown 2-arg command '%s'.", GetConnectedAddress().c_str(), cmd.command.c_str());
					break;
				}
				_log(NET__PRES_DEBUG, "%s: Got Queue Check command.", GetConnectedAddress().c_str());
				//they return position in queue
				PyRep *tmp = new PyRepInteger(1);
				_QueueRep(tmp);
				delete tmp;

				//now act like client just connected
				//send out handshake again
				_SendHandshake();
				//and set proper state
				m_state = VersionNotReceived;
			} else if(t->items.size() == 3) {
				//this is sent when client is logging in
				NetCommand_VK cmd;
				if(!cmd.Decode(&t)) {
					_log(NET__PRES_ERROR, "%s: Failed to decode 3-arg command.", GetConnectedAddress().c_str());
					break;
				}
				if(cmd.command != "VK") {
					_log(NET__PRES_ERROR, "%s: Unknown 3-arg command '%s'.", GetConnectedAddress().c_str(), cmd.command.c_str());
					break;
				}
				_log(NET__PRES_DEBUG, "%s: Got VK command, vipKey=%s.", GetConnectedAddress().c_str(), cmd.vipKey.c_str());
				m_state = CryptoRequestNotReceived;
			} else {
				_log(NET__PRES_ERROR, "%s: Received invalid command packet:", GetConnectedAddress().c_str());
				r->Dump(NET__PRES_ERROR, "	");
			}
		} break;

		case CryptoRequestNotReceived: {
			CryptoRequestPacket cr;
			if(!cr.Decode(&r)) {
				_log(NET__PRES_ERROR, "%S: Received invalid crypto request!", GetConnectedAddress().c_str());
				break;
			}

			if(cr.keyVersion == "placebo") {
				_log(NET__PRES_DEBUG, "%s: Received Placebo crypto request, accepting.", GetConnectedAddress().c_str());
				m_state = CryptoRequestReceived_ChallengeWait;

				//send out accept response
				PyRep *tmp = new PyRepString("OK CC");
				_QueueRep(tmp);
				delete tmp;
			} else {
				//im sure cr.keyVersion can specify either CryptoAPI or PyCrypto, but its all binary so im not sure how.
				PyRep *params = cr.keyParams.Clone();
				CryptoAPIRequestParams car;
				if(!car.Decode(&params)) {
					_log(NET__PRES_ERROR, "%s: Received invalid CryptoAPI request!", GetConnectedAddress().c_str());
					break;
				}
				
				_log(NET__PRES_ERROR, "%s: Unhandled CryptoAPI request: hashmethod=%s sessionkeylength=%d provider=%s sessionkeymethod=%s", GetConnectedAddress().c_str(), car.hashmethod.c_str(), car.sessionkeylength, car.provider.c_str(), car.sessionkeymethod.c_str());
				_log(NET__PRES_ERROR, "%s: You must change your client to use Placebo crypto in common.ini to talk to this server!\n", GetConnectedAddress().c_str());
			}
		}	break;
			
		case CryptoRequestReceived_ChallengeWait: {
			//just to be sure
			if(m_request != NULL)
				delete m_request;
			m_request = new CryptoChallengePacket;
			if(!m_request->Decode(&r)) {
				_log(NET__PRES_ERROR, "%s: Received invalid crypto challenge!", GetConnectedAddress().c_str());
				break;
			}

			_log(NET__PRES_DEBUG, "%s: Received Client Challenge.", GetConnectedAddress().c_str(), m_request->user_name.c_str());

			if(m_request->user_password->CheckType(PyRep::None)) {
				//this is little wrong because on live they send password version always, but only once,
				//but we send password version when we get request with hashed password ...
				_log(NET__PRES_DEBUG, "%s: Got hashed password, requesting plain.", GetConnectedAddress().c_str());
				delete m_request;
				m_request = NULL;

				//send passwordVersion required: 1=plain, 2=hashed
				PyRep *r = new PyRepInteger(1);
				_QueueRep(r);
				delete r;

				break;
			}

			_log(NET__PRES_DEBUG, "%s:		username='%s'", GetConnectedAddress().c_str(), m_request->user_name.c_str());

			//send our handshake
			CryptoServerHandshake server_shake;
			server_shake.serverChallenge = "hi";
			server_shake.func_marshaled_code = new PyRepBuffer(handshakeFunc, sizeof(handshakeFunc));
			server_shake.macho_version = MachoNetVersion;
			server_shake.boot_version = EVEVersionNumber;
			server_shake.boot_build = EVEBuildVersion;
			server_shake.boot_codename = EVEProjectCodename;
			server_shake.boot_region = EVEProjectRegion;
			server_shake.cluster_usercount = m_userCount;
			server_shake.proxy_nodeid = 0xFFAA;
			server_shake.user_logonqueueposition = 1;
			server_shake.challenge_responsehash = "654"; // binascii.crc_hqx of 64 zero bytes in a string, in a single element tuple, marshaled
			
			r = server_shake.Encode();
			_QueueRep(r);
			delete r;

			m_state = CryptoHandshakeSent;
		}	break;

		case CryptoHandshakeSent: {
			CryptoHandshakeResult hr;
			if(!hr.Decode(&r)) {
				_log(NET__PRES_ERROR, "%s: Received invalid crypto handshake result!", GetConnectedAddress().c_str());
				break;
			}

			//this is a bit crappy ...
			client->Login(m_request);

			delete m_request;
			m_request = NULL;

			m_state = AcceptingPackets;

		}	break;

		case AcceptingPackets: {
			//take the PyRep and turn it into a PyPacket
			PyPacket *p = new PyPacket;
			if(!p->Decode(r)) {	//r is consumed here
				_log(NET__PRES_ERROR, "%s: Failed to decode packet rep", GetConnectedAddress().c_str());
				break;
			}
			return(p);
		}	break;
	}
	//recurse, just in case the next packet is here already.
	return(PopPacket());
}

void EVEPresentation::_SendHandshake() {
	VersionExchange version;
	version.birthday = EVEBirthday;
	version.macho_version = MachoNetVersion;
	version.user_count = m_userCount;
	version.version_number = EVEVersionNumber;
	version.build_version = EVEBuildVersion;
	version.project_version = EVEProjectVersion;
	
	_log(NET__PRES_REP_OUT, "%s: Sending Low Level Version Exchange:", GetConnectedAddress().c_str());
	version.Dump(NET__PRES_REP_OUT, "    ");

	PyRep *r = version.Encode();

	_QueueRep(r);
	delete r;
}

std::string EVEPresentation::GetConnectedAddress() const {
	char buf[64];
	struct in_addr	in;
	in.s_addr = net->GetrIP();
	snprintf(buf, sizeof(buf), "%s:%d", inet_ntoa(in), net->GetrPort());
	return(buf);
	//return net->GetrIP();
}








