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
	Author:		Zhur
*/

#ifndef __PYSERVICECD_H_INCL__
#define __PYSERVICECD_H_INCL__

#include "PyCallable.h"

/*                                                                              
 * This whole concept exists to allow the generic PyService to make a
 * call to the specific `Scv` object with ->*
 */
template <class Svc>
class PyCallableDispatcher
	: public PyCallable::CallDispatcher
{
	typedef PyResult (Svc::*CallProc)(PyCallArgs &call);
	typedef typename std::map<std::string, CallProc>::iterator mapitr;
public:
	PyCallableDispatcher(Svc *parent)
	: m_parent(parent) {
	}
	
	virtual ~PyCallableDispatcher() {
	}
	
	void RegisterCall(const char *call_name, CallProc p) {
		m_serviceCalls[call_name] = p;
	}

	//CallDispatcher interface:
	virtual PyResult Dispatch(const std::string &method_name, PyCallArgs &call) {
		//this could be done a lot more efficiently with a custom data structure IF NEEDED
		mapitr res;
		res = m_serviceCalls.find(method_name);
		if(res == m_serviceCalls.end()) {
			sLog.Error("Unknown call to '%s' by '%s'", method_name.c_str(), call.client->GetName());
			return NULL;
		}
		
		CallProc p = res->second;
		return (m_parent->*p)(call);
	}
	
protected:   //_MAY_ consume args
	std::map<std::string, CallProc> m_serviceCalls;

	Svc *const m_parent;	//we do not own this pointer
};

//convenience macro, you do not HAVE to use this
#define PyCallable_REG_CALL(c,m) m_dispatch->RegisterCall(#m, &c::Handle_##m);

//macro of a template... nice.
#define PyCallable_Make_Dispatcher(objname) \
	class Dispatcher : public PyCallableDispatcher<objname> { \
	public: \
		Dispatcher(objname *c) : PyCallableDispatcher<objname>(c) {} \
	};
	
#define PyCallable_Make_InnerDispatcher(objname) \
	class objname::Dispatcher \
	: public PyCallableDispatcher<objname> { \
	public: \
		Dispatcher(objname *c) \
		: PyCallableDispatcher<objname>(c) {} \
	};

#endif // __PYSERVICECD_H_INCL__
