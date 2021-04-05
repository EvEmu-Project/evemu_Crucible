
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


#include "fleet/FleetManager.h"

class FleetObject
 : public PyService
{
public:
    FleetObject(PyServiceMgr *mgr);
    ~FleetObject();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(CreateFleet);

    //overloaded in order to support bound objects:
    virtual PyBoundObject *CreateBoundObject(Client *pClient, const PyRep *bind_args);

private:


};

#endif  // EVEMU_SRC_FLEET_OBJ_H_
