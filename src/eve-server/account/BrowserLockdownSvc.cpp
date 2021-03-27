/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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
    Updates:    Allan
*/

#include "eve-server.h"
#include "EVE_Consts.h"

#include "PyServiceCD.h"
#include "account/BrowserLockdownSvc.h"


PyCallable_Make_InnerDispatcher(BrowserLockdownService)

BrowserLockdownService::BrowserLockdownService( PyServiceMgr *mgr )
: PyService(mgr, "browserLockdownSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(BrowserLockdownService, GetFlaggedSitesHash);
    PyCallable_REG_CALL(BrowserLockdownService, GetFlaggedSitesList);
    PyCallable_REG_CALL(BrowserLockdownService, GetDefaultHomePage);
    PyCallable_REG_CALL(BrowserLockdownService, IsBrowserInLockdown);
}

BrowserLockdownService::~BrowserLockdownService() {
    delete m_dispatch;
}

PyResult BrowserLockdownService::Handle_GetFlaggedSitesHash(PyCallArgs &call)
{
    /* Future updates should be the md5 sum of the cache/browser/flaggedsites.dat file from user/appdata/Local/CCP/EVE
     *  as we dont have this list (which is sent from GetFlaggedSitesList), we send the md5 sum for an empty set.
     */
    return new PyString("df98509e1e3f0dd839083e7be1d2b360");    // mine
    //return new PyString("d751713988987e9331980363e24189ce");    // avianrr
}

PyResult BrowserLockdownService::Handle_GetFlaggedSitesList(PyCallArgs &call)
{
    // all packets show none here, however, this throws error for me in crucible.
    //  needs to be empty list to play nice with above hash
    return PyStatic.mtList();
}

PyResult BrowserLockdownService::Handle_GetDefaultHomePage(PyCallArgs &call)
{
    return new PyWString(HomePageURL);
}

PyResult BrowserLockdownService::Handle_IsBrowserInLockdown(PyCallArgs &call)
{
    return new PyBool(sConfig.server.DisableIGB);
}
