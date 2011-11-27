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
	Author:		caytchen
*/

#include "EVEServerPCH.h"

PyRep* MailDB::GetLabels(int characterID) const
{
	DBQueryResult res;
	if (!sDatabase.RunQuery(res, "SELECT bit, name, color, ownerId FROM mailLabel WHERE ownerID = %u", characterID))
		return NULL;
	 
	PyDict* ret = new PyDict();

	DBResultRow row;
	while (res.GetRow(row))
	{
		MailLabel label;
		label.id = (int)pow((float)2, row.GetInt(0));
		label.name = row.GetText(1);
		label.color = row.GetInt(2);

		ret->SetItem(new PyInt(label.id), label.Encode());
	}

	return ret;
}

bool MailDB::CreateLabel(int characterID, Call_CreateLabel& args, uint32& newID) const
{
	// we need to get the next free bit index; can't avoid a SELECT
	DBQueryResult res;
	sDatabase.RunQuery(res, "SELECT bit FROM mailLabel WHERE ownerID = %u ORDER BY bit DESC LIMIT 1", characterID);

	// 6 is a guessed default; there are some hardcoded labels that we don't have the details on yet
	int bit = 6;

	if (res.GetRowCount() > 0)
	{
		DBResultRow row;
		res.GetRow(row);
		// we want the next one, not the current one, so +1
		bit = row.GetInt(0) + 1;
	}

	DBerror error;
	if (!sDatabase.RunQuery(error, "INSERT INTO mailLabel (bit, name, color, ownerID) VALUES (%u, %s, %u, %u)", bit, args.name.c_str(), args.color, characterID))
	{
		codelog(SERVICE__ERROR, "Failed to insert new mail label into database");
		// since this is an out parameter, make sure we assign this even in case of an error
		newID = 0;
		return false;
	}

	// the client wants the power of 2, not the bitset index
	newID = (uint32)pow((float)2, bit);
	return true;
}

void MailDB::DeleteLabel(int characterID, int labelID) const
{
	int bit = BitFromLabelID(labelID);
	DBerror error;
	sDatabase.RunQuery(error, "DELETE FROM mailLabel WHERE ownerID = %u AND bit = %u", characterID, bit); 
}

void MailDB::EditLabel(int characterID, Call_EditLabel& args) const
{
	int bit = BitFromLabelID(args.labelId);

	DBerror error;
	if (args.name.length() == 0)
		sDatabase.RunQuery(error, "UPDATE mailLabel SET color = %u WHERE bit = %u AND ownerID = %u", args.color, bit, characterID);
	else if (args.color == -1)
		sDatabase.RunQuery(error, "UPDATE mailLabel SET name = %s WHERE bit = %u AND ownerID = %u", args.name.c_str(), bit, characterID);
	else
		sDatabase.RunQuery(error, "UPDATE mailLabel SET name = %s, color = %u WHERE bit = %u AND ownerID = %u", args.name.c_str(), args.color, bit, characterID);
}

int MailDB::BitFromLabelID(int id)
{
	// lets hope the compiler can do this better; I guess it still beats a floating point log, though
	for (int i = 0; i < sizeof(int); i++)
		if ((id & (1 << i)) > 0)
			return i;

    // This just gets rid of a warning, code execution should never reach here.
    sLog.Error("MailDB::BitFromLabelID", "ERROR: Could not get bit.");
    return NULL;
}
