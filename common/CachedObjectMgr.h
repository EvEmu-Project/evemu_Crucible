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

/*
 * this code has devolved into quite a mess, but it works for now...
 *
*/

#ifndef __CACHEDOBJECTMGR_H_INCL__
#define __CACHEDOBJECTMGR_H_INCL__

#include "common.h"
#include <string>
#include <map>

class PyRep;
class PyRepSubStream;
class PyRepDict;
class PyCachedObject;
class PyCachedCall;
class PyRepObject;
class PyRepBuffer;
class PyCachedObjectDecoder;

class CachedObjectMgr {
public:
	~CachedObjectMgr();

	//internal utility function to keep maps simpler.
	static std::string OIDToString(const PyRep *objectID);
	
	bool HaveCached(const std::string &objectID) const;
	bool HaveCached(const PyRep *objectID) const;
	void InvalidateCache(const PyRep *objectID);
	
//	bool IsObjectFresh(const std::string &objectID, uint32 version, uint64 timestamp);
	void UpdateCacheFromSS(const std::string &objectID, PyRepSubStream **in_cached_data);
	void UpdateCache(const std::string &objectID, PyRep **in_cached_data);
	void UpdateCache(const PyRep *objectID, PyRep **in_cached_data);
	PyRepObject *MakeCacheHint(const std::string &objectID);
	PyRepObject *GetCachedObject(const std::string &objectID);
	PyRepObject *MakeCacheHint(const PyRep *objectID);
	PyRepObject *GetCachedObject(const PyRep *objectID);

	bool IsCacheUpToDate(const PyRep *objectID, uint32 version, uint64 timestamp);
	
//OLD CCP FILE BASED ACCESS:
	//PyRep *_MakeCacheHint(const char *oname);
	//void AddCacheHint(const char *oname, const char *key, PyRepDict *into);
	
	bool LoadCachedObject(const char *obj_name, PyRepSubStream *into);
	bool LoadCachedFile(const char *filename, const char *oname, PyRepSubStream *into);
	PyCachedObjectDecoder *LoadCachedFile(const char *filename, const char *oname);
	PyCachedCall *LoadCachedCall(const char *filename, const char *oname);   //returns ownership

	//Cache file storage routines:
	bool LoadCachedFromFile(const std::string &cacheDir, const std::string &objectID);
	bool LoadCachedFromFile(const std::string &cacheDir, const PyRep *objectID);
	bool SaveCachedToFile(const std::string &cacheDir, const std::string &objectID) const;
	bool SaveCachedToFile(const std::string &cacheDir, const PyRep *objectID) const;
	
protected:
	bool LoadCachedObject(PyRep *key, const char *oname, PyRepSubStream *into);
	PyCachedObjectDecoder *LoadCachedObject(const char *obj_name);	//returns ownership
	PyCachedObjectDecoder *LoadCachedObject(PyRep *key, const char *oname);	//returns ownership

//	static bool AddCachedFileContents(const char *filename, const char *oname, PyRepSubStream *into);

	void GetCacheFileName(PyRep *key, std::string &into);

	void _UpdateCache(const PyRep *objectID, byte **data, uint32 length);
	
	class CacheRecord {
	public:
		~CacheRecord();
		PyRep *objectID;	//we own this
		uint64 timestamp;
		uint32 version;
//		PyRep *cache;	//we own this.
		PyRepBuffer *cache;	//we own this.
		PyRepObject *EncodeHint() const;
	};
	std::map<std::string, CacheRecord *> m_cachedObjects;	//we own these pointers
};



class PyCachedObject {
public:
	PyCachedObject();
	~PyCachedObject();

	void Dump(FILE *into, const char *pfx, bool contents_too = false);
//	bool Decode(PyRepSubStream **ss);	//consumes substream
	PyRepObject *Encode();
	PyCachedObject *Clone() const;

	//object version tuple:
	/*0*/	uint64 timestamp;
	/*0*/	uint32 version;		//might be swapped with nodeID

    /*1*/ //unknown None
	/*2*/uint32 nodeID;
	/*3*/bool shared;		//not sure

//	PyRep *raw_cache;
	/*4*/PyRep *cache;

	/*5*/bool compressed;	//guess
	/*6*/PyRep *objectID;	//generally a string or tuple.
};

class PyCachedObjectDecoder {
public:
	PyCachedObjectDecoder();
	~PyCachedObjectDecoder();

	void Dump(FILE *into, const char *pfx, bool contents_too = false);
	bool Decode(PyRepSubStream **ss);	//consumes substream
	PyRepObject *EncodeHint();

	//object version tuple:
	/*0*/	uint64 timestamp;
	/*0*/	uint32 version;		//might be swapped with nodeID

    /*1*/ //unknown None
	/*2*/uint32 nodeID;
	/*3*/bool shared;		//not sure
	
	/*4*/PyRepSubStream *cache;

	/*5*/bool compressed;	//guess
	/*6*/PyRep *objectID;	//generally a string or tuple.
};


class PyCachedCall {
public:
	PyCachedCall();
	~PyCachedCall();

	void Dump(FILE *into, const char *pfx, bool contents_too = false);
	bool Decode(PyRepSubStream **ss);	//consumes substream
	//PyRep *Encode();
	//PyRep *EncodeHint();
	PyCachedCall *Clone() const;

	//rret:
		//objectCaching.CachedMethodCallResult object
		//with ( dict(versionCheck, sessionInfo),
		// 		 substream (unknown right now)
		//		 tuple(version)
		// 		)
	//used:
		//win32 time
	//lret
		//the call result directly
		PyRep *result;
	//version:
		//copy of the version tuple in rret
};




#endif


