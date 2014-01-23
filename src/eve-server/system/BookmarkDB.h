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
    Author:        Bloody.Rabbit
*/

#ifndef __BOOKMARK_DB__H__INCL__
#define __BOOKMARK_DB__H__INCL__

#include "ServiceDB.h"

class BookmarkDB
: public ServiceDB
{
public:
    PyObjectEx *GetBookmarks(uint32 ownerID);
    PyObjectEx *GetFolders(uint32 ownerID);

    uint32 FindBookmarkTypeID(uint32 itemID);

    bool GetBookmarkInformation(uint32 bookmarkID, uint32 &ownerID, uint32 &itemID, uint32 &typeID,
                                uint32 &flag, std::string &memo, uint64 &created, double &x, double &y,
                                double &z, uint32 &locationID, std::string &note, uint32 &creatorID,
                                uint32 folderID);

    bool SaveNewBookmarkToDatabase(uint32 &bookmarkID, uint32 ownerID, uint32 itemID,
                                   uint32 typeID, uint32 flag, std::string memo, uint64 created,
                                   double x, double y, double z, uint32 locationID, std::string note,
                                   uint32 creatorID, uint32 folderID);

    bool DeleteBookmarkFromDatabase(uint32 ownerID, uint32 bookmarkID);

    bool DeleteBookmarksFromDatabase(uint32 ownerID, std::vector<unsigned long> *bookmarkList);

    bool UpdateBookmarkInDatabase(uint32 bookmarkID, uint32 ownerID, std::string memo, std::string note);

    bool SaveNewFolderToDatabase(uint32 &folderID, std::string folderName, uint32 ownerID, uint32 creatorID);

    bool UpdateFolderInDatabase(uint32 &folderID, std::string folderName, uint32 ownerID, uint32 creatorID);

    bool DeleteFolderFromDatabase(uint32 folderID, uint32 ownerID);
};

#endif /* !__BOOKMARK_DB__H__INCL__ */

