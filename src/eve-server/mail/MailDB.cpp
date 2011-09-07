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

PyRep* MailDB::GetLabels(int characterID)
{
	DBQueryResult res;
	if (!sDatabase.RunQuery(res, "SELECT labelID, name, color, ownerId FROM mailLabel WHERE ownerID = %u", characterID))
		return NULL;
	 
	PyDict* ret = new PyDict();

	DBResultRow row;
	while (res.GetRow(row))
	{
		MailLabel label;
		label.id = row.GetInt(0);
		label.name = row.GetText(1);
		label.color = row.GetInt(2);

		ret->SetItem(new PyInt(label.id), label.Encode());
	}

	return ret;
}

bool MailDB::CreateLabel(int characterID, Call_CreateLabel& args, uint32& newID)
{
	DBerror error;
	uint32 lastID;
	if (!sDatabase.RunQueryLID(error, lastID, "INSERT INTO mailLabel (name, color, ownerId) VALUES (%s, %u, %u)", args.name, args.color, characterID))
	{
		codelog(SERVICE__ERROR, "Failed to insert new mail label into database");
		// since this is an out parameter, make sure we assign this even in case of an error
		newID = 0;
		return false;
	}
	newID = lastID;
	return true;
}

void MailDB::DeleteLabel(int characterID, int labelID)
{
	DBerror error;
	sDatabase.RunQuery(error, "DELETE FROM mailLabel WHERE ownerId = %u AND labelID = %u", characterID, labelID); 
}

void MailDB::EditLabel(int characterID, Call_EditLabel& args)
{
	DBerror error;
	if (args.name.length() == 0)
		sDatabase.RunQuery(error, "UPDATE mailLabel SET color = %u WHERE labelID = %u AND ownerId = %u", args.color, args.labelId, characterID);
	else if (args.color == -1)
		sDatabase.RunQuery(error, "UPDATE mailLabel SET name = %s WHERE labelID = %u AND ownerId = %u", args.name.c_str(), args.labelId, characterID);
	else
		sDatabase.RunQuery(error, "UPDATE mailLabel SET name = %s, color = %u WHERE labelID = %u AND ownerId = %u", args.name.c_str(), args.color, args.labelId, characterID);
}