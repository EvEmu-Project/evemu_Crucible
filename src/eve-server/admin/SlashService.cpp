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
    Author:        Zhur
*/


#include "eve-server.h"

#include "PyServiceCD.h"
#include "admin/CommandDispatcher.h"
#include "admin/SlashService.h"


PyCallable_Make_InnerDispatcher(SlashService)

SlashService::SlashService(PyServiceMgr *mgr, CommandDispatcher *cd)
: PyService(mgr, "slash"),
  m_dispatch(new Dispatcher(this)),
  m_commandDispatch(cd)
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(SlashService, SlashCmd);
}

SlashService::~SlashService() {
    delete m_dispatch;
}

PyResult SlashService::Handle_SlashCmd( PyCallArgs& call )
{
    if (is_log_enabled(COMMAND__DUMP)) {
        sLog.White("SlashService::Handle_SlashCmd()", "size=%li", call.tuple->size());
        call.Dump(COMMAND__DUMP);
    }
    Call_SingleStringArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return SlashCommand( call.client, arg.arg );
}

PyResult SlashService::SlashCommand(Client * client, std::string command)
{
    if ((client->GetAccountRole() & Acct::Role::SLASH) != Acct::Role::SLASH) {
        _log( COMMAND__ERROR, "%s: Client '%s' used a slash command but does not have Acct::Role::SLASH.", GetName(), client->GetName() );
        throw CustomError ("You need to have Acct::Role::SLASH to execute commands.");
    }

    return m_commandDispatch->Execute( client, command.c_str() );
}

/*(258713, `Role GMH needed`)
 * (258714, `Role GML needed`)
 */