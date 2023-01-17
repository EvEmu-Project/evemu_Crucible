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
    Author:        Ubiquitatis
    Updates:    Allan
*/

#ifndef __EVEMU_CORP_CORPBM_H
#define __EVEMU_CORP_CORPBM_H

#include "services/Service.h"
#include "system/BookmarkDB.h"

class CorpBookmarkMgr : public Service <CorpBookmarkMgr>
{
public:
    CorpBookmarkMgr();

protected:
    BookmarkDB m_db;

    PyResult GetBookmarks(PyCallArgs& call);
    PyResult UpdateBookmark(PyCallArgs& call, PyInt* bookmarkID, PyInt* ownerID, PyRep* memo, PyRep* comment, std::optional<PyInt*> folderID);
    PyResult UpdatePlayerBookmark(PyCallArgs& call, PyInt* bookmarkID, PyInt* ownerID, PyRep* memo, PyRep* comment, std::optional<PyInt*> folderID);
    PyResult MoveBookmarksToFolder(PyCallArgs& call, PyInt* folderID, std::optional<PyObjectEx*> bookmarkIDs);
    PyResult CreateFolder(PyCallArgs& call, PyRep* folderName);
    PyResult UpdateFolder(PyCallArgs& call, PyInt* folderID, PyRep* folderName);
    PyResult CopyBookmarks(PyCallArgs& call, std::optional<PyRep*> bookmarksToCopy, PyInt* folderID);
    PyResult DeleteFolder(PyCallArgs& call, PyInt* folderID, PyRep* unused);
    PyResult MoveFoldersToDB(PyCallArgs& call, PyRep* info);
    PyResult DeleteBookmarks(PyCallArgs& call, std::optional<PyObjectEx*> bookmarkIDs);

};

#endif  // __EVEMU_CORP_CORPBM_H