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

PyCallable_Make_InnerDispatcher(WarRegistryService)

class WarRegistryBound : public PyBoundObject {
public:
	PyCallable_Make_Dispatcher(WarRegistryBound)

	WarRegistryBound(PyServiceMgr *mgr, uint32 corporationID)
	: PyBoundObject(mgr),
	  m_dispatch(new Dispatcher(this)),
	  m_corporationID(corporationID)
	{
		_SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "WarRegistryBound";

		PyCallable_REG_CALL(WarRegistryBound, GetWars);
	}
	~WarRegistryBound()
	{
		delete m_dispatch;
	}

	void Release() {
		//I hate this statement
		delete this;
	}

	PyCallable_DECL_CALL(GetWars)

private:
	Dispatcher *const m_dispatch;

	uint32 m_corporationID;
};

WarRegistryService::WarRegistryService(PyServiceMgr *mgr)
: PyService(mgr, "warRegistry"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);
}

WarRegistryService::~WarRegistryService()
{
	delete m_dispatch;
}

PyBoundObject *WarRegistryService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	Call_TwoIntegerArgs args;
	//crap
	PyRep *t = bind_args->Clone();
	if(!args.Decode(&t)) {
		_log(SERVICE__ERROR, "%s: Failed to decode bind object params.", GetName());
		return NULL;
	}
	//arg1 = corporationID
	//arg2 = ???
	return(new WarRegistryBound(m_manager, args.arg1));
}

PyResult WarRegistryBound::Handle_GetWars( PyCallArgs& call )
{
    sLog.Debug( "WarRegistryBound", "Called GetWars stub." );

	util_IndexRowset irowset;

	irowset.header.push_back( "warID" );
	irowset.header.push_back( "declaredByID" );
	irowset.header.push_back( "againstID" );
	irowset.header.push_back( "timeDeclared" );
	irowset.header.push_back( "timeFinished" );
	irowset.header.push_back( "retracted" );
	irowset.header.push_back( "retractedBy" );
	irowset.header.push_back( "billID" );
	irowset.header.push_back( "mutual" );

	irowset.idName = "warID";

	return irowset.Encode();
}
