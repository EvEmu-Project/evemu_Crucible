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
	Author:		Aknor Jaden
*/


#include "EVEServerPCH.h"


APIAccountDB::APIAccountDB()
{
}

bool APIAccountDB::GetCharactersList(uint32 accountID, std::map<std::string, std::map<std::string, std::string> > * characterList)
{
	DBQueryResult res;

	// Find accountID in 'account' table using accountName:
	if( !sDatabase.RunQuery(res,
        " SELECT "
        "   character_.characterID, "
        "   character_.corporationID, "
        "   corporation.corporationName, "
        "   entity.itemName AS name "
        " FROM `character_` "
        "   LEFT JOIN corporation ON corporation.corporationID = character_.corporationID "
        "   LEFT JOIN entity ON entity.itemID = character_.characterID "
        " WHERE `accountID` = %u ", accountID ))
	{
		sLog.Error( "APIAccountDB::GetCharactersList()", "Cannot find accountID %u", accountID );
		return false;
	}

	DBResultRow row;
    std::map<std::string, std::string> charInfo;
    while( res.GetRow( row ) )
    {
        charInfo.clear();
        charInfo.insert( "corporationID", row.GetText(1) );
        charInfo.insert( "corporationName", row.GetText(2) );
        charInfo.insert( "name", row.GetText(3) );
        characterList->insert( std::make_pair<std::string, std::map<std::string, std::string> >( row.GetText(0), charInfo ));
    }

	return true;
}
