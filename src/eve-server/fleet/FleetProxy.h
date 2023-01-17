
 /**
  * @name FleetProxy.h
  *     Fleet Proxy code for EVEmu
  *
  * @Author:        Allan
  * @date:          05 August 2014 (original skeleton outline)
  * @update:        21 November 2017 (begin actual implementation)
  *
  */

#ifndef EVEMU_SRC_FLEET_PROXY_H_
#define EVEMU_SRC_FLEET_PROXY_H_

#include "services/Service.h"
#include "fleet/FleetManager.h"

class FleetProxy : public Service <FleetProxy>
{
public:
    FleetProxy();

protected:
    PyResult GetAvailableFleets(PyCallArgs& call);
    PyResult ApplyToJoinFleet(PyCallArgs& call, PyInt* fleetID);
    PyResult AddFleetFinderAdvert(PyCallArgs& call, PyObject* info);
    PyResult RemoveFleetFinderAdvert(PyCallArgs& call);
    PyResult GetMyFleetFinderAdvert(PyCallArgs& call);
    PyResult UpdateAdvertInfo(PyCallArgs& call, PyInt* numMembers);
};

#endif  // EVEMU_SRC_FLEET_PROXY_H_
