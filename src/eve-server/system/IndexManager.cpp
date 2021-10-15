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
    Author:        Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "system/IndexManager.h"
#include "system/sov/SovereigntyDataMgr.h"
#include "inventory/AttributeEnum.h"

PyCallable_Make_InnerDispatcher(IndexManager)

IndexManager::IndexManager(PyServiceMgr *mgr)
: PyService(mgr, "devIndexManager"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(IndexManager, GetAllDevelopmentIndices);
    PyCallable_REG_CALL(IndexManager, GetDevelopmentIndicesForSystem);
}

IndexManager::~IndexManager() {
    delete m_dispatch;
}

PyResult IndexManager::Handle_GetAllDevelopmentIndices( PyCallArgs& call ) {

    /*
22:49:13 L IndexManager::Handle_GetAllDevelopmentIndices(): size= 0
        for indexInfo in sm.RemoteSvc('devIndexManager').GetAllDevelopmentIndices():
            systemToIndexMap[indexInfo.solarSystemID] = {const.attributeDevIndexMilitary: indexInfo.militaryPoints,
             const.attributeDevIndexIndustrial: indexInfo.industrialPoints,
             const.attributeDevIndexSovereignty: indexInfo.claimedFor * CLAIM_DAYS_TO_SECONDS}
  */
    return nullptr;
}

PyResult IndexManager::Handle_GetDevelopmentIndicesForSystem( PyCallArgs& call ) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    DBRowDescriptor *header = new DBRowDescriptor();
    header->AddColumn("attributeID", DBTYPE_I4);
    header->AddColumn("points", DBTYPE_I2);
    header->AddColumn("increasing", DBTYPE_BOOL);
    //CRowSet *rowset = new CRowSet(&header);

    SovereigntyData sovData = svDataMgr.GetSovereigntyData(args.arg);
    PyPackedRow *row1 = new PyPackedRow(header);
    PyPackedRow *row2 = new PyPackedRow(header);
    PyPackedRow *row3 = new PyPackedRow(header);
    
    // Strategic
    row1->SetField("attributeID", new PyInt(EveAttrEnum::AttrdevIndexSovereignty));
    double daysSinceClaim = (GetFileTimeNow() - sovData.claimTime) / Win32Time_Day;
    row1->SetField("points", new PyInt(uint32(daysSinceClaim)));
    row1->SetField("increasing", new PyBool(false));

    // Military
    row2->SetField("attributeID", new PyInt(EveAttrEnum::AttrdevIndexMilitary));
    row2->SetField("points", new PyInt(sovData.militaryPoints));
    row2->SetField("increasing", new PyBool(false));

    // Industrial
    row3->SetField("attributeID", new PyInt(EveAttrEnum::AttrdevIndexIndustrial));
    row3->SetField("points", new PyInt(sovData.industrialPoints));
    row3->SetField("increasing", new PyBool(false));

    PyDict* dict = new PyDict();
    dict->SetItem(new PyInt(EveAttrEnum::AttrdevIndexSovereignty), row1);
    dict->SetItem(new PyInt(EveAttrEnum::AttrdevIndexMilitary), row2);
    dict->SetItem(new PyInt(EveAttrEnum::AttrdevIndexIndustrial), row3);

    return dict;
}

/*
developmentIndices = [attributeDevIndexMilitary, attributeDevIndexIndustrial, attributeDevIndexSovereignty]
attributeDevIndexMilitary = 1583
attributeDevIndexIndustrial = 1584
attributeDevIndexSovereignty = 1615
*/