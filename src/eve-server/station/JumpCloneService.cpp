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



#include "station/JumpCloneService.h"

JumpCloneService::JumpCloneService(EVEServiceManager& mgr) :
    BindableService("jumpCloneSvc", mgr)
{ 
}

BoundDispatcher* JumpCloneService::BindObject (Client* client, PyRep* bindParameters)
{
    return new JumpCloneBound(this->GetServiceManager(), *this, &m_db, client->GetLocationID());
}
void JumpCloneService::BoundReleased (JumpCloneBound* bound) {


}
JumpCloneBound::JumpCloneBound (EVEServiceManager& mgr, JumpCloneService& parent, StationDB* db, uint32 locationID) :
    EVEBoundObject(mgr, parent),
    m_db(db),
    m_locationID(locationID),                 // station or ship
    m_locGroupID(EVEDB::invGroups::Station)   // solarsystem(for ship) or station
{
    this->Add("GetCloneState", &JumpCloneBound::GetCloneState);
    this->Add("GetShipCloneState", &JumpCloneBound::GetShipCloneState);
    this->Add("GetPriceForClone", &JumpCloneBound::GetPriceForClone);
    this->Add("InstallCloneInStation", &JumpCloneBound::InstallCloneInStation);
    this->Add("GetStationCloneState", &JumpCloneBound::GetStationCloneState);
    this->Add("OfferShipCloneInstallation", &JumpCloneBound::OfferShipCloneInstallation);
    this->Add("DestroyInstalledClone", &JumpCloneBound::DestroyInstalledClone);
    this->Add("AcceptShipCloneInstallation", &JumpCloneBound::AcceptShipCloneInstallation);
    this->Add("CancelShipCloneInstallation", &JumpCloneBound::CancelShipCloneInstallation);
    this->Add("CloneJump", &JumpCloneBound::CloneJump);
    /*
     _ _notifyevents__ = ['OnShipJumpCloneIns*tallationOffered',
     'OnShipJumpCloneInstallationDone',
     'OnJumpCloneCacheInvalidated',
     'OnShipJumpCloneCacheInvalidated',
     'OnStationJumpCloneCacheInvalidated',
     'OnShipJumpCloneInstallationCanceled']
     */

    if (sDataMgr.IsStation(m_locationID))
        m_locGroupID = EVEDB::invGroups::Solar_System;
}

PyResult JumpCloneBound::InstallCloneInStation(PyCallArgs &call) {
    //19:02:15 W JumpCloneBound::Handle_InstallCloneInStation(): size= 0

  return nullptr;
}

PyResult JumpCloneBound::GetCloneState(PyCallArgs &call) {
    /*  stationClones{jumpCloneID, locationID}
     * shipClones{jumpCloneID, ownerID, locationID}
     * cloneImplants{jumpCloneID, implants}
     */

    PyDict* dict = new PyDict();
    PyDict* clones = new PyDict();  //jumpCloneID, locationID [, ownerID - for shipClones only]
    PyDict* implants = new PyDict();  //jumpCloneID, implants{tuple of implantID?, typeID}
    //PyTuple* implants = new PyTuple(2);

    dict->SetItemString( "clones", clones );
    dict->SetItemString( "implants", implants );
    dict->SetItemString( "timeLastJump", new PyLong(GetFileTimeNow() -(EvE::Time::Hour *MakeRandomFloat(1, 23))) );   /** @todo fix this to call.client->GetChar()->LastJumpTime()*/

    return new PyObject( "util.KeyVal", dict );
}

PyResult JumpCloneBound::GetShipCloneState(PyCallArgs &call) {
    _log(CHARACTER__INFO, "JumpCloneBound::Handle_GetShipCloneState()");

    //Define PyList for ship clones (not dict since client is looking to index through list)
    PyList* clones = new PyList();

    // returns list
    return clones;
}

PyResult JumpCloneBound::GetStationCloneState(PyCallArgs &call) {
    _log(CHARACTER__INFO, "JumpCloneBound::Handle_GetStationCloneState()");

    PyDict* dict = new PyDict();
    PyDict* clones = new PyDict();  //jumpCloneID, locationID [, ownerID - for shipClones only]
    PyDict* implants = new PyDict();  //jumpCloneID, implants{tuple of implantID?, typeID}
    //PyTuple* implants = new PyTuple(2);

    dict->SetItemString( "clones", clones );
    dict->SetItemString( "implants", implants );
    dict->SetItemString( "timeLastJump", new PyLong(GetFileTimeNow() -(EvE::Time::Hour *MakeRandomFloat(1, 23))) );   /** @todo fix this to call.client->GetChar()->LastJumpTime()*/

    return new PyObject( "util.KeyVal", dict );
}

PyResult JumpCloneBound::GetPriceForClone(PyCallArgs &call) {
    /*        kwargs = {'amount': None, 'player': 140000038}
     * TypeError: Numeric Formatter expects floating point or signed integer types.
     */

    return new PyInt(1000000);
}

PyResult JumpCloneBound::OfferShipCloneInstallation(PyCallArgs &call, PyInt* characterID) {
    //    OfferShipCloneInstallation(charID)  //offeringCharID, targetCharID, shipID, b (b=unknown)
    _log(CHARACTER__INFO, "JumpCloneBound::Handle_OfferShipCloneInstallation()");

    return nullptr;
}

PyResult JumpCloneBound::DestroyInstalledClone(PyCallArgs &call, PyInt* cloneID) {
    //    lm.DestroyInstalledClone(cloneID)
    _log(CHARACTER__INFO, "JumpCloneBound::Handle_DestroyInstalledClone()");

    return nullptr;
}

PyResult JumpCloneBound::AcceptShipCloneInstallation(PyCallArgs &call) {
    //lm.AcceptShipCloneInstallation()
    _log(CHARACTER__INFO, "JumpCloneBound::Handle_AcceptShipCloneInstallation()");

    return nullptr;
}

PyResult JumpCloneBound::CancelShipCloneInstallation(PyCallArgs &call) {
    //lm.CancelShipCloneInstallation()
    _log(CHARACTER__INFO, "JumpCloneBound::Handle_CancelShipCloneInstallation()");

    return nullptr;
}

PyResult JumpCloneBound::CloneJump(PyCallArgs &call, PyInt* locationID) {
    //lm.CloneJump, destLocationID
_log(CHARACTER__INFO, "JumpCloneBound::Handle_CloneJump()");

    return nullptr;
}

