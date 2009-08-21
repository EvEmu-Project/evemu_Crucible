/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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



#include "EvemuPCH.h"

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
	PyTuple *result = new PyTuple(2);
	PyString str("machoNet.serviceInfo");

	PyRep *serverinfo;
	if(!m_manager->cache_service->IsCacheLoaded(&str)) {
		//not cached yet, we have to generate it
		PyDict *dict = new PyDict;

		dict->add("trademgr", new PyString("station"));
		dict->add("stationSvc", new PyNone());
		dict->add("zsystem", new PyNone());
		dict->add("invbroker", new PyNone());
		dict->add("droneMgr", new PyNone());
		dict->add("userSvc", new PyNone());
		dict->add("map", new PyNone());
		dict->add("beyonce", new PyNone());
		dict->add("standing2", new PyNone());
		dict->add("ram", new PyNone());
		dict->add("DB", new PyNone());
		dict->add("posMgr", new PyNone());
		dict->add("voucher", new PyNone());
		dict->add("entity", new PyNone());
		dict->add("damageTracker", new PyNone());
		dict->add("agentMgr", new PyNone());
		dict->add("dogmaIM", new PyNone());
		dict->add("machoNet", new PyNone());
		dict->add("dungeonExplorationMgr", new PyNone());
		dict->add("watchdog", new PyNone());
		dict->add("ship", new PyNone());
		dict->add("DB2", new PyNone());
		dict->add("market", new PyNone());
		dict->add("dungeon", new PyNone());
		dict->add("npcSvc", new PyNone());
		dict->add("sessionMgr", new PyNone());
		dict->add("LSC", new PyString("location"));
		dict->add("allianceRegistry", new PyNone());
		dict->add("tutorialSvc", new PyString("station"));
		dict->add("bookmark", new PyString("station"));
		dict->add("cache", new PyNone());
		dict->add("character", new PyNone());
		dict->add("factory", new PyNone());
		dict->add("facWarMgr", new PyNone());
		dict->add("corpStationMgr", new PyNone());
		dict->add("authentication", new PyNone());
		dict->add("station", new PyString("location"));
		dict->add("effectCompiler", new PyNone());
		dict->add("slash", new PyString("station"));
		dict->add("charmgr", new PyNone());
		dict->add("BSD", new PyNone());
		dict->add("reprocessingSvc", new PyNone());
		dict->add("config", new PyString("locationPreferred"));
		dict->add("billingMgr", new PyNone());
		dict->add("billMgr", new PyNone());
		dict->add("lookupSvc", new PyNone());
		dict->add("emailreader", new PyNone());
		dict->add("lootSvc", new PyNone());
		dict->add("http", new PyNone());
		dict->add("repairSvc", new PyNone());
		dict->add("gagger", new PyNone());
		dict->add("dataconfig", new PyNone());
		dict->add("lien", new PyNone());
		dict->add("i2", new PyNone());
		dict->add("wormholeMgr", new PyString("station"));
		dict->add("pathfinder", new PyNone());
		dict->add("alert", new PyNone());
		dict->add("director", new PyNone());
		dict->add("dogma", new PyNone());
		dict->add("aggressionMgr", new PyNone());
		dict->add("corporationSvc", new PyNone());
		dict->add("certificateMgr", new PyNone());
		dict->add("clones", new PyNone());
		dict->add("jumpCloneSvc", new PyNone());
		dict->add("insuranceSvc", new PyNone());
		dict->add("corpmgr", new PyNone());
		dict->add("warRegistry", new PyNone());
		dict->add("corpRegistry", new PyNone());
		dict->add("account", new PyString("station"));
		dict->add("gangSvc", new PyString("station"));
		dict->add("objectCaching", new PyNone());
		dict->add("counter", new PyNone());
		dict->add("petitioner", new PyNone());
		dict->add("keeper", new PyString("solarsystem"));
		dict->add("LPSvc", new PyNone());
		dict->add("clientStatsMgr", new PyNone());
		dict->add("jumpbeaconsvc", new PyNone());
		dict->add("scanMgr", new PyString("solarsystem"));
		dict->add("contractMgr", new PyString("station"));
		dict->add("debug", new PyNone());
		dict->add("languageSvc", new PyNone());
		dict->add("skillMgr", new PyNone());
		dict->add("voiceMgr", new PyNone());
		dict->add("onlineStatus", new PyNone());
		dict->add("gangSvcObjectHandler", new PyNone());

		//register it
		m_manager->cache_service->GiveCache(&str, (PyRep **)&dict);
	}
	serverinfo = m_manager->cache_service->GetCacheHint(str.value.c_str());

	PyDict *initvals = new PyDict();
    //send all the cache hints needed for server info.
	m_manager->cache_service->InsertCacheHints(ObjCacheService::hLoginCachables, initvals);

	result->items[0] = serverinfo;
	result->items[1] = initvals;

	return result;
}

PyResult NetService::Handle_GetTime(PyCallArgs &call) {
	return(new PyInt(Win32TimeNow()));
}




















