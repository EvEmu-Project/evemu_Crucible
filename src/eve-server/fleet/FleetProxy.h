
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


#include "fleet/FleetManager.h"

class FleetProxy
 : public PyService
{
  public:
    FleetProxy(PyServiceMgr *mgr);
    ~FleetProxy();

  protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(GetAvailableFleets);
    PyCallable_DECL_CALL(AddFleetFinderAdvert);
    PyCallable_DECL_CALL(RemoveFleetFinderAdvert);
    PyCallable_DECL_CALL(ApplyToJoinFleet);
    PyCallable_DECL_CALL(GetMyFleetFinderAdvert);
    PyCallable_DECL_CALL(UpdateAdvertInfo);

protected:

};

#endif  // EVEMU_SRC_FLEET_PROXY_H_
