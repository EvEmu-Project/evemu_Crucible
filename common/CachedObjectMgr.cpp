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


#include "CachedObjectMgr.h"

#include "PyRep.h"
#include "logsys.h"
#include "Base64.h"
#include "misc.h"
#include "EVEMarshal.h"
#include "packet_functions.h"
#include "crc32.h"
#include "EVEUtils.h"
#include "EVEMarshalOpcodes.h"
#include "PyDumpVisitor.h"

#include "../packets/ObjectCaching.h"

#include <errno.h>
#include <string.h>

static const uint32 HackCacheNodeID = 333444;


//run through the rep, concatenating all the strings together and noting if
//there are ano non-string types in the rep (lists and tuples are OK)
class StringCollapseVisitor : public PyVisitor {
public:
	std::string result;
	bool good;
	StringCollapseVisitor()
	: good(true) {
	}
	virtual void VisitBoolean(const PyRepBoolean *rep) {
		good = false;
	}
	virtual void VisitBuffer(const PyRepBuffer *rep) {
		good = false;
	}
	virtual void VisitChecksumedStream(const PyRepChecksumedStream *rep) {
		good = false;
	}
	virtual void VisitDict(const PyRepDict *rep) {
		good = false;
	}
	virtual void VisitInteger(const PyRepInteger *rep) {
		good = false;
	}
	virtual void VisitNone(const PyRepNone *rep) {
		good = false;
	}
	virtual void VisitObject(const PyRepObject *rep) {
		good = false;
	}
	virtual void VisitPackedObject(const PyRepPackedObject *rep) {
		good = false;
	}
	virtual void VisitPackedObject1(const PyRepPackedObject1 *rep) {
		good = false;
	}
	virtual void VisitPackedObject2(const PyRepPackedObject2 *rep) {
		good = false;
	}
	virtual void VisitPackedRow(const PyRepPackedRow *rep) {
		good = false;
	}
	virtual void VisitReal(const PyRepReal *rep) {
		good = false;
	}
	virtual void VisitString(const PyRepString *rep) {
		if(!result.empty())
			result += ".";
		result += rep->value;
	}
	virtual void VisitSubStream(const PyRepSubStream *rep) {
		good = false;
	}
	virtual void VisitSubStruct(const PyRepSubStruct *rep) {
		good = false;
	}
	//allow tuples and lists.
};




CachedObjectMgr::~CachedObjectMgr() {
	std::map<std::string, CacheRecord *>::iterator cur, end;
	cur = m_cachedObjects.begin();
	end = m_cachedObjects.end();
	for(; cur != end; cur++) {
		delete cur->second;
	}
}

CachedObjectMgr::CacheRecord::~CacheRecord() {
	delete cache;
	delete objectID;
}

PyRepObject *CachedObjectMgr::CacheRecord::EncodeHint() const {
	objectCaching_CachedObject_spec spec;

	spec.objectID = objectID->Clone();
	spec.nodeID = HackCacheNodeID;
	spec.timestamp = timestamp;
	spec.version = version;
	
	return(spec.FastEncode());
}

//extract out the string contents of the object ID... if its a single string,
// then this visit will be boring, but if its a nested structure of strings, 
// its more interesting, either way we should come out with some string...
std::string CachedObjectMgr::OIDToString(const PyRep *objectID) {
	StringCollapseVisitor v;
	objectID->visit(&v);
	if(!v.good) {
		//codelog(SERVICE__ERROR, "Failed to convert cache hind object ID into collapsed string:");
		objectID->Dump(SERVICE__ERROR, "    ");
		return("");
	}
	return(v.result);
}

bool CachedObjectMgr::HaveCached(const std::string &objectID) const {
	//this is very sub-optimal, but it keeps things more consistent (in case StringCollapseVisitor ever gets more complicated)
	PyRepString str(objectID);
	return(HaveCached(&str));
}

bool CachedObjectMgr::HaveCached(const PyRep *objectID) const {
	const std::string str = OIDToString(objectID);
	return(m_cachedObjects.find(str) != m_cachedObjects.end());
}

void CachedObjectMgr::InvalidateCache(const PyRep *objectID) {
	const std::string str = OIDToString(objectID);
	std::map<std::string, CacheRecord *>::iterator res;
	res = m_cachedObjects.find(str);
	if(res != m_cachedObjects.end()) {
		delete res->second;
		m_cachedObjects.erase(res);
	}
}

//#define RAW_CACHE_CONTENTS

void CachedObjectMgr::UpdateCacheFromSS(const std::string &objectID, PyRepSubStream **in_cached_data) {
	PyCachedObjectDecoder cache;
	if(!cache.Decode(in_cached_data)) {
		_log(SERVICE__ERROR, "Failed to decode cache stream");
		return;
	}
	
	PyRepString str(objectID);
	_UpdateCache(&str, &cache.cache->data, cache.cache->length);
}

void CachedObjectMgr::UpdateCache(const std::string &objectID, PyRep **in_cached_data) {
	PyRepString str(objectID);
	UpdateCache(&str, in_cached_data);
}

void CachedObjectMgr::UpdateCache(const PyRep *objectID, PyRep **in_cached_data) {
	PyRep *cached_data = *in_cached_data;
	*in_cached_data = NULL;

	uint32 len;
	byte *buf = Marshal(cached_data, len);

	_UpdateCache(objectID, &buf, len);

	if(is_log_enabled(SERVICE__CACHE_DUMP)) {
		PyLogsysDump dumper(SERVICE__CACHE_DUMP, SERVICE__CACHE_DUMP, false, true);
		cached_data->visit(&dumper);
	}
	delete cached_data;
}

void CachedObjectMgr::_UpdateCache(const PyRep *objectID, byte **data, uint32 length) {
	
	//this is the hard one..
	CacheRecord *r = new CacheRecord;
	r->timestamp = Win32TimeNow();
	r->objectID = objectID->Clone();

	//consider compressing it.

	//This is not as complete as the python, as they indicate to the netcode (with compressedPart) 
	//how much of the packet is compressed when considering this packet for compression when sending
	uint32 deflen = length;
	byte *buf = DeflatePacket(*data, deflen);
	if(buf == NULL || deflen >= length) {
		//failed to deflate or it did no good (client checks this)
		//passes ownership of the encoded buffer to the new PyRepBuffer
		delete[] buf;
		buf = NULL;
		r->cache = new PyRepBuffer(data, length);
	} else {
		//compression successful, use that.
		r->cache = new PyRepBuffer(&buf, deflen);
		delete[] *data;
		*data = NULL;
	}
	
	r->version = CRC32::Generate(r->cache->GetBuffer(), r->cache->GetLength());

	const std::string str = OIDToString(objectID);

	//find and destroy any older version of this object.
	std::map<std::string, CacheRecord *>::iterator res;
	res = m_cachedObjects.find(str);
	if(res != m_cachedObjects.end()) {
		_log(SERVICE__CACHE, "Destroying old cached object with ID '%s' of length %lu with checksum 0x%lx", str.c_str(), res->second->cache->GetLength(), res->second->version);
		delete res->second;
	}
	
	_log(SERVICE__CACHE, "Registering new cached object with ID '%s' of length %lu with checksum 0x%lx", str.c_str(), r->cache->GetLength(), r->version);
	
	m_cachedObjects[str] = r;
}

PyRepObject *CachedObjectMgr::MakeCacheHint(const std::string &objectID) {
	//this is sub-optimal, but it keeps things more consistent (in case StringCollapseVisitor ever gets more complicated)
	PyRepString str(objectID);
	return(MakeCacheHint(&str));
}

PyRepObject *CachedObjectMgr::MakeCacheHint(const PyRep *objectID) {
	const std::string str = OIDToString(objectID);
	std::map<std::string, CacheRecord *>::iterator res;
	res = m_cachedObjects.find(str);
	if(res == m_cachedObjects.end())
		return(NULL);
	return(res->second->EncodeHint());
}

PyRepObject *CachedObjectMgr::GetCachedObject(const std::string &objectID) {
	//this is sub-optimal, but it keeps things more consistent (in case StringCollapseVisitor ever gets more complicated)
	PyRepString str(objectID);
	return(GetCachedObject(&str));
}

PyRepObject *CachedObjectMgr::GetCachedObject(const PyRep *objectID) {
	const std::string str = OIDToString(objectID);
	std::map<std::string, CacheRecord *>::iterator res;
	res = m_cachedObjects.find(str);
	if(res == m_cachedObjects.end())
		return(NULL);
	
	PyCachedObject co;
	co.timestamp = res->second->timestamp;
	co.version = res->second->version;
	co.nodeID = HackCacheNodeID;	//hack, dosent matter until we have multi-node networks.
	co.shared = true;
	co.objectID = res->second->objectID->Clone();
	if(res->second->cache->GetLength() == 0 || res->second->cache->GetBuffer()[0] == SubStreamHeaderByte)
		co.compressed = false;
	else
		co.compressed = true;

	_log(SERVICE__CACHE, "Returning cached object '%s' with checksum 0x%x", str.c_str(), co.version);
	
	co.cache = res->second->cache;
		PyRepObject *result = co.Encode();
	co.cache = NULL;	//avoid a copy
	return(result);
}

bool CachedObjectMgr::IsCacheUpToDate(const PyRep *objectID, uint32 version, uint64 timestamp) {
	const std::string str = OIDToString(objectID);
	std::map<std::string, CacheRecord *>::iterator res;
	res = m_cachedObjects.find(str);
	if(res == m_cachedObjects.end())
		return(false);
	//for now, only support exact matches...
	if(res->second->version != version)
		return(false);
	if(res->second->timestamp != timestamp)
		return(false);
	return(true);
}


typedef struct {
	uint64 timestamp;
	uint32 version;
	uint32 length;
	uint32 magic;
} CacheFileHeader;

static const uint32 CacheFileMagic = 0xFF886622;


bool CachedObjectMgr::LoadCachedFromFile(const std::string &cacheDir, const std::string &objectID) {
	//this is sub-optimal, but it keeps things more consistent (in case StringCollapseVisitor ever gets more complicated)
	PyRepString str(objectID);
	return(LoadCachedFromFile(cacheDir, &str));
}

bool CachedObjectMgr::LoadCachedFromFile(const std::string &cacheDir, const PyRep *objectID) {
	const std::string str = OIDToString(objectID);

	std::string filename(cacheDir);
	filename += "/";
	filename += str;
	filename += ".cache";
	
	FILE *f = fopen(filename.c_str(), "rb");
	if(f == NULL) {
		return(false);
	}

	CacheFileHeader header;
	if(fread(&header, sizeof(header), 1, f) != 1) {
		fclose(f);
		return(false);
	}
	if(header.magic != CacheFileMagic) {
		//not a valid cache file.
		fclose(f);
		return(false);
	}

	CacheRecord *cache = new CacheRecord();
	cache->objectID = objectID->Clone();
	cache->cache = new PyRepBuffer(header.length);
	cache->timestamp = header.timestamp;
	cache->version = header.version;
	
	if(fread(cache->cache->GetBuffer(), sizeof(byte), header.length, f) != header.length) {
		delete cache;
		fclose(f);
		return(false);
	}
	fclose(f);
	
	std::map<std::string, CacheRecord *>::iterator res;
	res = m_cachedObjects.find(str);
	if(res != m_cachedObjects.end()) {
		//clear out the old one
		delete res->second;
	}
	
	m_cachedObjects[str] = cache;
	
	return(true);
}

bool CachedObjectMgr::SaveCachedToFile(const std::string &cacheDir, const std::string &objectID) const {
	//this is sub-optimal, but it keeps things more consistent (in case StringCollapseVisitor ever gets more complicated)
	PyRepString str(objectID);
	return(SaveCachedToFile(cacheDir, &str));
}

bool CachedObjectMgr::SaveCachedToFile(const std::string &cacheDir, const PyRep *objectID) const {
	const std::string str = OIDToString(objectID);
	std::map<std::string, CacheRecord *>::const_iterator res;
	res = m_cachedObjects.find(str);
	if(res == m_cachedObjects.end())
		return(false);	//cant save something we dont have...
	
	std::string filename(cacheDir);
	filename += "/";
	filename += str;
	filename += ".cache";
	
	FILE *f = fopen(filename.c_str(), "wb");
	if(f == NULL) {
		return(false);
	}

	CacheFileHeader header;
	header.timestamp = res->second->timestamp;
	header.version = res->second->version;
	header.magic = CacheFileMagic;
	header.length = res->second->cache->GetLength();
	if(fwrite(&header, sizeof(header), 1, f) != 1) {
		fclose(f);
		return(false);
	}
	if(fwrite(res->second->cache->GetBuffer(), sizeof(byte), header.length, f) != header.length) {
		fclose(f);
		return(false);
	}
	fclose(f);
	return(true);
}



/*
void CachedObjectMgr::AddCacheHint(const char *oname, const char *key, PyRepDict *into) {
	PyRep *t = _MakeCacheHint(oname);
	if(t == NULL)
		return;
	into->add(key, t);
}
*/

bool CachedObjectMgr::LoadCachedObject(const char *obj_name, PyRepSubStream *into) {
	PyRepString oname_str(obj_name);
	return(LoadCachedObject(&oname_str, obj_name, into));
}

bool CachedObjectMgr::LoadCachedObject(PyRep *key, const char *oname, PyRepSubStream *into) {
	std::string fname;
	GetCacheFileName(key, fname);

	std::string abs_fname = "../data/cache/";
	abs_fname += fname;

	return(LoadCachedFile(abs_fname.c_str(), oname, into));
}


/*bool CachedObjectMgr::AddCachedFileContents(const char *filename, const char
    *oname, PyRepSubStream *into) { PyRepSubStream *cache;
	if(!LoadCachedFile(filename, oname, cache))
		return(false);
	
}*/

bool CachedObjectMgr::LoadCachedFile(const char *abs_fname, const char *oname, PyRepSubStream *into) {
	uint32 file_length = GetFileLength(abs_fname);
	if(file_length == 0) {
		_log(CLIENT__ERROR, "Unable to stat cache file '%s' for oname '%s'", abs_fname, oname);
		return(false);
	}
	
	FILE *f = fopen(abs_fname, "rb");
	if(f == NULL) {
		_log(CLIENT__ERROR, "Unable to open cache file '%s' for oname '%s': %s", abs_fname, oname, strerror(errno));
		return(false);
	}
	
	byte *b = new byte[file_length+10];
	int32 len = fread(b, 1, file_length+10, f);
	fclose(f);

	into->data = b;
	into->length = len;

	_log(CLIENT__MESSAGE, "Loaded cache file for '%s': length %d/%d", oname, len, file_length+10);
	
	return(true);
}

PyCachedObjectDecoder *CachedObjectMgr::LoadCachedObject(const char *obj_name) {
	PyRepString s(obj_name);
	return(LoadCachedObject(&s, obj_name));
}

PyCachedObjectDecoder *CachedObjectMgr::LoadCachedFile(const char *filename, const char *oname) {
	PyRepSubStream *into = new PyRepSubStream();
	if(!LoadCachedFile(filename, oname, into)) {
		delete into;
		return(NULL);
	}

	PyCachedObjectDecoder *obj = new PyCachedObjectDecoder();
	if(!obj->Decode(&into)) {	//into is consumed.
		delete obj;
		return(NULL);
	}

	return(obj);
}

PyCachedCall *CachedObjectMgr::LoadCachedCall(const char *filename, const char *oname) {
	PyRepSubStream *into = new PyRepSubStream();
	if(!LoadCachedFile(filename, oname, into)) {
		delete into;
		return(NULL);
	}

	PyCachedCall *obj = new PyCachedCall();
	if(!obj->Decode(&into)) {	//into is consumed.
		delete obj;
		return(NULL);
	}

	return(obj);
}

PyCachedObjectDecoder *CachedObjectMgr::LoadCachedObject(PyRep *key, const char *oname) {
	std::string fname;
	
	GetCacheFileName(key, fname);
	
	std::string abs_fname = "../data/cache/";
	abs_fname += fname;

	return(LoadCachedFile(abs_fname.c_str(), oname));
}

void CachedObjectMgr::GetCacheFileName(PyRep *key, std::string &into) {
	
	uint32 len = 0;
	byte *data = Marshal(key, len);
	
	Base64::encode(data, len, into, false);
	
	std::string::size_type epos = into.find('=');
	if(epos != std::string::npos)
		into.resize(epos);
	
	into += ".cache";
}

/*
PyRep *CachedObjectMgr::_MakeCacheHint(const char *oname) {
	//this is a ton of work we are doing here to generate a simple hint, but
	//theres not really a better way to do it while we are using the on-disk
	//cache files as the data source.
	PyCachedObject *obj = LoadCachedObject(oname);
	if(obj == NULL) {
		_log(CLIENT__ERROR, "Unable to load cache file for '%s' in order to build hint", oname);
		return(NULL);
	}

	PyRep *hint = obj->EncodeHint();
	delete obj;
	return(hint);
}
*/

PyCachedObjectDecoder::PyCachedObjectDecoder()
: timestamp(0),
  version(0),
  nodeID(0),
  shared(false),
  cache(NULL),
  compressed(false),
  objectID(NULL)
{
}

PyCachedObjectDecoder::~PyCachedObjectDecoder() {
	delete cache;
	delete objectID;
}

PyCachedObject::PyCachedObject()
: timestamp(0),
  version(0),
  nodeID(0),
  shared(false),
  cache(NULL),
  compressed(false),
  objectID(NULL)
{
}

PyCachedObject::~PyCachedObject() {
	delete cache;
	delete objectID;
}

PyCachedObject *PyCachedObject::Clone() const {
	PyCachedObject *res = new PyCachedObject();
	res->timestamp = timestamp;
	res->version  = version;
	res->nodeID  = nodeID;
	res->shared = shared;
	res->cache = (PyRepBuffer *) cache->Clone();
	res->compressed = compressed;
	res->objectID = objectID->Clone();
	return(res);
}

void PyCachedObjectDecoder::Dump(FILE *into, const char *pfx, bool contents_too) {
	std::string s(pfx);
	s += "    ";
	fprintf(into, "%sCached Object:\n", pfx);
	fprintf(into, "%s  ObjectID:\n", pfx);
		objectID->Dump(into, s.c_str());
	fprintf(into, "%s  Version Time: %llu\n", pfx, timestamp);
	fprintf(into, "%s  Version: %lu\n", pfx, version);
	fprintf(into, "%s  NodeID: %lu\n", pfx, nodeID);
	fprintf(into, "%s  Shared: %s\n", pfx, shared?"yes":"no");
	fprintf(into, "%s  Compressed: %s\n", pfx, compressed?"yes":"no");
	if(contents_too) {
		fprintf(into, "%s  Contents:\n", pfx);
			cache->Dump(into, s.c_str());
	}
}

void PyCachedObject::Dump(FILE *into, const char *pfx, bool contents_too) {
	std::string s(pfx);
	s += "    ";
	fprintf(into, "%sCached Object:\n", pfx);
	fprintf(into, "%s  ObjectID:\n", pfx);
		objectID->Dump(into, s.c_str());
	fprintf(into, "%s  Version Time: %llu\n", pfx, timestamp);
	fprintf(into, "%s  Version: %lu\n", pfx, version);
	fprintf(into, "%s  NodeID: %lu\n", pfx, nodeID);
	fprintf(into, "%s  Shared: %s\n", pfx, shared?"yes":"no");
	fprintf(into, "%s  Compressed: %s\n", pfx, compressed?"yes":"no");
	if(contents_too) {
		fprintf(into, "%s  Contents:\n", pfx);
			cache->Dump(into, s.c_str());
	}
}

bool PyCachedObjectDecoder::Decode(PyRepSubStream **in_ss) {
	PyRepSubStream *ss = *in_ss;	//consume
	*in_ss = NULL;

	delete cache;
	cache = NULL;
	delete objectID;
	objectID = NULL;
	
	ss->DecodeData();
	if(ss->decoded == NULL) {
		_log(CLIENT__ERROR, "Unable to decode initial stream for PyCachedObject");
		delete ss;
		return(false);
	}
	
	if(!ss->decoded->CheckType(PyRep::Object)) {
		_log(CLIENT__ERROR, "Cache substream does not contain an object: %s", ss->decoded->TypeString());
		delete ss;
		return(false);
	}
	PyRepObject *po = (PyRepObject *) ss->decoded;
	//TODO: could check type string, dont care... (should be objectCaching.CachedObject)
	
	if(!po->arguments->CheckType(PyRep::Tuple)) {
		_log(CLIENT__ERROR, "Cache object's args is not a tuple: %s", po->arguments->TypeString());
		delete ss;
		return(false);
	}
	PyRepTuple *args = (PyRepTuple *) po->arguments;
	
	if(args->items.size() != 7) {
		_log(CLIENT__ERROR, "Cache object's args tuple has %d elements instead of 7", args->items.size());
		delete ss;
		return(false);
	}

	if(!args->items[0]->CheckType(PyRep::Tuple)) {
		_log(CLIENT__ERROR, "Cache object's arg %d is not a Tuple: %s", 0, args->items[0]->TypeString());
		delete ss;
		return(false);
	}
	//ignore unknown [1]
	/*if(!args->items[1]->CheckType(PyRep::Integer)) {
		_log(CLIENT__ERROR, "Cache object's arg %d is not a None: %s", 1, args->items[1]->TypeString());
		delete ss;
		return(false);
	}*/
	if(!args->items[2]->CheckType(PyRep::Integer)) {
		_log(CLIENT__ERROR, "Cache object's arg %d is not an Integer: %s", 2, args->items[2]->TypeString());
		delete ss;
		return(false);
	}
	if(!args->items[3]->CheckType(PyRep::Integer)) {
		_log(CLIENT__ERROR, "Cache object's arg %d is not an Integer: %s", 3, args->items[3]->TypeString());
		delete ss;
		return(false);
	}
	if(!args->items[5]->CheckType(PyRep::Integer)) {
		_log(CLIENT__ERROR, "Cache object's arg %d is not a : %s", 5, args->items[5]->TypeString());
		delete ss;
		return(false);
	}

	PyRepTuple *objVt = (PyRepTuple *) args->items[0];
	if(!objVt->items[0]->CheckType(PyRep::Integer)) {
		_log(CLIENT__ERROR, "Cache object's version tuple %d is not an Integer: %s", 0, objVt->items[0]->TypeString());
		delete ss;
		return(false);
	}
	if(!objVt->items[1]->CheckType(PyRep::Integer)) {
		_log(CLIENT__ERROR, "Cache object's version tuple %d is not an Integer: %s", 1, objVt->items[1]->TypeString());
		delete ss;
		return(false);
	}
	
	PyRepInteger *nodeidr = (PyRepInteger *) args->items[2];
	PyRepInteger *sharedr = (PyRepInteger *) args->items[3];
	PyRepInteger *compressedr = (PyRepInteger *) args->items[5];
	PyRepInteger *timer = (PyRepInteger *) objVt->items[0];
	PyRepInteger *versionr = (PyRepInteger *) objVt->items[1];

	timestamp = timer->value;
	version = versionr->value;
	nodeID = nodeidr->value;
	shared = (sharedr->value != 0);
	compressed = (compressedr->value != 0);

	
	
	//content (do this as the last thing, since its the heavy lifting):
	if(args->items[4]->CheckType(PyRep::SubStream)) {
		cache = (PyRepSubStream *) args->items[4];
		//take it
		args->items[4] = NULL;
	} else if(args->items[4]->CheckType(PyRep::Buffer)) {
		//this is a data buffer, likely compressed.
		PyRepBuffer *buf = (PyRepBuffer *) args->items[4];
		cache = buf->CreateSubStream();
		if(cache == NULL) {
			_log(CLIENT__ERROR, "Cache object's content buffer is not a substream!");
			delete ss;
			return(false);
		}
	} else if(args->items[4]->CheckType(PyRep::String)) {
		//this is a data buffer, likely compressed, not sure why it comes through as a string...
		PyRepString *buf = (PyRepString *) args->items[4];
		//hack for now:
		PyRepBuffer tmpbuf((const byte *) buf->value.c_str(), buf->value.length());
		cache = tmpbuf.CreateSubStream();
		if(cache == NULL) {
			_log(CLIENT__ERROR, "Cache object's content buffer is not a substream!");
			delete ss;
			return(false);
		}
	} else {
		_log(CLIENT__ERROR, "Cache object's arg %d is not a substream or buffer: %s", 4, args->items[4]->TypeString());
		delete ss;
		return(false);
	}

	objectID = args->items[6]->Clone();

	delete ss;
	return(true);
}

PyRepObject *PyCachedObject::Encode() {
	PyRepObject *obj = new PyRepObject();

	obj->type = "objectCaching.CachedObject";
	PyRepTuple *arg_tuple = new PyRepTuple(7);
	obj->arguments = arg_tuple;

	PyRepTuple *versiont = new PyRepTuple(2);
	versiont->items[0] = new PyRepInteger(timestamp);
	versiont->items[1] = new PyRepInteger(version);

	
	arg_tuple->items[0] = versiont;

	arg_tuple->items[1] = new PyRepNone();

	arg_tuple->items[2] = new PyRepInteger(nodeID);
	
	arg_tuple->items[3] = new PyRepInteger(shared?1:0);

	//compression or not, we want to encode this into bytes so it dosent
	//get cloned in obect form just to be encoded later
/*	cache->EncodeData();
	if(compressed) {
		byte *buf = new byte[cache->length];
		uint32 deflen = DeflatePacket(cache->data, cache->length, buf, cache->length);
		if(deflen == 0 || deflen >= cache->length) {
			//failed to deflate or it did no good (client checks this)
			memcpy(buf, cache->data, cache->length);
			deflen = cache->length;
			compressed = false;
		}
		//buf is consumed:
		arg_tuple->items[4] = new PyRepBuffer(&buf, deflen);
	} else {
		//TODO: we dont really need to clone this if we can figure out a way to say "this is read only"
		//or if we can change this encode method to consume the PyCachedObject (which will almost always be the case)
		arg_tuple->items[4] = cache->Clone();
	}*/
	//TODO: we dont really need to clone this if we can figure out a way to say "this is read only"
	//or if we can change this encode method to consume the PyCachedObject (which will almost always be the case)
	arg_tuple->items[4] = cache->Clone();
	
	arg_tuple->items[5] = new PyRepInteger(compressed?1:0);

	//same cloning stattement as above.
	arg_tuple->items[6] = objectID->Clone();
	
	return(obj);
}

PyRepObject *PyCachedObjectDecoder::EncodeHint() {
	PyRepObject *obj = new PyRepObject();

	obj->type = "util.CachedObject";
	PyRepTuple *arg_tuple = new PyRepTuple(3);
	obj->arguments = arg_tuple;

	PyRepTuple *versiont = new PyRepTuple(2);
	versiont->items[0] = new PyRepInteger(timestamp);
	versiont->items[1] = new PyRepInteger(version);

	
	arg_tuple->items[0] = objectID->Clone();
	
	arg_tuple->items[1] = new PyRepInteger(nodeID);
	
	arg_tuple->items[2] = versiont;
	
	return(obj);
}


PyCachedCall::PyCachedCall()
: result(NULL)
{
}

PyCachedCall::~PyCachedCall() {
	delete result;
}

PyCachedCall *PyCachedCall::Clone() const {
	PyCachedCall *res = new PyCachedCall();
	res->result = result->Clone();
	return(res);
}

void PyCachedCall::Dump(FILE *into, const char *pfx, bool contents_too) {
	std::string s(pfx);
	s += "    ";
	fprintf(into, "%sCached Call: (empty right now) \n", pfx);
	if(contents_too) {
		fprintf(into, "%s  Contents:\n", pfx);
			result->Dump(into, s.c_str());
	}
}

bool PyCachedCall::Decode(PyRepSubStream **in_ss) {
	PyRepSubStream *ss = *in_ss;	//consume
	*in_ss = NULL;

	delete result;
	result = NULL;
	
	ss->DecodeData();
	if(ss->decoded == NULL) {
		_log(CLIENT__ERROR, "Unable to decode initial stream for PyCachedCall");
		delete ss;
		return(false);
	}
	
	if(!ss->decoded->CheckType(PyRep::Dict)) {
		_log(CLIENT__ERROR, "Cached call substream does not contain a dict: %s", ss->decoded->TypeString());
		delete ss;
		return(false);
	}
	PyRepDict *po = (PyRepDict *) ss->decoded;
	
	PyRepDict::iterator cur, end;
	cur = po->begin();
	end = po->end();
	for(; cur != end; cur++) {
		if(!cur->first->CheckType(PyRep::String))
			continue;
		PyRepString *key = (PyRepString *) cur->first;
		if(key->value == "lret") {
			result = cur->second;
			cur->second = NULL;
		}
	}

	delete ss;
	return(result != NULL);
}

















