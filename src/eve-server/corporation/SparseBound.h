

#include "../eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "corporation/CorporationDB.h"


class SparseBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(SparseBound)

    SparseBound(PyServiceMgr *mgr, CorporationDB& db, uint32 corpID);

    virtual ~SparseBound() {delete m_dispatch;}
    virtual void Release() {
        delete this;
    }

    PyCallable_DECL_CALL(Fetch);
    PyCallable_DECL_CALL(SelectByUniqueColumnValues);
    // these may not be used in this version...
    PyCallable_DECL_CALL(FetchByKey); //([keys])
    PyCallable_DECL_CALL(GetByKey); //(key)


protected:
    Dispatcher *const m_dispatch;

    CorporationDB& m_db;
    uint32 m_corpID;
};