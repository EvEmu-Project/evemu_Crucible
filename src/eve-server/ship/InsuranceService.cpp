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


PyCallable_Make_InnerDispatcher(InsuranceService)


/*
class InsuranceBound
: public PyBoundObject {
public:
	
	class Dispatcher
	: public PyCallableDispatcher<InsuranceBound> {
	public:
		Dispatcher(InsuranceBound *c)
		: PyCallableDispatcher<InsuranceBound>(c) {}
	};
	
	InsuranceBound(PyServiceMgr *mgr, InsuranceDB *db)
	: PyBoundObject(mgr, "InsuranceBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(InsuranceBound, )
		PyCallable_REG_CALL(InsuranceBound, )
	}
	virtual ~InsuranceBound() {}
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL()
	PyCallable_DECL_CALL()

protected:
	InsuranceDB *const m_db;
	Dispatcher *const m_dispatch;
};
*/


InsuranceService::InsuranceService(PyServiceMgr *mgr)
: PyService(mgr, "insuranceSvc"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(InsuranceService, GetContractForShip)
	//PyCallable_REG_CALL(InsuranceService, )
}

InsuranceService::~InsuranceService() {
	delete m_dispatch;
}


/*
PyBoundObject* InsuranceService::_CreateBoundObject( Client* c, const PyRep* bind_args )
{
	_log( CLIENT__MESSAGE, "InsuranceService bind request for:" );
	bind_args->Dump( CLIENT__MESSAGE, "    " );

	return new InsuranceBound( m_manager, &m_db );
}*/


PyResult InsuranceService::Handle_GetContractForShip( PyCallArgs& call )
{
    sLog.Debug( "InsuranceService", "Called GetContractForShip stub." );

    return new PyNone;
}


























