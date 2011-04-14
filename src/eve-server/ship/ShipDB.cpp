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

PyObject* ShipDB::GetEffectInformation(std::string effectName) {

	DBQueryResult res;

	if(sDatabase.RunQuery(res,
		" SELECT "
		" effectID, "
		" effectName, " 
		" effectCategory, "
		" preExpression, "
		" postExpression, "
		" description, "
		" guid, "
		" graphicID, " 
		" isOffensive, " 
		" isAssistance, "
		" durationAttributeID, "
		" trackingSpeedAttributeID, "
		" dischargeAttributeID, "
		" rangeAttributeID, "
		" falloffAttributeID, " 
		" disallowAutoRepeate, "
		" published, "
		" displayName, " 
		" isWarpStable, "
		" rangeChance, "
		" electronicChance, "
		" propulsionChance, "
		" distribution, "
		" sfxName, "
		" npcUsageChanceAttributeID, "
		" npcActivationChanceAttributeID, "
		" fittingUsageChanceAttributeID "
		" FROM dgmeffects "
		" WHERE effectName = '%s' ",
		effectName.c_str()))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}
