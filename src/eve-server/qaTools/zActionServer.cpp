

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
#include "zActionServer.h"

zActionServer::zActionServer() :
    Service("zActionServer")
{
    this->Add("RequestActionStart", &zActionServer::RequestActionStart);
    this->Add("QA_RequestForceActionStart", &zActionServer::QA_RequestForceActionStart);
}

PyResult zActionServer::RequestActionStart(PyCallArgs& call, PyInt* entityID, PyInt* actionID, PyBool* interrupt, PyDict* clientProps)
{
    //    requestThread = uthread.new(self.GetZactionServer().RequestActionStart, entID, actionID, interrupt, clientProps)
    _log(QATOOLS__CALL,  "zActionServer::Handle_RequestActionStart size: %lli", call.tuple->size());
    call.Dump(QATOOLS__DUMP);

    return nullptr;
}

PyResult zActionServer::QA_RequestForceActionStart(PyCallArgs& call, PyInt* entityID, PyInt* actionID)
{
    //    requestThread = uthread.new(self.GetZactionServer().QA_RequestForceActionStart, entID, actionID)
    _log(QATOOLS__CALL,  "zActionServer::Handle_QA_RequestForceActionStart size: %lli", call.tuple->size());
    call.Dump(QATOOLS__DUMP);

    return nullptr;
}
