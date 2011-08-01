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

PyCallable_Make_InnerDispatcher(CharMgrService)

CharMgrService::CharMgrService(PyServiceMgr *mgr)
: PyService(mgr, "charMgr"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(CharMgrService, GetPublicInfo)
	PyCallable_REG_CALL(CharMgrService, GetPublicInfo3)
	PyCallable_REG_CALL(CharMgrService, GetTopBounties)
	PyCallable_REG_CALL(CharMgrService, GetOwnerNoteLabels)
	PyCallable_REG_CALL(CharMgrService, GetContactList)
	PyCallable_REG_CALL(CharMgrService, GetCloneTypeID)
	PyCallable_REG_CALL(CharMgrService, GetHomeStation)
}

CharMgrService::~CharMgrService() {
	delete m_dispatch;
}

PyResult CharMgrService::Handle_GetContactList(PyCallArgs &call)
{
	// another dummy
	DBRowDescriptor *header = new DBRowDescriptor();
	header->AddColumn("contactID", DBTYPE_I4);
	header->AddColumn("inWatchList", DBTYPE_BOOL);
	header->AddColumn("relationshipID", DBTYPE_R8);
	header->AddColumn("labelMask", DBTYPE_I8);
	CRowSet *rowset = new CRowSet( &header );

	PyDict* dict = new PyDict();
	dict->SetItemString("addresses", rowset);
	dict->SetItemString("blocked", rowset);
	PyObject *keyVal = new PyObject( "util.KeyVal", dict);

	return keyVal;
}

PyResult CharMgrService::Handle_GetOwnerNoteLabels(PyCallArgs &call)
{
	// just a dummy for now
	DBRowDescriptor *header = new DBRowDescriptor();
	header->AddColumn("noteID", DBTYPE_I4);
	header->AddColumn("label", DBTYPE_WSTR);
	CRowSet *rowset = new CRowSet( &header );

	return rowset;
}

PyResult CharMgrService::Handle_GetPublicInfo(PyCallArgs &call) {
	//takes a single int arg: char id
	// or corp id
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
		return NULL;
	}

	/*if(IsAgent(args.arg)) {
		//handle agents special right now...
		PyRep *result = m_db.GetAgentPublicInfo(args.arg);
		if(result == NULL) {
			codelog(CLIENT__ERROR, "%s: Failed to find agent %u", call.client->GetName(), args.arg);
			return NULL;
		}
		return result;
	}*/

	PyRep *result = m_db.GetCharPublicInfo(args.arg);
	if(result == NULL) {
		codelog(CLIENT__ERROR, "%s: Failed to find char %u", call.client->GetName(), args.arg);
		return NULL;
	}
	
	return result;
}

PyResult CharMgrService::Handle_GetPublicInfo3(PyCallArgs &call) {
	//takes a single int arg: char id
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
		return NULL;
	}
	
	PyRep *result = m_db.GetCharPublicInfo3(args.arg);
	if(result == NULL) {
		codelog(CLIENT__ERROR, "%s: Failed to find char %u", call.client->GetName(), args.arg);
		return NULL;
	}
	
	return result;
}

PyResult CharMgrService::Handle_GetTopBounties( PyCallArgs& call )
{
    sLog.Debug( "CharMgrService", "Called GetTopBounties stub." );

	util_Rowset rs;
    rs.lines = new PyList;

	rs.header.push_back( "characterID" );
	rs.header.push_back( "ownerName" );
	rs.header.push_back( "bounty" );
	rs.header.push_back( "online" );

	return rs.Encode();
}

PyResult CharMgrService::Handle_GetCloneTypeID( PyCallArgs& call )
{
	sLog.Debug( "CharMgrService", "Called GetCloneTypeID stub." );

	return NULL;
}

PyResult CharMgrService::Handle_GetHomeStation( PyCallArgs& call )
{
	sLog.Debug( "CharMgrService", "Called GetHomeStation stub." );

	return NULL;
}
























