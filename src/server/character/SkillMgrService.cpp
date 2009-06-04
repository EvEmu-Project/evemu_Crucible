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
	
	PyCallable_REG_CALL(SkillMgrBound, InjectSkillIntoBrain)
	PyCallable_REG_CALL(SkillMgrBound, CharStartTrainingSkillByTypeID)
	PyCallable_REG_CALL(SkillMgrBound, CharStopTrainingSkill)
	PyCallable_REG_CALL(SkillMgrBound, GetEndOfTraining)
	PyCallable_REG_CALL(SkillMgrBound, GetSkillHistory)
	PyCallable_REG_CALL(SkillMgrBound, CharAddImplant)
	PyCallable_REG_CALL(SkillMgrBound, RemoveImplantFromCharacter)
	PyCallable_REG_CALL(SkillMgrBound, GetSkillQueue)
	PyCallable_REG_CALL(SkillMgrBound, SaveSkillQueue)
	PyCallable_REG_CALL(SkillMgrBound, AddToEndOfSkillQueue)

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


PyResult SkillMgrBound::Handle_CharStopTrainingSkill(PyCallArgs &call) {
	Character *ch = call.client->Char();

	// clear & update ...
	ch->ClearSkillQueue();
	ch->UpdateSkillQueue();

 	return NULL;
 }

PyResult SkillMgrBound::Handle_GetEndOfTraining(PyCallArgs &call) {
	Character *ch = call.client->Char();

	return new PyRepInteger( ch->GetEndOfTraining() );
}

PyResult SkillMgrBound::Handle_GetSkillHistory(PyCallArgs &call) {
	_log(SERVICE__WARNING, "GetSkillHistory unimplemented.");

	util_Rowset rowset;

	rowset.header.push_back("logDateTime");
	rowset.header.push_back("eventID");
	rowset.header.push_back("skillTypeID");
	rowset.header.push_back("relativePoints");
	rowset.header.push_back("absolutePoints");

	return rowset.FastEncode();
}

PyResult SkillMgrBound::Handle_CharAddImplant(PyCallArgs &call) {
	//takes itemid
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return NULL;
	}

	codelog(CLIENT__ERROR, "%s: add implant arg: %u", call.client->GetName(), args.arg);
	codelog(SERVICE__ERROR, "Unimplemented.");

	return(NULL);
}

PyResult SkillMgrBound::Handle_RemoveImplantFromCharacter(PyCallArgs &call) {
	//takes itemid
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return NULL;
	}

	codelog(CLIENT__ERROR, "%s: remove implant arg: %u", call.client->GetName(), args.arg);
	codelog(SERVICE__ERROR, "Unimplemented.");

	return(NULL);
}

PyResult SkillMgrBound::Handle_GetSkillQueue(PyCallArgs &call) {
	// returns list of skills currently in the skill queue.
	Character *ch = call.client->Char();

	return ch->GetSkillQueue();
}

 PyResult SkillMgrBound::Handle_SaveSkillQueue(PyCallArgs &call) {
	Call_SaveSkillQueue args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return NULL;
	}

	Character *ch = call.client->Char();

	ch->ClearSkillQueue();

	std::vector<PyRep *>::iterator cur, end;
	cur = args.queue.items.begin();
	end = args.queue.items.end();
	for (; cur != end; cur++) {
		// take ownership
		PyRep *r = *cur;
		*cur = NULL;

		SkillQueue_Element el;
		if( !el.Decode( &r ) )
		{
			_log(CLIENT__ERROR, "%s: Failed to decode element of SkillQueue. Skipping.", call.client->GetName());
			continue;
		}

		ch->AddToSkillQueue( el.typeID, el.level );
	}

	ch->UpdateSkillQueue();

	return NULL;
 }

PyResult SkillMgrBound::Handle_AddToEndOfSkillQueue(PyCallArgs &call) {
	Call_TwoIntegerArgs args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return NULL;
	}

	Character *ch = call.client->Char();
	ch->AddToSkillQueue(args.arg1, args.arg2);
	ch->UpdateSkillQueue();
	return NULL;
}

PyResult SkillMgrBound::Handle_GetRespecInfo(PyCallArgs &call) {
	// takes no arguments
	_log(SERVICE__MESSAGE, "%s: GetRespecInfo unimplemented.", GetBindStr().c_str());

	// return dict
	PyRepDict *result = new PyRepDict;
	result->add("freeRespecs", new PyRepInteger(0));
	result->add("nextRespecTime", new PyRepInteger(Win32TimeNow() + Win32Time_Year));

	return(result);
}

PyResult SkillMgrBound::Handle_CharStartTrainingSkillByTypeID(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return NULL;
	}

	_log(SERVICE__ERROR, "%s: CharStartTrainingSkillByTypeID unimplemented.", call.client->GetName());
	return NULL;
}

PyResult SkillMgrBound::Handle_InjectSkillIntoBrain(PyCallArgs &call) {
	Call_InjectSkillIntoBrain args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return NULL;
	}

	Character *ch = call.client->Char();
	
	std::vector<uint32>::iterator cur, end;
	cur = args.skills.begin();
	end = args.skills.end();
	for (; cur != end; cur++) {
		InventoryItem *skill = m_manager->item_factory.GetItem( *cur , false);
		if(skill == NULL) {
			codelog(ITEM__ERROR, "%s: failed to load skill item %u for injection.", call.client->GetName(), *cur );
			continue;
		}
		
		if(!ch->InjectSkillIntoBrain(skill)) {
			//TODO: build and send UserError about injection failure.
			codelog(ITEM__ERROR, "%s: Injection of skill %u failed", call.client->GetName(), skill->itemID() );
		}

		skill->DecRef();
	}

	// TODO: send notification that the skill(s) injection was successful.
	return NULL;
}
