/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2011 The EVEmu Team
	For the latest information visit http://evemu.org
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

#ifndef __LSCCHANNEL_H_INCL__
#define __LSCCHANNEL_H_INCL__

#include "EntityList.h"

class PyRep;
class LSCService;
class LSCChannel;

class LSCChannelChar {
public:
	LSCChannelChar(LSCChannel *chan, uint32 corpID, uint32 charID, std::string charName, uint32 allianceID, uint32 warFactionID, uint64 role, uint32 extra) :
	  m_parent(chan),
	  m_corpID(corpID),
	  m_charID(charID),
          m_charName(charName),
	  m_allianceID(allianceID),
	  m_warFactionID(warFactionID),
	  m_role(role),
	  m_extra(extra) { }

	virtual ~LSCChannelChar() { }
	PyRep *Encode() const;

protected:
	LSCChannel *m_parent;
	uint32 m_corpID;
	uint32 m_charID;
        std::string m_charName;
	uint32 m_allianceID;
	uint32 m_warFactionID;
	uint64 m_role;
	uint32 m_extra;
};

class LSCChannelMod {
public:
	LSCChannelMod(LSCChannel * chan, uint32 accessor, uint32 mode, uint64 untilWhen, uint32 originalMode, std::string admin, std::string reason) :
	  m_parent(chan),
	  m_accessor(accessor),
	  m_mode(mode),
	  m_untilWhen(untilWhen),
	  m_originalMode(originalMode),
	  m_admin(admin),
	  m_reason(reason) { }

	virtual ~LSCChannelMod() { }

	PyRep * Encode();

protected:
	LSCChannel * m_parent;	// we do not own this

	uint32 m_accessor;
	uint32 m_mode;
	uint64 m_untilWhen;
	uint32 m_originalMode;
	std::string m_admin;
	std::string m_reason;
};

class LSCChannel {
public:
	typedef enum {
		normal = 0,
		corp = 1,
		solarsystem = 2,
		region = 3,
		constellation = 4
	} Type; 

	LSCChannel(
		LSCService *svc, 
		uint32 channelID,
		Type type,
		uint32 ownerID,
		const char *displayName,
		const char *motd,
		const char *comparisonKey,
		bool memberless,
		const char *password,
		bool mailingList,
		uint32 cspa,
		uint32 temporary,
		uint32 mode
		);
	virtual ~LSCChannel();

	PyRep *EncodeChannel(uint32 charID);
	PyRep *EncodeID();

	PyRep *EncodeChannelSmall(uint32 charID);
	PyRep *EncodeChannelMods();
	PyRep *EncodeChannelChars();
	PyRep *EncodeEmptyChannelChars();

	const char *GetTypeString();

	uint32 GetChannelID() { return m_channelID; }
	uint32 GetOwnerID() { return m_ownerID; }
	Type GetType() { return m_type; }
	std::string GetDisplayName() { return m_displayName; }
	std::string GetMOTD() { return m_motd; }
	std::string GetComparisonKey() { return m_comparisonKey; }
	bool GetMemberless() { return m_memberless; }
	std::string GetPassword() { return m_password; }
	bool GetMailingList() { return m_mailingList; }
	uint32 GetCSPA() { return m_cspa; }
	uint32 GetTemporary() { return m_temporary; }
	uint32 GetMode() { return m_mode; }
    uint32 GetMemberCount() { return m_chars.size(); }

	void SetOwnerID(uint32 ownerID) { m_ownerID = ownerID; }
	void SetType(Type new_type) { m_type = new_type; }
	void SetDisplayName(std::string displayName) { m_displayName = displayName; }
	void SetMOTD(std::string motd) { m_motd = motd; }
	void SetComparisonKey(std::string comparisonKey) { m_comparisonKey = comparisonKey; }
	void SetMemberless(bool memberless) { m_memberless = memberless; }
	void SetPassword(std::string password) { m_password = password; }
	void SetMailingList(bool mailingList) { m_mailingList = mailingList; }
	void SetCSPA(uint32 cspa) { m_cspa = cspa; }
	void SetTemporary(uint32 temporary) { m_temporary = temporary; }
	void SetMode(uint32 mode) { m_mode = mode; }

	void GetChannelInfo(uint32 * channelID, uint32 * ownerID, std::string &displayName, std::string &motd, std::string &comparisonKey,
		bool * memberless, std::string &password, bool * mailingList, uint32 * cspa, uint32 * temporary, uint32 * mode);
	void SetChannelInfo(uint32 ownerID, std::string displayName, std::string motd, std::string comparisonKey,
		bool memberless, std::string password, bool mailingList, uint32 cspa, uint32 temporary, uint32 mode);

	bool JoinChannel(Client * c);
	void LeaveChannel(Client *c, bool self = true);
	void LeaveChannel(uint32 charID, OnLSC_SenderInfo * si);
	bool IsJoined(uint32 charID);

	void Evacuate(Client * c);
	void SendMessage(Client * c, const char * message, bool self = false);
	
	
	static OnLSC_SenderInfo *_MakeSenderInfo(Client *from);
	
protected:
	LSCService *const m_service;	//we do not own this

	//EntityList::character_set m_members;
	
	uint32 m_ownerID;
	uint32 m_channelID;
	Type m_type;
	std::string m_displayName;
	std::string m_motd;
	std::string m_comparisonKey;
	bool m_memberless;
	std::string m_password;
	bool m_mailingList;
	uint32 m_cspa;
	uint32 m_temporary;
	uint32 m_mode;

	std::vector<LSCChannelMod> m_mods;
	std::map<uint32, LSCChannelChar> m_chars;


	OnLSC_SenderInfo *_FakeSenderInfo();

};




#endif


