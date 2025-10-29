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

CharFittingMgr::CharFittingMgr() : Service("charFittingMgr") {
    this->Add("GetFittings", &CharFittingMgr::GetFittings);
    this->Add("SaveFitting", &CharFittingMgr::SaveFitting);
    this->Add("SaveManyFittings", &CharFittingMgr::SaveManyFittings);
    this->Add("DeleteFitting", &CharFittingMgr::DeleteFitting);
    this->Add("UpdateNameAndDescription", &CharFittingMgr::UpdateNameAndDescription);
}

PyResult CharFittingMgr::GetFittings(PyCallArgs &call, PyInt *ownerID) {
    _log(PLAYER__CALL, "CharFittingMgr::Handle_GetFittings()");
    call.Dump(PLAYER__CALL_DUMP);

    auto fittings = m_db.GetCharacterShipFittings(ownerID->value());

    return fittings;
}

PyResult CharFittingMgr::SaveFitting(PyCallArgs &call, PyInt *ownerID, PyObject *fitting) {
    if (!fitting->arguments()->IsDict()) {
        codelog(SERVICE__ERROR, "CharFittingMgr::Handle_SaveFitting() - passed fitting is not a dict.");
        return nullptr;
    }
    auto fittingData = fitting->arguments()->AsDict();
    auto fittingID = m_db.SaveCharShipFitting(*fittingData, ownerID->value());

    return new PyInt(fittingID);
}

PyResult CharFittingMgr::SaveManyFittings(PyCallArgs &call, PyInt *ownerID, PyDict *fittingsToSave) {
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

PyResult CharFittingMgr::DeleteFitting(PyCallArgs &call, PyInt *ownerID, PyInt *fittingID) {
    DBerror err;
    if (sDatabase.RunQuery(err, "DELETE FROM chrShipFittings WHERE id = %u AND characterID = %u", fittingID->value(), ownerID->value())) {
        sDatabase.RunQuery(err, "DELETE FROM shipFittings WHERE fittingID = %u", fittingID->value());
    } else {
        _log(DATABASE__ERROR, "Error deleting fitting %u for character %u: %s", fittingID->value(), ownerID->value(), err.c_str());
    }

    return nullptr;
}

PyResult CharFittingMgr::UpdateNameAndDescription(PyCallArgs &call, PyInt *fittingID, PyInt *ownerID, PyWString *name, PyWString *description) {
    std::string cName, cDescription;
    sDatabase.DoEscapeString(cName, name->content());
    sDatabase.DoEscapeString(cDescription, description->content());

    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE chrShipFittings SET name = '%s', description = '%s' WHERE id = %u AND characterID = %u",
                            cName.c_str(),
                            cDescription.c_str(),
                            fittingID->value(), ownerID->value())) {
        _log(DATABASE__ERROR, "Error updating fitting %u for character %u: %s", fittingID->value(), ownerID->value(), err.c_str());
    }

    return nullptr;
}
