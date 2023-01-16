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

/* Dungeon Logging
 * DUNG__ERROR
 * DUNG__WARNING
 * DUNG__INFO
 * DUNG__MESSAGE
 * DUNG__TRACE
 * DUNG__CALL
 * DUNG__CALL_DUMP
 * DUNG__RSP_DUMP
 * DUNG__DB_ERROR
 * DUNG__DB_WARNING
 * DUNG__DB_INFO
 * DUNG__DB_MESSAGE
 */

#include "eve-server.h"

#include "system/KeeperService.h"
#include "system/SystemManager.h"

KeeperService::KeeperService(EVEServiceManager& mgr) :
    Service("keeper", eAccessLevel_SolarSystem),
    m_manager (mgr),
    m_instance (nullptr)
{
    this->Add("GetLevelEditor", &KeeperService::GetLevelEditor);
    this->Add("ActivateAccelerationGate", &KeeperService::ActivateAccelerationGate);
    this->Add("CanWarpToPathPlex", &KeeperService::CanWarpToPathPlex);

    //sm.RemoteSvc('keeper').ClientBSDRevisionChange(action, schemaName, tableName, rowKeys, columnValues, reverting)
}

void KeeperService::BoundReleased (KeeperBound* bound) {
    this->m_instance = nullptr;
}

PyResult KeeperService::GetLevelEditor(PyCallArgs &call)
{
    // self.ed = sm.RemoteSvc('keeper').GetLevelEditor()  (this is to bind new editor object)
    _log(DUNG__CALL,  "KeeperService::Handle_GetLevelEditor  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    if (this->m_instance == nullptr) {
        this->m_instance = new KeeperBound (m_manager, *this, &m_db);
    }

    this->m_instance->NewReference (call.client);

    return new PySubStruct(new PySubStream(this->m_instance->GetOID()));
}

PyResult KeeperService::CanWarpToPathPlex(PyCallArgs &call, PyInt* instanceID) {
    /*
                    resp = sm.RemoteSvc('keeper').CanWarpToPathPlex(node.rec.instanceID)
                if resp:
                    if resp is True:
                        m.append((uiutil.MenuLabel('UI/Inflight/WarpToBookmark'), self.WarpToHiddenDungeon, (node.id, node)))
                    else:
                        mickey = sm.StartService('michelle')
                        me = mickey.GetBall(eve.session.shipid)
                        dist = (foo.Vector3(resp) - foo.Vector3(me.x, me.y, me.z)).Length()

                        */
    _log(DUNG__CALL,  "KeeperService::Handle_CanWarpToPathPlex  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

	return nullptr;
}

/**  Hard-coded to random location....just to play with right now.
		will need to edit later to implement in missions/etc  */
PyResult KeeperService::ActivateAccelerationGate(PyCallArgs &call, PyInt* itemID) {
    _log(DUNG__CALL,  "KeeperService::Handle_ActivateAccelerationGate  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    Client *pClient(call.client);

    /** @todo   this should be called for gate... */
    //pClient->GetShipSE()->DestinyMgr()->SendSpecialEffect10(args.arg, 0, "effects.WarpGateEffect", 0, 1, 0);

    double distance = MakeRandomFloat(5, 25) * ONE_AU_IN_METERS;
    GPoint currentPosition(pClient->GetShipSE()->GetPosition());
    GPoint deltaPosition;
    deltaPosition.x = MakeRandomFloat(-1.0, 1.0) * distance;
    deltaPosition.y = MakeRandomFloat(-1.0, 1.0) * distance;
    deltaPosition.z = MakeRandomFloat(-2.0, 2.0) * ONE_AU_IN_METERS;
    GPoint warpToPoint(currentPosition+deltaPosition);              // Make a warp-in point variable
    GVector vectorToDestination(currentPosition, warpToPoint);
    double distanceToDestination = vectorToDestination.length();
    pClient->GetShipSE()->DestinyMgr()->WarpTo(warpToPoint, distanceToDestination);

    /* return error msg from this call, if applicable, else nodeid and timestamp */
    return new PyLong(Win32TimeNow());
}

KeeperBound::KeeperBound(EVEServiceManager& mgr, KeeperService& parent, SystemDB* db) :
    EVEBoundObject(mgr, parent),
    m_db(db)
{
    this->Add("EditDungeon", &KeeperBound::EditDungeon);
    this->Add("PlayDungeon", &KeeperBound::PlayDungeon);
    this->Add("Reset", &KeeperBound::Reset);
    this->Add("GotoRoom", &KeeperBound::GotoRoom); //(int room)
    this->Add("GetCurrentlyEditedRoomID", &KeeperBound::GetCurrentlyEditedRoomID);
}

PyResult KeeperBound::EditDungeon(PyCallArgs &call, PyInt* dungeonID)
{
    //ed.EditDungeon(dungeonID, roomID=roomID)
    _log(DUNG__CALL,  "KeeperBound::Handle_EditDungeon  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult KeeperBound::PlayDungeon(PyCallArgs &call, PyInt* dungeonID)
{
    //ed.PlayDungeon(dungeonID, roomID=roomID, godmode=godmode)
    _log(DUNG__CALL,  "KeeperBound::Handle_PlayDungeon  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult KeeperBound::Reset(PyCallArgs &call)
{
    _log(DUNG__CALL,  "KeeperBound::Handle_Reset  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult KeeperBound::GotoRoom(PyCallArgs &call, PyInt* roomID)
{
    _log(DUNG__CALL,  "KeeperBound::Handle_GotoRoom  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult KeeperBound::GetCurrentlyEditedRoomID(PyCallArgs &call)
{
//return sm.RemoteSvc('keeper').GetLevelEditor().GetCurrentlyEditedRoomID()
    _log(DUNG__CALL,  "KeeperBound::Handle_GetCurrentlyEditedRoomID  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

