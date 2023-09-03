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

PyResult AccountService::GiveCash(PyCallArgs &call, PyInt* toID, PyFloat* amount, std::string reason)
{
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

    TranserFunds(call.client->GetCharacterID(), toID->value(), amount->value(), reasonStr.c_str(), Journal::EntryType::PlayerDonation, call.client->GetCharacterID());
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

    TranserFunds(call.client->GetCorporationID(), toID->value(), amount->value(), reason.c_str(), Journal::EntryType::CorporationAccountWithdrawal, \
                call.client->GetCharacterID(), fromAcctKey->value(), toAcctKey, call.client);
    return nullptr;
}

void AccountService::TranserFunds(uint32 fromID, uint32 toID, double amount, std::string reason /*""*/, uint8 entryTypeID /*Journal::EntryType::Undefined*/, \
                                  uint32 referenceID/*0*/, uint16 fromKey/*Account::KeyType::Cash*/, uint16 toKey/*Account::KeyType::Cash*/,
                                  Client* pClient/*nullptr*/)
{
    if (is_log_enabled(ACCOUNT__TRACE))
        _log(ACCOUNT__TRACE, "TranserFunds() - from: %u, to: %u, entry: %u, refID: %u, amount: %.2f, fKey: %u, tKey: %u", \
                            fromID, toID, entryTypeID, referenceID, amount, fromKey, toKey);
    uint8 fromCurrency = Account::CreditType::ISK;
    if (IsAUR(fromKey)) {
        fromCurrency = Account::CreditType::AURUM;
    } else if (IsDustKey(fromKey)) {
        fromCurrency = Account::CreditType::MPLEX;
    }

    double newBalanceFrom(0), newBalanceTo(0);
    Client* pClientFrom(nullptr);
    if (IsCharacterID(fromID)) {
        pClientFrom = sEntityList.FindClientByCharID(fromID);
        if (pClientFrom == nullptr) {
            // sender is offline. xfer funds thru db.
            newBalanceFrom = AccountDB::OfflineFundXfer(fromID, -amount, fromCurrency);
        } else {
            // this will throw if it fails
            pClientFrom->AddBalance(-amount, fromCurrency);
            newBalanceFrom = pClientFrom->GetBalance(fromCurrency);
        }
        AccountDB::AddJournalEntry(fromID, entryTypeID, fromID, toID, fromCurrency, fromKey, -amount, newBalanceFrom, reason, referenceID);
    } else if (IsPlayerCorp(fromID)) {
        uint32 userID(0);
        if (pClient != nullptr)
            userID = pClient->GetCharacterID();
        HandleCorpTransaction(fromID, entryTypeID, userID?userID:fromID, toID, fromCurrency, fromKey, -amount, reason, referenceID);
    } // fromID could be npc or _System.  nothing to do on this side.

    uint8 toCurrency = Account::CreditType::ISK;
    if (IsAUR(toKey)) {
        toCurrency = Account::CreditType::AURUM;
    } else if (IsDustKey(toKey)) {
        toCurrency = Account::CreditType::MPLEX;
    }

    Client* pClientTo(nullptr);
    if (IsCharacterID(toID)) {
        pClientTo = sEntityList.FindClientByCharID(toID);
        if (pClientTo == nullptr) {
            // receipient is offline. xfer funds thru db
            newBalanceTo = AccountDB::OfflineFundXfer(toID, amount, toCurrency);
        } else {
            // this will throw if it fails
            pClientTo->AddBalance(amount, toCurrency);
            /** @todo if this DOES fail, return funds to origin.  this needs a try/catch block */
            //TranserFunds(corpSCC, fromID, amount, reason, Journal::EntryType::Undefined, referenceID, fromKey, fromKey);
            newBalanceTo = pClientTo->GetBalance(toCurrency);
        }
        AccountDB::AddJournalEntry(toID, entryTypeID, fromID, toID, toCurrency, toKey, amount, newBalanceTo, reason, referenceID);
    } else if (IsPlayerCorp(toID)) {
        uint32 userID(0);
        if (pClient != nullptr)
            userID = pClient->GetCharacterID();
        HandleCorpTransaction(toID, entryTypeID, fromID, userID?userID:toID, toCurrency, toKey, amount, reason, referenceID);
        return;
    } else {
        _log(ACCOUNT__TRACE, "TranserFunds() - toID: %s(%u) is neither player nor player corp.  Not sending update.", \
                sDataMgr.GetCorpName(toID).c_str(), toID);
        return;
    }

    if ((pClientTo != nullptr) and pClientTo->IsCharCreation())
        return;

    /* corp taxes...
     *   bounty prizes and mission rewards are taxed by the players corp based on corp tax rate.
     *   there is a possibility the char receiving these payments could be offline (for whatever reason)
     *   these payments are only taxed if they are above amount set in server config.
     */

    // are bounty payments grouped on timer?
    if ((entryTypeID == Journal::EntryType::BountyPrize)
    or  (entryTypeID == Journal::EntryType::BountyPrizes))
        if (sConfig.server.BountyPayoutDelayed)
            if (amount < sConfig.rates.TaxedAmount)  // is amount worth taxing?  default is 75k
                return;
    float tax = 0;
    uint32 corpID = 0;
    if (pClientTo != nullptr) {
        tax = pClientTo->GetCorpTaxRate() * amount;
        corpID = pClientTo->GetCorporationID();
    } else {
        //  recipient is offline...try to get needed data from db
        tax = CharacterDB::GetCorpTaxRate(toID) * amount;
        corpID = CharacterDB::GetCorpID(toID);
    }

    // just in case something went wrong.....
    if (!IsCorp(corpID))
        return;
    // is tax worth the accounting hassle? (from corp pov)  default is 5k
    if (tax < sConfig.rates.TaxAmount)
        return;

    reason = "DESC: Corporation Tax on pirate bounty";
    switch (entryTypeID) {
        // Corp Taxed payment types
        case Journal::EntryType::BountyPrize:
        case Journal::EntryType::BountyPrizes: {
            TranserFunds(toID, corpID, tax, reason.c_str(), Journal::EntryType::CorporationTaxNpcBounties, referenceID);
        } break;
        case Journal::EntryType::AgentMissionReward: {
            TranserFunds(toID, corpID, tax, reason.c_str(), Journal::EntryType::CorporationTaxAgentRewards, referenceID);
        } break;
        case Journal::EntryType::AgentMissionTimeBonusReward: {
            TranserFunds(toID, corpID, tax, reason.c_str(), Journal::EntryType::CorporationTaxAgentBonusRewards, referenceID);
        } break;
    }
}

void AccountService::HandleCorpTransaction(uint32 corpID, int8 entryTypeID, uint32 fromID, uint32 toID, int8 currency, \
                                            uint16 accountKey, double amount, std::string description, \
                                            uint32 referenceID/*0*/)
{
    if (is_log_enabled(ACCOUNT__TRACE))
        _log(ACCOUNT__TRACE, "HandleCorpTransaction() - corp: %u, from: %u, to: %u, entry: %u, refID: %u, amount: %.2f, key: %u, currency: %u", \
                        corpID, fromID, toID, entryTypeID, referenceID, amount, accountKey, currency);
    double balance = AccountDB::GetCorpBalance(corpID, accountKey);
    // verify funds available for withdraw first
    if (amount < 0) {
        if (-amount > balance) {
            std::map<std::string, PyRep *> args;
            args["owner"] = new PyString(CorporationDB::GetCorpName(corpID));
            args["amount"] = new PyFloat(-amount);
            args["balance"] = new PyFloat(balance);
            args["division"] = new PyString(CorporationDB::GetDivisionName(corpID, accountKey));
            throw UserError ("NotEnoughMoneyCorp")
                    .AddOwnerName ("owner", corpID)
                    .AddISK ("amount", -amount)
                    .AddISK ("balance", balance)
                    .AddFormatValue ("division", new PyString (CorporationDB::GetDivisionName (corpID, accountKey)));
        }
    }
    // get new corp balance
    balance += amount;
    // update corp balance
    AccountDB::UpdateCorpBalance(corpID, accountKey, balance);

    OnAccountChange oac;
    switch (accountKey) {
        case Account::KeyType::Cash2: oac.accountKey = "cash2"; break;
        case Account::KeyType::Cash3: oac.accountKey = "cash3"; break;
        case Account::KeyType::Cash4: oac.accountKey = "cash4"; break;
        case Account::KeyType::Cash5: oac.accountKey = "cash5"; break;
        case Account::KeyType::Cash6: oac.accountKey = "cash6"; break;
        case Account::KeyType::Cash7: oac.accountKey = "cash7"; break;
        case Account::KeyType::Cash:
        default:                      oac.accountKey = "cash";  break;
    }
    oac.balance = balance;
    oac.ownerid = corpID;
    sEntityList.CorpNotify(corpID, 126 /*WalletChange*/, "OnAccountChange", "*corpid&corpAccountKey", oac.Encode());
    AccountDB::AddJournalEntry(corpID, entryTypeID, fromID, toID, currency, accountKey, amount, balance, description, referenceID);
}
