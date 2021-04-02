/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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

#include "EntityList.h"
#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "packets/Planet.h"
#include "planet/PlanetORBBound.h"
#include "planet/CustomsOffice.h"
#include "system/SystemManager.h"

class PlanetORBBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(PlanetORBBound)

    PlanetORBBound(PyServiceMgr *mgr, uint32 systemID)
    : PyBoundObject(mgr),
    m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "PlanetORBBound";

        PyCallable_REG_CALL(PlanetORBBound, GetTaxRate);
        PyCallable_REG_CALL(PlanetORBBound, UpdateSettings);
        PyCallable_REG_CALL(PlanetORBBound, GetSettingsInfo);
        PyCallable_REG_CALL(PlanetORBBound, GMChangeSpaceObjectOwner);

        m_systemID = systemID;
    }
    virtual ~PlanetORBBound() { delete m_dispatch; }
    virtual void Release() {
        //He hates this statement
        delete this;
    }

    PyCallable_DECL_CALL(GetTaxRate);
    PyCallable_DECL_CALL(UpdateSettings);
    PyCallable_DECL_CALL(GetSettingsInfo);
    PyCallable_DECL_CALL(GMChangeSpaceObjectOwner);

protected:
    Dispatcher* const m_dispatch;
    PlanetDB* m_db;

private:
    uint32 m_systemID;
};

PyCallable_Make_InnerDispatcher(PlanetORB)


PlanetORB::PlanetORB(PyServiceMgr *mgr)
: PyService(mgr, "planetOrbitalRegistryBroker"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    //PyCallable_REG_CALL(PlanetORB, );
    //PyCallable_REG_CALL(PlanetORB, );
}

PlanetORB::~PlanetORB() {
    delete m_dispatch;
}

PyBoundObject* PlanetORB::CreateBoundObject(Client *pClient, const PyRep *bind_args) {
    _log(PLANET__INFO, "PlanetORB bind request for:");  // sends systemID in request
    bind_args->Dump(PLANET__INFO, "    ");
    if(!bind_args->IsInt()) {
        codelog(SERVICE__ERROR, "%s Service: invalid bind argument type %s", GetName(), bind_args->TypeString());
        return NULL;
    }

    return new PlanetORBBound(m_manager, bind_args->AsInt()->value());
}

PyResult PlanetORBBound::Handle_GetTaxRate( PyCallArgs& call )
{
    //  taxRate = moniker.GetPlanetOrbitalRegistry(session.solarsystemid).GetTaxRate(itemID)
    // NOTE:  "return PyNone()" = access denied to customs office.

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    CustomsSE* pCOSE = sEntityList.FindOrBootSystem(m_systemID)->GetSE(args.arg)->GetCOSE();

    /** @todo  there's more to this...check for standings, alliance  */
    if (IsPlayerCorp(pCOSE->GetOwnerID()))
        if (call.client->GetCorporationID() != pCOSE->GetOwnerID())
            return PyStatic.NewNone();

    return new PyFloat(pCOSE->GetTaxRate(call.client));
}

PyResult PlanetORBBound::Handle_GetSettingsInfo( PyCallArgs& call )
{
    /*   self.orbitalData = self.remoteOrbitalRegistry.GetSettingsInfo(self.orbitalID)  << for customs offices
     *   self.selectedHour, self.taxRateValues, self.standingLevel, self.allowAlliance, self.allowStandings = self.orbitalData
     */
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    CustomsSE* pCOSE = sEntityList.FindOrBootSystem(m_systemID)->GetSE(args.arg)->GetCOSE();
    return pCOSE->GetSettingsInfo();
}

PyResult PlanetORBBound::Handle_UpdateSettings( PyCallArgs& call )
{
    //remoteOrbitalRegistry.UpdateSettings(self.orbitalID, reinforceValue, taxRateValues, standingValue, allowAllianceValue, allowStandingsValue)
    _log(INV__MESSAGE, "Calling PlanetORBBound::UpdateSettings()");
    call.Dump(PLANET__DUMP);

    Call_UpdateSettings args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    PyDict* input = args.taxRateValues->AsObject()->arguments()->AsDict();
    Call_TaxRateValuesDict dict;
    if (!dict.Decode(input)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    CustomsSE* pCOSE = sEntityList.FindOrBootSystem(m_systemID)->GetSE(args.orbitalID)->GetCOSE();
    pCOSE->UpdateSettings(args.reinforceValue, args.standingValue, args.allowAlliance, args.allowStandings, dict);

    return nullptr;
}

PyResult PlanetORBBound::Handle_GMChangeSpaceObjectOwner( PyCallArgs& call )
{
    // this is called when taking ownership of control tower
    // sends itemID, corpID
    /*
                state = slimItem.orbitalState
                if state in (entities.STATE_UNANCHORING,
                 entities.STATE_ONLINING,
                 entities.STATE_ANCHORING,
                 entities.STATE_OPERATING,
                 entities.STATE_OFFLINING,
                 entities.STATE_SHIELD_REINFORCE):
                    stateText = pos.DISPLAY_NAMES[pos.Entity2DB(state)]
                    gm.append(('End orbital state change (%s)' % stateText, self.CompleteOrbitalStateChange, (itemID,)))
                elif state == entities.STATE_ANCHORED:
                    upgradeType = sm.GetService('godma').GetTypeAttribute2(slimItem.typeID, const.attributeConstructionType)
                    if upgradeType is not None:
                        gm.append(('Upgrade to %s' % cfg.invtypes.Get(upgradeType).typeName, self.GMUpgradeOrbital, (itemID,)))
                gm.append(('GM: Take Control', self.TakeOrbitalOwnership, (itemID, slimItem.planetID)))

    def TakeOrbitalOwnership(self, itemID, planetID):
        registry = moniker.GetPlanetOrbitalRegistry(session.solarsystemid)
        registry.GMChangeSpaceObjectOwner(itemID, session.corpid)
    */
    _log(PLANET__DEBUG, "PlanetORBBound::Handle_GMChangeSpaceObjectOwner - size %u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    return PyStatic.NewNone();
}


/**
    def ConfigureOrbital(self, item):
        sm.GetService('planetUI').OpenConfigureWindow(item)

        */
