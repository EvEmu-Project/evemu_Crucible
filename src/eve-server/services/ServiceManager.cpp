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

    return (*it).second->Dispatch(method, args);
}

PyResult EVEServiceManager::Dispatch(const BoundID& service, const std::string& method, PyCallArgs& args) {
    auto it = this->mBound.find(service);

    if (it == this->mBound.end())
        throw service_not_found("Bound " + std::to_string(service));

    return it->second->Dispatch(method, args);
}

void EVEServiceManager::ClearBoundObject(const BoundID& service) {
    auto it = this->mBound.find(service);

    if (it == this->mBound.end()) {
        _log(SERVICE__ERROR, "EVEServiceManager::ClearBoundObject() - Unable to find bound object %u to release.", service);
        return;
    }

    _log(SERVICE__MESSAGE, "EVEServiceManager::ClearBoundObject() - Clearing bound object at %s", it->second->GetIDString().c_str());
    
    // release the bound, this should cleanup any resources if needed
    it->second->Release();

    // finally remove it from the list
    this->mBound.erase(it);
}