/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2011 The EVEmu Team
	For the latest information visit http://evemu.org
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
	Author:		Zhur, Captnoord
*/

#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(AlertService)

AlertService::AlertService(PyServiceMgr *mgr) : PyService(mgr, "alert"), m_dispatch(new Dispatcher(this)), traceLogger(NULL)
{
	_SetCallDispatcher(m_dispatch);

	m_dispatch->RegisterCall("BeanCount", &AlertService::Handle_BeanCount);
	m_dispatch->RegisterCall("BeanDelivery", &AlertService::Handle_BeanDelivery);
	m_dispatch->RegisterCall("SendClientStackTraceAlert", &AlertService::Handle_SendClientStackTraceAlert);

#ifdef DEV_DEBUG_TREAT
	traceLogger = new PyTraceLog("evemu_client_stack_trace.txt", true, true);
#endif//DEV_DEBUG_TREAT
}

AlertService::~AlertService()
{
	delete m_dispatch;
}

/** Basically BeanCount means that a error has accured in the client python code, and it asks
  * the server how to handle it.
  * @Note: in normal operations we should send back a unique errorID to the client, it saves it and sends the stack traces
  *      to us through BeanDelivery every 15 minutes. When we are in developer mode we should send back PyNone asking the
  *      to send us the stack trace immediately.
  */
PyResult AlertService::Handle_BeanCount(PyCallArgs &call) {

	PyTuple *result = new PyTuple(2);

	// what we are sending back is just a static errorID and the command not to do anything with it.
#ifdef DEV_DEBUG_TREAT
	result->items[0] = new PyNone();			//errorID
#else
	result->items[0] = new PyInt(34135);	//errorID
#endif//DEV_DEBUG_TREAT
	
	result->items[1] = new PyInt(0);		//loggingMode, 0=local, 1=DB (Capt: This isn't correct at all as it seems..)

	return (PyRep*)result;
}

/** The client "stacks" up the python "stack" traces and sends them every 15 minutes.
  * @Note: this process is only usefully when we supply the client with a valid errorID.
  *      meaning that we should code a errorID tracker for it. To handle these.
  */
PyResult AlertService::Handle_BeanDelivery( PyCallArgs& call )
{
	/* Unhandled for now as we have no interest in receiving batched python stack traces
	 * nor official style debugging... Just gimme the info dude (see Handle_SendClientStackTraceAlert).
	 */
	return new PyNone;
}

/**
 * @brief The client sends us a python stack trace, from which we could make up what we did wrong.
 *
 * 
 *
 * @param[in] call is the python packet that contains the info for this function
 *
 * @note I'm sending PyNone back, this is just a wild guess. I don't know its actually required.
 * function is part of a system that allows us to ask the client to send the trace directly,
 * and skip the BeanDelivery system.
 * @return guess it should have PyNone back.
 */
PyResult AlertService::Handle_SendClientStackTraceAlert(PyCallArgs &call) {

#ifdef DEV_DEBUG_TREAT
	traceLogger->logTrace(*call.tuple);
#endif//DEV_DEBUG_TREAT
	return new PyNone();
}
