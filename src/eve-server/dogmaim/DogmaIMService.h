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
*/


#ifndef __DOGMAIM_SERVICE_H_INCL__
#define __DOGMAIM_SERVICE_H_INCL__

#include "services/BoundService.h"

class PyRep;

class DogmaIMService : public BindableService <DogmaIMService>
{
public:
    DogmaIMService(EVEServiceManager& mgr);

protected:
    PyResult GetAttributeTypes(PyCallArgs& call);

    //overloaded in order to support bound objects:
    BoundDispatcher* BindObject(Client *client, PyRep* bindParameters);

private:
    ObjCacheService* m_cache;
};


/** this is either DogmaLM (Location Manager) or DogmaIM (Instance Manager) for bound objects.
 * it depends on the object, location, and calling function
 *    i see no reason to change it at this point.
 */
class DogmaIMBound : public EVEBoundObject <DogmaIMBound>
{
public:
    DogmaIMBound(EVEServiceManager& mgr, PyRep* bindData, uint32 locationID, uint32 groupID);

protected:
    bool CanClientCall(Client* client) override;

    PyResult ChangeDroneSettings(PyCallArgs& call, PyDict* settings);
    PyResult LinkWeapons(PyCallArgs& call, PyInt* shipID, PyInt* masterID, PyInt* fromID);
    PyResult LinkAllWeapons(PyCallArgs& call, PyInt* shipID);
    PyResult UnlinkModule(PyCallArgs& call, PyInt* shipID, PyInt* moduleID);
    PyResult UnlinkAllModules(PyCallArgs& call, PyInt* shipID);
    PyResult OverloadRack(PyCallArgs& call, PyInt* itemID);
    PyResult StopOverloadRack(PyCallArgs& call, PyInt* itemID);
    PyResult CharGetInfo(PyCallArgs& call);
    PyResult ItemGetInfo(PyCallArgs& call, PyInt* itemID);
    PyResult GetAllInfo(PyCallArgs& call, PyBool* getCharInfo, PyBool* getShipInfo);
    PyResult DestroyWeaponBank(PyCallArgs& call, PyInt* shipID, PyInt* itemID);
    PyResult GetCharacterBaseAttributes(PyCallArgs& call);
    PyResult Activate(PyCallArgs& call, PyInt* itemID, PyInt* effectID);
    PyResult Activate(PyCallArgs& call, PyInt* itemID, PyWString* effectName, PyInt* target, PyInt* repeat);
    PyResult Deactivate(PyCallArgs& call, PyInt* itemID, PyInt* effect);
    PyResult Deactivate(PyCallArgs& call, PyInt* itemID, PyWString* effectName);
    PyResult Overload(PyCallArgs& call, PyInt* itemID, PyInt* effectID);
    PyResult StopOverload(PyCallArgs& call, PyInt* itemID, PyInt* effectID);
    PyResult CancelOverloading(PyCallArgs& call, PyInt* itemID);
    PyResult SetModuleOnline(PyCallArgs& call, PyInt* locationID, PyInt* moduleID);
    PyResult TakeModuleOffline(PyCallArgs& call, PyInt* locationID, PyInt* moduleID);
    PyResult LoadAmmoToBank(PyCallArgs& call, PyInt* shipID, PyInt* masterID, PyInt* chargeTypeID, PyList* cItemIDs, PyInt* chargeLocationID, PyInt* qty);
    PyResult LoadAmmoToModules(PyCallArgs& call, PyInt* shipID, PyList* cModuleIDs, PyInt* chargeTypeID, PyInt* itemID, PyInt* ammoLocationId);
    PyResult GetTargets(PyCallArgs& call);
    PyResult GetTargeters(PyCallArgs& call);
    PyResult AddTarget(PyCallArgs& call, PyInt* targetID);
    PyResult RemoveTarget(PyCallArgs& call, PyInt* targetID);
    PyResult ClearTargets(PyCallArgs& call);
    PyResult InitiateModuleRepair(PyCallArgs& call, PyInt* itemID);
    PyResult StopModuleRepair(PyCallArgs& call, PyInt* itemID);
    PyResult MergeModuleGroups(PyCallArgs& call, PyInt* shipID, PyInt* masterID, PyInt* slaveID);
    PyResult PeelAndLink(PyCallArgs& call, PyInt* shipID, PyInt* masterID, PyInt* slaveID);

    /*  OBO == ??  (pos targeting)
     * flag, targetList = self.GetDogmaLM().AddTargetOBO(sid, tid) (structureID, targetID)
     * self.GetDogmaLM().RemoveTargetOBO(sid, tid)  (structureID, targetID)
    */
private:

    uint32 m_locationID;
    uint32 m_groupID;
};

#endif
