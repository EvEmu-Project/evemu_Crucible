/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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

#include "EvemuPCH.h"

PyCallable_Make_InnerDispatcher(SkillMgrService)
PyCallable_Make_Dispatcher(SkillMgrBound)


SkillMgrBound::SkillMgrBound(PyServiceMgr *mgr, CharacterDB *db)
: PyBoundObject(mgr),
  m_db(db),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(SkillMgrBound, CharStartTrainingSkill)
	PyCallable_REG_CALL(SkillMgrBound, GetEndOfTraining)
	PyCallable_REG_CALL(SkillMgrBound, GetSkillHistory)
	PyCallable_REG_CALL(SkillMgrBound, CharAddImplant)
	PyCallable_REG_CALL(SkillMgrBound, RemoveImplantFromCharacter)
}

SkillMgrBound::~SkillMgrBound()
{
	delete m_dispatch;
	m_dispatch = NULL;
}

// TODO: redesign this so this is not needed
void SkillMgrBound::Release()
{
	delete this;
}

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

PyResult SkillMgrBound::Handle_CharStartTrainingSkill(PyCallArgs &call) {
	//takes itemid
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return NULL;
	}

	InventoryItem *skill = m_manager->item_factory.Load(args.arg, false);
	if(skill == NULL) {
		codelog(CLIENT__ERROR, "%s: failed to load skill item %lu", call.client->GetName(), args.arg);
		return NULL;
	}
	
	//TODO: check to see that we are allowed to train this skill (ownership and prerequisits)
	call.client->Char()->TrainSkill(skill);
	skill->Release();
	
	return NULL;
}

PyResult SkillMgrBound::Handle_GetEndOfTraining(PyCallArgs &call) {
	//takes itemid
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return NULL;
	}

	PyRep *result = NULL;
	
	codelog(SERVICE__ERROR, "Unimplemented.");
	result = new PyRepInteger(
		//hack hack hack
		Win32TimeNow() + Win32Time_Month
		);

	return(result);
}

PyResult SkillMgrBound::Handle_GetSkillHistory(PyCallArgs &call) {
	_log(SERVICE__WARNING, "GetSkillHistory unimplemented.");

	util_Rowset rowset;

	rowset.header.push_back("logDateTime");
	rowset.header.push_back("eventID");
	rowset.header.push_back("skillTypeID");
	rowset.header.push_back("relativePoints");
	rowset.header.push_back("absolutePoints");

	return(rowset.Encode());
}

PyResult SkillMgrBound::Handle_CharAddImplant(PyCallArgs &call) {
	//takes itemid
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return NULL;
	}

	PyRep *result = NULL;
	
	codelog(SERVICE__ERROR, "Unimplemented.");

	return(result);
}

PyResult SkillMgrBound::Handle_RemoveImplantFromCharacter(PyCallArgs &call) {
	//takes itemid
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return NULL;
	}

	PyRep *result = NULL;
	
	codelog(SERVICE__ERROR, "Unimplemented.");

	return(result);
}
