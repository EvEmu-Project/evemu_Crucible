
 /**
  * @name FleetObject.h
  *     Fleet Object code for EVEmu
  *
  * @Author:        Allan
  * @date:          05 August 2014 (original skeleton outline)
  * @update:        21 November 2017 (begin actual implementation)
  *
  */


#ifndef EVEMU_SRC_FLEET_OBJ_H_
#define EVEMU_SRC_FLEET_OBJ_H_

#include "services/BoundService.h"
#include "fleet/FleetManager.h"

class FleetBound;

class FleetObject : public BindableService <FleetObject, FleetBound>
{
public:
    FleetObject(EVEServiceManager& mgr);

    void BoundReleased (FleetBound* bound) override;

protected:
    PyResult CreateFleet(PyCallArgs& call);

    //overloaded in order to support bound objects:
    BoundDispatcher* BindObject(Client *client, PyRep* bindParameters) override;

private:
    std::map <uint32, FleetBound*> m_instances;

};

#endif  // EVEMU_SRC_FLEET_OBJ_H_
