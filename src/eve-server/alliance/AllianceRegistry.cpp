/*
 *   ------------------------------------------------------------------------------------
 *   LICENSE:
 *   ------------------------------------------------------------------------------------
 *   This file is part of EVEmu: EVE Online Server Emulator
 *   Copyright 2006 - 2021 The EVEmu Team
 *   For the latest information visit https://evemu.dev
 *   ------------------------------------------------------------------------------------
 *   This program is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free Software
 *   Foundation; either version 2 of the License, or (at your option) any later
 *   version.
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *   You should have received a copy of the GNU Lesser General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *   Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *   http://www.gnu.org/copyleft/lesser.txt.
 *   ------------------------------------------------------------------------------------
 *   Author:        Allan
 *   Updates:       James
 */

//work in progress
/** @note  this is a bound object!  */

#include "eve-server.h"

#include "PyServiceCD.h"
#include "alliance/AllianceRegistry.h"

/*
 * ALLY__ERROR
 * ALLY__WARNING
 * ALLY__INFO
 * ALLY__MESSAGE
 * ALLY__TRACE
 * ALLY__CALL
 * ALLY__CALL_DUMP
 * ALLY__RSP_DUMP
 */

PyCallable_Make_InnerDispatcher(AllianceRegistry)

AllianceRegistry::AllianceRegistry(PyServiceMgr *mgr)
: PyService(mgr, "allianceRegistry"),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(AllianceRegistry, GetAlliance);     //  bound object call
    PyCallable_REG_CALL(AllianceRegistry, GetRankedAlliances);
    PyCallable_REG_CALL(AllianceRegistry, GetEmploymentRecord);
    PyCallable_REG_CALL(AllianceRegistry, GetAllianceMembers);
}

AllianceRegistry::~AllianceRegistry()
{
    delete m_dispatch;
}

PyBoundObject* AllianceRegistry::CreateBoundObject( Client* pClient, const PyRep* bind_args )
{
    if (!bind_args->IsTuple()){
        sLog.Error( "AllianceRegistry::CreateBoundObject", "%s: bind_args is not tuple: '%s'. ", pClient->GetName(), bind_args->TypeString() );
        pClient->SendErrorMsg("Could not bind object for Ally Registry.  Ref: ServerError 02808.");
        return nullptr;
    }

    return new AllianceBound(m_manager, m_db, PyRep::IntegerValue(bind_args->AsTuple()->GetItem(0)));
}

// this is the bind call.  do it like fleet
PyResult AllianceRegistry::Handle_GetAlliance(PyCallArgs &call) {
    //alliance = sm.RemoteSvc('allianceRegistry').GetAlliance(allianceID)
    /*
     * 01:22:07 [SvcCall] Service allianceRegistry::MachoResolveObject()
     * 01:22:07 [SvcCall] Service allianceRegistry::MachoBindObject()
     * 01:22:07 [SvcError] allianceRegistry Service: allan: Unable to create bound object for:
     * 01:22:07 [SvcError]      Tuple: 2 elements
     * 01:22:07 [SvcError]       [ 0]    Integer: 0
     * 01:22:07 [SvcError]       [ 1]    Integer: 1
     */

    _log(ALLY__CALL, "AllianceRegistry::Handle_GetAlliance() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    return m_db.GetAlliance(args.arg);
}

PyResult AllianceRegistry::Handle_GetAllianceMembers(PyCallArgs &call) {
    // members = sm.RemoteSvc('allianceRegistry').GetAllianceMembers(itemID)  <-- returns dict of corpIDs
    _log(ALLY__CALL, "AllianceRegistry::Handle_GetAllianceMembers() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    return m_db.GetAllianceMembers(args.arg);
}

PyResult AllianceRegistry::Handle_GetRankedAlliances(PyCallArgs &call) {
    /*
     *           self.rankedAlliances.alliances = sm.RemoteSvc('allianceRegistry').GetRankedAlliances(maxLen)
     *           self.rankedAlliances.standings = {}
     *           for a in self.rankedAlliances.alliances:
     *               s = sm.GetService('standing').GetStanding(eve.session.corpid, a.allianceID)
     *               self.rankedAlliances.standings[a.allianceID] = s
     */

    _log(ALLY__CALL, "AllianceRegistry::Handle_GetRankedAlliances() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);

    return m_db.GetRankedAlliances();
}

//Not sure why this doesn't work
PyResult AllianceRegistry::Handle_GetEmploymentRecord(PyCallArgs &call) {
    //  allianceHistory = sm.RemoteSvc('allianceRegistry').GetEmploymentRecord(itemID)
    _log(ALLY__CALL, "AllianceRegistry::Handle_GetEmploymentRecord() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetEmploymentRecord(arg.arg);
}