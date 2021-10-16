

 /**
  * @name zActionServer.cpp
  *     *not sure yet* system for EVEmu
  *
  * @Author:        Allan
  * @date:          13 December 2018
  *
  */


 /*
  * # qaTools Logging:
  * QATOOLS__ERROR
  * QATOOLS__WARNING
  * QATOOLS__MESSAGE
  * QATOOLS__DEBUG
  * QATOOLS__INFO
  * QATOOLS__CALL
  * QATOOLS__DUMP
  * QATOOLS__RSPDUMP
  */




#include "eve-server.h"

#include "PyServiceCD.h"

#include "zActionServer.h"


PyCallable_Make_InnerDispatcher(zActionServer);

zActionServer::zActionServer(PyServiceMgr* mgr)
: PyService(mgr, "zActionServer"),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(zActionServer, RequestActionStart);
    PyCallable_REG_CALL(zActionServer, QA_RequestForceActionStart);
    
}

zActionServer::~zActionServer()
{
    delete m_dispatch;

}

PyResult zActionServer::Handle_RequestActionStart( PyCallArgs& call )
{
    //    requestThread = uthread.new(self.GetZactionServer().RequestActionStart, entID, actionID, interrupt, clientProps)
    _log(QATOOLS__CALL,  "zActionServer::Handle_RequestActionStart size: %li", call.tuple->size());
    call.Dump(QATOOLS__DUMP);

    return nullptr;
}

PyResult zActionServer::Handle_QA_RequestForceActionStart( PyCallArgs& call )
{
    //    requestThread = uthread.new(self.GetZactionServer().QA_RequestForceActionStart, entID, actionID)
    _log(QATOOLS__CALL,  "zActionServer::Handle_QA_RequestForceActionStart size: %li", call.tuple->size());
    call.Dump(QATOOLS__DUMP);

    return nullptr;
}
