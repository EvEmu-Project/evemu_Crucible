
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


#include "PyService.h"


class encounterSpawnServer
: public PyService
{
public:
    encounterSpawnServer(PyServiceMgr *mgr);
    ~encounterSpawnServer();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(GetMyEncounters);
    PyCallable_DECL_CALL(RequestActivateEncounters);
    PyCallable_DECL_CALL(RequestDeactivateEncounters);

};


#endif  // _EVEMU_QATOOLS_SPAWN_SERVER_H

