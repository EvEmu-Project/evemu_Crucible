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


PyCallable_Make_InnerDispatcher(FactionWarMgrService)

FactionWarMgrService::FactionWarMgrService(PyServiceMgr *mgr)
: PyService(mgr, "facWarMgr"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(FactionWarMgrService, GetWarFactions)
	PyCallable_REG_CALL(FactionWarMgrService, GetFacWarSystems)
	PyCallable_REG_CALL(FactionWarMgrService, GetMyCharacterRankOverview)
	PyCallable_REG_CALL(FactionWarMgrService, GetFactionMilitiaCorporation)
}

FactionWarMgrService::~FactionWarMgrService()
{
	delete m_dispatch;
}

PyResult FactionWarMgrService::Handle_GetWarFactions(PyCallArgs &call) {
	ObjectCachedMethodID method_id(GetName(), "GetWarFactions");

	if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
		PyRep *res = m_db.GetWarFactions();
		if(res == NULL)
			return NULL;
		m_manager->cache_service->GiveCache(method_id, &res);
	}

	return(m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id));
}

PyResult FactionWarMgrService::Handle_GetFacWarSystems( PyCallArgs& call )
{
	ObjectCachedMethodID method_id( GetName(), "GetFacWarSystems" );

	if( !m_manager->cache_service->IsCacheLoaded( method_id ) )
    {
		PyRep* res = m_db.GetWarFactions();
		if( res == NULL )
			return NULL;

		m_manager->cache_service->GiveCache( method_id, &res );
	}

	return m_manager->cache_service->MakeObjectCachedMethodCallResult( method_id );
}

PyResult FactionWarMgrService::Handle_GetMyCharacterRankOverview( PyCallArgs& call )
{
	Call_SingleIntegerArg arg;
	if( !arg.Decode( &call.tuple ) )
    {
		_log( SERVICE__ERROR, "Failed to decode args." );
		return NULL;
	}

    sLog.Debug( "FactionWarMgrService", "Called GetMyCharacterRankOverview stub." );

	util_Rowset rs;

	rs.header.push_back( "currentRank" );
	rs.header.push_back( "highestRank" );
	rs.header.push_back( "factionID" );
	rs.header.push_back( "lastModified" );

	return rs.Encode();
}

PyResult FactionWarMgrService::Handle_GetFactionMilitiaCorporation(PyCallArgs &call) {
	Call_SingleIntegerArg arg;
	if(!arg.Decode(&call.tuple)) {
		_log(SERVICE__ERROR, "Failed to decode args.");
		return NULL;
	}
	return(new PyInt(m_db.GetFactionMilitiaCorporation(arg.arg)));
}


