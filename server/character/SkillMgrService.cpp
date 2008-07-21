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


#include "SkillMgrService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../common/EVEUtils.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "../PyBoundObject.h"
#include "../inventory/InventoryItem.h"
#include "../inventory/ItemFactory.h"

#include "../packets/General.h"

PyCallable_Make_InnerDispatcher(SkillMgrService)




class SkillMgrBound
: public PyBoundObject {
public:
	
	PyCallable_Make_Dispatcher(SkillMgrBound)
	
	SkillMgrBound(PyServiceMgr *mgr, CharacterDB *db)
	: PyBoundObject(mgr, "SkillMgrBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(SkillMgrBound, CharStartTrainingSkill)
		PyCallable_REG_CALL(SkillMgrBound, GetEndOfTraining)
		PyCallable_REG_CALL(SkillMgrBound, CharAddImplant)
		PyCallable_REG_CALL(SkillMgrBound, RemoveImplantFromCharacter)
	}
	virtual ~SkillMgrBound() { delete m_dispatch; }
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL(CharStartTrainingSkill)
	PyCallable_DECL_CALL(GetEndOfTraining)
	PyCallable_DECL_CALL(CharAddImplant)
	PyCallable_DECL_CALL(RemoveImplantFromCharacter)

protected:
	CharacterDB *const m_db;
	Dispatcher *const m_dispatch;   //we own this
};



SkillMgrService::SkillMgrService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "skillMgr"),
m_dispatch(new Dispatcher(this)),
m_db(db)
{
	_SetCallDispatcher(m_dispatch);
}

SkillMgrService::~SkillMgrService() {
	delete m_dispatch;
}



PyBoundObject *SkillMgrService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	_log(CLIENT__MESSAGE, "SkillMgrService bind request for:");
	bind_args->Dump(CLIENT__MESSAGE, "    ");

	return(new SkillMgrBound(m_manager, &m_db));
}


PyCallResult SkillMgrBound::Handle_CharStartTrainingSkill(PyCallArgs &call) {
	//takes itemid
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return(NULL);
	}

	InventoryItem *skill = m_manager->item_factory->Load(args.arg, false);
	if(skill == NULL) {
		codelog(CLIENT__ERROR, "%s: failed to load skill item %lu", call.client->GetName(), args.arg);
		return(NULL);
	}
	
	//TODO: check to see that we are allowed to train this skill (ownership and prerequisits)
	call.client->Item()->TrainSkill(skill);
	skill->Release();
	
	return(NULL);
}


PyCallResult SkillMgrBound::Handle_GetEndOfTraining(PyCallArgs &call) {
	//takes itemid
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return(NULL);
	}

	PyRep *result = NULL;
	
	codelog(SERVICE__ERROR, "Unimplemented.");
	result = new PyRepInteger(
		//hack hack hack
		Win32TimeNow() + Win32Time_Month
		);

	return(result);
}


PyCallResult SkillMgrBound::Handle_CharAddImplant(PyCallArgs &call) {
	//takes itemid
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return(NULL);
	}

	PyRep *result = NULL;
	
	codelog(SERVICE__ERROR, "Unimplemented.");

	return(result);
}


PyCallResult SkillMgrBound::Handle_RemoveImplantFromCharacter(PyCallArgs &call) {
	//takes itemid
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return(NULL);
	}

	PyRep *result = NULL;
	
	codelog(SERVICE__ERROR, "Unimplemented.");

	return(result);
}




























