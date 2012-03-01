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
	Author:		Reve 
*/

//stub

#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(InfoGatheringService)

	InfoGatheringService::InfoGatheringService(PyServiceMgr *mgr)
	: PyService(mgr, "infoGatheringMgr"),
	  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(InfoGatheringService, GetStateAndConfig);
}

InfoGatheringService::~InfoGatheringService() {
	delete m_dispatch;
}

PyResult InfoGatheringService::Handle_GetStateAndConfig(PyCallArgs &call) {

	sLog.Debug("Server", "Called GetStateAndConfig");

	PyDict* args = new PyDict;
	
	args->SetItemString( "clientWorkerInterval", new PyInt( 0 ) );
	args->SetItemString( "isEnabled", new PyInt( 0 ) );
	args->SetItemString( "infoTypeAggregates", new PyDict );
	args->SetItemString( "infoTypesOncePerRun", new PyDict );
	args->SetItemString( "infoTypeParameters", new PyDict );
	args->SetItemString( "infoTypes", new PyDict );

	return new PyObject( "util.KeyVal", args );
}