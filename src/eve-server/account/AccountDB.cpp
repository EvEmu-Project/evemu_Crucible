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

PyObject *AccountDB::GetEntryTypes() {
	DBQueryResult res;
	
	if(!sDatabase.RunQuery(res, "SELECT refTypeID AS entryTypeID,refTypeText AS entryTypeName,description FROM market_refTypes"))
    {
        sLog.Error("Account DB", "Error in query: %s", res.error.c_str());
		return NULL;
	}
	
	return DBResultToRowset(res);
}

PyObject *AccountDB::GetKeyMap() {
	DBQueryResult res;
	
	if(!sDatabase.RunQuery(res, "SELECT accountKey AS keyID,accountType AS keyType,accountName AS keyName,description FROM market_keyMap"))
    {
        sLog.Error("Account DB", "Error in query: %s", res.error.c_str());
		return NULL;
	}
	
	return DBResultToRowset(res);
}

PyObject *AccountDB::GetJournal(uint32 charID, uint32 refTypeID, uint32 accountKey, uint64 transDate) {
//'refID', 'transDate', 'refTypeID','ownerID1', 'ownerID2', 'argID1', 'accountID', 'amount', 'balance', 'reason'

	DBQueryResult res;
	uint64 dT;
	//dF = transDate;
	dT = transDate - Win32Time_Day;
	// 1 sec = 10.000.000 wow...
	
	if(!sDatabase.RunQuery(res,
		"SELECT refID AS transactionID,transDate AS transactionDate,0 AS referenceID, refTypeID AS entryTypeID,ownerID1,ownerID2,argID1, accountKey,amount,balance,reason AS description "
		"FROM market_journal "
		"WHERE (transDate >= " I64u " AND transDate <= " I64u ") "
		"AND accountKey = %u "
		"AND (0 = %u OR refTypeID = %u) "
		"AND characterID=%u" , dT, transDate, accountKey, refTypeID, refTypeID, charID))
    {
        sLog.Error("Account DB", "Error in query: %s", res.error.c_str());
		return NULL;
	}
	
	return DBResultToRowset(res);
}

//////////////////////////////////
// temporarily moved into ServiceDB because other services needed access to
// it, eventually something better will need to be done (as the account system
// grows up)
bool ServiceDB::GiveCash( uint32 characterID, JournalRefType refTypeID, uint32 ownerFromID, uint32 ownerToID, const char *argID1,
	uint32 accountID, EVEAccountKeys accountKey, double amount, double balance, const char *reason )
{
//the only unknown it is argID1 , what is it ?
	DBQueryResult res;
	DBerror err;
	
	std::string eReason;
	sDatabase.DoEscapeString(eReason, reason);
	std::string eArg1;
	sDatabase.DoEscapeString(eArg1, argID1);

	if(!sDatabase.RunQuery(err, 
        "INSERT INTO market_journal(characterID,refID,transDate,refTypeID,ownerID1,ownerID2,argID1,accountID,accountKey,amount,balance,reason) "
        "VALUES (%u,NULL," I64u ",%u,%u,%u,\"%s\",%u,%u,%.2f,%.2f,\"%s\")",
		characterID, Win32TimeNow(), refTypeID, ownerFromID, ownerToID, eArg1.c_str(), accountID, accountKey, amount, balance, eReason.c_str()))
	{
        sLog.Error("Service DB", "Error in query : %s", err.c_str());
		return false;
	}

	return true;
}

bool AccountDB::CheckIfCorporation(uint32 corpID) {
	DBQueryResult res;
	DBResultRow row;
	if (!sDatabase.RunQuery(res, "SELECT corporationID FROM corporation WHERE corporationID = %u ", corpID))
	{
        sLog.Error("Service DB", "Error in query: %s", res.error.c_str());
		return false;
	}

	if (!res.GetRow(row))
    {
        sLog.Error("Service DB", "Failed to find corporation %u", corpID);
		return false;
	}
	return true;
}

bool ServiceDB::AddBalanceToCorp(uint32 corpID, double amount) {
	DBerror err;
	if (!sDatabase.RunQuery(err, "UPDATE corporation SET balance = balance + (%lf) WHERE corporationID = %u ", amount, corpID))
	{
        sLog.Error("Service DB", "Error in query: %s", err.c_str());
		return false;
	}
	return true;
}

double ServiceDB::GetCorpBalance(uint32 corpID) {
	DBQueryResult res;
	DBResultRow row;
	if (!sDatabase.RunQuery(res, "SELECT balance FROM corporation WHERE corporationID = %u ", corpID))
	{
        sLog.Error("Service DB", "Error in query: %s", res.error.c_str());
		return 0.0;
	}
	if (!res.GetRow(row))
    {
        sLog.Error("Service DB", "Corporation %u missing from database.", corpID);
		return 0.0;
	}
	return row.GetDouble(0);
}
