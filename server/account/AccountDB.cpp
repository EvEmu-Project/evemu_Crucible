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

AccountDB::AccountDB(DBcore *db)
: ServiceDB(db)
{
}

AccountDB::~AccountDB() {
}

PyRepObject *AccountDB::GetRefTypes() {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		"	refTypeID,refTypeText,description"
		" FROM market_refTypes"
	)) {
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

PyRepObject *AccountDB::GetKeyMap() {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		"	accountKey,accountType,accountName,description"
		" FROM market_keyMap"
	)) {
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

PyRepObject *AccountDB::GetJournal(uint32 charID, uint32 refTypeID, uint32 accountKey, uint64 transDate) {
//'refID', 'transDate', 'refTypeID','ownerID1', 'ownerID2', 'argID1', 'accountID', 'amount', 'balance', 'reason'

	DBQueryResult res;
	uint64 dT;
	//dF = transDate;
	dT = transDate - Win32Time_Day;
	// 1 sec = 10.000.000 wow...
	
	if(!m_db->RunQuery(res,
		"SELECT"
		" refID,transDate,refTypeID,ownerID1,ownerID2,argID1,"
		"	accountKey,amount,balance,reason"
		" FROM market_journal"
		" WHERE (transDate >= " I64u " AND transDate <= " I64u ") "
		" 	AND accountKey = %lu "
		" 	AND (0 = %lu OR refTypeID = %lu) "
		" 	AND characterID=%lu" , dT, transDate, accountKey, refTypeID, refTypeID, charID
	)) {
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

//////////////////////////////////
// temporarily moved into ServiceDB because other services needed access to
// it, eventually something better will need to be done (as the account system
// grows up)
bool ServiceDB::GiveCash(
	uint32 characterID,
	JournalRefType refTypeID,
	uint32 ownerFromID,
	uint32 ownerToID,
	const char *argID1,
	uint32 accountID,
	EVEAccountKeys accountKey,
	double amount,
	double balance,
	const char *reason)
{
//the only unknown it is argID1 , what is it ?
	DBQueryResult res;
	DBerror err;	
	
	std::string eReason;
	m_db->DoEscapeString(eReason, reason);
	std::string eArg1;
	m_db->DoEscapeString(eArg1, argID1);

	if(!m_db->RunQuery(err,
		"INSERT INTO market_journal ("
		"	characterID, refID, transDate, refTypeID, ownerID1,"
		"	ownerID2, argID1, accountID, accountKey, amount, balance, "
		"	reason"
		" ) VALUES ("
		"	%lu, NULL, " I64u ", %lu, %lu, "
		"	%lu, \"%s\", %lu, %lu, %.2f, %.2f, "
		"	\"%s\" )",
		characterID, Win32TimeNow(), refTypeID, ownerFromID, 
		ownerToID, eArg1.c_str(), accountID, accountKey, amount, balance, 
		eReason.c_str()))

	{
		_log(SERVICE__ERROR, "Error in query : %s", err.c_str());
		return(false);
	}

	return (true);
}

bool AccountDB::CheckIfCorporation(uint32 corpID) {
	DBQueryResult res;
	DBResultRow row;
	if (!m_db->RunQuery(res,
		" SELECT corporationID "
		" FROM corporation "
		" WHERE corporationID = %lu ", corpID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}

	if (!res.GetRow(row)) {
		_log(SERVICE__MESSAGE, "Failed to find corporation %lu", corpID);
		return false;
	}
	return true;
}

bool ServiceDB::AddBalanceToCorp(uint32 corpID, double amount) {
	DBerror err;
	if (!m_db->RunQuery(err, 
		" UPDATE corporation "
		" SET balance = balance + (%lf) "
		" WHERE corporationID = %lu ", amount, corpID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		return false;
	}
	return true;
}

double ServiceDB::GetCorpBalance(uint32 corpID) {
	DBQueryResult res;
	DBResultRow row;
	if (!m_db->RunQuery(res,
		" SELECT balance "
		" FROM corporation "
		" WHERE corporationID = %lu ", corpID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return 0;
	}
	if (!res.GetRow(row)) {
		_log(SERVICE__WARNING, "Corporation %lu missing from database.", corpID);
		return 0;
	}
	return row.GetDouble(0);
}













