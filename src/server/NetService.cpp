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

		dict->setStr("trademgr", new PyString("station"));
		dict->setStr("stationSvc", new PyNone());
		dict->setStr("zsystem", new PyNone());
		dict->setStr("invbroker", new PyNone());
		dict->setStr("droneMgr", new PyNone());
		dict->setStr("userSvc", new PyNone());
		dict->setStr("map", new PyNone());
		dict->setStr("beyonce", new PyNone());
		dict->setStr("standing2", new PyNone());
		dict->setStr("ram", new PyNone());
		dict->setStr("DB", new PyNone());
		dict->setStr("posMgr", new PyNone());
		dict->setStr("voucher", new PyNone());
		dict->setStr("entity", new PyNone());
		dict->setStr("damageTracker", new PyNone());
		dict->setStr("agentMgr", new PyNone());
		dict->setStr("dogmaIM", new PyNone());
		dict->setStr("machoNet", new PyNone());
		dict->setStr("dungeonExplorationMgr", new PyNone());
		dict->setStr("watchdog", new PyNone());
		dict->setStr("ship", new PyNone());
		dict->setStr("DB2", new PyNone());
		dict->setStr("market", new PyNone());
		dict->setStr("dungeon", new PyNone());
		dict->setStr("npcSvc", new PyNone());
		dict->setStr("sessionMgr", new PyNone());
		dict->setStr("LSC", new PyString("location"));
		dict->setStr("allianceRegistry", new PyNone());
		dict->setStr("tutorialSvc", new PyString("station"));
		dict->setStr("bookmark", new PyString("station"));
		dict->setStr("cache", new PyNone());
		dict->setStr("character", new PyNone());
		dict->setStr("factory", new PyNone());
		dict->setStr("facWarMgr", new PyNone());
		dict->setStr("corpStationMgr", new PyNone());
		dict->setStr("authentication", new PyNone());
		dict->setStr("station", new PyString("location"));
		dict->setStr("effectCompiler", new PyNone());
		dict->setStr("slash", new PyString("station"));
		dict->setStr("charmgr", new PyNone());
		dict->setStr("BSD", new PyNone());
		dict->setStr("reprocessingSvc", new PyNone());
		dict->setStr("config", new PyString("locationPreferred"));
		dict->setStr("billingMgr", new PyNone());
		dict->setStr("billMgr", new PyNone());
		dict->setStr("lookupSvc", new PyNone());
		dict->setStr("emailreader", new PyNone());
		dict->setStr("lootSvc", new PyNone());
		dict->setStr("http", new PyNone());
		dict->setStr("repairSvc", new PyNone());
		dict->setStr("gagger", new PyNone());
		dict->setStr("dataconfig", new PyNone());
		dict->setStr("lien", new PyNone());
		dict->setStr("i2", new PyNone());
		dict->setStr("wormholeMgr", new PyString("station"));
		dict->setStr("pathfinder", new PyNone());
		dict->setStr("alert", new PyNone());
		dict->setStr("director", new PyNone());
		dict->setStr("dogma", new PyNone());
		dict->setStr("aggressionMgr", new PyNone());
		dict->setStr("corporationSvc", new PyNone());
		dict->setStr("certificateMgr", new PyNone());
		dict->setStr("clones", new PyNone());
		dict->setStr("jumpCloneSvc", new PyNone());
		dict->setStr("insuranceSvc", new PyNone());
		dict->setStr("corpmgr", new PyNone());
		dict->setStr("warRegistry", new PyNone());
		dict->setStr("corpRegistry", new PyNone());
		dict->setStr("account", new PyString("station"));
		dict->setStr("gangSvc", new PyString("station"));
		dict->setStr("objectCaching", new PyNone());
		dict->setStr("counter", new PyNone());
		dict->setStr("petitioner", new PyNone());
		dict->setStr("keeper", new PyString("solarsystem"));
		dict->setStr("LPSvc", new PyNone());
		dict->setStr("clientStatsMgr", new PyNone());
		dict->setStr("jumpbeaconsvc", new PyNone());
		dict->setStr("scanMgr", new PyString("solarsystem"));
		dict->setStr("contractMgr", new PyString("station"));
		dict->setStr("debug", new PyNone());
		dict->setStr("languageSvc", new PyNone());
		dict->setStr("skillMgr", new PyNone());
		dict->setStr("voiceMgr", new PyNone());
		dict->setStr("onlineStatus", new PyNone());
		dict->setStr("gangSvcObjectHandler", new PyNone());

		//register it
		m_manager->cache_service->GiveCache(&str, (PyRep **)&dict);
	}
	serverinfo = m_manager->cache_service->GetCacheHint(str.content());

	PyDict *initvals = new PyDict();
    //send all the cache hints needed for server info.
	m_manager->cache_service->InsertCacheHints(ObjCacheService::hLoginCachables, initvals);

	result->items[0] = serverinfo;
	result->items[1] = initvals;

	return result;
}

PyResult NetService::Handle_GetTime(PyCallArgs &call) {
	return(new PyLong(Win32TimeNow()));
}




















