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


//note, using /slashes from LSC requires ROLE_SLASH


#include "EVEServerPCH.h"


PyCallable_Make_InnerDispatcher(SlashService)



/*
class SlashBound
: public PyBoundObject {
public:
	
	PyCallable_Make_Dispatcher(SlashBound)
	
	SlashBound(PyServiceMgr *mgr, SlashDB *db)
	: PyBoundObject(mgr, "SlashBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
		PyCallable_REG_CALL(SlashBound, )
		PyCallable_REG_CALL(SlashBound, )
	}
	virtual ~SlashBound() { delete m_dispatch; }
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL()
	PyCallable_DECL_CALL()

protected:
	SlashDB *const m_db;
	Dispatcher *const m_dispatch;   //we own this
};
*/


SlashService::SlashService(PyServiceMgr *mgr, CommandDispatcher *cd)
: PyService(mgr, "slash"),
  m_dispatch(new Dispatcher(this)),
  m_commandDispatch(cd)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(SlashService, SlashCmd)
}

SlashService::~SlashService() {
	delete m_dispatch;
}


/*
PyBoundObject *SlashService::_CreateBoundObject(Client *c, PyTuple *bind_args) {
	_log(CLIENT__MESSAGE, "SlashService bind request for:");
	bind_args->Dump(CLIENT__MESSAGE, "    ");

	return(new SlashBound(m_manager, &m_db));
}*/


PyResult SlashService::Handle_SlashCmd( PyCallArgs& call )
{
    if( !( call.client->GetAccountRole() & ROLE_SLASH ) )
    {
        _log( SERVICE__ERROR, "%s: Client '%s' used a slash command but does not have ROLE_SLASH. Modified client?", GetName(), call.client->GetName() );
        throw PyException( MakeCustomError( "You need to have ROLE_SLASH to execute commands." ) );
    }

    Call_SingleWStringSoftArg arg;
    if( !arg.Decode( &call.tuple ) )
    {
        codelog( SERVICE__ERROR, "Failed to decode arguments" );
        return NULL;
    }

    sLog.Debug( "SlashService::Handle_SlashCmd()", "Slash command called: '%s'", arg.arg.c_str() );

    return m_commandDispatch->Execute( call.client, arg.arg.c_str() );
}
