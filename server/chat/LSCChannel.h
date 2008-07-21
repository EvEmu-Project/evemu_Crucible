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

#ifndef __LSCCHANNEL_H_INCL__
#define __LSCCHANNEL_H_INCL__

#include "types.h"
#include "../EntityList.h"
#include "../packets/LSCPkts.h"

class PyRep;
class LSCService;
class LSCChannel;

class LSCChannelChar {
public:
	LSCChannelChar(LSCChannel * chan, uint32 corpID, uint32 charID, uint32 allianceID, uint32 role, uint32 extra) :
	  m_parent(chan),
	  m_corpID(corpID),
	  m_charID(charID),
	  m_allianceID(allianceID),
	  m_role(role),
	  m_extra(extra) { }

	virtual ~LSCChannelChar() { }
	PyRep *Encode();

protected:
	LSCChannel *m_parent;
	uint32 m_corpID;
	uint32 m_charID;
	uint32 m_allianceID;
	uint32 m_role;
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
		uint32 mailingList,
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
	uint32 m_mailingList;
	uint32 m_cspa;
	uint32 m_temporary;
	uint32 m_mode;

	std::vector<LSCChannelMod> m_mods;
	std::map<uint32, LSCChannelChar *> m_chars;	// we own these pointers


	OnLSC_SenderInfo *_FakeSenderInfo();

};




#endif


