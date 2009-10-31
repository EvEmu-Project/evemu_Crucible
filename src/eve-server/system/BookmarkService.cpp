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

#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(BookmarkService)

BookmarkService::BookmarkService(PyServiceMgr *mgr)
: PyService(mgr, "bookmark"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(BookmarkService, GetBookmarks)
	PyCallable_REG_CALL(BookmarkService, BookmarkLocation)
}

BookmarkService::~BookmarkService() {
	delete m_dispatch;
}


PyResult BookmarkService::Handle_GetBookmarks(PyCallArgs &call) {
	return(m_db.GetBookmarks(call.client->GetCharacterID()));
}


PyResult BookmarkService::Handle_BookmarkLocation(PyCallArgs &call) {
	//takes (locationID, None, description)

	//returns:
	//(bookmarkID, itemID, typeID, x, y, z, locationID,) 
	
	return(new PyNone);
}





















