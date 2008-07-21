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



#include "CorpMgrService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "../common/EVEUtils.h"

#include "../packets/CorporationPkts.h"
#include "../packets/General.h"

PyCallable_Make_InnerDispatcher(CorpMgrService)


CorpMgrService::CorpMgrService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "corpmgr"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(CorpMgrService, GetPublicInfo)
}

CorpMgrService::~CorpMgrService() {
	delete m_dispatch;
}


PyCallResult CorpMgrService::Handle_GetPublicInfo(PyCallArgs &call) {
	Call_SingleIntegerArg corpID;
	if (!corpID.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Bad param");
		return NULL;
	}

	return m_db.GetCorporation(corpID.arg);
}




















