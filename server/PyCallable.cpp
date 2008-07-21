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


#include "PyCallable.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "EntityList.h"
#include "PyServiceMgr.h"

PyCallable::PyCallable(PyServiceMgr *mgr, const char *callableName)
: m_manager(mgr),
  m_callableName(callableName),
  m_serviceDispatch(NULL)
{
}

PyCallable::~PyCallable()
{
}

PyCallResult PyCallable::Call(PyCallStream &call, PyCallArgs &args) {

	_log(SERVICE__CALLS, "%s Service: calling %s", m_callableName.c_str(), call.method.c_str());
	args.Dump(SERVICE__CALL_TRACE);
	
	//call the dispatcher, capturing the result.
	PyCallResult res = m_serviceDispatch->Dispatch(call.method, args);
	
	if(is_log_enabled(SERVICE__CALL_TRACE)) {
		switch(res.type) {
		case PyCallResult::RegularResult:
			_log(SERVICE__CALL_TRACE, "%s Service: Call %s returned:", m_callableName.c_str(), call.method.c_str());
			break;
		case PyCallResult::ThrowException:
			_log(SERVICE__CALL_TRACE, "%s Service: Call %s threw exception:", m_callableName.c_str(), call.method.c_str());
			break;
		//no default on purpose
		}
		res.ssResult->Dump(SERVICE__CALL_TRACE, "      ");
	}
	
	return(res);
}


PyCallArgs::PyCallArgs(Client *c, PyRepTuple **tup, PyRepDict **dict)
: client(c),
  tuple(*tup)
{
	*tup = NULL;
	
	PyRepDict *d = *dict;
	*dict = NULL;
	PyRepDict::iterator cur, end;
	cur = d->begin();
	end = d->end();
	for(; cur != end; cur++) {
		if(!cur->first->CheckType(PyRep::String)) {
			_log(SERVICE__ERROR, "Non-string key in call named arguments. Skipping.");
			cur->first->Dump(SERVICE__ERROR, "    ");
			continue;
		}
		PyRepString *s = (PyRepString *) cur->first;
		
		byname[s->value] = cur->second;
		cur->second = NULL;
	}
	//do not do anything with d except delete it, it has NULL pointers in it.
	delete d;
}

PyCallArgs::~PyCallArgs() {
	delete tuple;
	std::map<std::string, PyRep *>::iterator cur, end;
	cur = byname.begin();
	end = byname.end();
	for(; cur != end; cur++) {
		delete cur->second;
	}
}

void PyCallArgs::Dump(LogType type) const {
	if(!is_log_enabled(SERVICE__CALL_TRACE))
		return;
	
	_log(SERVICE__CALL_TRACE, "  Call Arguments:");
	tuple->Dump(stdout, "      ");
	if(!byname.empty()) {
		_log(SERVICE__CALL_TRACE, "  Call Named Arguments:");
		std::map<std::string, PyRep *>::const_iterator cur, end;
		cur = byname.begin();
		end = byname.end();
		for(; cur != end; cur++) {
			_log(SERVICE__CALL_TRACE, "    Argument '%s':", cur->first.c_str());
			cur->second->Dump(SERVICE__CALL_TRACE, "        ");
		}
	}
}

PyCallResult::PyCallResult()
: type(RegularResult),
  ssResult(NULL)
{
	_log(SERVER__INIT, "Constructing regular NULL");
}

PyCallResult::PyCallResult(PyRep *result)
: type(RegularResult),
  ssResult(
	  (result==NULL)
	  ? (new PyRepSubStream(new PyRepNone()))
	  : (new PyRepSubStream(result))
	  )
{
	_log(SERVER__INIT, "Constructing Regular %p", &(*ssResult));
}

PyCallResult::~PyCallResult() {
	_log(SERVER__INIT, "Destroying Regular %p", &(*ssResult));
}

PyCallRawResult::PyCallRawResult(PyRepSubStream *ss)
: m_ss(ss) {
	_log(SERVER__INIT, "Constructing Raw %p", &(*m_ss));
}

PyCallRawResult::~PyCallRawResult() {
	_log(SERVER__INIT, "Destroying Raw %p", &(*m_ss));
}

PyCallRawResult::operator PyCallResult() {
	_log(SERVER__INIT, "Casting Raw %p", &(*m_ss));
	PyCallResult res;
	res.ssResult = m_ss;
	res.type = PyCallResult::RegularResult;
	return(res);
}

PyCallException::PyCallException(PyRepObject *except)
: m_ss(new PyRepSubStream(except)) {
	_log(SERVER__INIT, "Constructing Exception %p", &(*m_ss));
}

PyCallException::~PyCallException() {
	_log(SERVER__INIT, "Destroying Exception %p", &(*m_ss));
}

PyCallException::operator PyCallResult() {
	_log(SERVER__INIT, "Casting Exception %p", &(*m_ss));
	PyCallResult res;
	res.ssResult = m_ss;
	res.type = PyCallResult::ThrowException;
	return(res);
}


























