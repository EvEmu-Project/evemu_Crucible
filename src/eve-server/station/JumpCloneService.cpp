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
*/

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "station/JumpCloneService.h"

class JumpCloneBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(JumpCloneBound)

    JumpCloneBound(PyServiceMgr *mgr, StationDB *db, uint32 locationID, uint8 locGroupID )
    : PyBoundObject(mgr),
      m_db(db),
      m_dispatch(new Dispatcher(this)),
      m_locationID(locationID),     // station or ship
      m_locGroupID(locGroupID)      // solarsystem(for ship) = 5, station = 15
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "JumpCloneBound";

        PyCallable_REG_CALL(JumpCloneBound, GetCloneState);
        PyCallable_REG_CALL(JumpCloneBound, GetShipCloneState);
        PyCallable_REG_CALL(JumpCloneBound, GetPriceForClone);
        PyCallable_REG_CALL(JumpCloneBound, InstallCloneInStation);
        PyCallable_REG_CALL(JumpCloneBound, GetStationCloneState);
        PyCallable_REG_CALL(JumpCloneBound, OfferShipCloneInstallation);
        PyCallable_REG_CALL(JumpCloneBound, DestroyInstalledClone);
        PyCallable_REG_CALL(JumpCloneBound, AcceptShipCloneInstallation);
        PyCallable_REG_CALL(JumpCloneBound, CancelShipCloneInstallation);
        PyCallable_REG_CALL(JumpCloneBound, CloneJump);
        /*
         _ _notifyevents__ = ['OnShipJumpCloneIns*tallationOffered',
         'OnShipJumpCloneInstallationDone',
         'OnJumpCloneCacheInvalidated',
         'OnShipJumpCloneCacheInvalidated',
         'OnStationJumpCloneCacheInvalidated',
         'OnShipJumpCloneInstallationCanceled']
         */
    }
    virtual ~JumpCloneBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(GetCloneState);
    PyCallable_DECL_CALL(GetShipCloneState);
    PyCallable_DECL_CALL(GetPriceForClone);
    PyCallable_DECL_CALL(InstallCloneInStation);
    PyCallable_DECL_CALL(GetStationCloneState);
    PyCallable_DECL_CALL(OfferShipCloneInstallation);
    PyCallable_DECL_CALL(DestroyInstalledClone);
    PyCallable_DECL_CALL(AcceptShipCloneInstallation);
    PyCallable_DECL_CALL(CancelShipCloneInstallation);
    PyCallable_DECL_CALL(CloneJump);

protected:
    StationDB *const m_db;        //we do not own this
    Dispatcher *const m_dispatch;    //we own this

    uint32 m_locationID;
    uint8 m_locGroupID;
};

PyCallable_Make_InnerDispatcher(JumpCloneService)

JumpCloneService::JumpCloneService(PyServiceMgr *mgr)
: PyService(mgr, "jumpCloneSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    //PyCallable_REG_CALL(JumpCloneService, GetShipCloneState);
}

JumpCloneService::~JumpCloneService() {
    delete m_dispatch;
}

PyBoundObject* JumpCloneService::CreateBoundObject( Client* pClient, const PyRep* bind_args )
{
    _log( CLIENT__MESSAGE, "JumpCloneService bind request for:" );
    bind_args->Dump( CLIENT__MESSAGE, "    " );

    return new JumpCloneBound( m_manager, &m_db, 0, 0 );
}

PyResult JumpCloneBound::Handle_InstallCloneInStation( PyCallArgs &call ) {
    //19:02:15 W JumpCloneBound::Handle_InstallCloneInStation(): size= 0

  return nullptr;
}

PyResult JumpCloneBound::Handle_GetCloneState(PyCallArgs &call) {
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

PyResult JumpCloneBound::Handle_GetShipCloneState(PyCallArgs &call) {
    _log(CHARACTER__INFO, "JumpCloneBound::Handle_GetShipCloneState()");

    PyDict* dict = new PyDict();
    PyDict* clones = new PyDict();  //jumpCloneID, locationID [, ownerID - for shipClones only]
    PyDict* implants = new PyDict();  //jumpCloneID, implants{tuple of implantID?, typeID}
    //PyTuple* implants = new PyTuple(2);

    dict->SetItemString( "clones", clones );
    dict->SetItemString( "implants", implants );
    dict->SetItemString( "timeLastJump", new PyLong(GetFileTimeNow() -(EvE::Time::Hour *MakeRandomFloat(1, 23))) );   /** @todo fix this to call.client->GetChar()->LastJumpTime()*/

    // returns list
    return new PyObject( "util.KeyVal", dict );
}

PyResult JumpCloneBound::Handle_GetStationCloneState(PyCallArgs &call) {
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

PyResult JumpCloneBound::Handle_GetPriceForClone(PyCallArgs &call) {
    /*        kwargs = {'amount': None, 'player': 140000038}
     * TypeError: Numeric Formatter expects floating point or signed integer types.
     */

    return new PyInt(1000000);
}

PyResult JumpCloneBound::Handle_OfferShipCloneInstallation(PyCallArgs &call) {
    //    OfferShipCloneInstallation(charID)  //offeringCharID, targetCharID, shipID, b (b=unknown)
    _log(CHARACTER__INFO, "JumpCloneBound::Handle_OfferShipCloneInstallation()");

    return nullptr;
}

PyResult JumpCloneBound::Handle_DestroyInstalledClone(PyCallArgs &call) {
    //    lm.DestroyInstalledClone(cloneID)
    _log(CHARACTER__INFO, "JumpCloneBound::Handle_DestroyInstalledClone()");

    return nullptr;
}

PyResult JumpCloneBound::Handle_AcceptShipCloneInstallation(PyCallArgs &call) {
    //lm.AcceptShipCloneInstallation()
    _log(CHARACTER__INFO, "JumpCloneBound::Handle_AcceptShipCloneInstallation()");

    return nullptr;
}

PyResult JumpCloneBound::Handle_CancelShipCloneInstallation(PyCallArgs &call) {
    //lm.CancelShipCloneInstallation()
    _log(CHARACTER__INFO, "JumpCloneBound::Handle_CancelShipCloneInstallation()");

    return nullptr;
}

PyResult JumpCloneBound::Handle_CloneJump(PyCallArgs &call) {
    //lm.CloneJump, destLocationID
_log(CHARACTER__INFO, "JumpCloneBound::Handle_CloneJump()");

    return nullptr;
}

