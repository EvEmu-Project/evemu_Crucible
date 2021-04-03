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
    Author:        Reve, Comet0
    Rewrite:    Allan
*/

//work in progress

/*
 * PLANET__ERROR
 * PLANET__WARNING
 * PLANET__MESSAGE
 * PLANET__DEBUG
 * PLANET__INFO
 * PLANET__TRACE
 * PLANET__DUMP
 * PLANET__RES_DUMP
 * PLANET__GC_DUMP
 * PLANET__PKT_TRACE
 * PLANET__DB_ERROR
 * PLANET__DB_WARNING
 */


#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "packets/Planet.h"
#include "planet/Colony.h"
#include "planet/Planet.h"
#include "planet/PlanetMgr.h"
#include "planet/PlanetMgrBound.h"
#include "system/SystemManager.h"

class PlanetMgrBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(PlanetMgrBound)

    PlanetMgrBound(PyServiceMgr *mgr, Client* pClient, PlanetSE* pPlanet)
    : PyBoundObject(mgr),
    m_dispatch(new Dispatcher(this)),
    m_planet(pPlanet)
    {
        _SetCallDispatcher(m_dispatch);

        m_colony = pPlanet->GetColony(pClient);

        m_colony->Init();

        m_planetMgr = new PlanetMgr(mgr, pClient, pPlanet, m_colony);

        m_strBoundObjectName = "PlanetMgrBound";

        PyCallable_REG_CALL(PlanetMgrBound, GetPlanetInfo);
        PyCallable_REG_CALL(PlanetMgrBound, GetPlanetResourceInfo);
        PyCallable_REG_CALL(PlanetMgrBound, GetCommandPinsForPlanet);
        PyCallable_REG_CALL(PlanetMgrBound, GetFullNetworkForOwner);
        PyCallable_REG_CALL(PlanetMgrBound, GetExtractorsForPlanet);
        PyCallable_REG_CALL(PlanetMgrBound, GetProgramResultInfo);
        PyCallable_REG_CALL(PlanetMgrBound, GetResourceData);
        PyCallable_REG_CALL(PlanetMgrBound, UserAbandonPlanet);
        PyCallable_REG_CALL(PlanetMgrBound, UserLaunchCommodities);
        PyCallable_REG_CALL(PlanetMgrBound, UserTransferCommodities);
        PyCallable_REG_CALL(PlanetMgrBound, UserUpdateNetwork);
        /* not supported yet */
        PyCallable_REG_CALL(PlanetMgrBound, GMAddCommodity);
        PyCallable_REG_CALL(PlanetMgrBound, GMConvertCommandCenter);
        PyCallable_REG_CALL(PlanetMgrBound, GMForceInstallProgram);
        PyCallable_REG_CALL(PlanetMgrBound, GMGetLocalDistributionReport);
        PyCallable_REG_CALL(PlanetMgrBound, GMGetSynchedServerState);
        PyCallable_REG_CALL(PlanetMgrBound, GMRunDepletionSim);
    }

    virtual ~PlanetMgrBound() {
        delete m_dispatch;
        SafeDelete(m_planetMgr);
    }

    virtual void Release() {
        delete this;
    }

    PyCallable_DECL_CALL(GetPlanetInfo);
    PyCallable_DECL_CALL(GetPlanetResourceInfo);
    PyCallable_DECL_CALL(GetCommandPinsForPlanet);
    PyCallable_DECL_CALL(GetFullNetworkForOwner);
    PyCallable_DECL_CALL(GetExtractorsForPlanet);
    PyCallable_DECL_CALL(GetProgramResultInfo);
    PyCallable_DECL_CALL(GetResourceData);
    PyCallable_DECL_CALL(UserAbandonPlanet);
    PyCallable_DECL_CALL(UserLaunchCommodities);
    PyCallable_DECL_CALL(UserTransferCommodities);
    PyCallable_DECL_CALL(UserUpdateNetwork);
    PyCallable_DECL_CALL(GMAddCommodity);
    PyCallable_DECL_CALL(GMConvertCommandCenter);
    PyCallable_DECL_CALL(GMForceInstallProgram);
    PyCallable_DECL_CALL(GMGetLocalDistributionReport);
    PyCallable_DECL_CALL(GMGetSynchedServerState);
    PyCallable_DECL_CALL(GMRunDepletionSim);
    /*

    data = planet.remoteHandler.GMGetCompleteResource(resourceTypeID, layer)
        sh = builder.CreateSHFromBuffer(data.data, data.numBands)

    self.planet.remoteHandler.GMCreateNuggetLayer(self.planetID, typeID)
        self.GMShowResource(typeID, 'nuggets')      {{ 'nuggets' = layer here }}

        */

protected:
    Colony* m_colony;
    PlanetDB* m_db;
    PlanetSE* m_planet;
    PlanetMgr* m_planetMgr;
    Dispatcher* const m_dispatch;
};

PyCallable_Make_InnerDispatcher(PlanetMgrService)

PlanetMgrService::PlanetMgrService(PyServiceMgr *mgr)
: PyService(mgr, "planetMgr"),  /*planetBaseBroker is for dust*/
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(PlanetMgrService, GetPlanet);
    PyCallable_REG_CALL(PlanetMgrService, DeleteLaunch);
    PyCallable_REG_CALL(PlanetMgrService, GetPlanetsForChar);
    PyCallable_REG_CALL(PlanetMgrService, GetMyLaunchesDetails);
}

PlanetMgrService::~PlanetMgrService() {
    delete m_dispatch;
}

PyBoundObject* PlanetMgrService::CreateBoundObject(Client *pClient, const PyRep *bind_args) {
    /* sends planetID */
    _log(PLANET__INFO, "PlanetMgrService bind request for:");
    bind_args->Dump(PLANET__INFO, "    ");
    if (!bind_args->IsInt()) {
        _log(SERVICE__ERROR, "%s Service: invalid bind argument type %s", GetName(), bind_args->TypeString());
        return nullptr;
    }

    StaticData sData = StaticData();
    sDataMgr.GetStaticInfo(bind_args->AsInt()->value(), sData);
    SystemManager* pSysMgr = sEntityList.FindOrBootSystem(sData.systemID);
    if (pSysMgr == nullptr) {
        pClient->SendErrorMsg("system boot failure");
        return nullptr;
    }
    SystemEntity* pSE = pSysMgr->GetSE(sData.itemID);
    if (!pSE->IsPlanetSE()) {
        _log(SERVICE__ERROR, "%s Service: itemID is not planetID or planet not found", GetName());
        return nullptr;
    }
    return new PlanetMgrBound(m_manager, pClient, pSE->GetPlanetSE());
}

PyResult PlanetMgrService::Handle_GetPlanetsForChar(PyCallArgs &call) {
  return m_db->GetPlanetsForChar(call.client->GetCharacterID());
}

PyResult PlanetMgrService::Handle_GetMyLaunchesDetails(PyCallArgs &call) {
    return m_db->GetMyLaunchesDetails(call.client->GetCharacterID());
}

PyResult PlanetMgrBound::Handle_GetPlanetResourceInfo(PyCallArgs &call) {
    if (!sConfig.cosmic.PIEnabled) {
        call.client->SendErrorMsg("The PI system is currently disabled.");
        return nullptr;
    }

    return m_planet->GetPlanetResourceInfo();
}

PyResult PlanetMgrBound::Handle_GetPlanetInfo(PyCallArgs &call) {
    if (!sConfig.cosmic.PIEnabled) {
        call.client->SendErrorMsg("The PI system is currently disabled.");
        return nullptr;
    }
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GetPlanetInfo() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    PyRep* res = m_planet->GetPlanetInfo(m_colony);
        res->Dump(PLANET__RES_DUMP, "    ");

    return res;
}

PyResult PlanetMgrBound::Handle_GetExtractorsForPlanet(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_planet->GetExtractorsForPlanet(args.arg);
}

PyResult PlanetMgrBound::Handle_UserUpdateNetwork(PyCallArgs &call) {
    if (!sConfig.cosmic.PIEnabled) {
        call.client->SendErrorMsg("The PI system is currently disabled.");
        return nullptr;
    }
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_UserUpdateNetwork() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    UUNCommandList uuncl;
    if (!uuncl.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_planetMgr->UpdateNetwork(uuncl);
}

PyResult PlanetMgrBound::Handle_GetProgramResultInfo(PyCallArgs &call) {
    /*
        qtyToDistribute, cycleTime, numCycles = self.remoteHandler.GetProgramResultInfo(pinID, typeID, pin.heads, headRadius)

     * 09:08:39 [PlanetDebug] PlanetMgrBound::Handle_GetProgramResultInfo() size=4
     * 09:08:39 [PlanetCallDump]   Call Arguments:
     * 09:08:39 [PlanetCallDump]       Tuple: 4 elements
     * 09:08:39 [PlanetCallDump]         [ 0] Integer field: 140006400              << ECU itemID
     * 09:08:39 [PlanetCallDump]         [ 1] Integer field: 2268                   << resource typeID
     * 09:08:39 [PlanetCallDump]         [ 2] List: 3 elements                      << heads
     * 09:08:39 [PlanetCallDump]         [ 2]   [ 0] Tuple: 3 elements
     * 09:08:39 [PlanetCallDump]         [ 2]   [ 0]   [ 0] Integer field: 0
     * 09:08:39 [PlanetCallDump]         [ 2]   [ 0]   [ 1] Real field: 0.323109
     * 09:08:39 [PlanetCallDump]         [ 2]   [ 0]   [ 2] Real field: 2.773487
     * 09:08:39 [PlanetCallDump]         [ 2]   [ 1] Tuple: 3 elements
     * 09:08:39 [PlanetCallDump]         [ 2]   [ 1]   [ 0] Integer field: 1
     * 09:08:39 [PlanetCallDump]         [ 2]   [ 1]   [ 1] Real field: 0.344639
     * 09:08:39 [PlanetCallDump]         [ 2]   [ 1]   [ 2] Real field: 2.882588
     * 09:08:39 [PlanetCallDump]         [ 2]   [ 2] Tuple: 3 elements
     * 09:08:39 [PlanetCallDump]         [ 2]   [ 2]   [ 0] Integer field: 2
     * 09:08:39 [PlanetCallDump]         [ 2]   [ 2]   [ 1] Real field: 0.378424
     * 09:08:39 [PlanetCallDump]         [ 2]   [ 2]   [ 2] Real field: 2.820603
     * 09:11:30 [PlanetCallDump]         [ 3] Real field: 0.010000                  << smallest size.  0d1hr
     * 09:12:30 [PlanetCallDump]         [ 3] Real field: 0.018581                  << medium size.    3d
     * 09:13:36 [PlanetCallDump]         [ 3] Real field: 0.050000                  << largest size.  14d

    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GetProgramResultInfo() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);
     */

    Call_ProgramResults args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return sPIDataMgr.GetProgramResultInfo(m_colony, args.ecuID, args.typeID, args.heads, args.headRadius);
}

PyResult PlanetMgrBound::Handle_GetResourceData(PyCallArgs &call) {
    //_log(PLANET__DEBUG, "PlanetMgrBound::Handle_GetResourceData() size=%u", call.tuple->size() );
    //call.Dump(PLANET__DUMP);
    /*
        inRange, sh = planet.GetResourceData(resourceTypeID)        << check packets for this call --- none.  :(
*/
            /*  this is called by planet view page, by "resource filter" for given typeID
             *     this is dump before dict decoding
             * 12:23:45 [PlanetCallDump]    Dictionary: 8 entries
             * 12:23:45 [PlanetCallDump]      [ 0] Key: String: 'proximity'
             * 12:23:45 [PlanetCallDump]      [ 0] Value: Integer field: 4
             * 12:23:45 [PlanetCallDump]      [ 1] Key: String: 'updateTime'
             * 12:23:45 [PlanetCallDump]      [ 1] Value: Integer field: 0
             * 12:23:45 [PlanetCallDump]      [ 2] Key: String: 'advancedPlanetology'
             * 12:23:45 [PlanetCallDump]      [ 2] Value: Integer field: 0
             * 12:23:45 [PlanetCallDump]      [ 3] Key: String: 'remoteSensing'
             * 12:23:45 [PlanetCallDump]      [ 3] Value: Integer field: 3
             * 12:23:45 [PlanetCallDump]      [ 4] Key: String: 'newBand'
             * 12:23:45 [PlanetCallDump]      [ 4] Value: Integer field: 15
             * 12:23:45 [PlanetCallDump]      [ 5] Key: String: 'planetology'
             * 12:23:45 [PlanetCallDump]      [ 5] Value: Integer field: 0
             * 12:23:45 [PlanetCallDump]      [ 6] Key: String: 'oldBand'
             * 12:23:45 [PlanetCallDump]      [ 6] Value: Integer field: 0
             * 12:23:45 [PlanetCallDump]      [ 7] Key: String: 'resourceTypeID'
             * 12:23:45 [PlanetCallDump]      [ 7] Value: Integer field: 2268
             */


    Call_ResourceDataDict dict;
    PyDict* input = call.tuple->AsTuple()->GetItem(0)->AsObject()->arguments()->AsDict();
    //input->Dump(PLANET__DUMP, "   ");
    if (!dict.Decode(&input)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_planet->GetResourceData(dict);
}

//01:52:23 [PlanetDebug] PlanetMgrBound::Handle_UserAbandonPlanet() size=0
PyResult PlanetMgrBound::Handle_UserAbandonPlanet(PyCallArgs &call) {
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_UserAbandonPlanet() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    m_planet->AbandonColony(m_colony);

    return nullptr;
}

PyResult PlanetMgrBound::Handle_UserLaunchCommodities(PyCallArgs &call) {
    /*
            lastLaunchTime = self.remoteHandler.UserLaunchCommodities(commandPinID, commoditiesToLaunch)
            for typeID, qty in commoditiesToLaunch.iteritems():
            */
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_UserLaunchCommodities() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);
    /* 20:00:35 L PlanetMgrBound: Handle_UserLaunchCommodities() size=2
     * 20:00:35 [PlanetCallDump]   Call Arguments:
     * 20:00:35 [PlanetCallDump]       Tuple: 2 elements
     * 20:00:35 [PlanetCallDump]         [ 0] Integer field: 140000083
     * 20:00:35 [PlanetCallDump]         [ 1] Dictionary: 1 entries
     * 20:00:35 [PlanetCallDump]         [ 1]   [ 0] Key: Integer field: 2268
     * 20:00:35 [PlanetCallDump]         [ 1]   [ 0] Value: Integer field: 1
     * 20:00:35 [PlanetCallDump]   Call Named Arguments:
     * 20:00:35 [PlanetCallDump]     Argument 'machoVersion':
     * 20:00:35 [PlanetCallDump]         Integer field: 1
     *
     *
21:44:05 [PlanetCallDump]    Dictionary: 1 entries
21:44:05 [PlanetCallDump]      [ 0] Key: Integer field: 3645
21:44:05 [PlanetCallDump]      [ 0] Value: Real field: 120.000000   << no clue why this changed to float from int.
eve-server: /usr/local/src/eve/EVEmu/src/eve-common/python/PyRep.h:141: PyInt* PyRep::AsInt(): Assertion `IsInt()' failed.
     */
    Call_LaunchCommodities args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    PyDict* dict = args.dict->AsDict();
    //dict->Dump(PLANET__DUMP, "   ");
    std::map<uint16, uint32> items;
    PyDict::const_iterator itr = dict->begin();
    for (; itr != dict->end(); ++itr)
        items.insert(std::pair<uint16, uint32>(PyRep::IntegerValue(itr->first), PyRep::IntegerValue(itr->second)));

    return m_colony->LaunchCommodities(args.pinID, items);
}

PyResult PlanetMgrBound::Handle_UserTransferCommodities(PyCallArgs &call) {
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_UserTransferCommodities() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);
/*
        simTime, sourceRunTime = self.remoteHandler.UserTransferCommodities(path, commodities)
        {{ simTime = time to stop (complete time), sourceRunTime = previous runtime}}
        15:19:15 [PlanetDebug] PlanetMgrBound::Handle_UserTransferCommodities() size=2
        15:19:15 [PlanetCallDump]   Call Arguments:
        15:19:15 [PlanetCallDump]       Tuple: 2 elements
        15:19:15 [PlanetCallDump]         [ 0] List: 5 elements
        15:19:15 [PlanetCallDump]         [ 0]   [ 0] Integer field: 140007050
        15:19:15 [PlanetCallDump]         [ 0]   [ 1] Integer field: 140007049
        15:19:15 [PlanetCallDump]         [ 0]   [ 2] Integer field: 140006983
        15:19:15 [PlanetCallDump]         [ 0]   [ 3] Integer field: 140007002
        15:19:15 [PlanetCallDump]         [ 0]   [ 4] Integer field: 140006971
        15:19:15 [PlanetCallDump]         [ 1] Dictionary: 2 entries
        15:19:15 [PlanetCallDump]         [ 1]   [ 0] Key: Integer field: 2390
        15:19:15 [PlanetCallDump]         [ 1]   [ 0] Value: Integer field: 100
        15:19:15 [PlanetCallDump]         [ 1]   [ 1] Key: Integer field: 2309
        15:19:15 [PlanetCallDump]         [ 1]   [ 1] Value: Integer field: 44400
        */

    PyDict* dict = call.tuple->GetItem(1)->AsDict();
    std::map<uint16, uint32> items;
    PyDict::const_iterator itr = dict->begin();
    for (; itr != dict->end(); ++itr)
        items.insert(std::pair<uint16, uint32>(PyRep::IntegerValue(itr->first), PyRep::IntegerValue(itr->second)));

    PyList* list = call.tuple->GetItem(0)->AsList();
    return m_colony->TransferCommodities(PyRep::IntegerValue(list->items.front()), PyRep::IntegerValue(list->items.back()), items);
}


/**     ***********************************************************************
 * @note   these do absolutely nothing at this time....
 */

PyResult PlanetMgrService::Handle_GetPlanet(PyCallArgs &call) {
    _log(PLANET__DEBUG, "PlanetMgrService::Handle_GetPlanet() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    return nullptr;
}

PyResult PlanetMgrService::Handle_DeleteLaunch(PyCallArgs &call) {
    //sm.RemoteSvc('planetMgr').DeleteLaunch
    _log(PLANET__DEBUG, "PlanetMgrService::Handle_DeleteLaunch() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    return nullptr;
}

PyResult PlanetMgrBound::Handle_GetCommandPinsForPlanet(PyCallArgs &call) {
    /*  called by "get other character's networks" in planet menu
     * 16:42:42 [PlanetDebug] PlanetMgrBound::Handle_GetCommandPinsForPlanet() size=1
     * 16:42:42 [PlanetCallDump]   Call Arguments:
     * 16:42:42 [PlanetCallDump]       Tuple: 1 elements
     * 16:42:42 [PlanetCallDump]         [ 0] Integer field: 40159411
     */
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GetCommandPinsForPlanet() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    // returns empty dict if none
    return new PyDict();
}

PyResult PlanetMgrBound::Handle_GetFullNetworkForOwner(PyCallArgs &call) {
    /*
     */
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GetFullNetworkForOwner() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    // returns empty dict if none
    return new PyDict();
}

PyResult PlanetMgrBound::Handle_GMAddCommodity(PyCallArgs &call) {
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GMAddCommodity() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    return nullptr;
}

PyResult PlanetMgrBound::Handle_GMConvertCommandCenter(PyCallArgs &call) {
    //self.remoteHandler.GMConvertCommandCenter(pinID)
    //  this is an option in the GM planet menu.  no clue what it's for or what it does.....
    /*
     * 02:48:32 [PlanetDebug] PlanetMgrBound::Handle_GMConvertCommandCenter() size=1
     * 02:48:32 [PlanetCallDump]   Call Arguments:
     * 02:48:32 [PlanetCallDump]       Tuple: 1 elements
     * 02:48:32 [PlanetCallDump]         [ 0] Integer field: 140006224      << CC PinID (currently wrong)
     */

    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GMConvertCommandCenter() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    return nullptr;
}

PyResult PlanetMgrBound::Handle_GMForceInstallProgram(PyCallArgs &call) {
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GMForceInstallProgram() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);
    /*
     *        if typeID not in resourceInfo or qtyPerCycle < 0 or cycleTime < 10 * SEC or lifetimeHours < 1 or headRadius <= 0.0:
     *            return
     *        self.remoteHandler.GMForceInstallProgram(pinID, typeID, cycleTime, lifetimeHours, qtyPerCycle, headRadius)
     *
     * 16:40:57 L PlanetMgrBound: Handle_GMForceInstallProgram() size=6
     * 16:40:57 [PlanetCallDump]   Call Arguments:
     * 16:40:57 [PlanetCallDump]       Tuple: 6 elements
     * 16:40:57 [PlanetCallDump]         [ 0] Tuple: 2 elements
     * 16:40:57 [PlanetCallDump]         [ 0]   [ 0] Integer field: 1
     * 16:40:57 [PlanetCallDump]         [ 0]   [ 1] Integer field: 1
     * 16:40:57 [PlanetCallDump]         [ 1] Integer field: 2272
     * 16:40:57 [PlanetCallDump]         [ 2] Integer field: 600000000
     * 16:40:57 [PlanetCallDump]         [ 3] Integer field: 24
     * 16:40:57 [PlanetCallDump]         [ 4] Integer field: 100
     * 16:40:57 [PlanetCallDump]         [ 5] Real field: 1.000000
     */
    return nullptr;
}

//15:15:02[00m L [37;01mPlanetMgrBound: [00mHandle_GMGetLocalDistributionReport() size=2
PyResult PlanetMgrBound::Handle_GMGetLocalDistributionReport(PyCallArgs &call) {
    /*
     *      return self.remoteHandler.GMGetLocalDistributionReport(self.planetID, (surfacePoint.theta, surfacePoint.phi))
     */
    /*
     *     15:15:02 [PlanetCallDump]   Call Arguments:
     *     15:15:02 [PlanetCallDump]       Tuple: 2 elements
     *     15:15:02 [PlanetCallDump]         [ 0] Integer field: 40216265      << planetID
     *     15:15:02 [PlanetCallDump]         [ 1] Tuple: 2 elements
     *     15:15:02 [PlanetCallDump]         [ 1]   [ 0] Real field: 0.359286  << theta
     *     15:15:02 [PlanetCallDump]         [ 1]   [ 1] Real field: 1.014020  << phi
     *     _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GMGetLocalDistributionReport() size=%u", call.tuple->size() );
     *     call.Dump(PLANET__DUMP);
     */

    return nullptr;
}

PyResult PlanetMgrBound::Handle_GMGetSynchedServerState(PyCallArgs &call) {
    /*
     *    def GMVerifySimulation(self):
     *        self.LogNotice('VerifySimulation -- starting')
     *        simulationDuration, remoteColonyData = self.remoteHandler.GMGetSynchedServerState(session.charid)
     *        simEndTime = remoteColonyData.currentSimTime
     *        colony = self.GetColony(session.charid)
     *        startTime = blue.os.GetWallclockTimeNow()
     *        colony.RunSimulation(runSimUntil=simEndTime)
     *        clientSimulationRuntime = blue.os.GetWallclockTimeNow() - startTime
     *        pins = remoteColonyData.pins
     *        self.LogNotice('simulation ran for', clientSimulationRuntime, 'on client, ', simulationDuration, 'on server')
     *        for pin in pins:
     *            clientPin = colony.GetPin(pin.id)
     *            if clientPin is None:
     *                self.LogError(pin.id, 'exists on server but not on client')
     *                continue
     *            for key, value in pin.__dict__.iteritems():
     *                if not hasattr(clientPin, key):
     *                    self.LogError(pin.id, 'on client does not have attribute ', key)
     *                    continue
     *                clientValue = getattr(clientPin, key)
     *                if clientValue != value:
     *                    self.LogError(pin.id, 'does not agree on a value for', key, 'Client says ', clientValue, 'but server', value)
     *
     *        self.LogNotice('VerifySimulation -- finished')
     */
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GMGetSynchedServerState() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    return nullptr;
}

PyResult PlanetMgrBound::Handle_GMRunDepletionSim(PyCallArgs &call) {
    /*
     * 18:37:58 [ClientCallRep] GMRunDepletionSim call made to
     * 18:37:58 [PlanetDebug] PlanetMgrBound::Handle_GMRunDepletionSim() size=2
     * 18:37:58 [PlanetCallDump]   Call Arguments:
     * 18:37:58 [PlanetCallDump]       Tuple: 2 elements
     * 18:37:58 [PlanetCallDump]         [ 0] (None)
     * 18:37:58 [PlanetCallDump]         [ 1] Object:
     * 18:37:58 [PlanetCallDump]         [ 1]   Type: String: 'util.KeyVal'
     * 18:37:58 [PlanetCallDump]         [ 1]   Args: Dictionary: 2 entries
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 0] Key: String: 'totalDuration'
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 0] Value: Integer field: 12096000000000
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 1] Key: String: 'points'
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 1] Value: List: 1 elements
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 1] Value:   [ 0] Object:
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 1] Value:   [ 0]   Type: String: 'util.KeyVal'
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 1] Value:   [ 0]   Args: Dictionary: 5 entries
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 1] Value:   [ 0]   Args:   [ 0] Key: String: 'duration'
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 1] Value:   [ 0]   Args:   [ 0] Value: Integer field: 1440
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 1] Value:   [ 0]   Args:   [ 1] Key: String: 'amount'
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 1] Value:   [ 0]   Args:   [ 1] Value: Integer field: 500
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 1] Value:   [ 0]   Args:   [ 2] Key: String: 'longitude'
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 1] Value:   [ 0]   Args:   [ 2] Value: Real field: 2.646786
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 1] Value:   [ 0]   Args:   [ 3] Key: String: 'headRadius'
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 1] Value:   [ 0]   Args:   [ 3] Value: Real field: 0.050000
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 1] Value:   [ 0]   Args:   [ 4] Key: String: 'latitude'
     * 18:37:58 [PlanetCallDump]         [ 1]   Args:   [ 1] Value:   [ 0]   Args:   [ 4] Value: Real field: 1.104679
     */
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GMRunDepletionSim() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    return nullptr;
}
