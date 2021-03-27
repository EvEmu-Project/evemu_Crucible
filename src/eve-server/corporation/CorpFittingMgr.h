/*
 *
 */

#ifndef EVE_COPORATION_FITTING_MGR_H
#define EVE_COPORATION_FITTING_MGR_H

#include "PyService.h"

class CorpFittingMgr: public PyService
{
public:
    CorpFittingMgr(PyServiceMgr *mgr);
    virtual ~CorpFittingMgr();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(GetFittings);
    PyCallable_DECL_CALL(SaveFitting);
    PyCallable_DECL_CALL(SaveManyFittings);
    PyCallable_DECL_CALL(DeleteFitting);
    PyCallable_DECL_CALL(UpdateNameAndDescription);

};

#endif  // EVE_COPORATION_FITTING_MGR_H
