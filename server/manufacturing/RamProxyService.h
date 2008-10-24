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

#ifndef __RAM_PROXY_SERVICE__H__
#define __RAM_PROXY_SERVICE__H__

#include "RamProxyDB.h"
#include "../PyService.h"
#include "../packets/Manufacturing.h"

static const uint32 ramProductionTimeLimit = 60*60*24*30;	//30 days

class RamProxyService : public PyService {
public:
	RamProxyService(PyServiceMgr *mgr, DBcore *db);
	virtual ~RamProxyService();

private:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	RamProxyDB m_db;

	// verifying functions
	void _VerifyInstallJob_Call(const Call_InstallJob &args, const InventoryItem *const installedItem, const PathElement &bomLocation, Client *const c);
	void _VerifyInstallJob_Install(const Rsp_InstallJob &rsp, const PathElement &bomLocation, const std::vector<RequiredItem> &reqItems, const uint32 runs, Client *const c);
	void _VerifyCompleteJob(const Call_CompleteJob &args, Client *const c);

	bool _Calculate(const Call_InstallJob &args, const InventoryItem *const installedItem, Client *const c, Rsp_InstallJob &into);
	void _EncodeBillOfMaterials(const std::vector<RequiredItem> &reqItems, double materialMultiplier, double charMaterialMultiplier, uint32 runs, BillOfMaterials &into);
	void _EncodeMissingMaterials(const std::vector<RequiredItem> &reqItems, const PathElement &bomLocation, Client *const c, double materialMultiplier, double charMaterialMultiplier, uint32 runs, std::map<uint32, PyRep *> &into);

	PyCallable_DECL_CALL(GetJobs2)
	PyCallable_DECL_CALL(AssemblyLinesSelect)
	PyCallable_DECL_CALL(AssemblyLinesGet)
	PyCallable_DECL_CALL(InstallJob)
	PyCallable_DECL_CALL(CompleteJob)
};

#endif
