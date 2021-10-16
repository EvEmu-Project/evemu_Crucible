/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:     Zhur
*/

/** @todo this whole system needs updating....
 *
    [PyTuple 1 items]
      [PySubStream 134 bytes]
        [PyObjectData Name: objectCaching.CachedObject]
          [PyTuple 7 items]
            [PyTuple 2 items]
              [PyIntegerVar 129511422600825710]
              [PyInt 46587]
            [PyNone]
            [PyInt 704421]
            [PyInt 1]
            [PySubStream 34 bytes]
              [PyString "https://gate.eveonline.com/"]
            [PyInt 0]
            [PyTuple 3 items]
              [PyString "Method Call"]
              [PyString "server"]
              [PyTuple 2 items]
                [PyString "browserLockdownSvc"]
                [PyString "GetDefaultHomePage"]
    [PyNone]

    PyTuple* tuple = new PyTuple(7);
    tuple->SetItem(0, itr_1);
    tuple->SetItem(1, GenerateLockdownCachedObject());
    tuple->SetItem(2, new PyNone());

    // build the tuple based on above packet...may not need, as that is cached packet....
    PyTuple* first = new PyTuple(2);
        first->SetItem(0, new PyLong(Win32TimeNow()));
        first->SetItem(1, new PyInt(46587)); //unknown
    PyTuple* second = new PyTuple(3);
        second->SetItem(0, new PyString("Method Call"));
        second->SetItem(1, new PyString("server"));
    PyTuple* third = new PyTuple(2);
        third->SetItem(0, new PyString("browserLockdownSvc"));
        third->SetItem(1, new PyString("GetDefaultHomePage"));
        second->SetItem(2, third);
    PyTuple* data = new PyTuple(7);
        data->SetItem(0, first);
        data->SetItem(1, new PyNone()); //unknown
        data->SetItem(2, new PyInt(704421)); //unknown - nodeID?
        data->SetItem(3, PyStatic.NewOne()); //unknown
        data->SetItem(4, new PySubStream(new PyString("https://evemu.dev/")));
        data->SetItem(5, new PyInt(0)); //unknown
        data->SetItem(6, second);
    return new PyObject( "objectCaching.CachedMethodCallResult", data );

 */

#include "eve-common.h"

#include "cache/CachedObjectMgr.h"
#include "marshal/EVEMarshal.h"
#include "marshal/EVEMarshalOpcodes.h"
#include "packets/ObjectCaching.h"
#include "python/PyVisitor.h"
#include "python/PyRep.h"
#include "python/PyDumpVisitor.h"
#include "utils/EVEUtils.h"

const uint32 CacheFileMagic = 0xFF886622;
static const uint32 HackCacheNodeID = 333444;

CachedObjectMgr::~CachedObjectMgr()
{
    CachedObjMapItr cur, end;
    cur = m_cachedObjects.begin();
    end = m_cachedObjects.end();

    for(; cur != end; cur++) {
        SafeDelete( cur->second );
    }
}

/************************************************************************/
/* CacheRecord                                                          */
/************************************************************************/
CachedObjectMgr::CacheRecord::CacheRecord() : objectID(nullptr), timestamp(0), version(0), cache(nullptr) {}
CachedObjectMgr::CacheRecord::~CacheRecord()
{
    PyDecRef( objectID );
    PyDecRef( cache );
}

PyObject *CachedObjectMgr::CacheRecord::EncodeHint() const
{
    objectCaching_CachedObject_spec spec;

    spec.objectID = objectID->Clone();
    spec.nodeID = HackCacheNodeID;
    spec.timestamp = timestamp;
    spec.version = version;

    return(spec.Encode());
}


/*
 * # Cache Logging:
 * CACHE=1
 * CACHE__ERROR=1
 * CACHE__WARNING=0
 * CACHE__MESSAGE=0
 * CACHE__DEBUG=0
 * CACHE__INFO=0
 * CACHE__TRACE=0
 * CACHE__DUMP=0
 */
//extract out the string contents of the object ID... if its a single string,
// then this visit will be boring, but if its a nested structure of strings,
// its more interesting, either way we should come out with some string...
std::string CachedObjectMgr::OIDToString(const PyRep *objectID)
{
    StringCollapseVisitor v;
    if ( !objectID->visit( v ) ) {
        sLog.Error("Cached Obj Mgr", "Failed to convert cache hind object ID into collapsed string:");
        objectID->Dump(CACHE__DUMP, "    ");
        assert(false);
        return "";
    }
    return v.result;
}

bool CachedObjectMgr::HaveCached(const std::string &objectID) const
{
    //this is very sub-optimal, but it keeps things more consistent (in case StringCollapseVisitor ever gets more complicated)
    PyString *str = new PyString( objectID );
    bool ret = HaveCached(str);
    PyDecRef(str);
    return ret;
}

bool CachedObjectMgr::HaveCached(const PyRep *objectID) const
{
    PyIncRef(objectID);
    const std::string str = OIDToString(objectID);

    return (m_cachedObjects.find(str) != m_cachedObjects.end());
}

void CachedObjectMgr::InvalidateCache(const PyRep *objectID)
{
    PyIncRef(objectID);
    const std::string str = OIDToString(objectID);
    CachedObjMapItr res = m_cachedObjects.find(str);

    if (res != m_cachedObjects.end()) {
        SafeDelete( res->second );
        m_cachedObjects.erase(res);
    }
}

//#define RAW_CACHE_CONTENTS

void CachedObjectMgr::UpdateCacheFromSS(const std::string &objectID, PySubStream **in_cached_data)
{
    PyCachedObjectDecoder cache;
    if (!cache.Decode(in_cached_data)) {
        sLog.Error("CachedObjMgr","Failed to decode stream");
        return;
    }

    PyString* str = new PyString( objectID );
    PyBuffer* buf = cache.cache->data();
    _UpdateCache(str, &buf);

    PyDecRef( str );
}

void CachedObjectMgr::UpdateCache(const std::string &objectID, PyRep **in_cached_data)
{
    PyString *str = new PyString( objectID );
    UpdateCache(str, in_cached_data);
    PyDecRef(str);
}

void CachedObjectMgr::UpdateCache(const PyRep *objectID, PyRep **in_cached_data)
{
    PyRep* cached_data(*in_cached_data);
    *in_cached_data = nullptr;

    //if (is_log_enabled(CACHE__DUMP)) {
    //  PyLogsysDump dumper(CACHE__DUMP, CACHE__DUMP, false, true);
        //cached_data->visit(&dumper, 0);
    //}

    Buffer* buf = new Buffer();
    bool res = MarshalDeflate( cached_data, *buf );

    if ( res ) {
        PyBuffer* pbuf = new PyBuffer( &buf );
        _UpdateCache( objectID, &pbuf );
    } else {
        sLog.Error( "Cached Obj Mgr", "Failed to marshal or deflate new cache object." );
    }

    SafeDelete( buf );
}

void CachedObjectMgr::_UpdateCache(const PyRep *objectID, PyBuffer **pbuf)
{
    //this is the hard one..
    CacheRecord *r = new CacheRecord();
    r->timestamp = GetFileTimeNow();
    r->objectID = objectID->Clone();

    // retake ownership
    r->cache = *pbuf;
    *pbuf = nullptr;

    r->version = CRC32::Generate( &r->cache->content()[0], r->cache->content().size() );

    const std::string str = OIDToString(objectID);

    //find and destroy any older version of this object.
    CachedObjMapItr res = m_cachedObjects.find(str);

    if (res != m_cachedObjects.end()) {
        sLog.Debug("CachedObjMgr","Destroying old cached object with ID '%s' of length %u with checksum 0x%x", str.c_str(), res->second->cache->content().size(), res->second->version);
        SafeDelete( res->second );
    }

    sLog.Debug("CachedObjMgr","Registering new cached object with ID '%s' of length %u with checksum 0x%x", str.c_str(), r->cache->content().size(), r->version);

    m_cachedObjects[str] = r;
}

PyObject *CachedObjectMgr::MakeCacheHint(const std::string &objectID)
{
    //this is sub-optimal, but it keeps things more consistent (in case StringCollapseVisitor ever gets more complicated)
    PyString* str = new PyString( objectID );
    PyObject* obj(MakeCacheHint(str));
    PyDecRef(str);
    return obj;
}

PyObject *CachedObjectMgr::MakeCacheHint(const PyRep *objectID)
{
    const std::string str = OIDToString(objectID);

    CachedObjMapItr res = m_cachedObjects.find(str);
    if (res == m_cachedObjects.end())
        return nullptr;

    return res->second->EncodeHint();
}

PyObject *CachedObjectMgr::GetCachedObject(const std::string &objectID)
{
    //this is sub-optimal, but it keeps things more consistent (in case StringCollapseVisitor ever gets more complicated)
    PyString* str = new PyString( objectID );
    PyObject* obj(GetCachedObject(str));
    PyDecRef(str);
    return obj;
}

PyObject *CachedObjectMgr::GetCachedObject(const PyRep *objectID)
{
    const std::string str = OIDToString(objectID);

    CachedObjMapItr res = m_cachedObjects.find(str);
    if (res == m_cachedObjects.end())
        return nullptr;

    PyCachedObject co;
    co.timestamp = res->second->timestamp;
    co.version = res->second->version;
    co.nodeID = HackCacheNodeID;    //hack, doesn't matter until we have multi-node networks.
    co.shared = true;
    co.objectID = res->second->objectID->Clone();
    co.cache = res->second->cache;

    if (res->second->cache->content().size() == 0 || res->second->cache->content()[0] == MarshalHeaderByte)
        co.compressed = false;
    else
        co.compressed = true;

    sLog.Debug("CachedObjMgr","Returning cached object '%s' with checksum 0x%x", str.c_str(), co.version);

    PyObject* result = co.Encode();
    co.cache = nullptr;    //avoid a copy

    return result;
}

bool CachedObjectMgr::IsCacheUpToDate(const PyRep *objectID, uint32 version, int64 timestamp)
{
    const std::string str = OIDToString(objectID);

    CachedObjMapItr res = m_cachedObjects.find(str);
    if (res == m_cachedObjects.end())
        return false;

    //for now, only support exact matches...
    return (   res->second->version == version
            && res->second->timestamp == timestamp);
}

bool CachedObjectMgr::LoadCachedFromFile(const std::string &cacheDir, const std::string &objectID)
{
    //this is sub-optimal, but it keeps things more consistent (in case StringCollapseVisitor ever gets more complicated)
    PyString* str = new PyString(objectID);
    bool ret = LoadCachedFromFile(cacheDir, str);
    PyDecRef(str);
    return ret;
}

/**
 * LoadCachedFromFile
 *
 * Load a cached object from file.
 */
bool CachedObjectMgr::LoadCachedFromFile(const std::string &cacheDir, const PyRep *objectID)
{
    const std::string str = OIDToString(objectID);

    std::string filename(cacheDir);
    filename += "/" + str + ".cache";

    FILE *f = fopen(filename.c_str(), "rb");

    if (f == nullptr)
        return false;

    CacheFileHeader header;
    if (fread(&header, sizeof(header), 1, f) != 1) {
        fclose(f);
        return false;
    }

    /* check if its a valid cache file */
    if (header.magic != CacheFileMagic) {
        fclose(f);
        return false;
    }

    Buffer* buf = new Buffer( header.length );

    if ( fread( &(*buf)[0], sizeof( uint8 ), header.length, f ) != header.length ) {
        SafeDelete( buf );
        fclose( f );
        return false;
    }

    fclose( f );

    CachedObjMapItr res = m_cachedObjects.find( str );

    if ( res != m_cachedObjects.end() )
        SafeDelete( res->second );

    CacheRecord* cache = m_cachedObjects[ str ] = new CacheRecord;
    cache->objectID = objectID->Clone();
    cache->cache = new PyBuffer( &buf );
    cache->timestamp = header.timestamp;
    cache->version = header.version;

    m_cachedObjects[ str ] = cache;

    SafeDelete( buf );
    return true;
}

//this is sub-optimal, but it keeps things more consistent (in case StringCollapseVisitor ever gets more complicated)
bool CachedObjectMgr::SaveCachedToFile(const std::string &cacheDir, const std::string &objectID) const
{
    PyString* str = new PyString(objectID);
    bool ret = SaveCachedToFile(cacheDir, str);
    PyDecRef(str);
    return ret;
}

bool CachedObjectMgr::SaveCachedToFile(const std::string &cacheDir, const PyRep *objectID) const
{
    const std::string str = OIDToString(objectID);
    CachedObjMapConstItr res = m_cachedObjects.find(str);

    /* make sure we don't try to save a object we don't have */
    if (res == m_cachedObjects.end())
        return false;

    std::string filename(cacheDir);
    filename += "/";
    filename += str;
    filename += ".cache";

    FILE *f = fopen(filename.c_str(), "wb");

    if (f == nullptr)
        return false;

    CacheFileHeader header;
    header.timestamp = res->second->timestamp;
    header.version = res->second->version;
    header.magic = CacheFileMagic;
    header.length = res->second->cache->content().size();

    if (fwrite(&header, sizeof(header), 1, f) != 1) {
        fclose(f);
        return false;
    }

    if (fwrite(&res->second->cache->content()[0], sizeof(uint8), header.length, f) != header.length) {
        assert(false);
        fclose(f);
        return false;
    }
    fclose(f);
    return true;
}

/*
void CachedObjectMgr::AddCacheHint(const char *oname, const char *key, PyDict *into) {
    PyRep *t = _MakeCacheHint(oname);
    if (t == nullptr)
        return;
    into->add(key, t);
}
*/


/*bool CachedObjectMgr::AddCachedFileContents(const char *filename, const char
    *oname, PySubStream *into) { PySubStream *cache;
    if (!LoadCachedFile(filename, oname, cache))
        return false;

}*/

PySubStream* CachedObjectMgr::LoadCachedFile(const char *obj_name)
{
    PyString* str = new PyString(obj_name);
    PySubStream* ret = LoadCachedFile(str, obj_name);
    PyDecRef(str);
    return ret;
}

PySubStream *CachedObjectMgr::LoadCachedFile(PyRep *key, const char *oname)
{
    std::string fname;
    GetCacheFileName(key, fname);

    std::string abs_fname = "../data/cache/";
    abs_fname += fname;

    return LoadCachedFile(abs_fname.c_str(), oname);
}

PySubStream* CachedObjectMgr::LoadCachedFile( const char* abs_fname, const char* oname )
{
    FILE* f = fopen( abs_fname, "rb" );

    if ( f == nullptr ) {
        sLog.Error("CachedObjMgr","Unable to open cache file '%s' for oname '%s': %s", abs_fname, oname, strerror( errno ) );
        return 0;
    }

    int64 file_length = filesize( f );
    if ( file_length == 0 )
    {
        sLog.Error("CachedObjMgr","Unable to stat cache file '%s' for oname '%s'", abs_fname, oname );
        fclose( f );
        return 0;
    }

    Buffer* buf = new Buffer(static_cast<size_t>(file_length));

    if ( file_length != fread( &( *buf )[0], sizeof( uint8 ), static_cast<size_t>(file_length), f ) ) {
        sLog.Error("CachedObjMgr","Unable to read cache file '%s' for oname '%s%': %s", abs_fname, oname, strerror( errno ) );
        SafeDelete( buf );
        fclose( f );
        return 0;
    }

    fclose( f );
    sLog.Debug("CachedObjMgr","Loaded cache file for '%s': length %u", oname, file_length );

    /** @todo Mem leak.  `new PyBuffer()` never freed */
    PySubStream* res = new PySubStream( new PyBuffer( &buf ) );
    SafeDelete( buf );
    return res;
}

PyCachedObjectDecoder *CachedObjectMgr::LoadCachedObject(const char *filename, const char *oname)
{
    PySubStream* ss = LoadCachedFile(filename, oname);
    if ( ss == nullptr )
        return nullptr;

    PyCachedObjectDecoder *obj = new PyCachedObjectDecoder();
    if (!obj->Decode(&ss)) {   //ss is consumed.
        SafeDelete( obj );
        return nullptr;
    }

    return obj;
}

PyCachedCall *CachedObjectMgr::LoadCachedCall(const char *filename, const char *oname)
{
    PySubStream* ss = LoadCachedFile(filename, oname);
    if ( ss == nullptr )
        return nullptr;

    PyCachedCall *obj = new PyCachedCall();
    if (!obj->Decode(&ss)) {   //ss is consumed.
        SafeDelete( obj );
        return nullptr;
    }

    return obj;
}

// Base64 encoding utilities
#include <Base64.h>
void CachedObjectMgr::GetCacheFileName(PyRep *key, std::string &into)
{
    Buffer data;
    Marshal( key, data );

    Base64::encode( &data[0], data.size(), into, false );

    std::string::size_type epos = into.find('=');
    if (epos != std::string::npos)
        into.resize(epos);

    into += ".cache";
}

/*
PyRep *CachedObjectMgr::_MakeCacheHint(const char *oname) {
    //this is a ton of work we are doing here to generate a simple hint, but
    //theres not really a better way to do it while we are using the on-disk
    //cache files as the data source.
    PyCachedObject *obj = LoadCachedObject(oname);
    if (obj == nullptr) {
        _log(SERVICE__ERROR, "Unable to load cache file for '%s' in order to build hint", oname);
        return nullptr;
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
  cache(nullptr),
  compressed(false),
  objectID(nullptr)
{
}

PyCachedObjectDecoder::~PyCachedObjectDecoder()
{
    PySafeDecRef( cache );
    PySafeDecRef( objectID );
}

PyCachedObject::PyCachedObject()
: timestamp(0),
  version(0),
  nodeID(0),
  shared(false),
  cache(nullptr),
  compressed(false),
  objectID(nullptr)
{
}

PyCachedObject::~PyCachedObject()
{
    PySafeDecRef( cache );
    PySafeDecRef( objectID );
}

PyCachedObject *PyCachedObject::Clone() const
{
    PyCachedObject *res = new PyCachedObject();
    res->timestamp = timestamp;
    res->version  = version;
    res->nodeID  = nodeID;
    res->shared = shared;
    res->cache = (PyBuffer *) cache->Clone();
    res->compressed = compressed;
    res->objectID = objectID->Clone();
    return res;
}

void PyCachedObjectDecoder::Dump(FILE *into, const char *pfx, bool contents_too)
{
    std::string s(pfx);
    s += "    ";
    fprintf(into, "%sCached Object:\n", pfx);
    fprintf(into, "%s  ObjectID:\n", pfx);
    objectID->Dump(into, s.c_str());
    fprintf(into, "%s  Version Time: %" PRIu64 "\n", pfx, timestamp);
    fprintf(into, "%s  Version: %u\n", pfx, version);
    fprintf(into, "%s  NodeID: %u\n", pfx, nodeID);
    fprintf(into, "%s  Shared: %s\n", pfx, shared?"yes":"no");
    fprintf(into, "%s  Compressed: %s\n", pfx, compressed?"yes":"no");
    if (contents_too) {
        fprintf(into, "%s  Contents:\n", pfx);
            cache->Dump(into, s.c_str());
    }
}

void PyCachedObject::Dump(FILE *into, const char *pfx, bool contents_too)
{
    std::string s(pfx);
    s += "    ";
    fprintf(into, "%sCached Object:\n", pfx);
    fprintf(into, "%s  ObjectID:\n", pfx);
        objectID->Dump(into, s.c_str());
    fprintf(into, "%s  Version Time: %" PRIu64 "\n", pfx, timestamp);
    fprintf(into, "%s  Version: %u\n", pfx, version);
    fprintf(into, "%s  NodeID: %u\n", pfx, nodeID);
    fprintf(into, "%s  Shared: %s\n", pfx, shared?"yes":"no");
    fprintf(into, "%s  Compressed: %s\n", pfx, compressed?"yes":"no");
    if (contents_too) {
        fprintf(into, "%s  Contents:\n", pfx);
            cache->Dump(into, s.c_str());
    }
}

bool PyCachedObjectDecoder::Decode(PySubStream **in_ss)
{
    PySubStream *ss = *in_ss;    //consume
    *in_ss = nullptr;

    PySafeDecRef( cache );
    PySafeDecRef( objectID );

    ss->DecodeData();
    if (ss->decoded() == nullptr) {
        sLog.Error("PyCachedObjectDecoder","Unable to decode initial stream for PycachedObject");

        PyDecRef( ss );
        return false;
    }

    if (!ss->decoded()->IsObject()) {
        sLog.Error("PyCachedObjectDecoder","Cache substream does not contain an object: %s", ss->decoded()->TypeString());

        PyDecRef( ss );
        return false;
    }
    PyObject *po = (PyObject *) ss->decoded();
    //TODO: could check type string, dont care... (should be objectCaching.CachedObject)

    if (!po->arguments()->IsTuple()) {
        sLog.Error("PyCachedObjectDecoder","Cache object's args is not a tuple: %s", po->arguments()->TypeString());

        PyDecRef( ss );
        return false;
    }
    PyTuple *args = (PyTuple *) po->arguments();

    if (args->items.size() != 7) {
        sLog.Error("PyCachedObjectDecoder","Cache object's args tuple has %lu elements instead of 7", args->items.size());

        PyDecRef( ss );
        return false;
    }

    if (!args->items[0]->IsTuple()) {
        sLog.Error("PyCachedObjectDecoder","Cache object's arg %d is not a Tuple: %s", 0, args->items[0]->TypeString());

        PyDecRef( ss );
        return false;
    }
    //ignore unknown [1]
    /*if (!args->items[1]->IsInt()) {
        _log(SERVICE__ERROR, "Cache object's arg %d is not a None: %s", 1, args->items[1]->TypeString());

        PyDecRef( ss );
        return false;
    }*/
    if (!args->items[2]->IsInt()) {
        sLog.Error("PyCachedObjectDecoder","Cache object's arg %d is not an Integer: %s", 2, args->items[2]->TypeString());
        PyDecRef( ss );
        return false;
    }

    if (!args->items[3]->IsInt()) {
        sLog.Error("PyCachedObjectDecoder","Cache object's arg %d is not an Integer: %s", 3, args->items[3]->TypeString());
        PyDecRef( ss );
        return false;
    }

    if (!args->items[5]->IsInt()) {
        sLog.Error("PyCachedObjectDecoder","Cache object's arg %d is not a : %s", 5, args->items[5]->TypeString());
        PyDecRef( ss );
        return false;
    }

    PyTuple *objVt = (PyTuple *) args->items[0];
    if (!objVt->items[0]->IsInt()) {
        sLog.Error("PyCachedObjectDecoder","Cache object's version tuple %d is not an Integer: %s", 0, objVt->items[0]->TypeString());
        PyDecRef( ss );
        return false;
    }

    if (!objVt->items[1]->IsInt()) {
        sLog.Error("PyCachedObjectDecoder","Cache object's version tuple %d is not an Integer: %s", 1, objVt->items[1]->TypeString());
        PyDecRef( ss );
        return false;
    }

    PyInt *nodeidr = (PyInt *) args->items[2];
    PyInt *sharedr = (PyInt *) args->items[3];
    PyInt *compressedr = (PyInt *) args->items[5];
    PyInt *timer = (PyInt *) objVt->items[0];
    PyInt *versionr = (PyInt *) objVt->items[1];

    timestamp = timer->value();
    version = versionr->value();
    nodeID = nodeidr->value();
    shared = ( sharedr->value() != 0 );
    compressed = ( compressedr->value() != 0 );

    //content (do this as the last thing, since its the heavy lifting):
    if (args->items[4]->IsSubStream()) {
        cache = (PySubStream *) args->items[4];
        //take it
        args->items[4] = nullptr;
    } else if (args->items[4]->IsBuffer()) {
        //this is a data buffer, likely compressed.
        PyBuffer* buf = args->items[4]->AsBuffer();
        cache = new PySubStream( buf );
    } else if (args->items[4]->IsString()) {
        //this is a data buffer, likely compressed, not sure why it comes through as a string...
        PyString* str = args->items[4]->AsString();
        cache = new PySubStream( new PyBuffer( *str ) );
    } else {
        sLog.Error("PyCachedObjectMgr", "Cache object's arg %d is not a substream or buffer: %s", 4, args->items[4]->TypeString());
        PyDecRef( ss );
        return false;
    }

    objectID = args->items[6]->Clone();

    PyDecRef( ss );
    return true;
}

PyObject *PyCachedObject::Encode()
{
    PyTuple *arg_tuple = new PyTuple(7);
    arg_tuple->items[0] = new_tuple(new PyLong(timestamp), new PyInt(version));
    arg_tuple->items[1] = new PyNone();
    arg_tuple->items[2] = new PyInt(nodeID);
    arg_tuple->items[3] = new PyInt(shared?1:0);

    //compression or not, we want to encode this into bytes so it doesn't
    //get cloned in object form just to be encoded later
/*  cache->EncodeData();
    if (compressed) {
        uint8 *buf = new uint8[cache->length];
        uint32 deflen = DeflatePacket(cache->data, cache->length, buf, cache->length);
        if (deflen == 0 || deflen >= cache->length) {
            //failed to deflate or it did no good (client checks this)
            memcpy(buf, cache->data, cache->length);
            deflen = cache->length;
            compressed = false;
        }
        //buf is consumed:
        arg_tuple->items[4] = new PyBuffer(&buf, deflen);
    } else {
        //TODO: we dont really need to clone this if we can figure out a way to say "this is read only"
        //or if we can change this encode method to consume the PyCachedObject (which will almost always be the case)
        arg_tuple->items[4] = cache->Clone();
    }*/
    //TODO: we don't really need to clone this if we can figure out a way to say "this is read only"
    //or if we can change this encode method to consume the PyCachedObject (which will almost always be the case)
    arg_tuple->items[4] = cache->Clone();
    arg_tuple->items[5] = new PyInt(compressed?1:0);
    //same cloning statement as above.
    arg_tuple->items[6] = objectID->Clone();

    return new PyObject( "objectCaching.CachedObject", arg_tuple );
}

PyObject *PyCachedObjectDecoder::EncodeHint() {
    PyTuple *versiont = new PyTuple(2);
        versiont->items[0] = new PyLong(timestamp);
        versiont->items[1] = new PyInt(version);
    PyTuple *arg_tuple = new PyTuple(3);
        arg_tuple->items[0] = objectID->Clone();
        arg_tuple->items[1] = new PyInt(nodeID);
        arg_tuple->items[2] = versiont;

    return new PyObject( "util.CachedObject", arg_tuple );
}


PyCachedCall::PyCachedCall() : result(nullptr) {}
PyCachedCall::~PyCachedCall()
{
    PySafeDecRef( result );
}

PyCachedCall *PyCachedCall::Clone() const {
    PyCachedCall *res = new PyCachedCall();
    res->result = result->Clone();
    return res;
}

void PyCachedCall::Dump(FILE *into, const char *pfx, bool contents_too)
{
    std::string s(pfx);
    s += "    ";
    fprintf(into, "%sCached Call: (empty right now) \n", pfx);
    if (contents_too) {
        fprintf(into, "%s  Contents:\n", pfx);
        result->Dump(into, s.c_str());
    }
}

bool PyCachedCall::Decode(PySubStream **in_ss)
{
    PySubStream *ss = *in_ss;    //consume
    *in_ss = nullptr;

    PySafeDecRef( result );

    ss->DecodeData();
    if (ss->decoded() == nullptr) {
        sLog.Error("PyCachedCall","Unable to decode initial stream for PyCachedCall");
        PyDecRef( ss );
        return false;
    }

    if (!ss->decoded()->IsDict()) {
        sLog.Error("PyCachedCall","Cached call substream does not contain a dict: %s", ss->decoded()->TypeString());
        PyDecRef( ss );
        return false;
    }
    PyDict *po = (PyDict *) ss->decoded();

    PyDict::const_iterator cur, end;
    cur = po->begin();
    end = po->end();
    for(; cur != end; cur++) {
        if (!cur->first->IsString())
            continue;
        PyString *key = (PyString *) cur->first;
        if ( key->content() == "lret" )
            result = cur->second->Clone();
    }

    PyDecRef( ss );
    return (result != nullptr);
}
