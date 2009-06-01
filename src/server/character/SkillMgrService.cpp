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
	call.client->Char()->StopTrainingSkill();

 	return NULL;
 }

PyResult SkillMgrBound::Handle_GetEndOfTraining(PyCallArgs &call) {
	InventoryItem *skill = call.client->Char()->GetSkillInTraining();
	PyRep *result = NULL;
	result = new PyRepInteger(skill->expiryTime());
	call.client->SetTrainStatus(true, skill->expiryTime());
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

	codelog(CLIENT__ERROR, "%s: add implant arg: %u", call.client->GetName(), args.arg);

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

	codelog(CLIENT__ERROR, "%s: remove implant arg: %u", call.client->GetName(), args.arg);

	PyRep *result = NULL;
	codelog(SERVICE__ERROR, "Unimplemented.");
	return(result);
}

PyResult SkillMgrBound::Handle_GetSkillQueue(PyCallArgs &call) {
	// returns list of skills currently in training
	PyRepList *result = new PyRepList;
	result = call.client->Char()->GetSkillsFromSkillQueue(call.client->Char()->itemID());
	return result;
}

 PyResult SkillMgrBound::Handle_SaveSkillQueue(PyCallArgs &call) {
	Call_SaveSkillQueue args;
	PyRep *result = NULL;

	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return(result);
	}
	
	// delete all skill queue rows for this character since we are about
	// to store new skill queue entries.
	if(!call.client->Char()->RemoveSkillsFromSkillQueue(call.client->Char()->itemID(), 0)) {
		codelog(CLIENT__TRACE, "%s: Failed to remove skills from skill queue for character %u.", call.client->GetName(), call.client->Char()->itemID());
		return(result);
	}

	uint32 typeID = 0;
	uint8 level = 0;
	uint32 itemID = 0;
	std::vector<PyRep *>::iterator cur, end;
	cur = args.arg.items.begin();
	end = args.arg.items.end();
	for (; cur != end; cur++) {

		// yea, the code below is the ugliest code EVAR, but I've tried probably 6 ways of getting both
		// the values out of this tuple, but I can never get both values out using []. If there is a
		// better way, let me know.
		typeID = 0;
		level = 0;
		std::vector<PyRep *>::iterator cur1, end1;
		cur1 = (*cur)->AsTuple().items.begin();
		end1 = (*cur)->AsTuple().items.end();
		for (; cur1 != end1; cur1++) { // loops 2 times
			if(typeID == 0)
			{
				typeID = (*cur1)->AsInteger().value;
			}
			else
			{
				level = (*cur1)->AsInteger().value;
			}
		}

		// grab the itemID of the first skill in the skill queue
		if(itemID == 0) {
			InventoryItem *skill = m_manager->item_factory.GetInvforType(typeID, call.client->Char()->itemID(), flagSkill, false);
			if(skill != NULL) {
				itemID = skill->itemID();
			}
			skill->DecRef();
		}

		if(!call.client->Char()->AddSkillToSkillQueue(call.client->Char()->itemID(), typeID, level)) {
			codelog(CLIENT__TRACE, "%s: Failed to add itemID: %u skillID:%u level: %u to the skill queue.", call.client->GetName(), call.client->Char()->itemID(), typeID , level );
		}
	}
	
	// TODO: Start training the first skill in the skillqueue.
	// find a much better way to do this.
	if(typeID != 0) { // crappy check to see if the loop looped, fix this later.
		InventoryItem *skill = m_manager->item_factory.GetInvforType(typeID, call.client->Char()->itemID(), flagSkill, false);
		skill->ChangeFlag(flagSkillInTraining, true);
	}

	args.arg.items.clear();
	return(result);
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

PyResult SkillMgrBound::Handle_CharStartTrainingSkillByTypeID(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	PyRep *result = NULL;

	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return(result);
	}

	InventoryItem *skillInTraining = m_manager->item_factory.GetInvforType(args.arg, call.client->Char()->itemID(), flagSkillInTraining, false);
	if(skillInTraining == NULL) {
		codelog(CLIENT__TRACE, "%s: Unable to get skill in training.  Continue and start training of passed skill.", call.client->GetName(), args.arg );
	}else{
		if(skillInTraining->typeID() == args.arg) {
			// TODO: Send notification that they are already training the passed skill.
			codelog(CLIENT__TRACE, "%s: Character already training skill %u.", call.client->GetName(), skillInTraining->itemID());
			return(result);
		}
		
		// stop training the current skill in training.
		// cheap hack to get the skill to stop training and it kinda works, but I will 
		// add a correct way later using TrainSkill.
		skillInTraining->ChangeFlag(flagSkill, true);
		skillInTraining->DecRef();
	}

	// start training passed skill if skill level is < 5.
	InventoryItem *skill = m_manager->item_factory.GetInvforType(args.arg, call.client->Char()->itemID(), flagSkill, false);
	if(skill == NULL) {
		codelog(CLIENT__TRACE, "%s: Unable to get skill to start training.", call.client->GetName());
		return(result);
	}
	
	// cheap hack to get the skill in training and it kinda works, but I will 
	// add a correct way later using TrainSkill.
	skill->ChangeFlag(flagSkillInTraining, true);
	skill->DecRef();
	return(result);
}


PyResult SkillMgrBound::Handle_InjectSkillIntoBrain(PyCallArgs &call) {
	Call_InjectSkillIntoBrain args;
	PyRep *result = NULL;

	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
		return(result);
	}
	
	std::vector<PyRep *>::iterator cur, end;
	cur = args.arg.items.begin();
	end = args.arg.items.end();
	for (; cur != end; cur++) {
		InventoryItem *skill = m_manager->item_factory.GetItem((*cur)->AsInteger().value , false);
		if(skill == NULL) {
			codelog(ITEM__ERROR, "%s: failed to load skill item %u", call.client->GetName(), (*cur)->AsInteger().value );
			continue;
		}
		
		if(call.client->Char()->SkillAlreadyInjected(skill)) {
			//TODO: build and send UserError for CharacterAlreadyKnowsSkill.
			codelog(ITEM__ERROR, "%s: Skill %u is already injected into character.", call.client->GetName(), (*cur)->AsInteger().value );
			continue;
		}

		if(!skill->SkillPrereqsComplete(skill)) {
			//TODO: build and send UserError for incomplete prerequisites.
			codelog(ITEM__ERROR, "%s: Cannot inject skill %u, prerequisites have not been met.", call.client->GetName(), (*cur)->AsInteger().value );
			continue;
		}

		if(!call.client->Char()->InjectSkillIntoBrain(skill)) {
			//TODO: build and send UserError about injection failure.
			codelog(ITEM__ERROR, "%s: Injection of skill %u failed", call.client->GetName(), (*cur)->AsInteger().value );
			continue;
		}
	}

	// TODO: send notification that the skill(s) injection was successful.
	args.arg.items.clear();
	return(result);
}
