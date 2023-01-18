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

#include "services/Callable.h"
#include "services/Service.h"
#include "Client.h"

typedef uint32_t BoundID;

template<class T> class Service;
class EVEServiceManager;
class PyResult;
class PyCallArgs;
class Client;

class BoundDispatcher {
public:
    /** @returns BoundID The id of the bound service */
    virtual BoundID GetBoundID() const = 0;
    /** @returns PyTuple* The OID of the bound object */
    virtual PyTuple* GetOID() const = 0;
    /** @returns The string ID */
    virtual const std::string& GetIDString() const = 0;
    /**
     * @brief Handles dispatching a call to this service
     */
    virtual PyResult Dispatch(const std::string& name, PyCallArgs& args) = 0;
    /**
     * @brief Builds a string with information about calling a method in this service
     */
    virtual std::string DebugDispatch (const std::string& name) = 0;
    /**
     * @brief Increases the number of references to this bound object
     */
    virtual void NewReference (Client* client) = 0;
    /**
     * @brief Releases this dispatcher and frees any kept resources
     * @returns Whether the bound object was destroyed or someone still has a reference to it
     */
    virtual bool Release (Client* client) = 0;
protected:
    /**
     * @brief Check performed before dispatching a call to this bound service
     */
    virtual bool CanClientCall(Client* client) = 0;
};
/**
 * Parent for any of the bound services
 *
 * @tparam Bound
 */
template <class Bound>
class BoundServiceParent {
public:
    /**
     * Called by the bound services when It's getting removed
     *
     * @param bound
     */
    virtual void BoundReleased (Bound* bound) = 0;
};

/**
 * @brief Service that exposes the mechanism for binding services.
 * 
 * Due to how EVE requests these bindings sometimes call info is included,
 * so this class provides all the required mechanisms to handle the whole proccess
 */
template<class Svc, class Bound>
class BindableService : public Service<Svc>, public BoundServiceParent<Bound> {
public:
    BindableService(const std::string& name, EVEServiceManager& mgr, AccessLevel level = eAccessLevel_None) :
        Service <Svc>(name, level),
        mManager(mgr)
    {
        this->Add("MachoResolveObject", &BindableService<Svc, Bound>::MachoResolveObject);
        this->Add("MachoBindObject", &BindableService<Svc, Bound>::MachoBindObject);
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

        // add the client to the allowed list
        bound->NewReference (args.client);

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

/**
 * @brief Implementation of EVE Online's bound objects
 * 
 * @see https://learn.microsoft.com/en-us/windows/win32/com/monikers
 */
template<class Bound>
class EVEBoundObject : public BoundDispatcher {
protected:
    EVEBoundObject(EVEServiceManager& mgr, BoundServiceParent<Bound>& parent) :
        mManager(mgr),
        mParent (parent)
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
     * Default destructor, override to free resources if needed
     */
    virtual ~EVEBoundObject () {
    }

    /**
     * @brief Registers a method handler
     */
    template <class H, class... Args>
    void Add(const std::string& name, PyResult(H::*callHandler)(PyCallArgs&, Args...)) {
        this->mHandlers.push_back(std::make_pair(std::string(name), new CallHandler <H> (callHandler)));
    }

public:
    /**
     * @brief Handles dispatching a call to this service
     */
    PyResult Dispatch(const std::string& name, PyCallArgs& args) override {
        if (this->CanClientCall(args.client) == false)
            throw CustomError("This client is not allowed to call this bound service");

        for (auto handler : this->mHandlers) {
            if (handler.first != name)
                continue;

            try
            {
                return (*handler.second)(reinterpret_cast <void*> (this), args);
            }
            catch (std::invalid_argument)
            {
                // ignored, this just means the function does not match the possible calls
            }
        }

        throw method_not_found ();
    }

    /**
     * @brief Builds a string with information about calling a method in this service
     */
    std::string DebugDispatch (const std::string& name) override {
        std::string result = name + " candidates: \n";

        for (auto handler : this->mHandlers) {
            if (handler.first != name)
                continue;

            result += "\t(" + handler.second->getSignature () + ")";
            result += "\n";
        }

        return result;
    }

    /**
     * @brief Increases the number of references to this bound object
     */
    void NewReference (Client* client) override {
        // ensure the client is not there yet
        auto it = this->mClients.find (client);

        if (it != this->mClients.end ())
            return;

        // the client didn't hold a reference to this service
        // so add it to the list and increase the RefCount
        this->mClients.insert_or_assign (client, true);
        // also add it to the bind list of the client
        client->AddBindID (this->GetBoundID ());
    }
    /**
     * @brief Signals this EVEBoundObject that one of the clients that held a reference, released it
     * @returns Whether the bound object was destroyed or someone still has a reference to it
     */
    bool Release(Client* client) override {
        auto it = this->mClients.find (client);

        // the client doesn't have access to this bound service, so nothing has to be done
        if (it == this->mClients.end ())
            return false;

        // remove the client for the list, and if that's the last one, free the service
        this->mClients.erase (it);

        if (this->mClients.size () == 0) {
            this->GetParent ().BoundReleased (reinterpret_cast <Bound*> (this));
            delete this; // we hate this
            return true;
        }

        return false;
    }

    bool CanClientCall(Client* client) override {
        return this->mClients.find (client) != this->mClients.end();
    }

    /** @returns BoundID The id of the bound service */
    BoundID GetBoundID() const override { return this->mBoundId; }
    /** @returns The normal service that created this service */
    BoundServiceParent<Bound>& GetParent () const { return this->mParent; }
    /** @returns PyTuple* The OID of the bound object */
    PyTuple* GetOID() const override { return this->mOID; }
    /** @returns The string ID */
    const std::string& GetIDString() const override { return this->mIdString; }
    /** @returns The service manager this service is registered in */
    EVEServiceManager& GetServiceManager() const { return this->mManager; }
    /** @returns The list of clients that requested access to this service */
    std::map <Client*, bool>& GetBoundClients () const { return this->m_clients; }
private:
    /** @var The service manager this bound service is registered in */
    EVEServiceManager& mManager;
    /** @var The OID of the service */
    PyTuple* mOID;
    /** @var The ID string that identifies this service for the clients */
    std::string mIdString;
    /** @var The normal service that created this bound object */
    BoundServiceParent<Bound>& mParent;
    /** @var The numeric ID of the bound service */
    BoundID mBoundId;
    /** @var The map of handlers for this service */
    std::vector <std::pair <std::string, CallHandlerBase*>> mHandlers;
    /** @var The clients that have access to this bound service */
    std::map <Client*, bool> mClients;
};

#endif /* !__BOUNDSERVICE_H__ */