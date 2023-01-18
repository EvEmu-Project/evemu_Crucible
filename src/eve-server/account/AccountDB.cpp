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

#include "eve-server.h"

#include "account/AccountDB.h"

/*
 * ACCOUNT__ERROR
 * ACCOUNT__WARNING
 * ACCOUNT__INFO
 * ACCOUNT__MESSAGE
 * ACCOUNT__TRACE
 * ACCOUNT__CALL
 * ACCOUNT__CALL_DUMP
 * ACCOUNT__RSP_DUMP
 * ACCOUNT__DB_ERROR
 * ACCOUNT__DB_WARNING
 * ACCOUNT__DB_INFO
 * ACCOUNT__DB_MESSAGE
 */

double AccountDB::OfflineFundXfer(uint32 charID, double amount, uint8 type)
{
    std::string from = "";
    switch (type) {
        case Account::CreditType::ISK:      from = "balance";    break;
        case Account::CreditType::AURUM:    from = "aurBalance"; break;
        case Account::CreditType::MPLEX:    return 0; //make error..this isnt used yet
    }

    double balance = 0;
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT %s FROM chrCharacters WHERE characterID = %u", from.c_str(), charID);
    DBResultRow row;
    if (res.GetRow(row))
        balance = row.GetDouble(0);

    balance += amount;
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE chrCharacters SET %s = %f WHERE characterID = %u", from.c_str(), balance, charID);
    return balance;
}


double AccountDB::GetCorpBalance(uint32 corpID, uint16 accountKey)
{
    std::string acctKey = "";
    switch (accountKey) {
        case Account::KeyType::Cash:    acctKey = "balance1"; break;
        case Account::KeyType::Cash2:   acctKey = "balance2"; break;
        case Account::KeyType::Cash3:   acctKey = "balance3"; break;
        case Account::KeyType::Cash4:   acctKey = "balance4"; break;
        case Account::KeyType::Cash5:   acctKey = "balance5"; break;
        case Account::KeyType::Cash6:   acctKey = "balance6"; break;
        case Account::KeyType::Cash7:   acctKey = "balance7"; break;
    }
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT %s FROM crpWalletDivisons WHERE corporationID = %u ", acctKey.c_str(), corpID)) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }
    DBResultRow row;
    if (!res.GetRow(row))
        return 0;
    return row.GetDouble(0);
}

void AccountDB::UpdateCorpBalance(uint32 corpID, uint16 accountKey, double amount)
{
    std::string acctKey = "";
    switch (accountKey) {
        case Account::KeyType::Cash:    acctKey = "balance1"; break;
        case Account::KeyType::Cash2:   acctKey = "balance2"; break;
        case Account::KeyType::Cash3:   acctKey = "balance3"; break;
        case Account::KeyType::Cash4:   acctKey = "balance4"; break;
        case Account::KeyType::Cash5:   acctKey = "balance5"; break;
        case Account::KeyType::Cash6:   acctKey = "balance6"; break;
        case Account::KeyType::Cash7:   acctKey = "balance7"; break;
    }
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE crpWalletDivisons SET %s = %.2f WHERE corporationID = %u ", acctKey.c_str(), amount, corpID);
}


PyRep *AccountDB::GetWalletDivisionsInfo(uint32 corpID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT balance1, balance2, balance3, balance4, balance5, balance6,  balance7"
        " FROM crpWalletDivisons"
        " WHERE corporationID = %u", corpID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    DBResultRow row;
    PyList *list = new PyList();
    if (res.GetRow(row))
        for (int8 i = 0; i < 7; ++i) {
            PyDict *dict = new PyDict();
            dict->SetItemString("key", new PyInt(1000 + i));
            dict->SetItemString("balance", new PyFloat(row.GetDouble(i)));
            list->AddItem(new PyObject("util.KeyVal", dict));
        }

    if (is_log_enabled(ACCOUNT__RSP_DUMP))
        list->Dump(ACCOUNT__RSP_DUMP, "    ");
    return list;
}

PyRep* AccountDB::GetJournal(uint32 ownerID, int8 entryTypeID, uint16 accountKey, int64 fromDate, bool reverse/*false*/)
{
    std::string tblName = "jnlCharacters";
    if (IsCorp(ownerID))
        tblName = "jnlCorporations";

    std::string entryType = "";
    if (entryTypeID) {
        entryType = " AND entryTypeID = ";
        entryType += std::to_string(entryTypeID);
    }

    std::string sort = "";
    if (reverse)
        sort = "";

    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT transactionID, transactionDate, referenceID, entryTypeID, ownerID1, ownerID2, accountKey, amount,"
        " balance, description, currency, 1 AS sortValue"
        " FROM %s"
        " WHERE transactionDate > %lli AND accountKey = %u %s AND ownerID = %u %s",
        tblName.c_str(), fromDate, accountKey, entryType.c_str(), ownerID, sort.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

void AccountDB::AddJournalEntry(uint32 ownerID, int8 entryTypeID, uint32 ownerFromID, uint32 ownerToID, int8 currency, \
                                uint16 accountKey, double amount, double newBalance, std::string description, uint32 referenceID/*0*/ )
{
    if (entryTypeID == Journal::EntryType::SkipLog)
        return;
    // account key 0 is usually sent by the client, it should be the main cash account
    if (accountKey == 0)
        accountKey = Account::KeyType::Cash;

    std::string eDesc;
    sDatabase.DoEscapeString(eDesc, description);

    std::string tblName = "jnlCharacters";
    if (IsCorp(ownerID))
        tblName = "jnlCorporations";

    // dont care if this fails...
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO %s (ownerID, entryTypeID, accountKey, transactionDate, ownerID1, ownerID2, referenceID, currency, amount, balance, description)"
        " VALUES (%u,%u,%u,%f,%u,%u,%u,%i,%.2f,%.2f,'%s')",
        tblName.c_str(), ownerID, entryTypeID, accountKey, GetFileTimeNow(), ownerFromID, ownerToID, referenceID, currency, amount, newBalance, eDesc.c_str());
}
