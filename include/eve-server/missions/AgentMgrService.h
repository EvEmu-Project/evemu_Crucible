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
	Author:		Zhur
*/


#ifndef __AGENTMGR_SERVICE_H_INCL__
#define __AGENTMGR_SERVICE_H_INCL__

#include "missions/MissionDB.h"
#include "PyService.h"

class Agent;

class AgentMgrService : public PyService
{
public:
	AgentMgrService(PyServiceMgr *mgr);
	virtual ~AgentMgrService();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;
	
	MissionDB m_db;

	//for now this lives here, might want to move eventually.
	std::map<uint32, Agent *> m_agents;	//we own these
	Agent *_GetAgent(uint32 agentID);

	PyCallable_DECL_CALL(GetAgents)
	PyCallable_DECL_CALL(GetMyJournalDetails)
	PyCallable_DECL_CALL(GetMyEpicJournalDetails)
	
	//overloaded in order to support bound objects:
	virtual PyBoundObject *_CreateBoundObject(Client *c, const PyRep *bind_args);
};




#endif


