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


#ifndef __BOOKMARK_SERVICE_H_INCL__
#define __BOOKMARK_SERVICE_H_INCL__

#include "PyService.h"

class BookmarkService : public PyService
{
public:
    static const uint32 MAX_BOOKMARK_ID;

	BookmarkService(PyServiceMgr *mgr);
	virtual ~BookmarkService();

    bool LookupBookmark(uint32 characterID, uint32 bookmarkID, uint32 &itemID, uint32 &typeID, double &x, double &y, double &z);

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;
    uint32 nextBookmarkID;

	BookmarkDB m_db;

	PyCallable_DECL_CALL(GetBookmarks)
	PyCallable_DECL_CALL(BookmarkLocation)
    PyCallable_DECL_CALL(DeleteBookmarks)
    PyCallable_DECL_CALL(UpdateBookmark)
};





#endif


