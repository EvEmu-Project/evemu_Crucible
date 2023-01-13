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
#include "services/Service.h"

class DungeonService : public Service <DungeonService> {
public:
    DungeonService();

protected:
    PyResult IsObjectLocked(PyCallArgs& call, PyInt* objectID);
    PyResult AddObject(PyCallArgs& call, PyInt* roomID, PyInt* tupeID, PyFloat* x, PyFloat* y, PyFloat* z, PyFloat* pitch, PyFloat* roll, PyFloat* radius);
    PyResult RemoveObject(PyCallArgs& call, PyInt* objectID);
    PyResult CopyObject(PyCallArgs& call, PyInt* objectID, PyInt* roomID, PyFloat* offsetX, PyFloat* offsetY, PyFloat* offsetZ);
    PyResult EditObject(PyCallArgs& call);
    PyResult EditObjectName(PyCallArgs& call, PyInt* newObjectID, PyWString* objectName);
    PyResult EditObjectRadius(PyCallArgs& call);
    PyResult EditObjectXYZ(PyCallArgs& call);
    PyResult EditObjectYawPitchRoll(PyCallArgs& call);
    PyResult TemplateAdd(PyCallArgs& call, PyWString* templateName, PyWString* templateDescription);
    PyResult TemplateRemove(PyCallArgs& call, PyInt* templateID);
    PyResult TemplateEdit(PyCallArgs& call, PyInt* templateID, PyWString* templateName, PyWString* templateDescription);
    PyResult AddTemplateObjects(PyCallArgs& call, PyInt* roomID, PyInt* objectID, PyTuple* position);
    PyResult TemplateObjectAddDungeonList(PyCallArgs& call, PyInt* templateID, PyList* objectIDs);
    PyResult GetArchetypes(PyCallArgs& call);
    PyResult DEGetDungeons(PyCallArgs& call);
    PyResult DEGetTemplates(PyCallArgs& call);
    PyResult DEGetRooms(PyCallArgs& call);
    PyResult DEGetRoomObjectPaletteData(PyCallArgs& call);
    PyResult DEGetFactions(PyCallArgs& call);


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


