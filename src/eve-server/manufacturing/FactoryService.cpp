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

#include "PyServiceCD.h"
#include "manufacturing/Blueprint.h"
#include "manufacturing/FactoryService.h"

PyCallable_Make_InnerDispatcher(FactoryService)

FactoryService::FactoryService(PyServiceMgr *mgr)
: PyService(mgr, "factory"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(FactoryService, GetBlueprintAttributes);
    PyCallable_REG_CALL(FactoryService, GetMaterialsForTypeWithActivity);
    PyCallable_REG_CALL(FactoryService, GetMaterialCompositionOfItemType);
    PyCallable_REG_CALL(FactoryService, GetBlueprintInformationAtLocation);
    PyCallable_REG_CALL(FactoryService, GetBlueprintInformationAtLocationWithFlag);
}

FactoryService::~FactoryService() {
    delete m_dispatch;
}

PyResult FactoryService::Handle_GetMaterialCompositionOfItemType(PyCallArgs &call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return nullptr;
    }

    // Outpost construction platforms require a different query
    if ((arg.arg == EVEDB::invTypes::CaldariResearchOutpost) or
    (arg.arg == EVEDB::invTypes::AmarrFactoryOutpost) or
    (arg.arg == EVEDB::invTypes::GallenteAdministrativeOutpost) or
    (arg.arg == EVEDB::invTypes::MinmatarServiceOutpost)) {
        DBQueryResult res;
        FactoryDB::GetOutpostMaterialCompositionOfItemType(arg.arg, res);
        return DBResultToRowset(res);
    }

    return FactoryDB::GetMaterialCompositionOfItemType(arg.arg);
}

PyResult FactoryService::Handle_GetBlueprintAttributes(PyCallArgs &call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return nullptr;
    }

    BlueprintRef bRef = sItemFactory.GetBlueprintRef( arg.arg );
    if (bRef.get() == nullptr)
        return nullptr;

    return bRef->GetBlueprintAttributes();
}

PyResult FactoryService::Handle_GetMaterialsForTypeWithActivity(PyCallArgs &call) {
    // this is the material and manuf tab of bp.  -working  allan 1Jan17
	Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return nullptr;
    }

    return sDataMgr.GetBPMatlData(arg.arg);
}


// these next two are for corp locked items calls
PyResult FactoryService::Handle_GetBlueprintInformationAtLocation(PyCallArgs &call) {
    //    blueprints = sm.RemoteSvc('factory').GetBlueprintInformationAtLocation(hangarID, 1)
    _log(MANUF__MESSAGE, "FactoryService::GetBlueprintInformationAtLocation() size= %li", call.tuple->size());
    call.Dump(MANUF__DUMP);
    return nullptr;
}

PyResult FactoryService::Handle_GetBlueprintInformationAtLocationWithFlag(PyCallArgs &call) {
    //blueprints = sm.RemoteSvc('factory').GetBlueprintInformationAtLocationWithFlag(locationID, self.flagInput, 1)
    _log(MANUF__MESSAGE, "FactoryService::Handle_GetBlueprintInformationAtLocationWithFlag() size= %li", call.tuple->size());
    call.Dump(MANUF__DUMP);
    return nullptr;
}
