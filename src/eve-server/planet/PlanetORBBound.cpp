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

#include "EntityList.h"
#include "Client.h"
#include "packets/Planet.h"
#include "planet/PlanetORBBound.h"
#include "planet/CustomsOffice.h"
#include "system/SystemManager.h"

PlanetORB::PlanetORB(EVEServiceManager& mgr) :
    BindableService("planetOrbitalRegistryBroker", mgr)
{
}

BoundDispatcher* PlanetORB::BindObject(Client* client, PyRep* bindParameters) {
    if (bindParameters->IsInt() == false) {
        throw CustomError("Cannot bind service");
    }

    uint32 systemID = bindParameters->AsInt()->value();
    auto it = this->m_instances.find (systemID);

    if (it != this->m_instances.end ())
        return it->second;

    PlanetORBBound* bound = new PlanetORBBound(this->GetServiceManager(), *this, systemID);

    this->m_instances.insert_or_assign (systemID, bound);

    return bound;
}

void PlanetORB::BoundReleased (PlanetORBBound* bound) {

}
PlanetORBBound::PlanetORBBound(EVEServiceManager& mgr, PlanetORB& parent, uint32 systemID) :
    EVEBoundObject(mgr, parent),
    m_systemID(systemID)
{
}

PyResult PlanetORBBound::GetTaxRate(PyCallArgs& call, PyInt* itemID)
{
    //  taxRate = moniker.GetPlanetOrbitalRegistry(session.solarsystemid).GetTaxRate(itemID)
    // NOTE:  "return PyNone()" = access denied to customs office.
    CustomsSE* pCOSE = sEntityList.FindOrBootSystem(m_systemID)->GetSE(itemID->value())->GetCOSE();

    /** @todo  there's more to this...check for standings, alliance  */
    if (IsPlayerCorp(pCOSE->GetOwnerID()))
        if (call.client->GetCorporationID() != pCOSE->GetOwnerID())
            return PyStatic.NewNone();

    return new PyFloat(pCOSE->GetTaxRate(call.client));
}

PyResult PlanetORBBound::GetSettingsInfo(PyCallArgs& call, PyInt* orbitalID)
{
    /*   self.orbitalData = self.remoteOrbitalRegistry.GetSettingsInfo(self.orbitalID)  << for customs offices
     *   self.selectedHour, self.taxRateValues, self.standingLevel, self.allowAlliance, self.allowStandings = self.orbitalData
     */
    CustomsSE* pCOSE = sEntityList.FindOrBootSystem(m_systemID)->GetSE(orbitalID->value())->GetCOSE();
    return pCOSE->GetSettingsInfo();
}

PyResult PlanetORBBound::UpdateSettings(PyCallArgs& call, PyInt* orbitalID, PyInt* reinforceValue, PyObject* taxRateValues, PyFloat* standingValue, PyBool* allowAllianceValue, PyBool* allowStandingsValue)
{
    //remoteOrbitalRegistry.UpdateSettings(self.orbitalID, reinforceValue, taxRateValues, standingValue, allowAllianceValue, allowStandingsValue)
    _log(INV__MESSAGE, "Calling PlanetORBBound::UpdateSettings()");
    call.Dump(PLANET__DUMP);

    PyDict* input = taxRateValues->arguments()->AsDict();
    Call_TaxRateValuesDict dict;
    if (!dict.Decode(input)) {
        codelog(SERVICE__ERROR, "PlanetORBBound: Failed to decode arguments.");
        return nullptr;
    }

    CustomsSE* pCOSE = sEntityList.FindOrBootSystem(m_systemID)->GetSE(orbitalID->value())->GetCOSE();
    pCOSE->UpdateSettings(reinforceValue->value(), standingValue->value(), allowAllianceValue->value(), allowStandingsValue->value(), dict);

    return nullptr;
}

PyResult PlanetORBBound::GMChangeSpaceObjectOwner(PyCallArgs& call, PyInt* itemID, PyInt* corpID)
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
    _log(PLANET__DEBUG, "PlanetORBBound::Handle_GMChangeSpaceObjectOwner - size=%li", call.tuple->size());
    call.Dump(PLANET__DUMP);

    return PyStatic.NewNone();
}


/**
    def ConfigureOrbital(self, item):
        sm.GetService('planetUI').OpenConfigureWindow(item)

        */
