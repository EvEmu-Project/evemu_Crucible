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
#include "corporation/CorporationDB.h"
#include <cmath>

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

double AccountDB::OfflineFundXfer(uint32 charID, double amount, uint8 type) {
    if (!std::isfinite(amount) || std::abs(amount) > 1000000000000.0 || (type != Account::CreditType::ISK && type != Account::CreditType::AURUM))
        throw CustomError("Invalid wallet change.");
    DBTransaction transaction;
    if (!transaction.Started()) throw CustomError("Wallet database unavailable.");
    const char* column = type == Account::CreditType::ISK ? "balance" : "aurBalance";
    DBQueryResult result; DBResultRow row;
    if (!sDatabase.RunQuery(result,"SELECT %s FROM chrCharacters WHERE characterID=%u FOR UPDATE",column,charID) || !result.GetRow(row))
        throw CustomError("Wallet owner unavailable.");
    const double oldBalance = row.GetDouble(0);
    const double balance = std::round((oldBalance + amount)*100.0)/100.0;
    if (!std::isfinite(balance) || balance < 0)
        throw UserError("NotEnoughMoney").AddISK("amount",-amount).AddISK("balance",oldBalance);
    if (balance > 1000000000000.0) throw CustomError("Wallet balance limit exceeded.");
    DBerror error; uint32 affected = 0;
    if (!sDatabase.RunQuery(error,affected,"UPDATE chrCharacters SET %s=%.2f WHERE characterID=%u",column,balance,charID) || affected != 1 || !transaction.Commit())
        throw CustomError("Wallet write failed; wallet was not changed.");
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

bool AccountDB::AddJournalEntry(uint32 ownerID, int8 entryTypeID, uint32 ownerFromID, uint32 ownerToID, int8 currency, \
                                uint16 accountKey, double amount, double newBalance, std::string description, uint32 referenceID/*0*/ )
{
    if (entryTypeID == Journal::EntryType::SkipLog)
        return true;
    // account key 0 is usually sent by the client, it should be the main cash account
    if (accountKey == 0)
        accountKey = Account::KeyType::Cash;

    std::string eDesc;
    sDatabase.DoEscapeString(eDesc, description);

    std::string tblName = "jnlCharacters";
    if (IsCorp(ownerID))
        tblName = "jnlCorporations";

    DBerror err;
    return sDatabase.RunQuery(err,
        "INSERT INTO %s (ownerID, entryTypeID, accountKey, transactionDate, ownerID1, ownerID2, referenceID, currency, amount, balance, description)"
        " VALUES (%u,%u,%u,%f,%u,%u,%u,%i,%.2f,%.2f,'%s')",
        tblName.c_str(), ownerID, entryTypeID, accountKey, GetFileTimeNow(), ownerFromID, ownerToID, referenceID, currency, amount, newBalance, eDesc.c_str());
}

std::vector<AccountDB::WalletChange> AccountDB::TransferAtomic(const std::vector<Transfer>& transfers, uint32 actingCharacterID) {
    DBTransaction transaction;
    if (!transaction.Started()) throw CustomError("Wallet database unavailable; payment was not completed.");
    std::vector<WalletChange> changes;
    for (const auto& transfer : transfers) {
        if (!std::isfinite(transfer.amount) || transfer.amount <= 0 || transfer.amount > 1000000000000.0)
            throw CustomError("Invalid payment amount.");
        const double amount = std::round(transfer.amount * 100.0) / 100.0;
        if (amount <= 0) throw CustomError("Payment must be at least 0.01.");
        auto currency = [](uint16 key) -> uint8 {
            if (key == 0 || key == Account::KeyType::Escrow || (key >= Account::KeyType::Cash && key <= Account::KeyType::Cash7)) return Account::CreditType::ISK;
            if (IsAUR(key)) return Account::CreditType::AURUM;
            throw CustomError("Unsupported wallet account.");
        };
        const uint8 fromCurrency = currency(transfer.fromKey), toCurrency = currency(transfer.toKey);
        if (fromCurrency != toCurrency) throw CustomError("Cannot transfer between different currencies.");
        for (unsigned side = 0; side < 2; ++side) {
            const uint32 owner = side ? transfer.toID : transfer.fromID;
            const uint16 requestedKey = side ? transfer.toKey : transfer.fromKey;
            const uint16 key = requestedKey ? requestedKey : Account::KeyType::Cash;
            const uint8 type = side ? toCurrency : fromCurrency;
            const double delta = side ? amount : -amount;
            double balance = 0;
            if (IsCharacterID(owner) || IsPlayerCorp(owner)) {
                const bool corp = IsPlayerCorp(owner);
                if ((!corp && key != Account::KeyType::Cash && !IsAUR(key)) || (corp && type != Account::CreditType::ISK))
                    throw CustomError("Invalid wallet account for owner.");
                const std::string table = corp ? "crpWalletDivisons" : "chrCharacters";
                const std::string id = corp ? "corporationID" : "characterID";
                const std::string column = corp ? "balance" + std::to_string(key - Account::KeyType::Cash + 1) : (type == Account::CreditType::ISK ? "balance" : "aurBalance");
                DBQueryResult result; DBResultRow row;
                if (!sDatabase.RunQuery(result,"SELECT %s FROM %s WHERE %s=%u FOR UPDATE",column.c_str(),table.c_str(),id.c_str(),owner) || !result.GetRow(row))
                    throw CustomError("Wallet owner unavailable; payment was not completed.");
                const double oldBalance = row.GetDouble(0);
                balance = std::round((oldBalance + delta) * 100.0) / 100.0;
                if (!std::isfinite(balance) || balance < 0) {
                    if (corp) throw UserError("NotEnoughMoneyCorp").AddOwnerName("owner",owner).AddISK("amount",-delta).AddISK("balance",oldBalance)
                        .AddFormatValue("division",new PyString(CorporationDB::GetDivisionName(owner,key)));
                    throw UserError("NotEnoughMoney").AddISK("amount",-delta).AddISK("balance",oldBalance);
                }
                if (balance > 1000000000000.0) throw CustomError("Wallet balance limit exceeded; payment was not completed.");
                DBerror error; uint32 affected = 0;
                if (!sDatabase.RunQuery(error,affected,"UPDATE %s SET %s=%.2f WHERE %s=%u",table.c_str(),column.c_str(),balance,id.c_str(),owner) || affected != 1)
                    throw CustomError("Wallet write failed; payment was not completed.");
                changes.push_back({owner,key,type,balance});
            } else if (owner != 1 && !IsNPCCorp(owner) && !IsAgent(owner)) {
                throw CustomError("Invalid wallet owner.");
            }
            uint32 from = transfer.fromID, to = transfer.toID;
            if (IsPlayerCorp(owner) && actingCharacterID) {
                if (side) to = actingCharacterID; else from = actingCharacterID;
            }
            if (!AddJournalEntry(owner,transfer.entryTypeID,from,to,type,key,delta,balance,transfer.description,transfer.referenceID))
                throw CustomError("Wallet journal write failed; payment was not completed.");
        }
    }
    if (!transaction.Commit()) throw CustomError("Wallet commit failed; payment was not completed.");
    return changes;
}
