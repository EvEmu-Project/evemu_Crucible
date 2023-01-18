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
    Author:        Zhur, Captnoord
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "admin/AlertService.h"

AlertService::AlertService() :
    Service("alert"),
    traceLogger(nullptr)
{
    this->Add("BeanCount", &AlertService::BeanCount);
    this->Add("BeanDelivery", &AlertService::BeanDelivery);
    this->Add("GroupBeanDelivery", &AlertService::GroupBeanDelivery);
    this->Add("SendClientStackTraceAlert", &AlertService::SendClientStackTraceAlert);

    if (sConfig.debug.StackTrace or is_log_enabled(CLIENT__STACK_TRACE))
        traceLogger = new PyTraceLog("evemu_client_stack_trace.txt", true, true);
}

AlertService::~AlertService()
{
    SafeDelete(traceLogger);
}

/** Basically BeanCount means that a error has occurred in the client python code, and it asks
  * the server how to handle it.
  * @Note: in normal operations we should send back a unique mErrorID to the client, it saves it and sends the stack traces
  *      to us through BeanDelivery every 15 minutes. When we are in developer mode we should send back PyNone asking the
  *      to send us the stack trace immediately.
  */
PyResult AlertService::BeanCount(PyCallArgs &call, PyRep* ignored) {
    _log(CLIENT__WARNING, "AlertService::Handle_BeanCount(): size=%lli", call.tuple->size());
    //call.Dump(CLIENT__CALL_DUMP);

    PyTuple *result = new PyTuple(2);

    // what we are sending back is just a static mErrorID and the command not to do anything with it.
    if (sConfig.debug.BeanCount or sConfig.debug.IsTestServer) {
        result->items[0] = PyStatic.NewNone();
    } else {
        result->items[0] = new PyInt(34135);    //ErrorID
    }

    result->items[1] = new PyInt(0);        //loggingMode, 0=local, 1=DB (Capt: This isn't correct at all as it seems..)

    return (PyRep*)result;
}

/** The client "stacks" up the python "stack" traces and sends them every 15 minutes.
  * @Note: this process is only useful when we supply the client with a valid mErrorID.
  *      meaning that we should code a mErrorID tracker for it. To handle these.
  */
// note:  this is a rather complicated system....
PyResult AlertService::BeanDelivery(PyCallArgs& call, PyList* beans)
{
    _log(CLIENT__WARNING, "AlertService::Handle_BeanDelivery(): size=%lli", call.tuple->size());
    //call.Dump(CLIENT__CALL_DUMP);
    /* Unhandled for now as we have no interest in receiving batched python stack traces
     * nor official style debugging... Just gimme the info dude (see Handle_SendClientStackTraceAlert).
     */
    return PyStatic.NewNone();
}

PyResult AlertService::GroupBeanDelivery(PyCallArgs& call, PyBuffer* compressedBeans)
{
    _log(CLIENT__WARNING, "AlertService::Handle_GroupBeanDelivery(): size=%u", call.tuple->size() );
    //call.Dump(CLIENT__CALL_DUMP);

    return PyStatic.NewNone();
}

/**
 * @brief The client sends us a python stack trace, from which we could make up what we did wrong.
 *
 * @param[in] call is the python packet that contains the info for this function
 *
 * @note I'm sending PyNone back, this is just a wild guess. I don't know its actually required.
 * function is part of a system that allows us to ask the client to send the trace directly,
 * and skip the BeanDelivery system.
 * @return guess it should have PyNone back.
 */
PyResult AlertService::SendClientStackTraceAlert(PyCallArgs &call, PyTuple* stackId, PyString* stackTrace, PyString* mode, PyRep* nextErrorKeyHash) {
    _log(CLIENT__WARNING, "AlertService::Handle_SendClientStackTraceAlert(): size=%lli", call.tuple->size());
    //call.Dump(CLIENT__CALL_DUMP);
    //  self.stacktraceLogMode[stackID[0]] = sm.ProxySvc('alert').SendClientStackTraceAlert(stackID, stackTrace, mode, nextErrorKeyHash)

  if (sConfig.debug.StackTrace or is_log_enabled(CLIENT__STACK_TRACE))
    traceLogger->logTrace(*call.tuple);

    return PyStatic.NewNone();
}
