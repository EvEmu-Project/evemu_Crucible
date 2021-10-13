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
    Author:       James
*/

#ifndef __SOVEREIGNTYDB_H_INCL__
#define __SOVEREIGNTYDB_H_INCL__

#include "ServiceDB.h"

class PyRep;
class Client;

class SovereigntyDB
: public ServiceDB
{
public:
    static void GetSovereigntyData(DBQueryResult& res);
    static void GetSovereigntyDataForSystem(DBQueryResult& res, uint32 systemID);
    static void AddSovereigntyData(SovereigntyData data, uint32& claimID);
    static void RemoveSovereigntyData(uint32 systemID);
    static void SetContested(uint32 systemID, bool contested);
    static void SetHubID(uint32 systemID, uint32 hubID);
    static void GetUpgradeData(DBQueryResult& res);
    static void GetUpgradesForSystem(DBQueryResult& res, uint32 systemID);
    static void AddSystemUpgrade(uint32 systemID, uint32 upgradeID);
    static void RemoveSystemUpgrade(uint32 systemID, uint32 upgradeID);
};

#endif
