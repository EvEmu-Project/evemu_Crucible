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

PyCallable_Make_InnerDispatcher(NetService)

NetService::NetService(PyServiceMgr *mgr)
: PyService(mgr, "machoNet"),
m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(NetService, GetInitVals)
	PyCallable_REG_CALL(NetService, GetTime)
}

NetService::~NetService() {
	delete m_dispatch;
}

PyResult NetService::Handle_GetInitVals(PyCallArgs &call) {
    PyString* str = new PyString( "machoNet.serviceInfo" );

	if(!m_manager->cache_service->IsCacheLoaded(str))
    {
		PyDict *dict = new PyDict;

        /* ServiceCallGPCS.py:197
        where = self.machoNet.serviceInfo[service]
        if where:
            for (k, v,) in self.machoNet.serviceInfo.iteritems():
                if ((k != service) and (v and (v.startswith(where) or where.startswith(v)))):
                    nodeID = self.services.get(k, None)
                    break
        */
		dict->SetItemString("trademgr", new PyString("station"));
		dict->SetItemString("tutorialSvc", new PyString("station"));
		dict->SetItemString("bookmark", new PyString("station"));
		dict->SetItemString("slash", new PyString("station"));
		dict->SetItemString("wormholeMgr", new PyString("station"));
		dict->SetItemString("account", new PyString("station"));
		dict->SetItemString("gangSvc", new PyString("station"));
		dict->SetItemString("contractMgr", new PyString("station"));

		dict->SetItemString("LSC", new PyString("location"));
		dict->SetItemString("station", new PyString("location"));
		dict->SetItemString("config", new PyString("locationPreferred"));

		dict->SetItemString("scanMgr", new PyString("solarsystem"));
		dict->SetItemString("keeper", new PyString("solarsystem"));

        dict->SetItemString("stationSvc", new PyNone());
		dict->SetItemString("zsystem", new PyNone());
		dict->SetItemString("invbroker", new PyNone());
		dict->SetItemString("droneMgr", new PyNone());
		dict->SetItemString("userSvc", new PyNone());
		dict->SetItemString("map", new PyNone());
		dict->SetItemString("beyonce", new PyNone());
		dict->SetItemString("standing2", new PyNone());
		dict->SetItemString("ram", new PyNone());
		dict->SetItemString("DB", new PyNone());
		dict->SetItemString("posMgr", new PyNone());
		dict->SetItemString("voucher", new PyNone());
		dict->SetItemString("entity", new PyNone());
		dict->SetItemString("damageTracker", new PyNone());
		dict->SetItemString("agentMgr", new PyNone());
		dict->SetItemString("dogmaIM", new PyNone());
		dict->SetItemString("machoNet", new PyNone());
		dict->SetItemString("dungeonExplorationMgr", new PyNone());
		dict->SetItemString("watchdog", new PyNone());
		dict->SetItemString("ship", new PyNone());
		dict->SetItemString("DB2", new PyNone());
		dict->SetItemString("market", new PyNone());
		dict->SetItemString("dungeon", new PyNone());
		dict->SetItemString("npcSvc", new PyNone());
		dict->SetItemString("sessionMgr", new PyNone());
		dict->SetItemString("allianceRegistry", new PyNone());
		dict->SetItemString("cache", new PyNone());
		dict->SetItemString("character", new PyNone());
		dict->SetItemString("factory", new PyNone());
		dict->SetItemString("facWarMgr", new PyNone());
		dict->SetItemString("corpStationMgr", new PyNone());
		dict->SetItemString("authentication", new PyNone());
		dict->SetItemString("effectCompiler", new PyNone());
		dict->SetItemString("charmgr", new PyNone());
		dict->SetItemString("BSD", new PyNone());
		dict->SetItemString("reprocessingSvc", new PyNone());
		dict->SetItemString("billingMgr", new PyNone());
		dict->SetItemString("billMgr", new PyNone());
		dict->SetItemString("lookupSvc", new PyNone());
		dict->SetItemString("emailreader", new PyNone());
		dict->SetItemString("lootSvc", new PyNone());
		dict->SetItemString("http", new PyNone());
		dict->SetItemString("repairSvc", new PyNone());
		dict->SetItemString("gagger", new PyNone());
		dict->SetItemString("dataconfig", new PyNone());
		dict->SetItemString("lien", new PyNone());
		dict->SetItemString("i2", new PyNone());
		dict->SetItemString("pathfinder", new PyNone());
		dict->SetItemString("alert", new PyNone());
		dict->SetItemString("director", new PyNone());
		dict->SetItemString("dogma", new PyNone());
		dict->SetItemString("aggressionMgr", new PyNone());
		dict->SetItemString("corporationSvc", new PyNone());
		dict->SetItemString("certificateMgr", new PyNone());
		dict->SetItemString("clones", new PyNone());
		dict->SetItemString("jumpCloneSvc", new PyNone());
		dict->SetItemString("insuranceSvc", new PyNone());
		dict->SetItemString("corpmgr", new PyNone());
		dict->SetItemString("warRegistry", new PyNone());
		dict->SetItemString("corpRegistry", new PyNone());
		dict->SetItemString("objectCaching", new PyNone());
		dict->SetItemString("counter", new PyNone());
		dict->SetItemString("petitioner", new PyNone());
		dict->SetItemString("LPSvc", new PyNone());
		dict->SetItemString("clientStatsMgr", new PyNone());
		dict->SetItemString("jumpbeaconsvc", new PyNone());
		dict->SetItemString("debug", new PyNone());
		dict->SetItemString("languageSvc", new PyNone());
		dict->SetItemString("skillMgr", new PyNone());
		dict->SetItemString("voiceMgr", new PyNone());
		dict->SetItemString("onlineStatus", new PyNone());
		dict->SetItemString("gangSvcObjectHandler", new PyNone());
		dict->SetItemString("sovMgr", new PyNone());

		//register it
		m_manager->cache_service->GiveCache(str, (PyRep **)&dict);
	}
	PyRep* serverinfo = m_manager->cache_service->GetCacheHint(str);
    PyDecRef( str );

	PyDict* initvals = new PyDict();
    //send all the cache hints needed for server info.
	m_manager->cache_service->InsertCacheHints(ObjCacheService::hLoginCachables, initvals);

	PyTuple* result = new PyTuple( 2 );
	result->SetItem( 0, serverinfo );
	result->SetItem( 1, initvals );
	return result;
}

PyResult NetService::Handle_GetTime(PyCallArgs &call) {
	return(new PyLong(Win32TimeNow()));
}
