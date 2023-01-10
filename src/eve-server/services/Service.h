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

#include "python/PyRep.h"
#include "PyCallable.h"

template<class T>
class Service;

enum AccessLevel {
    eAccessLevel_None = 0,
    eAccessLevel_Location = 1,
    eAccessLevel_LocationPreferred = 2,
    eAccessLevel_SolarSystem = 3,
    eAccessLevel_Station = 4
};

template <class T> struct is_optional : std::false_type {};
template <class T> struct is_optional<std::optional<T>> : std::true_type {};

template <class S>
struct CallHandler {
    template<class... Args> CallHandler(PyResult(S::*callHandler)(PyCallArgs&, Args...)) :
        erasedHandler(reinterpret_cast <PyResult(S::*)()> (callHandler)),
        handlerImpl {
            [this](S* service, PyResult(S::*erasedHandler)(), PyCallArgs& args) -> PyResult {
                auto handler = reinterpret_cast <PyResult(S::*)(PyCallArgs&, Args...)> (erasedHandler);

                if constexpr (sizeof...(Args) == 0) {
                    // allow calls with more than required arguments to go through
                    // so methods can be implemented even if we don't care about the extra info
                    return (service->*handler) (args);
                } else {
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
    template <typename T> bool validateArg(size_t index, PyCallArgs& args) {
        // handle optional values
        if constexpr (is_optional <T>::value) {
            // treat Nones on optional values as valid
            if (index >= args.tuple->size() || args.tuple->GetItem(index)->IsNone()) {
                return true;
            } else {
                return validateArg <typename T::value_type>(index, args);
            }
        }

        if (index >= args.tuple->size()) {
            return false;
        }

        if constexpr (std::is_same_v <T, PyRep*>)
            return true;

        PyRep* rep = args.tuple->GetItem(index);

        // validate type with their parameter equivalent
        if constexpr (std::is_same_v <T, PyBool*>)
            return rep->IsBool();
        if constexpr (std::is_same_v <T, PyInt*>)
            return rep->IsInt();
        if constexpr (std::is_same_v <T, PyLong*>)
            return rep->IsLong();
        if constexpr (std::is_same_v <T, PyFloat*>)
            return rep->IsFloat();
        if constexpr (std::is_same_v <T, PyBuffer*>)
            return rep->IsBuffer();
        if constexpr (std::is_same_v <T, PyString*>)
            return rep->IsString();
        if constexpr (std::is_same_v <T, PyToken*>)
            return rep->IsToken();
        if constexpr (std::is_same_v <T, PyTuple*>)
            return rep->IsTuple();
        if constexpr (std::is_same_v <T, PyList*>)
            return rep->IsList();
        if constexpr (std::is_same_v <T, PyDict*>)
            return rep->IsDict();
        if constexpr (std::is_same_v <T, PyNone*>)
            return rep->IsNone();
        if constexpr (std::is_same_v <T, PySubStruct*>)
            return rep->IsSubStruct();
        if constexpr (std::is_same_v <T, PySubStream*>)
            return rep->IsSubStream();
        if constexpr (std::is_same_v <T, PyChecksumedStream*>)
            return rep->IsChecksumedStream();
        if constexpr (std::is_same_v <T, PyObject*>)
            return rep->IsObject();
        if constexpr (std::is_same_v <T, PyObjectEx*>)
            return rep->IsObjectEx();
        if constexpr (std::is_same_v <T, PyPackedRow*>)
            return rep->IsPackedRow();

        return false;
    }

    template <typename T> decltype(auto) getAs(size_t index, PyTuple* tup) {
        if constexpr (is_optional <T>::value) {
            if (index >= tup->size() || tup->GetItem(index)->IsNone()) {
                return T{};
            } else {
                return std::make_optional(getAs <typename T::value_type>(index, tup));
            }
        }

        if (index >= tup->size()) {
            throw std::runtime_error("This should not happen. Trying to get parameter out of bounds. What happened to the validation?!");
        }

        PyRep* rep = tup->GetItem(index);

        if constexpr (std::is_same_v <T, PyRep*>)
            return rep;
        if constexpr (std::is_same_v <T, PyBool*>)
            return rep->AsBool();
        if constexpr (std::is_same_v <T, PyInt*>)
            return rep->AsInt();
        if constexpr (std::is_same_v <T, PyLong*>)
            return rep->AsLong();
        if constexpr (std::is_same_v <T, PyFloat*>)
            return rep->AsFloat();
        if constexpr (std::is_same_v <T, PyBuffer*>)
            return rep->AsBuffer();
        if constexpr (std::is_same_v <T, PyString*>)
            return rep->AsString();
        if constexpr (std::is_same_v <T, PyToken*>)
            return rep->AsToken();
        if constexpr (std::is_same_v <T, PyTuple*>)
            return rep->AsTuple();
        if constexpr (std::is_same_v <T, PyList*>)
            return rep->AsList();
        if constexpr (std::is_same_v <T, PyDict*>)
            return rep->AsDict();
        if constexpr (std::is_same_v <T, PyNone*>)
            return rep->AsNone();
        if constexpr (std::is_same_v <T, PySubStruct*>)
            return rep->AsSubStruct();
        if constexpr (std::is_same_v <T, PySubStream*>)
            return rep->AsSubStream();
        if constexpr (std::is_same_v <T, PyChecksumedStream*>)
            return rep->AsChecksumedStream();
        if constexpr (std::is_same_v <T, PyObject*>)
            return rep->AsObject();
        if constexpr (std::is_same_v <T, PyObjectEx*>)
            return rep->AsObjectEx();
        if constexpr (std::is_same_v <T, PyPackedRow*>)
            return rep->AsPackedRow();
    }

    template<class... Args>
    bool validateArgs(PyCallArgs& args) {
        constexpr size_t ArgCount = sizeof... (Args);
        const bool argCountIsValid = args.tuple->size() <= ArgCount;

        if (argCountIsValid == false) {
            return false;
        }

        bool isValid = true;
        size_t index = 0;

        ((
            isValid = validateArg<Args>(index, args) && isValid,
            ++index
            ), ...);

        return isValid;
    }

    template<class... Args, size_t... I>
    PyResult applyImpl(
        S* service,
        PyResult(S::*handler)(PyCallArgs& args, Args...),
        PyCallArgs& args,
        std::index_sequence<I...>) {
        return (service->*handler) (args, getAs<std::decay_t<Args>>(I, args.tuple)...);
    }

    template<class... Args>
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

        throw std::runtime_error("Cannot find matching function to call");
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