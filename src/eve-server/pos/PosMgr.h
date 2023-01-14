
/**
 * @name PosMgr.h
 *   Class for pos manager calls.
 *
 * @Author:         Allan
 * @date:   28 December 17
 */

#ifndef __POSMGRSVC_SERVICE_H_INCL__
#define __POSMGRSVC_SERVICE_H_INCL__

#include "services/BoundService.h"
#include "pos/PosMgrDB.h"

class PosMgr : public BindableService <PosMgr>
{
public:
    PosMgr(EVEServiceManager& mgr);

protected:
    PosMgrDB m_db;

    PyResult GetJumpArrays(PyCallArgs& call);
    PyResult GetControlTowers(PyCallArgs& call);
    PyResult GetControlTowerFuelRequirements(PyCallArgs& call);

    //PyCallable_DECL_CALL()

    //overloaded in order to support bound objects:
    BoundDispatcher* BindObject(Client* client, PyRep* bindParameters);
};

class PosMgrBound : public EVEBoundObject <PosMgrBound>
{
public:
    PosMgrBound(EVEServiceManager& mgr, PyRep* bindData, uint32 systemID);

protected:
    bool CanClientCall(Client* client) override;

    PyResult SetTowerPassword(PyCallArgs& call, PyInt* itemID, PyRep* password);
    PyResult SetTowerPassword(PyCallArgs& call, PyInt* itemID, PyRep* password, PyBool* allowCorp, PyBool* allowAlliance);
    PyResult SetShipPassword(PyCallArgs& call, PyWString* password);
    PyResult GetMoonForTower(PyCallArgs& call, PyInt* itemID);
    PyResult GetSiloCapacityByItemID(PyCallArgs& call, PyInt* itemID);
    PyResult AnchorOrbital(PyCallArgs& call, PyInt* itemID);
    PyResult UnanchorOrbital(PyCallArgs& call, PyInt* itemID);
    PyResult OnlineOrbital(PyCallArgs& call, PyInt* itemID);
    PyResult GMUpgradeOrbital(PyCallArgs& call, PyInt* itemID);
    PyResult AnchorStructure(PyCallArgs& call, PyInt* structureID, PyTuple* position);
    PyResult UnanchorStructure(PyCallArgs& call, PyInt* itemID);
    PyResult AssumeStructureControl(PyCallArgs& call, PyInt* itemID);
    PyResult RelinquishStructureControl(PyCallArgs& call, PyInt* itemID);
    PyResult ChangeStructureProvisionType(PyCallArgs& call, PyInt* towerID, PyInt* itemID, PyInt* typeID);
    PyResult CompleteOrbitalStateChange(PyCallArgs& call, PyInt* itemID);
    PyResult GetMoonProcessInfoForTower(PyCallArgs& call, PyInt* itemID);
    PyResult LinkResourceForTower(PyCallArgs& call, PyInt* itemID, PyList* connections);
    PyResult RunMoonProcessCycleforTower(PyCallArgs& call, PyInt* itemID);
    PyResult GetStarbasePermissions(PyCallArgs& call, PyInt* itemID);
    PyResult SetStarbasePermissions(PyCallArgs& call, PyInt* itemID, PyObject* deployFlags, PyObject* usageFlagsList);
    PyResult GetTowerNotificationSettings(PyCallArgs& call, PyInt* itemID);
    PyResult SetTowerNotifications(PyCallArgs& call, PyInt* itemID, PyBool* showInCalendar, PyBool* sendFuelNotifications);
    PyResult GetTowerSentrySettings(PyCallArgs& call, PyInt* itemID);
    PyResult SetTowerSentrySettings(PyCallArgs& call, PyInt* itemID, PyFloat* standing, PyFloat* status, PyBool* statusDrop, PyBool* corpWar, PyBool* useAllianceStandings);
    PyResult InstallJumpBridgeLink(PyCallArgs& call, PyInt* localItemID, PyInt* remoteSolarSystemID, PyInt* remoteItemID);
    PyResult UninstallJumpBridgeLink(PyCallArgs& call, PyInt* itemID);

private:
    PosMgrDB m_db;
    uint32 m_systemID;
};


#endif