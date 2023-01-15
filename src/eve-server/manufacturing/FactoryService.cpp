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
    Rewrite:    Allan
*/

/*
 * # Manufacturing Logging:
 * MANUF__ERROR
 * MANUF__WARNING
 * MANUF__MESSAGE
 * MANUF__INFO
 * MANUF__DEBUG
 * MANUF__TRACE
 * MANUF__DUMP
 */

#include "eve-server.h"


#include "manufacturing/Blueprint.h"
#include "manufacturing/FactoryService.h"

FactoryService::FactoryService() :
    Service("factory")
{
    this->Add("GetBlueprintAttributes", &FactoryService::GetBlueprintAttributes);
    this->Add("GetMaterialsForTypeWithActivity", &FactoryService::GetMaterialsForTypeWithActivity);
    this->Add("GetMaterialCompositionOfItemType", &FactoryService::GetMaterialCompositionOfItemType);
    this->Add("GetBlueprintInformationAtLocation", &FactoryService::GetBlueprintInformationAtLocation);
    this->Add("GetBlueprintInformationAtLocationWithFlag", &FactoryService::GetBlueprintInformationAtLocationWithFlag);
}

PyResult FactoryService::GetMaterialCompositionOfItemType(PyCallArgs &call, PyInt* typeID) {
    // Outpost construction platforms require a different query
    if ((typeID->value() == EVEDB::invTypes::CaldariResearchOutpost) or
    (typeID->value() == EVEDB::invTypes::AmarrFactoryOutpost) or
    (typeID->value() == EVEDB::invTypes::GallenteAdministrativeOutpost) or
    (typeID->value() == EVEDB::invTypes::MinmatarServiceOutpost)) {
        DBQueryResult res;
        FactoryDB::GetOutpostMaterialCompositionOfItemType(typeID->value(), res);
        return DBResultToRowset(res);
    }

    return FactoryDB::GetMaterialCompositionOfItemType(typeID->value());
}

PyResult FactoryService::GetBlueprintAttributes(PyCallArgs &call, PyInt* blueprintID) {
    BlueprintRef bRef = sItemFactory.GetBlueprintRef(blueprintID->value());
    if (bRef.get() == nullptr)
        return nullptr;

    return bRef->GetBlueprintAttributes();
}

PyResult FactoryService::GetMaterialsForTypeWithActivity(PyCallArgs &call, PyInt* typeID) {
    return sDataMgr.GetBPMatlData(typeID->value());
}


// these next two are for corp locked items calls
PyResult FactoryService::GetBlueprintInformationAtLocation(PyCallArgs &call, PyInt* hangarID, PyInt* one) {
    //    blueprints = sm.RemoteSvc('factory').GetBlueprintInformationAtLocation(hangarID, 1)
    _log(MANUF__MESSAGE, "FactoryService::GetBlueprintInformationAtLocation() size= %li", call.tuple->size());
    call.Dump(MANUF__DUMP);
    return nullptr;
}

PyResult FactoryService::GetBlueprintInformationAtLocationWithFlag(PyCallArgs &call, PyInt* locationID, PyInt* flag, PyInt* one) {
    //blueprints = sm.RemoteSvc('factory').GetBlueprintInformationAtLocationWithFlag(locationID, self.flagInput, 1)
    _log(MANUF__MESSAGE, "FactoryService::Handle_GetBlueprintInformationAtLocationWithFlag() size= %li", call.tuple->size());
    call.Dump(MANUF__DUMP);
    return nullptr;
}
