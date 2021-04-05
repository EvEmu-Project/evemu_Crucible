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

#ifndef __DUNGEON_SERVICE_H_INCL__
#define __DUNGEON_SERVICE_H_INCL__

#include "system/SystemDB.h"
#include "PyService.h"

class DungeonService
: public PyService
{
public:
    DungeonService(PyServiceMgr *mgr);
    ~DungeonService();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(DEGetFactions);
    PyCallable_DECL_CALL(DEGetDungeons);
    PyCallable_DECL_CALL(DEGetTemplates);
    PyCallable_DECL_CALL(DEGetRooms);
    PyCallable_DECL_CALL(DEGetRoomObjectPaletteData);
    PyCallable_DECL_CALL(TemplateRemove);
    PyCallable_DECL_CALL(TemplateEdit);
    PyCallable_DECL_CALL(GetArchetypes);
    PyCallable_DECL_CALL(RemoveObject);
    PyCallable_DECL_CALL(EditObjectName);
    PyCallable_DECL_CALL(CopyObject);
    PyCallable_DECL_CALL(EditObject);
    PyCallable_DECL_CALL(EditObjectRadius);
    PyCallable_DECL_CALL(EditObjectXYZ);
    PyCallable_DECL_CALL(EditObjectYawPitchRoll);
    PyCallable_DECL_CALL(IsObjectLocked);
    PyCallable_DECL_CALL(AddTemplateObjects);
    PyCallable_DECL_CALL(AddObject);
    PyCallable_DECL_CALL(TemplateAdd);
    PyCallable_DECL_CALL(TemplateObjectAddDungeonList);

};


#endif


