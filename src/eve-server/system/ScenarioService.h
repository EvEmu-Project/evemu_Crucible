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
*/

#ifndef __SCENARIO_SERVICE_H_INCL__
#define __SCENARIO_SERVICE_H_INCL__

#include "system/SystemDB.h"
#include "services/Service.h"

class ScenarioService : public Service <ScenarioService>
{
public:
    ScenarioService();

protected:
    SystemDB m_db;

    PyResult ResetD(PyCallArgs& call);
    PyResult PlayDungeon(PyCallArgs& call, PyInt* dungeonVID, PyInt* selectedRoom);
    PyResult EditRoom(PyCallArgs& call, PyInt* dungeonVID, PyInt* selectedRoom);
    PyResult GotoRoom(PyCallArgs& call, PyInt* selectedRoom);
    PyResult GetDunObjects(PyCallArgs& call);
    PyResult GetSelObjects(PyCallArgs& call);
    PyResult IsSelectedByObjID(PyCallArgs& call, PyInt* dunObjectID);
    PyResult DuplicateSelection(PyCallArgs& call, PyInt* amount, PyFloat* x, PyFloat* y, PyFloat* z);
    PyResult SetSelectionByID(PyCallArgs& call, PyList* ids);
    PyResult SetSelectedRadius(PyCallArgs& call, PyFloat* minRadius, PyFloat* maxRadius);
    PyResult SetRotate(PyCallArgs& call, PyFloat* yaw, PyFloat* pitch, PyFloat* roll);
    PyResult RotateSelected(PyCallArgs& call, PyFloat* yaw, PyFloat* pitch, PyFloat* roll);
    PyResult JitterSelection(PyCallArgs& call, PyFloat* x, PyFloat* y, PyFloat* z);
    PyResult ArrangeSelection(PyCallArgs& call, PyFloat* x, PyFloat* y, PyFloat* z);
    PyResult DeleteSelected(PyCallArgs& call);
    PyResult RefreshSelection(PyCallArgs& call);
};

#endif