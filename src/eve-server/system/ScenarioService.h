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
#include "PyService.h"

class ScenarioService
: public PyService
{
public:
    ScenarioService(PyServiceMgr *mgr);
    virtual ~ScenarioService();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    SystemDB m_db;

    PyCallable_DECL_CALL(ResetD);
    PyCallable_DECL_CALL(PlayDungeon);
    PyCallable_DECL_CALL(EditRoom);
    PyCallable_DECL_CALL(GotoRoom);
    PyCallable_DECL_CALL(GetDunObjects);
    PyCallable_DECL_CALL(GetSelObjects);
    PyCallable_DECL_CALL(IsSelectedByObjID);
    PyCallable_DECL_CALL(DuplicateSelection);
    PyCallable_DECL_CALL(SetSelectionByID);
    PyCallable_DECL_CALL(SetSelectedRadius);
    PyCallable_DECL_CALL(SetRotate);
    PyCallable_DECL_CALL(RotateSelected);
    PyCallable_DECL_CALL(JitterSelection);
    PyCallable_DECL_CALL(ArrangeSelection);
    PyCallable_DECL_CALL(DeleteSelected);
    PyCallable_DECL_CALL(RefreshSelection);

    //overloaded in order to support bound objects:
    //virtual PyBoundObject *CreateBoundObject(Client *pClient, const PyRep *bind_args);
};

#endif