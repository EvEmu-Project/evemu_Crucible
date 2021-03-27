
/**
 * @name WorldSpaceServer.h
 *   Specific Class for
 *
 * @Author:         Allan
 * @date:   31August17
 */


#ifndef __EVE_SYSTEM_WORLDSPACE_H_
#define __EVE_SYSTEM_WORLDSPACE_H_

#include "PyService.h"

class WorldSpaceServer
: public PyService
{
public:
    WorldSpaceServer(PyServiceMgr *mgr);
    virtual ~WorldSpaceServer();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(GetWorldSpaceTypeIDFromWorldSpaceID);
    PyCallable_DECL_CALL(GetWorldSpaceMachoAddress);

    //PyCallable_DECL_CALL()

    //overloaded in order to support bound objects:
    //virtual PyBoundObject *CreateBoundObject(Client* pClient, const PyRep* bind_args);
};

#endif  // __EVE_SYSTEM_WORLDSPACE_H_