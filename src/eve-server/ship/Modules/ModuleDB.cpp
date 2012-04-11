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
	Author:		Luck
*/

#include "EVEServerPCH.h"
#include "ServiceDB.h"


void ModuleDB::GetDgmEffects(uint32 effectID, DBQueryResult &res) 
{
	if( !sDatabase.RunQuery(res,
		" SELECT "
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
		" disallowAutoRepeat, "
		" published, "
		" displayName, " 
		" isWarpSafe, "
		" rangeChance, "
		" electronicChance, "
		" propulsionChance, "
		" distribution, "
		" sfxName, "
		" npcUsageChanceAttributeID, "
		" npcActivationChanceAttributeID, "
		" fittingUsageChanceAttributeID "
		" FROM dgmEffects "
		" WHERE effectID = '%u' ",
		effectID))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
	}
}

void ModuleDB::GetDgmEffectsInfo(uint32 effectID, DBQueryResult &res)
{
	if( !sDatabase.RunQuery(res,
		" SELECT "
		" targetAttributeID, "
		" sourceAttributeID, "
		" calculationTypeID, "
		" reverseCalculationTypeID "
		" FROM dgmEffectsInfo "
		" WHERE effectID = '%u' ",
		effectID))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
	}
}

void ModuleDB::GetDgmEffectsActions(uint32 effectID, DBQueryResult &res)
{
	if( !sDatabase.RunQuery(res,
		" SELECT "
		" effectAppliedInState, "
		" effectAppliedTo, "
		" effectApplicationType, "
		" stackingPenaltyApplied, "
        " nullifyOnlineEffect, "
        " nullifiedOnlineEffectID "
		" FROM dgmEffectsActions "
		" WHERE effectID = '%u' ",
		effectID))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
	}
}

void ModuleDB::GetDgmTypeEffectsInformation(uint32 typeID, DBQueryResult &res)
{
	if( !sDatabase.RunQuery(res,
		" SELECT "
		" effectID, "
		" isDefault "
		" FROM dgmTypeEffects "
		" WHERE typeID = '%u' ",
		typeID))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
	}

}
