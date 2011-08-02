#include "EVEServerPCH.h"

// crap
PyCallable_Make_InnerDispatcher(ContractProxyService)

ContractProxyService::ContractProxyService( PyServiceMgr *mgr ) 
: PyService(mgr, "contractProxy"), 
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(ContractProxyService, GetLoginInfo)
	PyCallable_REG_CALL(ContractProxyService, GetMyExpiredContractList)
}

ContractProxyService::~ContractProxyService()
{
    delete m_dispatch;
}

PyResult ContractProxyService::Handle_GetMyExpiredContractList(PyCallArgs &call) {
	sLog.Debug("Server", "Called GetMyExpiredContractList Stub.");

	return NULL;
}

PyResult ContractProxyService::Handle_GetLoginInfo(PyCallArgs &call)
{
    // currently a stub as I need to redesign or change some sub systems for this.

    PyDict* args = new PyDict;

    /* create needsAttention row descriptor */
    DBRowDescriptor *needsAttentionHeader = new DBRowDescriptor();
    needsAttentionHeader->AddColumn( "contractID",   DBTYPE_I4);
    needsAttentionHeader->AddColumn( "",             DBTYPE_I4);
    CRowSet *needsAttention_rowset = new CRowSet( &needsAttentionHeader );

    /* create inProgress row descriptor */
    DBRowDescriptor *inProgressHeader = new DBRowDescriptor();
    inProgressHeader->AddColumn( "contractID",      DBTYPE_I4 );
    inProgressHeader->AddColumn( "startStationID",  DBTYPE_I4 );
    inProgressHeader->AddColumn( "endStationID",    DBTYPE_I4 );
    inProgressHeader->AddColumn( "expires",         DBTYPE_FILETIME );
    CRowSet *inProgress_rowset = new CRowSet( &inProgressHeader );

    /* create assignedToMe row descriptor */
    DBRowDescriptor *assignedToMeHeader = new DBRowDescriptor();
    assignedToMeHeader->AddColumn( "contractID",    DBTYPE_I4);
    assignedToMeHeader->AddColumn( "issuerID",      DBTYPE_I4);
    CRowSet *assignedToMe_rowset = new CRowSet( &assignedToMeHeader );

    args->SetItemString( "needsAttention",          needsAttention_rowset );
    args->SetItemString( "inProgress",              inProgress_rowset );
    args->SetItemString( "assignedToMe",            assignedToMe_rowset );

    PyObject* res = new PyObject( "util.KeyVal", args );

    return res;
}

/* Description
 * GetLoginInfo sends back a util.KeyVal PyClass. This class contains 3 entries, those entries are
 * "needsAttention", "inProgress" and "assignedToMe".
 *
 * ------------------
 * assignedToMe
 * ------------------
 * assignedToMe is a pretty obvious packet, it contains rows of userid's and contractid's
 * that are belonging to the player.
 *
 * ------------------
 * inProgress
 * ------------------
 * inProgress is also pretty obvious, it contains rows of contractID, startStationID, endStationID and expires
 * that marks what contract is in progress.
 *
 * ------------------
 * needsAttention
 * ------------------
 * needsAttention contains contracts that need attention, its rather a way to make the journal blink. The
 * contracts off course are highlighted in the journal.
 *
        PyClass
          PyString:"util.KeyVal"
          PyDict:3
            dict["needsAttention"]=PyClass
              PyTuple:2
                itr[0]:PyTuple:1
                  itr[0]:PyClass
                    PyString:"dbutil.CRowset"
                itr[1]:PyDict:1
                  dict["header"]=PyClass
                    PyTuple:2
                      itr[0]:PyClass
                        PyString:"blue.DBRowDescriptor"
                      itr[1]:PyTuple:1
                        itr[0]:PyTuple:2
                          itr[0]:PyTuple:2
                            itr[0]:"contractID"
                            itr[1]:0x3
                          itr[1]:PyTuple:2
                            itr[0]:""
                            itr[1]:0x3
            dict["inProgress"]=PyClass
              PyTuple:2
                itr[0]:PyTuple:1
                  itr[0]:PyClass
                    PyString:"dbutil.CRowset"
                itr[1]:PyDict:1
                  dict["header"]=PyClass
                    PyTuple:2
                      itr[0]:PyClass
                        PyString:"blue.DBRowDescriptor"
                      itr[1]:PyTuple:1
                        itr[0]:PyTuple:4
                          itr[0]:PyTuple:2
                            itr[0]:"contractID"
                            itr[1]:0x3
                          itr[1]:PyTuple:2
                            itr[0]:"startStationID"
                            itr[1]:0x3
                          itr[2]:PyTuple:2
                            itr[0]:"endStationID"
                            itr[1]:0x3
                          itr[3]:PyTuple:2
                            itr[0]:"expires"
                            itr[1]:0x40
            dict["assignedToMe"]=PyClass
              PyTuple:2
                itr[0]:PyTuple:1
                  itr[0]:PyClass
                    PyString:"dbutil.CRowset"
                itr[1]:PyDict:1
                  dict["header"]=PyClass
                    PyTuple:2
                      itr[0]:PyClass
                        PyString:"blue.DBRowDescriptor"
                      itr[1]:PyTuple:1
                        itr[0]:PyTuple:2
                          itr[0]:PyTuple:2
                            itr[0]:"contractID"
                            itr[1]:0x3
                          itr[1]:PyTuple:2
                            itr[0]:"issuerID"
                            itr[1]:0x3
*/
