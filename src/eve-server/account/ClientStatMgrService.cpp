/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
#include "account/ClientStatMgrService.h"

PyCallable_Make_InnerDispatcher(ClientStatsMgr)

ClientStatsMgr::ClientStatsMgr(PyServiceMgr *mgr)
: PyService(mgr, "clientStatsMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(ClientStatsMgr, SubmitStats);
}

ClientStatsMgr::~ClientStatsMgr()
{
    delete m_dispatch;
}

PyResult ClientStatsMgr::Handle_SubmitStats( PyCallArgs& call )
{
    sLog.Debug( "ClientStatsMgr", "Called SubmitStats stub." );

    return PyStatic.NewNone();
}
/**
    def SendContentsToServer(self, contents = None):
        try:
            if not sm.services['machoNet'].IsConnected():
                return
        except:
            sys.exc_clear()
            return

        if contents is None:
            contents = self.prevContents
        if contents[0] != self.version:
            contents = {}
        else:
            contents = contents[1]
        build = boot.GetValue('build', None)
        contentType = CONTENT_TYPE_PREMIUM
        operatingSystem = PLATFORM_WINDOWS
        if blue.win32.IsTransgaming():
            operatingSystem = PLATFORM_MACOS
        blendedContents = self.entries
        blendedStateMask = self.stateMask
        self.entries = dict()
        self.stateMask = 0
        if contents.has_key(STATE_DISCONNECT):
            blendedContents[STATE_DISCONNECT] = contents[STATE_DISCONNECT]
            blendedStateMask += STATE_DISCONNECT
        if contents.has_key(STATE_GAMESHUTDOWN):
            blendedContents[STATE_GAMESHUTDOWN] = contents[STATE_GAMESHUTDOWN]
            blendedStateMask += STATE_GAMESHUTDOWN
        header = (self.version,
         blendedStateMask,
         build,
         operatingSystem,
         contentType)
        data = (header, blendedContents)
        try:
            uthread.Lock(self, 'sendContents')
            sm.RemoteSvc('clientStatsMgr').SubmitStats(data)
            if hasattr(self, 'prevContents'):
                delattr(self, 'prevContents')
            return True
        finally:
            uthread.UnLock(self, 'sendContents')
*/
