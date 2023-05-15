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
    Author:        Zhur
    Updates:    Allan
*/

#ifndef __KEEPER_SERVICE_H_INCL__
#define __KEEPER_SERVICE_H_INCL__

#include "system/SystemDB.h"
#include "services/Service.h"
#include "services/BoundService.h"
#include "Client.h"

class KeeperBound;

class KeeperService : public Service <KeeperService>, public BoundServiceParent <KeeperBound>
{
public:
    KeeperService(EVEServiceManager& mgr);

    void BoundReleased (KeeperBound* bound) override;
    KeeperBound* GetBound() { return m_instance; }

protected:
    SystemDB m_db;

    PyResult GetLevelEditor(PyCallArgs& call);
    PyResult CanWarpToPathPlex(PyCallArgs& call, PyInt* instanceID);
    PyResult ActivateAccelerationGate(PyCallArgs& call, PyInt* itemID);

private:
    EVEServiceManager& m_manager;
    KeeperBound* m_instance;
};


class KeeperBound : public EVEBoundObject <KeeperBound>
{
public:
    KeeperBound(EVEServiceManager& mgr, KeeperService& parent, SystemDB* db);
    virtual void AddRoomObject(DungeonEditSE *pSE) { m_roomObjects.push_back(pSE); }
    void RemoveRoomObject(uint32 itemID);
    DungeonEditSE* GetRoomObject(uint32 itemID);
    virtual uint32 GetCurrentRoomID() { return m_currentRoom; }

protected:
    PyResult EditDungeon(PyCallArgs& call, PyInt* dungeonID);
    PyResult PlayDungeon(PyCallArgs& call, PyInt* dungeonID);
    PyResult Reset(PyCallArgs& call);
    PyResult GotoRoom(PyCallArgs& call, PyInt* roomID);
    PyResult GetCurrentlyEditedRoomID(PyCallArgs& call);
    PyResult GetRoomObjects(PyCallArgs& call);
    PyResult GetRoomGroups(PyCallArgs& call, PyInt* roomID);
    PyResult ObjectSelection(PyCallArgs& call, PyList* objects);
    PyResult BatchStart(PyCallArgs& call);
    PyResult BatchEnd(PyCallArgs& call);

protected:
    SystemDB *const m_db;

private:
    uint32 m_currentDungeon;
    uint32 m_currentRoom;
    std::vector<DungeonEditSE*> m_roomObjects;
    std::vector<int32> m_selectedObjects;
};

#endif


