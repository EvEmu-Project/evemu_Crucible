
 /**
  * @name netStateServer.h
  *     *not sure yet* system for EVEmu
  *
  * @Author:        Allan
  * @date:          13 December 2018
  *
  */



#ifndef _EVEMU_QATOOLS_NETSTATE_SERVER_H
#define _EVEMU_QATOOLS_NETSTATE_SERVER_H


#include "PyService.h"


class netStateServer
: public PyService
{
public:
    netStateServer(PyServiceMgr *mgr);
    ~netStateServer();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    //PyCallable_DECL_CALL(DEGetFactions);

};


#endif  // _EVEMU_QATOOLS_NETSTATE_SERVER_H



