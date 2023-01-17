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

#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <map>
#include <string>
#include <vector>
#include <optional>

#include "eve-server.h"
#include "services/Callable.h"

template<class T>
class Service;
class Dispatcher;
class Client;

/**
 * @brief Base class used to abstract the dispatch part of the services
 * 
 * Used by the service manager to dispatch calls
 */
class Dispatcher {
public:
    /** Indicates what access level the service has to prevent clients from calling any methods when they shouldn't */
    virtual const AccessLevel GetAccessLevel() const = 0;
    /** Indicates the name of the service */
    virtual const std::string& GetName() const = 0;
    /**
     * @brief Handles dispatching a call to this service
     */
    virtual PyResult Dispatch(const std::string& name, PyCallArgs& args) = 0;
};

/**
 * @brief Represents an EVE Online service that exposes methods for clients to call
 * 
 * @author Almamu
 */
template<class T>
class Service : public Dispatcher {
protected:
    Service(const std::string& name, AccessLevel level = eAccessLevel_None) :
        mName(std::move(name)),
        mAccessLevel(level)
    {
    }

    /**
     * @brief Registers a method handler
     */
    void Add(const std::string& name, CallHandler <T> handler) {
        this->mHandlers.push_back(std::make_pair(std::string(name), handler));
    }

public:
    /** Indicates what access level the service has to prevent clients from calling any methods when they shouldn't */
    const AccessLevel GetAccessLevel() const override { return this->mAccessLevel; }
    /** Indicates the name of the service */
    const std::string& GetName() const override { return this->mName; }

    /**
     * @brief Handles dispatching a call to this service
     */
    PyResult Dispatch(const std::string& name, PyCallArgs& args) override {
        for (auto handler : this->mHandlers) {
            if (handler.first != name)
                continue;

            try
            {
                return handler.second(reinterpret_cast <T*> (this), args);
            }
            catch (std::invalid_argument)
            {
                // ignored, this just means the function does not match the possible calls
            }
        }

        throw method_not_found ();
    }

private:
    /** @var The name of the service */
    std::string mName;
    /** @var The access level required to access this service */
    AccessLevel mAccessLevel;
    /** @var The map of handlers for this service */
    std::vector <std::pair <std::string, CallHandler <T>>> mHandlers;
};

#endif /* !__SERVICE_H__ */