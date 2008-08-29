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





#ifndef EVE_CLIENT_SESSION_H
#define EVE_CLIENT_SESSION_H

#include "../common/logsys.h"
#include <string>

//class PyRepDict;
class SessionChangeNotification;

class ClientSession {
public:
	ClientSession();
	
	/*
	each attribute has a Set_* method and a Clear_* method.
	*/
	
	#define SI(name) \
		void Set_##name(uint32 v); \
		void Clear_##name();
	#define SL(name) \
		void Set_##name(uint64 v); \
		void Clear_##name();
	#define SS(name) \
		void Set_##name(const char *v); \
		void Clear_##name();
	#include "ClientSession_fields.h"
	
	bool IsDirty() const { return(m_dirty); }
	//PyRepDict *EncodeChange();
	void EncodeChange(SessionChangeNotification &into);
	void Dump(LogType type) const;

protected:
	class SessionData {
	public:
		SessionData();
		~SessionData();
		#define SI(name) \
			uint32 *name;
		#define SL(name) \
			uint64 *name;
		#define SS(name) \
			std::string *name;
		#include "ClientSession_fields.h"
	};
	
	bool m_dirty;
	SessionData m_current;
	SessionData m_last;
};



#endif

