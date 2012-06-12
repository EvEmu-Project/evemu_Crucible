/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Captnoord
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "account/BrowserLockdownSvc.h"

// crap
PyCallable_Make_InnerDispatcher(BrowserLockdownService)

BrowserLockdownService::BrowserLockdownService( PyServiceMgr *mgr )
: PyService(mgr, "browserLockdownSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(BrowserLockdownService, GetFlaggedSitesHash)
    PyCallable_REG_CALL(BrowserLockdownService, GetFlaggedSitesList)
    PyCallable_REG_CALL(BrowserLockdownService, GetDefaultHomePage)
}

BrowserLockdownService::~BrowserLockdownService() {
    delete m_dispatch;
}

PyObject* GenerateLockdownCachedObject()
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



PyResult BrowserLockdownService::Handle_GetFlaggedSitesHash(PyCallArgs &call)
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

PyResult BrowserLockdownService::Handle_GetFlaggedSitesList(PyCallArgs &call) {

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
    arg_tuple->SetItem(1, GenerateLockdownCachedObject());
    arg_tuple->SetItem(2, new PyNone());

    return new PyObject( "objectCaching.CachedMethodCallResult", arg_tuple );
}


PyResult BrowserLockdownService::Handle_GetDefaultHomePage(PyCallArgs &call) {
    return NULL;
}
