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


#ifndef __OBJCACHE_H_INCL__
#define __OBJCACHE_H_INCL__

#include "../PyService.h"

#include "ObjCacheDB.h"
#include "../common/CachedObjectMgr.h"

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
	ObjCacheService(PyServiceMgr *mgr, DBcore *dbc, const char *cacheDir);
	virtual ~ObjCacheService();

	void PrimeCache();

	//function provided to other services:
	typedef enum {
		hLoginCachables,
		hCharCreateCachables,
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
	static const char *const AppearanceCachableObjects[];
	static const uint32 AppearanceCachableObjectCount;
	
	class Dispatcher;
	Dispatcher *const m_dispatch;

	ObjCacheDB m_db;

	std::string m_cacheDir;
	CachedObjectMgr m_cache;

	bool _LoadCachableObject(const PyRep *objectID);

    typedef std::map<std::string, std::string>  CacheKeysMap;
    typedef CacheKeysMap::iterator              CacheKeysMapItr;
    typedef CacheKeysMap::const_iterator        CacheKeysMapConstItr;

	CacheKeysMap m_cacheKeys;

	PyCallable_DECL_CALL(GetCachableObject)
};

#endif
