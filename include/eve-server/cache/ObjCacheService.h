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


#ifndef __OBJCACHE_H_INCL__
#define __OBJCACHE_H_INCL__

#include "cache/ObjCacheDB.h"
#include "PyService.h"

class PyDict;

//little helper class for repeated code and memory management
class ObjectCachedMethodID {
public:
	ObjectCachedMethodID(const char *service, const char *method);
	~ObjectCachedMethodID();
	PyRep *objectID;
};

//little helper class for repeated code and memory management
class ObjectCachedSessionMethodID {
public:
	ObjectCachedSessionMethodID(const char *service, const char *method, int32 sessionValue);
	~ObjectCachedSessionMethodID();
	PyRep *objectID;
};

class ObjCacheService : public PyService {
public:
	ObjCacheService(PyServiceMgr *mgr, const char *cacheDir);
	virtual ~ObjCacheService();

	void PrimeCache();

	//function provided to other services:
	typedef enum {
		hLoginCachables,
		hCharCreateCachables,
		hCharCreateNewExtraCachables,
		hAppearanceCachables
	} hintSet;
	void InsertCacheHints(hintSet hset, PyDict *into);

    PyRep *GetCacheHint(const PyRep* objectID);

	PySubStream* LoadCachedFile(const char *filename, const char *oname);

	//handlers for simple cached method calls.
	bool IsCacheLoaded(const PyRep *objectID) const;
	bool IsCacheLoaded(const ObjectCachedMethodID &m) const { return(IsCacheLoaded(m.objectID)); }
	bool IsCacheLoaded(const ObjectCachedSessionMethodID &m) const { return(IsCacheLoaded(m.objectID)); }

	void InvalidateCache(const PyRep *objectID);
	void InvalidateCache(const ObjectCachedMethodID &m) { InvalidateCache(m.objectID); }

	void GiveCache(const PyRep *objectID, PyRep **contents);
	void GiveCache(const ObjectCachedMethodID &m, PyRep **contents) { GiveCache(m.objectID, contents); }
	void GiveCache(const ObjectCachedSessionMethodID &m, PyRep **contents) { GiveCache(m.objectID, contents); }

	PyObject *MakeObjectCachedMethodCallResult(const PyRep *objectID, const char *versionCheck="run");
	PyObject *MakeObjectCachedMethodCallResult(const ObjectCachedMethodID &m, const char *versionCheck="run") { return(MakeObjectCachedMethodCallResult(m.objectID, versionCheck)); }
	
	PyObject *MakeObjectCachedSessionMethodCallResult(const PyRep *objectID, const char *sessionInfoName, const char *clientWhen="always");
	PyObject *MakeObjectCachedSessionMethodCallResult(const ObjectCachedSessionMethodID &m, const char *sessionInfoName, const char *clientWhen="always") { return(MakeObjectCachedSessionMethodCallResult(m.objectID, sessionInfoName, clientWhen)); }
	
protected:

	static const char *const LoginCachableObjects[];
	static const uint32 LoginCachableObjectCount;
	static const char *const CharCreateCachableObjects[];
	static const uint32 CharCreateCachableObjectCount;
	static const char *const AppearanceCachableObjects[];
	static const uint32 AppearanceCachableObjectCount;
	static const char *const CharCreateNewExtraCachableObjects[];
	static const uint32 CharCreateNewExtraCachableObjectCount;
	
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
