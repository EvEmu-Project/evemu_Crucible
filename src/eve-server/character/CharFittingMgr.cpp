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
    Author:        Reve (outline only)
    Updates:    Allan
*/

//work in progress
// note:  these will be same calls as in corpFittingMgr


#include "eve-server.h"


#include "character/CharFittingMgr.h"

CharFittingMgr::CharFittingMgr() :
    Service("charFittingMgr")
{
    this->Add("GetFittings", &CharFittingMgr::GetFittings);
    this->Add("SaveFitting", &CharFittingMgr::SaveFitting);
    this->Add("SaveManyFittings", &CharFittingMgr::SaveManyFittings);
    this->Add("DeleteFitting", &CharFittingMgr::DeleteFitting);
    this->Add("UpdateNameAndDescription", &CharFittingMgr::UpdateNameAndDescription);
}

//11:37:43 L CharFittingMgr::Handle_GetFittings(): size= 1 from 'allan'
PyResult CharFittingMgr::GetFittings(PyCallArgs &call, PyInt* ownerID) {
    //self.fittings[ownerID] = self.GetFittingMgr(ownerID).GetFittings(ownerID)
    // client/script/environment/fittingsvc.py(112) PersistFitting

    _log(PLAYER__CALL, "CharFittingMgr::Handle_GetFittings()");
    call.Dump(PLAYER__CALL_DUMP);

    return nullptr;
}

PyResult CharFittingMgr::SaveFitting(PyCallArgs &call, PyInt* ownerID, PyObject* fitting)
{
    //    fitting.ownerID = ownerID
    //    fitting.fittingID = self.GetFittingMgr(ownerID).SaveFitting(ownerID, fitting)
    _log(PLAYER__CALL, "CharFittingMgr::Handle_SaveFitting()");
    call.Dump(PLAYER__CALL_DUMP);

    return nullptr;
}

PyResult CharFittingMgr::SaveManyFittings(PyCallArgs &call, PyInt* ownerID, PyDict* fittingsToSave)
{
    /*
        newFittingIDs = self.GetFittingMgr(ownerID).SaveManyFittings(ownerID, fittingsToSave)
        for row in newFittingIDs:
            self.fittings[ownerID][row.realFittingID] = fittingsToSave[row.tempFittingID]
            self.fittings[ownerID][row.realFittingID].fittingID = row.realFittingID
        */
    _log(PLAYER__CALL, "CharFittingMgr::Handle_SaveManyFittings()");
    call.Dump(PLAYER__CALL_DUMP);

    return nullptr;
}

PyResult CharFittingMgr::DeleteFitting(PyCallArgs &call, PyInt* ownerID, PyInt* fittingID)
{
    // self.GetFittingMgr(ownerID).DeleteFitting(ownerID, fittingID)
    _log(PLAYER__CALL, "CharFittingMgr::Handle_DeleteFitting()");
    call.Dump(PLAYER__CALL_DUMP);

    return nullptr;
}

PyResult CharFittingMgr::UpdateNameAndDescription(PyCallArgs &call, PyInt* fittingID, PyInt* ownerID, PyWString* name, PyWString* description)
{
    // self.GetFittingMgr(ownerID).UpdateNameAndDescription(fittingID, ownerID, name, description)
    _log(PLAYER__CALL, "CharFittingMgr::Handle_UpdateNameAndDescription()");
    call.Dump(PLAYER__CALL_DUMP);

    return nullptr;
}
