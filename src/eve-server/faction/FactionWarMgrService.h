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
#include "services/Service.h"

class FactionWarMgrService : public Service <FactionWarMgrService>
{
public:
    FactionWarMgrService(EVEServiceManager& mgr);

protected:
    PyResult GetWarFactions(PyCallArgs& call);
    PyResult GetFWSystems(PyCallArgs& call); 
    PyResult GetMyCharacterRankOverview(PyCallArgs& call);
    PyResult GetMyCharacterRankInfo(PyCallArgs& call);
    PyResult GetFactionMilitiaCorporation(PyCallArgs& call, PyInt* factionID);
    PyResult GetSystemStatus(PyCallArgs& call, PyInt* solarsystemID, PyInt* warFactionID);
    PyResult IsEnemyFaction(PyCallArgs& call, PyInt* enemyID, PyInt* factionID);
    PyResult IsEnemyCorporation(PyCallArgs& call, PyInt* enemyID, PyInt* factionID);
    PyResult GetCharacterRankInfo(PyCallArgs& call, PyInt* characterID);
    PyResult GetFactionalWarStatus(PyCallArgs& call);
    PyResult JoinFactionAsCharacter(PyCallArgs& call, PyInt* factionID);
    PyResult GetCorporationWarFactionID(PyCallArgs& call, PyInt* corporationID);
    PyResult GetSystemsConqueredThisRun(PyCallArgs& call);
    PyResult GetFactionCorporations(PyCallArgs& call, PyInt* factionID);
    PyResult JoinFactionAsCharacterRecommendationLetter(PyCallArgs& call, PyInt* factionID, PyInt* itemID);
    PyResult JoinFactionAsAlliance(PyCallArgs& call, PyInt* factionID);
    PyResult JoinFactionAsCorporation(PyCallArgs& call, PyInt* factionID);
    PyResult GetStats_FactionInfo(PyCallArgs& call);
    PyResult GetStats_TopAndAllKillsAndVPs(PyCallArgs& call);
    PyResult GetStats_Character(PyCallArgs& call);
    PyResult GetStats_Corp(PyCallArgs& call);
    PyResult GetStats_Alliance(PyCallArgs& call);
    PyResult GetStats_Militia(PyCallArgs& call);
    PyResult GetStats_CorpPilots(PyCallArgs& call);
    PyResult LeaveFactionAsAlliance(PyCallArgs& call, PyInt* factionID);
    PyResult LeaveFactionAsCorporation(PyCallArgs& call, PyInt* factionID);
    PyResult WithdrawJoinFactionAsAlliance(PyCallArgs& call, PyInt* factionID);
    PyResult WithdrawJoinFactionAsCorporation(PyCallArgs& call, PyInt* factionID);
    PyResult WithdrawLeaveFactionAsAlliance(PyCallArgs& call, PyInt* factionID);
    PyResult WithdrawLeaveFactionAsCorporation(PyCallArgs& call, PyInt* factionID);
    PyResult RefreshCorps(PyCallArgs& call);

private:
    ObjCacheService* m_cache;
    EVEServiceManager& m_manager;
    FactionWarMgrDB m_db;
};

#endif /* __FACTION_WAR_MGR_SERVICE__H__INCL__ */


