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

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "system/KeeperService.h"
#include "system/SystemManager.h"

class KeeperBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(KeeperBound)

    KeeperBound(PyServiceMgr *mgr, SystemDB *db)
    : PyBoundObject(mgr),
    m_db(db),
    m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "KeeperBound";

        PyCallable_REG_CALL(KeeperBound, EditDungeon);
        PyCallable_REG_CALL(KeeperBound, PlayDungeon);
        PyCallable_REG_CALL(KeeperBound, Reset);
        PyCallable_REG_CALL(KeeperBound, GotoRoom); //(int room)
        PyCallable_REG_CALL(KeeperBound, GetCurrentlyEditedRoomID);

    }
    virtual ~KeeperBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(EditDungeon);
    PyCallable_DECL_CALL(PlayDungeon);
    PyCallable_DECL_CALL(Reset);
    PyCallable_DECL_CALL(GotoRoom);
    PyCallable_DECL_CALL(GetCurrentlyEditedRoomID);

protected:
    SystemDB *const m_db;
    Dispatcher *const m_dispatch;   //we own this
};


PyCallable_Make_InnerDispatcher(KeeperService)

KeeperService::KeeperService(PyServiceMgr *mgr)
: PyService(mgr, "keeper"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(KeeperService, GetLevelEditor);
	PyCallable_REG_CALL(KeeperService, ActivateAccelerationGate);
	PyCallable_REG_CALL(KeeperService, CanWarpToPathPlex);

    //sm.RemoteSvc('keeper').ClientBSDRevisionChange(action, schemaName, tableName, rowKeys, columnValues, reverting)
}

KeeperService::~KeeperService() {
    delete m_dispatch;
}

PyBoundObject *KeeperService::CreateBoundObject(Client *pClient, const PyRep *bind_args) {
    _log(DUNG__TRACE, "KeeperService bind request for:");
    bind_args->Dump(DUNG__TRACE, "    ");

    return new KeeperBound(m_manager, &m_db);
}

PyResult KeeperService::Handle_GetLevelEditor(PyCallArgs &call)
{
    // self.ed = sm.RemoteSvc('keeper').GetLevelEditor()  (this is to bind new editor object)
    _log(DUNG__CALL,  "KeeperService::Handle_GetLevelEditor  size: %u", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    KeeperBound *ib = new KeeperBound(m_manager, &m_db);

    return m_manager->BindObject(call.client, ib);
}

PyResult KeeperService::Handle_CanWarpToPathPlex(PyCallArgs &call) {
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
    _log(DUNG__CALL,  "KeeperService::Handle_CanWarpToPathPlex  size: %u", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

	return nullptr;
}

/**  Hard-coded to random location....just to play with right now.
		will need to edit later to implement in missions/etc  */
PyResult KeeperService::Handle_ActivateAccelerationGate(PyCallArgs &call) {
    _log(DUNG__CALL,  "KeeperService::Handle_ActivateAccelerationGate  size: %u", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

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



PyResult KeeperBound::Handle_EditDungeon(PyCallArgs &call)
{
    //ed.EditDungeon(dungeonID, roomID=roomID)
    _log(DUNG__CALL,  "KeeperBound::Handle_EditDungeon  size: %u", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult KeeperBound::Handle_PlayDungeon(PyCallArgs &call)
{
    //ed.PlayDungeon(dungeonID, roomID=roomID, godmode=godmode)
    _log(DUNG__CALL,  "KeeperBound::Handle_PlayDungeon  size: %u", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult KeeperBound::Handle_Reset(PyCallArgs &call)
{
    _log(DUNG__CALL,  "KeeperBound::Handle_Reset  size: %u", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult KeeperBound::Handle_GotoRoom(PyCallArgs &call)
{
    _log(DUNG__CALL,  "KeeperBound::Handle_GotoRoom  size: %u", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult KeeperBound::Handle_GetCurrentlyEditedRoomID(PyCallArgs &call)
{
//return sm.RemoteSvc('keeper').GetLevelEditor().GetCurrentlyEditedRoomID()
    _log(DUNG__CALL,  "KeeperBound::Handle_GetCurrentlyEditedRoomID  size: %u", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

