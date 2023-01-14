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

AllianceRegistry::AllianceRegistry(EVEServiceManager& mgr) :
    BindableService("allianceRegistry", mgr)
{
    this->Add("GetAlliance", &AllianceRegistry::GetAlliance);
    this->Add("GetRankedAlliances", &AllianceRegistry::GetRankedAlliances);
    this->Add("GetEmploymentRecord", &AllianceRegistry::GetEmploymentRecord);
    this->Add("GetAllianceMembers", &AllianceRegistry::GetAllianceMembers);
}

BoundDispatcher* AllianceRegistry::BindObject(Client* client, PyRep* bindParameters)
{
    if (!bindParameters->IsTuple()){
        sLog.Error( "AllianceRegistry::CreateBoundObject", "%s: bind_args is not tuple: '%s'. ", client->GetName(), bindParameters->TypeString() );
        client->SendErrorMsg("Could not bind object for Ally Registry.  Ref: ServerError 02808.");
        return nullptr;
    }

    return new AllianceBound(this->GetServiceManager(), bindParameters, m_db, PyRep::IntegerValue(bindParameters->AsTuple()->GetItem(0)));
}

// this is the bind call.  do it like fleet
PyResult AllianceRegistry::GetAlliance(PyCallArgs &call, PyInt* allianceID) {
    //alliance = sm.RemoteSvc('allianceRegistry').GetAlliance(allianceID)
    /*
     * 01:22:07 [SvcCall] Service allianceRegistry::MachoResolveObject()
     * 01:22:07 [SvcCall] Service allianceRegistry::MachoBindObject()
     * 01:22:07 [SvcError] allianceRegistry Service: allan: Unable to create bound object for:
     * 01:22:07 [SvcError]      Tuple: 2 elements
     * 01:22:07 [SvcError]       [ 0]    Integer: 0
     * 01:22:07 [SvcError]       [ 1]    Integer: 1
     */

    _log(ALLY__CALL, "AllianceRegistry::Handle_GetAlliance() size=%li", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    return m_db.GetAlliance(allianceID->value());
}

PyResult AllianceRegistry::GetAllianceMembers(PyCallArgs &call, PyInt* allianceID) {
    // members = sm.RemoteSvc('allianceRegistry').GetAllianceMembers(itemID)  <-- returns dict of corpIDs
    _log(ALLY__CALL, "AllianceRegistry::Handle_GetAllianceMembers() size=%li", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    return m_db.GetAllianceMembers(allianceID->value());
}

PyResult AllianceRegistry::GetRankedAlliances(PyCallArgs &call, PyInt* maxLen) {
    /*
     *           self.rankedAlliances.alliances = sm.RemoteSvc('allianceRegistry').GetRankedAlliances(maxLen)
     *           self.rankedAlliances.standings = {}
     *           for a in self.rankedAlliances.alliances:
     *               s = sm.GetService('standing').GetStanding(eve.session.corpid, a.allianceID)
     *               self.rankedAlliances.standings[a.allianceID] = s
     */

    _log(ALLY__CALL, "AllianceRegistry::Handle_GetRankedAlliances() size=%li", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    return m_db.GetRankedAlliances();
}

//Not sure why this doesn't work
PyResult AllianceRegistry::GetEmploymentRecord(PyCallArgs &call, PyInt* corporationID) {
    //  allianceHistory = sm.RemoteSvc('allianceRegistry').GetEmploymentRecord(itemID)
    _log(ALLY__CALL, "AllianceRegistry::Handle_GetEmploymentRecord() size=%li", call.tuple->size());
    call.Dump(ALLY__CALL_DUMP);

    return m_db.GetEmploymentRecord(corporationID->value());
}