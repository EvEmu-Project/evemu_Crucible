
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

#include "encounterSpawnServer.h"

encounterSpawnServer::encounterSpawnServer() :
    Service("encounterSpawnServer")
{
    this->Add("GetMyEncounters", &encounterSpawnServer::GetMyEncounters);
    this->Add("RequestActivateEncounters", &encounterSpawnServer::RequestActivateEncounters);
    this->Add("RequestDeactivateEncounters", &encounterSpawnServer::RequestDeactivateEncounters);
}

PyResult encounterSpawnServer::GetMyEncounters (PyCallArgs& call)
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
    _log(QATOOLS__CALL,  "encounterSpawnServer::Handle_GetMyEncounters size: %li", call.tuple->size());
    call.Dump(QATOOLS__DUMP);

    return nullptr;
}

PyResult encounterSpawnServer::RequestActivateEncounters(PyCallArgs& call, PyList* encounterList)
{
    //    logResults = self.encounterSpawnServer.RequestActivateEncounters(encounterList, logResults=True)
    _log(QATOOLS__CALL,  "encounterSpawnServer::Handle_RequestActivateEncounters size: %li", call.tuple->size());
    call.Dump(QATOOLS__DUMP);

    return nullptr;
}

PyResult encounterSpawnServer::RequestDeactivateEncounters(PyCallArgs& call, PyList* encounterList)
{
    //  logResults = self.encounterSpawnServer.RequestDeactivateEncounters(encounterList, logResults=True)
    _log(QATOOLS__CALL,  "encounterSpawnServer::Handle_RequestDeactivateEncounters size: %li", call.tuple->size());
    call.Dump(QATOOLS__DUMP);

    return nullptr;
}

