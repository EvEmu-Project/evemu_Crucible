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
    Author:        Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "system/ScenarioService.h"


PyCallable_Make_InnerDispatcher(ScenarioService)

ScenarioService::ScenarioService(PyServiceMgr *mgr)
: PyService(mgr, "scenario"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(ScenarioService, ResetD);
    PyCallable_REG_CALL(ScenarioService, PlayDungeon);
    PyCallable_REG_CALL(ScenarioService, EditRoom);
    PyCallable_REG_CALL(ScenarioService, GotoRoom);
    PyCallable_REG_CALL(ScenarioService, GetDunObjects);
    PyCallable_REG_CALL(ScenarioService, GetSelObjects);
    PyCallable_REG_CALL(ScenarioService, IsSelectedByObjID);
    PyCallable_REG_CALL(ScenarioService, DuplicateSelection);
    PyCallable_REG_CALL(ScenarioService, SetSelectionByID);
    PyCallable_REG_CALL(ScenarioService, SetSelectedRadius);
    PyCallable_REG_CALL(ScenarioService, SetRotate);
    PyCallable_REG_CALL(ScenarioService, RotateSelected);
    PyCallable_REG_CALL(ScenarioService, JitterSelection);
    PyCallable_REG_CALL(ScenarioService, ArrangeSelection);
    PyCallable_REG_CALL(ScenarioService, DeleteSelected);
    PyCallable_REG_CALL(ScenarioService, RefreshSelection);
}

ScenarioService::~ScenarioService() {
    delete m_dispatch;
}


PyResult ScenarioService::Handle_ResetD( PyCallArgs& call )
{
    _log(DUNG__CALL, "ScenarioService::ResetD()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_PlayDungeon( PyCallArgs& call )
{
//PlayDungeon(dungeonVID, selectedRoom)
    _log(DUNG__CALL, "ScenarioService::PlayDungeon()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_EditRoom( PyCallArgs& call )
{
    //EditRoom(dungeonVID, selectedRoom)
    _log(DUNG__CALL, "ScenarioService::EditRoom()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_GotoRoom( PyCallArgs& call )
{
//GotoRoom(selectedRoom)
    _log(DUNG__CALL, "ScenarioService::GotoRoom()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_GetDunObjects( PyCallArgs& call )
{
//GetDunObjects() (returns list of slim items)
    _log(DUNG__CALL, "ScenarioService::GetDunObjects()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_GetSelObjects( PyCallArgs& call )
{
//GetSelObjects() (returns list of slim items)
    _log(DUNG__CALL, "ScenarioService::GetSelObjects()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_IsSelectedByObjID( PyCallArgs& call )
{
//IsSelectedByObjID(dunObjectID)
    _log(DUNG__CALL, "ScenarioService::IsSelectedByObjID()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_DuplicateSelection( PyCallArgs& call )
{
//DuplicateSelection(amount, X, Y, Z)
    _log(DUNG__CALL, "ScenarioService::DuplicateSelection()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_SetSelectionByID( PyCallArgs& call )
{
//SetSelectionByID(ids)
    _log(DUNG__CALL, "ScenarioService::SetSelectionByID()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_SetSelectedRadius( PyCallArgs& call )
{
//SetSelectedRadius(minRadius, maxRadius)
    _log(DUNG__CALL, "ScenarioService::SetSelectedRadius()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_SetRotate( PyCallArgs& call )
{
//SetRotate(y, p, r)
    _log(DUNG__CALL, "ScenarioService::SetRotate()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_RotateSelected( PyCallArgs& call )
{
//RotateSelected(yaw, pitch, roll)
    _log(DUNG__CALL, "ScenarioService::RotateSelected()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_JitterSelection( PyCallArgs& call )
{
//JitterSelection(X, Y, Z)
    _log(DUNG__CALL, "ScenarioService::JitterSelection()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_ArrangeSelection( PyCallArgs& call )
{
//ArrangeSelection(X, Y, Z)
    _log(DUNG__CALL, "ScenarioService::ArrangeSelection()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_DeleteSelected( PyCallArgs& call )
{
//DeleteSelected()
    _log(DUNG__CALL, "ScenarioService::DeleteSelected()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_RefreshSelection( PyCallArgs& call )
{
    //RefreshSelection()
    _log(DUNG__CALL, "ScenarioService::RefreshSelection()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}
