/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "EvemuPCH.h"



PyCallable_Make_InnerDispatcher(AccountService)

AccountService::AccountService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "account"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);
	
	PyCallable_REG_CALL(AccountService, GetCashBalance)
	PyCallable_REG_CALL(AccountService, GetRefTypes)
	PyCallable_REG_CALL(AccountService, GetKeyMap)
	PyCallable_REG_CALL(AccountService, GiveCash)//mandela
	PyCallable_REG_CALL(AccountService, GiveCashFromCorpAccount)
	PyCallable_REG_CALL(AccountService, GetJournal)//mandela
	
}

AccountService::~AccountService() {
	delete m_dispatch;
}


PyResult AccountService::Handle_GetCashBalance(PyCallArgs &call) {
	Call_SingleArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return(NULL);
	}
	
	//we can get an integer or a boolean right now...
	bool corporate_wallet = false;
	
	if(args.arg->CheckType(PyRep::Integer)) {
		PyRepInteger *i = (PyRepInteger *) args.arg;
		corporate_wallet = (i->value != 0);
	} else if(args.arg->CheckType(PyRep::Boolean)) {
		PyRepBoolean *i = (PyRepBoolean *) args.arg;
		corporate_wallet = i->value;
	} else {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return(NULL);
	}
	
	
	if(!corporate_wallet) { //personal wallet
		PyRep *result = new PyRepReal(call.client->GetBalance());
		return(result);
	} else {	//corporate wallet
		PyRep *result = new PyRepReal(
			m_db.GetCorpBalance(call.client->GetCorporationID())
			);
		return(result);
	}
	return(NULL);
}

//givecache takes (ownerID, retval['qty'], retval['reason'][:40])
//GiveCashFromCorpAccount(ownerID, retval['qty'], retval['reason'][:40])
// notify OnAccountChange:
// 		accountKey: 'cash', ownerID: charID or corpID, new balance

PyResult AccountService::Handle_GetRefTypes(PyCallArgs &call) {
	PyRep *result = NULL;

	ObjectCachedMethodID method_id(GetName(), "GetRefTypes");

	//check to see if this method is in the cache already.
	ObjCacheService *cache = m_manager->GetCache();
	if(!cache->IsCacheLoaded(method_id)) {
		//this method is not in cache yet, load up the contents and cache it.
		result = m_db.GetRefTypes();
		if(result == NULL) {
			codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
			result = new PyRepNone();
		}
		cache->GiveCache(method_id, &result);
	}
	
	//now we know its in the cache one way or the other, so build a 
	//cached object cached method call result.
	result = cache->MakeObjectCachedMethodCallResult(method_id);
	
	return(result);
}

PyResult AccountService::Handle_GetKeyMap(PyCallArgs &call) {
	PyRep *result = NULL;

	ObjectCachedMethodID method_id(GetName(), "GetKeyMap");

	//check to see if this method is in the cache already.
	ObjCacheService *cache = m_manager->GetCache();
	if(!cache->IsCacheLoaded(method_id)) {
		//this method is not in cache yet, load up the contents and cache it.
		result = m_db.GetKeyMap();
		if(result == NULL) {
			codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
			result = new PyRepNone();
		}
		cache->GiveCache(method_id, &result);
	}
	
	//now we know its in the cache one way or the other, so build a 
	//cached object cached method call result.
	result = cache->MakeObjectCachedMethodCallResult(method_id);
	
	return(result);
}

//givecache takes (ownerID, retval['qty'], retval['reason'][:40])
PyResult AccountService::Handle_GiveCash(PyCallArgs &call) {
	Call_GiveCash args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return(NULL);
	}

	if(args.amount == 0)
		return(NULL);

	if(args.amount < 0 || args.amount > call.client->GetBalance()) {
		_log(CLIENT__ERROR, "%s: Invalid amount in GiveCash(): %.2f", call.client->GetName(), args.amount);
		call.client->SendErrorMsg("Invalid amount '%.2f'", args.amount);
		return(NULL);
	}
	
	SystemManager *system = call.client->System();
	if(system == NULL) {
		codelog(CLIENT__ERROR, "%s: bad system", call.client->GetName());
		return(NULL);
	}

	//NOTE: this will need work once we reorganize the entity list...
	bool targetIsChar;
	Client *other = m_manager->entity_list->FindCharacter(args.destination);
	if(other == NULL) {
		// then the money has to be sent to a corporation...
		// check this too
		if (m_db.CheckIfCorporation(args.destination)) {
			targetIsChar = false;
		} else {
			_log(CLIENT__ERROR, "%s: Failed to find character %lu", call.client->GetName(), args.destination);
			call.client->SendErrorMsg("Unable to find the target");
			return(NULL);
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
	
PyRepTuple * AccountService::GiveCashToCorp(Client * const client, uint32 corpID, double amount, const char *reason, JournalRefType refTypeID) {
	if(!client->AddBalance(-amount)) {
		_log(CLIENT__ERROR, "%s: Failed to remove %.2f ISK from %lu for donation to %lu", 
			client->GetName(),
			amount,
			client->GetCharacterID(),
			corpID );
		client->SendErrorMsg("Failed to transfer money from your account.");
		return(NULL);
	}
	if(!m_db.AddBalanceToCorp(corpID, amount)) {
		_log(CLIENT__ERROR, "%s: Failed to add %.2f ISK to %lu for donation from %lu", 
			client->GetName(),
			amount,
			corpID,
			client->GetCharacterID());
		client->SendErrorMsg("Failed to transfer money to your destination.");
		
		//try to refund the money..
		client->AddBalance(amount);
		
		return(NULL);
	}
	
	double cnb = m_db.GetCorpBalance(corpID);

	// Send notification about the cash change
	OnAccountChange oac;
	oac.accountKey = "cash";
	oac.balance = cnb;
	oac.ownerid = corpID;
	PyRepTuple * answer = oac.Encode();

	MulticastTarget mct;
	mct.corporations.insert(corpID);
	m_manager->entity_list->Multicast("OnAccountChange", "*corpid&corpAccountKey", &answer, mct);

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
	PyRepTuple *ans= new PyRepTuple(2);
	ans->items[0]=new PyRepReal(cnb);//new balance
	ans->items[1]=new PyRepReal(cnb);//new balance, not an error need to send it 2 times
	
	return ans;
}
 
PyRepTuple * AccountService::GiveCashToChar(Client * const client, Client * const other, double amount, const char *reason, JournalRefType refTypeID) {
	if(!client->AddBalance(-amount)) {
		_log(CLIENT__ERROR, "%s: Failed to remove %.2f ISK from %lu for donation to %lu", 
			client->GetName(),
			amount,
			client->GetCharacterID(),
			other->GetCharacterID() );
		client->SendErrorMsg("Failed to transfer money from your account.");
		return(NULL);
	}
	if(!other->AddBalance(amount)) {
		_log(CLIENT__ERROR, "%s: Failed to add %.2f ISK to %lu for donation from %lu", 
			client->GetName(),
			amount,
			other->GetCharacterID(),
			client->GetCharacterID());
		client->SendErrorMsg("Failed to transfer money to your destination.");
		
		//try to refund the money..
		client->AddBalance(amount);
		
		return(NULL);
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
	PyRepTuple *ans= new PyRepTuple(2);
	ans->items[0]=new PyRepReal(client->GetBalance());//new balance
	ans->items[1]=new PyRepReal(client->GetBalance());//new balance, not an error need to send it 2 times
	
	return ans;
}

PyResult AccountService::Handle_GetJournal(PyCallArgs &call) {
	Call_GetJournal args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return(NULL);
	}

	bool ca = false;
	if (args.corpAccount->CheckType(PyRep::Boolean)) {
		ca = ((PyRepBoolean *)args.corpAccount)->value;
	} else if (args.corpAccount->CheckType(PyRep::Integer)) {
		ca = (((PyRepInteger *)args.corpAccount)->value != 0);
	} else {
		// problem
		_log(SERVICE__WARNING, "%s: Unsupported value for corpAccount", GetName());
		return NULL;
	}
		
	if (ca) {
		return m_db.GetJournal(call.client->GetCorporationID(), args.refTypeID, args.accountKey, args.fromDate);
	} else {
		return m_db.GetJournal(call.client->GetCharacterID(), args.refTypeID, args.accountKey, args.fromDate);
	}
}

PyResult AccountService::Handle_GiveCashFromCorpAccount(PyCallArgs &call) {
	Call_GiveCash args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return(NULL);
	}

	if(args.amount == 0)
		return(NULL);

	if(args.amount < 0 || args.amount > m_db.GetCorpBalance(call.client->GetCorporationID())) {
		_log(CLIENT__ERROR, "%s: Invalid amount in GiveCashFromCorpAccount(): %.2f", call.client->GetName(), args.amount);
		call.client->SendErrorMsg("Invalid amount '%.2f'", args.amount);
		return(NULL);
	}
	
	SystemManager *system = call.client->System();
	if(system == NULL) {
		codelog(CLIENT__ERROR, "%s: bad system", call.client->GetName());
		return(NULL);
	}

	//NOTE: this will need work once we reorganize the entity list...
	Client *other = m_manager->entity_list->FindCharacter(args.destination);
	if(other == NULL) {
		_log(CLIENT__ERROR, "%s: Failed to find character %lu", call.client->GetName(), args.destination);
		call.client->SendErrorMsg("Unable to find the target");
		return(NULL);
	}


	return WithdrawCashToChar(call.client, other, args.amount, args.reason.c_str(), RefType_corpAccountWithdrawal);
}

PyRepTuple * AccountService::WithdrawCashToChar(Client * const client, Client * const other, double amount, const char *reason, JournalRefType refTypeID) {
	// remove money from the corp
	uint32 corpID = client->GetCorporationID();
	if (!m_db.AddBalanceToCorp(corpID, double(-amount))) {
		_log(CLIENT__ERROR, "%s: Failed to remove %.2f ISK from %lu for withdrawal to %lu", 
			client->GetName(),
			amount,
			corpID,
			other->GetCharacterID() );
		client->SendErrorMsg("Failed to transfer money from your account.");
		return(NULL);
	}

	double ncb = m_db.GetCorpBalance(corpID);

	// Send notification about the cash change
	OnAccountChange oac;
	oac.accountKey = "cash";
	oac.balance = ncb;
	oac.ownerid = corpID;
	PyRepTuple * answer = oac.Encode();

	MulticastTarget mct;
	mct.corporations.insert(corpID);
	m_manager->entity_list->Multicast("OnAccountChange", "*corpid&corpAccountKey", &answer, mct);

	if(!other->AddBalance(amount)) {
		_log(CLIENT__ERROR, "%s: Failed to add %.2f ISK to %lu for donation from %lu", 
			client->GetName(),
			amount,
			corpID,
			client->GetCharacterID());
		client->SendErrorMsg("Failed to transfer money to your destination.");
		
		//try to refund the money..
		m_db.AddBalanceToCorp(corpID, double(amount));
		// if we're here, we have a more serious problem than
		// corp's balance not being displayed properly, so i won't bother with it
		
		return(NULL);
	}
	
	//record the transactions in the wallet.
	//first on the send side.
	char argID[15];
	snprintf(argID, 14, "%lu", client->GetCharacterID());
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
	PyRepTuple *ans= new PyRepTuple(2);

	// maybe this needs it this way, just like the other ones...
	// i'm not sure, but it works for sure
	ans->items[0]=new PyRepReal(ncb);
	ans->items[1]=new PyRepReal(ncb);
	
	return ans;
}















