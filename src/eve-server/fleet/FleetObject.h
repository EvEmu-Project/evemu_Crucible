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

#ifndef EVEMU_SHIP_FLEETOBJ_H_
#define EVEMU_SHIP_FLEETOBJ_H_

#include "PyService.h"
#include "fleet/FleetManager.h"

class FleetObject
 : public PyService
{
public:
    FleetObject(PyServiceMgr *mgr);
    ~FleetObject();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(CreateFleet);
    PyCallable_DECL_CALL(CreateWing);
    PyCallable_DECL_CALL(CreateSquad);
    PyCallable_DECL_CALL(SetMotdEx);
    PyCallable_DECL_CALL(GetMotd);
    PyCallable_DECL_CALL(CheckIsInFleet);
    PyCallable_DECL_CALL(MakeLeader);
    PyCallable_DECL_CALL(SetBooster);
    PyCallable_DECL_CALL(MoveMember);
    PyCallable_DECL_CALL(KickMember);
    PyCallable_DECL_CALL(DeleteWing);
    PyCallable_DECL_CALL(DeleteSquad);
    PyCallable_DECL_CALL(LeaveFleet);

    //overloaded in order to support bound objects:
    virtual PyBoundObject *_CreateBoundObject(Client *c, const PyRep *bind_args);

private:
    FleetService m_Svc;
};

#endif  // EVEMU_SHIP_FLEETOBJ_H_
