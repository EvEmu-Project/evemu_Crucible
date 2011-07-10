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

PyServiceMgr::PyServiceMgr( uint32 nodeID, EntityList& elist, ItemFactory& ifactory )
: item_factory( ifactory ),
  entity_list( elist ),
  lsc_service( NULL ),
  cache_service( NULL ),
  m_nextBindID( 100 ),
  m_nodeID( nodeID ),
  m_svcDB()
{
	entity_list.UseServices(this);
}

PyServiceMgr::~PyServiceMgr() {
	{
		std::set<PyService *>::iterator cur, end;
		cur = m_services.begin();
		end = m_services.end();
		for(; cur != end; cur++) {
			delete *cur;
		}
	}

	{
		std::map<uint32, BoundObject>::iterator cur, end;
		cur = m_boundObjects.begin();
		end = m_boundObjects.end();
		for(; cur != end; cur++) {
			delete cur->second.destination;
		}
	}
}

void PyServiceMgr::Process() {
	//well... we used to have something to do, but not right now...
}

void PyServiceMgr::RegisterService(PyService *d) {
	m_services.insert(d);
}

PyService *PyServiceMgr::LookupService(const std::string &name) {
	std::set<PyService *>::iterator cur, end;
	cur = m_services.begin();
	end = m_services.end();
	for(; cur != end; cur++) {
		if(name == (*cur)->GetName())
		{
			//this is added here so you know which server opens the call
			//that if it gets loaded
			sLog.Debug("ServiceOfIterest", (*cur)->GetName());
			return(*cur);
		}
	}
	return NULL;
}

PySubStruct *PyServiceMgr::BindObject(Client *c, PyBoundObject *cb, PyDict **dict) {
	if(cb == NULL)
    {
        sLog.Error("Service Mgr", "Tried to bind a NULL object!");
		return new PySubStruct(new PyNone());
	}

	cb->_SetNodeBindID(GetNodeID(), _GetBindID());	//tell the object what its bind ID is.

	BoundObject obj;
	obj.client = c;
	obj.destination = cb;

	m_boundObjects[cb->bindID()] = obj;

    //sLog.Debug("Service Mgr", "Binding %s to service %s", bind_str, cb->GetName());

	std::string bind_str = cb->GetBindStr();
	//not sure what this really is...
	uint64 expiration = Win32TimeNow() + Win32Time_Hour;

	PyTuple *objt;
	if(dict == NULL || *dict == NULL)
	{
		objt = new PyTuple(2);

		objt->items[0] = new PyString(bind_str);
		objt->items[1] = new PyLong(expiration);	//expiration?
	}
	else
	{
		objt = new PyTuple(3);

		objt->items[0] = new PyString(bind_str);
		objt->items[1] = *dict; *dict = NULL;			//consumed
		objt->items[2] = new PyInt(expiration);	//expiration?
	}

	return new PySubStruct(new PySubStream(objt));
}

void PyServiceMgr::ClearBoundObjects(Client *who) {
	ObjectsBoundMapItr cur, end;
	cur = m_boundObjects.begin();
	end = m_boundObjects.end();

    while(cur != end) {
		if(cur->second.client == who)
        {
            //sLog.Debug("Service Mgr", "Clearing bound object %s", cur->first.c_str());
			cur->second.destination->Release();

			ObjectsBoundMapItr tmp = cur++;
			m_boundObjects.erase(tmp);
		}
        else
        {
			cur++;
		}
	}
}

PyBoundObject *PyServiceMgr::FindBoundObject(uint32 bindID) {
	std::map<uint32, BoundObject>::iterator res;
	res = m_boundObjects.find(bindID);
	if(res == m_boundObjects.end())
		return NULL;
	else
		return res->second.destination;
}

void PyServiceMgr::ClearBoundObject(uint32 bindID)
{
	std::map<uint32, BoundObject>::iterator res;
	res = m_boundObjects.find(bindID);
	if(res == m_boundObjects.end()) {
        sLog.Error("Service Mgr", "Unable to find bound object %u to release.", bindID);
		return;
	}
	
	PyBoundObject *bo = res->second.destination;
	
    //sLog.Debug("Service Mgr", "Clearing bound object %s (released)", res->first.c_str());

	m_boundObjects.erase(res);
	bo->Release();
}
