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
    Author:     Zhur
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "account/AuthService.h"

AuthService::AuthService() :
    Service ("authentication")
{
    this->Add("Ping", &AuthService::Ping);
    this->Add("GetPostAuthenticationMessage", &AuthService::GetPostAuthenticationMessage);
    this->Add("AmUnderage", &AuthService::AmUnderage);
    this->Add("AccruedTime", &AuthService::AccruedTime);
    this->Add("SetLanguageID", &AuthService::SetLanguageID);
}

PyResult AuthService::Ping(PyCallArgs &call) {
    return new PyLong(GetFileTimeNow());
}

PyResult AuthService::GetPostAuthenticationMessage(PyCallArgs &call)
{
    if (sConfig.account.loginMessage.empty())
        return PyStatic.NewNone();

    PyDict* args = new PyDict;
        args->SetItemString( "message", new PyString( sConfig.account.loginMessage ) );
    return new PyObject( "util.KeyVal", args );
}

PyResult AuthService::AmUnderage(PyCallArgs &call)
{
    //  return sm.RemoteSvc('authentication').AmUnderage()
    sLog.Warning("AuthService", "Handle_AmUnderage() size=%lli", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    // return boolean
    return new PyBool(false);
}

PyResult AuthService::AccruedTime(PyCallArgs &call)
{
    // return sm.RemoteSvc('authentication').AccruedTime()
    sLog.Warning("AuthService", "Handle_AccruedTime() size=%lli", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult AuthService::SetLanguageID(PyCallArgs &call, PyRep* languageID)
{
    //sm.RemoteSvc('authentication').SetLanguageID(setlanguageID)
    sLog.Warning("AuthService", "Handle_SetLanguageID() size=%lli", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}
