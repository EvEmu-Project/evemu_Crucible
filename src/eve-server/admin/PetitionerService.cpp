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

PyCallable_Make_InnerDispatcher(PetitionerService)

/*
class PetitionerBound
: public PyBoundObject {
public:
	
	PyCallable_Make_Dispatcher(PetitionerBound)
	
	PetitionerBound(PyServiceMgr *mgr, PetitionerDB *db)
	: PyBoundObject(mgr, "PetitionerBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(PetitionerBound, )
		PyCallable_REG_CALL(PetitionerBound, )
	}
	virtual ~PetitionerBound() { delete m_dispatch; }
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL()
	PyCallable_DECL_CALL()

protected:
	PetitionerDB *const m_db;
	Dispatcher *const m_dispatch;   //we own this
};
*/


PetitionerService::PetitionerService(PyServiceMgr *mgr)
: PyService(mgr, "petitioner"),
  m_dispatch(new Dispatcher(this))
//m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(PetitionerService, GetCategories)
	PyCallable_REG_CALL(PetitionerService, GetUnreadMessages)
}

PetitionerService::~PetitionerService() {
	delete m_dispatch;
}


/*
PyBoundObject* PetitionerService::_CreateBoundObject( Client* c, const PyRep* bind_args )
{
	_log( CLIENT__MESSAGE, "PetitionerService bind request for:" );
	bind_args->Dump( CLIENT__MESSAGE, "    " );

	return new PetitionerBound( m_manager, &m_db );
}*/


PyResult PetitionerService::Handle_GetCategories( PyCallArgs& call )
{
	//takes no arguments
	
    sLog.Debug( "PetitionerService", "Called GetCategories stub." );

	PyList* result = new PyList;
	result->AddItemString( "Test Cat" );
	result->AddItemString( "Test Cat2" );

	return result;
}


PyResult PetitionerService::Handle_GetUnreadMessages( PyCallArgs& call )
{
	//takes no arguments

    sLog.Debug( "PetitionerService", "Called GetUnreadMessages stub." );

	//unknown...
	return new PyList;
}
