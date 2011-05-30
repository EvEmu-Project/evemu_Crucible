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
//#include "ContractMgrService.h"



PyCallable_Make_InnerDispatcher(ContractMgrService)

ContractMgrService::ContractMgrService(PyServiceMgr *mgr)
: PyService(mgr, "contractMgr"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(ContractMgrService, NumRequiringAttention);
}

ContractMgrService::~ContractMgrService()
{
	delete m_dispatch;
}

PyResult ContractMgrService::Handle_NumRequiringAttention( PyCallArgs& call )
{
    sLog.Debug( "ContractMgrService", "Called NumRequiringAttention stub." );

	PyDict* args = new PyDict;
	args->SetItemString( "n", new PyInt( 0 ) );
	args->SetItemString( "ncorp", new PyInt( 0 ) );

	return new PyObject(
        new PyString( "util.KeyVal" ), args
    );
}


