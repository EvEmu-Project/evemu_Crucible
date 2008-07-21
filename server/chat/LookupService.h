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


#ifndef __LOOKUPSVC_SERVICE_H_INCL__
#define __LOOKUPSVC_SERVICE_H_INCL__

#include "../PyService.h"

#include "LSCDB.h"

class LookupService
: public PyService {
public:
	LookupService(PyServiceMgr *mgr, DBcore *db);
	virtual ~LookupService();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	LSCDB m_db;

	PyCallable_DECL_CALL(LookupCharacters)
	PyCallable_DECL_CALL(LookupCorporations)
	PyCallable_DECL_CALL(LookupFactions)
	PyCallable_DECL_CALL(LookupCorporationTickers)
	PyCallable_DECL_CALL(LookupStations)
	PyCallable_DECL_CALL(LookupLocationsByGroup)
	//PyCallable_DECL_CALL()

	//overloaded in order to support bound objects:
	//virtual PyBoundObject *_CreateBoundObject(Client *c, const PyRep *bind_args);
};





#endif


