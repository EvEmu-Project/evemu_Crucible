/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        Reve
*/

//work in progress

#include "eve-server.h"

#include "PyServiceCD.h"
#include "dungeon/DungeonExplorationMgrService.h"

PyCallable_Make_InnerDispatcher(DungeonExplorationMgrService)

DungeonExplorationMgrService::DungeonExplorationMgrService(PyServiceMgr *mgr)
: PyService(mgr, "dungeonExplorationMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(DungeonExplorationMgrService, GetMyEscalatingPathDetails);
}

DungeonExplorationMgrService::~DungeonExplorationMgrService()
{
    delete m_dispatch;
}

PyResult DungeonExplorationMgrService::Handle_GetMyEscalatingPathDetails(PyCallArgs &call) {
    // cached response
    /**00:51:32 L DungeonExplorationMgrService::Handle_GetMyEscalatingPathDetails(): size= 0
     * 00:51:32 [SvcCall]   Call Arguments:
     * 00:51:32 [SvcCall]       Tuple: Empty
     * 00:51:32 [SvcCall]   Call Named Arguments:
     * 00:51:32 [SvcCall]     Argument 'machoVersion':
     * 00:51:32 [SvcCall]         Integer field: 1
     *
     * /client/script/ui/shared/neocom/journal.py(922) Load
     * /client/script/ui/shared/neocom/journal.py(1502) ShowExpeditions
     *        expeditionRowset = None
     *        self = form.Journal object at 0x2ebcc490, name=journal, destroyed=False>
     *        scrolllist = []
     *        reload = 0
     * TypeError: 'NoneType' object is not iterable
     *
     *
     *    def ShowExpeditions(self, reload = 0):
     *        expeditionRowset = sm.GetService('journal').GetExpeditions(force=reload)
     *        self.ShowLoad()
     *        try:
     *            scrolllist = []
     *            for expeditionData in expeditionRowset:
     *                dungeonData = expeditionData.dungeon
     *                destDungeonData = expeditionData.destDungeon
     *                pathStep = expeditionData.pathStep
     *                expiryTime = expeditionData.expiryTime
     *                if destDungeonData is not None:
     *                    jumps = sm.GetService('starmap').ShortestPath(dungeonData.solarSystemID, eve.session.solarsystemid2)
     *                    jumps = jumps or []
     *                    totaljumps = max(0, len(jumps) - 1)
     *                    if dungeonData.solarSystemID is not None:
     *                        solName = cfg.evelocations.Get(dungeonData.solarSystemID).name
     *                    else:
     *                        solName = localization.GetByLabel('UI/Common/Unknown')
     *                    txt = localization.GetByMessageID(pathStep.journalEntryTemplateID)
     *                    if len(txt) > MAX_COL_LENGTH:
     *                        txt = txt[:MAX_COL_LENGTH] + '...'
     *                    text = '<t>'.join(['      ',
     *                     util.FmtDate(dungeonData.creationTime),
     *                     solName,
     *                     str(totaljumps),
     *                     txt])
     *                    expired = not expiryTime - blue.os.GetWallclockTime() > 0
     *                    data = {'id': dungeonData.instanceID,
     *                     'rec': expeditionData,
     *                     'text': txt,
     *                     'label': text,
     *                     'hint': localization.GetByMessageID(pathStep.journalEntryTemplateID),
     *                     'jumps': totaljumps,
     *                     'expired': expired,
     *                     'sort_' + localization.GetByLabel('UI/Journal/JournalWindow/Expeditions/HeaderExpiration'): expiryTime}
     *                    scrolllist.append(listentry.Get('EscalatingPathLocationEntry', data))
     *
     */
    sLog.White("DungeonExplorationMgrService::Handle_GetMyEscalatingPathDetails()",  "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return PyStatic.NewNone();
}


/**
  dungeonTracking.GetEscalatingPathDungeonsEntered()
  dungeonTracking.GetDistributionDungeonsEntered()
        (OnDistributionDungeonEntered)
sm.RemoteSvc('dungeonExplorationMgr').DeleteExpiredPathStep


*/
