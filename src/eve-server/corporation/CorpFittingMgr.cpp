/*
 *
 *
 * note:  these will be same calls as in charFittingMgr
 */

//work in progress


#include "eve-server.h"


#include "corporation/CorpFittingMgr.h"

CorpFittingMgr::CorpFittingMgr() :
    Service("corpFittingMgr")
{
    this->Add("GetFittings", &CorpFittingMgr::GetFittings);
    this->Add("SaveFitting", &CorpFittingMgr::SaveFitting);
    this->Add("SaveManyFittings", &CorpFittingMgr::SaveManyFittings);
    this->Add("DeleteFitting", &CorpFittingMgr::DeleteFitting);
    this->Add("UpdateNameAndDescription", &CorpFittingMgr::UpdateNameAndDescription);
}

PyResult CorpFittingMgr::GetFittings(PyCallArgs &call, PyInt* ownerID)
{
    //self.fittings[ownerID] = self.GetFittingMgr(ownerID).GetFittings(ownerID)
    _log(CORP__CALL, "CorpFittingMgr::Handle_GetFittings()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpFittingMgr::SaveFitting(PyCallArgs &call, PyInt* ownerID, PyObject* fitting)
{
    //    fitting.ownerID = ownerID
    //    fitting.fittingID = self.GetFittingMgr(ownerID).SaveFitting(ownerID, fitting)
    _log(CORP__CALL, "CorpFittingMgr::Handle_SaveFitting()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpFittingMgr::SaveManyFittings(PyCallArgs &call, PyInt* ownerID, PyDict* fittingsToSave)
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

PyResult CorpFittingMgr::DeleteFitting(PyCallArgs &call, PyInt* ownerID, PyInt* fittingID)
{
    // self.GetFittingMgr(ownerID).DeleteFitting(ownerID, fittingID)
    _log(CORP__CALL, "CorpFittingMgr::Handle_DeleteFitting()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpFittingMgr::UpdateNameAndDescription(PyCallArgs &call, PyInt* fittingID, PyInt* ownerID, PyWString* name, PyWString* description)
{
    // self.GetFittingMgr(ownerID).UpdateNameAndDescription(fittingID, ownerID, name, description)
    _log(CORP__CALL, "CorpFittingMgr::Handle_UpdateNameAndDescription()");
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

