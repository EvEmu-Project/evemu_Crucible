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

#include "ServiceManager.h"

EVEServiceManager::EVEServiceManager(NodeID nodeId) :
    mLastBoundId (1),
    mNodeId (nodeId)
{
}

void EVEServiceManager::Register(Dispatcher* service) {
    this->mServices.insert(std::make_pair(service->GetName(), service));
}

Dispatcher* EVEServiceManager::Lookup(const std::string& service) {
    auto it = this->mServices.find(service);

    if (it == this->mServices.end())
        throw service_not_found(service);

    return (*it).second;
}

BoundID EVEServiceManager::RegisterBoundService(BoundDispatcher* service) {
    BoundID newBoundId = this->mLastBoundId++;

    // add the bound service to the list
    this->mBound.insert(
        std::make_pair(newBoundId, service)
    );

    return newBoundId;
}

PyResult EVEServiceManager::Dispatch(const std::string& service, const std::string& method, PyCallArgs& args) {
    auto it = this->mServices.find(service);

    if (it == this->mServices.end())
        throw service_not_found(service);

    // check access levels
    ClientSession* session = args.client->GetSession();

    if (
            (it->second->GetAccessLevel() == eAccessLevel_Location && session->HasValue ("locationid") == false) ||
            (it->second->GetAccessLevel() == eAccessLevel_SolarSystem && session->HasValue ("solarsystemid") == false) ||
            (it->second->GetAccessLevel() == eAccessLevel_SolarSystem2 && session->HasValue ("solarsystemid2") == false) ||
            (it->second->GetAccessLevel() == eAccessLevel_Character && session->HasValue ("charid") == false) ||
            (it->second->GetAccessLevel() == eAccessLevel_Corporation && session->HasValue ("corpid") == false) ||
            (it->second->GetAccessLevel() == eAccessLevel_User && session->HasValue ("userid") == false)
        )
        throw CustomError ("You're not allowed to access this service");

    return it->second->Dispatch(method, args);
}

PyResult EVEServiceManager::Dispatch(const BoundID& service, const std::string& method, PyCallArgs& args) {
    auto it = this->mBound.find(service);

    if (it == this->mBound.end())
        throw service_not_found("Bound " + std::to_string(service));

    return it->second->Dispatch(method, args);
}

std::string pyRepToString (PyRep* v) {
    if (v->IsBool () == true)
        return "PyBool*";
    if (v->IsInt () == true)
        return "PyInt*";
    if (v->IsLong () == true)
        return "PyLong*";
    if (v->IsFloat () == true)
        return "PyFloat*";
    if (v->IsBuffer () == true)
        return "PyBuffer*";
    if (v->IsString () == true)
        return "PyString*";
    if (v->IsWString () == true)
        return "PyWString*";
    if (v->IsToken () == true)
        return "PyToken*";
    if (v->IsTuple () == true)
        return "PyTuple*";
    if (v->IsList () == true)
        return "PyList*";
    if (v->IsDict () == true)
        return "PyDict*";
    if (v->IsNone () == true)
        return "PyNone*";
    if (v->IsSubStruct () == true)
        return "PySubStruct*";
    if (v->IsSubStream () == true)
        return "PySubStream*";
    if (v->IsChecksumedStream () == true)
        return "PyChecksumedStream*";
    if (v->IsObject () == true)
        return "PyObject*";
    if (v->IsObjectEx () == true)
        return "PyObjectEx*";
    if (v->IsPackedRow () == true)
        return "PyPackedRow*";

    return "Unknown";
}

std::string EVEServiceManager::DebugDispatch (const std::string& service, const std::string& method, PyCallArgs& args) {
    auto it = this->mServices.find (service);

    if (it == this->mServices.end ())
        return "Service not found";

    std::string result = it->second->DebugDispatch (method);

    result += "\n\nDoes not match parameters: \n\t(";

    for (int i = 0; i < args.tuple->size(); i ++) {
        result += pyRepToString(args.tuple->GetItem(i));

        if (i < (args.tuple->size () - 1)) {
            result += ",";
        }
    }

    result += ")";

    return result;
}

std::string EVEServiceManager::DebugDispatch (const BoundID& service, const std::string& method, PyCallArgs& args) {
    auto it = this->mBound.find (service);

    if (it == this->mBound.end ())
        return "Service not found";

    std::string result = it->second->DebugDispatch (method);

    result += "\n\nDoes not match parameters: \n\t(";

    for (int i = 0; i < args.tuple->size(); i ++) {
        result += pyRepToString(args.tuple->GetItem(i));

        if (i < (args.tuple->size () - 1)) {
            result += ",";
        }
    }

    result += ")";

    return result;
}

void EVEServiceManager::ClearBoundObject(const BoundID& service, Client* client) {
    auto it = this->mBound.find(service);

    if (it == this->mBound.end()) {
        _log(SERVICE__ERROR, "EVEServiceManager::ClearBoundObject() - Unable to find bound object %u to release.", service);
        return;
    }

    _log(SERVICE__MESSAGE, "EVEServiceManager::ClearBoundObject() - Clearing bound object at %s", it->second->GetIDString().c_str());
    
    // release the bound, this should cleanup any resources if needed
    if (it->second->Release(client) == true) {
        // finally remove it from the list
        this->mBound.erase(it);
    }
}

const EVEServiceManager::ServicesMap& EVEServiceManager::GetServices() const {
    return this->mServices;
}