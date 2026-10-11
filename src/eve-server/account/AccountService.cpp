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
    Rewrite:    Allan
*/

#include <boost/algorithm/string.hpp>
#include "eve-server.h"
#include <cmath>

#include "EntityList.h"

#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "cache/ObjCacheService.h"
#include "corporation/CorporationDB.h"

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

AccountService::AccountService() :
    Service("account", eAccessLevel_Character)
{
    this->Add("GetCashBalance", static_cast <PyResult (AccountService::*)(PyCallArgs &, std::optional<PyBool*>, std::optional<PyInt*>)> (&AccountService::GetCashBalance));
    this->Add("GetCashBalance", static_cast <PyResult(AccountService::*)(PyCallArgs &, std::optional<PyInt*>, std::optional<PyInt*>)> (&AccountService::GetCashBalance));
    this->Add("GetEntryTypes", &AccountService::GetEntryTypes);
    this->Add("GetKeyMap", &AccountService::GetKeyMap);
    this->Add("GiveCash", static_cast <PyResult (AccountService::*)(PyCallArgs &, PyInt *, PyInt *, std::optional <PyWString*>)> (&AccountService::GiveCash));
    this->Add("GiveCash", static_cast <PyResult(AccountService::*)(PyCallArgs &, PyInt *, PyFloat *, std::optional <PyWString*>)> (&AccountService::GiveCash));
    this->Add("GiveCash", static_cast <PyResult (AccountService::*)(PyCallArgs &, PyInt *, PyInt *, std::optional <PyString*>)> (&AccountService::GiveCash));
    this->Add("GiveCash", static_cast <PyResult(AccountService::*)(PyCallArgs &, PyInt *, PyFloat *, std::optional <PyString*>)> (&AccountService::GiveCash));
    this->Add("GiveCashFromCorpAccount", static_cast <PyResult (AccountService::*)(PyCallArgs &, PyInt *, PyInt *, PyInt *)> (&AccountService::GiveCashFromCorpAccount));
    this->Add("GiveCashFromCorpAccount", static_cast <PyResult (AccountService::*)(PyCallArgs &, PyInt *, PyFloat *, PyInt *)> (&AccountService::GiveCashFromCorpAccount));
    this->Add("GetJournal", static_cast <PyResult (AccountService::*)(PyCallArgs &, PyInt *, PyLong *, std::optional<PyInt*>, PyInt *, std::optional <PyInt*>, std::optional<PyInt*>)> (&AccountService::GetJournal));
    this->Add("GetJournal", static_cast <PyResult(AccountService::*)(PyCallArgs&, PyInt*, PyLong*, std::optional<PyInt*>, PyBool*, std::optional <PyInt*>, std::optional<PyInt*>)> (&AccountService::GetJournal));
    this->Add("GetJournalForAccounts", static_cast <PyResult (AccountService::*) (PyCallArgs &, PyInt *, PyLong *, std::optional<PyInt*>, PyBool *, std::optional <PyInt*>, std::optional<PyInt*>)>(&AccountService::GetJournalForAccounts));
    this->Add("GetJournalForAccounts", static_cast <PyResult(AccountService::*) (PyCallArgs&, PyInt*, PyLong*, std::optional<PyInt*>, PyInt*, std::optional <PyInt*>, std::optional<PyInt*>)>(&AccountService::GetJournalForAccounts));
    this->Add("GetWalletDivisionsInfo", &AccountService::GetWalletDivisionsInfo);
    this->Add("GetDefaultContactCost", &AccountService::GetDefaultContactCost);
    this->Add("SetContactCost", &AccountService::SetContactCost);
}

PyResult AccountService::GetKeyMap(PyCallArgs &call)
{
    return sDataMgr.GetKeyMap();    // account key types
}

PyResult AccountService::GetEntryTypes(PyCallArgs &call)
{
    return sDataMgr.GetEntryTypes();    // journal entry IDs
}

PyResult AccountService::GetWalletDivisionsInfo(PyCallArgs &call)
{
    return m_db.GetWalletDivisionsInfo(call.client->GetCorporationID());
}

// from mail/label window->settings
PyResult AccountService::GetDefaultContactCost(PyCallArgs &call)
{
    /*
            self.defaultContactCost = self.GetAccountSvc().GetDefaultContactCost()
            if self.defaultContactCost is None:
                self.defaultContactCost = -1
        */

    sLog.Log( "AccountService::Handle_GetDefaultContactCost()", "size=%lu", call.tuple->size());
    call.Dump(ACCOUNT__CALL_DUMP);

    //return m_db.GetDefaultContactCost(call.client->GetCorporationID());

    // returning "none" will block all contact attempts
    return PyStatic.NewNone();
}

PyResult AccountService::SetContactCost(PyCallArgs &call, std::optional<PyInt*> cost)
{
    /*
        self.GetAccountSvc().SetContactCost(cost)

    def BlockAll(self):
        self.GetAccountSvc().SetContactCost(None)
        */

    sLog.Log( "AccountService::Handle_SetContactCost()", "size=%lu", call.tuple->size());
    call.Dump(ACCOUNT__CALL_DUMP);
    // m_db.SetContactCost(call.client->GetCorporationID());

    // returns nothing
    return nullptr;
}

PyResult AccountService::GetCashBalance(PyCallArgs& call, std::optional<PyBool*> isCorpWallet, std::optional<PyInt*> walletKey) {
    return GetCashBalance(call, new PyInt(isCorpWallet.has_value() ? isCorpWallet.value()->value() : 0), walletKey);
}

PyResult AccountService::GetCashBalance(PyCallArgs &call, std::optional<PyInt*> isCorpWallet, std::optional<PyInt*> walletKey) {
    //corrected, updated, optimized     -allan 26jan15      ReVisited/Rewrote  -allan 7Dec17    Update  -allan 20May19
    if (is_log_enabled(ACCOUNT__CALL_DUMP)) {
        sLog.Log( "AccountService::Handle_GetCashBalance()", "size=%lu", call.tuple->size());
        call.Dump(ACCOUNT__CALL_DUMP);
    }

    double balance(0);
    int16 accountKey(call.client->GetCorpAccountKey());
    if (call.byname.find("accountKey") != call.byname.end())
        accountKey = PyRep::IntegerValueU32(call.byname.find("accountKey")->second);
    else if (walletKey.has_value())
        accountKey = walletKey.value()->value();

    if (isCorpWallet.has_value() && isCorpWallet.value()->value()) {
        balance = AccountDB::GetCorpBalance( call.client->GetCorporationID(), accountKey);
    } else {
        int8 type = Account::CreditType::ISK;
        if (accountKey == Account::KeyType::AUR) {
            type = Account::CreditType::AURUM;
        } else if (accountKey == Account::KeyType::DUST_ISK) {
            type = Account::CreditType::MPLEX;
        }
        balance = call.client->GetBalance(type);
    }

    return new PyFloat(balance);
}

PyResult AccountService::GetJournal(PyCallArgs& call, PyInt* accountKey, PyLong* fromDate, std::optional<PyInt*> entryTypeID, PyBool* corpAccount, std::optional <PyInt*> transactionID, std::optional<PyInt*> rev) {
    return GetJournal(call, accountKey, fromDate, entryTypeID, new PyInt(corpAccount->value()), transactionID, rev);
}

PyResult AccountService::GetJournal(PyCallArgs &call, PyInt* accountKey, PyLong* fromDate, std::optional<PyInt*> entryTypeID, PyInt* corpAccount, std::optional <PyInt*> transactionID, std::optional<PyInt*> rev)
{    // this asks for data for a single acctKey
    if (is_log_enabled(ACCOUNT__CALL_DUMP)) {
        sLog.Log( "AccountService::Handle_GetJournal()", "size=%lu", call.tuple->size());
        call.Dump(ACCOUNT__CALL_DUMP);
    }

    uint32 ownerID(call.client->GetCharacterID());
    if (corpAccount->value())
        ownerID = call.client->GetCorporationID();

    PyRep* res = m_db.GetJournal(ownerID, entryTypeID.has_value() ? entryTypeID.value()->value() : 0, accountKey->value(), fromDate->value(), rev.has_value() ? rev.value()->value() : 0);
    if (is_log_enabled(ACCOUNT__RSP_DUMP))
        res->Dump(ACCOUNT__RSP_DUMP, "    ");
    return res;
}

PyResult AccountService::GetJournalForAccounts(PyCallArgs& call, PyInt* accountKeys, PyLong* fromDate, std::optional<PyInt*> entryTypeID, PyBool* corpAccount, std::optional <PyInt*> transactionID, std::optional<PyInt*> rev) {
    return GetJournalForAccounts(call, accountKeys, fromDate, entryTypeID, new PyInt(corpAccount->value()), transactionID, rev);
}

/** @todo this isnt right.... */
PyResult AccountService::GetJournalForAccounts(PyCallArgs &call, PyInt* accountKeys, PyLong* fromDate, std::optional<PyInt*> entryTypeID, PyInt* corpAccount, std::optional <PyInt*> transactionID, std::optional<PyInt*> rev) {
    // this asks for data for multiple acctKeys
    // self.journalData[key] = self.GetAccountSvc().GetJournalForAccounts(accountKeys, fromDate, entryTypeID, corpAccount, transactionID, rev)
    if (is_log_enabled(ACCOUNT__CALL_DUMP)) {
        sLog.Log( "AccountService::Handle_GetJournalForAccounts()", "size=%lu", call.tuple->size());
        call.Dump(ACCOUNT__CALL_DUMP);
    }
    uint32 ownerID = call.client->GetCharacterID();
    if (corpAccount->value())
        ownerID = call.client->GetCorporationID();

    uint16 acctKey = Account::KeyType::Cash;

    PyRep* res = m_db.GetJournal(ownerID, entryTypeID.has_value() ? entryTypeID.value()->value() : 0, acctKey, fromDate->value(), rev.has_value() ? rev.value()->value() : 0);
    if (is_log_enabled(ACCOUNT__RSP_DUMP))
        res->Dump(ACCOUNT__RSP_DUMP, "    ");
    return res;
}

PyResult AccountService::GiveCash(PyCallArgs& call, PyInt* toID, PyInt* amount, std::optional <PyWString*> reason) {
    return GiveCash(call, toID, new PyFloat(amount->value()), reason.has_value()?reason.value()->content():std::string());
}

PyResult AccountService::GiveCash(PyCallArgs& call, PyInt* toID, PyFloat* amount, std::optional <PyWString*> reason) {
    return GiveCash(call, toID, amount, reason.has_value()?reason.value()->content():std::string());
}

PyResult AccountService::GiveCash(PyCallArgs& call, PyInt* toID, PyInt* amount, std::optional <PyString*> reason) {
    return GiveCash(call, toID, new PyFloat(amount->value()), reason.has_value()?reason.value()->content():std::string());
}

PyResult AccountService::GiveCash(PyCallArgs& call, PyInt* toID, PyFloat* amount, std::optional <PyString*> reason) {
    return GiveCash(call, toID, amount, reason.has_value()?reason.value()->content():std::string());
}

PyResult AccountService::GiveCash(PyCallArgs &call, PyInt* toID, PyFloat* amount, std::string reason) {
    if (is_log_enabled(ACCOUNT__CALL_DUMP)) {
        sLog.Log( "AccountService::Handle_GiveCash()", "size=%lu", call.tuple->size());
        call.Dump(ACCOUNT__CALL_DUMP);
    }

    std::string reasonStr = "DESC: ";
    if (reason.size() < 1) {
        reasonStr += "No Reason Given";
    } else {
        // this hits db directly, so test for possible sql injection code
        for (const auto cur : badChars)
            if (EvE::icontains(reason, cur))
                throw CustomError ("Description contains invalid characters");
        reasonStr += reason;
    }

    TransferFunds(
        call.client->GetCharacterID(),
        toID->value(),
        amount->value(),
        reasonStr.c_str(),
        Journal::EntryType::PlayerDonation,
        call.client->GetCharacterID()
    );

    return nullptr;
}

PyResult AccountService::GiveCashFromCorpAccount(PyCallArgs& call, PyInt* toID, PyInt* amount, PyInt* fromAcctKey) {
    return GiveCashFromCorpAccount(call, toID, new PyFloat(amount->value()), fromAcctKey);
}

PyResult AccountService::GiveCashFromCorpAccount(PyCallArgs &call, PyInt* toID, PyFloat* amount, PyInt* fromAcctKey)
{
    if (is_log_enabled(ACCOUNT__CALL_DUMP)) {
        sLog.Log( "AccountService::Handle_GiveCashFromCorpAccount()", "size=%lu", call.tuple->size());
        call.Dump(ACCOUNT__CALL_DUMP);
    }

    uint16 toAcctKey = Account::KeyType::Cash;
    if (call.byname.find("toAccountKey") != call.byname.end())
        toAcctKey = PyRep::IntegerValue(call.byname.find("toAccountKey")->second);

    std::string reason= "DESC: ";
    if (call.byname.find("reason") != call.byname.end()) {
        // make sure that the reason has anything in it so YAML parsing is correct
        std::string content = PyRep::StringContent (call.byname.find ("reason")->second);

        if (content.size () < 1) {
            reason += "No Reason Given by ";
            reason += call.client->GetCharName();
        } else {
            // this hits db directly, so test for possible sql injection code
            for (const auto cur: badChars)
                if (EvE::icontains(content, cur))
                    throw CustomError("Reason contains invalid characters");

            reason += PyRep::StringContent(call.byname.find("reason")->second);
        }
    } else {
        reason += "No Reason Given by ";
        reason += call.client->GetCharName();
    }

    TransferFunds(
        call.client->GetCorporationID(),
        toID->value(),
        amount->value(),
        reason.c_str(),
        Journal::EntryType::CorporationAccountWithdrawal,
        call.client->GetCharacterID(),
        fromAcctKey->value(),
        toAcctKey,
        call.client
    );

    return nullptr;
}

void AccountService::TransferFunds(
    uint32 fromID,
    uint32 toID,
    double amount,
    std::string reason /*""*/,
    uint8 entryTypeID /*Journal::EntryType::Undefined*/,
    uint32 referenceID /*0*/,
    uint16 fromKey /*Account::KeyType::Cash*/,
    uint16 toKey /*Account::KeyType::Cash*/,
    Client* pClient /*nullptr*/
) {
    if (is_log_enabled(ACCOUNT__TRACE)) {
        _log(ACCOUNT__TRACE,
            "TransferFunds() - from: %u, to: %u, entry: %u, refID: %u, amount: %.2f, fKey: %u, tKey: %u",
            fromID,
            toID,
            entryTypeID,
            referenceID,
            amount,
            fromKey,
            toKey
        );
    }

    if (amount == 0) return; // Optional fees and money-free trades use zero.
    if (!std::isfinite(amount) || amount < 0 || amount > 1000000000000.0)
        throw CustomError("Invalid payment amount.");
    amount = std::round(amount * 100.0) / 100.0;
    if (amount == 0) return;
    std::vector<AccountDB::Transfer> transfers{{fromID,toID,fromKey,toKey,amount,entryTypeID,referenceID,reason}};
    Client* recipient = IsCharacterID(toID) ? sEntityList.FindClientByCharID(toID) : nullptr;
    uint8 taxEntry = 0;
    switch (entryTypeID) {
        case Journal::EntryType::BountyPrize:
        case Journal::EntryType::BountyPrizes: taxEntry = Journal::EntryType::CorporationTaxNpcBounties; break;
        case Journal::EntryType::AgentMissionReward: taxEntry = Journal::EntryType::CorporationTaxAgentRewards; break;
        case Journal::EntryType::AgentMissionTimeBonusReward: taxEntry = Journal::EntryType::CorporationTaxAgentBonusRewards; break;
    }
    if (taxEntry && IsCharacterID(toID) && (!recipient || !recipient->IsCharCreation())) {
        const uint32 corp = recipient ? recipient->GetCorporationID() : CharacterDB::GetCorpID(toID);
        const double rate = recipient ? recipient->GetCorpTaxRate() : CharacterDB::GetCorpTaxRate(toID);
        const double tax = std::round(rate * amount * 100.0) / 100.0;
        const bool delayedBounty = sConfig.server.BountyPayoutDelayed && (entryTypeID == Journal::EntryType::BountyPrize || entryTypeID == Journal::EntryType::BountyPrizes);
        if (IsPlayerCorp(corp) && tax > 0 && tax >= sConfig.rates.TaxAmount && (!delayedBounty || amount >= sConfig.rates.TaxedAmount))
            transfers.push_back({toID,corp,toKey,Account::KeyType::Cash,tax,taxEntry,referenceID,"DESC: Corporation Tax"});
    }
    const auto changes = AccountDB::TransferAtomic(transfers,pClient ? pClient->GetCharacterID() : 0);
    // Nested transfers publish only when the complete exchange has committed.
    DBTransaction::Publish([changes]() {
    for (const auto& change : changes) {
        if (IsCharacterID(change.ownerID)) {
            Client* client = sEntityList.FindClientByCharID(change.ownerID);
            if (client) client->GetChar()->SetBalanceFromDatabase(change.balance,change.currency);
        } else {
            OnAccountChange notification;
            notification.ownerid = change.ownerID;
            notification.balance = change.balance;
            notification.accountKey = change.key == Account::KeyType::Cash ? "cash" : "cash" + std::to_string(change.key-Account::KeyType::Cash+1);
            sEntityList.CorpNotify(change.ownerID,126,"OnAccountChange","*corpid&corpAccountKey",notification.Encode());
        }
    }
    });
}
