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



#include "../common/common.h"

#include "EVEPresentation.h"
#include "EVETCPConnection.h"
#include "PyPacket.h"
#include "PyRep.h"
#include "EVEUnmarshal.h"
#include "EVEMarshal.h"
#include "PyDumpVisitor.h"
#include "EVEVersion.h"
#include "../common/logsys.h"

#include "../packets/Crypto.h"


EVEPresentation::EVEPresentation(EVETCPConnection *n)
: net(n),
  m_version(new VersionExchange)
#ifdef DRAGON
,  m_state(VersionNotReceived)
#endif
{
}

EVEPresentation::~EVEPresentation() {
	delete m_version;
	//let the server thread know we are done with this connection
	
	net->Free();
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
		r->Dump(stdout, "    ");
	}

#ifdef DRAGON
	switch(m_state) {
	case VersionNotReceived: {
		//we are waiting for thier version information...
		VersionExchange ve;
		if(!ve.Decode(&r)) {
			_log(NET__PRES_ERROR, "%s: Received invalid version exchange!", GetConnectedAddress().c_str());
			return(NULL);
		}

		//TODO: check this crap...

		_log(NET__PRES_DEBUG, "%s: Received version exchange (%d, %d, %d, %f, %d, '%s'), waiting for crypto request.", GetConnectedAddress().c_str(), ve.birthday, ve.macho_version, ve.user_count, ve.version_number, ve.build_version, ve.project_version.c_str());
		
		m_state = CryptoNotNegotiated;

		//recurse, just in case the next packet is here already.
		return(PopPacket());
	}	break;
		
	case CryptoNotNegotiated: {
		//we are waiting for their crypto request.
		CryptoRequestPacket cr;
		if(!cr.Decode(&r)) {
			_log(NET__PRES_ERROR, "%S: Received invalid crypto request!", GetConnectedAddress().c_str());
			return(NULL);
		}

		if(cr.keyVersion == "placebo") {
			m_state = CryptoRequestReceived_ChallengeWait;
			_log(NET__PRES_DEBUG, "%s: Received Placebo crypto request, accepting.", GetConnectedAddress().c_str());
		} else {
			//im sure cr.keyVersion can specify either CryptoAPI or PyCrypto, but its all binary so im not sure how.
			CryptoAPIRequestPacket car;
			if(!car.Decode(&r)) {
				_log(NET__PRES_ERROR, "%s: Received invalid CryptoAPI request!", GetConnectedAddress().c_str());
				return(NULL);
			}
			
			_log(NET__PRES_ERROR, "%s: Unhandled CryptoAPI request: hashmethod=%s sessionkeylength=%d provider=%s sessionkeymethod=%s", GetConnectedAddress().c_str(), car.hashmethod.c_str(), car.sessionkeylength, car.provider.c_str(), car.sessionkeymethod.c_str());
			_log(NET__PRES_ERROR, "%s: You must change your client to use Placebo crypto in common.ini to talk to this server!\n", GetConnectedAddress().c_str());
			return(NULL);
		}

		//recurse, just in case the next packet is here already.
		return(PopPacket());
	}	break;

	case CryptoRequestReceived_ChallengeWait: {
		CryptoChallengePacket cc;
		if(!cc.Decode(&r)) {
			_log(NET__PRES_ERROR, "%s: Received invalid crypto challenge!", GetConnectedAddress().c_str());
			return(NULL);
		}

		_log(NET__PRES_DEBUG, "%s: Received Client Challenge: username=%s", GetConnectedAddress().c_str(), cc.user_name.c_str());

		CryptoServerHandshake server_shake;
		server_shake.serverChallenge = "hi";
		server_shake.func = "1";	//this is arbitrary python code which the client executes, and returns us the output and result from
		//server_shake.context		//leave empty
		server_shake.macho_version = MachoNetVersion;
		server_shake.boot_version = EVEVersionNumber;
		server_shake.boot_build = EVEBuildVersion;
		server_shake.boot_codename = EVEProjectCodename;
		server_shake.boot_region = EVEProjectRegion;
		server_shake.cluster_usercount = 2;
		server_shake.proxy_nodeid = 0xFFAA;
		server_shake.user_logonqueueposition = 1;
		server_shake.challenge_responsehash = "654"; /* binascii.crc_hqx of 64 zero bytes in a string, in a single element tuple, marshaled */
		
		PyRep *r = server_shake.Encode();
		_QueueRep(r);
		delete r;

		m_state = CryptoHandshakeSent;
	}	break;

	case CryptoHandshakeSent: {
		CryptoHandshakeResult hr;
		if(!hr.Decode(&r)) {
			_log(NET__PRES_ERROR, "%s: Received invalid crypto handshake result!", GetConnectedAddress().c_str());
			return(NULL);
		}

		//send a handshake ack.
		PyRepDict *d = new PyRepDict();
		//its possible to have the key 'live_updates' in the response
        //dict, whos value is a list of code segments to execute on the client side.
		PyRepDict *si = new PyRepDict();
			si->add("userid", "44444");
		d->add("session_init", si);
		PyRep *r = new PyRepSubStream(d);
		_QueueRep(r);
		delete r;
		
		m_state = CryptoActive;
	}	break;
	
	case CryptoActive: {
		//TODO: handle packet types other than PyPacket (like the initial status query dudes, maybe)
		//take the PyRep and turn it into a PyPacket
		PyPacket *p = new PyPacket;
		if(!p->Decode(r)) {	//r is consumed here
			_log(NET__PRES_ERROR, "%s: Failed to decode packet rep", GetConnectedAddress().c_str());
			return(NULL);
		}
		
		return(p);
	}	break;
	}
	
	return(NULL);
#else
	
	if(r->CheckType(PyRep::Tuple)) {
		VersionExchange inc;
		if(!inc.Decode(&r)) {
			_log(NET__PRES_ERROR, "%s: Failed to decode low level version exchange.", GetConnectedAddress().c_str());
			return(NULL);
		}

		_log(NET__PRES_REP, "%s: Received Low Level Version Exchange:\n", GetConnectedAddress().c_str());
		inc.Dump(NET__PRES_REP, "    ");

		if(inc.birthday != m_version->birthday) {
			_log(NET__PRES_ERROR, "%s: Client's birthday does not match ours!", GetConnectedAddress().c_str());
		}
		if(inc.macho_version != m_version->macho_version) {
			_log(NET__PRES_ERROR, "%s: Client's macho_version not match ours!", GetConnectedAddress().c_str());
		}
		if(inc.version_number != m_version->version_number) {
			_log(NET__PRES_ERROR, "%s: Client's version_number not match ours!", GetConnectedAddress().c_str());
		}
		if(inc.build_version != m_version->build_version) {
			_log(NET__PRES_ERROR, "%s: Client's build_version not match ours!", GetConnectedAddress().c_str());
		}
		if(inc.project_version != m_version->project_version) {
			_log(NET__PRES_ERROR, "%s: Client's project_version not match ours!", GetConnectedAddress().c_str());
		}
		
		return(NULL);
	} else {
		//take the PyRep and turn it into a PyPacket
		PyPacket *p = new PyPacket;
		if(!p->Decode(r)) {	//r is consumed here
			_log(NET__PRES_ERROR, "%s: Failed to decode packet rep", GetConnectedAddress().c_str());
			return(NULL);
		}
		
		return(p);
	}
	
#endif
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
	packet->data = MarshalAndDeflate(rep, packet->length);
	if(packet->data == NULL) {
		_log(NET__PRES_ERROR, "%s: Error marshaling or deflating packet!", GetConnectedAddress().c_str());
		return;
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

void EVEPresentation::SendHandshake(uint32 macho_version, uint32 user_count, double version_number, uint16 build_version, const char *project_version) {
	m_version->birthday = EVEBirthday;
	m_version->macho_version = macho_version;
	m_version->user_count = user_count;
	m_version->version_number = version_number;
	m_version->build_version = build_version;
	m_version->project_version = project_version;
	
	_log(NET__PRES_REP_OUT, "%s: Sending Low Level Version Exchange:", GetConnectedAddress().c_str());
	m_version->Dump(NET__PRES_REP_OUT, "    ");

	PyRep *r = m_version->Encode();

	_QueueRep(r);
	delete r;
}

std::string EVEPresentation::GetConnectedAddress() const {
	char buf[54];
	struct in_addr	in;
	in.s_addr = net->GetrIP();
	snprintf(buf, sizeof(buf), "%s:%d", inet_ntoa(in), net->GetrPort());
	return(buf);
}







