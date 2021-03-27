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
*/


#ifndef __ACCOUNTSERVICE_H_INCL__
#define __ACCOUNTSERVICE_H_INCL__

#include "PyService.h"
#include "account/AccountDB.h"

class Client;

class AccountService
: public PyService {
public:
    AccountService(PyServiceMgr *mgr);
    ~AccountService();

    // this moves currency, adds journal entries, and sends blink event. handles applicable corp taxes internally.
    //  will throw if fails
    static void TranserFunds(uint32 fromID, uint32 toID, double amount, std::string reason = "", \
                            uint8 entryTypeID = Journal::EntryType::Undefined, uint32 referenceID = 0, \
                            uint16 fromKey = Account::KeyType::Cash, uint16 toKey = Account::KeyType::Cash, \
                            Client* pClient=nullptr);

    // this should be the ONLY method to alter corp balances, and ONLY called from TransferFunds()
    static void HandleCorpTransaction(uint32 corpID, int8 entryTypeID, uint32 fromID, uint32 toID, int8 currency, uint16 accountKey, \
                                      double amount, std::string description, uint32 referenceID = 0);

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    AccountDB m_db;

    PyCallable_DECL_CALL(GetCashBalance);
    PyCallable_DECL_CALL(GetEntryTypes);
    PyCallable_DECL_CALL(GetKeyMap);
    PyCallable_DECL_CALL(GiveCash);
    PyCallable_DECL_CALL(GiveCashFromCorpAccount);
    PyCallable_DECL_CALL(GetJournal);
    PyCallable_DECL_CALL(GetJournalForAccounts);
    PyCallable_DECL_CALL(GetWalletDivisionsInfo);
    PyCallable_DECL_CALL(GetDefaultContactCost);
    PyCallable_DECL_CALL(SetContactCost);
};

#endif
