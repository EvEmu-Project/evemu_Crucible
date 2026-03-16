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
    Author: Zhur
*/

#include "eve-server.h"

#include "structuredirectory/StructureDirectoryService.h"

StructureDirectoryService::StructureDirectoryService() :
    Service<StructureDirectoryService>("structureDirectory")
{
    this->Add("GetMyDockableStructures", &StructureDirectoryService::GetMyDockableStructures);
    this->Add("GetStructuresInSystem", &StructureDirectoryService::GetStructuresInSystem);
    this->Add("GetStructureMapData", &StructureDirectoryService::GetStructureMapData);
}

PyResult StructureDirectoryService::GetMyDockableStructures(PyCallArgs& call)
{
    sLog.Debug("StructureDirectoryService", "GetMyDockableStructures called");
    
    PyList* list = new PyList();
    sLog.Debug("StructureDirectoryService", "GetMyDockableStructures: Returning empty list with %zu items", list->size());
    
    return list;
}

PyResult StructureDirectoryService::GetStructuresInSystem(PyCallArgs& call)
{
    sLog.Debug("StructureDirectoryService", "GetStructuresInSystem called - returning empty list");
    PyList* list = new PyList();
    return list;
}

PyResult StructureDirectoryService::GetStructureMapData(PyCallArgs& call)
{
    sLog.Debug("StructureDirectoryService", "GetStructureMapData called - returning empty list");
    PyList* list = new PyList();
    return list;
}
