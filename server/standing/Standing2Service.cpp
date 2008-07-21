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



#include "Standing2Service.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "../cache/ObjCacheService.h"
#include "../packets/Standing2.h"
#include "../packets/General.h"

PyCallable_Make_InnerDispatcher(Standing2Service)

Standing2Service::Standing2Service(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "standing2"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(Standing2Service, GetMyKillRights)
	PyCallable_REG_CALL(Standing2Service, GetNPCNPCStandings)
	PyCallable_REG_CALL(Standing2Service, GetMyStandings)
	PyCallable_REG_CALL(Standing2Service, GetSecurityRating)
	PyCallable_REG_CALL(Standing2Service, GetStandingTransactions)
	
}

Standing2Service::~Standing2Service() {
	delete m_dispatch;
}


PyCallResult Standing2Service::Handle_GetMyKillRights(PyCallArgs &call) {
	PyRep *result = NULL;

	PyRepTuple *tu = new PyRepTuple(2);
	PyRepDict *u1 = new PyRepDict();
	PyRepDict *u2 = new PyRepDict();
	tu->items[0] = u1;
	tu->items[1] = u2;
	result = tu;
	
	return(result);
}

PyCallResult Standing2Service::Handle_GetMyStandings(PyCallArgs &call) {
	PyRep *result = NULL;

	PyRep *charstandings;
	PyRep *charprime;
	PyRep *npccharstandings;

	charstandings = m_db.GetCharStandings(call.client->GetCharacterID());
	charprime = m_db.GetCharPrimeStandings(call.client->GetCharacterID());
	npccharstandings = m_db.GetCharNPCStandings(call.client->GetCharacterID());
	
	PyRepDict *corpstandings = new PyRepDict();
	PyRepDict *corpprime = new PyRepDict();
	PyRepDict *npccorpstandings = new PyRepDict();
	
	PyRepTuple *tu = new PyRepTuple(6);
	tu->items[0] = charstandings;
	tu->items[1] = charprime;
	tu->items[2] = npccharstandings;
	tu->items[3] = corpstandings;
	tu->items[4] = corpprime;
	tu->items[5] = npccorpstandings;
	result = tu;
	
	return(result);
}


PyCallResult Standing2Service::Handle_GetNPCNPCStandings(PyCallArgs &call) {
	PyRep *result = NULL;

	ObjectCachedMethodID method_id(GetName(), "GetNPCNPCStandings");

	//check to see if this method is in the cache already.
	ObjCacheService *cache = m_manager->GetCache();
	if(!cache->IsCacheLoaded(method_id)) {
		//this method is not in cache yet, load up the contents and cache it.
		result = m_db.GetNPCStandings();
		if(result == NULL)
			result = new PyRepNone();
		cache->GiveCache(method_id, &result);
	}
	
	//now we know its in the cache one way or the other, so build a 
	//cached object cached method call result.
	result = cache->MakeObjectCachedMethodCallResult(method_id);
	
	return(result);
}

PyCallResult Standing2Service::Handle_GetSecurityRating(PyCallArgs &call) {
	//takes an integer: characterID
	PyRep *result = NULL;

	_log(SERVICE__ERROR, "%s: GetSecurityRating not implemented!", GetName());

	//returns the security status as a float.
	result = new PyRepReal(1.0);
	
	return(result);
}

PyCallResult Standing2Service::Handle_GetStandingTransactions(PyCallArgs &call) {
	Call_GetStandingTransactions args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
		return(NULL);
	}

	PyRepObject * result = m_db.GetStandingTransactions(args.toID);

	return (result);
}
























