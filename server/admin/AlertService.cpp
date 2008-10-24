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

AlertService::AlertService(PyServiceMgr *mgr)
: PyService(mgr, "alert"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(AlertService, BeanCount)
	PyCallable_REG_CALL(AlertService, BeanDelivery)
}

AlertService::~AlertService() {
	delete m_dispatch;
}


PyResult AlertService::Handle_BeanCount(PyCallArgs &call) {
	PyRep *result = NULL;

	PyRepTuple *tt;
	tt = new PyRepTuple(2);
	result = tt;
	
	tt->items[0] = new PyRepInteger(34135);	//errorID
	tt->items[1] = new PyRepInteger(0);		//loggingMode, 0=local, 1=DB
	
	_log(CLIENT__ERROR, "Got BeanCount request!");
	_log(CLIENT__ERROR, "Got BeanCount request!");
	_log(CLIENT__ERROR, "Got BeanCount request!");
	_log(CLIENT__ERROR, "Got BeanCount request!");
	_log(CLIENT__ERROR, "Got BeanCount request!");
	_log(CLIENT__ERROR, "Got BeanCount request!");
	_log(CLIENT__ERROR, "Got BeanCount request!");
	_log(CLIENT__ERROR, "Got BeanCount request!");
	
	return(result);
}

PyResult AlertService::Handle_BeanDelivery(PyCallArgs &call) {
	PyRep *result = NULL;
	/*                                                                              
     *
     * example contents:
     * 
----     Tuple: 1 elements
----       [ 0] Dictionary: 1 entries
----       [ 0]   [ 0] Key: Integer field: 34135
----       [ 0]   [ 0] Value: Tuple: 2 elements
----       [ 0]   [ 0] Value:   [ 0] Integer field: 2
----       [ 0]   [ 0] Value:   [ 1] Integer field: 127943740414147685

     *
    */
	_log(CLIENT__ERROR, "Unhandled BeanDelivery!");

	result = new PyRepNone();
	
	return(result);
}






















