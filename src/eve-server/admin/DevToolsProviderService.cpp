/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
    Author:        Ubiquitatis
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "admin/DevToolsProviderService.h"

PyCallable_Make_InnerDispatcher(DevToolsProviderService)

DevToolsProviderService::DevToolsProviderService(PyServiceMgr* mgr)
: PyService(mgr, "devToolsProvider"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(DevToolsProviderService, GetLoader);
}

DevToolsProviderService::~DevToolsProviderService()
{
    delete m_dispatch;
}

PyResult DevToolsProviderService::Handle_GetLoader(PyCallArgs& call)
{
    FILE *pFile;
    if(pFile = fopen(EVEMU_ROOT"/etc/devtools.raw", "rb"))
    {
        fseek(pFile, 0, SEEK_END);
	int size = ftell(pFile);
	char * buf = new char[size];
	fseek(pFile, 0, SEEK_SET);
	fread(buf, 1, size, pFile);
	fclose(pFile);
	return new PyString(buf, size);
    }
    return new PyNone();
}
