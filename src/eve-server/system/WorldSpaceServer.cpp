
/**
 * @name WorldSpaceServer.cpp
 *   Specific Class for
 *
 * @Author:         Allan
 * @date:   31August17
 */

#include "eve-server.h"

#include "ServiceDB.h"

#include "system/WorldSpaceServer.h"

WorldSpaceServer::WorldSpaceServer() : Service("worldSpaceServer") {
  this->Add("GetWorldSpaceTypeIDFromWorldSpaceID",
            &WorldSpaceServer::GetWorldSpaceTypeIDFromWorldSpaceID);
  this->Add("GetWorldSpaceMachoAddress",
            &WorldSpaceServer::GetWorldSpaceMachoAddress);

  /*
      ws = world.GetWorldSpace(worldSpaceTypeID)
      return ws.GetDistrictID()


      currentRevs =
     sm.GetService('jessicaWorldSpaceClient').GetWorldSpace(self.id).GetWorldSpaceSpawnRevisionsList()
      */
}

PyResult
WorldSpaceServer::GetWorldSpaceTypeIDFromWorldSpaceID(PyCallArgs &call,
                                                      PyInt *worldSpaceID) {
  if (worldSpaceID == nullptr)
    return new PyInt(0);

  return new PyInt(ServiceDB::GetSceneIDForStation(worldSpaceID->value()));
}

PyResult WorldSpaceServer::GetWorldSpaceMachoAddress(PyCallArgs &call,
                                                     PyString *address) {
  // A complete captainsQuartersSvc is not registered in this server.
  return PyStatic.NewNone();
}
