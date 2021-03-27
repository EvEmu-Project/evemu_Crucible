/*
 *
 *
 * note:  these will be same calls as in charFittingMgr
 */

//work in progress


#include "eve-server.h"

#include "PyServiceCD.h"
#include "corporation/CorpFittingMgr.h"

PyCallable_Make_InnerDispatcher(CorpFittingMgr)

CorpFittingMgr::CorpFittingMgr(PyServiceMgr *mgr)
: PyService(mgr, "corpFittingMgr"),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CorpFittingMgr, GetFittings);
    PyCallable_REG_CALL(CorpFittingMgr, SaveFitting);
    PyCallable_REG_CALL(CorpFittingMgr, SaveManyFittings);
    PyCallable_REG_CALL(CorpFittingMgr, DeleteFitting);
    PyCallable_REG_CALL(CorpFittingMgr, UpdateNameAndDescription);
}

CorpFittingMgr::~CorpFittingMgr() {
    delete m_dispatch;
}

PyResult CorpFittingMgr::Handle_GetFittings(PyCallArgs &call)
{
    //self.fittings[ownerID] = self.GetFittingMgr(ownerID).GetFittings(ownerID)
    _log(CORP__CALL, "CorpFittingMgr::Handle_GetFittings()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpFittingMgr::Handle_SaveFitting(PyCallArgs &call)
{
    //    fitting.ownerID = ownerID
    //    fitting.fittingID = self.GetFittingMgr(ownerID).SaveFitting(ownerID, fitting)
    _log(CORP__CALL, "CorpFittingMgr::Handle_SaveFitting()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpFittingMgr::Handle_SaveManyFittings(PyCallArgs &call)
{
    /*
        newFittingIDs = self.GetFittingMgr(ownerID).SaveManyFittings(ownerID, fittingsToSave)
        for row in newFittingIDs:
            self.fittings[ownerID][row.realFittingID] = fittingsToSave[row.tempFittingID]
            self.fittings[ownerID][row.realFittingID].fittingID = row.realFittingID
        */
    _log(CORP__CALL, "CorpFittingMgr::Handle_SaveManyFittings()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpFittingMgr::Handle_DeleteFitting(PyCallArgs &call)
{
    // self.GetFittingMgr(ownerID).DeleteFitting(ownerID, fittingID)
    _log(CORP__CALL, "CorpFittingMgr::Handle_DeleteFitting()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpFittingMgr::Handle_UpdateNameAndDescription(PyCallArgs &call)
{
    // self.GetFittingMgr(ownerID).UpdateNameAndDescription(fittingID, ownerID, name, description)
    _log(CORP__CALL, "CorpFittingMgr::Handle_UpdateNameAndDescription()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

