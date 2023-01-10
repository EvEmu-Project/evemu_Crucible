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
    Author:        Almamu
*/

#include "BoundService.h"

template<class T>
BindableService<T>::BindableService(const std::string& name, EVEServiceManager* mgr) :
    Service<T> (name),
    mManager (mgr)
{
    this->Add("MachoResolveObject", &BindableService<T>::MachoResolveObject);
    this->Add("MachoBindObject", &BindableService<T>::MachoBindObject);
}

template<class T>
PyResult BindableService<T>::MachoResolveObject(PyCallArgs& args, PyRep* bindParameters, PyRep* justQuery) {
    return new PyInt(this->mManager->GetNodeID());
}

template<class T>
PyResult BindableService<T>::MachoBindObject(PyCallArgs& args, PyRep* bindParameters, std::optional<PyTuple*> call) {
    // register the new instance in the service manager
    BoundDispatcher* bound = this->BindObject(bindParameters);

    // build the bound service identifier
    PyTuple* rsp = new PyTuple(2);
    PyDict* byName = new PyDict();

    byName["OID+"] = bound->GetOID();

    rsp->SetItem(0, new PySubStruct(new PySubStream(bound->GetOID())));
    
    if (call.has_value() == false) {
        rsp->SetItem(1, PyStatic.NewNone());
    } else {
        // dispatch call
        CallMachoBindObject_call boundcall;

        if (boundcall.Decode(&call.value()) == false) {
            codelog(SERVICE__ERROR, "%s Service: Failed to decode boundcall arguments", this->GetName().c_str());
            return nullptr;
        }

        _log(SERVICE__MESSAGE, "%s Service: MachoBindObject also contains call to %s", this->GetName().c_str(), boundcall.method_name.c_str());

        PyCallArgs subArgs(args.client, boundcall.arguments, boundcall.dict_arguments);

        PyResult result = bound->Dispatch(boundcall.method_name, subArgs);

        // set the tuple data
        rsp->SetItem(1, result.ssResult);
        // TODO: merge the dicts to return the full response data?
        // Py types are lacking lots of helper methods that could be useful
    }

    // return the response
    return PyResult(rsp, byName);
}

template<class T>
EVEBoundObject<T>::EVEBoundObject(EVEServiceManager* mgr, PyRep* bindData) :
    Service<T>(""), // bound services do not really need a name
    mManager(mgr),
    mBoundData(bindData)
{
    this->mBoundId = this->mManager->RegisterBoundService(this);

    // build the id string
    std::stringstream strBuilder;
    strBuilder << "N=" << this->mManager->GetNodeID() << ":" << this->mBoundId;

    // store it
    this->mIdString = strBuilder.str();

    // build the OID
    this->mOID = new PyTuple(2);
    this->mOID[0] = new PyString(this->mIdString);
    this->mOID[1] = new PyLong(GetFileTimeNow()); // this isn't really the datetime, should be a unique ID
}

template<class T>
PyResult EVEBoundObject<T>::Dispatch(const std::string& name, PyCallArgs& args) {
    if (this->CanClientCall(args.client) == false)
        throw std::runtime_error("This client is not allowed to call this bound service");

    return Service<T>::Dispatch(name, args);
}
