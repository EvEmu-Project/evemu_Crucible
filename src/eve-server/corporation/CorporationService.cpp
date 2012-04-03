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

PyCallable_Make_InnerDispatcher(CorporationService)

CorporationService::CorporationService(PyServiceMgr *mgr)
: PyService(mgr, "corporationSvc"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(CorporationService, GetFactionInfo)
	PyCallable_REG_CALL(CorporationService, GetCorpInfo)
	PyCallable_REG_CALL(CorporationService, GetNPCDivisions)
	PyCallable_REG_CALL(CorporationService, GetEmploymentRecord)
	PyCallable_REG_CALL(CorporationService, GetMedalsReceived)
	PyCallable_REG_CALL(CorporationService, GetAllCorpMedals)
	PyCallable_REG_CALL(CorporationService, GetRecruitmentAdTypes)
	PyCallable_REG_CALL(CorporationService, GetRecruitmentAdsByCriteria)
}

CorporationService::~CorporationService() {
	delete m_dispatch;
}

PyResult CorporationService::Handle_GetFactionInfo(PyCallArgs &call) {
	
	GetFactionInfoRsp rsp;
	
	if(!m_db.ListAllCorpFactions(rsp.factionIDbyNPCCorpID)) {
		codelog(SERVICE__ERROR, "Failed to service request");
		return NULL;
	}
	if(!m_db.ListAllFactionStationCounts(rsp.factionStationCount)) {
		codelog(SERVICE__ERROR, "Failed to service request");
		return NULL;
	}
	if(!m_db.ListAllFactionSystemCounts(rsp.factionSolarSystemCount)) {
		codelog(SERVICE__ERROR, "Failed to service request");
		return NULL;
	}
	if(!m_db.ListAllFactionRegions(rsp.factionRegions)) {
		codelog(SERVICE__ERROR, "Failed to service request");
		return NULL;
	}
	if(!m_db.ListAllFactionConstellations(rsp.factionConstellations)) {
		codelog(SERVICE__ERROR, "Failed to service request");
		return NULL;
	}
	if(!m_db.ListAllFactionSolarSystems(rsp.factionSolarSystems)) {
		codelog(SERVICE__ERROR, "Failed to service request");
		return NULL;
	}
	if(!m_db.ListAllFactionRaces(rsp.factionRaces)) {
		codelog(SERVICE__ERROR, "Failed to service request");
		return NULL;
	}
	
	rsp.npcCorpInfo = m_db.ListAllCorpInfo();
	if(rsp.npcCorpInfo == NULL) {
		codelog(SERVICE__ERROR, "Failed to service request");
		return NULL;
	}
	
	return(rsp.Encode());
	

    /*
	
	std::string abs_fname = "../data/cache/fgAAAAAsEA5jb3Jwb3JhdGlvblN2YxAOR2V0RmFjdGlvbkluZm8.cache";
	
	PySubStream *ss = new PySubStream();

	if(!call.client->services().GetCache()->LoadCachedFile(abs_fname.c_str(), "GetFactionInfo", ss)) {
		_log(CLIENT__ERROR, "GetFactionInfo Failed to load cache file '%s'", abs_fname.c_str());
		ss->decoded = new PyNone();
		return(ss);
	}

	//total hack:
	ss->length -= 79;
	uint8 *data = ss->data;
	ss->data = new uint8[ss->length];
	memcpy(ss->data, data + 79, ss->length);
	delete[] data;
	delete ss->decoded;
	ss->decoded = NULL;
	
	_log(CLIENT__MESSAGE, "Sending cache reply for GetFactionInfo");

	return(ss);
*/
}

PyResult CorporationService::Handle_GetCorpInfo(PyCallArgs &call) { 

	sLog.Debug("Server", "Called GetCorpInfo Stub.");

	return NULL;
}


PyResult CorporationService::Handle_GetNPCDivisions(PyCallArgs &call) {
	PyRep *result = m_db.ListNPCDivisions();
	
	return result;
}

PyResult CorporationService::Handle_GetEmploymentRecord(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if (!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Bad arguments");
		return (NULL);
	}
	
	PyRep * answer = m_db.GetEmploymentRecord(args.arg);
	
	return (answer);
}

PyResult CorporationService::Handle_GetMedalsReceived(PyCallArgs &call) {
	Call_SingleIntegerArg arg;

	if(!arg.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Bad arguments");
		return (NULL);
	}

	PyTuple *t = new PyTuple(2);
	t->items[0] = m_db.GetMedalsReceived(arg.arg);
	t->items[1] = new PyList;

	return t;
}

PyResult CorporationService::Handle_GetAllCorpMedals( PyCallArgs& call )
{
	//arg is corporationID
	Call_SingleIntegerArg arg;
	if( !arg.Decode( &call.tuple ) )
    {
		_log( SERVICE__ERROR, "Failed to decode args." );
		return NULL;
	}

    sLog.Debug( "CorporationService", "Called GetAllCorpMedals stub." );

	PyList* res = new PyList;

	util_Rowset rs;

	rs.header.push_back( "medalID" );
	rs.header.push_back( "ownerID" );
	rs.header.push_back( "title" );
	rs.header.push_back( "description" );
	rs.header.push_back( "creatorID" );
	rs.header.push_back( "date" );
	rs.header.push_back( "noRecepients" );
	res->AddItem( rs.Encode() );

	rs.header.clear();

	rs.header.push_back( "medalID" );
	rs.header.push_back( "part" );
	rs.header.push_back( "layer" );
	rs.header.push_back( "graphic" );
	rs.header.push_back( "color" );
	res->AddItem( rs.Encode() );

	return res;
}

PyResult CorporationService::Handle_GetRecruitmentAdTypes( PyCallArgs& call )
{
	//no args

    sLog.Debug( "CorporationService", "Called GetRecruitmentAdTypes stub." );

	util_Rowset rs;

	rs.header.push_back( "groupID" );
	rs.header.push_back( "groupName" );
	rs.header.push_back( "typeID" );
	rs.header.push_back( "typeMask" );
	rs.header.push_back( "typeName" );
	rs.header.push_back( "description" );
	rs.header.push_back( "dataID" );
	rs.header.push_back( "groupDataID" );

	return rs.Encode();
}

PyResult CorporationService::Handle_GetRecruitmentAdsByCriteria( PyCallArgs& call )
{
	//this is cached on live with check "5 minutes"

	Call_GetRecruitmentAdsByCriteria args;
	if( !args.Decode( &call.tuple ) )
    {
		_log( SERVICE__ERROR, "Failed to decode args." );
		return NULL;
	}

    sLog.Debug( "CorporationService", "Called GetRecruitmentAdsByCriteria stub." );

	util_Rowset rs;

	rs.header.push_back( "adID" );
	rs.header.push_back( "corporationID" );
	rs.header.push_back( "allianceID" );
	rs.header.push_back( "expiryDateTime" );
	rs.header.push_back( "stationID" );
	rs.header.push_back( "regionID" );
	rs.header.push_back( "raceMask" );
	rs.header.push_back( "typeMask" );
	rs.header.push_back( "description" );
	rs.header.push_back( "createDateTime" );
	rs.header.push_back( "skillPoints" );
	rs.header.push_back( "channelID" );

	return rs.Encode();
}
