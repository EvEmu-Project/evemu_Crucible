
 /**
  * @name zActionServer.h
  *     *not sure yet* system for EVEmu
  *
  * @Author:        Allan
  * @date:          13 December 2018
  *
  */



#ifndef _EVEMU_QATOOLS_ZACTION_SERVER_H
#define _EVEMU_QATOOLS_ZACTION_SERVER_H

#include "services/Service.h"

class zActionServer : public Service <zActionServer>
{
public:
    zActionServer();

protected:
    PyResult RequestActionStart(PyCallArgs& call, PyInt* entityID, PyInt* actionID, PyBool* interrupt, PyDict* clientProps);
    PyResult QA_RequestForceActionStart(PyCallArgs& call, PyInt* entityID, PyInt* actionID);

};


#endif  // _EVEMU_QATOOLS_ZACTION_SERVER_H


