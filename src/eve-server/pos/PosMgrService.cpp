/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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
    Author:        Zhur, Cometo
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "pos/PosMgrService.h"
#include "PyBoundObject.h"

class PosMgrServiceBound
        : public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(PosMgrServiceBound)

    PosMgrServiceBound(PyServiceMgr *mgr)
            : PyBoundObject(mgr),
              m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);
        m_strBoundObjectName = "PosMgrServiceBound";

        PyCallable_REG_CALL(PosMgrServiceBound, SetShipPassword)
        /* //TODO, Verify that these are all bound calls.
         * AnchorOrbital
         * AnchorStructure
         * AssumeStructureControl
         * ChangeStructureProvisionType
         * CompleteOrbitalStateChange
         * GetMoonForTower
         * GetMoonProcessInfoForTower
         * GMUpgradeOrbital
         * LinkResourcesForTower
         * OnlineOrbital
         * RelinquishStructureControl
         * RunMoonProcessCycleforTower
         * SetShipPassword
         * SetStarbasePermissions
         * SetTowerPassword
         * SetTowerNotifications
         * SetTowerSentrySettings
         * UnanchorOrbital
         */

    }
    virtual ~PosMgrServiceBound() { delete m_dispatch; }
    virtual void Release() {
        delete this;
    }

    PyCallable_DECL_CALL(SetShipPassword)

protected:
    Dispatcher *const m_dispatch;
};

PyCallable_Make_InnerDispatcher(PosMgrService)

PosMgrService::PosMgrService(PyServiceMgr *mgr)
: PyService(mgr, "posMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(PosMgrService, GetControlTowerFuelRequirements)
    /* //TODO, Verify that these are all unbound calls.
     * GetControlTowerFuelRequirements
     * GetControlTowers
     * GetJumpArrays
     * GetSiloCapacityByItemID
     */
}

PosMgrService::~PosMgrService() {
    delete m_dispatch;
}

PyBoundObject *PosMgrService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    if(!bind_args->IsInt()) {
        codelog(SERVICE__ERROR, "%s Service: invalid bind argument type %s", GetName(), bind_args->TypeString());
        return NULL;
    }
    bind_args->Dump(stdout, "PosMgrService:CBO:    ");
    return new PosMgrServiceBound(m_manager);
}

PyResult PosMgrService::Handle_GetControlTowerFuelRequirements(PyCallArgs &args) {
    return m_db.GetControlTowerFuelRequirements();
}

PyResult PosMgrServiceBound::Handle_SetShipPassword(PyCallArgs &call) {
    sLog.Debug("PosMgrServiceBound","Called SetShipPassword Stub.");
    call.tuple->Dump(stdout, "PMSB:SSP:    ");

    return  NULL;
}
