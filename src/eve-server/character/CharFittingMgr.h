/*
 *
 *
 */

#ifndef EVE_CHARACTER_FITTING_MGR_H
#define EVE_CHARACTER_FITTING_MGR_H

#include "services/Service.h"

class CharFittingMgr : public Service <CharFittingMgr>
{
public:
    CharFittingMgr();

protected:
    PyResult GetFittings(PyCallArgs& call, PyInt* ownerID);
    PyResult SaveFitting(PyCallArgs& call, PyInt* ownerID, PyObject* fitting);
    PyResult SaveManyFittings(PyCallArgs& call, PyInt* ownerID, PyDict* fittingsToSave);
    PyResult DeleteFitting(PyCallArgs& call, PyInt* ownerID, PyInt* fittingID);
    PyResult UpdateNameAndDescription(PyCallArgs& call, PyInt* fittingID, PyInt* ownerID, PyWString* name, PyWString* description);

    PyCallable_DECL_CALL(GetFittings);
    PyCallable_DECL_CALL(SaveFitting);
    PyCallable_DECL_CALL(SaveManyFittings);
    PyCallable_DECL_CALL(DeleteFitting);
    PyCallable_DECL_CALL(UpdateNameAndDescription);

};

#endif  // EVE_CHARACTER_FITTING_MGR_H