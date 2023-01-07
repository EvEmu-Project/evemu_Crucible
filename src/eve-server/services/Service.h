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

#include "python/PyRep.h"
#include "PyCallable.h"

template<class T>
class Service;

enum AccessLevel {
    None = 0,
    Location = 1,
    LocationPreferred = 2,
    SolarSystem = 3,
    Station = 4
};

template <class S>
struct CallHandler {
    template<typename... Args> CallHandler(PyResult(S::*callHandler)(PyCallArgs&, Args...)) :
        erasedHandler(reinterpret_cast <PyResult(S::*)()> (callHandler)),
        handlerImpl {
            [this](S* service, PyResult(S::*erasedHandler)(), PyCallArgs& args) -> PyResult {
                auto handler = reinterpret_cast <PyResult(S::*)(PyCallArgs&, Args...)> (erasedHandler);

                if constexpr (sizeof...(Args) == 0) {
                    // ensure theres no arguments in the data
                    if (args.tuple->size() != 0)
                        throw std::invalid_argument("expected 0 arguments");

                    // all the functions must have the PyCallArgs to know important info about the call
                    // like the client that sent it
                    return (service->*handler) (args);
                } else {
                    // TODO: validate parameters
                    if (validateArgs <std::decay_t<Args>...>(args) == false)
                        throw std::invalid_argument("arguments do not match");

                    return apply(service, handler, args);
                }
            }
        }
    { }

    PyResult operator() (S* service, PyCallArgs& args) const {
        return handlerImpl(service, erasedHandler, args);
    }

private:
    template<typename... Args>
    bool validateArgs(PyCallArgs& args) {
        constexpr size_t ArgCount = sizeof... (Args);
        const bool argCountIsValid = args.tuple->size() == ArgCount;

        if (argCountIsValid == false) {
            return false;
        }

        bool isValid = true;
        size_t index = 0;

        ((
            isValid = index < args.tuple->size() && validateArg<Args>(index, args) && isValid,
            ++index
            ), ...);

        return isValid;
    }

    template <typename T>
    bool validateArg(size_t index, PyCallArgs& args) {
        // TODO: implement all Py types checks here
        return (std::is_same <T, PyBool*>() && args.tuple->GetItem(index)->IsBool()) ||
               (std::is_same <T, PyInt*>() && args.tuple->GetItem(index)->IsInt()) ||
               (std::is_same <T, PyLong*>() && args.tuple->GetItem(index)->IsLong()) ||
               (std::is_same <T, PyFloat*>() && args.tuple->GetItem(index)->IsFloat());
    }

    template<typename T>
    decltype(auto) getAs(PyRep* v) {
        if constexpr (std::is_same_v <T, PyBool*> == true)
            return v->AsBool();
        else if constexpr (std::is_same_v <T, PyInt*> == true)
            return v->AsInt();
        else if constexpr (std::is_same_v <T, PyLong*> == true)
            return v->AsLong();
        else if constexpr (std::is_same_v <T, PyFloat*> == true)
            return v->AsFloat();
    }

    template<typename... Args, size_t... I>
    PyResult applyImpl(
        S* service,
        PyResult(S::*handler)(PyCallArgs& args, Args...),
        PyCallArgs& args,
        std::index_sequence<I...>) {
        return (service->*handler) (args, getAs<std::decay_t<Args>>(args.tuple->GetItem(I))...);
    }

    template<typename... Args>
    PyResult apply(
        S* service,
        PyResult(S::*handler) (PyCallArgs& args, Args...),
        PyCallArgs& args) {
        return applyImpl(
            service, handler, args,
            std::make_index_sequence <sizeof...(Args)>{}
        );
    }

    PyResult(S::*erasedHandler)() = nullptr;
    std::function <PyResult(S* service, PyResult(S::* erasedHandler)(), PyCallArgs& args)> handlerImpl;
};

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
    Service(const std::string& name, AccessLevel level = AccessLevel::None);

    /**
     * @brief Registers a method handler
     */
    void add (const std::string& name, CallHandler <T> handler);

public:

    /** Indicates what access level the service has to prevent clients from calling any methods when they shouldn't */
    const AccessLevel GetAccessLevel() const override;
    /** Indicates the name of the service */
    const std::string& GetName() const override;

    /**
     * @brief Handles dispatching a call to this service
     */
    PyResult Dispatch(const std::string& name, PyCallArgs& args) override;

private:
    /** @var The name of the service */
    std::string mName;
    /** @var The access level required to access this service */
    AccessLevel mAccessLevel;
    /** @var The map of handlers for this service */
    std::vector <std::pair <std::string, CallHandler <T>>> mHandlers;
};


class MachoNetServiceTest : public Service<MachoNetServiceTest> {
public:
    MachoNetServiceTest();

    PyResult test(PyCallArgs& args, PyBool* boolean);
};

template class Service<MachoNetServiceTest>;
template struct CallHandler<MachoNetServiceTest>;

#endif /* !__SERVICE_H__ */