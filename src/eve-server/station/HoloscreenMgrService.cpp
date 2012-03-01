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
	Author:		Ubiquitatis
*/

#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(HoloscreenMgrService)

HoloscreenMgrService::HoloscreenMgrService(PyServiceMgr* mgr) : PyService(mgr, "holoscreenMgr"), m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(HoloscreenMgrService, GetTwoHourCache)
    PyCallable_REG_CALL(HoloscreenMgrService, GetRecentEpicArcCompletions)
	PyCallable_REG_CALL(HoloscreenMgrService, GetRuntimeCache)
}

HoloscreenMgrService::~HoloscreenMgrService()
{
    delete m_dispatch;
}

PyResult HoloscreenMgrService::Handle_GetTwoHourCache(PyCallArgs& call)
{
	sLog.Debug("HoloscreenMgrService", "Called GetTwoHourCache stub.");

    return new PyNone();
}

PyResult HoloscreenMgrService::Handle_GetRecentEpicArcCompletions(PyCallArgs& call)
{
	sLog.Debug("HoloscreenMgrService", "Called GetTwoHourCache stub.");

    return new PyNone();
}

PyResult HoloscreenMgrService::Handle_GetRuntimeCache(PyCallArgs& call)
{
	sLog.Debug("HoloscreenMgrService", "Called GetTwoHourCache stub.");

    return new PyNone();
}