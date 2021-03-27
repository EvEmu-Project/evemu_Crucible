
 /**
  * @name encounterSpawnServer.cpp
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

#include "encounterSpawnServer.h"



PyCallable_Make_InnerDispatcher(encounterSpawnServer);

encounterSpawnServer::encounterSpawnServer(PyServiceMgr *mgr)
: PyService(mgr, "encounterSpawnServer"),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(encounterSpawnServer, GetMyEncounters);
    PyCallable_REG_CALL(encounterSpawnServer, RequestActivateEncounters);
    PyCallable_REG_CALL(encounterSpawnServer, RequestDeactivateEncounters);

}


encounterSpawnServer::~encounterSpawnServer() {
    delete m_dispatch;
}


PyResult encounterSpawnServer::Handle_GetMyEncounters( PyCallArgs& call )
{
    /*
    encounters = self.encounterSpawnServer.GetMyEncounters()
    contentList = []
    for encounter in encounters:
        data = {'text': encounter['encounterName'],
            'label': encounter['encounterName'],
            'id': encounter['encounterID'],
            'encounterID': encounter['encounterID'],
            'disableToggle': True,
            'hint': encounter['encounterName']}
            */
    _log(QATOOLS__CALL,  "encounterSpawnServer::Handle_GetMyEncounters size: %u", call.tuple->size());
    call.Dump(QATOOLS__DUMP);

    return nullptr;
}

PyResult encounterSpawnServer::Handle_RequestActivateEncounters( PyCallArgs& call )
{
    //    logResults = self.encounterSpawnServer.RequestActivateEncounters(encounterList, logResults=True)
    _log(QATOOLS__CALL,  "encounterSpawnServer::Handle_RequestActivateEncounters size: %u", call.tuple->size());
    call.Dump(QATOOLS__DUMP);

    return nullptr;
}

PyResult encounterSpawnServer::Handle_RequestDeactivateEncounters( PyCallArgs& call )
{
    //  logResults = self.encounterSpawnServer.RequestDeactivateEncounters(encounterList, logResults=True)
    _log(QATOOLS__CALL,  "encounterSpawnServer::Handle_RequestDeactivateEncounters size: %u", call.tuple->size());
    call.Dump(QATOOLS__DUMP);

    return nullptr;
}

