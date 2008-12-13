/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "EvemuPCH.h"

PyCallable_Make_InnerDispatcher(AlertService)

AlertService::AlertService(PyServiceMgr *mgr) : PyService(mgr, "alert"), m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	m_dispatch->RegisterCall("BeanCount", &AlertService::Handle_BeanCount);
	m_dispatch->RegisterCall("BeanDelivery", &AlertService::Handle_BeanDelivery);
	m_dispatch->RegisterCall("SendClientStackTraceAlert", &AlertService::Handle_SendClientStackTraceAlert);
}

AlertService::~AlertService()
{
	delete m_dispatch;
}

/* please only enable this when your a developer who knows how to use it. Its NOT suitable for normal evemu users */
//#define DEV_DEBUG_TREAT

/** Basically BeanCount means that a error has accured in the client python code, and it asks
  * the server how to handle it.
  *\Note: in normal operations we should send back a unique errorID to the client, it saves it and sends the stack traces
  *\      to us through BeanDelivery every 15 minutes. When we are in developer mode we should send back PyNone asking the
  *\      to send us the stack trace immediately.
  */
PyResult AlertService::Handle_BeanCount(PyCallArgs &call) {

	PyRepTuple *result = new PyRepTuple(2);

	// what we are sending back is just a static errorID and the command not to do anything with it.
#ifdef DEV_DEBUG_TREAT
	result->items[0] = new PyRepNone();			//errorID
#else
	result->items[0] = new PyRepInteger(34135);	//errorID
#endif//DEV_DEBUG_TREAT
	
	result->items[1] = new PyRepInteger(1);//0		//loggingMode, 0=local, 1=DB

	return (PyRep*)result;
}

/** The client "stacks" up the python "stack" traces and sends them every 15 minutes.
  *\Note: this process is only usefully when we supply the client with a valid errorID.
  *\      meaning that we should code a errorID tracker for it. To handle these.
  */
PyResult AlertService::Handle_BeanDelivery(PyCallArgs &call) {
	PyRep *result = NULL;

	//_log(CLIENT__ERROR, "Unhandled BeanDelivery!");

	result = new PyRepNone();
	
	return result;
}

/** The client sends us a python stack trace, from which we could make up what we did wrong.
  *\Note: I'm sending PyNone back, this is just a wild guess. I don't know its actually required.
  *\Note: function is part of a system that allows us to ask the client to send the trace directly,
  *\      and skip the BeanDelivery system.
  */
PyResult AlertService::Handle_SendClientStackTraceAlert(PyCallArgs &call) {

#ifdef DEV_DEBUG_TREAT
	/* TODO: write a proper track trace dump class, to store and handle these traces */
	printf("SendClientStackTraceAlert:\n");
	call.Dump(CLIENT__ERROR);
#endif//DEV_DEBUG_TREAT

	PyRep *result = new PyRepNone();
	return result;
}
