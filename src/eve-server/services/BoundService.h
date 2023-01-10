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

#include "Service.h"
#include "ServiceManager.h"


class BoundDispatcher : public Dispatcher {
public:
    /**
     * @brief Check performed before dispatching a call to this bound service
     */
    virtual bool CanClientCall(Client* client) = 0;
    /** @returns BoundID The id of the bound service */
    virtual const BoundID GetBoundID() = 0;
    /** @returns PyRep* The information of this bound object */
    virtual const PyRep* GetBoundData() = 0;
    /** @returns PyTuple* The OID of the bound object */
    virtual const PyTuple* GetOID() = 0;
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
    BindableService(const std::string& name, EVEServiceManager* mgr);

    PyResult MachoResolveObject(PyCallArgs& args, PyRep* bindParameters, PyRep* justQuery);
    PyResult MachoBindObject(PyCallArgs& args, PyRep* bindParameters, std::optional<PyTuple*> call);

protected:
    /**
     * @brief Handles the creation of the bound service
     */
    virtual BoundDispatcher* BindObject(PyRep* bindParameters) = 0;

private:
    EVEServiceManager* mManager;
};

/*
 * @brief Implementation of EVE Online's bound objects
 */
template<class T>
class EVEBoundObject : public Service<T>, public BoundDispatcher {
public:
    EVEBoundObject(EVEServiceManager* mgr, PyRep* bindData);

    /**
     * @brief Handles dispatching a call to this service
     */
    PyResult Dispatch(const std::string& name, PyCallArgs& args) override;

    /** @returns BoundID The id of the bound service */
    const BoundID GetBoundID() override { return this->mBoundId; }
    /** @returns PyRep* The information of this bound object */
    const PyRep* GetBoundData() override { return this->mBoundData; }
    /** @returns PyTuple* The OID of the bound object */
    const PyTuple* GetOID() override { return this->mOID; }
private:
    EVEServiceManager* mManager;
    PyTuple* mOID;
    std::string mIdString;
    PyRep* mBoundData;
    BoundID mBoundId;
};

#endif /* !__BOUNDSERVICE_H__ */