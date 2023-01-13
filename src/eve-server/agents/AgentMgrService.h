/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
    Author:        Zhur
*/


#ifndef __AGENTMGR_SERVICE_H_INCL__
#define __AGENTMGR_SERVICE_H_INCL__

#include "agents/AgentDB.h"
#include "PyService.h"

class Agent;

class AgentMgrService : public BindableService <AgentMgrService> {
public:
    AgentMgrService(EVEServiceManager& mgr);

protected:
    PyResult GetAgents(PyCallArgs& call);
    PyResult GetSolarSystemOfAgent(PyCallArgs& call, PyInt* agentID);
    PyResult GetMyJournalDetails(PyCallArgs& call);
    PyResult GetMyEpicJournalDetails(PyCallArgs& call);
    PyResult GetCareerAgents(PyCallArgs& call);

    PyCallable_DECL_CALL(GetAgents);
    PyCallable_DECL_CALL(GetCareerAgents);
    PyCallable_DECL_CALL(GetMyJournalDetails);
    PyCallable_DECL_CALL(GetSolarSystemOfAgent);
    PyCallable_DECL_CALL(GetMyEpicJournalDetails);

    //overloaded in order to support bound objects:
    BoundDispatcher* BindObject(Client *client, PyRep* bindParameters) override;
};

class EpicArcService : public Service <EpicArcService> {
public:
    EpicArcService();

protected:
    PyResult AgentHasEpicMissionsForCharacter(PyCallArgs& call, PyInt* agentID);
};


#endif


