/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit http://evemu.mmoforge.org
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

#include "EVECollectDisp.h"
#include "../common/logsys.h"
#include "../common/PyPacket.h"
#include "../common/PyRep.h"
#include "../common/PyVisitor.h"
#include "../packets/AccountPkts.h"
#include "../packets/General.h"
#include "../packets/Destiny.h"
#include "../common/PyDumpVisitor.h"
#include "../common/PyXMLGenerator.h"
#include "../common/CachedObjectMgr.h"
#include "../common/RowsetReader.h"
#include "../common/DestinyBinDump.h"

#include <cstddef>

namespace {

constexpr std::size_t kMaxAddBalls2DiagnosticBytes = 64U * 1024U;
constexpr std::size_t kMaxAddBalls2DiagnosticEntries = 4096U;

void ReleaseAddBalls2State(AddBalls2 *update) {
    if (update == nullptr)
        return;

    PySafeDecRef(update->state);
    update->state = nullptr;
}

} // namespace

//NOTE: there is also a plural version of thisDoDestinyUpdates, which
//carries any number of DoDestinyUpdate bodies in its top level list.
void EVECollectDispatcher::Notify_DoDestinyUpdate(const PyPacket *packet, EVENotificationStream **in_notify) {
    EVENotificationStream *notify = *in_notify;
    *in_notify = NULL;

    DoDestinyUpdateMain   dumain;
    DoDestinyUpdateMain_2 dumain2;
    PyRepList::iterator cur, end;

    if(notify->args->items.size() == 2) {
        //no idea why some of them have 2..
        if(!dumain2.Decode(&notify->args)) {
            delete notify;
            codelog(COLLECT__ERROR, "Unable to decode destiny update main (2).");
            return;
        }
        cur = dumain2.updates.begin();
        end = dumain2.updates.end();
    } else {
        if(!dumain.Decode(&notify->args)) {
            delete notify;
            codelog(COLLECT__ERROR, "Unable to decode destiny update main.");
            return;
        }
        cur = dumain.updates.begin();
        end = dumain.updates.end();
    }
    delete notify;

    for(; cur != end; cur++) {
        PyRep *r = *cur;
        *cur = NULL;

        _log(COLLECT__DESTINY_REP, "DoDestinyUpdate Action:");
        PyLookupDump dumper(&lookResolver, COLLECT__DESTINY_REP);
        r->visit(&dumper);

        DoDestinyAction action;
        if(!action.Decode(&r)) {
            codelog(COLLECT__ERROR, "Unable to decode destiny update action");
            return;
        }

        //bah..
        if(!action.update->CheckType(PyRep::Tuple)) {
            codelog(COLLECT__ERROR, "Expected tuple in DoDestinyAction, got %s", action.update->TypeString());
            continue;
        }
        PyRepTuple *t = (PyRepTuple *) action.update;
        if(t->items.size() != 2) {
            codelog(COLLECT__ERROR, "Expected size 2 tuple in DoDestinyAction, got %d", t->items.size());
            continue;
        }
        if(!t->items[0]->CheckType(PyRep::String)) {
            codelog(COLLECT__ERROR, "Expected string in DoDestinyAction update type, got %s", t->items[0]->TypeString());
            continue;
        }

        PyRepString *updtype = (PyRepString *) t->items[0];

        _log(COLLECT__NOTIFY_SUMMARY, "    Destiny Update: %s", updtype->value.c_str());

        //try to dispatch it...
        std::map<std::string, _DestinyDispatch>::const_iterator res;
        res = m_destinyDisp.find(updtype->value);
        if(res != m_destinyDisp.end()) {
            _log(COLLECT__DESTINY, "Dispatching action '%s' with ID %d", updtype->value.c_str(), action.update_id);
            _DestinyDispatch dsp = res->second;
            (this->*dsp)(packet, &action);
        }
    }
    dumain.updates.items.clear();
}

void EVECollectDispatcher::Destiny_AddBalls(const PyPacket *packet, DoDestinyAction *action) {
    DoDestiny_AddBalls upd;
    if(!upd.Decode(&action->update)) {
        codelog(COLLECT__ERROR, "Failed to decode arguments");
        return;
    }

    const byte *data = upd.destiny_binary->GetBuffer();
    uint32 len = upd.destiny_binary->GetLength();

    _log(COLLECT__DESTINY_HEX, "AddBalls binary data:");
    _hex(COLLECT__DESTINY_HEX, data, len);

    Destiny::DumpUpdate(COLLECT__DESTINY, data, len);
}

void EVECollectDispatcher::Destiny_AddBalls2(
    const PyPacket *packet, DoDestinyAction *action) {
    if (action == nullptr || action->update == nullptr) {
        codelog(COLLECT__ERROR, "Missing AddBalls2 action data");
        return;
    }

    AddBalls2 update;
    if (!update.Decode(action->update)) {
        ReleaseAddBalls2State(&update);
        codelog(COLLECT__ERROR, "Failed to decode AddBalls2 arguments");
        return;
    }

    if (update.state == nullptr || update.extraBallData == nullptr) {
        ReleaseAddBalls2State(&update);
        codelog(COLLECT__ERROR, "Decoded AddBalls2 has missing data");
        return;
    }

    const std::size_t state_bytes = update.state->size();
    const std::size_t entry_count = update.extraBallData->size();
    _log(COLLECT__DESTINY,
         "AddBalls2 stamp=%d state_bytes=%zu entries=%zu",
         update.stateStamp, state_bytes, entry_count);

    if (state_bytes > kMaxAddBalls2DiagnosticBytes
        || entry_count > kMaxAddBalls2DiagnosticEntries) {
        codelog(COLLECT__ERROR,
                "Skipping oversized AddBalls2 diagnostic: bytes=%zu "
                "entries=%zu",
                state_bytes, entry_count);
        ReleaseAddBalls2State(&update);
        return;
    }

    if (is_log_enabled(COLLECT__DESTINY)) {
        _log(COLLECT__DESTINY, "AddBalls2 state hash=%d",
             update.state->hash());
    }

    if (state_bytes != 0 && is_log_enabled(COLLECT__DESTINY)) {
        const uint8 *data = &update.state->content()[0];
        Destiny::DumpUpdate(
            COLLECT__DESTINY, data, static_cast<uint32>(state_bytes));
    }

    ReleaseAddBalls2State(&update);
}


void EVECollectDispatcher::Destiny_SetState(const PyPacket *packet, DoDestinyAction *action) {
    DoDestiny_SetState upd;
    if(!upd.Decode(&action->update)) {
        codelog(COLLECT__ERROR, "Failed to decode arguments");
        return;
    }

    const byte *data = upd.destiny_state->GetBuffer();
    uint32 len = upd.destiny_state->GetLength();

    _log(COLLECT__DESTINY_HEX, "SetState binary data: (ego=%u)", upd.ego);
    _hex(COLLECT__DESTINY_HEX, data, len);

    Destiny::DumpUpdate(COLLECT__DESTINY, data, len);
}











