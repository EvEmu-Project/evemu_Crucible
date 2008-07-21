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
#ifndef __PYLOOKUPDUMP_H_INCL__
#define __PYLOOKUPDUMP_H_INCL__

#include "PyDumpVisitor.h"
#include <map>
#include <string>

class PyLookupResolver {
public:
	PyLookupResolver();

	bool LoadIntFile(const char *file);
	bool LoadStringFile(const char *file);

	const char *LookupInt(uint64 value) const;
	const char *LookupString(const char *value) const;

protected:
	std::map<uint32, std::string> m_intRecords;
	std::map<std::string, std::string> m_strRecords;
	mutable std::string m_dateBuffer;	//not thread safe or reentrant...
};

class PyLookupDump
: public PyLogsysDump {
public:
	PyLookupDump(PyLookupResolver *res, LogType type, bool full_hex = false, bool full_lists=false);
	PyLookupDump(PyLookupResolver *res, LogType type, LogType hex_type, bool full_hex, bool full_lists);
	virtual ~PyLookupDump();

protected:
	virtual void VisitInteger(const PyRepInteger *rep);
	virtual void VisitString(const PyRepString *rep);
	
	PyLookupResolver *const m_resolver;	//we do not own this.
};

#endif



