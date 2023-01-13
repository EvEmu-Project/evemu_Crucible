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
    Author:        Zhur, Aknor Jaden
*/


#include "eve-server.h"

#include "PyServiceCD.h"
#include "chat/LookupService.h"

LookupService::LookupService() :
    Service("lookupSvc")
{
    this->Add("LookupCharacters", &LookupService::LookupCharacters);
    this->Add("LookupOwners", &LookupService::LookupOwners);
    this->Add("LookupPlayerCharacters", &LookupService::LookupPlayerCharacters);
    this->Add("LookupCorporations", &LookupService::LookupCorporations);
    this->Add("LookupFactions", &LookupService::LookupFactions);
    this->Add("LookupCorporationTickers", &LookupService::LookupCorporationTickers);
    this->Add("LookupStations", &LookupService::LookupStations);
    this->Add("LookupKnownLocationsByGroup", &LookupService::LookupKnownLocationsByGroup);
    this->Add("LookupEvePlayerCharacters", &LookupService::LookupEvePlayerCharacters);
    this->Add("LookupPCOwners", &LookupService::LookupPCOwners);
    this->Add("LookupNoneNPCAccountOwners", &LookupService::LookupNoneNPCAccountOwners);
}

PyResult LookupService::LookupEvePlayerCharacters(PyCallArgs& call, PyWString* searchString, PyInt* exact) {
    return ServiceDB::LookupChars(searchString->content().c_str(), exact->value() ? true : false);
}

PyResult LookupService::LookupCharacters(PyCallArgs &call, PyWString* searchString, PyInt* exact) {
    return ServiceDB::LookupChars(searchString->content().c_str(), exact->value() ? true : false);
}

// this may actually be a call to search for player corps by name.
PyResult LookupService::LookupPCOwners(PyCallArgs &call, PyWString* searchString, PyInt* exact) {
    return ServiceDB::LookupChars(searchString->content().c_str(), exact->value() ? true : false);
}
//LookupOwners
PyResult LookupService::LookupOwners(PyCallArgs &call, PyWString* searchString, PyInt* exact) {
    return ServiceDB::LookupOwners(searchString->content().c_str(), exact->value() ? true : false);
}

PyResult LookupService::LookupNoneNPCAccountOwners(PyCallArgs &call, PyWString* searchString, PyInt* exact) {
    return ServiceDB::LookupOwners(searchString->content().c_str(), exact->value() ? true : false);
}

PyResult LookupService::LookupPlayerCharacters(PyCallArgs &call, PyWString* searchString) {
    return ServiceDB::LookupChars(searchString->content().c_str(), false);
}
PyResult LookupService::LookupCorporations(PyCallArgs &call, PyWString* searchString) {
    return ServiceDB::LookupCorporations(searchString->content().c_str());
}
PyResult LookupService::LookupFactions(PyCallArgs &call, PyWString* searchString) {
    return ServiceDB::LookupFactions(searchString->content().c_str());
}
PyResult LookupService::LookupCorporationTickers(PyCallArgs &call, PyWString* searchString) {
    return ServiceDB::LookupCorporationTickers(searchString->content().c_str());
}
PyResult LookupService::LookupStations(PyCallArgs &call, PyWString* searchString) {
    return ServiceDB::LookupStations(searchString->content().c_str());
}

PyResult LookupService::LookupKnownLocationsByGroup(PyCallArgs &call, PyWString* searchString, PyInt* exact) {
    return ServiceDB::LookupKnownLocationsByGroup(searchString->content().c_str(), exact->value() ? true : false);
}
