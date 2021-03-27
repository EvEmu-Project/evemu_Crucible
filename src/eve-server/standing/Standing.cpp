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
    Author:        Zhur
    Updates:        Allan
*/

#include "standing/Standing.h"

/*  re-write of standing system  -allan 10Apr15
 * see notes in StandingDB.cpp
 */

/*
 * STANDING__ERROR
 * STANDING__WARNING
 * STANDING__MESSAGE
 * STANDING__DEBUG
 * STANDING__INFO
 * STANDING__TRACE
 * STANDING__DUMP
 * STANDING__RSPDUMP
 */

PyCallable_Make_InnerDispatcher(Standing)

Standing::Standing(PyServiceMgr *mgr)
: PyService(mgr, "standing2"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(Standing, GetSecurityRating);
    PyCallable_REG_CALL(Standing, GetMyKillRights);
    PyCallable_REG_CALL(Standing, GetCharStandings);
    PyCallable_REG_CALL(Standing, GetCorpStandings);
    PyCallable_REG_CALL(Standing, GetNPCNPCStandings);
    PyCallable_REG_CALL(Standing, GetStandingTransactions);
    PyCallable_REG_CALL(Standing, GetStandingCompositions);

}

Standing::~Standing() {
    delete m_dispatch;
}

PyResult Standing::Handle_GetCharStandings(PyCallArgs &call) {
    return m_db.GetCharStandings(call.client);
}

PyResult Standing::Handle_GetCorpStandings(PyCallArgs &call) {
    return m_db.GetCorpStandings(call.client);
}

PyResult Standing::Handle_GetNPCNPCStandings(PyCallArgs &call) {
    return sStandingMgr.GetFactionStandings();
}

/** @todo  need to add a standing from corpCONCORD to any/all charID, corpID, allyID  for security rating (as seen in client code) */

PyResult Standing::Handle_GetSecurityRating(PyCallArgs &call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    CharacterRef cRef = sItemFactory.GetCharacter( arg.arg );
    if  (cRef.get() == nullptr) {
        _log(STANDING__WARNING, "Character %u not found.", arg.arg);
        return nullptr;
    }

    return new PyFloat( cRef->GetSecurityRating() );
}

PyResult Standing::Handle_GetMyKillRights(PyCallArgs &call) {
    // self.killRightsCache, self.killedRightsCache = sm.RemoteSvc('standing2').GetMyKillRights()
    // each cache holds k,v where key is toID or fromID
    _log(STANDING__MESSAGE,  "Standing::Handle_GetMyKillRights()");
    PyTuple* KillRights = new PyTuple(2);
    PyDict* killRightsCache = new PyDict();
    PyDict* killedRightsCache = new PyDict();
        KillRights->items[0] = killRightsCache;
        KillRights->items[1] = killedRightsCache;

    if (is_log_enabled(STANDING__RSPDUMP)) {
        _log(STANDING__RSPDUMP, "Standing::Handle_GetMyKillRights() RSP:" );
        KillRights->Dump(STANDING__RSPDUMP, "    ");
    }

    return KillRights;
}

PyResult Standing::Handle_GetStandingTransactions(PyCallArgs &call) {
    // data = sm.RemoteSvc('standing2').GetStandingTransactions(fromID, toID, direction, eventID, eventType, eventDateTime)
    _log(STANDING__MESSAGE,  "Standing::Handle_GetStandingTransactions()");
    call.Dump(STANDING__DUMP);

    Call_GetStandingTransactions args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetStandingTransactions(args);
}

PyResult Standing::Handle_GetStandingCompositions(PyCallArgs &call) {
/**  no clue what this is yet
                self.sr.data = sm.RemoteSvc('standing2').GetStandingCompositions(fromID, toID)
            if self.sr.data:
                prior = 0.0
                for each in self.sr.data:
                    if each.ownerID == fromID:
                        prior = each.standing
                        */
    _log(STANDING__MESSAGE,  "Standing::Handle_GetStandingCompositions()");
    call.Dump(STANDING__DUMP);

    Call_GetStandingComposition args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetStandingCompositions(args.toID, args.fromID);
}