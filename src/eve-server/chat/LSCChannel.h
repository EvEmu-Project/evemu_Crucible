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

#ifndef __LSCCHANNEL_H_INCL__
#define __LSCCHANNEL_H_INCL__

#include "EntityList.h"
#include "EVE_LSC.h"
#include "packets/LSCPkts.h"

class PyRep;
class LSCService;
class LSCChannel;

class LSCChannelChar {
public:
    LSCChannelChar(LSCChannel *chan, uint32 corpID, uint32 charID, std::string charName, uint32 allianceID, uint32 warFactionID, int64 role, uint32 extra, LSC::Mode mode) :
      m_parent(chan),
      m_corpID(corpID),
      m_charID(charID),
      m_charName(charName),
      m_allianceID(allianceID),
      m_warFactionID(warFactionID),
      m_role(role),
      m_extra(extra),
      m_mode(mode) { }

    ~LSCChannelChar() { }
    PyRep *Encode() const;

protected:
    LSCChannel *m_parent;
    uint32 m_corpID;
    uint32 m_charID;
    std::string m_charName;
    uint32 m_allianceID;
    uint32 m_warFactionID;
    int64 m_role;
    uint32 m_extra;
    LSC::Mode m_mode;
};

class LSCChannelMod {
public:
    LSCChannelMod(LSCChannel * chan, uint32 accessor, int64 untilWhen, uint32 originalMode, std::string admin, std::string reason, LSC::Mode mode) :
      m_parent(chan),
      m_accessor(accessor),
      m_mode(mode),
      m_untilWhen(untilWhen),
      m_originalMode(originalMode),
      m_admin(admin),
      m_reason(reason) { }

    ~LSCChannelMod() { }
    PyRep * Encode();

protected:
    LSCChannel * m_parent;    // we do not own this
    uint32 m_accessor;
    LSC::Mode m_mode;
    int64 m_untilWhen;
    uint32 m_originalMode;
    std::string m_admin;
    std::string m_reason;
};

class LSCChannel {
public:
    LSCChannel(
        LSCService* svc, int32 channelID, LSC::Type type, uint32 ownerID, const char* displayName, const char* comparisonKey, std::string motd,
        bool memberless, const char* password, bool mailingList, uint32 cspa, bool temporary, bool languageRestriction, int32 groupMessageID, int32 channelMessageID
        );
    ~LSCChannel();

    PyRep *EncodeID();
    PyRep *EncodeStaticChannel(uint32 charID);
    PyRep *EncodeDynamicChannel(uint32 charID);
    PyRep *EncodeChannelMods();
    PyRep *EncodeChannelChars();
    PyRep *EncodeEmptyChannelChars();

    const char *GetTypeString();
    bool JoinChannel(Client *pClient);
    void LeaveChannel(Client* pClient);
    bool IsJoined(uint32 charID);

    // this is used for updating joined clients when channel config is changed.
    void UpdateConfig();

    void Evacuate(Client * c);
    void SendMessage(Client * c, const char * message, bool self = false);
    void SendServerMOTD(Client* pClient);

    void GetChannelInfo(int32 * channelID, uint32 * ownerID, std::string &displayName, std::string &motd, std::string &comparisonKey,
            bool * memberless, std::string &password, bool * mailingList, uint32 * cspa, uint32 * temporary);

    static OnLSC_SenderInfo *_MakeSenderInfo(Client *from);

    void        SetDisplayName(std::string displayName) { m_displayName = displayName; }
    void                SetMOTD(std::string motd)       { m_motd = motd; }
    void                SetMemberless(bool memberless)  { m_memberless = memberless; }
    void              SetPassword(std::string password) { m_password = password; }


    bool                GetMemberless()                 { return m_memberless; }
    bool                GetMailingList()                { return m_mailingList; }
    bool                GetTemporary()                  { return m_temporary; }
    int32               GetGrpMsgID()                   { return m_groupMessageID; }
    int32               GetChMsgID()                    { return m_channelMessageID; }
    uint16              GetCSPA()                       { return m_cspa; }
    uint32              GetOwnerID()                    { return m_ownerID; }
    int32               GetChannelID()                  { return m_channelID; }
    uint32              GetMemberCount()                { return (uint32)m_chars.size(); }
    LSC::Type           GetType()                       { return m_type; }
    LSC::Mode           GetMode()                       { return m_mode; }
    std::string         GetDisplayName()                { return m_displayName; }
    std::string         GetMOTD()                       { return m_motd; }
    std::string         GetComparisonKey()              { return m_comparisonKey; }
    std::string         GetPassword()                   { return m_password; }

protected:
    LSCService *const   m_service;    //we do not own this

    LSC::Type           m_type;
    LSC::Mode           m_mode;
    // memberless - true = estimate member count, send estimatedMemberCount in packet.  false = actual memberList.count()   (5m refresh in client)
    // non-npc corp, fleet, and k-space local are always memberless=false
    bool                m_memberless;
    bool                m_mailingList;
    bool                m_temporary;
    bool                m_languageRestriction;
    int32               m_groupMessageID;
    int32               m_channelMessageID;
    uint16              m_cspa;
    uint32              m_ownerID;
    int32               m_channelID;            // ids < 0 are automatic conversationalist mode (or creator) and invite only (per client)
    std::string         m_displayName;
    std::string         m_motd;
    std::string         m_comparisonKey;
    std::string         m_password;

    std::map<uint32, LSCChannelMod> m_mods;
    std::map<uint32, LSCChannelChar> m_chars;

    OnLSC_SenderInfo *_FakeSenderInfo();
};

#endif
