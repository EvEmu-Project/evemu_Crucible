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

ScenarioService::ScenarioService() :
    Service("scenario")
{
    this->Add("ResetD", &ScenarioService::ResetD);
    this->Add("PlayDungeon", &ScenarioService::PlayDungeon);
    this->Add("EditRoom", &ScenarioService::EditRoom);
    this->Add("GotoRoom", &ScenarioService::GotoRoom);
    this->Add("GetDunObjects", &ScenarioService::GetDunObjects);
    this->Add("GetSelObjects", &ScenarioService::GetSelObjects);
    this->Add("IsSelectedByObjID", &ScenarioService::IsSelectedByObjID);
    this->Add("DuplicateSelection", &ScenarioService::DuplicateSelection);
    this->Add("SetSelectionByID", &ScenarioService::SetSelectionByID);
    this->Add("SetSelectedRadius", &ScenarioService::SetSelectedRadius);
    this->Add("SetRotate", &ScenarioService::SetRotate);
    this->Add("RotateSelected", &ScenarioService::RotateSelected);
    this->Add("JitterSelection", &ScenarioService::JitterSelection);
    this->Add("ArrangeSelection", &ScenarioService::ArrangeSelection);
    this->Add("DeleteSelected", &ScenarioService::DeleteSelected);
    this->Add("RefreshSelection", &ScenarioService::RefreshSelection);
}

PyResult ScenarioService::ResetD(PyCallArgs& call)
{
    _log(DUNG__CALL, "ScenarioService::ResetD()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::PlayDungeon(PyCallArgs& call, PyInt* dungeonVID, PyInt* selectedRoom)
{
//PlayDungeon(dungeonVID, selectedRoom)
    _log(DUNG__CALL, "ScenarioService::PlayDungeon()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::EditRoom(PyCallArgs& call, PyInt* dungeonVID, PyInt* selectedRoom)
{
    //EditRoom(dungeonVID, selectedRoom)
    _log(DUNG__CALL, "ScenarioService::EditRoom()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::GotoRoom(PyCallArgs& call, PyInt* selectedRoom)
{
//GotoRoom(selectedRoom)
    _log(DUNG__CALL, "ScenarioService::GotoRoom()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::GetDunObjects(PyCallArgs& call)
{
//GetDunObjects() (returns list of slim items)
    _log(DUNG__CALL, "ScenarioService::GetDunObjects()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::GetSelObjects(PyCallArgs& call)
{
//GetSelObjects() (returns list of slim items)
    _log(DUNG__CALL, "ScenarioService::GetSelObjects()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::IsSelectedByObjID(PyCallArgs& call, PyInt* dunObjectID)
{
//IsSelectedByObjID(dunObjectID)
    _log(DUNG__CALL, "ScenarioService::IsSelectedByObjID()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::DuplicateSelection(PyCallArgs& call, PyInt* amount, PyFloat* x, PyFloat* y, PyFloat* z)
{
//DuplicateSelection(amount, X, Y, Z)
    _log(DUNG__CALL, "ScenarioService::DuplicateSelection()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::SetSelectionByID(PyCallArgs& call, PyList* ids)
{
//SetSelectionByID(ids)
    _log(DUNG__CALL, "ScenarioService::SetSelectionByID()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::SetSelectedRadius(PyCallArgs& call, PyFloat* minRadius, PyFloat* maxRadius)
{
//SetSelectedRadius(minRadius, maxRadius)
    _log(DUNG__CALL, "ScenarioService::SetSelectedRadius()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::SetRotate(PyCallArgs& call, PyFloat* yaw, PyFloat* pitch, PyFloat* roll)
{
//SetRotate(y, p, r)
    _log(DUNG__CALL, "ScenarioService::SetRotate()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::RotateSelected(PyCallArgs& call, PyFloat* yaw, PyFloat* pitch, PyFloat* roll)
{
//RotateSelected(yaw, pitch, roll)
    _log(DUNG__CALL, "ScenarioService::RotateSelected()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::JitterSelection(PyCallArgs& call, PyFloat* x, PyFloat* y, PyFloat* z)
{
//JitterSelection(X, Y, Z)
    _log(DUNG__CALL, "ScenarioService::JitterSelection()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::ArrangeSelection(PyCallArgs& call, PyFloat* x, PyFloat* y, PyFloat* z)
{
//ArrangeSelection(X, Y, Z)
    _log(DUNG__CALL, "ScenarioService::ArrangeSelection()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::DeleteSelected(PyCallArgs& call)
{
//DeleteSelected()
    _log(DUNG__CALL, "ScenarioService::DeleteSelected()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::RefreshSelection(PyCallArgs& call)
{
    //RefreshSelection()
    _log(DUNG__CALL, "ScenarioService::RefreshSelection()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}
