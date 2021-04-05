/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Updates:    Allan
*/

#ifndef __EVEMU_SYSTEM_BOOKMARKDB_H_
#define __EVEMU_SYSTEM_BOOKMARKDB_H_

#include "ServiceDB.h"

/*
validCategories =
    const.categoryCelestial,
    const.categoryAsteroid,
    const.categoryStation,
    const.categoryShip,
    const.categoryStructure,
    const.categoryPlanetaryInteraction
*/

struct BmData {
    // uint8 flag;
    uint16 typeID;
    uint32 bookmarkID;
    uint32 ownerID;
    uint32 itemID;
    uint32 locationID;
    uint32 folderID;
    uint32 creatorID;
    int64 created;
    GPoint point;
    std::string memo;
    std::string note;
};

class BookmarkDB
: public ServiceDB
{
public:
    PyRep* GetBookmarksInFolder(uint32 folderID);
    PyRep* GetBookmarks(uint32 ownerID);
    PyRep* GetFolders(uint32 ownerID);

    static PyTuple* GetBookmarkDescription(uint32 bookmarkID);
    static const char* GetBookmarkName(uint32 bookmarkID);
    bool GetBookmarkInformation(uint32 bookmarkID, uint32& itemID, uint16& typeID, uint32& locationID, double& x, double& y, double& z);

    bool UpdateBookmark(uint32 bookmarkID, uint32 ownerID, std::string memo, std::string note, uint32 folderID=0);
    bool DeleteBookmark(uint32 ownerID, uint32 bookmarkID);
    bool DeleteBookmarks(std::vector<int32>* bookmarkList);

    bool UpdateFolder(int32 folderID, std::string folderName);
    bool DeleteFolder(int32 folderID);

    uint32 SaveNewFolder(std::string folderName, uint32 ownerID);
    void SaveNewBookmark(BmData &data);

    void ChangeOwner(uint32 bookmarkID, uint32 ownerID=1);
    void MoveBookmarkToFolder(int32 folderID, std::vector< int32 >* bookmarkList);

    void GetBookmarkByFolderID(int32 folderID, std::vector< int32 >& bmIDs);

    void GetVoucherData(BmData &data);
};

#endif  // __EVEMU_SYSTEM_BOOKMARKDB_H_

