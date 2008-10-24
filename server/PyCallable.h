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


#ifndef __PYCALLABLE_H__
#define __PYCALLABLE_H__

#include <string>
#include <map>
#include "../common/counted_ptr.h"
#include "../common/logsys.h"
#include "ServiceDB.h"
#include "../common/EVEUtils.h"

class Client;

class PyRep;
class PyRepTuple;
class PyRepDict;

class PyServiceMgr;
class PyCallStream;




class PyCallArgs {
public:
	PyCallArgs(Client *c, PyRepTuple **tup, PyRepDict **dict);
	~PyCallArgs();

	void Dump(LogType type) const;

	Client *const client;	//we do not own this
	PyRepTuple *tuple;		//we own this, but it may be taken
	std::map<std::string, PyRep *> byname;	//we own this, but elements may be taken.
};




class PyResult {
protected:
	PyResult();
public:
	PyResult(PyRep *result);	//takes ownership
	~PyResult();
	
	//I dislike smart pointers, but I need them here due to copying when returning objects
	counted_ptr<PyRep> ssResult;	//must never be NULL
};

class PyException {
protected:
	PyException();
public:
	PyException(PyRep *except);		//takes ownership
	~PyException();

	counted_ptr<PyRep> ssException;	//must never be NULL
};





class PyCallable {
public:
	class CallDispatcher {
	public:
		virtual ~CallDispatcher() {}
		virtual PyResult Dispatch(const std::string &method_name, PyCallArgs &call) = 0;
	};
	
	PyCallable(PyServiceMgr *mgr, const char *callableName);
	virtual ~PyCallable();

	const char *GetName() const { return(m_callableName.c_str()); }
	
	//returns ownership:
	virtual PyResult Call(PyCallStream &call, PyCallArgs &args);
	
protected:
	void _SetCallDispatcher(CallDispatcher *d) { m_serviceDispatch = d; }
	
	PyServiceMgr *const m_manager;
private:
	const std::string m_callableName;
	CallDispatcher *m_serviceDispatch;	//must not be NULL after constructor, we do not own this.
};

#endif // __PYCALLABLE_H__
