
/**
 * @name PosMgr.h
 *   Class for pos manager calls.
 *
 * @Author:         Allan
 * @date:   28 December 17
 */

#ifndef __POSMGRSVC_SERVICE_H_INCL__
#define __POSMGRSVC_SERVICE_H_INCL__

#include "PyService.h"
#include "pos/PosMgrDB.h"

class PosMgr
: public PyService
{
public:
    PosMgr(PyServiceMgr *mgr);
    virtual ~PosMgr();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;
    PosMgrDB m_db;

    PyCallable_DECL_CALL(GetJumpArrays);
    PyCallable_DECL_CALL(GetControlTowers);
    PyCallable_DECL_CALL(GetControlTowerFuelRequirements);

    //PyCallable_DECL_CALL()

    //overloaded in order to support bound objects:
    virtual PyBoundObject *CreateBoundObject(Client* pClient, const PyRep* bind_args);
};

#endif