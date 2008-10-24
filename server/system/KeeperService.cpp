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


PyCallable_Make_InnerDispatcher(KeeperService)




class KeeperBound
: public PyBoundObject {
public:
	
	PyCallable_Make_Dispatcher(KeeperBound)
	
	KeeperBound(PyServiceMgr *mgr, SystemDB *db)
	: PyBoundObject(mgr, "KeeperBound"),
	  m_db(db),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);
		
//		PyCallable_REG_CALL(KeeperBound, EditDungeon)
//		PyCallable_REG_CALL(KeeperBound, PlayDungeon)
//		PyCallable_REG_CALL(KeeperBound, Reset)
//		PyCallable_REG_CALL(KeeperBound, GotoRoom) //(int room)
	}
	virtual ~KeeperBound() { delete m_dispatch; }
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	//PyCallable_DECL_CALL()
	//PyCallable_DECL_CALL()

protected:
	SystemDB *const m_db;
	Dispatcher *const m_dispatch;   //we own this
};


KeeperService::KeeperService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "keeper"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(KeeperService, GetLevelEditor)
}

KeeperService::~KeeperService() {
	delete m_dispatch;
}

PyBoundObject *KeeperService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	_log(CLIENT__MESSAGE, "KeeperService bind request for:");
	bind_args->Dump(CLIENT__MESSAGE, "    ");

	return(new KeeperBound(m_manager, &m_db));
}


PyResult KeeperService::Handle_GetLevelEditor(PyCallArgs &call) {
	PyRep *result = NULL;
	
	KeeperBound *ib = new KeeperBound(m_manager, &m_db);
	result = m_manager->BindObject(call.client, ib);

	return(result);
}



























