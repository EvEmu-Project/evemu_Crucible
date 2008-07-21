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




#include "../common/common.h"
#include "ClientSession.h"
#include "../common/PyRep.h"


ClientSession::ClientSession()
: m_dirty(false)
{
}
	
#define SI(name) \
	void ClientSession::Set_##name(uint32 v) { \
		if(m_current.name == NULL) \
			m_current.name = new uint32; \
		*(m_current.name) = v; \
		m_dirty = true; \
	} \
	\
	void ClientSession::Clear_##name() { \
		delete m_current.name; \
		m_current.name = NULL; \
		m_dirty = true; \
	}
#define SL(name) \
	void ClientSession::Set_##name(uint64 v) { \
		if(m_current.name == NULL) \
			m_current.name = new uint64; \
		*(m_current.name) = v; \
		m_dirty = true; \
	} \
	\
	void ClientSession::Clear_##name() { \
		delete m_current.name; \
		m_current.name = NULL; \
		m_dirty = true; \
	}
#define SS(name) \
	void ClientSession::Set_##name(const char *v) { \
		if(m_current.name == NULL) \
			m_current.name = new std::string; \
		*(m_current.name) = v; \
		m_dirty = true; \
	} \
	\
	void ClientSession::Clear_##name() { \
		delete m_current.name; \
		m_current.name = NULL; \
		m_dirty = true; \
	}
#include "ClientSession_fields.h"
	
PyRepDict *ClientSession::EncodeChange() {
	PyRepDict *keys = new PyRepDict();
	PyRepTuple *t;
	
#define ENCODE(name, PyType, argtype) \
	t = NULL; \
	if(m_current.name == NULL) { \
		if(m_last.name == NULL) { \
		} else { \
			_log(CLIENT__SESSION, "Session change: " #name " is now NULL"); \
			t = new PyRepTuple(2); \
			t->items[0] = new PyType(*(m_last.name)); \
			t->items[1] = new PyRepNone(); \
			delete m_last.name; \
			m_last.name = NULL; \
		} \
	} else { \
		if(m_last.name == NULL) { \
			_log(CLIENT__SESSION, "Session change: " #name " is no longer NULL."); \
			t = new PyRepTuple(2); \
			t->items[0] = new PyRepNone(); \
			t->items[1] = new PyType(*(m_current.name)); \
			m_last.name = new argtype; \
		} else if(*(m_last.name) != *(m_current.name)) { \
			_log(CLIENT__SESSION, "Session change: " #name " changed in value."); \
			t = new PyRepTuple(2); \
			t->items[0] = new PyType(*(m_last.name)); \
			t->items[1] = new PyType(*(m_current.name)); \
		} \
		*(m_last.name) = *(m_current.name); \
	} \
	if(t != NULL) \
		keys->items[ new PyRepString(#name) ] = t;
#define SI(name) ENCODE(name, PyRepInteger, uint32 )
#define SL(name) ENCODE(name, PyRepInteger, uint64 )
#define SS(name) ENCODE(name, PyRepString , std::string )
#include "ClientSession_fields.h"
#undef ENCODE
	
	m_dirty = false;
	return(keys);
}
	
void ClientSession::Dump(LogType type) const {
	if(!is_log_enabled(type))
		return;	//dont bother with all the work...
	
	_log(type, "Session Dump:");
#define SI(name) \
	if(m_current.name == NULL) { \
		_log(type, "    " #name ": NULL"); \
	} else { \
		_log(type, "    " #name ": %lu", *(m_current.name)); \
	}
#define SL(name) \
	if(m_current.name == NULL) { \
		_log(type, "    " #name ": NULL"); \
	} else { \
		_log(type, "    " #name ": %I64u", *(m_current.name)); \
	}
#define SS(name) \
	if(m_current.name == NULL) { \
		_log(type, "    " #name ": NULL"); \
	} else { \
		_log(type, "    " #name ": '%s'", m_current.name->c_str()); \
	}
#include "ClientSession_fields.h"
#undef ENCODE
}



ClientSession::SessionData::SessionData() {
#define SI(name) \
	name = NULL;
#define SL(name) \
	name = NULL;
#define SS(name) \
	name = NULL;
#include "ClientSession_fields.h"
}

ClientSession::SessionData::~SessionData() {
#define SI(name) \
	delete name;
#define SL(name) \
	delete name;
#define SS(name) \
	delete name;
#include "ClientSession_fields.h"
}














