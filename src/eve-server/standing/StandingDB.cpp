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

PyObject *StandingDB::GetNPCStandings() {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT "
		" fromID,toID,standing"
		" FROM npcStandings"
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}
	
	return DBResultToRowset(res);
}


PyObject *StandingDB::GetCharStandings(uint32 characterID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT "
		" toID, standing"
		" FROM chrStandings"
		" WHERE characterID=%u", characterID
	))
	{
		_log(SERVICE__ERROR, "Error in GetCharStandings query: %s", res.error.c_str());
		return NULL;
	}
	
	return DBResultToRowset(res);
}


PyObject *StandingDB::GetCharPrimeStandings(uint32 characterID) {
	DBQueryResult res;

#ifndef WIN32
#warning hacking GetCharPrimeStandings until we know what it does
#endif
	if(!sDatabase.RunQuery(res,
		"SELECT "
		" itemID AS ownerID,"
		" itemName AS ownerName,"
		" typeID"
		" FROM entity"
		" WHERE itemID < 0"
	))
	{
		_log(SERVICE__ERROR, "Error in GetCharPrimeStandings query: %s", res.error.c_str());
		return NULL;
	}
	
	return DBResultToRowset(res);
}


PyObject *StandingDB::GetCharNPCStandings(uint32 characterID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT "
		" fromID, standing"
		" FROM chrNPCStandings"
		" WHERE characterID=%u", characterID
	))
	{
		_log(SERVICE__ERROR, "Error in GetCharNPCStandings query: %s", res.error.c_str());
		return NULL;
	}
	
	return DBResultToRowset(res);
}

PyObject *StandingDB::GetStandingTransactions( uint32 characterID )
{
	/*DBQueryResult res;

	if (!sDatabase.RunQuery(res,
		" SELECT * FROM chrStandingChanges "
		" WHERE toID = %u ", characterID
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}

	return (DBResultToRowset(res));*/

	//since we dont support standing changes in any way yet, its useless to have such stuff in db
    sLog.Debug( "StandingDB", "Called GetStandingTransactions stub." );

	util_Rowset res;

	res.header.push_back( "eventID" );
	res.header.push_back( "eventTypeID" );
	res.header.push_back( "eventDateTime" );
	res.header.push_back( "fromID" );
	res.header.push_back( "toID" );
	res.header.push_back( "modification" );
	res.header.push_back( "originalFromID" );
	res.header.push_back( "originalToID" );
	res.header.push_back( "int_1" );
	res.header.push_back( "int_2" );
	res.header.push_back( "int_3" );
	res.header.push_back( "msg" );

	return res.Encode();
}





















