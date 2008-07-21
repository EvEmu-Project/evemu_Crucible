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


#ifndef __INVBROKER_SERVICE_H_INCL__
#define __INVBROKER_SERVICE_H_INCL__

#include "../PyService.h"

#include "InventoryDB.h"

class PyRep;

class InvBrokerService : public PyService {
public:
	InvBrokerService(PyServiceMgr *mgr, DBcore *db);
	virtual ~InvBrokerService();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	InventoryDB m_db;

	//overloaded in order to support bound objects:
	virtual PyBoundObject *_CreateBoundObject(Client *c, const PyRep *bind_args);
	
	//PyCallable_DECL_CALL(MachoBindObject)
};





#endif


