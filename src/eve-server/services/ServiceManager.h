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

#ifndef __SERVICEMANAGER_H__
#define __SERVICEMANAGER_H__

#include <map>
#include <string>

#include "services/Callable.h"
#include "services/Service.h"
#include "services/BoundService.h"
#include "Client.h"

typedef uint32_t NodeID;

class service_not_found {
public:
    service_not_found(const std::string& service) :
        service(service)
    {
    }

    const std::string& service;
};

class EVEServiceManager {
public:
    typedef std::map<std::string, Dispatcher*> ServicesMap;
    typedef std::map<BoundID, BoundDispatcher*> BoundServicesMap;

    EVEServiceManager(NodeID nodeId);

    /**
     * @brief Registers a new service in the normal service list
     */
    void Register(Dispatcher* service);

    /**
     * @brief Looks up the given service and returns a reference to it
     */
    Dispatcher* Lookup(const std::string& service);

    /**
     * @brief Looks up the given service and returns a reference to it,
     * casting to the specified type
     */
    template<class T>
    T* Lookup(const std::string& service) { return static_cast <T*> (this->Lookup (service)); }

    /**
     * @brief Registers a new bound service in this service manager
     */
    BoundID RegisterBoundService(BoundDispatcher* obj);

    /**
     * @brief Dispatches a normal call to the requested service and method and returns the result
     */
    PyResult Dispatch(const std::string& service, const std::string& method, PyCallArgs& args);

    /**
     * @brief Dispatches a call to the requested bound service and method and returns the result
     */
    PyResult Dispatch(const BoundID& service, const std::string& method, PyCallArgs& args);
    
    /**
     * @brief Destroys the specified bound object
     */
    void ClearBoundObject(const BoundID& service);

    /**
     * @brief Gives access to the service list
     */
    const ServicesMap& GetServices() const;

    /** @returns int The nodeID for the server */
    NodeID GetNodeID() { return this->mNodeId; }

private:
    ServicesMap mServices;
    BoundServicesMap mBound;
    
    BoundID mLastBoundId;
    NodeID mNodeId;
};

#endif /* !__SERVICEMANAGER_H__ */