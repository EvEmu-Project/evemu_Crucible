/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        Reve
    Providing clothes to the poor
    Updates:    Allan
*/

#include "eve-server.h"

#include "character/PaperDollDB.h"

PyRep* PaperDollDB::GetPaperDollAvatar(uint32 charID) const {

    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
		"SELECT hairDarkness FROM avatars WHERE charID=%u", charID))
    {
        _log(DATABASE__ERROR, "Error in GetMyPaperDollData query: %s", res.error.c_str());
        return nullptr;
    }

	DBResultRow row;
	res.GetRow(row);

	return DBRowToRow(row, "util.Row");
}

PyRep* PaperDollDB::GetPaperDollAvatarColors(uint32 charID) const {

    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
		"SELECT a.colorID, a.colorNameA, a.colorNameBC, "
		"a.weight, a.gloss "
		"FROM avatar_colors AS a "
		"INNER JOIN paperdollColors AS c "
		"ON c.colorID = a.colorID "
		"INNER JOIN paperdollColorNames AS nA "
		"ON nA.colorNameID = a.colorNameA "
		"LEFT JOIN paperdollColorNames AS nBC "
		"ON nBC.colorNameID = a.colorNameBC "
		"WHERE a.charID=%u "
		"AND (a.colorNameBC = 0 OR nBC.colorNameID IS NOT NULL)",
		charID))
    {
        _log(DATABASE__ERROR, "Error in GetMyPaperDollData query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyRep* PaperDollDB::GetPaperDollAvatarModifiers(uint32 charID) const {

    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
		"SELECT m.modifierLocationID, m.paperdollResourceID, "
		"m.paperdollResourceVariation "
		"FROM avatar_modifiers AS m "
		"INNER JOIN paperdollModifierLocations AS l "
		"ON l.modifierLocationID = m.modifierLocationID "
		"INNER JOIN paperdollResources AS r "
		"ON r.paperdollResourceID = m.paperdollResourceID "
		"WHERE m.charID=%u",
		charID))
    {
        _log(DATABASE__ERROR, "Error in GetMyPaperDollData query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyRep* PaperDollDB::GetPaperDollAvatarSculpts(uint32 charID) const {

    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
		"SELECT s.sculptLocationID, s.weightUpDown, "
		"s.weightLeftRight, s.weightForwardBack "
		"FROM avatar_sculpts AS s "
		"INNER JOIN paperdollSculptingLocations AS l "
		"ON l.sculptLocationID = s.sculptLocationID "
		"WHERE s.charID=%u",
		charID))
    {
        _log(DATABASE__ERROR, "Error in GetMyPaperDollData query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyRep* PaperDollDB::GetPaperDollPortraitData(uint32 charID) const
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(
            res,
            "SELECT p.* FROM chrPortraitData AS p "
            "INNER JOIN chrBackgrounds AS b "
            "ON b.backgroundID = p.backgroundID "
            "INNER JOIN chrLights AS l "
            "ON l.lightID = p.lightID "
            "INNER JOIN paperdollColorNames AS c "
            "ON c.colorNameID = p.lightColorID "
            "WHERE p.charID = %u",
            charID)) {
        _log(DATABASE__ERROR, "Error in GetMyPaperDollData query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}
