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

#ifndef __BOUNDSERVICE_H__
#define __BOUNDSERVICE_H__

typedef uint32_t BoundID;

class BoundDispatcher;
template<class T>
class BindableService;
template<class T>
class EVEBoundObject;

#include "Service.h"
#include "ServiceManager.h"

class BoundDispatcher {
public:
    /** @returns BoundID The id of the bound service */
    virtual BoundID GetBoundID() const = 0;
    /** @returns PyRep* The information of this bound object */
    virtual PyRep* GetBoundData() const = 0;
    /** @returns PyTuple* The OID of the bound object */
    virtual PyTuple* GetOID() const = 0;
    /** @returns The string ID */
    virtual const std::string& GetIDString() const = 0;
    /**
     * @brief Handles dispatching a call to this service
     */
    virtual PyResult Dispatch(const std::string& name, PyCallArgs& args) = 0;
    /**
     * @brief Releases this dispatcher and frees any kept resources
     */
    virtual void Release() = 0;
protected:
    /**
     * @brief Check performed before dispatching a call to this bound service
     */
    virtual bool CanClientCall(Client* client) = 0;
};

/**
 * @brief Service that exposes the mechanism for binding services.
 * 
 * Due to how EVE requests these bindings sometimes call info is included,
 * so this class provides all the required mechanisms to handle the whole proccess
 */
template<class T>
class BindableService : public Service<T> {
public:
    BindableService(const std::string& name, EVEServiceManager& mgr) :
        Service <T>(name),
        mManager(mgr)
    {
        this->Add("MachoResolveObject", &BindableService<T>::MachoResolveObject);
        this->Add("MachoBindObject", &BindableService<T>::MachoBindObject);
    }

    PyResult MachoResolveObject(PyCallArgs& args, PyRep* bindParameters, PyRep* justQuery) {
        return new PyInt(this->GetServiceManager().GetNodeID());
    }

    PyResult MachoBindObject(PyCallArgs& args, PyRep* bindParameters, std::optional<PyTuple*> call) {
        // register the new instance in the service manager
        BoundDispatcher* bound = this->BindObject(args.client, bindParameters);

        // binding failed for whatever reason, just return none and get on with our lifes
        if (bound == nullptr) {
            return nullptr;
        }

        // build the bound service identifier
        PyTuple* rsp = new PyTuple(2);
        PyDict* byName = new PyDict();

        byName->SetItem("OID+", bound->GetOID());

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

protected:
    /**
     * @brief Handles the creation of the bound service
     */
    virtual BoundDispatcher* BindObject(Client* client, PyRep* bindParameters) = 0;
    /** @returns The service manager this service is registered in */
    EVEServiceManager& GetServiceManager() const { return this->mManager; }
private:
    EVEServiceManager& mManager;
};

/*
 * @brief Implementation of EVE Online's bound objects
 * 
 * @see https://learn.microsoft.com/en-us/windows/win32/com/monikers
 */
template<class T>
class EVEBoundObject : public BoundDispatcher {
protected:
    EVEBoundObject(EVEServiceManager& mgr, PyRep* bindData) :
        mManager(mgr),
        mBoundData(bindData)
    {
        this->mBoundId = this->GetServiceManager().RegisterBoundService(this);

        // build the id string
        std::stringstream strBuilder;
        strBuilder << "N=" << this->GetServiceManager().GetNodeID() << ":" << this->mBoundId;

        // store it
        this->mIdString = strBuilder.str();

        // build the OID
        this->mOID = new PyTuple(2);
        this->mOID->SetItem (0, new PyString (this->mIdString));
        this->mOID->SetItem (1, new PyLong(GetFileTimeNow())); // this isn't really the datetime, should be a unique ID
    }

    /**
     * @brief Registers a method handler
     */
    void Add(const std::string& name, CallHandler <T> handler) {
        this->mHandlers.push_back(std::make_pair(std::string(name), handler));
    }

public:
    /**
     * @brief Handles dispatching a call to this service
     */
    PyResult Dispatch(const std::string& name, PyCallArgs& args) override {
        if (this->CanClientCall(args.client) == false)
            throw std::runtime_error("This client is not allowed to call this bound service");

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
    /**
     * @brief Releases this dispatcher and frees any kept resources
     */
    void Release() override {
        delete this; // we hate this
    }

    /** @returns BoundID The id of the bound service */
    BoundID GetBoundID() const override { return this->mBoundId; }
    /** @returns PyRep* The information of this bound object */
    PyRep* GetBoundData() const override { return this->mBoundData; }
    /** @returns PyTuple* The OID of the bound object */
    PyTuple* GetOID() const override { return this->mOID; }
    /** @returns The string ID */
    const std::string& GetIDString() const override { return this->mIdString; }
    /** @returns The service manager this service is registered in */
    EVEServiceManager& GetServiceManager() const { return this->mManager; }
private:
    /** @var The service manager this bound service is registered in */
    EVEServiceManager& mManager;
    /** @var The OID of the service */
    PyTuple* mOID;
    /** @var The ID string that identifies this service for the clients */
    std::string mIdString;
    /** @var The data used to request the binding of this service */
    PyRep* mBoundData;
    /** @var The numeric ID of the bound service */
    BoundID mBoundId;
    /** @var The map of handlers for this service */
    std::vector <std::pair <std::string, CallHandler <T>>> mHandlers;
};

#endif /* !__BOUNDSERVICE_H__ */