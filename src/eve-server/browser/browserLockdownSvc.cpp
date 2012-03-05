#include "EVEServerPCH.h"

// crap
PyCallable_Make_InnerDispatcher(BrowserLockDownService)

BrowserLockDownService::BrowserLockDownService( PyServiceMgr *mgr ) : PyService(mgr, "browserLockdownSvc"), m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(BrowserLockDownService, GetFlaggedSitesHash)
    PyCallable_REG_CALL(BrowserLockDownService, GetFlaggedSitesList)
	PyCallable_REG_CALL(BrowserLockDownService, GetDefaultHomePage)
}

BrowserLockDownService::~BrowserLockDownService() {
    delete m_dispatch;
}

PyObject * GenerateLockDownCachedObject()
{
/*
PyString:"util.CachedObject"
    PyTuple:3
    itr[0]:PyTuple:3
      itr[0]:"Method Call"
      itr[1]:"server"
      itr[2]:PyTuple:2
        itr[0]:"browserLockdownSvc"
        itr[1]:"GetFlaggedSitesHash"
    itr[1]:0xACECD
    itr[2]:PyTuple:2
      itr[0]:0x1CC26986D4B75A0
      itr[1]:0xC017
        */
    PyTuple * arg_tuple = new PyTuple(3);

    arg_tuple->SetItem(0, new_tuple("Method Call", "server", new_tuple("browserLockdownSvc", "GetFlaggedSitesHash")));
    arg_tuple->SetItem(1, new PyInt(0xACECD));
    arg_tuple->SetItem(2, new_tuple(0x1CC26986D4B75A0, 0xC017));
    
    return new PyObject( "util.CachedObject" , arg_tuple );
}

/**
 * CacheOK python class. This class simulates the python class CacheOK which is derived from the StandardError class.
 */
class CacheOK : public PyObjectEx_Type1
{
public:
    CacheOK() : PyObjectEx_Type1( new PyToken("objectCaching.CacheOK"), new_tuple("CacheOK") ) {}
};



PyResult BrowserLockDownService::Handle_GetFlaggedSitesHash(PyCallArgs &call)
{
    /* if cache hash is correct send CacheOK */

    /**
      PyTuple:4
        itr[0]:PyInt(0x1)
        itr[1]:"GetFlaggedSitesHash"
        itr[2]:PyTuple:0
        itr[3]:PyDict:1
          dict["machoVersion"]=PyList:2
            itr:PyLong(0x1CC26986D4B75A0) // time
            itr:PyInt(0xC017) // hash?
     */
    return new CacheOK();
}

PyResult BrowserLockDownService::Handle_GetFlaggedSitesList(PyCallArgs &call) {
	
	//PyDict* args = new PyDict;

	//return new PyObject( "objectCaching.CachedMethodCallResult", args );
//}

        /*PyClass
          PyString:"objectCaching.CachedMethodCallResult"
          PyTuple:3
            itr[0]:PyDict:1
              dict["versionCheck"]=PyTuple:3
                itr[0]:"run"
                itr[1]:"run"
                itr[2]:"run"
            itr[1]:PyClass
              PyString:"util.CachedObject"
              PyTuple:3
                itr[0]:PyTuple:3
                  itr[0]:"Method Call"
                  itr[1]:"server"
                  itr[2]:PyTuple:2
                    itr[0]:"browserLockdownSvc"
                    itr[1]:"GetFlaggedSitesHash"
                itr[1]:0xACECD
                itr[2]:PyTuple:2
                  itr[0]:0x1CC26986D4B75A0
                  itr[1]:0xC017
            itr[2]:None
            */


    PyTuple* arg_tuple = new PyTuple(3);

        PyDict* itr_1 = new PyDict();
        itr_1->SetItem("versionCheck", new_tuple("run", "run", "run"));

    arg_tuple->SetItem(0, itr_1);
    arg_tuple->SetItem(1, GenerateLockDownCachedObject());
    arg_tuple->SetItem(2, new PyNone());

    return new PyObject( "objectCaching.CachedMethodCallResult", arg_tuple );
}
    

PyResult BrowserLockDownService::Handle_GetDefaultHomePage(PyCallArgs &call) {
    return NULL;
}
