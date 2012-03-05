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
	Author:		ozatomic
*/

#include "EVEServerPCH.h"


PyCallable_Make_InnerDispatcher(InfoGatheringMgr)

InfoGatheringMgr::InfoGatheringMgr(PyServiceMgr *mgr)
: PyService(mgr, "infoGatheringMgr"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(InfoGatheringMgr, GetStateAndConfig);
	PyCallable_REG_CALL(InfoGatheringMgr, LogInfoEventsFromClient);

}

InfoGatheringMgr::~InfoGatheringMgr() {
	delete m_dispatch;
}

PyResult InfoGatheringMgr::Handle_GetStateAndConfig(PyCallArgs &call) {
	
	PyDict *rsp = new PyDict;
	
	rsp->SetItemString("clientWorkerInterval", new PyInt(600000)); //Default From packetlogs is 600000
	rsp->SetItemString("isEnabled", new PyInt(0)); //0 = Disabled, 1 = Enabled. Set to 0 becuase jsut gettting rid of exception.

	rsp->SetItemString("infoTypeAggregates", new PyNone());
	rsp->SetItemString("infoTypesOncePerRun", new PyNone());
	rsp->SetItemString("infoTypeParameters", new PyNone());

	PyList *infoTypes = new PyList;
	infoTypes->AddItemInt(999); //Adding a value that was not in live so when its checks list it will always return false for now.
	
	rsp->SetItemString("infoTypes", new PyObjectEx_Type1( new PyToken("__builtin__.set"), new_tuple(infoTypes)));
	
	return new PyObject( "util.KeyVal", rsp );
}

PyResult InfoGatheringMgr::Handle_LogInfoEventsFromClient(PyCallArgs &call) {
	return new PyNone;
}
