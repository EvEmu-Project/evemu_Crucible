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
    Author: Zhur
*/

#include "eve-server.h"

#include "skillmgr2/SkillHandler.h"
#include "skillmgr2/SkillMgr2Service.h"

SkillHandler::SkillHandler(EVEServiceManager& mgr, SkillMgr2Service& parent) :
    EVEBoundObject<SkillHandler>(mgr, parent),
    m_parent(parent)
{
    this->Add("GetSkills", &SkillHandler::GetSkills);
    this->Add("GetAttributes", &SkillHandler::GetAttributes);
    this->Add("GetSkillQueueAndFreePoints", &SkillHandler::GetSkillQueueAndFreePoints);
    this->Add("GetSkillHistory", &SkillHandler::GetSkillHistory);
}

PyResult SkillHandler::GetSkills(PyCallArgs& call)
{
    sLog.Debug("SkillHandler", "GetSkills called - returning character skills");
    
    CharacterRef cref = call.client->GetChar();
    if (!cref) {
        sLog.Error("SkillHandler", "GetSkills: No character found for client %u", call.client->GetCharacterID());
        return PyStatic.NewNone();
    }
    
    PyDict* skills = new PyDict();
    
    std::vector<InventoryItemRef> skillList;
    cref->GetSkillsList(skillList);
    
    for (auto skillItem : skillList) {
        if (!skillItem) continue;
        
        uint32 typeID = skillItem->typeID();
        uint32 level = skillItem->GetAttribute(AttrSkillLevel).get_uint32();
        int64 points = skillItem->GetAttribute(AttrSkillPoints).get_int();
        float rank = skillItem->GetAttribute(AttrSkillTimeConstant).get_float();
        
        PyDict* skillInfo = new PyDict();
        skillInfo->SetItem("skillLevel", new PyInt((int32)level));
        skillInfo->SetItem("skillPoints", new PyLong(points));
        skillInfo->SetItem("skillRank", new PyFloat(rank));
        
        skills->SetItem(new PyInt(typeID), new PyObject("util.KeyVal", skillInfo));
    }
    
    return skills;
}

PyResult SkillHandler::GetAttributes(PyCallArgs& call)
{
    sLog.Debug("SkillHandler", "GetAttributes called - returning character attributes");
    
    CharacterRef cref = call.client->GetChar();
    if (!cref) {
        sLog.Error("SkillHandler", "GetAttributes: No character found for client %u", call.client->GetCharacterID());
        return PyStatic.NewNone();
    }
    
    PyDict* attrs = new PyDict();
    attrs->SetItem(new PyString("intelligence"), cref->GetAttribute(AttrIntelligence).GetPyObject());
    attrs->SetItem(new PyString("perception"), cref->GetAttribute(AttrPerception).GetPyObject());
    attrs->SetItem(new PyString("charisma"), cref->GetAttribute(AttrCharisma).GetPyObject());
    attrs->SetItem(new PyString("willpower"), cref->GetAttribute(AttrWillpower).GetPyObject());
    attrs->SetItem(new PyString("memory"), cref->GetAttribute(AttrMemory).GetPyObject());
    
    return attrs;
}

PyResult SkillHandler::GetSkillQueueAndFreePoints(PyCallArgs& call)
{
    sLog.Debug("SkillHandler", "GetSkillQueueAndFreePoints called - returning skill queue");
    
    CharacterRef cref = call.client->GetChar();
    if (!cref) {
        sLog.Error("SkillHandler", "GetSkillQueueAndFreePoints: No character found for client %u", call.client->GetCharacterID());
        PyTuple* result = new PyTuple(2);
        result->SetItem(0, new PyList());
        result->SetItem(1, new PyLong(0));
        return result;
    }
    
    return cref->SendSkillQueue();
}

PyResult SkillHandler::GetSkillHistory(PyCallArgs& call)
{
    sLog.Debug("SkillHandler", "GetSkillHistory called - returning skill history");
    
    CharacterRef cref = call.client->GetChar();
    if (!cref) {
        sLog.Error("SkillHandler", "GetSkillHistory: No character found for client %u", call.client->GetCharacterID());
        PyList* result = new PyList();
        return result;
    }
    
    PyRep* history = cref->GetSkillHistory();
    if (!history) {
        sLog.Warning("SkillHandler", "GetSkillHistory: No skill history found for character %u", call.client->GetCharacterID());
        PyList* result = new PyList();
        return result;
    }
    
    return history;
}
