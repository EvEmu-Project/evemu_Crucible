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
    Updates:    Allan
*/
#ifndef EVE_ACCOUNT_DB_H
#define EVE_ACCOUNT_DB_H

#include "ServiceDB.h"


class AccountDB
{
public:
    PyRep* GetWalletDivisionsInfo(uint32 corpID);

    PyRep* GetJournal(uint32 ownerID, int8 entryTypeID, uint16 accountKey, int64 fromDate, bool reverse = false);

    static double OfflineFundXfer(uint32 charID, double amount, uint8 type=Account::CreditType::ISK);
    static double GetCorpBalance(uint32 corpID, uint16 accountKey);
    static void UpdateCorpBalance(uint32 corpID, uint16 accountKey, double amount);

    static void AddJournalEntry(uint32 ownerID, int8 entryTypeID, uint32 ownerFromID, uint32 ownerToID, int8 currency, uint16 accountKey, \
                                double amount, double newBalance, std::string description, uint32 referenceID = 0);

};


#endif  // EVE_ACCOUNT_DB_H
