
 /**
  * @name FleetManager.h
  *     Fleet Manager code for EVEmu
  *
  * @Author:        Allan
  * @date:          05 August 2014 (original skeleton outline)
  * @update:        21 November 2017 (begin actual implementation)
  *
  */

#ifndef EVEMU_SRC_FLEET_MGR_H_
#define EVEMU_SRC_FLEET_MGR_H_

#include "services/Service.h"
#include "fleet/FleetService.h"

class FleetManager: public Service <FleetManager>
{
public:
    FleetManager();

protected:
    PyResult ForceLeaveFleet(PyCallArgs& call);
    PyResult GetActiveStatus(PyCallArgs& call);
    PyResult BroadcastToBubble(PyCallArgs& call, PyString* name, PyInt* groupID, PyInt* itemID);
    PyResult BroadcastToSystem(PyCallArgs& call, PyString* name, PyInt* groupID, PyInt* itemID);
    PyResult AddToWatchlist(PyCallArgs& call, PyInt* characterID, PyRep* fav);
    PyResult RemoveFromWatchlist(PyCallArgs& call, PyInt* characterID, PyRep* fav);
    PyResult RegisterForDamageUpdates(PyCallArgs& call, PyRep* fav);
};

#endif  // EVEMU_SRC_FLEET_MGR_H_
