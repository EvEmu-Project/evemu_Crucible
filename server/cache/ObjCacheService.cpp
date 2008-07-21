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



#include "ObjCacheService.h"

#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../common/CachedObjectMgr.h"

#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"

#include "../packets/ObjectCaching.h"


const char *const ObjCacheService::LoginCachableObjects[] = {
	"config.BulkData.ramactivities",
	"config.BulkData.billtypes",
	"config.Bloodlines",
	"config.Units",
	"config.BulkData.dgmtypeattribs",
	"config.BulkData.messages",
	"config.BulkData.ramtyperequirements",
	"config.Statistics",
	"config.BulkData.ramaltypesdetailpergroup",
	"config.BulkData.ramaltypes",
	"config.BulkData.allianceshortnames",
	"config.BulkData.ramcompletedstatuses",
	"config.BulkData.categories",
	"config.BulkData.invtypereactions",
	"config.BulkData.dgmtypeeffects",
	"config.BulkData.metagroups",
	"config.BulkData.ramtypematerials",
	"config.EncyclopediaTypes",
	"config.ChannelTypes",
	"config.BulkData.owners",
	"config.StaticOwners",
	"config.Races",
	"config.BulkData.ramaltypesdetailpercategory",
	"config.Attributes",
	"config.BulkData.tickernames",
	"config.Roles",
	"config.BulkData.groups",
	"config.BulkData.shiptypes",
	"config.BulkData.dgmattribs",
	"config.BulkData.locations",
	"config.BulkData.constants",
	"config.Flags",
	"config.BulkData.bptypes",
	"config.BulkData.graphics",
	"config.StaticLocations",
	"config.InvContrabandTypes",
	"config.BulkData.dgmeffects",
	"config.BulkData.types",
	"config.BulkData.invmetatypes"
};
const uint32 ObjCacheService::LoginCachableObjectCount = sizeof(ObjCacheService::LoginCachableObjects) / sizeof(const char *);

const char *const ObjCacheService::CharCreateCachableObjects[] = {
	"charCreationInfo.bl_eyebrows",
	"charCreationInfo.bl_costumes",
	"charCreationInfo.bl_decos",
	"charCreationInfo.fields",
	"charCreationInfo.bl_hairs",
	"charCreationInfo.bl_backgrounds",
	"charCreationInfo.bloodlines",
	"charCreationInfo.departments",
	"charCreationInfo.bl_accessories",
	"charCreationInfo.bl_lights",
	"charCreationInfo.races",
	"charCreationInfo.ancestries",
	"charCreationInfo.bl_skins",
	"charCreationInfo.specialities",
	"charCreationInfo.schools",
	"charCreationInfo.attributes",
	"charCreationInfo.bl_beards",
	"charCreationInfo.bl_eyes",
	"charCreationInfo.bl_lipsticks",
	"charCreationInfo.bl_makeups"
};
const uint32 ObjCacheService::CharCreateCachableObjectCount = sizeof(ObjCacheService::CharCreateCachableObjects) / sizeof(const char *);


const char *const ObjCacheService::AppearanceCachableObjects[] = {
	"charCreationInfo.eyebrows",
	"charCreationInfo.eyes",
	"charCreationInfo.decos",
	"charCreationInfo.hairs",
	"charCreationInfo.backgrounds",
	"charCreationInfo.accessories",
	"charCreationInfo.costumes",
	"charCreationInfo.lights",
	"charCreationInfo.makeups",
	"charCreationInfo.beards",
	"charCreationInfo.skins",
	"charCreationInfo.lipsticks"
};
const uint32 ObjCacheService::AppearanceCachableObjectCount = sizeof(ObjCacheService::AppearanceCachableObjects) / sizeof(const char *);



PyCallable_Make_InnerDispatcher(ObjCacheService)

ObjCacheService::ObjCacheService(PyServiceMgr *mgr, DBcore *dbc, const std::string &CacheDirectory)
: PyService(mgr, "objectCaching"),
  m_dispatch(new Dispatcher(this)),
  m_db(dbc),
  m_cacheDir(CacheDirectory)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(ObjCacheService, GetCachableObject)

	//register full name -> short key in m_cacheKeys
	#define KCACHABLE(name, key, symbol, type, query) \
		m_cacheKeys[name] = key;
	#include "CachableList.h"
}

ObjCacheService::~ObjCacheService() {
	delete m_dispatch;
}

PyCallResult ObjCacheService::Handle_GetCachableObject(PyCallArgs &call) {
	CallGetCachableObject args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Unable to decode arguments", call.client->GetName());
		return(NULL);
	}
	
	if(!_LoadCachableObject(args.objectID))
		return(NULL);	//print done already
	
	//should we check their version? I am pretty sure they check it and only request what they want.
	//well, we want to do something like this, but this dosent seem to be it. taken
	// out until we have time to figure out how to properly throw the CacheOK exception.
	/*if(m_cache.IsCacheUpToDate(args.objectID, args.version, args.timestamp)) {
		//they throw an exception for "its up to date", lets give it a try...
		objectCaching_CacheOK except;
		return(PyCallException(except.FastEncode()));
	}*/
	
	PyRepObject *result = m_cache.GetCachedObject(args.objectID);
	
	return(result);
}

void ObjCacheService::PrimeCache() {
	std::map<std::string, std::string>::const_iterator cur, end;
	cur = m_cacheKeys.begin();
	end = m_cacheKeys.end();
	for(; cur != end; cur++) {
		_log(SERVICE__CACHE, "Priming cache object '%s'", cur->first.c_str());
		PyRepString str(cur->first);
		_LoadCachableObject(&str);
	}
}

bool ObjCacheService::LoadCachedFile(const char *filename, const char *oname, PyRepSubStream *into) {
	//temp hack...
	return(m_cache.LoadCachedFile(filename, oname, into));
}


bool ObjCacheService::_LoadCachableObject(const PyRep *objectID) {
	if(m_cache.HaveCached(objectID))
		return(true);
	
	const std::string objectID_string = CachedObjectMgr::OIDToString(objectID);

	if(!m_cacheDir.empty()) {
		if(m_cache.LoadCachedFromFile(m_cacheDir, objectID)) {
			_log(SERVICE__CACHE, "Loaded cached object '%s' from file.", objectID_string.c_str());
			return(true);
		}
	}
	
	PyRepSubStream *ss;
	
	//first try to generate it from the database...
	//we go to the DB with a string, not a rep
	PyRep *cache = m_db.GetCachableObject(objectID_string);
	if(cache != NULL) {
		//we have generated the cache file in question, remember it
		m_cache.UpdateCache(objectID, &cache);
	} else {
		//failed to query from the database... fall back to old
		//hackish file loading.
		ss = new PyRepSubStream();
		if(!m_cache.LoadCachedObject(objectID_string.c_str(), ss)) {
			_log(SERVICE__ERROR, "Failed to create or load cache file for '%s'", objectID_string.c_str());
			return(false);
		}
		
		//we have generated the cache file in question, remember it
		m_cache.UpdateCacheFromSS(objectID_string, &ss);
	}
	
	//if we have a cache dir, write out the cache entry:
	if(!m_cacheDir.empty()) {
		if(!m_cache.SaveCachedToFile(m_cacheDir, objectID)) {
			_log(SERVICE__ERROR, "Failed to save cache file for '%s' in '%s'", objectID_string.c_str(), m_cacheDir.c_str());
		} else {
			_log(SERVICE__CACHE, "Saved cached object '%s' to file.", objectID_string.c_str());
		}
	}

	return(true);
}

PyRep *ObjCacheService::GetCacheHint(const char *objectID) {
	PyRepString str(objectID);
	
	if(!_LoadCachableObject(&str))
		return(NULL);	//print done already

	PyRepObject *cache_hint = m_cache.MakeCacheHint(&str);
	if(cache_hint == NULL) {
		_log(SERVICE__ERROR, "Unable to build cache hint for object ID '%s' (h), skipping.", objectID);
		return(NULL);
	}

	return(cache_hint);
}

void ObjCacheService::InsertCacheHints(hintSet hset, PyRepDict *into) {
	const char *const *objects = NULL;
	uint32 object_count = 0;
	switch(hset) {
	case hLoginCachables:
		objects = LoginCachableObjects;
		object_count = LoginCachableObjectCount;
		break;
	case hCharCreateCachables:
		objects = CharCreateCachableObjects;
		object_count = CharCreateCachableObjectCount;
		break;
	case hAppearanceCachables:
		objects = AppearanceCachableObjects;
		object_count = AppearanceCachableObjectCount;
		break;
	}
	if(objects == NULL)
		return;
	uint32 r;
	std::map<std::string, std::string>::const_iterator res;
	for(r = 0; r < object_count; r++) {
		//find the dict key to use for this object
		res = m_cacheKeys.find(objects[r]);
		if(res == m_cacheKeys.end()) {
			_log(SERVICE__ERROR, "Unable to find cache key for object ID '%s', skipping.", objects[r]);
			continue;
		}

		//get the hint
		PyRep *cache_hint = GetCacheHint(objects[r]);
		if(cache_hint == NULL)
			continue;	//print already done.

		into->add(res->second.c_str(), cache_hint);
	}
}

bool ObjCacheService::IsCacheLoaded(const PyRep *objectID) const {
	return(m_cache.HaveCached(objectID));
}

void ObjCacheService::InvalidateCache(const PyRep *objectID) {
	m_cache.InvalidateCache(objectID);
}

void ObjCacheService::GiveCache(const PyRep *objectID, PyRep **contents) {
	//contents is consumed.
	m_cache.UpdateCache(objectID, contents);
}

PyRepObject *ObjCacheService::MakeObjectCachedMethodCallResult(const PyRep *objectID, const char *versionCheck) {
	if(!IsCacheLoaded(objectID))
		return(NULL);
	objectCaching_CachedMethodCallResult_object c;
	c.versionCheck = versionCheck;
	c.object = m_cache.MakeCacheHint(objectID);
	return(c.Encode());
}

ObjectCachedMethodID::ObjectCachedMethodID(const char *service, const char *method) {
	SimpleMethodCallID c;
	c.service = service;
	c.method = method;
	objectID = c.Encode();
}

ObjectCachedMethodID::~ObjectCachedMethodID() {
	delete objectID;
}





















