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

#ifndef EVE_CLIENT_SESSION_H
#define EVE_CLIENT_SESSION_H

#include "../common/logsys.h"

class SessionChangeNotification;

class ClientSession {
public:
	ClientSession();

	/*
		each attribute has a Set_* method and a Clear_* method.
	*/

	void Set_userType(uint32 v);
	void Clear_userType();
	void Set_userid(uint32 v);
	void Clear_userid();
	void Set_address(const char *v);
	void Clear_address();
	void Set_role(uint32 v);
	void Clear_role();
	void Set_languageID(const char *v);
	void Clear_languageID();
	void Set_constellationid(uint32 v);
	void Clear_constellationid();
	void Set_corpid(uint32 v);
	void Clear_corpid();
	void Set_regionid(uint32 v);
	void Clear_regionid();
	void Set_stationid(uint32 v);
	void Clear_stationid();
	void Set_solarsystemid(uint32 v);
	void Clear_solarsystemid();
	void Set_locationid(uint32 v);
	void Clear_locationid();
	void Set_gangrole(uint32 v);
	void Clear_gangrole();
	void Set_hqID(uint32 v);
	void Clear_hqID();
	void Set_solarsystemid2(uint32 v);
	void Clear_solarsystemid2();
	void Set_shipid(uint32 v);
	void Clear_shipid();
	void Set_charid(uint32 v);
	void Clear_charid();
	void Set_corprole(uint64 v);
	void Clear_corprole();
	void Set_rolesAtAll(uint64 v);
	void Clear_rolesAtAll();
	void Set_rolesAtBase(uint64 v);
	void Clear_rolesAtBase();
	void Set_rolesAtHQ(uint64 v);
	void Clear_rolesAtHQ();
	void Set_rolesAtOther(uint64 v);
	void Clear_rolesAtOther();

	bool IsDirty() const { return(m_dirty); }

	void EncodeChange(SessionChangeNotification &into);
	void Dump(LogType type) const;

protected:
	class SessionData
	{
	public:
		SessionData();
		~SessionData();

		std::string *address;
		std::string *languageID;

		uint32 *userType;
		uint32 *userid;
		uint32 *role;
		uint32 *constellationid;
		uint32 *corpid;
		uint32 *regionid;
		uint32 *stationid;
		uint32 *solarsystemid;
		uint32 *locationid;
		uint32 *gangrole;
		uint32 *hqID;
		uint32 *solarsystemid2;
		uint32 *shipid;
		uint32 *charid;
		uint64 *corprole;
		uint64 *rolesAtAll;
		uint64 *rolesAtBase;
		uint64 *rolesAtHQ;
		uint64 *rolesAtOther;
	};

	bool m_dirty;
	SessionData m_current;
	SessionData m_last;
};


#endif

