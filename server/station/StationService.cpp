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



#include "StationService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"

PyCallable_Make_InnerDispatcher(StationService)

StationService::StationService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "station"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(StationService, GetStationItemBits)
	PyCallable_REG_CALL(StationService, GetGuests)
}

StationService::~StationService() {
	delete m_dispatch;
}


PyCallResult StationService::Handle_GetStationItemBits(PyCallArgs &call) {
	return m_db.GetStationItemBits(call.client->GetStationID());
/*
	PyRep *result = NULL;

	PyRepTuple *tt;
	tt = new PyRepTuple(5);
	result = tt;

	tt->items[0] = new PyRepInteger(241);		//hangarGraphicID
	tt->items[1] = new PyRepInteger(1000044);	//station owner corp
	tt->items[2] = new PyRepInteger(60004420);	//station ID
	tt->items[3] = new PyRepInteger(7680597);	//serviceMask
	tt->items[4] = new PyRepInteger(1529);		//stationTypeID

	return(result);*/
}


PyCallResult StationService::Handle_GetGuests(PyCallArgs &call) {
	PyRep *result = NULL;

	PyRepList *l = new PyRepList();
	result = l;

	l->add(new PyRepInteger(call.client->GetCharacterID()));

	return(result);
}



























