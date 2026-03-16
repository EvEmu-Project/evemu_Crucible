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
    Author:        EVEmu Team
*/

#include "eve-server.h"

#include "ship/ShipSkinMgrService.h"

ShipSkinMgrService::ShipSkinMgrService() :
    Service("shipSkinMgr", eAccessLevel_Character)
{
    this->Add("GetAppliedSkin", &ShipSkinMgrService::GetAppliedSkin);
    this->Add("GetAppliedSkinMaterialSetID", &ShipSkinMgrService::GetAppliedSkinMaterialSetID);
    this->Add("ActivateSkinLicense", &ShipSkinMgrService::ActivateSkinLicense);
    this->Add("ApplySkin", &ShipSkinMgrService::ApplySkin);
}

ShipSkinMgrService::~ShipSkinMgrService()
{
}

PyResult ShipSkinMgrService::GetAppliedSkin(PyCallArgs &call, PyInt* characterID, PyInt* itemID, PyInt* typeID)
{
    _log(SERVICE__MESSAGE, "shipSkinMgr::GetAppliedSkin called for character %u, item %u, type %u", 
         characterID ? characterID->value() : 0, 
         itemID ? itemID->value() : 0, 
         typeID ? typeID->value() : 0);

    return PyStatic.NewNone();
}

PyResult ShipSkinMgrService::GetAppliedSkinMaterialSetID(PyCallArgs &call, PyInt* characterID, PyInt* itemID, PyInt* typeID)
{
    _log(SERVICE__MESSAGE, "shipSkinMgr::GetAppliedSkinMaterialSetID called for character %u, item %u, type %u", 
         characterID ? characterID->value() : 0, 
         itemID ? itemID->value() : 0, 
         typeID ? typeID->value() : 0);

    return PyStatic.NewNone();
}

PyResult ShipSkinMgrService::ActivateSkinLicense(PyCallArgs &call, PyInt* itemID)
{
    _log(SERVICE__MESSAGE, "shipSkinMgr::ActivateSkinLicense called for item %u", 
         itemID ? itemID->value() : 0);

    return PyStatic.NewNone();
}

PyResult ShipSkinMgrService::ApplySkin(PyCallArgs &call, PyInt* itemID, PyInt* skinID)
{
    _log(SERVICE__MESSAGE, "shipSkinMgr::ApplySkin called for item %u, skin %u", 
         itemID ? itemID->value() : 0, 
         skinID ? skinID->value() : 0);

    return PyStatic.NewNone();
}
