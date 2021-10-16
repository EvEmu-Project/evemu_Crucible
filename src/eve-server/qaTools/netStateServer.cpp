
 /**
  * @name netStateServer.h
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

#include "netStateServer.h"


PyCallable_Make_InnerDispatcher(netStateServer);

netStateServer::netStateServer(PyServiceMgr* mgr)
: PyService(mgr, "netStateServer"),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    //PyCallable_REG_CALL(DungeonService, IsObjectLocked);

}

netStateServer::~netStateServer()
{
    delete m_dispatch;

}


/*
 * PyResult encounterSpawnServer::Handle_( PyCallArgs& call )
 * {
 *    //
 *    _log(QATOOLS__CALL,  "encounterSpawnServer::Handle_ size: %li", call.tuple->size());
 *    call.Dump(QATOOLS__DUMP);
 *
 *    return nullptr;
 * }
 */