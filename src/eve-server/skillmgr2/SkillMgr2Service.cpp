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

#include "skillmgr2/SkillMgr2Service.h"
#include "skillmgr2/SkillHandler.h"

SkillMgr2Service::SkillMgr2Service(EVEServiceManager& mgr) :
    BindableService("skillMgr2", mgr)
{
    this->Add("GetMySkillHandler", &SkillMgr2Service::GetMySkillHandler);
    this->Add("GetSkills", &SkillMgr2Service::GetSkills);
    this->Add("GetSkillPoints", &SkillMgr2Service::GetSkillPoints);
    this->Add("GetSkillQueueAndFreePoints", &SkillMgr2Service::GetSkillQueueAndFreePoints);
    this->Add("GetFreeSkillPoints", &SkillMgr2Service::GetFreeSkillPoints);
    this->Add("CheckAndSendNotifications", &SkillMgr2Service::CheckAndSendNotifications);
    this->Add("GetBoosters", &SkillMgr2Service::GetBoosters);
}

void SkillMgr2Service::BoundReleased(SkillHandler* bound)
{
    sLog.Debug("SkillMgr2Service", "BoundReleased called");
}

BoundDispatcher* SkillMgr2Service::BindObject(Client* client, PyRep* bindParameters)
{
    sLog.Debug("SkillMgr2Service", "BindObject called");
    return new SkillHandler(this->GetServiceManager(), *this);
}

PyResult SkillMgr2Service::GetMySkillHandler(PyCallArgs& call)
{
    sLog.Debug("SkillMgr2Service", "GetMySkillHandler called - creating bound object");
    
    PyRep* bindParameters = new PyNone();
    
    BoundDispatcher* bound = this->BindObject(call.client, bindParameters);
    
    if (bound == nullptr) {
        sLog.Error("SkillMgr2Service", "BindObject returned nullptr");
        return PyStatic.NewNone();
    }
    
    bound->NewReference(call.client);
    
    PyDict* byName = new PyDict();
    byName->SetItem("OID+", bound->GetOID());
    
    PySubStruct* subStruct = new PySubStruct(new PySubStream(bound->GetOID()));
    
    PyResult result(subStruct, byName);
    return result;
}

PyResult SkillMgr2Service::GetSkills(PyCallArgs& call)
{
    sLog.Debug("SkillMgr2Service", "GetSkills called - stub");
    PyTuple* result = new PyTuple(2);
    result->SetItem(0, new PyLong(0));
    result->SetItem(1, new PyLong(0));
    return result;
}

PyResult SkillMgr2Service::GetSkillPoints(PyCallArgs& call)
{
    sLog.Debug("SkillMgr2Service", "GetSkillPoints called - stub");
    PyTuple* result = new PyTuple(2);
    result->SetItem(0, new PyLong(0));
    result->SetItem(1, new PyLong(0));
    return result;
}

PyResult SkillMgr2Service::GetSkillQueueAndFreePoints(PyCallArgs& call)
{
    sLog.Debug("SkillMgr2Service", "GetSkillQueueAndFreePoints called - returning empty queue and 0 free points");
    PyTuple* result = new PyTuple(2);
    result->SetItem(0, new PyList());
    result->SetItem(1, new PyLong(0));
    return result;
}

PyResult SkillMgr2Service::GetFreeSkillPoints(PyCallArgs& call)
{
    sLog.Debug("SkillMgr2Service", "GetFreeSkillPoints called - returning 0");
    return new PyLong(0);
}

PyResult SkillMgr2Service::CheckAndSendNotifications(PyCallArgs& call)
{
    sLog.Debug("SkillMgr2Service", "CheckAndSendNotifications called - stub");
    return PyStatic.NewNone();
}

PyResult SkillMgr2Service::GetBoosters(PyCallArgs& call)
{
    sLog.Debug("SkillMgr2Service", "GetBoosters called - returning empty dict");
    PyDict* result = new PyDict();
    return result;
}
