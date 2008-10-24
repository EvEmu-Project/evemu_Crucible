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


PyCallable_Make_InnerDispatcher(LSCService)


LSCService::LSCService(PyServiceMgr *mgr, DBcore *db, CommandDispatcher* cd)
: PyService(mgr, "LSC"),
  m_dispatch(new Dispatcher(this)),
  m_commandDispatch(cd),
  m_db(db),
  nextFreeChannelID(200000000)
{
	_SetCallDispatcher(m_dispatch);

	//make sure you edit the header file too
	PyCallable_REG_CALL(LSCService, GetChannels)
	PyCallable_REG_CALL(LSCService, GetRookieHelpChannel)
	PyCallable_REG_CALL(LSCService, JoinChannels)
	PyCallable_REG_CALL(LSCService, LeaveChannels)
	PyCallable_REG_CALL(LSCService, LeaveChannel)
	PyCallable_REG_CALL(LSCService, CreateChannel)
	PyCallable_REG_CALL(LSCService, DestroyChannel)
	PyCallable_REG_CALL(LSCService, SendMessage)
	
	PyCallable_REG_CALL(LSCService, GetMyMessages)
	PyCallable_REG_CALL(LSCService, GetMessageDetails)
	PyCallable_REG_CALL(LSCService, Page)
	PyCallable_REG_CALL(LSCService, MarkMessagesRead)
	PyCallable_REG_CALL(LSCService, DeleteMessages)

	InitiateStaticChannels();
}

LSCService::~LSCService() {
	delete m_dispatch;
	std::map<uint32, LSCChannel *>::iterator cur, end;
	cur = m_channels.begin();
	end = m_channels.end();
	for(; cur != end; cur++) {
		delete cur->second;
	}
}

PyResult LSCService::Handle_GetChannels(PyCallArgs &call) {
	/*
		Assume this is only called when the char's logging in.
		Next step from the client is to join to all channels that's been sent by this
		So only send back the relevant channelIDs
		These are:
			- Help/Rookie, which is 1
			- character
			- corporation
			- solar system
			- region
			- constellation

			and later on:
			- allianceID
			- gangID

			also there are things like:
			- empireID
	*/
	ChannelInfo info;
	std::vector<std::string> names;

	Client * c = call.client;
	m_db.GetChannelNames(call.client->GetCharacterID(), names);



	uint32 channelID;
	uint32 charID;
	charID = channelID = c->GetCharacterID();

	if 
		(m_channels.find(channelID) == m_channels.end())
		CreateChannel(channelID, call.client->GetChar().name.c_str(), LSCChannel::normal, true);
	
	channelID = c->GetCorporationID();
	if
		(m_channels.find(channelID) == m_channels.end()) 
		CreateChannel(channelID, "System Channels\\Corp", names[1].c_str(), LSCChannel::corp);

	channelID = c->GetSystemID();
	if
		(m_channels.find(channelID) == m_channels.end())
		CreateChannel(channelID, "System Channels\\Local", names[2].c_str(), LSCChannel::solarsystem);

	channelID = c->GetConstellationID();
	if
		(m_channels.find(channelID) == m_channels.end())
		CreateChannel(channelID, "System Channels\\Constellation", names[3].c_str(), LSCChannel::constellation);

	channelID = c->GetRegionID();
	if
		(m_channels.find(channelID) == m_channels.end())
		CreateChannel(channelID, "System Channels\\Region", names[4].c_str(), LSCChannel::region);

	std::map<uint32, LSCChannel *>::iterator cur = m_channels.begin(), end = m_channels.end();
	for (;cur!=end;cur++) {
		info.lines.add(cur->second->EncodeChannel(charID));
	}


	return info.Encode();
}

PyResult LSCService::Handle_GetRookieHelpChannel(PyCallArgs &call) {
	return(new PyRepInteger(1));
}

PyResult LSCService::Handle_JoinChannels(PyCallArgs &call) {
	CallJoinChannels args;
	
	std::set<uint32> toJoin;

	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(NULL);
	}
	
	PyRepList::const_iterator cur = args.channels.begin(), end = args.channels.end();
	PyRepTuple * prt;

	for (;cur!=end;cur++) {
		if ((*cur)->CheckType(PyRep::Integer)) {
			toJoin.insert(((PyRepInteger *)(*cur))->value);
		} else if ((*cur)->CheckType(PyRep::Tuple)) {
			prt = (PyRepTuple*)*cur;
			if (prt->items.size() != 1 || !prt->items[0]->CheckType(PyRep::Tuple)) {
				codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
				continue;
			}
			prt = (PyRepTuple*)prt->items[0];
			if (prt->items.size() != 2 || /* !prt->items[0]->CheckType(PyRep::String) || unnessecary */ !prt->items[1]->CheckType(PyRep::Integer)) {
				codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
				continue;
			}
			toJoin.insert(((PyRepInteger*)prt->items[1])->value);
		} else {
			codelog(SERVICE__ERROR, "%s: Bad argument ", call.client->GetName());
			return NULL;
		}
	}
	

	uint32 channelID;
	uint32 charID = call.client->GetCharacterID();
		
	// and now ensure the working of the system
	toJoin.insert(charID);
		
	PyRepList *ml = new PyRepList();
	ChannelJoinReply * rep;
		
	LSCChannel * channel;

	std::set<uint32>::iterator curs, ends;
	curs = toJoin.begin();
	ends = toJoin.end();

	for (;curs!=ends;curs++) {
		channelID = *curs;
		if (m_channels.find(channelID) == m_channels.end())
			channel = CreateChannel(channelID);
		else
			channel = m_channels[channelID];

		if (!channel->IsJoined(charID)) {
			rep = new ChannelJoinReply();
		
			rep->ChannelID = channel->EncodeID();
			rep->ChannelInfo = channel->EncodeChannelSmall(charID);
			// this one'll create an empty query result
			// noone implemented channel mods.
			rep->ChannelMods = channel->EncodeChannelMods();
			//rep->ChannelChars = channel->EncodeChannelChars();
			rep->ChannelChars = channel->EncodeEmptyChannelChars();
		
			channel->JoinChannel(call.client);
			ml->add(rep->Encode());
		}
	}
	
	return ml;
}

PyResult LSCService::Handle_LeaveChannels(PyCallArgs &call) {
	CallLeaveChannels args;

	if(!args.Decode(&call.tuple)) {
		_log(LSC__ERROR, "Unable to parse arguments to LeaveChannels");
		return NULL;
	}
		
	std::set<uint32> toLeave;

	{
        PyRepList::iterator cur, end;
		cur = args.channels.begin();
		end = args.channels.end();

		PyRepTuple * prt;

		for(;cur!=end;cur++) {
			if ((*cur)->CheckType(PyRep::Integer)) {
				toLeave.insert(((PyRepInteger*)(*cur))->value);
			} else if ((*cur)->CheckType(PyRep::Tuple)) {
				prt = (PyRepTuple*)(*cur);
				if (prt->items[0]->CheckType(PyRep::Integer)) {
					toLeave.insert(((PyRepInteger*)prt->items[0])->value);
					continue;
				}
				if (!prt->items[0]->CheckType(PyRep::Tuple)) {
					codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
					continue;
				}
				prt = (PyRepTuple*)prt->items[0];
				if (prt->items[0]->CheckType(PyRep::Tuple)) {
					prt = (PyRepTuple*)prt->items[0];
				}
				if (prt->items.size() != 2 || !prt->items[1]->CheckType(PyRep::Integer)) {
					codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
					continue;
				}
				toLeave.insert(((PyRepInteger*)prt->items[1])->value);
				} else {
					codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
					continue;
				}
			}
		}

	{
		std::set<uint32>::iterator cur = toLeave.begin(), end = toLeave.end();
		for (;cur!=end;cur++) {
			if (m_channels.find(*cur) != m_channels.end()) {
				m_channels[*cur]->LeaveChannel(call.client);
			}
		}
	}
	
	return (new PyRepNone());
}
void LSCService::CharacterLogout(uint32 charID, OnLSC_SenderInfo * si) {
	std::map<uint32, LSCChannel*>::iterator cur = m_channels.begin(), end = m_channels.end();
	for (;cur!=end;cur++) {
		if ((*cur).second->IsJoined(charID)) (*cur).second->LeaveChannel(charID, si->Clone());
	}
	delete si;
}

PyResult LSCService::Handle_LeaveChannel(PyCallArgs &call) {
	CallLeaveChannel arg;
	if (!arg.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return NULL;
	}

	uint32 toLeave;
	PyRepTuple * prt;
		
	if (arg.channel->CheckType(PyRep::Integer)) {
		toLeave = ((PyRepInteger*)(arg.channel))->value;
	} else if (arg.channel->CheckType(PyRep::Tuple)) {
		prt = (PyRepTuple*)(arg.channel);
		if (prt->items[0]->CheckType(PyRep::Integer)) {
			toLeave = ((PyRepInteger*)prt->items[0])->value;
	} else {
			if (!prt->items[0]->CheckType(PyRep::Tuple)) {
				codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
				return NULL;
			}
			prt = (PyRepTuple*)prt->items[0];
			if (prt->items.size() != 2 || !prt->items[1]->CheckType(PyRep::Integer)) {
				codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
				return NULL;
			}
			toLeave = ((PyRepInteger*)prt->items[1])->value;
		}
	} else {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return NULL;
	}
	

	if (m_channels.find(toLeave) != m_channels.end()) {
		m_channels[toLeave]->LeaveChannel(call.client);
	}

	return NULL;
}
PyResult LSCService::Handle_CreateChannel(PyCallArgs &call) {
	Call_SingleStringArg name;
	if (!name.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return NULL;
	}

	LSCChannel * channel;
	channel = CreateChannel(name.arg.c_str());
	
	channel->JoinChannel(call.client);
	ChannelCreateReply reply;
	reply.ChannelChars = channel->EncodeChannelChars();
	reply.ChannelInfo = channel->EncodeChannelSmall(call.client->GetCharacterID());
	reply.ChannelMods = channel->EncodeChannelMods();

	return reply.Encode();
}
PyResult LSCService::Handle_DestroyChannel(PyCallArgs &call) {
	Call_SingleIntegerArg arg;
	if (!arg.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return NULL;
	}
	
	if (m_channels.find(arg.arg) == m_channels.end()) {
		return new PyRepNone();
	}
	
	m_channels[arg.arg]->Evacuate(call.client);
	delete m_channels[arg.arg];
	m_channels.erase(arg.arg);

	return new PyRepNone();
}
PyResult LSCService::Handle_SendMessage(PyCallArgs &call) {
	uint32 channelID;
	PyRepTuple * arg = call.tuple;

	if (arg->items.size() != 2 || !arg->items[0]->CheckType(PyRep::Tuple) && !arg->items[1]->CheckType(PyRep::String)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments (T0)", call.client->GetName());
		return new PyRepInteger(0);
	}
	
	const char * message = ((PyRepString*)arg->items[1])->value.c_str();
	arg = (PyRepTuple *)arg->items[0];

	
	if (arg->CheckType(PyRep::Integer)) {
		channelID = ((PyRepInteger*)arg)->value;
	} else if (arg->CheckType(PyRep::Tuple)) {
		if (arg->items.size() != 1 || !arg->items[0]->CheckType(PyRep::Tuple)) {
			codelog(SERVICE__ERROR, "%s: Bad arguments (T1)", call.client->GetName());
			return new PyRepInteger(0);
		}
		arg = (PyRepTuple *)arg->items[0];
		if (arg->items.size() != 2 || !arg->items[1]->CheckType(PyRep::Integer)) {
			codelog(SERVICE__ERROR, "%s: Bad arguments (T2)", call.client->GetName());
			return new PyRepInteger(0);
		}
		channelID = ((PyRepInteger*)arg->items[1])->value;
	} else {
		codelog(SERVICE__ERROR, "%s: Bad arguments (T3)", call.client->GetName());
		return new PyRepInteger(0);
	}

	if (m_channels.find(channelID) == m_channels.end()) {
		codelog(SERVICE__ERROR, "%s: Couldn't find channel %lu", call.client->GetName(), channelID);
		return new PyRepInteger(0);
	}
	LSCChannel * channel = m_channels[channelID];

	channel->SendMessage(call.client, message);
	
	return new PyRepInteger(1);
}

	
	


	

	

const int cmode = 2950;

LSCChannel *LSCService::CreateChannel(uint32 channelID, const char * name, const char * motd, LSCChannel::Type type, bool maillist) {
	return m_channels[channelID] = new LSCChannel(this, channelID, type, channelID, name, motd, NULL, true, "", maillist, true, false, cmode);
}
LSCChannel *LSCService::CreateChannel(uint32 channelID, const char * name, LSCChannel::Type type, bool maillist) {
	return m_channels[channelID] = new LSCChannel(this, channelID, type, channelID, name, NULL, NULL, true, "", maillist, true, false, cmode);
}
LSCChannel *LSCService::CreateChannel(uint32 channelID) {
	LSCChannel::Type type;
	std::string name;
	std::string motd;
	if (IsRegion(channelID)) { type = LSCChannel::region; name = "System Channels\\Region"; motd = m_db.GetRegionName(channelID); }
	else if (IsConstellation(channelID)) {type = LSCChannel::constellation; name = "System Channels\\Constellation"; motd = m_db.GetConstellationName(channelID); }
	else if (IsSolarSystem(channelID)) { type = LSCChannel::solarsystem; name = "System Channels\\Local"; motd = m_db.GetSolarSystemName(channelID); }
	// official corporations
	else if ((channelID >= 1000000) && (channelID < 2000000)) { type = LSCChannel::corp; name = "System Channels\\Corp"; motd = m_db.GetCorporationName(channelID); }
	// player-created corporations
	else if ((channelID >= 2000000) && (channelID < 3000000)) { type = LSCChannel::corp; name = "System Channels\\Corp"; motd = m_db.GetCorporationName(channelID); }
	else { type = LSCChannel::normal; name = m_db.GetChatacterName(channelID); motd = ""; } // this is either a char channel or something i have no idea about

	return m_channels[channelID] = new LSCChannel(this, channelID, type, channelID, name.c_str(), motd.c_str(), NULL, true, NULL, false, true, false, cmode);
}
LSCChannel *LSCService::CreateChannel(uint32 channelID, LSCChannel::Type type) {
	std::string name;
	std::string motd;
	if (IsRegion(channelID)) { name = "System Channels\\Region"; motd = m_db.GetRegionName(channelID); }
	else if (IsConstellation(channelID)) { name = "System Channels\\Constellation"; motd = m_db.GetConstellationName(channelID); }
	else if (IsSolarSystem(channelID)) { name = "System Channels\\Local"; motd = m_db.GetSolarSystemName(channelID); }
	// official corporations
	else if ((channelID >= 1000000) && (channelID < 2000000)) { name = "System Channels\\Corp"; motd = m_db.GetCorporationName(channelID); }
	// player-created corporations
	else if ((channelID >= 2000000) && (channelID < 3000000)) { name = "System Channels\\Corp"; motd = m_db.GetCorporationName(channelID); }
	// this is either a char channel or something i have no idea about
	else { name = m_db.GetChatacterName(channelID); motd = ""; }
	
	return m_channels[channelID] = new LSCChannel(this, channelID, type, channelID, name.c_str(), motd.c_str(), NULL, true, NULL, false, true, false, cmode);
}
LSCChannel *LSCService::CreateChannel(const char * name, bool maillist) {
	return CreateChannel(nextFreeChannelID++, name, LSCChannel::normal, maillist);
}
void LSCService::CreateSystemChannel(uint32 systemID) {
	if (m_channels.find(systemID) == m_channels.end()) {
		CreateChannel(systemID, "System Channels\\Local", m_db.GetSolarSystemName(systemID).c_str(), LSCChannel::solarsystem);
	}
}
void LSCService::InitiateStaticChannels() {
	// maybe, but only maybe, this stuff could be replaced by a nice database call
	CreateChannel(1, "Help\\Rookie Help", "Rookie motd", LSCChannel::normal);
	CreateChannel(2, "Help\\Help", "Help motd", LSCChannel::normal);
}

	
PyResult LSCService::Handle_GetMyMessages(PyCallArgs &call) {
	return(m_db.GetMailHeaders(call.client->GetCharacterID()));
}

PyResult LSCService::Handle_GetMessageDetails(PyCallArgs &call) {
	Call_TwoIntegerArgs args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(NULL);
	}

	//TODO: verify ability to read this message...

	return(m_db.GetMailDetails(args.arg2, args.arg1));
}

PyResult LSCService::Handle_Page(PyCallArgs &call) {
	Call_Page args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(NULL);
	}

	_log(SERVICE__MESSAGE, "%s: Received evemail msg with subject '%s': %s", call.client->GetName(), args.subject.c_str(), args.body.c_str());
	
	SendMail(call.client->GetCharacterID(), args.recipients, args.subject, args.body);
	
	return(NULL);
}

//stuck here to be close to related functionality
void LSCService::SendMail(uint32 sender, const std::vector<uint32> &recipients, const std::string &subject, const std::string &content) {
	NotifyOnMessage notify;
	std::set<uint32> successful_recipients;
	
	notify.subject = subject;
	notify.sentTime = Win32TimeNow();
	notify.senderID = sender;

	// there's attachmentID and messageID... does this means a single message can contain multiple attachments?
	// eg. text/plain and text/html? we should be watching for this at reading mails...
	// created should be creation time. But Win32TimeNow returns uint64, and is stored as bigint(20),
	// so change in the db is needed
	std::vector<uint32>::const_iterator cur, end;
	cur = recipients.begin();
	end = recipients.end();

	for(; cur != end; cur++) {
		uint32 messageID = m_db.StoreMail(sender, *cur, subject.c_str(), content.c_str(), notify.sentTime);
		if(messageID == 0) {
			_log(SERVICE__ERROR, "Failed to store message from %lu for recipient %lu", sender, *cur);
			continue;
		}
		//TODO: supposed to have a different messageID in each notify I suspect..
		notify.messageID = messageID;
		
		_log(SERVICE__MESSAGE, "Delivered message from %lu to recipient %lu", sender, *cur);
		//record this person in the 'delivered to' list:
		notify.recipients.push_back(*cur);
		successful_recipients.insert(*cur);
	}
	
	//now, send a notification to each successful recipient
	PyRepTuple *answer = notify.Encode();
	m_manager->entity_list->Multicast(successful_recipients, "OnMessage", "*multicastID", &answer, false);
}

//stuck here to be close to related functionality
//theres a lot of duplicated crap in here...
//this could be replaced by the SendNewEveMail if it weren't in the Client
void Client::SelfEveMail(const char *subject, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str = NULL;
	vaMakeAnyLenString(&str, fmt, args);
	va_end(args);

	m_services->lsc_service->SendMail(GetCharacterID(), GetCharacterID(), subject, str);
}

PyResult LSCService::Handle_MarkMessagesRead(PyCallArgs &call) {
	Call_SingleIntList args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(NULL);
	}
	
	std::vector<uint32>::iterator cur, end;
	cur = args.ints.begin();
	end = args.ints.end();
	for(; cur != end; cur++) {
		m_db.MarkMessageRead(*cur);
	}
	return(NULL);
}

PyResult LSCService::Handle_DeleteMessages(PyCallArgs &call) {
	Call_DeleteMessages args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(NULL);
	}

	if(args.channelID != call.client->GetCharacterID()) {
		_log(SERVICE__ERROR, "%s (%d) tried to delete messages in channel %lu. Denied.", call.client->GetName(), call.client->GetCharacterID(), args.channelID);
		return(NULL);
	}
	
	std::vector<uint32>::iterator cur, end;
	cur = args.messages.begin();
	end = args.messages.end();
	for(; cur != end; cur++) {
		m_db.DeleteMessage(*cur, args.channelID);
	}

	return(NULL);
}


PyResult LSCService::ExecuteCommand(Client *from, const char *msg) {
	return(m_commandDispatch->Execute(from, msg));
}






















