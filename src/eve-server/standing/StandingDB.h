/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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
    Updates:        Allan
*/


#ifndef __STANDINGDB_H_INCL__
#define __STANDINGDB_H_INCL__

#include "ServiceDB.h"
#include "packets/Standing.h"

class PyRep;
class Client;

class StandingDB
: public ServiceDB
{
public:
    static PyObjectEx* GetFactionStandings();
    PyRep* GetCorpStandings(Client* pClient);
    PyRep* GetCharStandings(Client* pClient);
    static PyRep* GetSystemSovInfo(uint32 systemID);
    PyRep* PrimeCharStandings(uint32 charID);
    PyRep* GetCharNPCStandings(uint32 charID);
    PyRep* GetStandingTransactions(Call_GetStandingTransactions &args);
    PyRep* GetStandingCompositions(uint32 fromID, uint32 toID);


    /*  all standings are in same table now, but follow identical rules
     * NPC Faction<-->NPC Faction - populated, hard-coded  -- cant change
     * agent-->char, agent-->PC corp  -- changed by missions status'
     * corporation<-->alliance, alliance<-->alliance  -- changed thru Corp window
     * corporation-->character, corporation<-->corporation  -- changed thru Corp window
     * character<-->character, character-->corporation  -- changed thru PnP window
     * NPC corp-->char, NPC corp-->PC corp  -- changed by missions and faction kills
     */
    static float GetStanding(uint32 fromID, uint32 toID);

    static void SetStanding(uint32 fromID, uint32 toID, float standing);
    static void UpdateStanding(uint32 fromID, uint32 toID, float standing);
    static void SaveStandingChanges(uint32 fromID, uint32 toID, uint16 eventType, float amount, std::string msg);

    static PyRep* GetMyStandings(uint32 charID);
};

#endif
