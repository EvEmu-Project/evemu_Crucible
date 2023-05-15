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
    Author:        caytchen
*/

#include "eve-server.h"



#include "character/AggressionMgrService.h"

AggressionMgrBound::AggressionMgrBound(EVEServiceManager& mgr, AggressionMgrService& parent, uint32 systemID) :
    EVEBoundObject (mgr, parent),
    m_systemID (systemID)
{
    this->Add("GetCriminalTimeStamps", &AggressionMgrBound::GetCriminalTimeStamps);
    this->Add("CheckLootRightExceptions", &AggressionMgrBound::CheckLootRightExceptions);
}

PyResult AggressionMgrBound::GetCriminalTimeStamps(PyCallArgs &call, PyInt* characterID)
{
    return new PyDict();
}

PyResult AggressionMgrBound::CheckLootRightExceptions(PyCallArgs &call, PyInt* containerID)
{
    // return true to allow looting
    return new PyBool(true);
}

AggressionMgrService::AggressionMgrService(EVEServiceManager& mgr) :
    BindableService ("aggressionMgr", mgr)
{
}

BoundDispatcher* AggressionMgrService::BindObject(Client* client, PyRep* bindParameters) {
    if (bindParameters->IsInt() == false) {
        throw CustomError("Cannot bind service");
    }

    uint32 systemID = bindParameters->AsInt()->value();
    auto it = this->m_instances.find (systemID);

    if (it != this->m_instances.end ())
        return it->second;

    AggressionMgrBound* bound = new AggressionMgrBound(this->GetServiceManager(), *this, bindParameters->AsInt()->value());

    this->m_instances.insert_or_assign (systemID, bound);

    return bound;
}

void AggressionMgrService::BoundReleased (AggressionMgrBound* bound) {
    auto it = this->m_instances.find (bound->GetSystemID());

    if (it == this->m_instances.end ())
        return;

    this->m_instances.erase (it);
}