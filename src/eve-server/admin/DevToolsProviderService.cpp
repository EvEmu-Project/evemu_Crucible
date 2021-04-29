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
    PyCallable_REG_CALL(DevToolsProviderService, ExceptionFluentExample);
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
    return PyStatic.NewNone();
}

PyResult DevToolsProviderService::Handle_ExceptionFluentExample (PyCallArgs& call)
{
    Call_SingleIntegerArg arg;

    if (arg.Decode (call.tuple) == false)
    {
        codelog(SERVICE__ERROR, "%s: Failed to decode bind args.", GetName());
        return nullptr;
    }

    switch (arg.arg)
    {
        case 0:
            throw UserError ("SalvagingTooComplex").AddTypeName ("type", EVEItemTypeID::itemTypeTrit);
        case 1:
            throw UserError ("SalvagingTooComplex").AddOwnerName ("type", call.client->GetCharID ());
        case 2:
            throw UserError ("SalvagingTooComplex").AddDate ("type", Win32TimeNow());
        case 3:
            throw UserError ("SalvagingTooComplex").AddDateTime ("type", Win32TimeNow());
        case 4:
            throw UserError ("SalvagingTooComplex").AddTime ("type", Win32TimeNow());
        case 5:
            throw UserError ("SalvagingTooComplex").AddTimeShort ("type", Win32Time_Hour + (Win32Time_Minute * 5));
        case 6:
            throw UserError ("SalvagingTooComplex").AddISK ("type", 160.52);
        case 7:
            throw UserError ("SalvagingTooComplex").AddAmount ("type", 150000);
        case 8:
            throw UserError ("SalvagingTooComplex").AddTypeDescription ("type", EVEItemTypeID::itemTypeTrit);
        case 9:
            throw UserError ("SalvagingTooComplex").AddGroupName ("type", EVEItemGroups::Clone);
        case 10:
            throw UserError ("SalvagingTooComplex").AddA ("type", "HELLO!");
        case 11:
            throw UserError ("SalvagingTooComplex").AddThe ("type", "hello world");
        case 12:
            throw UserError ("SalvagingTooComplex").AddBlueprintTypeName ("type", 681);
        case 13:
            throw UserError ("SalvagingTooComplex").AddCategoryName ("type", 1);
        case 14:
            throw UserError ("SalvagingTooComplex").AddCategoryDescription ("type", 1);
        case 15:
            throw UserError ("SalvagingTooComplex").AddAUR ("type", 1500.00);
        case 16:
            throw UserError ("SalvagingTooComplex").AddDistance ("type", 150000);
        case 17:
            throw UserError ("SalvagingTooComplex").AddLocationName ("type", call.client->GetStationID ());
        case 18:
            throw UserError ("SalvagingTooComplex").AddOwnerNick ("type", call.client->GetCharID ());
        case 19:
            throw UserError ("SalvagingTooComplex").AddTypeDescription ("type", 658);
        case 20:
        {
            PyList* list = new PyList (2);

            list->SetItem (0, new PyInt (35));
            list->SetItem (1, new PyInt (34));

            throw UserError ("SalvagingTooComplex").AddTypeList ("type", list);
        }
        case 21:
            throw UserError ("SalvagingTooComplex").AddTypeIDAndQuantity ("type", 35, 1500);
        case 22:
            throw UserError ("SalvagingTooComplex").AddUELocalization ("type", "UI/Commands/ShowInfo", nullptr);
        default:
            throw UserError ("SalvagingTooComplex").AddGroupDescription ("type", EVEItemGroups::Clone);
    }
}