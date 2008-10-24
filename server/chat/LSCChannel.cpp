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


PyRep *LSCChannelChar::Encode() {
	ChannelJoinChannelCharsLine line;

	line.allianceID = m_allianceID;
	line.charID = m_charID;
	line.corpID = m_corpID;
	line.extra = m_extra;
	line.role = m_role;

	return line.Encode();
}

PyRep *LSCChannelMod::Encode() {
	ChannelJoinChannelModsLine line;

	line.accessor = m_accessor;
	line.admin = m_admin;
	line.mode = m_mode;
	line.originalMode = m_originalMode;
	line.reason = m_reason;
	line.untilWhen = m_untilWhen;

	return line.Encode();
}

LSCChannel::LSCChannel(
	LSCService *svc, 
	uint32 channelID,
	Type type,
	uint32 ownerID,
	const char *displayName,
	const char *motd,
	const char *comparisonKey,
	bool memberless,
	const char *password,
	uint32 mailingList,
	uint32 cspa,
	uint32 temporary,
	uint32 mode
)
: m_service(svc),
  m_channelID(channelID),
  m_type(type),
  m_ownerID(ownerID),
  m_displayName(displayName==NULL?"":displayName),
  m_motd(motd==NULL?"":motd),
  m_comparisonKey(comparisonKey==NULL?"":comparisonKey),
  m_memberless(memberless),
  m_password(password==NULL?"":password),
  m_mailingList(mailingList),
  m_cspa(cspa),
  m_temporary(temporary),
  m_mode(mode)
{
	_log(LSC__CHANNELS, "Creating channel \"%s\"", m_displayName.c_str());
}

LSCChannel::~LSCChannel() {
	_log(LSC__CHANNELS, "Destroying channel \"%s\"", m_displayName.c_str());
}

bool LSCChannel::JoinChannel(Client * c) {
	_log(LSC__CHANNELS, "Channel %s: Join from %s", m_displayName.c_str(), c->GetName());

	m_chars[c->GetCharacterID()] = new LSCChannelChar(this, c->GetCorporationID(), c->GetCharacterID(), c->GetAllianceID(), c->GetRole(), 0);
	m_memberless = false;
	c->ChannelJoined(this);

	//if ((m_type != LSCChannel::normal) && (m_channelID > 2)) {
		OnLSC_JoinChannel join;
		join.sender = _MakeSenderInfo(c);
		join.member_count = m_chars.size();
		join.channelID = EncodeID();

		MulticastTarget mct;
		std::map<uint32, LSCChannelChar *>::iterator cur, end;
		cur = m_chars.begin(); end = m_chars.end();
		for(;cur!=end;cur++)
			mct.characters.insert((*cur).first);
		PyRepTuple * answer = join.Encode();
		m_service->GetEntityList()->Multicast("OnLSC", GetTypeString(), &answer, mct);
	//}

	return true;
}
void LSCChannel::LeaveChannel(uint32 charID, OnLSC_SenderInfo * si) {
	_log(LSC__CHANNELS, "Channel %s: Leave from %lu", m_displayName.c_str(), charID);

	if (m_chars.find(charID) == m_chars.end())
		return;

	m_chars.erase(charID);

	OnLSC_LeaveChannel leave;
	leave.sender = si;
	leave.member_count = m_chars.size();
	leave.channelID = EncodeID();

	MulticastTarget mct;
	std::map<uint32, LSCChannelChar *>::iterator cur, end;
	cur = m_chars.begin(); end = m_chars.end();
	for(;cur!=end;cur++)
		mct.characters.insert((*cur).first);
	PyRepTuple * answer = leave.Encode();
	m_service->GetEntityList()->Multicast("OnLSC", GetTypeString(), &answer, mct);
}
void LSCChannel::LeaveChannel(Client *c, bool self) {
	_log(LSC__CHANNELS, "Channel %s: Leave from %s", m_displayName.c_str(), c->GetName());

	uint32 charID = c->GetCharacterID();

	if (m_chars.find(charID) == m_chars.end())
		return;
	
	OnLSC_LeaveChannel leave;
	leave.sender = _MakeSenderInfo(c);
	leave.member_count = m_chars.size();
	leave.channelID = EncodeID();

	MulticastTarget mct;
	std::map<uint32, LSCChannelChar *>::iterator cur, end;
	cur = m_chars.begin(); end = m_chars.end();
	for(;cur!=end;cur++)
		mct.characters.insert((*cur).first);
	PyRepTuple * answer = leave.Encode();
	m_service->GetEntityList()->Multicast("OnLSC", GetTypeString(), &answer, mct);

	m_chars.erase(charID);
	c->ChannelLeft(this);
}
void LSCChannel::Evacuate(Client * c) {
	OnLSC_DestroyChannel dc;

	dc.channelID = EncodeID();
	dc.member_count = 0;
	dc.sender = _MakeSenderInfo(c);

	MulticastTarget mct;
	std::map<uint32, LSCChannelChar *>::iterator cur = m_chars.begin(), end = m_chars.end();
	for (;cur!=end;cur++) {
		mct.characters.insert(cur->first);
	}
	
	PyRepTuple *answer = dc.Encode();
	m_service->GetEntityList()->Multicast("OnLSC", GetTypeString(), &answer, mct);
}
void LSCChannel::SendMessage(Client * c, const char * message, bool self) {
	MulticastTarget mct;

	OnLSC_SendMessage sm;

	if (message[0] == '#') {
		m_service->ExecuteCommand(c, message);
		mct.characters.insert(c->GetCharacterID());
		sm.sender = _MakeSenderInfo(c);
	} else {
		if (self) {
			mct.characters.insert(c->GetCharacterID());
			sm.sender = _FakeSenderInfo();
		} else {
			std::map<uint32, LSCChannelChar *>::iterator cur = m_chars.begin(), end = m_chars.end();
			for (;cur!=end;cur++) {
				mct.characters.insert(cur->first);
			}
			sm.sender = _MakeSenderInfo(c);
	}
	}

	sm.channelID = EncodeID();
	sm.message = message;
	sm.member_count = m_chars.size();

	PyRepTuple *answer = sm.Encode();
	m_service->GetEntityList()->Multicast("OnLSC", GetTypeString(), &answer, mct);
}

	

		
	
		
	





bool LSCChannel::IsJoined(uint32 charID) {
	return m_chars.find(charID) != m_chars.end();
}

OnLSC_SenderInfo *LSCChannel::_MakeSenderInfo(Client *c) {
	OnLSC_SenderInfo *sender = new OnLSC_SenderInfo;
	
	sender->senderID = c->GetCharacterID();
	sender->senderName = c->GetName();
	sender->senderType = c->GetChar().typeID;
	sender->corpID = c->GetCorporationID();
	sender->role = c->GetRole();
	sender->corp_role = c->GetCorpInfo().corprole;

	return(sender);
}

OnLSC_SenderInfo *LSCChannel::_FakeSenderInfo() {
	OnLSC_SenderInfo *sender = new OnLSC_SenderInfo();
	
	sender->senderID = 1;
	sender->senderName = "EVE System";
	sender->senderType = 1;
	sender->corpID = 1;
	sender->role = 1;
	sender->corp_role = 1;

	return sender;
}

PyRep *LSCChannel::EncodeChannel(uint32 charID) {
	ChannelInfoLine line;

	line.channelID = m_channelID;
	line.comparisonKey = m_comparisonKey;
	line.cspa = m_cspa;
	line.displayName = m_displayName;
	line.estimatedMemberCount = m_chars.size();
	line.mailingList = m_mailingList;
	line.memberless = m_memberless;
	line.mode = m_mode;
	line.motd = m_motd;
	line.ownerID = m_ownerID;
	line.password = m_password;
	line.subscribed = !(m_chars.find(charID) == m_chars.end());
	line.temporary = m_temporary;

	return line.Encode();
}
PyRep *LSCChannel::EncodeID() {
	if (m_type == normal)
		return (new PyRepInteger(m_channelID));

	LSCChannelMultiDesc desc;
	desc.number = m_channelID;
	desc.type = GetTypeString();

	return desc.Encode();
}

PyRep *LSCChannel::EncodeChannelSmall(uint32 charID) {
	ChannelJoinChannelInfo info;

	info.channelID = m_channelID;
	info.comparisonKey = m_comparisonKey;
	info.cspa = m_cspa;
	info.displayName = m_displayName;
	info.mailingList = m_mailingList;
	info.memberless = m_memberless;
	info.motd = m_motd;
	info.ownerID = m_ownerID;
	info.password = m_password.empty() ? (PyRep*)new PyRepNone() : (PyRep*)new PyRepString(m_password);
	info.subscribed = !(m_chars.find(charID) == m_chars.end());
	info.temporary = m_temporary;
	
	return info.Encode();
}
PyRep *LSCChannel::EncodeChannelMods() {
	ChannelJoinChannelMods info;

	int i = 0, N = m_mods.size();

	for (; i < N; i++) {
		info.lines.add(m_mods[i].Encode());
	}

	return info.Encode();
}
PyRep *LSCChannel::EncodeChannelChars() {
	ChannelJoinChannelChars info;

	std::map<uint32, LSCChannelChar *>::iterator cur, end;
	cur = m_chars.begin(); end = m_chars.end();

	for (;cur!=end;cur++) {
		info.lines.add(m_chars[(*cur).first]->Encode());
	}

	return info.Encode();
}
PyRep *LSCChannel::EncodeEmptyChannelChars() {
	ChannelJoinChannelChars info;
	return info.Encode();
}

const char *LSCChannel::GetTypeString() {
	switch(m_type) {
	case normal:
		return("normal");
	case corp:
		return("corpid");
	case solarsystem:
		return("solarsystemid2");
	case region:
		return("regionid");
	case constellation:
		return("constellationid");
	//no default on purpose
	}
	return("unknown");
}













