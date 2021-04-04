
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

#include "PyService.h"
#include "fleet/FleetService.h"

class FleetManager: public PyService
{
public:
    FleetManager(PyServiceMgr *mgr);
    ~FleetManager();

protected:

    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(ForceLeaveFleet);
    PyCallable_DECL_CALL(AddToWatchlist);
    PyCallable_DECL_CALL(RemoveFromWatchlist);
    PyCallable_DECL_CALL(RegisterForDamageUpdates);
    PyCallable_DECL_CALL(GetActiveStatus);
    PyCallable_DECL_CALL(BroadcastToBubble);
    PyCallable_DECL_CALL(BroadcastToSystem);

};

#endif  // EVEMU_SRC_FLEET_MGR_H_
