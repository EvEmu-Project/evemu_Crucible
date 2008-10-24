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


#ifndef __OBJCACHE_H_INCL__
#define __OBJCACHE_H_INCL__

#include "../PyService.h"

#include "ObjCacheDB.h"
#include "../common/CachedObjectMgr.h"
#include <map>

class PyRepDict;

//little helper class for repeated code and memory management
class ObjectCachedMethodID {
public:
	ObjectCachedMethodID(const char *service, const char *method);
	~ObjectCachedMethodID();
	PyRep *objectID;
};

class ObjCacheService : public PyService {
public:
	ObjCacheService(PyServiceMgr *mgr, DBcore *dbc, const std::string &CacheDirectory);
	virtual ~ObjCacheService();

	void PrimeCache();

	//function provided to other services:
	typedef enum {
		hLoginCachables,
		hCharCreateCachables,
		hCharNewExtraCreateCachables,
		hAppearanceCachables
	} hintSet;
	void InsertCacheHints(hintSet hset, PyRepDict *into);
	PyRep *GetCacheHint(const char *objectID);

	bool LoadCachedFile(const char *filename, const char *oname, PyRepSubStream *into);

	//handlers for simple cached method calls.
	bool IsCacheLoaded(const PyRep *objectID) const;
	void InvalidateCache(const PyRep *objectID);
	void GiveCache(const PyRep *objectID, PyRep **contents);
	PyRepObject *MakeObjectCachedMethodCallResult(const PyRep *objectID, const char *versionCheck="run");
	inline bool IsCacheLoaded(const ObjectCachedMethodID &m) const { return(IsCacheLoaded(m.objectID)); }
	inline void GiveCache(const ObjectCachedMethodID &m, PyRep **contents) { GiveCache(m.objectID, contents); }
	inline PyRepObject *MakeObjectCachedMethodCallResult(const ObjectCachedMethodID &m, const char *versionCheck="run") { return(MakeObjectCachedMethodCallResult(m.objectID, versionCheck)); }
	inline void InvalidateCache(const ObjectCachedMethodID &m) { InvalidateCache(m.objectID); }
	
protected:

	static const char *const LoginCachableObjects[];
	static const uint32 LoginCachableObjectCount;
	static const char *const CharCreateCachableObjects[];
	static const uint32 CharCreateCachableObjectCount;
	static const char *const CharNewExtraCreateCachableObjects[];
	static const uint32 CharNewExtraCreateCachableObjectCount;
	static const char *const AppearanceCachableObjects[];
	static const uint32 AppearanceCachableObjectCount;
	
	class Dispatcher;
	Dispatcher *const m_dispatch;

	ObjCacheDB m_db;

	std::string m_cacheDir;
	CachedObjectMgr m_cache;

	bool _LoadCachableObject(const PyRep *objectID);
	std::map<std::string, std::string> m_cacheKeys;

	PyCallable_DECL_CALL(GetCachableObject)
};





#endif


