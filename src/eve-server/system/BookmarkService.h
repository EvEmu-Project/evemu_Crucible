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
    Author:        Zhur, Allan
*/


#ifndef __BOOKMARK_SERVICE_H_INCL__
#define __BOOKMARK_SERVICE_H_INCL__

#include "services/Service.h"

#include "system/BookmarkDB.h"
#include "Client.h"

class BookmarkService : public Service <BookmarkService>
{
public:
    BookmarkService();

    bool LookupBookmark(uint32 bookmarkID, uint32& itemID, uint16& typeID, uint32& locationID, double& x, double& y, double& z);

protected:
    uint32 nextBookmarkID;

    BookmarkDB m_db;

    PyResult GetBookmarks(PyCallArgs& call);
    PyResult BookmarkLocation(PyCallArgs& call, PyInt* itemID, PyInt* ownerID, PyRep* memo, PyRep* comment, std::optional<PyInt*> folderID);
    PyResult DeleteBookmarks(PyCallArgs& call, std::optional <PyObjectEx*> bookmarks);
    PyResult CreateFolder(PyCallArgs& call, PyRep* name);
    PyResult UpdateFolder(PyCallArgs& call, PyInt* folderID, PyRep* folderName);
    PyResult DeleteFolder(PyCallArgs& call, PyInt* folderID);
    PyResult MoveBookmarksToFolder(PyCallArgs& call, PyInt* folderID, std::optional <PyObjectEx*> bookmarkIDs);
    PyResult CopyBookmarks(PyCallArgs& call, std::optional <PyObjectEx*> bookmarksToCopy, std::optional <PyInt*> folderID);
    PyResult AddBookmarkFromVoucher(PyCallArgs& call, PyInt* itemID, PyInt* ownerID, std::optional <PyInt*> folderID);
    PyResult BookmarkScanResult(PyCallArgs& call, PyInt* locationID, PyRep* memo, PyRep* comment, PyString* resultID, PyInt* ownerID, std::optional<PyInt*> folderID);
};

#endif

/*{'FullPath': u'UI/Messages', 'messageID': 263643, 'label': u'CannotModifyBookmarkNotCreatorBody'}(u'You cannot modify the location as you did not create it and are not a communication officer.', None, None)
 */