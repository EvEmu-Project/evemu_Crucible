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

static const uint32 PING_INTERVAL_US = 60000;

CharacterAppearance::CharacterAppearance() {
	//NULL all dynamic fields
#define NULL_FIELD(v) \
	v = NULL;

#define INT_DYN(v) NULL_FIELD(v)
#define REAL_DYN(v) NULL_FIELD(v)
#include "character/CharacterAppearance_fields.h"

#undef NULL_FIELD
}

CharacterAppearance::CharacterAppearance(const CharacterAppearance &from) {
	//just do deep copy
	*this = from;
}

CharacterAppearance::~CharacterAppearance() {
	//delete all dynamic fields
#define CLEAR_FIELD(v) \
	Clear_##v();

#define INT_DYN(v) CLEAR_FIELD(v)
#define REAL_DYN(v) CLEAR_FIELD(v)
#include "character/CharacterAppearance_fields.h"

#undef CLEAR_FIELD
}

void CharacterAppearance::Build(const std::map<std::string, PyRep *> &from) {
	//builds our contents from strdict
	std::map<std::string, PyRep *>::const_iterator itr;

	_log(CLIENT__MESSAGE, "  Appearance Data:");

#define INT(v) \
	itr = from.find(#v); \
	if(itr != from.end()) { \
		if(!itr->second->CheckType(PyRep::Integer)) \
			_log(CLIENT__ERROR, "Invalid type for " #v ": expected integer, got %s.", itr->second->TypeString()); \
		else { \
			v = ((PyRepInteger *)itr->second)->value; \
			_log(CLIENT__MESSAGE, "     %s: %lu", itr->first.c_str(), v); \
		} \
	}
#define INT_DYN(v) \
	itr = from.find(#v); \
	if(itr != from.end()) { \
		if(!itr->second->CheckType(PyRep::Integer)) \
			_log(CLIENT__ERROR, "Invalid type for " #v ": expected integer, got %s.", itr->second->TypeString()); \
		else { \
			Set_##v(((PyRepInteger *)itr->second)->value); \
			_log(CLIENT__MESSAGE, "     %s: %lu", itr->first.c_str(), Get_##v()); \
		} \
	}
#define REAL(v) \
	itr = from.find(#v); \
	if(itr != from.end()) { \
		if(!itr->second->CheckType(PyRep::Real)) \
			_log(CLIENT__ERROR, "Invalid type for " #v ": expected real, got %s.", itr->second->TypeString()); \
		else { \
			v = ((PyRepReal *)itr->second)->value; \
			_log(CLIENT__MESSAGE, "     %s: %f", itr->first.c_str(), v); \
		} \
	}
#define REAL_DYN(v) \
	itr = from.find(#v); \
	if(itr != from.end()) { \
		if(!itr->second->CheckType(PyRep::Real)) \
			_log(CLIENT__ERROR, "Invalid type for " #v ": expected real, got %s.", itr->second->TypeString()); \
		else { \
			Set_##v(((PyRepReal *)itr->second)->value); \
			_log(CLIENT__MESSAGE, "     %s: %f", itr->first.c_str(), Get_##v()); \
		} \
	}
#include "character/CharacterAppearance_fields.h"
}

void CharacterAppearance::operator=(const CharacterAppearance &from) {
#define COPY(v) \
	v = from.v;
#define COPY_DYN(v) \
	if(!from.IsNull_##v()) \
		Set_##v(from.Get_##v());

#define INT(v) COPY(v)
#define INT_DYN(v) COPY_DYN(v)
#define REAL(v) COPY(v)
#define REAL_DYN(v) COPY_DYN(v)
#include "character/CharacterAppearance_fields.h"

#undef COPY
#undef COPY_DYN
}

Client::Client(PyServiceMgr *services, EVETCPConnection **con)
: DynamicSystemEntity(NULL),
  modules(this),
  m_ship(NULL),
  m_services(services),
  m_net(*con, this),
  m_pingTimer(PING_INTERVAL_US),
  m_accountID(0),
  m_role(1),
  m_gangRole(1),
  m_system(NULL),
//  m_destinyTimer(1000, true),	//accurate timing is essential
//  m_lastDestinyTime(Timer::GetTimeSeconds()),
  m_moveState(msIdle),
  m_moveTimer(500),
  m_movePoint(0, 0, 0),
  m_nextNotifySequence(1)
//  m_nextDestinyUpdate(46751)
{
	*con = NULL;

	m_moveTimer.Disable();
	m_pingTimer.Start();

	m_char.name = "monkey";
	m_char.charid = 444666;
	m_char.bloodlineID = 0;
	m_char.genderID = 1;
	m_char.ancestryID = 2;
	m_char.careerID = 3;
	m_char.schoolID = 4;
	m_char.careerSpecialityID = 5;
	m_char.Intelligence = 6;
	m_char.Charisma = 7;
	m_char.Perception = 8;
	m_char.Memory = 9;
	m_char.Willpower = 10;

	//initialize connection
	m_net.SendHandshake(m_services->entity_list->GetClientCount());
}

Client::~Client() {
	{
		m_services->lsc_service->CharacterLogout(GetCharacterID(), LSCChannel::_MakeSenderInfo(this));
		/*std::list<uint32> toLeave;
		std::set<LSCChannel *>::iterator cur, end;
		cur = m_channels.begin();
		end = m_channels.end();
		for(; cur != end; cur++) {
			
			(*cur)->LeaveChannel(this, false);
		}*/
	}
	
	
	m_services->ClearBoundObjects(this);
	
	//before we remove ourself from the system, store our last location.
	SavePosition();

	if(m_char.charid != 444666) {
 		//johnsus - characterOnline mod
		m_services->GetServiceDB()->SetCharacterOnlineStatus(m_char.charid, false);
		m_services->GetServiceDB()->SetAccountOnlineStatus(m_accountID, false);
	}

	if(m_system != NULL)
		m_system->RemoveClient(this);	//handles removing us from bubbles and sending RemoveBall events.
	
	m_ship->Release();
	targets.DoDestruction();
}

void Client::QueuePacket(PyPacket *p) {
	p->userid = m_accountID;
	m_net.QueuePacket(p);
}

void Client::FastQueuePacket(PyPacket **p) {
	m_net.FastQueuePacket(p);
}

bool Client::ProcessNet() {
	//TRY_SIGEXCEPT {
		
		if(!m_net.Connected())
			return(false);
		
		if(m_pingTimer.Check()) {
			_log(CLIENT__TRACE, "%s: Sending ping request.", GetName());
			_SendPingRequest();
		}
		
		PyPacket *p;
		while((p = m_net.PopPacket())) {
			{
				PyLogsysDump dumper(CLIENT__IN_ALL);
				_log(CLIENT__IN_ALL, "Received packet:");
				p->Dump(CLIENT__IN_ALL, &dumper);
			}
	
			switch(p->type) {
			case CALL_REQ:
				_ProcessCallRequest(p);
				break;
			case NOTIFICATION:
				_ProcessNotification(p);
				break;
			case PING_REQ:
				_log(CLIENT__TRACE, "%s: Unhandled ping request.", GetName());
				break;
			case PING_RSP:
				_log(CLIENT__TRACE, "%s: Received ping response.", GetName());
				break;
			default:
				_log(CLIENT__ERROR, "%s: Unhandled message type %d", GetName(), p->type);
			}
			
			delete p;
			p = NULL;
		}
	
		return(true);
	/*} CATCH_SIGEXCEPT(e) {
		  _log(CLIENT__ERROR, "%s: Exception caught processing network packets\n%s", GetName(), e.stack_string().c_str());
		  return(false);
	}*/
}

void Client::Process() {
	if(m_moveTimer.Check(false)) {
		m_moveTimer.Disable();
		_MoveState s = m_moveState;
		m_moveState = msIdle;
		switch(s) {
		case msIdle:
			_log(CLIENT__ERROR, "%s: Move timer expired when no move is pending.", GetName());
			break;
		//used to delay stargate animation
		case msJump:
			_ExecuteJump();
			break;
		}
	}

	modules.Process();
	
	SystemEntity::Process();
}

void Client::ProcessDestiny() {
	//send changes from previous tic
	_SendQueuedUpdates(DestinyManager::GetStamp()-1);
	DynamicSystemEntity::ProcessDestiny();
}

//this displays a modal error dialog on the client side.
void Client::SendErrorMsg(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str = NULL;
	vaMakeAnyLenString(&str, fmt, args);
	
	_log(CLIENT__ERROR, "Sending Error Message to %s:", GetName());
	log_messageVA(CLIENT__ERROR, fmt, args);
	va_end(args);

	//want to send some sort of notify with a "ServerMessage" message ID maybe?
	//else maybe a "ChatTxt"??
	Notify_OnRemoteMessage n;
	n.msgType = "CustomError";
	n.args[ "error" ] = new PyRepString(str);
	PyRepTuple *tmp = n.FastEncode();
	
	SendNotification("OnRemoteMessage", "charid", &tmp);
	
	delete[] str;
}

//this displays a modal info dialog on the client side.
void Client::SendInfoModalMsg(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str = NULL;
	vaMakeAnyLenString(&str, fmt, args);
	
	_log(CLIENT__MESSAGE, "Info Modal to %s:", GetName());
	log_messageVA(CLIENT__MESSAGE, fmt, args);
	va_end(args);

	//want to send some sort of notify with a "ServerMessage" message ID maybe?
	//else maybe a "ChatTxt"??
	Notify_OnRemoteMessage n;
	n.msgType = "ServerMessage";
	n.args[ "msg" ] = new PyRepString(str);
	PyRepTuple *tmp = n.FastEncode();
	
	SendNotification("OnRemoteMessage", "charid", &tmp);
	
	delete[] str;
}

//this displays a little notice (like combat messages)
void Client::SendNotifyMsg(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str = NULL;
	vaMakeAnyLenString(&str, fmt, args);

	_log(CLIENT__MESSAGE, "Notify to %s:", GetName());
	log_messageVA(CLIENT__MESSAGE, fmt, args);
	va_end(args);

	//want to send some sort of notify with a "ServerMessage" message ID maybe?
	//else maybe a "ChatTxt"??
	Notify_OnRemoteMessage n;
	n.msgType = "CustomNotify";
	n.args[ "notify" ] = new PyRepString(str);
	PyRepTuple *tmp = n.FastEncode();
	
	SendNotification("OnRemoteMessage", "charid", &tmp);
	
	delete[] str;
}

//there may be a less hackish way to do this.
void Client::SelfChatMessage(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str = NULL;
	vaMakeAnyLenString(&str, fmt, args);
	va_end(args);

	if(m_channels.empty()) {
		_log(CLIENT__ERROR, "%s: Tried to send self chat, but we are not joined to any channels: %s", GetName(), str);
		delete[] str;
		return;
	}

	
	_log(CLIENT__TEXT, "%s: Self message on all channels: %s", GetName(), str);

	//this is such a pile of crap, but im not sure whats better.
	//maybe a private message...
	std::set<LSCChannel *>::iterator cur, end;
	cur = m_channels.begin();
	end = m_channels.end();
	for(; cur != end; cur++) {
		(*cur)->SendMessage(this, str, true);
	}

	//m_channels[

	//just send it to the first channel we are in..
    /*LSCChannel *chan = *(m_channels.begin());
	char self_id[24];	//such crap..
	snprintf(self_id, sizeof(self_id), "%lu", GetCharacterID());
	if(chan->GetName() == self_id) {
		if(m_channels.size() > 1) {
			chan = *(++m_channels.begin());
		}
    }*/
	
	delete[] str;
}

void Client::ChannelJoined(LSCChannel *chan) {
	m_channels.insert(chan);
}

void Client::ChannelLeft(LSCChannel *chan) {
	m_channels.erase(chan);
}

void Client::Login(CryptoChallengePacket *pack) {
	_log(CLIENT__MESSAGE, "Login with %s", pack->user_name.c_str());

	if(!pack->user_password->CheckType(PyRep::PackedObject2)) {
		_log(CLIENT__ERROR, "Failed to get password: user password is not PackedObject2.");
		return;
	}
	PyRepPackedObject2 *obj = (PyRepPackedObject2 *)pack->user_password;
	//we can check type, should be "util.PasswordString"

	Call_SingleStringArg pass;
	if(!pass.Decode(&obj->args1)) {
		_log(CLIENT__ERROR, "Failed to decode password.");
		return;
	}
	
	if(!m_services->GetServiceDB()->DoLogin(pack->user_name.c_str(), pass.arg.c_str(), m_accountID, m_role)) {
		_log(CLIENT__MESSAGE, "%s: Login rejected by DB", pack->user_name.c_str());

		PyRepPackedObject1 *e = new PyRepPackedObject1("exceptions.GPSTransportClosed");
		e->args = new PyRepTuple(1);
		e->args->items[0] = new PyRepString("LoginAuthFailed");

		throw(PyException(e));
	}
	
	// this is needed so if we exit before selecting a character, the account online flag would switch back to 0
	m_char.charid = 0;
	m_services->GetServiceDB()->SetAccountOnlineStatus(m_accountID, true);

	//send this before session change
	CryptoHandshakeAck ack;
	ack.connectionLogID = 1;	//TODO: what is this??
	ack.jit = pack->user_languageid;
	ack.userid = m_accountID;
	ack.maxSessionTime = new PyRepNone;
	ack.userType = 1;	//TODO: what is this??
	ack.role = m_role;
	ack.address = m_net.GetConnectedAddress();
	ack.inDetention = new PyRepNone;
	ack.user_clientid = m_accountID;

	m_net._QueueRep(ack.Encode());

	session.Set_userType(1);	//TODO: what is this??
	session.Set_userid(m_accountID);
	session.Set_address(m_net.GetConnectedAddress().c_str());
	session.Set_role(m_role);
	session.Set_languageID(pack->user_languageid.c_str());

	_CheckSessionChange();
}

void Client::_SendPingRequest() {
	PyPacket *ping_req = new PyPacket();
	
	ping_req->type = PING_REQ;
	ping_req->type_string = "macho.PingReq";
	
	ping_req->source.type = PyAddress::Node;
	ping_req->source.typeID = m_services->GetNodeID();
	ping_req->source.service = "ping";
	ping_req->source.callID = 0;
	
	ping_req->dest.type = PyAddress::Client;
	ping_req->dest.typeID = GetAccountID();
	ping_req->dest.callID = 0;
	
	ping_req->userid = m_accountID;
	
	ping_req->payload = new PyRepTuple(1);
	ping_req->payload->items[0] = new PyRepList();	//times
	ping_req->named_payload = new PyRepDict();
	
	FastQueuePacket(&ping_req);
}

void Client::_CheckSessionChange() {
	if(!session.IsDirty())
		return;

	SessionChangeNotification scn;

	session.EncodeChange(scn);
	if(scn.changes.empty())
		return;
	
	_log(CLIENT__SESSION, "Session updated, sending session change");
	scn.changes.Dump(CLIENT__SESSION, "  Changes: ");

	//this is prolly not necessary...
	scn.nodesOfInterest.push_back(m_services->GetNodeID());

	//build the packet:
	PyPacket *p = new PyPacket();
	p->type_string = "macho.SessionChangeNotification";
	p->type = SESSIONCHANGENOTIFICATION;
	
	p->source.type = PyAddress::Node;
	p->source.typeID = m_services->GetNodeID();
	p->source.callID = 0;

	p->dest.type = PyAddress::Client;
	p->dest.typeID = GetAccountID();
	p->dest.callID = 0;

	p->userid = 0;
	
	p->payload = scn.Encode();
	
	p->named_payload = new PyRepDict();
	p->named_payload->add("channel", new PyRepString("sessionchange"));

	FastQueuePacket(&p);
}

/* sync the session with our current state. */
void Client::SessionSync() {
	//account
	session.Set_userid(GetAccountID());
	session.Set_role(GetRole());

	//character
	session.Set_charid(GetCharacterID());
	session.Set_corpid(GetCorporationID());
	session.Set_shipid(GetShipID());

	//location
	if(IsInSpace()) {
		session.Set_locationid(GetSystemID());
		session.Clear_stationid();
		session.Set_solarsystemid(GetSystemID());
	} else {
		session.Set_locationid(GetStationID());
		session.Set_stationid(GetStationID());
		session.Clear_solarsystemid();
	}
	session.Set_solarsystemid2(GetSystemID());
	session.Set_constellationid(GetConstellationID());
	session.Set_regionid(GetRegionID());

	//corporation
	session.Set_corprole(m_corpstate.corprole);
	session.Set_rolesAtAll(m_corpstate.rolesAtAll);
	session.Set_rolesAtBase(m_corpstate.rolesAtBase);
	session.Set_rolesAtHQ(m_corpstate.rolesAtHQ);
	session.Set_rolesAtOther(m_corpstate.rolesAtOther);
	session.Set_hqID(m_corpstate.corpHQ);

	//gang
	session.Set_gangrole(m_gangRole);

	//force a session send.
	_CheckSessionChange();
}

bool Client::EnterSystem() {
	if(m_system != NULL && m_system->GetID() != GetSystemID()) {
		//we have different m_system
		m_system->RemoveClient(this);
		m_system = NULL;
		//send removeball after we are removed from the system, so we dont get it.
		//if(m_destiny != NULL)	//if we were in a station, we have no destiny manager.
		//	m_destiny->SendRemoveBall();
		delete m_destiny;
		m_destiny = NULL;
	}

	if(m_system == NULL) {
		//m_system is NULL, we need new system
		//find our system manager and register ourself with it.
		m_system = m_services->entity_list->FindOrBootSystem(GetSystemID());
		if(m_system == NULL) {
			_log(CLIENT__ERROR, "Failed to boot system %lu for char %s (%lu)", GetSystemID(), GetName(), GetCharacterID());
			SendErrorMsg("Unable to boot system %lu", GetSystemID());
			return(false);
		}
		m_system->AddClient(this);
	}
	
	if(IsStation(GetLocationID())) {
		//we entered station, delete m_destiny
		delete m_destiny;
		m_destiny = NULL;

		//remove ourselves from any bubble
		m_system->bubbles.Remove(this, false);

		NotifyOnCharNowInStation n;
		n.charID = GetCharacterID();
		n.corpID = GetCorporationID();
		n.allianceID = GetAllianceID();
		PyRepTuple *tmp = n.Encode();
		m_services->entity_list->Broadcast("OnCharNowInStation", "stationid", &tmp);
	} else if(IsSolarSystem(GetLocationID())) {
		//if we are not in a station, we are in a system, so we need a destiny manager
		m_destiny = new DestinyManager(this, m_system);
		//ship should never be NULL.
		m_destiny->SetShipCapabilities(m_ship);
		//set position.
		m_destiny->SetPosition(Ship()->position(), false);
		//for now, we always enter a system stopped.
		m_destiny->Halt(false);
	} /*else {	//just dont do anything extra and let them be where they are
		_log(CLIENT__ERROR, "Char %s (%lu) is in a bad location %lu", GetName(), GetCharacterID(), GetLocationID());
		SendErrorMsg("In a bad location %lu", GetLocationID());
		return(false);
	}*/
	
	return(true);
}

void Client::MoveToLocation(uint32 location, const GPoint &pt) {
	if(GetLocationID() == location) {
		// This is a warp or simple movement
		MoveToPosition(pt);
		return;
	}
	
	if(IsStation(GetLocationID())) {
		//TODO: send OnCharNoLongerInStation
	}
	
	if(IsStation(location)) {
		// Entering station
		m_char.stationID = location;
		
		m_services->GetServiceDB()->GetStationParents(
			m_char.stationID,
			m_char.solarSystemID, m_char.constellationID, m_char.regionID );

		Ship()->Move(location, flagHangar);
	} else if(IsSolarSystem(location)) {
		// Entering a solarsystem
		// source is GetLocation()
		// destinaion is location

		m_char.stationID = 0;
		m_char.solarSystemID = location;
		
		m_services->GetServiceDB()->GetSystemParents(
			m_char.solarSystemID,
			m_char.constellationID, m_char.regionID );

		Ship()->Move(location, flagShipOffline);
		Ship()->Relocate(pt);
	} else {
		SendErrorMsg("Move requested to unsupported location %lu", location);
		return;
	}

	if(!EnterSystem())
		return;
	
	//move the character_ record... we really should derive the char's location from the entity table...
	m_services->GetServiceDB()->SetCharacterLocation(
		GetCharacterID(),
		m_char.stationID, m_char.solarSystemID, 
		m_char.constellationID, m_char.regionID );

	SessionSync();
}

void Client::MoveToPosition(const GPoint &pt) {
	if(m_destiny == NULL)
		return;
	m_destiny->Halt(true);
	m_destiny->SetPosition(pt, true);
	Ship()->Relocate(pt);
}

void Client::MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag) {
	
	InventoryItem *item = m_services->item_factory->Load(itemID, false);
	if(item == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to load item %lu", GetName(), itemID);
		return;
	}
	
	bool was_module = ( item->flag() >= flagSlotFirst && item->flag() <= flagSlotLast);

	//do the move. This will update the DB and send the notification.
	item->Move(location, flag);

	if(was_module || (item->flag() >= flagSlotFirst && item->flag() <= flagSlotLast)) {
		//it was equipped, or is now. so modules need to know.
		modules.UpdateModules();
	}
	
	//release the item ref
	item->Release();
}

void Client::BoardShip(InventoryItem *new_ship) {
	//TODO: make sure we are really allowed to board this thing...
	
	if(!new_ship->singleton()) {
		_log(CLIENT__ERROR, "%s: tried to board ship %lu, which is not assembled.", GetName(), new_ship->itemID());
		SendErrorMsg("You cannot board a ship which is not assembled!");
		return;
	}
	
	if(m_system != NULL)
		m_system->RemoveClient(this);
	
	m_ship->Release();			//release old ref
	m_ship = new_ship->Ref();	//get new ref
	m_self->MoveInto(new_ship, flagPilot, false);

	session.Set_shipid(new_ship->itemID());

	//I am not sure where the right place to do this is, but until
	//we properly persist ship attributes into the DB, we are just
	//going to do it here. Could be exploited. oh well.
	// TODO: use the ship aggregate value.
	m_ship->Set_shieldCharge(m_ship->shieldCapacity());
	
	modules.UpdateModules();

	if(m_system != NULL)
		m_system->AddClient(this);

	if(m_destiny != NULL)
		m_destiny->SetShipCapabilities(m_ship);
}

void Client::_ProcessCallRequest(PyPacket *packet) {

	PyService *svc = m_services->LookupService(packet);
	if(svc == NULL) {
		_log(CLIENT__ERROR, "Unable to find service to handle call to:");
		packet->dest.Dump(stdout, "    ");
#ifndef WIN32
#warning TODO: throw proper exception to client (exceptions.ServiceNotFound).
#endif
		PyRep *except = new PyRepNone();
		_SendException(packet, WRAPPEDEXCEPTION, &except);
		return;
	}
	
	//turn this thing into a call:
	PyCallStream call;
	if(!call.Decode(packet->type_string, packet->payload)) {	//payload is consumed
		_log(CLIENT__ERROR, "Failed to convert rep into a call stream");
#ifndef WIN32
#warning TODO: throw proper exception to client.
#endif
		PyRep *except = new PyRepNone();
		_SendException(packet, WRAPPEDEXCEPTION, &except);
		return;
	}

	if(is_log_enabled(CLIENT__CALL_DUMP)) {
		PyLogsysDump dumper(CLIENT__CALL_DUMP);
		_log(CLIENT__CALL_DUMP, "Call Stream Decoded:");
		packet->source.Dump(CLIENT__CALL_DUMP, "  From: ");
		packet->dest.Dump(CLIENT__CALL_DUMP, "  To: ");
		call.Dump(CLIENT__CALL_DUMP, &dumper);
	}

	//build arguments
	PyCallArgs args(this, &call.arg_tuple, &call.arg_dict);

	//try {
		//parts of call may be consumed here
		PyResult result = svc->Call(call, args);

		//successful call.
		PyRep *res = result.ssResult.hijack();

		_CheckSessionChange();	//send out the session change before the return.

		_SendCallReturn(packet, &res);
	/*} catch(sigexcept_exception e)
		std::string str = e.to_string();

		_log(CLIENT__ERROR, "%s invoked exception %s by calling %s::%s\n%s",
			GetName(), e.type_string(), svc->GetName(), call.method.c_str(), str.c_str());

		//replace newline with <br>
		for(size_t i = str.find("\n"); i < str.max_size(); i = str.find("\n", i))
			str.replace(i, 1, "<br>");

		//build exception
		PyRep *except = MakeCustomError(
			"Exception %s occured while processing %s::%s<br>"
			"<br>"
			"%s",
			e.type_string(), svc->GetName(), call.method.c_str(),
			str.c_str()
			);

		//send it to client
		_SendException(packet, WRAPPEDEXCEPTION, &except);
	} catch(PyException &e) {
		PyRep *except = e.ssException.hijack();

		_SendException(packet, WRAPPEDEXCEPTION, &except);
	}*/
}

void Client::_ProcessNotification(PyPacket *packet) {
	//turn this thing into a notify stream:
	ServerNotification notify;
	if(!notify.Decode(&packet->payload)) {	//payload is consumed
		_log(CLIENT__ERROR, "Failed to convert rep into a notify stream");
		return;
	}
	
	if(notify.method == "ClientHasReleasedTheseObjects") {
		PyRep *tmp;
		ServerNotification_ReleaseObj element;
		PyRepList::iterator cur, end;
		cur = notify.elements.begin();
		end = notify.elements.end();
		for(; cur != end; cur++) {
			//damn casting thing...
			tmp = *cur;
			*cur = NULL;
			if(!element.Decode(&tmp))
				_log(CLIENT__ERROR, "Notification '%s' from %s: Failed to decode element. Skipping.", notify.method.c_str(), GetName());
			else
				m_services->ClearBoundObject(element.boundID.c_str());
		}
	} else
		_log(CLIENT__ERROR, "Unhandled notification from %s: unknown method '%s'", GetName(), notify.method.c_str());
	
	_CheckSessionChange();	//just for good measure...
}

void Client::_SendCallReturn(PyPacket *req, PyRep **return_value, const char *channel) {
	
	//build the packet:
	PyPacket *p = new PyPacket();
	p->type_string = "macho.CallRsp";
	p->type = CALL_RSP;
	
	p->source = req->dest;

	p->dest.type = PyAddress::Client;
	p->dest.typeID = GetAccountID();
	p->dest.callID = req->source.callID;

	p->userid = m_accountID;
	
	p->payload = new PyRepTuple(1);
	p->payload->items[0] = new PyRepSubStream(*return_value);
	*return_value = NULL;	//consumed
	
	if(channel != NULL) {
		p->named_payload = new PyRepDict();
		p->named_payload->add("channel", new PyRepString(channel));
	}

	FastQueuePacket(&p);
}

void Client::_SendException(PyPacket *req, MACHONETERR_TYPE type, PyRep **payload) {
	//build the packet:
	PyPacket *p = new PyPacket();
	p->type_string = "macho.ErrorResponse";
	p->type = ERRORRESPONSE;
	
	p->source = req->dest;

	p->dest.type = PyAddress::Client;
	p->dest.typeID = GetAccountID();
	p->dest.callID = req->source.callID;

	p->userid = m_accountID;
	
	macho_MachoException e;
	e.in_response_to = req->type;
	e.exception_type = type;
	e.payload = *payload;
	*payload = NULL;	//consumed

	p->payload = e.Encode();
	FastQueuePacket(&p);
}

//these are specialized Queue functions when our caller can
//easily provide us with our own copy of the data.
void Client::QueueDestinyUpdate(PyRepTuple **du) {
	m_destinyUpdateQueue.push_back(*du);
	*du = NULL;
}

void Client::QueueDestinyEvent(PyRepTuple **multiEvent) {
	m_destinyEventQueue.push_back(*multiEvent);
	*multiEvent = NULL;
}

void Client::_SendQueuedUpdates(uint32 stamp) {
	std::vector<PyRepTuple *>::const_iterator cur, end;
	
	if(m_destinyUpdateQueue.empty()) {
		//no destiny stuff to do...
		if(m_destinyEventQueue.empty()) {
			//no multi-events either...
			return;
		}
		//so, we have multi-event stuff, but no destiny stuff to send.
		//send it out as an OnMultiEvent instead.
		Notify_OnMultiEvent nom;
		nom.events.items = m_destinyEventQueue;
		m_destinyEventQueue.clear();
		PyRepTuple *tmp = nom.FastEncode();	//this is consumed below
		tmp->Dump(DESTINY__UPDATES, "");
		SendNotification("OnMultiEvent", "charid", &tmp);
		return;
	}
	
	DoDestinyUpdateMain dum;

	//first encode the destiny updates.
	cur = m_destinyUpdateQueue.begin();
	end = m_destinyUpdateQueue.end();
	for(; cur != end; cur++) {
		DoDestinyAction act;	//inside the loop just in case memory management techniques ever chance.
		act.update_id = stamp;
		act.update = *cur;
		dum.updates.add( act.FastEncode() );
	}
	m_destinyUpdateQueue.clear();
	
	//right now, we never wait. I am sure they do this for a reason, but
	//I haven't found it yet
	dum.waitForBubble = false;

	//encode any multi-events which go along with it.
	dum.events.items = m_destinyEventQueue;
	m_destinyEventQueue.clear();

	PyRepTuple *tmp = dum.FastEncode();
	tmp->Dump(DESTINY__UPDATES, "");
	SendNotification("DoDestinyUpdate", "clientID", &tmp);
}

void Client::SendNotification(const char *notifyType, const char *idType, PyRepTuple **payload, bool seq) {
	
	//build a little notification out of it.
	EVENotificationStream notify;
	notify.remoteObject = 1;
	notify.args = *payload;
	*payload = NULL;	//consumed

	//now sent it to the client
	PyAddress dest;
	dest.type = PyAddress::Broadcast;
	dest.service = notifyType;
	dest.bcast_idtype = idType;
	SendNotification(dest, &notify, seq);
}


void Client::SendNotification(const PyAddress &dest, EVENotificationStream *noti, bool seq) {

	//build the packet:
	PyPacket *p = new PyPacket();
	p->type_string = "macho.Notification";
	p->type = NOTIFICATION;

	p->source.type = PyAddress::Node;
	p->source.typeID = m_services->GetNodeID();

	p->dest = dest;

	p->userid = GetAccountID();

	p->payload = noti->Encode();

	if(seq) {
		p->named_payload = new PyRepDict();
		p->named_payload->add("sn", new PyRepInteger(m_nextNotifySequence++));
	}

	_log(CLIENT__NOTIFY_DUMP, "Sending notify of type %s with ID type %s", dest.service.c_str(), dest.bcast_idtype.c_str());
	if(is_log_enabled(CLIENT__NOTIFY_REP)) {
		PyLogsysDump dumper(CLIENT__NOTIFY_REP, CLIENT__NOTIFY_REP, true, true);
		p->Dump(CLIENT__NOTIFY_REP, &dumper);
	}

	FastQueuePacket(&p);
}

void Client::WarpTo(const GPoint &to) {
	if(m_moveState != msIdle || m_moveTimer.Enabled()) {
		_log(CLIENT__ERROR, "%s: WarpTo called when a move is already pending. Ignoring.", GetName());
		return;
	}

	m_destiny->WarpTo(to, 15000.0);
	//TODO: OnModuleAttributeChange with attribute 18 for capacitory charge
}

void Client::StargateJump(uint32 fromGate, uint32 toGate) {
	if(m_moveState != msIdle || m_moveTimer.Enabled()) {
		_log(CLIENT__ERROR, "%s: StargateJump called when a move is already pending. Ignoring.", GetName());
		return;
	}

	//TODO: verify that they are actually close to 'fromGate'
	//TODO: verify that 'fromGate' actually jumps to 'toGate'
	
	uint32 regionID;
	uint32 constellationID;
	uint32 solarSystemID;
	GPoint location;
	if(!m_services->GetServiceDB()->GetStaticLocation(
		toGate,
		regionID, constellationID, solarSystemID,
		location
		)
	) {
		codelog(CLIENT__ERROR, "%s: Failed to query information for stargate %lu", GetName(), toGate);
		return;
	}
	
	m_moveSystemID = solarSystemID;
	m_movePoint = location;
	m_movePoint.x -= 15000;

	m_destiny->SendJumpOut(fromGate);
	//TODO: send 'effects.GateActivity' on 'toGate' at the same time

	//delay the move so they can see the JumpOut animation
	_postMove(msJump, 5000);
}

void Client::_postMove(_MoveState type, uint32 wait_ms) {
	m_moveState = type;
	m_moveTimer.Start(wait_ms);
}

void Client::_ExecuteJump() {
	if(m_destiny == NULL)
		return;
	
	MoveToLocation(m_moveSystemID, m_movePoint);
}

bool Client::AddBalance(double amount) {
	if(amount == 0)
		return(true);
	
	double result = m_char.balance + amount;
	
	//remember, this can take a negative amount...
	if(result < 0) {
		return(false);
	}
	
	m_char.balance = result;
	
	if(!m_services->GetServiceDB()->SetCharacterBalance(GetCharacterID(), m_char.balance))
		return(false);
	
	//send notification of change
	OnAccountChange ac;
	ac.accountKey = "cash";
	ac.ownerid = GetCharacterID();
	ac.balance = m_char.balance;
	PyRepTuple *answer = ac.Encode();
	SendNotification("OnAccountChange", "cash", &answer, false);

	return(true);
}



bool Client::Load(uint32 char_id) {
	if(!m_services->GetServiceDB()->LoadCharacter(char_id, m_char)) {
		_log(CLIENT__ERROR, "Failed to load character data for char %lu.", char_id);
		return(false);
	}
	if(!m_services->GetServiceDB()->LoadCorporationMemberInfo(char_id, m_corpstate)) {
		_log(CLIENT__ERROR, "Failed to load corp member info for char %lu.", char_id);
		return(false);
	}

	//get char
	InventoryItem *character = m_services->item_factory->Load(char_id, true);
	if(character == NULL)
		return(false);

	InventoryItem *ship = m_services->item_factory->Load(character->locationID(), true);
	if(ship == NULL)
		return(false);

	_SetSelf(character);	//ref is stored
	BoardShip(ship);	//updates modules
	ship->Release();

	if(!EnterSystem())
		return(false);

	SessionSync();

	return(true);
}

uint32 Client::GetShipID() const {
	if(m_ship == NULL)
		return(0);
	return(m_ship->itemID());
}

double Client::GetMass() const {
	if(m_ship == NULL)
		return(0);
	return(m_ship->mass());
}

double Client::GetMaxVelocity() const {
	if(m_ship == NULL)
		return(0);
	return(m_ship->maxVelocity());
}

double Client::GetAgility() const {
	if(m_ship == NULL)
		return(0);
	return(m_ship->agility());
}

double Client::GetPropulsionStrength() const {
	if(m_ship == NULL)
		return(3.0f);
	//just making shit up, I think skills modify this, as newbies
	//tend to end up with 3.038 instead of the base 3.0 on their ship..
	double res;
	res =  m_ship->propulsionFusionStrength();
	res += m_ship->propulsionIonStrength();
	res += m_ship->propulsionMagpulseStrength();
	res += m_ship->propulsionPlasmaStrength();
	res += m_ship->propulsionFusionStrengthBonus();
	res += m_ship->propulsionIonStrengthBonus();
	res += m_ship->propulsionMagpulseStrengthBonus();
	res += m_ship->propulsionPlasmaStrengthBonus();
	res += 0.038f;
	return(res);
}

void Client::TargetAdded(SystemEntity *who) {

	/*send a destiny update with:
		destiny: OnDamageStateChange
		OnMultiEvent: OnTarget add*/
	//then maybe OnMultiEvent: OnTarget try? for auto target?

	//this is pretty weak
	if(m_destiny != NULL) {
		m_destiny->DoTargetAdded(who);
	}
}

void Client::TargetLost(SystemEntity *who) {
	//OnMultiEvent: OnTarget lost
    Notify_OnTarget te;
    te.mode = "lost";
	te.targetID = who->GetID();

	Notify_OnMultiEvent multi;
    multi.events.add(te.FastEncode());
	
	PyRepTuple *tmp = multi.FastEncode();	//this is consumed below
	SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::TargetedAdd(SystemEntity *who) {
	//OnMultiEvent: OnTarget otheradd
    Notify_OnTarget te;
    te.mode = "otheradd";
	te.targetID = who->GetID();

	Notify_OnMultiEvent multi;
    multi.events.add(te.FastEncode());
	
	PyRepTuple *tmp = multi.FastEncode();	//this is consumed below
	SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::TargetedLost(SystemEntity *who) {
	//OnMultiEvent: OnTarget otherlost
    Notify_OnTarget te;
    te.mode = "otherlost";
	te.targetID = who->GetID();

	Notify_OnMultiEvent multi;
    multi.events.add(te.FastEncode());
	
	PyRepTuple *tmp = multi.FastEncode();	//this is consumed below
	SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::TargetsCleared() {
	//OnMultiEvent: OnTarget clear
    Notify_OnTarget te;
    te.mode = "clear";
	te.targetID = 0;

	Notify_OnMultiEvent multi;
    multi.events.add(te.FastEncode());
	
	PyRepTuple *tmp = multi.FastEncode();	//this is consumed below
	SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::SavePosition() {
	if(m_ship == NULL || m_destiny == NULL) {
		_log(CLIENT__TRACE, "%s: Unable to save position. We are prolly not in space.", GetName());
		return;
	}
	m_ship->Relocate( m_destiny->GetPosition() );
}

bool Client::LaunchDrone(InventoryItem *drone) {
#if 0
drop 166328265

OnItemChange ,*args= (Row(itemID: 166328265,typeID: 15508,
	ownerID: 105651216,locationID: 166363674,flag: 87,
	contraband: 0,singleton: 1,quantity: 1,groupID: 100,
	categoryID: 18,customInfo: (166363674, None)), 
	{10: None}) ,**kw= {}

OnItemChange ,*args= (Row(itemID: 166328265,typeID: 15508,
	ownerID: 105651216,locationID: 30001369,flag: 0,
	contraband: 0,singleton: 1,quantity: 1,groupID: 100,
	categoryID: 18,customInfo: (166363674, None)), 
	{3: 166363674, 4: 87}) ,**kw= {}
	
returns list of deployed drones.

internaly scatters:
	OnItemLaunch ,*args= ([166328265], [166328265])

DoDestinyUpdate ,*args= ([(31759, 
	('OnDroneStateChange', 
		[166328265, 105651216, 166363674, 0, 15508, 105651216])
	), (31759, 
	('OnDamageStateChange', 
		(2100212375, [(1.0, 400000.0, 127997215757036940L), 1.0, 1.0]))
	), (31759, ('AddBalls',
		...
	True

DoDestinyUpdate ,*args= ([(31759, 
	('Orbit', (166328265, 166363674, 750.0))
	), (31759, 
	('SetSpeedFraction', (166328265, 0.265625)))], 
	False) ,**kw= {} )
#endif

	if(!IsSolarSystem(GetLocationID())) {
		_log(SERVICE__ERROR, "%s: Trying to launch drone when not in space!", GetName());
		return(false);
	}

	_log(CLIENT__MESSAGE, "%s: Launching drone %lu", GetName(), drone->itemID());
	
	//first, the item gets moved into space
	//TODO: set customInfo to a tuple: (shipID, None)
	drone->Move(GetSystemID(), flagAutoFit);
	//temp for testing:
	drone->Move(GetShipID(), flagDroneBay, false);

	//now we create an NPC to represent it.
	GPoint position(GetPosition());
	position.x += 750.0f;	//total crap.
	
	//this adds itself into the system.
	NPC *drone_npc = new NPC(
		m_system,
		m_services,
		drone, 
		GetCorporationID(),
		GetAllianceID(),
		position);
	m_system->AddNPC(drone_npc);

	//now we tell the client that "its ALIIIIIVE"
	//DoDestinyUpdate:

	//drone_npc->Destiny()->SendAddBall();
	
    /*PyRepList *actions = new PyRepList();
	DoDestinyAction act;
	act.update_id = NextDestinyUpdateID();	//update ID?
    */
	// 	OnDroneStateChange
/*  {
		DoDestiny_OnDroneStateChange du;
		du.droneID = drone->itemID();
		du.ownerID = GetCharacterID();
		du.controllerID = GetShipID();
		du.activityState = 0;
		du.droneTypeID = drone->typeID();
		du.controllerOwnerID = Ship()->ownerID();
		act.update = du.FastEncode();
		actions->add( act.FastEncode() );
    }*/
	
	// 	AddBall
    /*{
		DoDestiny_AddBall addball;
		drone_npc->MakeAddBall(addball, act.update_id);
		act.update = addball.FastEncode();
		actions->add( act.FastEncode() );
    }*/
	
	// 	Orbit
/*	{
		DoDestiny_Orbit du;
		du.entityID = drone->itemID();
		du.orbitEntityID = GetCharacterID();
		du.distance = 750;
		act.update = du.FastEncode();
		actions->add( act.FastEncode() );
	}
	
	// 	SetSpeedFraction
	{
		DoDestiny_SetSpeedFraction du;
		du.entityID = drone->itemID();
		du.fraction = 0.265625f;
		act.update = du.FastEncode();
		actions->add( act.FastEncode() );
	}*/

//testing:
/*	{
		DoDestiny_SetBallInteractive du;
		du.entityID = drone->itemID();
		du.interactive = 1;
		act.update = du.FastEncode();
		actions->add( act.FastEncode() );
	}
	{
		DoDestiny_SetBallInteractive du;
		du.entityID = GetCharacterID();
		du.interactive = 1;
		act.update = du.FastEncode();
		actions->add( act.FastEncode() );
	}*/
	
	//NOTE: we really want to broadcast this...
	//TODO: delay this until after the return.
	//_SendDestinyUpdate(&actions, false);

	return(false);
}

//assumes that the backend DB stuff was already done.
void Client::JoinCorporationUpdate(uint32 corp_id) {
	m_char.corporationID = corp_id;
	
	//TODO: recursively change corp on all our items.
	
	m_services->GetServiceDB()->LoadCorporationMemberInfo(GetCharacterID(), m_corpstate);
	
	session.Set_corpid(corp_id);
	session.Set_corprole(m_corpstate.corprole);
	session.Set_rolesAtAll(m_corpstate.rolesAtAll);
	session.Set_rolesAtBase(m_corpstate.rolesAtBase);
	session.Set_rolesAtHQ(m_corpstate.rolesAtHQ);
	session.Set_rolesAtOther(m_corpstate.rolesAtOther);

	//logs indicate that we need to push this update out asap.
	_CheckSessionChange();
}

/*
FunctorTimerQueue::TimerID Client::Delay( uint32 time_in_ms, void (Client::* clientCall)() ) {
	Functor *f = new SimpleClientFunctor(this, clientCall);
	return(m_delayQueue.Schedule( &f, time_in_ms ));
}

FunctorTimerQueue::TimerID Client::Delay( uint32 time_in_ms, ClientFunctor **functor ) {
	Functor *f = *functor;
	*functor = NULL;
	return(m_delayQueue.Schedule( &f, time_in_ms ));
}




FunctorTimerQueue::FunctorTimerQueue()
: m_nextID(0)
{
}

FunctorTimerQueue::~FunctorTimerQueue() {
	std::vector<Entry *>::iterator cur, end;
	cur = m_queue.begin();
	end = m_queue.end();
	for(; cur != end; cur++) {
		delete *cur;
	}
}

FunctorTimerQueue::TimerID FunctorTimerQueue::Schedule(Functor **what, uint32 in_how_many_ms) {
	Entry *e = new Entry(++m_nextID, *what, in_how_many_ms);
	*what = NULL;
	m_queue.push_back(e);
	return(e->id);
}

bool FunctorTimerQueue::Cancel(TimerID id) {
	std::vector<Entry *>::iterator cur, end;
	cur = m_queue.begin();
	end = m_queue.end();
	for(; cur != end; cur++) {
		if((*cur)->id == id) {
			m_queue.erase(cur);
			return(true);
		}
	}
	return(false);
}

void FunctorTimerQueue::Process() {
	std::vector<Entry *>::iterator cur, tmp;
	cur = m_queue.begin();
	while(cur != m_queue.end()) {
		Entry *e = *cur;
		if(e->when.Check(false)) {
			//call the functor.
			(*e->func)();
			//we are done with this crap.
			delete e;
			cur = m_queue.erase(cur);
		} else {
			cur++;
		}
	}
}

FunctorTimerQueue::Entry::Entry(TimerID _id, Functor *_func, uint32 time_ms)
: id(_id),
  func(_func),
  when(time_ms)
{
	when.Start();
}

FunctorTimerQueue::Entry::~Entry() {
	delete func;
}
*/





















