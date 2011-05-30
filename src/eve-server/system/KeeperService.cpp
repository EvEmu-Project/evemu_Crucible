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


PyCallable_Make_InnerDispatcher(KeeperService)




class KeeperBound
: public PyBoundObject {
public:
	
	PyCallable_Make_Dispatcher(KeeperBound)
	
	KeeperBound(PyServiceMgr *mgr, SystemDB *db)
	: PyBoundObject(mgr),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "KeeperBound";
		
//		PyCallable_REG_CALL(KeeperBound, EditDungeon)
//		PyCallable_REG_CALL(KeeperBound, PlayDungeon)
//		PyCallable_REG_CALL(KeeperBound, Reset)
//		PyCallable_REG_CALL(KeeperBound, GotoRoom) //(int room)
	}
	virtual ~KeeperBound() { delete m_dispatch; }
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	//PyCallable_DECL_CALL()
	//PyCallable_DECL_CALL()

protected:
	SystemDB *const m_db;
	Dispatcher *const m_dispatch;   //we own this
};


KeeperService::KeeperService(PyServiceMgr *mgr)
: PyService(mgr, "keeper"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(KeeperService, GetLevelEditor)
}

KeeperService::~KeeperService() {
	delete m_dispatch;
}

PyBoundObject *KeeperService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	_log(CLIENT__MESSAGE, "KeeperService bind request for:");
	bind_args->Dump(CLIENT__MESSAGE, "    ");

	return(new KeeperBound(m_manager, &m_db));
}


PyResult KeeperService::Handle_GetLevelEditor(PyCallArgs &call) {
	PyRep *result = NULL;
	
	KeeperBound *ib = new KeeperBound(m_manager, &m_db);
	result = m_manager->BindObject(call.client, ib);

	return result;
}



























