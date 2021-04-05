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
    Author:        Zhur
    Updates:    Allan
*/

#ifndef __FACTION_WAR_MGR__H__INCL__
#define __FACTION_WAR_MGR__H__INCL__

#include "faction/FactionWarMgrDB.h"
#include "PyService.h"

class FactionWarMgrService : public PyService
{
public:
    FactionWarMgrService(PyServiceMgr *mgr);
    ~FactionWarMgrService();

    PyCallable_DECL_CALL(GetWarFactions);
    PyCallable_DECL_CALL(GetFWSystems);
    PyCallable_DECL_CALL(RefreshCorps);
    PyCallable_DECL_CALL(GetMyCharacterRankOverview);
    PyCallable_DECL_CALL(GetMyCharacterRankInfo);
    PyCallable_DECL_CALL(GetFactionMilitiaCorporation);
    PyCallable_DECL_CALL(GetCorporationWarFactionID);
    PyCallable_DECL_CALL(GetCharacterRankInfo);
    PyCallable_DECL_CALL(GetFactionalWarStatus);
    PyCallable_DECL_CALL(GetSystemStatus);
    PyCallable_DECL_CALL(GetFactionCorporations);
    PyCallable_DECL_CALL(GetSystemsConqueredThisRun);
    PyCallable_DECL_CALL(IsEnemyFaction);
    PyCallable_DECL_CALL(IsEnemyCorporation);

    PyCallable_DECL_CALL(JoinFactionAsCharacter);
    PyCallable_DECL_CALL(JoinFactionAsCharacterRecommendationLetter);
    PyCallable_DECL_CALL(JoinFactionAsCorporation);
    PyCallable_DECL_CALL(JoinFactionAsAlliance);
    PyCallable_DECL_CALL(LeaveFactionAsAlliance);
    PyCallable_DECL_CALL(LeaveFactionAsCorporation);
    PyCallable_DECL_CALL(WithdrawJoinFactionAsAlliance);
    PyCallable_DECL_CALL(WithdrawJoinFactionAsCorporation);
    PyCallable_DECL_CALL(WithdrawLeaveFactionAsAlliance);
    PyCallable_DECL_CALL(WithdrawLeaveFactionAsCorporation);

    PyCallable_DECL_CALL(GetStats_FactionInfo);
    PyCallable_DECL_CALL(GetStats_Corp);
    PyCallable_DECL_CALL(GetStats_Character);
    PyCallable_DECL_CALL(GetStats_Alliance);
    PyCallable_DECL_CALL(GetStats_Militia);
    PyCallable_DECL_CALL(GetStats_CorpPilots);
    PyCallable_DECL_CALL(GetStats_TopAndAllKillsAndVPs);


private:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    FactionWarMgrDB m_db;
};

#endif /* __FACTION_WAR_MGR_SERVICE__H__INCL__ */


