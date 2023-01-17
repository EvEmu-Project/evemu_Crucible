
 /**
  * @name encounterSpawnServer.h
  *     *not sure yet* system for EVEmu
  *
  * @Author:        Allan
  * @date:          13 December 2018
  *
  */



#ifndef _EVEMU_QATOOLS_SPAWN_SERVER_H
#define _EVEMU_QATOOLS_SPAWN_SERVER_H

#include "services/Service.h"

class encounterSpawnServer : public Service<encounterSpawnServer>
{
public:
    encounterSpawnServer();

protected:
    PyResult GetMyEncounters(PyCallArgs& call);
    PyResult RequestActivateEncounters(PyCallArgs& call, PyList* encounterList);
    PyResult RequestDeactivateEncounters(PyCallArgs& call, PyList* encounterList);
};

#endif  // _EVEMU_QATOOLS_SPAWN_SERVER_H

