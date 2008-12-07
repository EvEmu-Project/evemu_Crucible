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

#include "EvemuPCH.h"

PyCallable_Make_InnerDispatcher(CertificateMgrService)

CertificateMgrService::CertificateMgrService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "certificateMgr"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(CertificateMgrService, GetMyCertificates)
	PyCallable_REG_CALL(CertificateMgrService, GetCertificateCategories)
	PyCallable_REG_CALL(CertificateMgrService, GetAllShipCertificateRecommendations)
	PyCallable_REG_CALL(CertificateMgrService, GetCertificateClasses)
	PyCallable_REG_CALL(CertificateMgrService, GrantCertificate)
	PyCallable_REG_CALL(CertificateMgrService, BatchCertificateGrant)
	PyCallable_REG_CALL(CertificateMgrService, BatchCertificateUpdate)
}

PyResult CertificateMgrService::Handle_GetMyCertificates(PyCallArgs &call) {
	return(m_db.GetMyCertificates(call.client->GetCharacterID()));
}

PyResult CertificateMgrService::Handle_GetCertificateCategories(PyCallArgs &call) {
	ObjectCachedMethodID method_id(GetName(), "GetCertificateCategories");

	ObjCacheService *cache = m_manager->GetCache();
	if(!cache->IsCacheLoaded(method_id)) {
		PyRep *res = m_db.GetCertificateCategories();
		if(res == NULL) {
			codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
			res = new PyRepNone();
		}
		cache->GiveCache(method_id, &res);
	}

	return(cache->MakeObjectCachedMethodCallResult(method_id));
}

PyResult CertificateMgrService::Handle_GetAllShipCertificateRecommendations(PyCallArgs &call) {
	ObjectCachedMethodID method_id(GetName(), "GetAllShipCertificateRecommendations");

	ObjCacheService *cache = m_manager->GetCache();
	if(!cache->IsCacheLoaded(method_id)) {
		PyRep *res = m_db.GetAllShipCertificateRecommendations();
		if(res == NULL) {
			codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
			res = new PyRepNone();
		}
		cache->GiveCache(method_id, &res);
	}

	return(cache->MakeObjectCachedMethodCallResult(method_id));
}

PyResult CertificateMgrService::Handle_GetCertificateClasses(PyCallArgs &call) {
	ObjectCachedMethodID method_id(GetName(), "GetCertificateClasses");

	ObjCacheService *cache = m_manager->GetCache();
	if(!cache->IsCacheLoaded(method_id)) {
		PyRep *res = m_db.GetCertificateClasses();
		if(res == NULL) {
			codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
			res = new PyRepNone();
		}
		cache->GiveCache(method_id, &res);
	}

	return(cache->MakeObjectCachedMethodCallResult(method_id));
}

PyResult CertificateMgrService::Handle_GrantCertificate(PyCallArgs &call) {
	Call_SingleIntegerArg arg;
	if(!arg.Decode(&call.tuple)) {
		_log(CLIENT__ERROR, "Failed to decode args.");
		return(NULL);
	}

	return(_GrantCertificate(call.client->GetCharacterID(), arg.arg) ? new PyRepInteger(arg.arg) : NULL);
}

PyResult CertificateMgrService::Handle_BatchCertificateGrant(PyCallArgs &call) {
	Call_SingleIntList arg;
	if(!arg.Decode(&call.tuple)) {
		_log(CLIENT__ERROR, "Failed to decode args.");
		return(NULL);
	}

	PyRepList *res = new PyRepList;

	std::vector<uint32>::iterator cur, end;
	cur = arg.ints.begin();
	end = arg.ints.end();
	for(; cur != end; cur++) {
		if(_GrantCertificate(call.client->GetCharacterID(), *cur))
			res->addInt(*cur);
	}

	return(res);
}

PyResult CertificateMgrService::Handle_BatchCertificateUpdate(PyCallArgs &call) {
	Call_BatchCertificateUpdate args;
	if(!args.Decode(&call.tuple)) {
		_log(CLIENT__ERROR, "Failed to decode args.");
		return(NULL);
	}

	std::map<uint32, uint32>::iterator cur, end;
	cur = args.update.begin();
	end = args.update.end();
	for(; cur != end; cur++)
		_UpdateCertificate(call.client->GetCharacterID(), cur->first, cur->second);

	return(NULL);
}

bool CertificateMgrService::_GrantCertificate(uint32 characterID, uint32 certificateID) {
	_log(SERVICE__MESSAGE, "%lu asked to grant certificate %lu.", characterID, certificateID);
	_log(SERVICE__ERROR, "Granting certificates not supported yet.");

	return(false);
}

bool CertificateMgrService::_UpdateCertificate(uint32 characterID, uint32 certificateID, bool pub) {
	_log(SERVICE__MESSAGE, "%lu asked to make his certificate %lu %s.", characterID, certificateID, (pub ? "public" : "private"));
	_log(SERVICE__ERROR, "Updating certificates not supported yet.");

	return(false);
}


