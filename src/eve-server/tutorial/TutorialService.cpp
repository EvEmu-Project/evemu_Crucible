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


PyCallable_Make_InnerDispatcher(TutorialService)

TutorialService::TutorialService(PyServiceMgr *mgr)
: PyService(mgr, "tutorialSvc"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(TutorialService, GetTutorialInfo)
	PyCallable_REG_CALL(TutorialService, GetTutorials)
	PyCallable_REG_CALL(TutorialService, GetCriterias)
	PyCallable_REG_CALL(TutorialService, GetCategories)
	PyCallable_REG_CALL(TutorialService, GetContextHelp)
	PyCallable_REG_CALL(TutorialService, GetCharacterTutorialState)
}

TutorialService::~TutorialService() {
	delete m_dispatch;
}

PyResult TutorialService::Handle_GetTutorialInfo(PyCallArgs &call) {
	Call_GetTutorialInfo args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Can't parse args.");
		return NULL;
	}

	Rsp_GetTutorialInfo rsp;

	rsp.pagecriterias = m_db.GetPageCriterias(args.tutorialID);
	if(rsp.pagecriterias == NULL) {
		codelog(SERVICE__ERROR, "An error occured while getting pagecriterias for tutorial %u.", args.tutorialID);
		return NULL;
	}

	rsp.pages = m_db.GetPages(args.tutorialID);
	if(rsp.pages == NULL) {
		codelog(SERVICE__ERROR, "An error occured while getting pages for tutorial %u.", args.tutorialID);
		return NULL;
	}

	rsp.tutorial = m_db.GetTutorial(args.tutorialID);
	if(rsp.tutorial == NULL) {
		codelog(SERVICE__ERROR, "An error occured while getting tutorial %u.", args.tutorialID);
		return NULL;
	}

	rsp.criterias = m_db.GetTutorialCriterias(args.tutorialID);
	if(rsp.criterias == NULL) {
		codelog(SERVICE__ERROR, "An error occured while getting criterias for tutorial %u.", args.tutorialID);
		return NULL;
	}

	return(rsp.Encode());
}

PyResult TutorialService::Handle_GetTutorials(PyCallArgs &call) {
	return(m_db.GetAllTutorials());
}

PyResult TutorialService::Handle_GetCriterias(PyCallArgs &call) {
	return(m_db.GetAllCriterias());
}

PyResult TutorialService::Handle_GetCategories(PyCallArgs &call) {
	return(m_db.GetCategories());
}

PyResult TutorialService::Handle_GetContextHelp( PyCallArgs& call )
{
    sLog.Debug( "TutorialService", "Called GetContextHelp stub." );

    return new PyList;
}

PyResult TutorialService::Handle_GetCharacterTutorialState( PyCallArgs& call )
{
    sLog.Debug( "TutorialService", "Called GetCharacterTutorialState stub." );

	util_Rowset rs;
    rs.lines = new PyList;

	rs.header.push_back( "characterID" );
	rs.header.push_back( "tutorialID" );
	rs.header.push_back( "pageID" );
	rs.header.push_back( "eventTypeID" );

	return rs.Encode();
}
