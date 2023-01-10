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
    Author:        ozatomic
*/


#ifndef __INFOGATHERINGMGR_SERVICE_H_INCL__
#define __INFOGATHERINGMGR_SERVICE_H_INCL__

#include "services/Service.h"

class InfoGatheringMgr : public Service <InfoGatheringMgr>
{
public:
    InfoGatheringMgr();

protected:
    PyResult GetStateAndConfig(PyCallArgs& call);
    PyResult LogInfoEventsFromClient(PyCallArgs& call, PyList* loggedEvents);
};

#endif
