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
PyCallable_Make_InnerDispatcher(SkillMgrBound)

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

	return(new SkillMgrBound(m_manager, m_db));
}

SkillMgrBound::SkillMgrBound(PyServiceMgr *mgr, CharacterDB &db)
: PyBoundObject(mgr),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(SkillMgrBound, CharStartTrainingSkill)
	PyCallable_REG_CALL(SkillMgrBound, GetEndOfTraining)
	PyCallable_REG_CALL(SkillMgrBound, GetSkillHistory)
	PyCallable_REG_CALL(SkillMgrBound, CharAddImplant)
	PyCallable_REG_CALL(SkillMgrBound, RemoveImplantFromCharacter)
	PyCallable_REG_CALL(SkillMgrBound, GetSkillQueue)
	PyCallable_REG_CALL(SkillMgrBound, SaveSkillQueue)
	PyCallable_REG_CALL(SkillMgrBound, GetRespecInfo)
}

SkillMgrBound::~SkillMgrBound()
{
	delete m_dispatch;
}

// TODO: redesign this so this is not needed
void SkillMgrBound::Release()
{
	delete this;
}

PyResult SkillMgrBound::Handle_CharStartTrainingSkill(PyCallArgs &call) {
	//takes itemid
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return NULL;
	}

	InventoryItem *skill = m_manager->item_factory.GetItem(args.arg, false);
	if(skill == NULL) {
		codelog(CLIENT__ERROR, "%s: failed to load skill item %u", call.client->GetName(), args.arg);
		return NULL;
	}
	
	//TODO: check to see that we are allowed to train this skill (ownership and prerequisits)
	call.client->Char()->TrainSkill(skill);
	skill->DecRef();
	
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

PyResult SkillMgrBound::Handle_GetSkillQueue(PyCallArgs &call) {
	// returns list of skills currently in training
	PyRepList *result = new PyRepList;

	// fill the result
	/*
	PyRepTuple *t;

	t = new PyRepTuple(2);
	t->items[0] = new PyRepInteger(3300); // Gunnery
	t->items[1] = new PyRepInteger(4); // Level to be trained
	result->add(t);

	t = new PyRepTuple(2);
	t->items[0] = new PyRepInteger(3327); // Spaceship command
	t->items[1] = new PyRepInteger(4);
	result->add(t);
	*/

	// return the result
	return result;
}

PyResult SkillMgrBound::Handle_SaveSkillQueue(PyCallArgs &call) {
	// Takes list of skills to train;
	// exactly same format as GetSkillQueue.

	_log(SERVICE__MESSAGE, "%s: SaveSkillQueue unimplemented.", GetBindStr().c_str());

	return NULL;
}

PyResult SkillMgrBound::Handle_GetRespecInfo(PyCallArgs &call) {
	// takes no arguments

	_log(SERVICE__MESSAGE, "%s: GetRespecInfo unimplemented.", GetBindStr().c_str());

	// return dict
	PyRepDict *result = new PyRepDict;
	result->add("freeRespecs", new PyRepInteger(0));
	result->add("nextRespecTime", new PyRepInteger(Win32TimeNow() + Win32Time_Year));

	return result;
}
