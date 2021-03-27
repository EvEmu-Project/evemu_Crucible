
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


#include "PyService.h"


class zActionServer
: public PyService
{
public:
    zActionServer(PyServiceMgr *mgr);
    ~zActionServer();


protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(RequestActionStart);
    PyCallable_DECL_CALL(QA_RequestForceActionStart);

};


#endif  // _EVEMU_QATOOLS_ZACTION_SERVER_H


