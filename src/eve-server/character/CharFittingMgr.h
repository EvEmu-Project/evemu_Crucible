/*
 *
 *
 */


#ifndef EVE_CHARACTER_FITTING_MGR_H
#define EVE_CHARACTER_FITTING_MGR_H

#include "PyService.h"

class CharFittingMgr: public PyService
{
public:
    CharFittingMgr(PyServiceMgr *mgr);
    virtual ~CharFittingMgr();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;
    
    PyCallable_DECL_CALL(GetFittings);
    PyCallable_DECL_CALL(SaveFitting);
    PyCallable_DECL_CALL(SaveManyFittings);
    PyCallable_DECL_CALL(DeleteFitting);
    PyCallable_DECL_CALL(UpdateNameAndDescription);

};

#endif  // EVE_CHARACTER_FITTING_MGR_H