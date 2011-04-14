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


#include "EVEServerPCH.h"


PyCallable_Make_InnerDispatcher(OnlineStatusService)


OnlineStatusService::OnlineStatusService(PyServiceMgr *mgr)
: PyService(mgr, "onlineStatus"),
m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(OnlineStatusService, GetInitialState)
}

OnlineStatusService::~OnlineStatusService() {
	delete m_dispatch;
}


PyResult OnlineStatusService::Handle_GetInitialState(PyCallArgs &call) {

	//this is used to query the initial online state of all friends.

	util_Rowset rs;
    rs.lines = new PyList;

	rs.header.push_back( "characterID" );
	rs.header.push_back( "online" );

	return rs.Encode();
}
