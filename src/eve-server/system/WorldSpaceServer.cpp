
/**
 * @name WorldSpaceServer.cpp
 *   Specific Class for
 *
 * @Author:         Allan
 * @date:   31August17
 */


#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "system/WorldSpaceServer.h"


PyCallable_Make_InnerDispatcher(WorldSpaceServer)

WorldSpaceServer::WorldSpaceServer(PyServiceMgr *mgr)
: PyService(mgr, "worldSpaceServer"),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(WorldSpaceServer, GetWorldSpaceTypeIDFromWorldSpaceID);
    PyCallable_REG_CALL(WorldSpaceServer, GetWorldSpaceMachoAddress);

    /*
        ws = world.GetWorldSpace(worldSpaceTypeID)
        return ws.GetDistrictID()


        currentRevs = sm.GetService('jessicaWorldSpaceClient').GetWorldSpace(self.id).GetWorldSpaceSpawnRevisionsList()
        */
}

WorldSpaceServer::~WorldSpaceServer() {
    delete m_dispatch;
}


PyResult WorldSpaceServer::Handle_GetWorldSpaceTypeIDFromWorldSpaceID(PyCallArgs &call) {
    /**
     *        worldSpaceTypeID = self.GetWorldSpaceTypeIDFromWorldSpaceID(worldSpaceID)
     */
    sLog.White( "WorldSpaceServer::Handle_GetWorldSpaceTypeIDFromWorldSpaceID()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return PyStatic.NewNone();
}

PyResult WorldSpaceServer::Handle_GetWorldSpaceMachoAddress(PyCallArgs &call) {
    /**
     *       service, address = wss.GetWorldSpaceMachoAddress(address)
     */
    sLog.White( "WorldSpaceServer::Handle_GetWorldSpaceMachoAddress()", "size=%lu", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return PyStatic.NewNone();
}

