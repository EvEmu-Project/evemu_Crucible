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

#include "EvemuPCH.h"

static const uint32 PING_INTERVAL_US = 60000;

Client::Client(PyServiceMgr &services, EVETCPConnection *&con)
: DynamicSystemEntity(NULL),
  modules(this),
  m_char(NULL),
  m_services(services),
  m_net(con, this),
  m_pingTimer(PING_INTERVAL_US),
  m_accountID(0),
  m_accountRole(1),
  m_gangRole(1),
  m_system(NULL),
//  m_destinyTimer(1000, true),	//accurate timing is essential
//  m_lastDestinyTime(Timer::GetTimeSeconds()),
  m_moveState(msIdle),
  m_moveTimer(500),
  m_movePoint(0, 0, 0),
  m_timeEndTrain(0),
  m_nextNotifySequence(1)
//  m_nextDestinyUpdate(46751)
{
	con = NULL;

	m_moveTimer.Disable();
	m_pingTimer.Start();

	//initialize connection
	m_net.SendHandshake(m_services.entity_list.GetClientCount());
}

Client::~Client() {
	if(m_char != NULL) {
		// we have valid character

		// LSC logout
		m_services.lsc_service->CharacterLogout(GetCharacterID(), LSCChannel::_MakeSenderInfo(this));

		//before we remove ourself from the system, store our last location.
		SavePosition();

		// remove ourselves from system
		if(m_system != NULL)
			m_system->RemoveClient(this);	//handles removing us from bubbles and sending RemoveBall events.

 		//johnsus - characterOnline mod
		// switch character online flag to 0
		m_services.serviceDB().SetCharacterOnlineStatus(GetCharacterID(), false);

		// release our char ref
		m_char->DecRef();
	}

	if(GetAccountID() != 0) { // this is not very good ....
		m_services.serviceDB().SetAccountOnlineStatus(GetAccountID(), false);
	}

	m_services.ClearBoundObjects(this);

	targets.DoDestruction();
}

void Client::QueuePacket(PyPacket *p) {
	p->userid = GetAccountID();
	m_net.QueuePacket(p);
}

void Client::FastQueuePacket(PyPacket **p) {
	m_net.FastQueuePacket(p);
}

/** Small utility wrapper function to handle client ping request
  * reason why its not cleanly in the client class: The client isn't clean in the first place.
  * TODO insert into client class.
  */
void HandlePingRequest(Client* client, PyPacket * packet)
{
	if (client == NULL)
		return;

	if (packet == NULL)
		return;

	/* clone the incoming packet as our outgoing packet looks a lot like it */
	PyPacket * ret = packet->Clone();	// cloning is evil
	ret->type = PING_RSP;
	ret->type_string = "macho.PingReq";
	
	// evil hacking
	PyAddress temp = ret->source;
	ret->source = ret->dest;
	ret->dest = temp;

	ret->source.typeID = client->services().GetNodeID();

	ret->dest.typeID = client->GetAccountID();
	
	/*	Here the hacking begins, the ping packet handles the timestamps of various packet handling steps.
		To really simulate/emulate that we need the various packet handlers which in fact we don't have ( :P ).
		So the next piece of code "fake's" it, with a slight delay on the received packet time.
	*/
	PyRepList* pingList = (PyRepList*)ret->payload->items[0];
	if (pingList->IsList())
	{
		PyRepTuple * pingTuple = new PyRepTuple(3);

		pingTuple->items[0] = new PyRepInteger(Win32TimeNow() - 20);		// this should be the time the packet was received (we cheat here a bit)
		pingTuple->items[1] = new PyRepInteger(Win32TimeNow());				// this is the time the packet is (handled/writen) by the (proxy/server) so we're cheating a bit again.
		pingTuple->items[2] = new PyRepString("proxy::handle_message");

		pingList->add(pingTuple);

		pingTuple = new PyRepTuple(3);
		pingTuple->items[0] = new PyRepInteger(Win32TimeNow() - 20);
		pingTuple->items[1] = new PyRepInteger(Win32TimeNow());
		pingTuple->items[2] = new PyRepString("proxy::writing");

		pingList->add(pingTuple);

		pingTuple = new PyRepTuple(3);
		pingTuple->items[0] = new PyRepInteger(Win32TimeNow() - 20);
		pingTuple->items[1] = new PyRepInteger(Win32TimeNow());
		pingTuple->items[2] = new PyRepString("server::handle_message");

		pingList->add(pingTuple);

		pingTuple = new PyRepTuple(3);
		pingTuple->items[0] = new PyRepInteger(Win32TimeNow() - 20);	
		pingTuple->items[1] = new PyRepInteger(Win32TimeNow());			
		pingTuple->items[2] = new PyRepString("server::turnaround");

		pingList->add(pingTuple);

		pingTuple = new PyRepTuple(3);
		pingTuple->items[0] = new PyRepInteger(Win32TimeNow() - 20);
		pingTuple->items[1] = new PyRepInteger(Win32TimeNow());
		pingTuple->items[2] = new PyRepString("proxy::handle_message");

		pingList->add(pingTuple);

		pingTuple = new PyRepTuple(3);
		pingTuple->items[0] = new PyRepInteger(Win32TimeNow() - 20);
		pingTuple->items[1] = new PyRepInteger(Win32TimeNow());
		pingTuple->items[2] = new PyRepString("proxy::writing");

		pingList->add(pingTuple);
	}

	client->FastQueuePacket(&ret);	// doesn't clone so eats the ret object upon sending.
}

bool Client::ProcessNet() {
		
	if(!m_net.Connected())
		return false;
	
	if(m_pingTimer.Check()) {
		//_log(CLIENT__TRACE, "%s: Sending ping request.", GetName());
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
			//_log(CLIENT__TRACE, "%s: Unhandled ping request.", GetName());
			HandlePingRequest(this,p);
			break;
		case PING_RSP:
			//_log(CLIENT__TRACE, "%s: Received ping response.", GetName());
			break;
		default:
			_log(CLIENT__ERROR, "%s: Unhandled message type %d", GetName(), p->type);
		}
		
		SafeDelete(p);
	}

	// send queued updates
	_SendQueuedUpdates();

	return true;
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

	if( m_timeEndTrain != 0 )
	{
		if( m_timeEndTrain <= Win32TimeNow() )
			GetChar()->UpdateSkillQueue();
	}

	modules.Process();
	
	SystemEntity::Process();
}

//this displays a modal error dialog on the client side.
void Client::SendErrorMsg(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str = NULL;
	vasprintf(&str, fmt, args);
	
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
	
	free(str);
}

//this displays a modal info dialog on the client side.
void Client::SendInfoModalMsg(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str = NULL;
	vasprintf(&str, fmt, args);
	
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
	
	free(str);
}

//this displays a little notice (like combat messages)
void Client::SendNotifyMsg(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str = NULL;
	vasprintf(&str, fmt, args);

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
	
	free(str);
}

//there may be a less hackish way to do this.
void Client::SelfChatMessage(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str = NULL;
	vasprintf(&str, fmt, args);
	va_end(args);

	if(m_channels.empty()) {
		_log(CLIENT__ERROR, "%s: Tried to send self chat, but we are not joined to any channels: %s", GetName(), str);
		free(str);
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
	snprintf(self_id, sizeof(self_id), "%u", GetCharacterID());
	if(chan->GetName() == self_id) {
		if(m_channels.size() > 1) {
			chan = *(++m_channels.begin());
		}
    }*/
	
	free(str);
}

void Client::ChannelJoined(LSCChannel *chan) {
	m_channels.insert(chan);
}

void Client::ChannelLeft(LSCChannel *chan) {
	m_channels.erase(chan);
}

void Client::Login(CryptoChallengePacket *pack) {
	_log(CLIENT__MESSAGE, "Login with %s", pack->user_name.c_str());

	util_PasswordString pass;
	if(!pass.Decode(&pack->user_password)) {
		_log(CLIENT__ERROR, "Failed to decode password.");
		return;
	}
	
	if(!m_services.serviceDB().DoLogin(pack->user_name.c_str(), pass.password.c_str(), m_accountID, m_accountRole)) {
		_log(CLIENT__MESSAGE, "%s: Login rejected by DB", pack->user_name.c_str());

		util_NewObject1 no;
		no.type = "exceptions.GPSTransportClosed";

		no.args = new PyRepTuple(1);
		no.args->items[0] = new PyRepString("LoginAuthFailed");

		no.keywords.add("msgkey", "LoginAuthFailed");

		throw(PyException(no.FastEncode()));
	}

	m_services.serviceDB().SetAccountOnlineStatus(GetAccountID(), true);

	//send this before session change
	CryptoHandshakeAck ack;
	ack.connectionLogID = 1;	//TODO: what is this??
	ack.jit = pack->user_languageid;
	ack.userid = GetAccountID();
	ack.maxSessionTime = new PyRepNone;
	ack.userType = 1;
	ack.role = GetAccountRole();
	ack.address = m_net.GetConnectedAddress();
	ack.inDetention = new PyRepNone;
	ack.user_clientid = GetAccountID();

	m_net._QueueRep(ack.FastEncode());

	session.Set_userType(1);	//user type 1 is normal user, type 23 is a trial account user.
	session.Set_userid(GetAccountID());
	session.Set_address(m_net.GetConnectedAddress().c_str());
	session.Set_role(GetAccountRole());
	session.Set_languageID(pack->user_languageid.c_str());
	session.Set_inDetention(0);

	_CheckSessionChange();
}

void Client::_SendPingRequest() {
	PyPacket *ping_req = new PyPacket();
	
	ping_req->type = PING_REQ;
	ping_req->type_string = "macho.PingReq";
	
	ping_req->source.type = PyAddress::Node;
	ping_req->source.typeID = m_services.GetNodeID();
	ping_req->source.service = "ping";
	ping_req->source.callID = 0;
	
	ping_req->dest.type = PyAddress::Client;
	ping_req->dest.typeID = GetAccountID();
	ping_req->dest.callID = 0;
	
	ping_req->userid = GetAccountID();
	
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

	//this is probably not necessary...
	scn.nodesOfInterest.push_back(m_services.GetNodeID());

	//build the packet:
	PyPacket *p = new PyPacket();
	p->type_string = "macho.SessionChangeNotification";
	p->type = SESSIONCHANGENOTIFICATION;
	
	p->source.type = PyAddress::Node;
	p->source.typeID = m_services.GetNodeID();
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
	if(m_char == NULL) {
		// print a warning because we're probably gonna mess up our session values with bunch of zeroes.
		_log(CLIENT__MESSAGE, "SessionSync() called with no valid character!");
	}

	//account
	session.Set_userid(GetAccountID());
	session.Set_role(GetAccountRole());

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
	session.Set_hqID(GetCorpHQ());
	session.Set_corprole(GetCorpRole());
	session.Set_rolesAtAll(GetRolesAtAll());
	session.Set_rolesAtBase(GetRolesAtBase());
	session.Set_rolesAtHQ(GetRolesAtHQ());
	session.Set_rolesAtOther(GetRolesAtOther());

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

		delete m_destiny;
		m_destiny = NULL;
	}

	if(m_system == NULL) {
		//m_system is NULL, we need new system
		//find our system manager and register ourself with it.
		m_system = m_services.entity_list.FindOrBootSystem(GetSystemID());
		if(m_system == NULL) {
			_log(CLIENT__ERROR, "Failed to boot system %u for char %s (%u)", GetSystemID(), GetName(), GetCharacterID());
			SendErrorMsg("Unable to boot system %u", GetSystemID());
			return false;
		}
		m_system->AddClient(this);
	}
	
	if(IsStation(GetLocationID())) {
		//we entered station, delete m_destiny
		delete m_destiny;
		m_destiny = NULL;

		//remove ourselves from any bubble
		m_system->bubbles.Remove(this, false);

		OnCharNowInStation();
	} else if(IsSolarSystem(GetLocationID())) {
		//we are in a system, so we need a destiny manager
		m_destiny = new DestinyManager(this, m_system);
		//ship should never be NULL.
		m_destiny->SetShipCapabilities(GetShip());
		//set position.
		m_destiny->SetPosition(GetShip()->position(), false);
		//for now, we always enter a system stopped.
		m_destiny->Halt(false);
	}
	
	return true;
}

void Client::MoveToLocation(uint32 location, const GPoint &pt) {
	if(GetLocationID() == location) {
		// This is a warp or simple movement
		MoveToPosition(pt);
		return;
	}

	if(IsStation(GetLocationID())) {
		OnCharNoLongerInStation();
	}

	uint32 stationID, solarSystemID, constellationID, regionID;
	if(IsStation(location)) {
		// Entering station
		stationID = location;
		
		m_services.serviceDB().GetStationInfo(
			stationID,
			&solarSystemID, &constellationID, &regionID,
			NULL, NULL, NULL
		);

		GetShip()->Move(stationID, flagHangar);
	} else if(IsSolarSystem(location)) {
		// Entering a solarsystem
		// source is GetLocation()
		// destinaion is location

		stationID = 0;
		solarSystemID = location;
		
		m_services.serviceDB().GetSystemInfo(
			solarSystemID,
			&constellationID, &regionID,
			NULL, NULL
		);

		GetShip()->Move(solarSystemID, flagShipOffline);
		GetShip()->Relocate(pt);
	} else {
		SendErrorMsg("Move requested to unsupported location %u", location);
		return;
	}

	//move the character_ record... we really should derive the char's location from the entity table...
	GetChar()->SetLocation(stationID, solarSystemID, constellationID, regionID);

	EnterSystem();
	SessionSync();
}

void Client::MoveToPosition(const GPoint &pt) {
	if(m_destiny == NULL)
		return;
	m_destiny->Halt(true);
	m_destiny->SetPosition(pt, true);
	GetShip()->Relocate(pt);
}

void Client::MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag) {
	
	InventoryItem *item = m_services.item_factory.GetItem(itemID, false);
	if(item == NULL) {
		codelog(SERVICE__ERROR, "%s: Unable to load item %u", GetName(), itemID);
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
	item->DecRef();
}

void Client::BoardShip(Ship *new_ship) {
	//TODO: make sure we are really allowed to board this thing...
	
	if(!new_ship->singleton()) {
		_log(CLIENT__ERROR, "%s: tried to board ship %u, which is not assembled.", GetName(), new_ship->itemID());
		SendErrorMsg("You cannot board a ship which is not assembled!");
		return;
	}
	
	if(m_system != NULL)
		m_system->RemoveClient(this);

	_SetSelf(new_ship->IncRef());
	m_char->MoveInto( new_ship, flagPilot, false );

	session.Set_shipid(new_ship->itemID());

	modules.UpdateModules();

	if(m_system != NULL)
		m_system->AddClient(this);

	if(m_destiny != NULL)
		m_destiny->SetShipCapabilities(GetShip());
}

void Client::_ProcessCallRequest(PyPacket *packet) {
	PyCallable *dest = NULL;
	if(!packet->dest.service.empty()) {
		//service
		dest = m_services.LookupService(packet->dest.service);

		if(dest == NULL) {
			_log(CLIENT__ERROR, "Unable to find service to handle call to:");
			packet->dest.Dump(CLIENT__ERROR, "    ");
#ifndef WIN32
#	warning TODO: throw proper exception to client (exceptions.ServiceNotFound).
#endif
			PyRep *except = new PyRepNone();
			_SendException(packet, WRAPPEDEXCEPTION, &except);
			return;
		}
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

	if(packet->dest.service.empty()) {
		//bound object
		uint32 nodeID, bindID;
		if(sscanf(call.remoteObjectStr.c_str(), "N=%u:%u", &nodeID, &bindID) != 2) {
			_log(CLIENT__ERROR, "Failed to parse bind string '%s'.", call.remoteObjectStr.c_str());
			return;
		}

		if(nodeID != m_services.GetNodeID()) {
			_log(CLIENT__ERROR, "Unknown nodeID %u received (expected %u).", nodeID, m_services.GetNodeID());
			return;
		}

		dest = m_services.FindBoundObject(bindID);
		if(dest == NULL) {
			_log(CLIENT__ERROR, "Failed to find bound object %u.", bindID);
			return;
		}
	}

	//build arguments
	PyCallArgs args(this, &call.arg_tuple, &call.arg_dict);

	try {
		//parts of call may be consumed here
		PyResult result = dest->Call(call.method, args);

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
		_SendException(packet, WRAPPEDEXCEPTION, &except);*/
	} catch(PyException &e) {
		PyRep *except = e.ssException.hijack();

		_SendException(packet, WRAPPEDEXCEPTION, &except);
	}
}

void Client::_ProcessNotification(PyPacket *packet) {
	//turn this thing into a notify stream:
	ServerNotification notify;
	if(!notify.Decode(&packet->payload)) {	//payload is consumed
		_log(CLIENT__ERROR, "Failed to convert rep into a notify stream");
		return;
	}
	
	if(notify.method == "ClientHasReleasedTheseObjects") {
		PyRep *n;
		ServerNotification_ReleaseObj element;
		PyRepList::iterator cur, end;
		cur = notify.elements.begin();
		end = notify.elements.end();
		for(; cur != end; cur++) {
			//damn casting thing...
			n = *cur;
			*cur = NULL;

			if(!element.Decode(&n)) {
				_log(CLIENT__ERROR, "Notification '%s' from %s: Failed to decode element. Skipping.", notify.method.c_str(), GetName());
				continue;
			}

			uint32 nodeID, bindID;
			if(sscanf(element.boundID.c_str(), "N=%u:%u", &nodeID, &bindID) != 2) {
				_log(CLIENT__ERROR, "Notification '%s' from %s: Failed to parse bind string '%s'. Skipping.",
					notify.method.c_str(), GetName(), element.boundID.c_str());
				continue;
			}

			if(nodeID != m_services.GetNodeID()) {
				_log(CLIENT__ERROR, "Notification '%s' from %s: Unknown nodeID %u received (expected %u). Skipping.",
					notify.method.c_str(), GetName(), nodeID, m_services.GetNodeID());
				continue;
			}

			m_services.ClearBoundObject(bindID);
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

	p->userid = GetAccountID();
	
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

	p->userid = GetAccountID();
	
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
	DoDestinyAction act;
	act.update_id = DestinyManager::GetStamp();
	act.update = *du;
	*du = NULL;

	m_destinyUpdateQueue.push_back( act.FastEncode() );
}

void Client::QueueDestinyEvent(PyRepTuple **multiEvent) {
	m_destinyEventQueue.push_back(*multiEvent);
	*multiEvent = NULL;
}

void Client::_SendQueuedUpdates() {
	if(!m_destinyUpdateQueue.empty()) {
		DoDestinyUpdateMain dum;

		//first insert the destiny updates.
		dum.updates.items = m_destinyUpdateQueue;
		m_destinyUpdateQueue.clear();
		
		//encode any multi-events which go along with it.
		dum.events.items = m_destinyEventQueue;
		m_destinyEventQueue.clear();

		//right now, we never wait. I am sure they do this for a reason, but
		//I haven't found it yet
		dum.waitForBubble = false;

		//now send it
		PyRepTuple *t = dum.FastEncode();
		t->Dump(DESTINY__UPDATES, "");
		SendNotification("DoDestinyUpdate", "clientID", &t);
	} else if(!m_destinyEventQueue.empty()) {
		Notify_OnMultiEvent nom;

		//insert updates, clear our queue
		nom.events.items = m_destinyEventQueue;
		m_destinyEventQueue.clear();

		//send it
		PyRepTuple *t = nom.FastEncode();	//this is consumed below
		t->Dump(DESTINY__UPDATES, "");
		SendNotification("OnMultiEvent", "charid", &t);
	} //else nothing to do ...
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
	p->source.typeID = m_services.GetNodeID();

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

PyRepDict *Client::MakeSlimItem() const {
	PyRepDict *slim = DynamicSystemEntity::MakeSlimItem();

	slim->add("charID", new PyRepInteger(GetCharacterID()));
	slim->add("corpID", new PyRepInteger(GetCorporationID()));
	slim->add("allianceID", new PyRepNone);
	slim->add("warFactionID", new PyRepNone);

	//encode the modules list, if we have any visible modules
	std::vector<InventoryItem *> items;
	GetShip()->FindByFlagRange(flagHiSlot0, flagHiSlot7, items, false);
	if(!items.empty()) {
		PyRepList *l = new PyRepList();
		std::vector<InventoryItem *>::iterator cur, end;
		cur = items.begin();
		end = items.end();
		for(; cur != end; cur++) {
			InventoryItem *i = *cur;

			PyRepTuple *t = new PyRepTuple(2);
			t->items[0] = new PyRepInteger(i->itemID());
			t->items[1] = new PyRepInteger(i->typeID());

			l->add(t);
		}
		slim->add("modules", l);
	}

	slim->add("color", new PyRepReal(0.0));
	slim->add("bounty", new PyRepInteger(GetBounty()));
	slim->add("securityStatus", new PyRepReal(GetSecurityRating()));
	
	return(slim);
}

void Client::WarpTo(const GPoint &to, double distance) {
	if(m_moveState != msIdle || m_moveTimer.Enabled()) {
		_log(CLIENT__ERROR, "%s: WarpTo called when a move is already pending. Ignoring.", GetName());
		return;
	}

	m_destiny->WarpTo(to, distance);
	//TODO: OnModuleAttributeChange with attribute 18 for capacitory charge
}

void Client::StargateJump(uint32 fromGate, uint32 toGate) {
	if(m_moveState != msIdle || m_moveTimer.Enabled()) {
		_log(CLIENT__ERROR, "%s: StargateJump called when a move is already pending. Ignoring.", GetName());
		return;
	}

	//TODO: verify that they are actually close to 'fromGate'
	//TODO: verify that 'fromGate' actually jumps to 'toGate'
	
	uint32 solarSystemID, constellationID, regionID;
	GPoint position;
	if(!m_services.serviceDB().GetStaticItemInfo(
		toGate,
		&solarSystemID, &constellationID, &regionID, &position
	)) {
		codelog(CLIENT__ERROR, "%s: Failed to query information for stargate %u", GetName(), toGate);
		return;
	}
	
	m_moveSystemID = solarSystemID;
	m_movePoint = position;
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
	if(!GetChar()->AlterBalance(amount))
		return false;
	
	//send notification of change
	OnAccountChange ac;
	ac.accountKey = "cash";
	ac.ownerid = GetCharacterID();
	ac.balance = GetBalance();
	PyRepTuple *answer = ac.FastEncode();
	SendNotification("OnAccountChange", "cash", &answer, false);

	return true;
}



bool Client::SelectCharacter(uint32 char_id) {
	//get char
	Character *character = m_services.item_factory.GetCharacter(char_id, true);
	if(character == NULL)
		return false;

	Ship *ship = m_services.item_factory.GetShip(character->locationID(), true);
	if(ship == NULL)
		return false;

	m_char = character;	//ref is stored
	BoardShip(ship);	//updates modules
	ship->DecRef();

	if(!EnterSystem())
		return false;

	// update skill queue
	GetChar()->UpdateSkillQueue();

	SessionSync();

	//johnsus - characterOnline mod
	m_services.serviceDB().SetCharacterOnlineStatus(GetCharacterID(), true);

	return true;
}

void Client::UpdateSkillTraining()
{
	Character *ch = GetChar();
	if( ch == NULL )
		m_timeEndTrain = 0;
	else
		m_timeEndTrain = ch->GetEndOfTraining();
}

double Client::GetPropulsionStrength() const {
	if(GetShip() == NULL)
		return(3.0f);
	//just making shit up, I think skills modify this, as newbies
	//tend to end up with 3.038 instead of the base 3.0 on their ship..
	double res;
	res =  GetShip()->propulsionFusionStrength();
	res += GetShip()->propulsionIonStrength();
	res += GetShip()->propulsionMagpulseStrength();
	res += GetShip()->propulsionPlasmaStrength();
	res += GetShip()->propulsionFusionStrengthBonus();
	res += GetShip()->propulsionIonStrengthBonus();
	res += GetShip()->propulsionMagpulseStrengthBonus();
	res += GetShip()->propulsionPlasmaStrengthBonus();
	res += 0.038f;
	return res;
}

void Client::TargetAdded(SystemEntity *who) {
	PyRepTuple *up = NULL;

	DoDestiny_OnDamageStateChange odsc;
	odsc.entityID = who->GetID();
	odsc.state = who->MakeDamageState();
	up = odsc.FastEncode();
	QueueDestinyUpdate(&up);
	delete up;
	
	Notify_OnTarget te;
	te.mode = "add";
	te.targetID = who->GetID();
	up = te.FastEncode();
	QueueDestinyEvent(&up);
	delete up;
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
	if(GetShip() == NULL || m_destiny == NULL) {
		_log(CLIENT__TRACE, "%s: Unable to save position. We are probably not in space.", GetName());
		return;
	}
	GetShip()->Relocate( m_destiny->GetPosition() );
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
		return false;
	}

	_log(CLIENT__MESSAGE, "%s: Launching drone %u", GetName(), drone->itemID());
	
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
		du.controllerOwnerID = GetShip()->ownerID();
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

	return false;
}

//assumes that the backend DB stuff was already done.
void Client::JoinCorporationUpdate(uint32 corp_id) {
	GetChar()->JoinCorporation(corp_id);
	
	//logs indicate that we need to push this update out asap.
	SessionSync();
}

/************************************************************************/
/* character notification messages wrapper                              */
/************************************************************************/
void Client::OnCharNoLongerInStation()
{
	NotifyOnCharNoLongerInStation packet;

	packet.charID = GetCharacterID();
	packet.corpID = GetCorporationID();
	packet.allianceID = GetAllianceID();
	PyRepTuple *tmp = packet.Encode();

	// this entire line should be something like this Broadcast("OnCharNoLongerInStation", "stationid", &tmp);
	services().entity_list.Broadcast("OnCharNoLongerInStation", "stationid", &tmp);
}

/* besides broadcasting the message this function should handle everything for this event */
void Client::OnCharNowInStation()
{
	NotifyOnCharNowInStation n;
	n.charID = GetCharacterID();
	n.corpID = GetCorporationID();
	n.allianceID = GetAllianceID();
	PyRepTuple *tmp = n.Encode();
	services().entity_list.Broadcast("OnCharNowInStation", "stationid", &tmp);
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
			return true;
		}
	}
	return false;
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

