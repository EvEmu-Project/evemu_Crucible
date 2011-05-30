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
    Author:     Zhur
*/

#include "EVECommonPCH.h"

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
    std::map<std::string, CacheRecord*>::iterator cur, end;
    cur = m_cachedObjects.begin();
    end = m_cachedObjects.end();
    for(; cur != end; cur++)
        SafeDelete( cur->second );
}

CachedObjectMgr::CacheRecord::~CacheRecord()
{
	PyDecRef( objectID );
	PyDecRef( cache );
}

PyObject *CachedObjectMgr::CacheRecord::EncodeHint() const {
    objectCaching_CachedObject_spec spec;

    spec.objectID = objectID->Clone();
    spec.nodeID = HackCacheNodeID;
    spec.timestamp = timestamp;
    spec.version = version;

    return(spec.Encode());
}

//extract out the string contents of the object ID... if its a single string,
// then this visit will be boring, but if its a nested structure of strings,
// its more interesting, either way we should come out with some string...
std::string CachedObjectMgr::OIDToString(const PyRep *objectID) {
    StringCollapseVisitor v;
    if( !objectID->visit( v ) )
    {
        sLog.Error("Cached Obj Mgr", "Failed to convert cache hind object ID into collapsed string:");
        objectID->Dump(SERVICE__ERROR, "    ");
        return "";
    }
    return v.result;
}

bool CachedObjectMgr::HaveCached(const std::string &objectID) const {
    //this is very sub-optimal, but it keeps things more consistent (in case StringCollapseVisitor ever gets more complicated)
    PyString str( objectID );
    return(HaveCached(&str));
}

bool CachedObjectMgr::HaveCached(const PyRep *objectID) const {
    const std::string str = OIDToString(objectID);

    return(m_cachedObjects.find(str) != m_cachedObjects.end());
}

void CachedObjectMgr::InvalidateCache(const PyRep *objectID) {
    const std::string str = OIDToString(objectID);

    std::map<std::string, CacheRecord *>::iterator res = m_cachedObjects.find(str);;
    if(res != m_cachedObjects.end())
    {
        SafeDelete( res->second );
        m_cachedObjects.erase(res);
    }
}

//#define RAW_CACHE_CONTENTS

void CachedObjectMgr::UpdateCacheFromSS(const std::string &objectID, PySubStream **in_cached_data) {
    PyCachedObjectDecoder cache;
    if(!cache.Decode(in_cached_data)) {
		sLog.Error("CachedObjectMgr","Failed to decode stream");
        return;
    }

    PyString* str = new PyString( objectID );

    PyBuffer* buf = cache.cache->data();
    PyIncRef( buf );

    _UpdateCache(str, &buf);

    PyDecRef( str );
}

void CachedObjectMgr::UpdateCache(const std::string &objectID, PyRep **in_cached_data) {
    PyString str( objectID );
    UpdateCache(&str, in_cached_data);
}

void CachedObjectMgr::UpdateCache(const PyRep *objectID, PyRep **in_cached_data) {
    PyRep *cached_data = *in_cached_data;
    *in_cached_data = NULL;

    //if(is_log_enabled(SERVICE__CACHE_DUMP)) {
      //  PyLogsysDump dumper(SERVICE__CACHE_DUMP, SERVICE__CACHE_DUMP, false, true);
        //cached_data->visit(&dumper, 0);
    //}

    Buffer* data = new Buffer;
    bool res = MarshalDeflate( cached_data, *data );
	PyDecRef( cached_data );

    if( res )
    {
	    PyBuffer* buf = new PyBuffer( &data );
        _UpdateCache( objectID, &buf );
    }
    else
        sLog.Error( "Cached Obj Mgr", "Failed to marshal or deflate new cache object." );

    SafeDelete( data );
}

void CachedObjectMgr::_UpdateCache(const PyRep *objectID, PyBuffer **buffer) {

    //this is the hard one..
    CacheRecord *r = new CacheRecord;
    r->timestamp = Win32TimeNow();
    r->objectID = objectID->Clone();

	// retake ownership
    r->cache = *buffer;
	*buffer = NULL;

    r->version = CRC32::Generate( &r->cache->content()[0], r->cache->content().size() );

    const std::string str = OIDToString(objectID);

    //find and destroy any older version of this object.
    std::map<std::string, CacheRecord *>::iterator res;
    res = m_cachedObjects.find(str);
    if(res != m_cachedObjects.end())
    {
		sLog.Debug("CahcedObjectMgr","Destroying old cached object with ID '%s' of length %u with checksum 0x%x", str.c_str(), res->second->cache->content().size(), res->second->version); 
		SafeDelete( res->second );
    }

	sLog.Debug("CachedObjectMgr","Registering new cached object with ID '%s' of length %u with checksum 0x%x", str.c_str(), r->cache->content().size(), r->version);

    m_cachedObjects[str] = r;
}

PyObject *CachedObjectMgr::MakeCacheHint(const std::string &objectID) {
    //this is sub-optimal, but it keeps things more consistent (in case StringCollapseVisitor ever gets more complicated)
    PyString str( objectID );
    return(MakeCacheHint(&str));
}

PyObject *CachedObjectMgr::MakeCacheHint(const PyRep *objectID) {
    const std::string str = OIDToString(objectID);

    std::map<std::string, CacheRecord *>::iterator res = m_cachedObjects.find(str);
    if(res == m_cachedObjects.end())
        return NULL;
    return(res->second->EncodeHint());
}

PyObject *CachedObjectMgr::GetCachedObject(const std::string &objectID) {
    //this is sub-optimal, but it keeps things more consistent (in case StringCollapseVisitor ever gets more complicated)
    PyString str( objectID );
    return(GetCachedObject(&str));
}

PyObject *CachedObjectMgr::GetCachedObject(const PyRep *objectID) {
    const std::string str = OIDToString(objectID);

    std::map<std::string, CacheRecord *>::iterator res = m_cachedObjects.find(str);
    if(res == m_cachedObjects.end())
        return NULL;

    PyCachedObject co;
    co.timestamp = res->second->timestamp;
    co.version = res->second->version;
    co.nodeID = HackCacheNodeID;    //hack, doesn't matter until we have multi-node networks.
    co.shared = true;
    co.objectID = res->second->objectID->Clone();
    co.cache = res->second->cache;
    if(res->second->cache->content().size() == 0 || res->second->cache->content()[0] == MarshalHeaderByte)
        co.compressed = false;
    else
        co.compressed = true;

	sLog.Debug("CachedObjectMgr","Returning cached object '%s' with checksum 0x%x", str.c_str(), co.version);

    PyObject *result = co.Encode();
    co.cache = NULL;    //avoid a copy

    return result;
}

bool CachedObjectMgr::IsCacheUpToDate(const PyRep *objectID, uint32 version, uint64 timestamp) {
    const std::string str = OIDToString(objectID);

    std::map<std::string, CacheRecord *>::iterator res = m_cachedObjects.find(str);
    if(res == m_cachedObjects.end())
        return false;

    //for now, only support exact matches...
    return (   res->second->version == version
            && res->second->timestamp == timestamp);
}

bool CachedObjectMgr::LoadCachedFromFile(const std::string &cacheDir, const std::string &objectID) {
    //this is sub-optimal, but it keeps things more consistent (in case StringCollapseVisitor ever gets more complicated)
    PyString str(objectID);
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
        return false;
    }

    CacheFileHeader header;
    if(fread(&header, sizeof(header), 1, f) != 1) {
        fclose(f);
        return false;
    }
    if(header.magic != CacheFileMagic) {
        //not a valid cache file.
        fclose(f);
        return false;
    }

    Buffer* buf = new Buffer( header.length );
    if( fread( &(*buf)[0], sizeof( uint8 ), header.length, f ) != header.length )
    {
        SafeDelete( buf );

        fclose( f );
        return false;
    }
    fclose( f );

    std::map<std::string, CacheRecord *>::iterator res = m_cachedObjects.find( str );
    if( res != m_cachedObjects.end() )
        SafeDelete( res->second );

    CacheRecord* cache = m_cachedObjects[ str ] = new CacheRecord;
    cache->objectID = objectID->Clone();
    cache->cache = new PyBuffer( &buf );
    cache->timestamp = header.timestamp;
    cache->version = header.version;

    return true;
}

bool CachedObjectMgr::SaveCachedToFile(const std::string &cacheDir, const std::string &objectID) const {
    //this is sub-optimal, but it keeps things more consistent (in case StringCollapseVisitor ever gets more complicated)
    PyString str(objectID);
    return(SaveCachedToFile(cacheDir, &str));
}

bool CachedObjectMgr::SaveCachedToFile(const std::string &cacheDir, const PyRep *objectID) const {
    const std::string str = OIDToString(objectID);
    std::map<std::string, CacheRecord *>::const_iterator res;
    res = m_cachedObjects.find(str);
    if(res == m_cachedObjects.end())
        return false;   //cant save something we dont have...

    std::string filename(cacheDir);
    filename += "/";
    filename += str;
    filename += ".cache";

    FILE *f = fopen(filename.c_str(), "wb");
    if(f == NULL) {
        return false;
    }

    CacheFileHeader header;
    header.timestamp = res->second->timestamp;
    header.version = res->second->version;
    header.magic = CacheFileMagic;
    header.length = res->second->cache->content().size();
    if(fwrite(&header, sizeof(header), 1, f) != 1) {
        fclose(f);
        return false;
    }
    if(fwrite(&res->second->cache->content()[0], sizeof(uint8), header.length, f) != header.length) {
        fclose(f);
        return false;
    }
    fclose(f);
    return true;
}

/*
void CachedObjectMgr::AddCacheHint(const char *oname, const char *key, PyDict *into) {
    PyRep *t = _MakeCacheHint(oname);
    if(t == NULL)
        return;
    into->add(key, t);
}
*/


/*bool CachedObjectMgr::AddCachedFileContents(const char *filename, const char
    *oname, PySubStream *into) { PySubStream *cache;
    if(!LoadCachedFile(filename, oname, cache))
        return false;

}*/

PySubStream* CachedObjectMgr::LoadCachedFile(const char *obj_name) {
    PyString oname_str(obj_name);
    return LoadCachedFile(&oname_str, obj_name);
}

PySubStream *CachedObjectMgr::LoadCachedFile(PyRep *key, const char *oname) {
    std::string fname;
    GetCacheFileName(key, fname);

    std::string abs_fname = "../data/cache/";
    abs_fname += fname;

    return(LoadCachedFile(abs_fname.c_str(), oname));
}

PySubStream* CachedObjectMgr::LoadCachedFile( const char* abs_fname, const char* oname )
{
    FILE* f = fopen( abs_fname, "rb" );
    if( f == NULL )
    {
		sLog.Error("CachedObjectMgr","Unable to open cache file '%s' for oname '%s': %s", abs_fname, oname, strerror( errno ) );
        return false;
    }

    uint32 file_length = filesize( f );
    if( file_length == 0 )
    {
		sLog.Error("CachedObjectMgr","Unable to stat cache file '%s' for oname '%s'", abs_fname, oname );

        fclose( f );
        return false;
    }

    Buffer* buf = new Buffer( file_length );

    if( file_length != fread( &( *buf )[0], sizeof( uint8 ), file_length, f ) )
    {
		sLog.Error("CachedObjectMgr","Unable to read cache file '%s' for oname '%s%': %s", abs_fname, oname, strerror( errno ) );

        SafeDelete( buf );
        fclose( f );
        return false;
    }
    fclose( f );

	sLog.Debug("CachedObjectMgr","Loaded cache file for '%s': length %u", oname, file_length );

    return new PySubStream( new PyBuffer( &buf ) );
}

PyCachedObjectDecoder *CachedObjectMgr::LoadCachedObject(const char *filename, const char *oname) {
    PySubStream* ss = LoadCachedFile(filename, oname);
    if( ss == NULL )
        return NULL;

    PyCachedObjectDecoder *obj = new PyCachedObjectDecoder();
    if(!obj->Decode(&ss)) {   //ss is consumed.
        SafeDelete( obj );
        return NULL;
    }

    return(obj);
}

PyCachedCall *CachedObjectMgr::LoadCachedCall(const char *filename, const char *oname) {
    PySubStream* ss = LoadCachedFile(filename, oname);
    if( ss == NULL )
        return NULL;

    PyCachedCall *obj = new PyCachedCall();
    if(!obj->Decode(&ss)) {   //ss is consumed.
        SafeDelete( obj );
        return NULL;
    }

    return(obj);
}

void CachedObjectMgr::GetCacheFileName(PyRep *key, std::string &into)
{
    Buffer data;
    Marshal( key, data );

    Base64::encode( &data[0], data.size(), into, false );

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
        return NULL;
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
    PySafeDecRef( cache );
    PySafeDecRef( objectID );
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
    PySafeDecRef( cache );
    PySafeDecRef( objectID );
}

PyCachedObject *PyCachedObject::Clone() const {
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

void PyCachedObjectDecoder::Dump(FILE *into, const char *pfx, bool contents_too) {
    std::string s(pfx);
    s += "    ";
    fprintf(into, "%sCached Object:\n", pfx);
    fprintf(into, "%s  ObjectID:\n", pfx);
    objectID->Dump(into, s.c_str());
    fprintf(into, "%s  Version Time: "I64u"\n", pfx, timestamp);
    fprintf(into, "%s  Version: %u\n", pfx, version);
    fprintf(into, "%s  NodeID: %u\n", pfx, nodeID);
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
    fprintf(into, "%s  Version Time: "I64u"\n", pfx, timestamp);
    fprintf(into, "%s  Version: %u\n", pfx, version);
    fprintf(into, "%s  NodeID: %u\n", pfx, nodeID);
    fprintf(into, "%s  Shared: %s\n", pfx, shared?"yes":"no");
    fprintf(into, "%s  Compressed: %s\n", pfx, compressed?"yes":"no");
    if(contents_too) {
        fprintf(into, "%s  Contents:\n", pfx);
            cache->Dump(into, s.c_str());
    }
}

bool PyCachedObjectDecoder::Decode(PySubStream **in_ss) {
    PySubStream *ss = *in_ss;    //consume
    *in_ss = NULL;

    PySafeDecRef( cache );
    PySafeDecRef( objectID );

    ss->DecodeData();
    if(ss->decoded() == NULL) {
		sLog.Error("PyCachedObjectDecoder","Unable to decode initial stream for PycachedObject");

        PyDecRef( ss );
        return false;
    }

    if(!ss->decoded()->IsObject()) {
		sLog.Error("PyCachedObjectDecoder","Cache substream does not contain an object: %s", ss->decoded()->TypeString());

        PyDecRef( ss );
        return false;
    }
    PyObject *po = (PyObject *) ss->decoded();
    //TODO: could check type string, dont care... (should be objectCaching.CachedObject)

    if(!po->arguments()->IsTuple()) {
		sLog.Error("PyCachedObjectDecoder","Cache object's args is not a tuple: %s", po->arguments()->TypeString());

        PyDecRef( ss );
        return false;
    }
    PyTuple *args = (PyTuple *) po->arguments();

    if(args->items.size() != 7) {
		sLog.Error("PyCachedObjectDecoder","Cache object's args tuple has %lu elements instead of 7", args->items.size());

        PyDecRef( ss );
        return false;
    }

    if(!args->items[0]->IsTuple()) {
		sLog.Error("PyCachedObjectDecoder","Cache object's arg %d is not a Tuple: %s", 0, args->items[0]->TypeString());

        PyDecRef( ss );
        return false;
    }
    //ignore unknown [1]
    /*if(!args->items[1]->IsInt()) {
        _log(CLIENT__ERROR, "Cache object's arg %d is not a None: %s", 1, args->items[1]->TypeString());

        PyDecRef( ss );
        return false;
    }*/
    if(!args->items[2]->IsInt()) {
		sLog.Error("PyCachedObjectDecoder","Cache object's arg %d is not an Integer: %s", 2, args->items[2]->TypeString());
        PyDecRef( ss );
        return false;
    }
    if(!args->items[3]->IsInt()) {
		sLog.Error("PyCachedObjectDecoder","Cache object's arg %d is not an Integer: %s", 3, args->items[3]->TypeString());
        PyDecRef( ss );
        return false;
    }
    if(!args->items[5]->IsInt()) {
		sLog.Error("PyCachedObjectDecoder","Cache object's arg %d is not a : %s", 5, args->items[5]->TypeString());
        PyDecRef( ss );
        return false;
    }

    PyTuple *objVt = (PyTuple *) args->items[0];
    if(!objVt->items[0]->IsInt()) {
		sLog.Error("PyCachedObjectDecoder","Cache object's version tuple %d is not an Integer: %s", 0, objVt->items[0]->TypeString());
        PyDecRef( ss );
        return false;
    }
    if(!objVt->items[1]->IsInt()) {
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
    if(args->items[4]->IsSubStream()) {
        cache = (PySubStream *) args->items[4];
        //take it
        args->items[4] = NULL;
    } else if(args->items[4]->IsBuffer()) {
        //this is a data buffer, likely compressed.
        PyBuffer* buf = args->items[4]->AsBuffer();

        PyIncRef( buf );
		cache = new PySubStream( buf );
    } else if(args->items[4]->IsString()) {
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

PyObject *PyCachedObject::Encode() {
    PyTuple *arg_tuple = new PyTuple(7);

    PyTuple *versiont = new PyTuple(2);
    versiont->items[0] = new PyLong(timestamp);
    versiont->items[1] = new PyInt(version);


    arg_tuple->items[0] = versiont;

    arg_tuple->items[1] = new PyNone();

    arg_tuple->items[2] = new PyInt(nodeID);

    arg_tuple->items[3] = new PyInt(shared?1:0);

    //compression or not, we want to encode this into bytes so it doesn't
    //get cloned in object form just to be encoded later
/*  cache->EncodeData();
    if(compressed) {
        uint8 *buf = new uint8[cache->length];
        uint32 deflen = DeflatePacket(cache->data, cache->length, buf, cache->length);
        if(deflen == 0 || deflen >= cache->length) {
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

    //same cloning stattement as above.
    arg_tuple->items[6] = objectID->Clone();

    return new PyObject(
        new PyString( "objectCaching.CachedObject" ), arg_tuple
    );
}

PyObject *PyCachedObjectDecoder::EncodeHint() {
    PyTuple *arg_tuple = new PyTuple(3);

    PyTuple *versiont = new PyTuple(2);
    versiont->items[0] = new PyLong(timestamp);
    versiont->items[1] = new PyInt(version);


    arg_tuple->items[0] = objectID->Clone();

    arg_tuple->items[1] = new PyInt(nodeID);

    arg_tuple->items[2] = versiont;

    return new PyObject(
        new PyString( "util.CachedObject" ), arg_tuple
    );
}


PyCachedCall::PyCachedCall() : result(NULL)
{
}

PyCachedCall::~PyCachedCall() {
    PySafeDecRef( result );
}

PyCachedCall *PyCachedCall::Clone() const {
    PyCachedCall *res = new PyCachedCall();
    res->result = result->Clone();
    return res;
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

bool PyCachedCall::Decode(PySubStream **in_ss) {
    PySubStream *ss = *in_ss;    //consume
    *in_ss = NULL;

    PySafeDecRef( result );

    ss->DecodeData();
    if(ss->decoded() == NULL) {
		sLog.Error("PyCachedCall","Unable to decode initial stream for PyCachedCall");
        PyDecRef( ss );
        return false;
    }

    if(!ss->decoded()->IsDict()) {
		sLog.Error("PyCachedCall","Cached call substream does not contain a dict: %s", ss->decoded()->TypeString());
        PyDecRef( ss );
        return false;
    }
    PyDict *po = (PyDict *) ss->decoded();

    PyDict::const_iterator cur, end;
    cur = po->begin();
    end = po->end();
    for(; cur != end; cur++) {
        if(!cur->first->IsString())
            continue;
        PyString *key = (PyString *) cur->first;
        if( key->content() == "lret" )
            result = cur->second->Clone();
    }

    PyDecRef( ss );
    return(result != NULL);
}
