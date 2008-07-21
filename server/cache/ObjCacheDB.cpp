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


#include "ObjCacheDB.h"
#include "../common/dbcore.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/EVEDBUtils.h"

ObjCacheDB::ObjCacheDB(DBcore *db)
: ServiceDB(db)
{
	//register all the generators
	#define KCACHABLE(name, key, symbol, type, query) \
		m_generators[name] = &ObjCacheDB::Generate_##symbol;
	#include "CachableList.h"
}

ObjCacheDB::~ObjCacheDB() {
}

PyRep *ObjCacheDB::GetCachableObject(const std::string &type) {
	std::map<std::string, genFunc>::const_iterator res;
	res = m_generators.find(type);
	if(res == m_generators.end()) {
		_log(SERVICE__ERROR, "Unable to find cachable object generator for type '%s'", type.c_str());
		return(NULL);
	}

	genFunc f = res->second;
	return((this->*f)());
}

PyRep *ObjCacheDB::DBResultToRowsetTuple(DBQueryResult &result) {
	//this is a vicious hack to get around the fact that we do not know 
	//how to build a blue.DBRow/blue.DBRowDescriptor object representation
	//of a table for transmission over the wire.
	// 
	// after studying the python, I figured out a representation that 
	// could pass, functionality wise, for how they were using the real one.
	PyRepTuple *t = new PyRepTuple(2);
	t->items[0] = new PyRepNone();
	t->items[1] = DBResultToRowset(result);

	return(t);
}

//implement all the generators:
#define KCACHABLE(name, key, symbol, type, query) \
PyRep *ObjCacheDB::Generate_##symbol() { \
	DBQueryResult res; \
	const char *q = query; \
	if(q == NULL) /* handle NULL queries, meaning fall back to files */ \
		return(NULL); \
	if(!m_db->RunQuery(res, q)) { \
		codelog(SERVICE__ERROR, "Error in query for cached object '" name "': %s", res.error.c_str()); \
		return(NULL); \
	} \
	return(DBResultTo##type(res)); \
}
#define ICACHABLE(name, symbol, type, key, query) \
PyRep *ObjCacheDB::Generate_##symbol() { \
	DBQueryResult res; \
	const char *q = query; \
	if(q == NULL) /* handle NULL queries, meaning fall back to files */ \
		return(NULL); \
	if(!m_db->RunQuery(res, q)) { \
		codelog(SERVICE__ERROR, "Error in query for cached object '" name "': %s", res.error.c_str()); \
		return(NULL); \
	} \
	return(DBResultTo##type(key, res)); \
}
#include "CachableList.h"


























