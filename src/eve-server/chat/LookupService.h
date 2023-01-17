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


#ifndef __LOOKUPSVC_SERVICE_H_INCL__
#define __LOOKUPSVC_SERVICE_H_INCL__

#include "chat/LSCDB.h"
#include "services/Service.h"

class LookupService : public Service <LookupService>
{
public:
    LookupService();

protected:
    // most of these don't seem to be in the client code anymore and should probably be removed
    PyResult LookupEvePlayerCharacters(PyCallArgs& call, PyWString* searchString, PyInt* exact);
    PyResult LookupCharacters(PyCallArgs& call, PyWString* searchString, PyInt* exact);
    PyResult LookupPCOwners(PyCallArgs& call, PyWString* searchString, PyInt* exact);
    PyResult LookupOwners(PyCallArgs& call, PyWString* searchString, PyInt* exact);
    PyResult LookupNoneNPCAccountOwners(PyCallArgs& call, PyWString* searchString, PyInt* exact);
    PyResult LookupPlayerCharacters(PyCallArgs& call, PyWString* searchString);
    PyResult LookupCorporations(PyCallArgs& call, PyWString* searchString);
    PyResult LookupFactions(PyCallArgs& call, PyWString* searchString);
    PyResult LookupCorporationTickers(PyCallArgs& call, PyWString* searchString);
    PyResult LookupStations(PyCallArgs& call, PyWString* searchString);
    PyResult LookupKnownLocationsByGroup(PyCallArgs& call, PyWString* searchString, PyInt* exact);
};


#endif
