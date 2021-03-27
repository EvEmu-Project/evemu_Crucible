/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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
    Author:        Zhur
*/

#include "eve-server.h"
#include "EVEVersion.h"

#include "Client.h"
#include "ConsoleCommands.h"
#include "chat/LSCChannel.h"
#include "chat/LSCService.h"


PyRep *LSCChannelChar::Encode() const {
    ChannelJoinChannelCharsLine line;

    line.allianceID = m_allianceID;
    line.charID = m_charID;
    line.corpID = m_corpID;
    line.role = m_role;
    line.warFactionID = m_warFactionID;
    line.mode = m_mode;

    // extra is a list of charID, charName, charTypeID, unknown bool, unknown PyNone
    util_Row rs;
    rs.header.push_back("ownerID");
    rs.header.push_back("ownerName");
    rs.header.push_back("typeID");
    rs.line = new PyList();
    rs.line->AddItemInt( m_charID );
    rs.line->AddItemString( m_charName.c_str() );
    rs.line->AddItemInt( 1378 );    // fix this ... wtf is this??
    line.extra = rs.Encode();//m_extra;

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

LSCChannel::LSCChannel(LSCService* svc, int32 channelID, LSC::Type type, uint32 ownerID, const char* displayName, const char* comparisonKey, std::string motd,
                       bool memberless, const char* password, bool mailingList, uint32 cspa, bool temporary, bool languageRestriction, int32 groupMessageID, int32 channelMessageID)
: m_service(svc),
  m_ownerID(ownerID),
  m_channelID(channelID),
  m_type(type),
  m_motd(motd),
  m_displayName(displayName==nullptr?"":displayName),
  m_comparisonKey(comparisonKey==nullptr?"":comparisonKey),
  m_memberless(memberless),
  m_password(password==nullptr?"":password),
  m_mailingList(mailingList),
  m_cspa(cspa),
  m_temporary(temporary),
  m_languageRestriction(languageRestriction),
  m_groupMessageID(groupMessageID),
  m_channelMessageID(channelMessageID)
{
    m_mode = LSC::Mode::chConversationalist;    // default mode '3' for enabling all speakers (till i figure out how to correctly set/change later)
    _log(LSC__CHANNELS, "Creating channel %u - \"%s\"", m_channelID, (m_displayName == "") ? ((m_comparisonKey == "") ? "null" : m_comparisonKey.c_str()) : m_displayName.c_str());
}

LSCChannel::~LSCChannel() {
    _log(LSC__CHANNELS, "Destroying channel %u - \"%s\"", m_channelID, (m_displayName == "") ? ((m_comparisonKey == "") ? "null" : m_comparisonKey.c_str()) : m_displayName.c_str());
}

void LSCChannel::GetChannelInfo(int32* channelID, uint32* ownerID, std::string& displayName, std::string& motd, std::string& comparisonKey, bool* memberless, std::string& password, bool* mailingList, uint32* cspa, uint32* temporary)
{
    *channelID = m_channelID;
    *ownerID = m_ownerID;
    displayName = m_displayName;
    motd = m_motd;
    comparisonKey = m_comparisonKey;
    *memberless = m_memberless;
    password = m_password;
    *mailingList = m_mailingList;
    *cspa = m_cspa;
    *temporary = m_temporary;
}

bool LSCChannel::JoinChannel(Client* pClient) {
    /** @todo determine moderator/other rights for given char in this channel and set Mode accordingly */
    m_chars.insert(
        std::make_pair(
            pClient->GetCharacterID(),
            LSCChannelChar( this, pClient->GetCorporationID(), pClient->GetCharacterID(), pClient->GetCharName(), pClient->GetAllianceID(), pClient->GetWarFactionID(), pClient->GetAccountRole(), 0,\
            (m_ownerID == pClient->GetCharacterID() ? LSC::Mode::chCreator : LSC::Mode::chConversationalist))
        )
    );
    pClient->ChannelJoined( this );

    OnLSC_JoinChannel join;
        join.sender = _MakeSenderInfo(pClient);
        join.member_count = (int32)m_chars.size();
        join.channelID = EncodeID();

    MulticastTarget mct;
    std::map<uint32, LSCChannelChar>::iterator cur;
    cur = m_chars.begin();
    for( ; cur != m_chars.end(); ++cur )
        mct.characters.insert( cur->first );
    PyTuple *answer = join.Encode();
    sEntityList.Multicast( "OnLSC", GetTypeString(), &answer, mct );

    _log(LSC__CHANNELS, "%s Joined Channel %u - %s", pClient->GetName(), m_channelID, m_displayName.c_str());
    return true;
}

void LSCChannel::LeaveChannel(Client *pClient)
{
    if (sConsole.IsShutdown())
        return;

    if (m_chars.empty())
        return;

    uint32 charID = pClient->GetCharacterID();
    if (m_chars.find(charID) == m_chars.end())
        return;

    m_chars.erase(charID);

    MulticastTarget mct;
    std::map<uint32, LSCChannelChar>::iterator cur;
    cur = m_chars.begin();
    for(; cur != m_chars.end(); ++cur)
        mct.characters.insert( cur->first );

    OnLSC_LeaveChannel leave;
    leave.sender = _MakeSenderInfo(pClient);
    leave.member_count = (int32)m_chars.size();
    leave.channelID = EncodeID();

    PyTuple *answer = leave.Encode();
    sEntityList.Multicast("OnLSC", GetTypeString(), &answer, mct);

    _log(LSC__CHANNELS, "%s Left Channel %u - %s", pClient->GetName(), m_channelID, m_displayName.c_str());

    pClient->ChannelLeft(this);

    /** @todo delete channel from system if non-static and empty */
}

void LSCChannel::Evacuate(Client * c) {
    OnLSC_DestroyChannel dc;

    dc.channelID = EncodeID();
    dc.member_count = 0;
    dc.sender = _MakeSenderInfo(c);

    MulticastTarget mct;

    std::map<uint32, LSCChannelChar>::iterator cur;
    cur = m_chars.begin();
    for(; cur != m_chars.end(); cur++)
        mct.characters.insert(cur->first);

    PyTuple *answer = dc.Encode();
    sEntityList.Multicast("OnLSC", GetTypeString(), &answer, mct);
}

void LSCChannel::SendMessage(Client * c, const char * message, bool self/*false*/) {
// to send system msgs, senderID should be 1 (system owner)

    MulticastTarget mct;
    if (self) {
        mct.characters.insert(c->GetCharacterID());
    } else {
        std::map<uint32, LSCChannelChar>::iterator itr = m_chars.begin();
        for(; itr != m_chars.end(); ++itr)
            mct.characters.insert( itr->first );
    }

    OnLSC_SendMessage sm;
    sm.sender = _MakeSenderInfo(c);
    sm.channelID = EncodeID();
    sm.message = message;
    sm.member_count = m_chars.size();

    PyTuple *answer = sm.Encode();
    sEntityList.Multicast("OnLSC", GetTypeString(), &answer, mct);
}

void LSCChannel::SendServerMOTD(Client* pClient) {
    std::string uptime;
    sEntityList.GetUpTime(uptime);
    std::string msg = "<br>Welcome to EVEmu Server";
    //msg += pClient->GetCharName();
    msg += ".<br>Server Version: ";
    msg += EVEMU_REVISION;
    msg += "<br>Revision Date: ";
    msg += EVEMU_BUILD_DATE;
    msg += "<br>Uptime: ";
    msg += uptime;
    msg += "<br>Current Population: ";
    msg += std::to_string(sEntityList.GetClientCount());
    msg += "<br><br>Character Options:";
    msg += "<br>Module AutoStop: ";
    msg += (pClient->AutoStop() ? "On" : "Off");
    msg += "<br>RAM Event: On";
    //msg += (pClient->RAMEvent() ? "On" : "Off");
    // check account roles for this one
    msg += "<br>ShowAll: Off";
    //msg += (pClient->ShowAll() ? "On" : "Off");

    OnLSC_SendMessage sm;
    sm.sender = _FakeSenderInfo();
    sm.channelID = EncodeID();
    sm.message = msg;
    sm.member_count = m_chars.size();

    PyTuple *answer = sm.Encode();
    pClient->SendNotification("OnLSC", GetTypeString(), &answer);
}

bool LSCChannel::IsJoined(uint32 charID) {
    return (m_chars.find(charID) != m_chars.end());
}

void LSCChannel::UpdateConfig()
{
    // ********** TODO **********
    // Figure out how to send a packet to all clients subscribed to this channel that contains all channel parameters
    // so that their clients can update everything that has changed in this channel's access control.
    // **************************

}


OnLSC_SenderInfo *LSCChannel::_MakeSenderInfo(Client *c) {
    OnLSC_SenderInfo *sender = new OnLSC_SenderInfo;
        sender->senderID = c->GetCharacterID();
        sender->senderName = c->GetName();
        sender->senderType = c->GetChar()->typeID();
        sender->corpID = c->GetCorporationID();
        sender->role = c->GetAccountRole();
        sender->corp_role = c->GetCorpRole();
    return sender;
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

PyRep *LSCChannel::EncodeID() {
    switch (m_type) {
        case LSC::Type::global:
        case LSC::Type::corp:
        case LSC::Type::region:
        case LSC::Type::constellation:
        case LSC::Type::solarsystem:
        case LSC::Type::solarsystem2:
        case LSC::Type::fleet:
        case LSC::Type::wing:
        case LSC::Type::squad: {
            LSCChannelMultiDesc desc;
                desc.type = GetTypeString();
                desc.id = m_channelID;
            return desc.Encode();
        }
        case LSC::Type::character:
        case LSC::Type::alliance:
        case LSC::Type::warfaction:
        case LSC::Type::incursion:
        case LSC::Type::normal:
        case LSC::Type::custom: {
            return new PyInt(m_channelID);
        }
    }
    return new PyInt(m_channelID);
}

PyRep *LSCChannel::EncodeStaticChannel(uint32 charID) {
    ChannelInfoLine line;
        line.channelID = m_channelID;
        line.ownerID = m_ownerID;
        line.displayName = m_displayName;
        line.motd = m_motd;
        line.comparisonKey = m_comparisonKey;
        line.memberless = m_memberless;
        line.password = m_password;
        line.mailingList = m_mailingList;
        line.cspa = m_cspa;
        line.temporary = m_temporary;
        line.languageRestriction = m_languageRestriction;
        line.mode = (int8)m_mode;   // determine calling char's mode for this channel
        //HACK auto-subscribe to system channels.  TODO determine if calling char is subscribed to this channel
        line.subscribed = IsStaticChannel(m_channelID);
        line.groupMessageID = m_groupMessageID;
        line.channelMessageID = m_channelMessageID;
        line.estimatedMemberCount = m_chars.size();
    return line.Encode();
}

PyRep *LSCChannel::EncodeDynamicChannel(uint32 charID) {
    ChannelJoinChannelInfo info;
        info.channelID = m_channelID;
        info.comparisonKey = m_comparisonKey;
        info.cspa = m_cspa;
        info.displayName = m_displayName;
        info.mailingList = m_mailingList;
        info.memberless = m_memberless;
        info.motd = m_motd;
        info.ownerID = m_ownerID;
        info.password = m_password;
        //HACK auto-subscribe to system channels.  TODO determine if calling char is subscribed to this channel
        info.subscribed = IsStaticChannel(m_channelID);
        info.temporary = m_temporary;
        info.mode = (int8)m_mode;   // determine calling char's mode for this channel
        info.languageRestriction = m_languageRestriction;
        info.groupMessageID = m_groupMessageID;
        info.channelMessageID = m_channelMessageID;
        info.estimatedMemberCount = m_chars.size();
    return info.Encode();
}

PyRep *LSCChannel::EncodeChannelMods()
{
    ChannelJoinChannelMods info;
    info.lines = new PyList();
    std::map<uint32, LSCChannelMod>::iterator itr = m_mods.begin();
    for(; itr != m_mods.end(); itr++)
        info.lines->AddItem( itr->second.Encode() );
    return info.Encode();
}

PyRep *LSCChannel::EncodeChannelChars() {
    /*
        [PyObjectData Name: util.Rowset]
          [PyDict 3 kvp]
            [PyString "header"]
            [PyList 6 items]
              [PyString "charID"]
              [PyString "corpID"]
              [PyString "allianceID"]
              [PyString "warFactionID"]
              [PyString "role"]
              [PyString "extra"]
            [PyString "RowClass"]
            [PyToken util.Row]
            [PyString "lines"]
            [PyList 84 items]
            */
    ChannelJoinChannelChars info;
    info.lines = new PyList();
    std::map<uint32, LSCChannelChar>::iterator itr = m_chars.begin();
    for(; itr != m_chars.end(); itr++)
        info.lines->AddItem( itr->second.Encode() );
    return info.Encode();
}

PyRep *LSCChannel::EncodeEmptyChannelChars() {
    ChannelJoinChannelChars info;
    info.lines = new PyList();
    return info.Encode();
}

const char *LSCChannel::GetTypeString() {
    switch(m_type) {
        case LSC::Type::normal:         return "normal";
        case LSC::Type::custom:         return "private";
        case LSC::Type::corp:           return "corpid";
        case LSC::Type::solarsystem2:   return "solarsystemid2";
        case LSC::Type::region:         return "regionid";
        case LSC::Type::constellation:  return "constellationid";
        case LSC::Type::global:         return "global";
        case LSC::Type::alliance:       return "allianceid";
        case LSC::Type::fleet:          return "fleetid";
        case LSC::Type::squad:          return "squadid";
        case LSC::Type::wing:           return "wingid";
        case LSC::Type::warfaction:     return "warfactionid";
        case LSC::Type::character:      return "charid";
        case LSC::Type::solarsystem:    return "solarsystemid";
        case LSC::Type::incursion:      return "incursion";
        default:                        return "unknown";
    }
}

