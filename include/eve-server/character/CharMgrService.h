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


#ifndef __CHARMGR_SERVICE_H_INCL__
#define __CHARMGR_SERVICE_H_INCL__

#include "character/CharacterDB.h"
#include "PyService.h"

class CharMgrService : public PyService {
public:
	CharMgrService(PyServiceMgr *mgr);
	virtual ~CharMgrService();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	CharacterDB m_db;	//using this for now until we decide if we need to split them. Might be bad since we actually have two instances of it, but so far it has no member data.
	
	PyCallable_DECL_CALL(GetPublicInfo)
	PyCallable_DECL_CALL(GetPublicInfo3)
	PyCallable_DECL_CALL(GetTopBounties)
	PyCallable_DECL_CALL(GetOwnerNoteLabels)
	PyCallable_DECL_CALL(GetContactList)
	PyCallable_DECL_CALL(GetCloneTypeID)
	PyCallable_DECL_CALL(GetHomeStation)
	PyCallable_DECL_CALL(GetFactions)
};





#endif


