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
 * AgentMgr bound(agentID):
 *  -> DoAction(actionID or None)
 *  -> WarpToLocation(locationType, locationNumber, warpRange, is_gang)
 *
 *   Also sent an OnRemoteMessage(AgtMissionOfferWarning)
 *
 *   and various OnAgentMissionChange()
 *
*/

#include "AgentMgrService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "../PyBoundObject.h"
#include "../cache/ObjCacheService.h"
#include "Agent.h"

#include "../packets/General.h"
#include "../packets/Missions.h"

PyCallable_Make_InnerDispatcher(AgentMgrService)

class AgentMgrBound
: public PyBoundObject {
public:
	
	PyCallable_Make_Dispatcher(AgentMgrBound)
	
	AgentMgrBound(PyServiceMgr *mgr, MissionDB *db, Agent *agt)
	: PyBoundObject(mgr, "AgentMgrBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this)),
	  m_agent(agt)
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(AgentMgrBound, GetInfoServiceDetails)
		PyCallable_REG_CALL(AgentMgrBound, DoAction)
		PyCallable_REG_CALL(AgentMgrBound, GetMyJournalDetails)
	}
	virtual ~AgentMgrBound() { delete m_dispatch; }
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL(GetInfoServiceDetails)
	PyCallable_DECL_CALL(DoAction)
	PyCallable_DECL_CALL(GetMyJournalDetails)

protected:
	MissionDB *const m_db;		//we do not own this
	Dispatcher *const m_dispatch;	//we own this
	Agent *const m_agent;	//we do not own this.
};

AgentMgrService::AgentMgrService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "agentMgr"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(AgentMgrService, GetAgents)
	PyCallable_REG_CALL(AgentMgrService, GetMyJournalDetails)
}

AgentMgrService::~AgentMgrService() {
	delete m_dispatch;
	std::map<uint32, Agent *>::iterator cur, end;
	cur = m_agents.begin();
	end = m_agents.end();
	for(; cur != end; cur++) {
		delete cur->second;
	}
}

Agent *AgentMgrService::_GetAgent(uint32 agentID) {
	std::map<uint32, Agent *>::iterator res;
	res = m_agents.find(agentID);
	if(res != m_agents.end())
		return(res->second);
	Agent *a = new Agent(agentID);
	if(!a->Load(&m_db)) {
		delete a;
		return(NULL);
	}
	m_agents[agentID] = a;
	return(a);
}

PyBoundObject *AgentMgrService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	_log(CLIENT__MESSAGE, "%s bind request for:", GetName());
	bind_args->Dump(CLIENT__MESSAGE, "    ");

	if(!bind_args->CheckType(PyRep::Integer)) {
		codelog(CLIENT__ERROR, "%s: Non-integer bind argument '%s'", c->GetName(), bind_args->TypeString());
		return(NULL);
	}
	
	const PyRepInteger *a = (const PyRepInteger *) bind_args;

	Agent *agent = _GetAgent(a->value);
	if(agent == NULL) {
		codelog(CLIENT__ERROR, "%s: Unable to obtain agent %lu", c->GetName(), a->value);
		return(NULL);
	}
	
	return(new AgentMgrBound(m_manager, &m_db, agent));
}


PyCallResult AgentMgrService::Handle_GetAgents(PyCallArgs &call) {
	PyRep *result = NULL;

	ObjectCachedMethodID method_id(GetName(), "GetAgents");

	//check to see if this method is in the cache already.
	ObjCacheService *cache = m_manager->GetCache();
	if(!cache->IsCacheLoaded(method_id)) {
		//this method is not in cache yet, load up the contents and cache it.
		result = m_db.GetAgents();
		if(result == NULL) {
			codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
			result = new PyRepNone();
		}
		cache->GiveCache(method_id, &result);
	}
	
	//now we know its in the cache one way or the other, so build a 
	//cached object cached method call result.
	result = cache->MakeObjectCachedMethodCallResult(method_id);
	
	return(result);
}

PyCallResult AgentMgrService::Handle_GetMyJournalDetails(PyCallArgs &call) {
	PyRep *result = NULL;

	PyRepTuple *t = new PyRepTuple(3);
	t->items[0] = new PyRepList();
	t->items[1] = new PyRepList();
	t->items[2] = new PyRepList();
	result = t;
	
	return(result);
}

PyCallResult AgentMgrBound::Handle_GetInfoServiceDetails(PyCallArgs &call) {
	//takes no arguments
	
	codelog(SERVICE__ERROR, "%s: GetInfoServiceDetails unimplemented.", GetName());
	
	return(new PyRepNone());
}

PyCallResult AgentMgrBound::Handle_DoAction(PyCallArgs &call) {
	//takes a single argument, which may be None, or may be an integer actionID
	Call_SingleArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Failed to decode args from '%s'", call.client->GetName());
		return(NULL);
	}
	
	uint32 actionID = 0;
	if(args.arg->CheckType(PyRep::Integer)) {
		PyRepInteger *i = (PyRepInteger *) args.arg;
		actionID = i->value;
	}
	
	uint32 loyaltyPoints;
	DoAction_Result res;
	std::map<uint32, std::string> choices;

	loyaltyPoints = m_agent->GetLoyaltyPoints(call.client);
	m_agent->DoAction(call.client, actionID, res.agentSays, choices);

	DoAction_Dialogue_Item choice;
	std::map<uint32, std::string>::iterator cur, end;
	cur = choices.begin();
	end = choices.end();
	for(; cur != end; cur++) {
		choice.actionID = cur->first;
		choice.actionText = cur->second;
		res.dialogue.add(choice.Encode());
	}

	//TODO: send loyaltyPoints in the keywords return.
	
	return(res.Encode());
}


PyCallResult AgentMgrBound::Handle_GetMyJournalDetails(PyCallArgs &call) {
	PyRep *result = NULL;

	PyRepTuple *t = new PyRepTuple(3);
	//missions:
	t->items[0] = new PyRepList();
	//offers:
	t->items[1] = new PyRepList();
	//research:
	t->items[2] = new PyRepList();
	result = t;
	
	return(result);
}






















