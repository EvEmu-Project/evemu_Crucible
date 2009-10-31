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

#include "EVEServerPCH.h"

bool CommandDB::ItemSearch(const char *query, std::map<uint32, std::string> &into) {
	
	std::string escaped;
	sDatabase.DoEscapeString(escaped, query);

	DBQueryResult result;
	DBResultRow row;
	
	into.clear();

	//we need to query out the primary message here... not sure how to properly
	//grab the "main message" though... the text/plain clause is pretty hackish.
	if (!sDatabase.RunQuery(result,
		" SELECT typeID,typeName"
		" FROM invTypes"
		" WHERE"
		"  typeName rlike '%s'",
			escaped.c_str()
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", result.error.c_str());
		return (false);
	}
	
	while(result.GetRow(row)) {
		into[row.GetUInt(0)] = row.GetText(1);
	}
	return true;
}

bool CommandDB::GetRoidDist(const char * sec, std::map<double, uint32> &roids) {
	DBQueryResult res;
	DBResultRow row;

	if (!sDatabase.RunQuery(res,
		" SELECT roidID, percent FROM roidDistribution WHERE systemSec = '%s' ", sec))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}

	double tot = 0.0;
	while (res.GetRow(row)) {
		tot += row.GetDouble(1);
		roids[tot] = row.GetUInt(0);
	}

	return !roids.empty();
}
























