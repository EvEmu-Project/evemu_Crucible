/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2011 The EVEmu Team
	For the latest information visit http://evemu.org
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
	Author:		Zhur
*/

#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(AccountService)

AccountService::AccountService(PyServiceMgr *mgr)
: PyService(mgr, "account"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);
	
	PyCallable_REG_CALL(AccountService, GetCashBalance)
	PyCallable_REG_CALL(AccountService, GetEntryTypes)
	PyCallable_REG_CALL(AccountService, GetKeyMap)
	PyCallable_REG_CALL(AccountService, GiveCash)
	PyCallable_REG_CALL(AccountService, GiveCashFromCorpAccount)
	PyCallable_REG_CALL(AccountService, GetJournal)	
}

AccountService::~AccountService() {
	delete m_dispatch;
}

PyResult AccountService::Handle_GetCashBalance(PyCallArgs &call) {
	Call_SingleArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return NULL;
	}
	
	//we can get an integer or a boolean right now...
	bool corporate_wallet = false;
	
	if( args.arg->IsInt() )
		corporate_wallet = ( args.arg->AsInt()->value() != 0 );
	else if( args.arg->IsBool() )
		corporate_wallet = args.arg->AsBool()->value();
	else
    {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return NULL;
	}
	
	if(corporate_wallet)
        //corporate wallet
		return new PyFloat( m_db.GetCorpBalance( call.client->GetCorporationID() ) );
    else
        //personal wallet
		return new PyFloat( call.client->GetBalance() );
}

//givecash takes (ownerID, retval['qty'], retval['reason'][:40])
//GiveCashFromCorpAccount(ownerID, retval['qty'], retval['reason'][:40])
// notify OnAccountChange:
// 		accountKey: 'cash', ownerID: charID or corpID, new balance

PyResult AccountService::Handle_GetEntryTypes(PyCallArgs &call) {
	PyRep *result = NULL;

	ObjectCachedMethodID method_id(GetName(), "GetEntryTypes");

	//check to see if this method is in the cache already.
	if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
		//this method is not in cache yet, load up the contents and cache it.
		result = m_db.GetEntryTypes();
		if(result == NULL) {
			codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
			result = new PyNone();
		}
		m_manager->cache_service->GiveCache(method_id, &result);
	}
	
	//now we know its in the cache one way or the other, so build a 
	//cached object cached method call result.
	result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);
	
	return result;
}

PyResult AccountService::Handle_GetKeyMap(PyCallArgs &call) {
	PyRep *result = NULL;

	ObjectCachedMethodID method_id(GetName(), "GetKeyMap");

	//check to see if this method is in the cache already.
	if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
		//this method is not in cache yet, load up the contents and cache it.
		result = m_db.GetKeyMap();
		if(result == NULL) {
			codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
			result = new PyNone();
		}
		m_manager->cache_service->GiveCache(method_id, &result);
	}
	
	//now we know its in the cache one way or the other, so build a 
	//cached object cached method call result.
	result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);
	
	return result;
}

//give cash takes (ownerID, retval['qty'], retval['reason'][:40])
PyResult AccountService::Handle_GiveCash(PyCallArgs &call) {
	Call_GiveCash args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return NULL;
	}

	if(args.amount == 0)
		return NULL;

	if(args.amount < 0 || args.amount > call.client->GetBalance()) {
		_log(CLIENT__ERROR, "%s: Invalid amount in GiveCash(): %.2f", call.client->GetName(), args.amount);
		call.client->SendErrorMsg("Invalid amount '%.2f'", args.amount);
		return NULL;
	}
	
	SystemManager *system = call.client->System();
	if(system == NULL) {
		codelog(CLIENT__ERROR, "%s: bad system", call.client->GetName());
		return NULL;
	}

	//NOTE: this will need work once we reorganize the entity list...
	bool targetIsChar;
	Client *other = m_manager->entity_list.FindCharacter(args.destination);
	if(other == NULL) {
		// then the money has to be sent to a corporation...
		// check this too
		if (m_db.CheckIfCorporation(args.destination)) {
			targetIsChar = false;
		} else {
			_log(CLIENT__ERROR, "%s: Failed to find character %u", call.client->GetName(), args.destination);
			call.client->SendErrorMsg("Unable to find the target");
			return NULL;
		}
	} else {
		targetIsChar = true;
	}


	if (targetIsChar) {
		return GiveCashToChar(call.client, other, args.amount, args.reason.c_str(), RefType_playerDonation);
	} else {
		// here comes the corp's stuff
		return GiveCashToCorp(call.client, args.destination, args.amount, args.reason.c_str(), RefType_playerDonation);
	}
}
	
PyTuple * AccountService::GiveCashToCorp(Client * const client, uint32 corpID, double amount, const char *reason, JournalRefType refTypeID) {
	if(!client->AddBalance(-amount)) {
		_log(CLIENT__ERROR, "%s: Failed to remove %.2f ISK from %u for donation to %u", 
			client->GetName(),
			amount,
			client->GetCharacterID(),
			corpID );
		client->SendErrorMsg("Failed to transfer money from your account.");
		return NULL;
	}
	if(!m_db.AddBalanceToCorp(corpID, amount)) {
		_log(CLIENT__ERROR, "%s: Failed to add %.2f ISK to %u for donation from %u", 
			client->GetName(),
			amount,
			corpID,
			client->GetCharacterID());
		client->SendErrorMsg("Failed to transfer money to your destination.");
		
		//try to refund the money..
		client->AddBalance(amount);
		
		return NULL;
	}
	
	double cnb = m_db.GetCorpBalance(corpID);

	// Send notification about the cash change
	OnAccountChange oac;
	oac.accountKey = "cash";
	oac.balance = cnb;
	oac.ownerid = corpID;
	PyTuple * answer = oac.Encode();

	MulticastTarget mct;
	mct.corporations.insert(corpID);
	m_manager->entity_list.Multicast("OnAccountChange", "*corpid&corpAccountKey", &answer, mct);

	//record the transactions in the wallet.
	if(!m_db.GiveCash(
		client->GetCharacterID(),
		refTypeID,
		client->GetCharacterID(),
		corpID,
		"unknown",
		client->GetAccountID(),
		accountCash,
		-amount,
		client->GetBalance(),
		reason
		)
	) {
		codelog(CLIENT__ERROR, "Failed to record transaction on sending side");
		//no good reason to return... the money has actually been moved.
	}
	
	if(!m_db.GiveCash(
		corpID,
		refTypeID,
		client->GetCharacterID(),
		corpID,
		"unknown",
		corpID,
		accountCash,
		amount,
		cnb,
		reason
		)
	) {
		codelog(CLIENT__ERROR, "Failed to record transaction on recieveing side");
		//no good reason to return... the money has actually been moved.
	}
	
	//send back the new balance
	PyTuple *ans= new PyTuple(2);
	ans->items[0]=new PyFloat(cnb);//new balance
	ans->items[1]=new PyFloat(cnb);//new balance, not an error need to send it 2 times
	
	return ans;
}
 
PyTuple * AccountService::GiveCashToChar(Client * const client, Client * const other, double amount, const char *reason, JournalRefType refTypeID) {
	if(!client->AddBalance(-amount)) {
		_log(CLIENT__ERROR, "%s: Failed to remove %.2f ISK from %u for donation to %u", 
			client->GetName(),
			amount,
			client->GetCharacterID(),
			other->GetCharacterID() );
		client->SendErrorMsg("Failed to transfer money from your account.");
		return NULL;
	}
	if(!other->AddBalance(amount)) {
		_log(CLIENT__ERROR, "%s: Failed to add %.2f ISK to %u for donation from %u", 
			client->GetName(),
			amount,
			other->GetCharacterID(),
			client->GetCharacterID());
		client->SendErrorMsg("Failed to transfer money to your destination.");
		
		//try to refund the money..
		client->AddBalance(amount);
		
		return NULL;
	}
	
	//record the transactions in the wallet.
	//first on the send side.
	if(!m_db.GiveCash(
		client->GetCharacterID(),
		refTypeID,
		client->GetCharacterID(),
		other->GetCharacterID(),
		"unknown",
		client->GetAccountID(),
		accountCash,
		-amount,
		client->GetBalance(),
		reason
		)
	) {
		codelog(CLIENT__ERROR, "Failed to record transaction on recieveing side");
		//no good reason to return... the money has actually been moved.
	}

	//then on the receive side.
	if(!m_db.GiveCash(
		other->GetCharacterID(),
		refTypeID,
		other->GetCharacterID(),
		client->GetCharacterID(),
		"unknown",
		other->GetAccountID(),
		accountCash,
		amount,
		other->GetBalance(),
		reason
		)
	) {
		codelog(CLIENT__ERROR, "Failed to record transaction on sending side");
		//no good reason to return... the money has actually been moved.
	}

	
	//send back the new balance
	PyTuple *ans= new PyTuple(2);
	ans->items[0]=new PyFloat(client->GetBalance());//new balance
	ans->items[1]=new PyFloat(client->GetBalance());//new balance, not an error need to send it 2 times
	
	return ans;
}

PyResult AccountService::Handle_GetJournal(PyCallArgs &call) {
	Call_GetJournal args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return NULL;
	}

	bool ca = false;
	if( args.corpAccount->IsBool() )
		ca = args.corpAccount->AsBool()->value();
	else if( args.corpAccount->IsInt() )
		ca = ( args.corpAccount->AsInt()->value() != 0 );
	else
    {
		// problem
		_log( SERVICE__WARNING, "%s: Unsupported value for corpAccount", GetName() );

		return NULL;
	}
		
    return m_db.GetJournal(
        ( ca ? call.client->GetCorporationID() : call.client->GetCharacterID() ),
        args.refTypeID,
        args.accountKey,
        args.fromDate
    );
}

PyResult AccountService::Handle_GiveCashFromCorpAccount(PyCallArgs &call) {
	Call_GiveCash args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return NULL;
	}

	if(args.amount == 0)
		return NULL;

	if(args.amount < 0 || args.amount > m_db.GetCorpBalance(call.client->GetCorporationID())) {
		_log(CLIENT__ERROR, "%s: Invalid amount in GiveCashFromCorpAccount(): %.2f", call.client->GetName(), args.amount);
		call.client->SendErrorMsg("Invalid amount '%.2f'", args.amount);
		return NULL;
	}
	
	SystemManager *system = call.client->System();
	if(system == NULL) {
		codelog(CLIENT__ERROR, "%s: bad system", call.client->GetName());
		return NULL;
	}

	//NOTE: this will need work once we reorganize the entity list...
	Client *other = m_manager->entity_list.FindCharacter(args.destination);
	if(other == NULL) {
		_log(CLIENT__ERROR, "%s: Failed to find character %u", call.client->GetName(), args.destination);
		call.client->SendErrorMsg("Unable to find the target");
		return NULL;
	}


	return WithdrawCashToChar(call.client, other, args.amount, args.reason.c_str(), RefType_corpAccountWithdrawal);
}

PyTuple * AccountService::WithdrawCashToChar(Client * const client, Client * const other, double amount, const char *reason, JournalRefType refTypeID) {
	// remove money from the corp
	uint32 corpID = client->GetCorporationID();
	if (!m_db.AddBalanceToCorp(corpID, double(-amount))) {
		_log(CLIENT__ERROR, "%s: Failed to remove %.2f ISK from %u for withdrawal to %u", 
			client->GetName(),
			amount,
			corpID,
			other->GetCharacterID() );
		client->SendErrorMsg("Failed to transfer money from your account.");
		return NULL;
	}

	double ncb = m_db.GetCorpBalance(corpID);

	// Send notification about the cash change
	OnAccountChange oac;
	oac.accountKey = "cash";
	oac.balance = ncb;
	oac.ownerid = corpID;
	PyTuple * answer = oac.Encode();

	MulticastTarget mct;
	mct.corporations.insert(corpID);
	m_manager->entity_list.Multicast("OnAccountChange", "*corpid&corpAccountKey", &answer, mct);

	if(!other->AddBalance(amount)) {
		_log(CLIENT__ERROR, "%s: Failed to add %.2f ISK to %u for donation from %u", 
			client->GetName(),
			amount,
			corpID,
			client->GetCharacterID());
		client->SendErrorMsg("Failed to transfer money to your destination.");
		
		//try to refund the money..
		m_db.AddBalanceToCorp(corpID, double(amount));
		// if we're here, we have a more serious problem than
		// corp's balance not being displayed properly, so i won't bother with it
		
		return NULL;
	}
	
	//record the transactions in the wallet.
	//first on the send side.
	char argID[15];
	snprintf(argID, 14, "%u", client->GetCharacterID());
	if(!m_db.GiveCash(
		corpID,
		refTypeID,
		corpID,
		other->GetCharacterID(),
		argID,
		corpID,
		accountCash,
		-amount,
		ncb,
		reason
		)
	) {
		codelog(CLIENT__ERROR, "Failed to record transaction on recieveing side");
		//no good reason to return... the money has actually been moved.
	}

	//then on the receive side.
	if(!m_db.GiveCash(
		other->GetCharacterID(),
		refTypeID,
		corpID,
		other->GetCharacterID(),
		argID,
		other->GetAccountID(),
		accountCash,
		amount,
		other->GetBalance(),
		reason
		)
	) {
		codelog(CLIENT__ERROR, "Failed to record transaction on sending side");
		//no good reason to return... the money has actually been moved.
	}

	
	//send back the new balance
	PyTuple *ans= new PyTuple(2);

	// maybe this needs it this way, just like the other ones...
	// i'm not sure, but it works for sure
	ans->items[0]=new PyFloat(ncb);
	ans->items[1]=new PyFloat(ncb);
	
	return ans;
}
