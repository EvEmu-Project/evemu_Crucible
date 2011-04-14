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
	Author:		Bloody.Rabbit
*/

#ifndef __BOOKMARK_DB__H__INCL__
#define __BOOKMARK_DB__H__INCL__

#include "ServiceDB.h"

class BookmarkDB
: public ServiceDB
{
public:
	PyObject *GetBookmarks(uint32 ownerID);

   	uint32 GetNextAvailableBookmarkID();

    uint32 FindBookmarkTypeID(uint32 itemID);

    bool GetBookmarkInformation(uint32 bookmarkID, uint32 &ownerID, uint32 &itemID, uint32 &typeID,
                                uint32 &flag, std::string &memo, uint64 &created, double &x, double &y,
                                double &z, uint32 &locationID);

    bool SaveNewBookmarkToDatabase(uint32 &bookmarkID, uint32 ownerID, uint32 itemID,
                                   uint32 typeID, uint32 flag, std::string memo, uint64 created,
                                   double x, double y, double z, uint32 locationID);

    bool DeleteBookmarksFromDatabase(uint32 ownerID, std::vector<unsigned long> * bookmarkList);

    bool UpdateBookmarkInDatabase(uint32 bookmarkID, uint32 ownerID, std::string memo);
};

#endif /* !__BOOKMARK_DB__H__INCL__ */

