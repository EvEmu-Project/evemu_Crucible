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
#include <algorithm>


#include "system/IndexManager.h"
#include "system/sov/SovereigntyDataMgr.h"
#include "inventory/AttributeEnum.h"

IndexManager::IndexManager() :
    Service("devIndexManager", eAccessLevel_SolarSystem2)
{
    this->Add("GetAllDevelopmentIndices", &IndexManager::GetAllDevelopmentIndices);
    this->Add("GetDevelopmentIndicesForSystem", &IndexManager::GetDevelopmentIndicesForSystem);
}

PyResult IndexManager::GetAllDevelopmentIndices(PyCallArgs& call) {

    /*
22:49:13 L IndexManager::Handle_GetAllDevelopmentIndices(): size= 0
        for indexInfo in sm.RemoteSvc('devIndexManager').GetAllDevelopmentIndices():
            systemToIndexMap[indexInfo.solarSystemID] = {const.attributeDevIndexMilitary: indexInfo.militaryPoints,
             const.attributeDevIndexIndustrial: indexInfo.industrialPoints,
             const.attributeDevIndexSovereignty: indexInfo.claimedFor * CLAIM_DAYS_TO_SECONDS}
  */
    return svDataMgr.GetAllDevelopmentIndicesForClients();
}

PyResult IndexManager::GetDevelopmentIndicesForSystem(PyCallArgs& call, PyInt* solarSystemID) {
    DBRowDescriptor *header = new DBRowDescriptor();
    header->AddColumn("attributeID", DBTYPE_I4);
    header->AddColumn("points", DBTYPE_I4);
    header->AddColumn("increasing", DBTYPE_BOOL);
    //CRowSet *rowset = new CRowSet(&header);

    SovereigntyData sovData = svDataMgr.GetSovereigntyData(solarSystemID->value());
    PyPackedRow *row1 = new PyPackedRow(header);
    PyPackedRow *row2 = new PyPackedRow(header);
    PyPackedRow *row3 = new PyPackedRow(header);
    
    // Strategic
    row1->SetField("attributeID", new PyInt(EveAttrEnum::AttrdevIndexSovereignty));
    // Unclaimed empire/low-sec systems have claimTime == 0. Reporting days
    // since the Windows epoch would give a bogus strategic index.
    const uint32 strategicDays = std::min(
        svDataMgr.GetEffectiveStrategicDays(sovData), uint32(65535));
    row1->SetField("points", new PyInt(strategicDays));
    row1->SetField("increasing", new PyBool(false));

    // Military
    row2->SetField("attributeID", new PyInt(EveAttrEnum::AttrdevIndexMilitary));
    row2->SetField("points", new PyInt(svDataMgr.GetEffectiveMilitaryPoints(sovData)));
    row2->SetField("increasing", new PyBool(false));

    // Industrial
    row3->SetField("attributeID", new PyInt(EveAttrEnum::AttrdevIndexIndustrial));
    row3->SetField("points", new PyInt(svDataMgr.GetEffectiveIndustrialPoints(sovData)));
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