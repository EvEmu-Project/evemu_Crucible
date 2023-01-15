

#include "../eve-server.h"

#include "services/BoundService.h"

#include "corporation/CorporationDB.h"
#include "Client.h"

class SparseBound : public EVEBoundObject <SparseBound>
{
public:
    SparseBound(EVEServiceManager &mgr, CorporationDB& db, uint32 corpID);

protected:
    bool CanClientCall(Client* client) override;
    PyResult Fetch(PyCallArgs& call, PyInt* startPos, PyInt* fetchSize);
    PyResult SelectByUniqueColumnValues(PyCallArgs& call, PyRep* columnName, PyList* values);
    // these may not be used in this version...
    PyResult FetchByKey(PyCallArgs& call, PyList* keys);
    PyResult GetByKey(PyCallArgs& call, PyInt* keys);

protected:
    CorporationDB& m_db;
    uint32 m_corpID;
};