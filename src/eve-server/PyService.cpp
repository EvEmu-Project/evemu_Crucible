/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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
    Author:        Zhur
*/

#include "eve-server.h"

#include "Client.h"
#include "PyBoundObject.h"
#include "PyService.h"

PyService::PyService(PyServiceMgr *mgr, const char *serviceName)
: m_manager(mgr),
  m_name(serviceName)
{
}

PyService::~PyService()
{
}

//overload this to hack in our special bind routines at the service level
PyResult PyService::Call(const std::string &method, PyCallArgs &args) {
    if (method == "MachoResolveObject"){
        _log(SERVICE__CALLS, "%s::MachoResolveObject()", GetName());
        return Handle_MachoResolveObject(args);
    } else if (method == "MachoBindObject") {
        _log(SERVICE__CALLS, "%s::MachoBindObject()", GetName());
        return Handle_MachoBindObject(args);
    } else {
        _log(SERVICE__CALLS, "%s::%s()", GetName(), method.c_str());
        args.Dump(SERVICE__CALL_TRACE);
        return PyCallable::Call(method, args);
    }
}


/*
 * For now, we are going to keep it simple by trying to use only a single
 * node ID, this may have to change some day.
*/

PyResult PyService::Handle_MachoResolveObject(PyCallArgs &call) {
    //returns nodeID
    _log(SERVICE__MESSAGE, "%s Service: MachoResolveObject requested, returning %u", GetName(), m_manager->GetNodeID());
    return new PyInt(m_manager->GetNodeID());
}


PyResult PyService::Handle_MachoBindObject( PyCallArgs& call )
{
    CallMachoBindObject args;
    if (!args.Decode(&call.tuple)) {
        codelog( SERVICE__ERROR, "%s Service: Failed to decode arguments", GetName() );
        return nullptr;
    }

    _log( SERVICE__MESSAGE, "%s Service: Processing MachoBindObject", GetName() );

    //first we need to get our implementation to actually create the object they are trying to bind to.
    PyBoundObject* obj = CreateBoundObject(call.client, args.bindParams);
    if (obj == nullptr) {
        _log( SERVICE__ERROR, "%s Service: Unable to create bound object:", GetName());
        args.bindParams->Dump(SERVICE__ERROR, "    ");

        return nullptr;
    }

    //now we register
    PyTuple* rsp = new PyTuple(2);
    PyDict* oid = new PyDict();
    rsp->SetItem(0, m_manager->BindObject(call.client, obj, nullptr, oid));

    if (args.call->IsNone()) {
        //no call was specified...
        rsp->SetItem(1, PyStatic.NewNone());
    } else {
        CallMachoBindObject_call boundcall;
        if (!boundcall.Decode(&args.call)) {
            codelog(SERVICE__ERROR, "%s Service: Failed to decode boundcall arguments", GetName());
            return nullptr;
        }

        _log(SERVICE__MESSAGE, "%s Service: MachoBindObject also contains call to %s", GetName(), boundcall.method_name.c_str());

        PyCallArgs sub_args(call.client, boundcall.arguments, boundcall.dict_arguments);

        //do the call:
        PyResult result = obj->Call(boundcall.method_name, sub_args);

        rsp->SetItem(1, result.ssResult);
    }

    return PyResult(rsp, oid);
}

const char *const PyService::s_checkTimeStrings[_checkCount] = {
     "always",
     "never",
     "year",
     "6 months",
     "3 months",
     "month",
     "week",
     "day",
     "12 hours",
     "6 hours",
     "3 hours",
     "2 hours",
     "1 hour",
     "30 minutes",
     "15 minutes",
     "5 minutes",
     "1 minute",
     "30 seconds",
     "15 seconds",
     "5 seconds",
     "1 second"
};

/* this is untested... */
PyObject *PyService::_BuildCachedReturn( PySubStream** in_result, const char* sessionInfo, CacheCheckTime check )
{
    objectCaching_CachedMethodCallResult cached;

    PySubStream* result = *in_result;
    *in_result = nullptr;        //consume it.

    //we need to checksum the marshaled data...
    result->EncodeData();
    if (!result->data()) {
        _log( SERVICE__ERROR, "%s: Failed to build cached return", GetName() );

        PyDecRef( result );
        return nullptr;
    }

    cached.call_return = result;    //this entire result is going to get cloned in the Encode(), and then destroyed when we return... what a waste...
    cached.sessionInfo = sessionInfo;
    cached.clientWhen = s_checkTimeStrings[ check ];

    cached.timeStamp = Win32TimeNow();
    //we can use whatever checksum we want here, as the client just remembers it and sends it back to us.
    cached.version = crc_hqx( &result->data()->content()[0], result->data()->content().size(), 0 );

    return cached.Encode();
}

PyBoundObject * PyService::CreateBoundObject( Client *c, const PyRep *bind_args )
{
    _log( SERVICE__ERROR, "Called default CreateBoundObject()");
    EvE::traceStack();
    return nullptr;
}
