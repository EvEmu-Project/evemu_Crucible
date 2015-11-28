/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
    Author:        Reve
*/

//work in progress

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "pos/PlanetMgr.h"
#include "Colony.h"


class PlanetMgrBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(PlanetMgrBound)

    PlanetMgrBound(PyServiceMgr *mgr, uint32 planetID, uint32 charID)
    : PyBoundObject(mgr),
      m_dispatch(new Dispatcher(this)), m_planetID(planetID)
    {
        _SetCallDispatcher(m_dispatch);
        m_colony = new Colony(charID, m_planetID);
        m_strBoundObjectName = "PlanetMgrBound";

        PyCallable_REG_CALL(PlanetMgrBound, GetCommandPinsForPlanet)
        PyCallable_REG_CALL(PlanetMgrBound, GetExtractorsForPlanet)
        PyCallable_REG_CALL(PlanetMgrBound, GetPlanetInfo)
        PyCallable_REG_CALL(PlanetMgrBound, GetPlanetResourceInfo)
        PyCallable_REG_CALL(PlanetMgrBound, GetProgramResultInfo)
        PyCallable_REG_CALL(PlanetMgrBound, GetResourceData)
        PyCallable_REG_CALL(PlanetMgrBound, GMAddCommodity)
        PyCallable_REG_CALL(PlanetMgrBound, GMConvertCommandCenter)
        PyCallable_REG_CALL(PlanetMgrBound, GMForceInstallProgram)
        PyCallable_REG_CALL(PlanetMgrBound, GMGetLocalDistributionReport)
        PyCallable_REG_CALL(PlanetMgrBound, GMGetSynchedServerState)
        PyCallable_REG_CALL(PlanetMgrBound, GMRunDepletionSim)
        PyCallable_REG_CALL(PlanetMgrBound, UserAbandonPlanet)
        PyCallable_REG_CALL(PlanetMgrBound, UserLaunchCommodities)
        PyCallable_REG_CALL(PlanetMgrBound, UserTransferCommodities)
        PyCallable_REG_CALL(PlanetMgrBound, UserUpdateNetwork)
    }
    virtual ~PlanetMgrBound() { delete m_dispatch; }
    virtual void Release() {
        //He hates this statement
        delete this;
    }

    PyCallable_DECL_CALL(GetCommandPinsForPlanet)
    PyCallable_DECL_CALL(GetExtractorsForPlanet)
    PyCallable_DECL_CALL(GetPlanetInfo)
    PyCallable_DECL_CALL(GetPlanetResourceInfo)
    PyCallable_DECL_CALL(GetProgramResultInfo)
    PyCallable_DECL_CALL(GetResourceData)
    PyCallable_DECL_CALL(GMAddCommodity)
    PyCallable_DECL_CALL(GMConvertCommandCenter)
    PyCallable_DECL_CALL(GMForceInstallProgram)
    PyCallable_DECL_CALL(GMGetLocalDistributionReport)
    PyCallable_DECL_CALL(GMGetSynchedServerState)
    PyCallable_DECL_CALL(GMRunDepletionSim)
    PyCallable_DECL_CALL(UserAbandonPlanet)
    PyCallable_DECL_CALL(UserLaunchCommodities)
    PyCallable_DECL_CALL(UserTransferCommodities)
    PyCallable_DECL_CALL(UserUpdateNetwork)

protected:
    Dispatcher *const m_dispatch;
    const uint32 m_planetID;
    Colony *m_colony;
};

PyCallable_Make_InnerDispatcher(PlanetMgrService)

PlanetMgrService::PlanetMgrService(PyServiceMgr *mgr)
: PyService(mgr, "planetMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(PlanetMgrService, GetPlanetsForChar)
    PyCallable_REG_CALL(PlanetMgrService, GetMyLaunchesDetails)
}

PlanetMgrService::~PlanetMgrService() {
    delete m_dispatch;
}

PyBoundObject *PlanetMgrService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    if(!bind_args->IsInt()) {
        codelog(SERVICE__ERROR, "%s Service: invalid bind argument type %s", GetName(), bind_args->TypeString());
        return NULL;
    }
    return new PlanetMgrBound(m_manager, bind_args->AsInt()->value(), c->GetCharacterID());
}

PyResult PlanetMgrService::Handle_GetPlanetsForChar(PyCallArgs &call) {
    /* Used by the client to populate the industry:planets tab
     */
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT `solarSystemID`, `planetID`, `typeID`, `numberOfPins` FROM `planetsforchar` WHERE `characterID` = %u", call.client->GetCharacterID())) {
        codelog(SERVICE__ERROR, "Error in GetPlanetsForChar Query: %s", res.error.c_str());
        return NULL;
    }
    return DBResultToCRowset(res);
}

PyResult PlanetMgrService::Handle_GetMyLaunchesDetails(PyCallArgs &call) {
    //TODO, double check if this requires x,y,z, or if only Beyonce uses them.
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT `solarSystemID`, `planetID`, `launchTime`, `launchID`, `x`, `y`, `z` FROM `planetlaunches` WHERE `characterID` = %u", call.client->GetCharacterID())) {
        codelog(SERVICE__ERROR, "Error in GetMyLaunchesDetails Query: %s", res.error.c_str());
        return NULL;
    }
    return DBResultToRowset(res);
}

PyResult PlanetMgrBound::Handle_GetCommandPinsForPlanet(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GetCommandPinsForPlanet stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GetExtractorsForPlanet(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GetExtractorsForPlanet incomplete.");
    /* Incomplete, Needs to retrieve data from tables that do not exist yet.
     * Currently stops the client from throwing errors.
     */
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT 2130 AS `typeID`, 0 as `ownerID`")) {
        codelog(SERVICE__ERROR, "Error in GetExtractorsForPlanet Query: %s", res.error.c_str());
        return NULL;
    }
    return DBResultToRowset(res);
}

PyResult PlanetMgrBound::Handle_GetPlanetInfo(PyCallArgs &call) {
    sLog.Debug("Server", "Called GetPlanetInfo Incomplete.");
    /* Incomplete, needs to check if planet is colonised by char, if so, return full colony + planet data.
     * Right now every planet is un-colonised.
     */

    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT `solarSystemID`, `typeID` AS `planetTypeID`, `itemID` AS `planetID`, `radius` FROM mapdenormalize WHERE `itemID` = %u", m_planetID)) {
        codelog(SERVICE__ERROR, "Error in GetPlanetInfo query: %s", res.error.c_str());
        return NULL;
    }
    DBResultRow row;
    if(!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Error in GetPlanetInfo query, failed to get row");
        return NULL;
    }
    return DBRowToKeyVal(row);
}

PyResult PlanetMgrBound::Handle_GetPlanetResourceInfo(PyCallArgs &call) {
    /* Used by the client to draw the planet resource bars.
     * returns: {typeID:quality, typeID:quality, typeID:quality, typeID:quality, typeID:quality}
     * quality: (min=1.0, max=154.275)
     */
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT `itemID1`, `itemID2`, `itemID3`, `itemID4`, `itemID5`, `quality1`, `quality2`, `quality3`, `quality4`, `quality5` FROM `planetresourceinfo` WHERE `planetID` = %u", m_planetID)) {
        codelog(SERVICE__ERROR, "Error in GetPlanetResourceInfo Query: %s", res.error.c_str());
        return NULL;
    }
    DBResultRow row;
    if(!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Error in GetPlanetResourceInfo Query, Returned 0 rows.");
        return NULL;
    }
    PyDict *rtn = new PyDict();
    rtn->SetItem(new PyInt(row.GetInt(0)), new PyFloat(row.GetFloat(5)));
    rtn->SetItem(new PyInt(row.GetInt(1)), new PyFloat(row.GetFloat(6)));
    rtn->SetItem(new PyInt(row.GetInt(2)), new PyFloat(row.GetFloat(7)));
    rtn->SetItem(new PyInt(row.GetInt(3)), new PyFloat(row.GetFloat(8)));
    rtn->SetItem(new PyInt(row.GetInt(4)), new PyFloat(row.GetFloat(9)));
    return rtn;
}

PyResult PlanetMgrBound::Handle_GetProgramResultInfo(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GetProgramResultInfo stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GetResourceData(PyCallArgs &call) {
    /* TODO, Figure out how to populate PyBuffer with more than char.
     *         and figure out the client buffer structure, etc.
     * TODO, optimise this function maybe?
     */
    PyDict *input = call.tuple->AsTuple()->GetItem(0)->AsObject()->arguments()->AsDict();
    int proximity = input->GetItemString("proximity")->AsInt()->value();
    int resourceTypeID = input->GetItemString("resourceTypeID")->AsInt()->value();
    int offset = 0;

    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT `itemID1`, `itemID2`, `itemID3`, `itemID4`, `itemID5`, `data1`, `data2`, `data3`, `data4`, `data5`, `numBands1`, `numBands2`, `numBands3`, `numBands4`, `numBands5` FROM `planetresourceinfo` WHERE `planetID` = %u", m_planetID)) {
        codelog(SERVICE__ERROR, "Error in GetResourceData Query: %s", res.error.c_str());
        return NULL;
    }
    DBResultRow row;
    if(!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Error in GetResourceData Query failed to get row.");
        return NULL;
    }

    if (row.GetInt(0) == resourceTypeID)
        offset = 0;
    else if(row.GetInt(1) == resourceTypeID)
        offset = 1;
    else if(row.GetInt(2) == resourceTypeID)
        offset = 2;
    else if(row.GetInt(3) == resourceTypeID)
        offset = 3;
    else if(row.GetInt(4) == resourceTypeID)
        offset = 4;

    const char bufferData = *row.GetText(5+offset);
    int numBands = row.GetInt(10+offset);

    PyDict *args = new PyDict();
    PyObject *rtn = new PyObject("util.KeyVal", args);
    args->SetItemString("data", new PyBuffer(numBands*numBands*4, bufferData));
    args->SetItemString("numBands", new PyInt(numBands));
    args->SetItemString("proximity", new PyInt(proximity));
    return rtn;
}

PyResult PlanetMgrBound::Handle_GMAddCommodity(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GMAddCommodity stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GMConvertCommandCenter(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GMConvertCommandCenter stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GMForceInstallProgram(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GMForceInstallProgram stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GMGetLocalDistributionReport(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GMGetLocalDistributionReport stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GMGetSynchedServerState(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GMGetSynchedServerState stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_GMRunDepletionSim(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called GMRunDepletionSim stub.");

    return NULL;
}

PyResult PlanetMgrBound::Handle_UserAbandonPlanet(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called UserAbandonPlanet stub.");
    call.tuple->Dump(stdout, "[DEBUG] UAP: ");

    return NULL;
}

PyResult PlanetMgrBound::Handle_UserLaunchCommodities(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called UserLaunchCommodities stub.");
    call.tuple->Dump(stdout, "[DEBUG] ULC: ");

    return NULL;
}

PyResult PlanetMgrBound::Handle_UserTransferCommodities(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called UserTransferCommodities stub.");
    call.tuple->Dump(stdout, "[DEBUG] UTC: ");

    return NULL;
}

PyResult PlanetMgrBound::Handle_UserUpdateNetwork(PyCallArgs &call) {
    sLog.Debug("PlanetMgrBound", "Called UserUpdateNetwork incomplete.");
    call.tuple->Dump(stdout, "[DEBUG] UUN: ");

    UUNCommandList uuncl;
    if(!uuncl.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Failed to decode args for UUNCommandList");
        return NULL;
    }

    for(int i = 0; i < uuncl.commandList->size(); i++) {
        UUNCommand uunc;
        if(!uunc.Decode(uuncl.commandList->GetItem(i)->AsTuple())) {
            codelog(CLIENT__ERROR, "Failed to decode args for UUNCommand");
            return NULL;
        }
        sLog.Debug("PlanetMgrBound", "UserUpdateNetwork: loop: %u, command: %u", i, uunc.command);
        switch(uunc.command) {
            case 1: //COMMAND_CREATEPIN
            {
                uint32 typeID = uunc.command_data->GetItem(1)->AsInt()->value();
                uint32 groupID = m_manager->item_factory.GetType(typeID)->groupID();
                if(groupID == 1027) {
                    /* Command Pin
                     */
                    UUNCCommandCenter uunccc;
                    if(!uunccc.Decode(uunc.command_data)) {
                        codelog(SERVICE__ERROR, "Failed to decode args for UUNCCommandCenter!");
                    }

                    if(!m_colony->CreateCommandPin(uunccc.pinID, uunccc.typeID, uunccc.latitude, uunccc.longitude)) {
                        sLog.Error("PlanetMgrBound", "UserUpdateNetwork: Failed to create command center");
                    }else {
                        sLog.Success("PlanetMgrBound", "UserUpdateNetwork: Success creating command center");
                    }
                } else if(groupID == 1029 || groupID == 1030) {
                    /* Storage / Spaceport Pin
                     */
                    UUNCStandardPin uuncsp;
                    if(!uuncsp.Decode(uunc.command_data)) {
                        codelog(SERVICE__ERROR, "Failed to decode args for UUNCStandardPin!");
                    }

                    if(!m_colony->CreateSpaceportPin(uuncsp.pinID2, uuncsp.typeID, uuncsp.latitude, uuncsp.longitude)) {
                        sLog.Error("PlanetMgrBound", "UserUpdateNetwork: Failed to create storage/spaceport");
                    }else {
                        sLog.Success("PlanetMgrBound", "UserUpdateNetwork: Success creating storage/spaceport");
                    }
                } else if(groupID == 1028) {
                    /* Process Pin
                     */
                    UUNCStandardPin uuncsp;
                    if(!uuncsp.Decode(uunc.command_data)) {
                        codelog(SERVICE__ERROR, "Failed to decode args for UUNCStandardPin!");
                    }

                    if(!m_colony->CreateProcessPin(uuncsp.pinID2, uuncsp.typeID, uuncsp.latitude, uuncsp.longitude)) {
                        sLog.Error("PlanetMgrBound", "UserUpdateNetwork: Failed to create process");
                    }else {
                        sLog.Success("PlanetMgrBound", "UserUpdateNetwork: Success creating process");
                    }
                } else if(groupID == 1063) {
                    /* Extractor Pin
                     * //TODO, Still broken, client complains that extractor is not built.
                     */
                    UUNCStandardPin uuncsp;
                    if(!uuncsp.Decode(uunc.command_data)) {
                        codelog(SERVICE__ERROR, "Failed to decode args for UUNCStandardPin!");
                    }

                    if(!m_colony->CreateExtractorPin(uuncsp.pinID2, uuncsp.typeID, uuncsp.latitude, uuncsp.longitude)) {
                        sLog.Error("PlanetMgrBound", "UserUpdateNetwork: Failed to create extractor");
                    }else {
                        sLog.Success("PlanetMgrBound", "UserUpdateNetwork: Success creating extractor");
                    }
                }else {
                    // Invalid...
                    sLog.Debug("PlanetMgrBound", "UserUpdateNetwork: INVALID CREATEPIN groupID");
                }
                break;
            }
            case 2: //COMMAND_REMOVEPIN
            {
                //                                command_data           pinID2
                uint32 pinID = uunc.command_data->GetItem(0)->AsTuple()->GetItem(1)->AsInt()->value();
                if(!m_colony->RemovePin(pinID)){
                    sLog.Error("PlanetMgrBound", "UserUpdateNetwork: Failed to remove pin");
                }else {
                    sLog.Success("PlanetMgrBound", "UserUpdateNetwork: Success removing pin");
                }
                break;
            }
            case 3: //COMMAND_CREATELINK
            {
                if(uunc.command_data->GetItem(0)->IsInt()) {
                    UUNCLinkCommand uunclc;
                    if(!uunclc.Decode(uunc.command_data)) {
                        codelog(SERVICE__ERROR, "Failed to decode args for UUNCLinkCommand!");
                    }

                    if(!m_colony->CreateLink(uunclc.src, uunclc.dest2, uunclc.level, true)) {
                        sLog.Error("PlanetMgrBound", "UserUpdateNetwork: Failed to create link");
                    }else {
                        sLog.Success("PlanetMgrBound", "UserUpdateNetwork: Success creating link");
                    }
                }else if(uunc.command_data->GetItem(0)->IsTuple()) {
                    UUNCLinkStandard uuncls;
                    if(!uuncls.Decode(uunc.command_data)) {
                        codelog(SERVICE__ERROR, "Failed to decode args for UUNCLinkStandard!");
                    }

                    if(!m_colony->CreateLink(uuncls.src2, uuncls.dest2, uuncls.level, false)) {
                        sLog.Error("PlanetMgrBound", "UserUpdateNetwork: Failed to create link");
                    }else {
                        sLog.Success("PlanetMgrBound", "UserUpdateNetwork: Success creating link");
                    }
                }else {
                    //Invalid...
                    sLog.Debug("PlanetMgrBound", "UserUpdateNetwork: INVALID CREATELINK");
                }
                break;
            }
            case 4: //COMMAND_REMOVELINK
            {
                if(uunc.command_data->GetItem(0)->IsInt()) {
                    uint32 src = uunc.command_data->GetItem(0)->AsInt()->value();
                    uint32 dest2 = uunc.command_data->GetItem(1)->AsTuple()->GetItem(1)->AsInt()->value();
                    if(!m_colony->RemoveLink(src, dest2, true)) {
                        sLog.Error("PlanetMgrBound", "UserUpdateNetwork: Failed to remove link");
                    }else {
                        sLog.Success("PlanetMgrBound", "UserUpdateNetwork: Success removing link");
                    }
                }else if(uunc.command_data->GetItem(0)->IsTuple()) {
                    uint32 src = uunc.command_data->GetItem(0)->AsTuple()->GetItem(1)->AsInt()->value();
                    uint32 dest2 = uunc.command_data->GetItem(1)->AsTuple()->GetItem(1)->AsInt()->value();
                    if(!m_colony->RemoveLink(src, dest2, false)) {
                        sLog.Error("PlanetMgrBound", "UserUpdateNetwork: Failed to remove link");
                    }else {
                        sLog.Success("PlanetMgrBound", "UserUpdateNetwork: Success removing link");
                    }
                }
                break;
            }
            case 5: //COMMAND_SETLINKLEVEL
            {
                if(uunc.command_data->GetItem(0)->IsInt()) {
                    UUNCLinkCommand uunclc;
                    if(!uunclc.Decode(uunc.command_data)) {
                        codelog(SERVICE__ERROR, "Failed to decode args for UUNCLinkCommand!");
                    }

                    if(!m_colony->UpgradeLink(uunclc.src, uunclc.dest2, uunclc.level, true)) {
                        sLog.Error("PlanetMgrBound", "UserUpdateNetwork: Failed to upgrade link");
                    }else {
                        sLog.Success("PlanetMgrBound", "UserUpdateNetwork: Success upgrading link");
                    }
                }else if(uunc.command_data->GetItem(0)->IsTuple()) {
                    UUNCLinkStandard uuncls;
                    if (!uuncls.Decode(uunc.command_data)) {
                        codelog(SERVICE__ERROR, "Failed to decode args for UUNCLinkStandard!");
                    }

                    if (!m_colony->UpgradeLink(uuncls.src2, uuncls.dest2, uuncls.level, false)) {
                        sLog.Error("PlanetMgrBound", "UserUpdateNetwork: Failed to upgrade link");
                    } else {
                        sLog.Success("PlanetMgrBound", "UserUpdateNetwork: Success upgrading link");
                    }
                }
                break;
            }
            case 6: //COMMAND_CREATEROUTE
            {
                break;
            }
            case 7: //COMMAND_REMOVEROUTE
            {
                break;
            }
            case 8: //COMMAND_SETSCHEMATIC
            {
                break;
            }
            case 9: //COMMAND_UPGRADECOMMANDCENTER
            {
                uint32 pinID = uunc.command_data->GetItem(0)->AsInt()->value();
                uint32 level = uunc.command_data->GetItem(1)->AsInt()->value();
                m_colony->UpgradeCommandCenter(pinID, level);
                break;
            }
            case 10: //COMMAND_ADDEXTRACTORHEAD
            {
                break;
            }
            case 11: //COMMAND_KILLEXTRACTORHEAD
            {
                break;
            }
            case 12: //COMMAND_MOVEEXTRACTORHEAD
            {
                break;
            }
            case 13: //COMMAND_INSTALLPROGRAM
            {
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return m_colony->GetColony();
}
