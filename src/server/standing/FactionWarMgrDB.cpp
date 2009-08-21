/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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

#include "EvemuPCH.h"

FactionWarMgrDB::FactionWarMgrDB(DBcore *db)
: ServiceDB(db)
{
}

PyRep *FactionWarMgrDB::GetWarFactions() {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT factionID, militiaCorporationID"
		" FROM chrFactions"
		" WHERE militiaCorporationID IS NOT NULL"))
	{
		_log(DATABASE__ERROR, "Failed to query war factions: %s.", res.error.c_str());
		return NULL;
	}

	return(DBResultToIntIntDict(res));
}

PyRep *FactionWarMgrDB::GetFacWarSystems() {
	_log(DATABASE__MESSAGE, "GetFacWarSystems unimplemented.");

	//fill some crap
	PyDict *result = new PyDict;
	PyDict *dict;

	dict = new PyDict;
	dict->add("occupierID", new PyInt(500002));
	dict->add("factionID", new PyInt(500002));
	result->add(new PyInt(30002097), dict);

	return result;
}

uint32 FactionWarMgrDB::GetFactionMilitiaCorporation(const uint32 factionID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT militiaCorporationID"
		" FROM chrFactions"
		" WHERE factionID=%u",
		factionID))
	{
		_log(DATABASE__ERROR, "Failed to query militia corporation for faction %u: %s.", factionID, res.error.c_str());
		return NULL;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Faction %u not found.", factionID);
		return NULL;
	}

	return(row.GetUInt(0));
}

