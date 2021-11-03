/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Updates:    Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "character/SkillMgrService.h"

PyCallable_Make_InnerDispatcher(SkillMgrService)
PyCallable_Make_InnerDispatcher(SkillMgrBound)

SkillMgrService::SkillMgrService(PyServiceMgr *mgr)
: PyService(mgr, "skillMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);
}

SkillMgrService::~SkillMgrService() {
    delete m_dispatch;
}

PyBoundObject *SkillMgrService::CreateBoundObject(Client *pClient, const PyRep *bind_args) {
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

    m_strBoundObjectName = "SkillMgrBound";

    PyCallable_REG_CALL(SkillMgrBound, InjectSkillIntoBrain);
    PyCallable_REG_CALL(SkillMgrBound, GetSkillQueueAndFreePoints);
    PyCallable_REG_CALL(SkillMgrBound, SaveSkillQueue);
    PyCallable_REG_CALL(SkillMgrBound, AddToEndOfSkillQueue);
    PyCallable_REG_CALL(SkillMgrBound, CharStartTrainingSkill);
    PyCallable_REG_CALL(SkillMgrBound, CharStartTrainingSkillByTypeID);
    PyCallable_REG_CALL(SkillMgrBound, CharStopTrainingSkill);
    PyCallable_REG_CALL(SkillMgrBound, GetEndOfTraining);
    PyCallable_REG_CALL(SkillMgrBound, GetSkillHistory);
    PyCallable_REG_CALL(SkillMgrBound, CharAddImplant);
    PyCallable_REG_CALL(SkillMgrBound, RemoveImplantFromCharacter);
    PyCallable_REG_CALL(SkillMgrBound, GetRespecInfo);
    PyCallable_REG_CALL(SkillMgrBound, RespecCharacter);
    PyCallable_REG_CALL(SkillMgrBound, GetCharacterAttributeModifiers);
}

SkillMgrBound::~SkillMgrBound()
{
    delete m_dispatch;
}

/** @todo redesign this so this is not needed */
void SkillMgrBound::Release()
{
    delete this;
}

PyResult SkillMgrBound::Handle_GetRespecInfo( PyCallArgs& call ) {
    return m_db.GetRespecInfo(call.client->GetCharacterID());
}

PyResult SkillMgrBound::Handle_GetSkillQueueAndFreePoints(PyCallArgs &call) {
    return call.client->GetChar()->SendSkillQueue();
}

PyResult SkillMgrBound::Handle_GetEndOfTraining(PyCallArgs &call) {
    return new PyLong( call.client->GetChar()->GetEndOfTraining() );
}

PyResult SkillMgrBound::Handle_GetSkillHistory( PyCallArgs& call ) {
    return call.client->GetChar()->GetSkillHistory();
}

PyResult SkillMgrBound::Handle_CharStopTrainingSkill(PyCallArgs &call) {
    // called when pausing skill queue
    call.client->GetChar()->PauseSkillQueue();
    // returns nothing
    return nullptr;
}

PyResult SkillMgrBound::Handle_CharStartTrainingSkill( PyCallArgs& call ) {
    // sm.GetService('godma').GetSkillHandler().CharStartTrainingSkill(skillX.itemID, skillX.locationID)
    Call_TwoIntegerArgs args;
    if ( !args.Decode( call.tuple ) )
    {
        codelog( SERVICE__ERROR, "%s: Failed to decode arguments.", GetName() );
        return nullptr;
    }

    _log(SKILL__WARNING, "Called CharStartTrainingSkill for itemID %i in location %i", args.arg1, args.arg2);
    return nullptr;
}

PyResult SkillMgrBound::Handle_AddToEndOfSkillQueue(PyCallArgs &call) {
    //  sm.StartService('godma').GetSkillHandler().AddToEndOfSkillQueue(skillID, nextLevel)
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    CharacterRef cRef(call.client->GetChar());
    cRef->AddToSkillQueue(args.arg1, args.arg2);
    cRef->UpdateSkillQueueEndTime();
    return nullptr;
}

PyResult SkillMgrBound::Handle_InjectSkillIntoBrain(PyCallArgs &call)
{
    Call_InjectSkillIntoBrain args;
    if (!args.Decode(&call.tuple)) {
        codelog( SERVICE__ERROR, "%s: Failed to decode arguments.", GetName() );
        return nullptr;
    }

    // make a list of skills successfully injected to display after injection
    // name, ret value  where 1=success, 2=prereqs, 3=already known, 4=split fail, 5=load fail
    std::map<std::string, uint8> skills;
    SkillRef skill(nullptr);
    CharacterRef cRef(call.client->GetChar());
    for (auto cur : args.skills)  {
        skill = sItemFactory.GetSkillRef(cur);
        if (skill.get() == nullptr) {
            _log( ITEM__ERROR, "%s: failed to load skill %u for injection.", call.client->GetName(), cur);
            std::string str = "Invalid Name #";
            str += std::to_string(cur);
            skills.emplace(str, 5);
            continue;
        }

        skills.emplace(skill->itemName(), cRef->InjectSkillIntoBrain(skill));
    }

    // build and populate status reply
    if (skills.empty())
        return nullptr;

    if (skills.size() == 1) {
        std::string status;
        switch (skills.begin()->second) {
            //1=success, 2=prereqs, 3=already known, 4=split fail, 5=load fail
            case 1: status = "<color=green>Success.</color>"; break;
            case 2: status = "<color=red>Failed:</color> <color=yellow>Prerequisites incomplete.</color>"; break;
            case 3: status = "<color=red>Failed:</color> <color=cyan>Skill already known.</color>"; break;
            case 4: status = "<color=red>Failed:</color> <color=red>Stack split failure.</color>"; break;
            case 5: status = "<color=red>Failed:</color> <color=maroon>Skill loading failure.</color>"; break;
            default: status = "<color=red>Failed:</color> <color=red>Unknown Error.</color>"; break;
        }
        call.client->SendInfoModalMsg("Injection of %s:  %s", skills.begin()->first.c_str(), status.c_str());
    } else {
        std::string status;
        std::ostringstream str;
        str.clear();
        str << "The Injection of %u skills for %s has resulted in the following outcome.<br><br>"; //40

        for (auto cur : skills) {
            switch (cur.second) {
                //1=success, 2=prereqs, 3=already known, 4=split fail, 5=load fail
                case 1: status = "<color=green>Success.</color>"; break;
                case 2: status = "<color=red>Failed:</color> <color=yellow>Prerequisites incomplete.</color>"; break;
                case 3: status = "<color=red>Failed:</color> <color=cyan>Skill already known.</color>"; break;
                case 4: status = "<color=red>Failed:</color> <color=red>Stack split failure.</color>"; break;
                case 5: status = "<color=red>Failed:</color> <color=maroon>Skill loading failure.</color>"; break;
                default: status = "<color=red>Failed:</color> <color=red>Unknown Error.</color>"; break;
            }
            str << cur.first << " - " << status << "<br>"; //40 for name, 80 for status (120)
        }

        int size = skills.size() * 120;
        size += 100;    // for header, including char name
        char reply[size];
        snprintf(reply, size, str.str().c_str(), skills.size(), call.client->GetName());

        call.client->SendInfoModalMsg(reply);
    }

    PyTuple* tmp = new PyTuple(1);
    tmp->SetItem(0, new PyString("OnSkillInjected"));
    call.client->QueueDestinyEvent(&tmp);
    return nullptr;
}

PyResult SkillMgrBound::Handle_SaveSkillQueue(PyCallArgs &call) {
    // called when previously-set skill queue changed
    Call_SaveSkillQueue args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    // xml decode will now check for and fix level being a float instead of int and leading to client freakout
    CharacterRef cRef(call.client->GetChar());
    _log(SKILL__QUEUE, "%s(%u) calling SaveSkillQueue()", cRef->name(), cRef->itemID());
    cRef->ClearSkillQueue(true);
    SkillQueue_Element el;
    std::vector<PyRep*>::const_iterator itr = args.queue->begin(), end = args.queue->end();
    for (; itr != end; ++itr) {
        if (!el.Decode(*itr))         {
            _log(SERVICE__ERROR, "%s: Failed to decode element of SkillQueue (%u). Skipping.", call.client->GetName(), PyRep::IntegerValueU32(*itr));
            continue;
        }
        cRef->AddToSkillQueue( el.typeID, el.level );
    }

    cRef->UpdateSkillQueueEndTime();
    PyTuple* tmp = new PyTuple(1);
        tmp->SetItem(0, new PyString("OnSkillTrainingSaved"));
    call.client->QueueDestinyEvent(&tmp);
    return nullptr;
}

PyResult SkillMgrBound::Handle_CharStartTrainingSkillByTypeID( PyCallArgs& call )
{
    // called when skill queue empty or paused
    // sends skill typeID to start training
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog( SERVICE__ERROR, "%s: Failed to decode arguments.", GetName() );
        return nullptr;
    }

    call.client->GetChar()->LoadPausedSkillQueue(args.arg);
    return nullptr;
}

PyResult SkillMgrBound::Handle_RespecCharacter(PyCallArgs &call)
{
    Call_RespecCharacter args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    CharacterRef cRef(call.client->GetChar());
    if (cRef->GetSkillInTraining() != nullptr)
        throw UserError ("RespecSkillInTraining");

    // return early if this is an illegal call
    if (!m_db.ReportRespec(call.client->GetCharacterID()))
        return nullptr;
    uint8 multiplier(sConfig.character.statMultiplier);
    cRef->SetAttribute(AttrCharisma, args.charisma * multiplier);
    cRef->SetAttribute(AttrIntelligence, args.intelligence * multiplier);
    cRef->SetAttribute(AttrMemory, args.memory * multiplier);
    cRef->SetAttribute(AttrPerception, args.perception * multiplier);
    cRef->SetAttribute(AttrWillpower, args.willpower * multiplier);
    cRef->SaveAttributes();

    // no return value
    return nullptr;
}

PyResult SkillMgrBound::Handle_GetCharacterAttributeModifiers(PyCallArgs &call)
{
    //  for (itemID, typeID, operation, value,) in modifiers:

    /*
     * client sends attrib# of stat in question...
     *            [PyString "GetCharacterAttributeModifiers"]
     *            [PyTuple 1 items]
     *              [PyInt 165]
     * we return this...
     *        [PyList 1 items]
     *          [PyTuple 4 items]
     *            [PyIntegerVar 1866309449]   << implantID
     *            [PyInt 9943]                << implantTypeID
     *            [PyInt 2]                   << operation
     *            [PyFloat 3]                 << value
     */
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    CharacterRef cRef(call.client->GetChar());
    PyList* list = new PyList();
    // for each implant, make tuple and put into list
    PyTuple* tuple = new PyTuple(4);
        tuple->SetItem(0, PyStatic.NewZero());   //implantID
        tuple->SetItem(1, PyStatic.NewZero());   //implantTypeID
        tuple->SetItem(2, PyStatic.NewZero());   //operation
        tuple->SetItem(3, PyStatic.NewZero());   //value
        list->AddItem(tuple);

    return list;
}

PyResult SkillMgrBound::Handle_CharAddImplant( PyCallArgs& call )
{
    //sends itemid
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    //{'FullPath': u'UI/Messages', 'messageID': 259242, 'label': u'OnlyOneBoosterActiveBody'}(u'You cannot consume the {typeName} as you are already using another similar booster {typeName2}.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}, u'{typeName2}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName2'}})
    //{'FullPath': u'UI/Messages', 'messageID': 259243, 'label': u'OnlyOneImplantActiveBody'}(u'You cannot install the {typeName} as there is already an implant installed in the slot it needs to occupy.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})

    return nullptr;
}

PyResult SkillMgrBound::Handle_RemoveImplantFromCharacter( PyCallArgs& call )
{
    //sends itemid
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return nullptr;
}
