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
#include "String.h"

PyTuple* ShipDB::GetFormations()
{
	//vicious crap... but this is gunna be a bit of work to load from the DB (nested tuples)
	PyTuple* res = new PyTuple( 2 );

	Beyonce_Formation f;

	//Diamond formation
	f.name = "Diamond";

    f.pos1.x = 100;
    f.pos1.y = 0;
    f.pos1.z = 0;

    f.pos2.x = 0;
    f.pos2.y = 100;
    f.pos2.z = 0;

    f.pos3.x = -100;
    f.pos3.y = 0;
    f.pos3.z = 0;

    f.pos4.x = 0;
    f.pos4.y = -100;
    f.pos4.z = 0;

    res->SetItem( 0, f.Encode() );

	//Arrow formation
	f.name = "Arrow";

    f.pos1.x = 100;
    f.pos1.y = 0;
    f.pos1.z = -50;

    f.pos2.x = 50;
    f.pos2.y = 0;
    f.pos2.z = 0;

    f.pos3.x = -100;
    f.pos3.y = 0;
    f.pos3.z = -50;

    f.pos4.x = -50;
    f.pos4.y = 0;
    f.pos4.z = 0;

    res->SetItem( 1, f.Encode() );

	return res;
}

int ShipDB::GetEffectID(int typeID) {

	DBQueryResult res;

	if( !sDatabase.RunQuery(res,
		" SELECT "
		" effectID "
		" FROM dgmtypeeffects "
		" WHERE typeID = '%u' AND isDefault = 1 ",
		typeID) )
	{
		_log( DATABASE__ERROR, "Error in effect query for typeID = %u", typeID );
	}

	DBResultRow row;

	if( !res.GetRow(row) ) {
		_log( DATABASE__ERROR, "Error in effect get row for typeID = %u", typeID );
	}



	return row.GetUInt(0);
}

std::string ShipDB::GetEffectName(int effectID) {

	DBQueryResult res;

	if( !sDatabase.RunQuery(res,
		" SELECT "
		" effectName "
		" FROM dgmeffects "
		" WHERE effectID = '%u' ",
		effectID) )
	{
		_log( DATABASE__ERROR, "Error in effect query for effectID = %u", effectID );
	}

	DBResultRow row;

	if( !res.GetRow(row) ) {
		_log( DATABASE__ERROR, "Error in effect get row for effectID = %u", effectID );
	}
	
	if( row.ColumnCount() == 0 )
		return "none";
	
	return row.GetText(0);

}

std::string ShipDB::GetSFXEffectName(int effectID) {

	DBQueryResult res;

	if( !sDatabase.RunQuery(res,
		" SELECT "
		" guid "
		" FROM dgmeffects "
		" WHERE effectID = '%u' ",
		effectID) )
	{
		_log( DATABASE__ERROR, "Error in effect query for guid where effectID = %u", effectID );
	}

	DBResultRow row;

	if( !res.GetRow(row) ) {
		_log( DATABASE__ERROR, "Error in effect get row for guid where effectID = %u", effectID );
	}
	

	return row.GetText(0);

}

