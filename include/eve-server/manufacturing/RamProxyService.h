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
    Author:     Zhur
*/

#ifndef __RAM_PROXY_SERVICE__H__
#define __RAM_PROXY_SERVICE__H__

#include "manufacturing/RamProxyDB.h"
#include "PyService.h"

static const uint32 ramProductionTimeLimit = 60*60*24*30;   //30 days

class RamProxyService : public PyService {
public:
    RamProxyService(PyServiceMgr *mgr);
    virtual ~RamProxyService();

private:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    RamProxyDB m_db;

    // verifying functions
    void _VerifyInstallJob_Call(const Call_InstallJob &args, InventoryItemRef installedItem, const PathElement &bomLocation, Client *const c);
    void _VerifyInstallJob_Install(const Rsp_InstallJob &rsp, const PathElement &bomLocation, const std::vector<RequiredItem> &reqItems, const uint32 runs, Client *const c);
    void _VerifyCompleteJob(const Call_CompleteJob &args, Client *const c);

    bool _Calculate(const Call_InstallJob &args, InventoryItemRef installedItem, Client *const c, Rsp_InstallJob &into);
    void _EncodeBillOfMaterials(const std::vector<RequiredItem> &reqItems, double materialMultiplier, double charMaterialMultiplier, uint32 runs, BillOfMaterials &into);
    void _EncodeMissingMaterials(const std::vector<RequiredItem> &reqItems, const PathElement &bomLocation, Client *const c, double materialMultiplier, double charMaterialMultiplier, int32 runs, std::map<int32, PyRep *> &into);

    void _GetBOMItems(const PathElement &bomLocation, std::vector<InventoryItemRef> &into);

    PyCallable_DECL_CALL(GetJobs2)
    PyCallable_DECL_CALL(AssemblyLinesSelect)
    PyCallable_DECL_CALL(AssemblyLinesGet)
    PyCallable_DECL_CALL(InstallJob)
    PyCallable_DECL_CALL(CompleteJob)
	PyCallable_DECL_CALL(GetRelevantCharSkills)
	PyCallable_DECL_CALL(AssemblyLinesSelectPublic)
};

#endif
