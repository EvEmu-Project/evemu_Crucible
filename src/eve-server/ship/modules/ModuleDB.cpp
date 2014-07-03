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
    Author:        Aknor Jaden, Luck
*/

#include "eve-server.h"

#include "ship/modules/ModuleDB.h"

void ModuleDB::GetAllTypeIDs(DBQueryResult &res)
{
    if( !sDatabase.RunQuery(res,
        " SELECT "
        " typeID "
        " FROM invTypes "
        " WHERE 1 "))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
    }
}

void ModuleDB::GetAllDgmEffects(DBQueryResult &res)
{
    if( !sDatabase.RunQuery(res,
        " SELECT "
        " effectID "
        " FROM dgmEffects "
        " WHERE 1 "))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
    }
}

void ModuleDB::GetAllDgmEffectsInfo(DBQueryResult &res)
{
    if( !sDatabase.RunQuery(res,
        " SELECT "
        " effectID "
        " FROM dgmEffectsInfo "
        " WHERE 1 "))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
    }
}

void ModuleDB::GetAllDgmTypeEffects(DBQueryResult &res)
{
    if( !sDatabase.RunQuery(res,
        " SELECT "
        " effectID "
        " FROM dgmTypeEffects "
        " WHERE 1 "))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
    }
}

void ModuleDB::GetAllDgmSkillBonusModifiers(DBQueryResult &res)
{
    if( !sDatabase.RunQuery(res,
        " SELECT "
        " skillID "
        " FROM dgmSkillBonusModifiers "
        " WHERE 1 "))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
    }
}

void ModuleDB::GetAllDgmShipBonusModifiers(DBQueryResult &res)
{
    if( !sDatabase.RunQuery(res,
        " SELECT "
        " shipID "
        " FROM dgmShipBonusModifiers "
        " WHERE 1 "))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
    }
}

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
        " sourceAttributeID, "
        " targetAttributeID, "
        " calculationTypeID, "
		" description, "
        " reverseCalculationTypeID, "
        " targetGroupIDs, "
        " stackingPenaltyApplied, "
        " effectAppliedInState, "
        " affectingID, "
        " affectingType, "
        " affectedType "
        " FROM dgmEffectsInfo "
        " WHERE effectID = '%u' ",
        effectID))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
    }
}

void ModuleDB::GetDgmTypeEffects(uint32 typeID, DBQueryResult &res)
{
    if( !sDatabase.RunQuery(res,
        " SELECT "
        " effectID, "
		" isDefault "
        " FROM dgmTypeEffects "
        " WHERE typeID = '%u' ", typeID))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
    }
}

void ModuleDB::GetDgmSkillBonusModifiers(uint32 skillID, DBQueryResult &res)
{
    if( !sDatabase.RunQuery(res,
        " SELECT "
		" effectID, "
        " sourceAttributeID, "
        " targetAttributeID, "
        " calculationTypeID, "
		" description, "
        " reverseCalculationTypeID, "
        " targetGroupIDs, "
        " targetChargeSize, "
        " appliedPerLevel, "
        " affectingType, "
        " affectedType "
        " FROM dgmSkillBonusModifiers "
        " WHERE skillID = '%u' ",
        skillID))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
    }
}

void ModuleDB::GetDgmShipBonusModifiers(uint32 shipID, DBQueryResult &res)
{
    if( !sDatabase.RunQuery(res,
        " SELECT "
		" effectID, "
		" attributeSkillID, "
        " sourceAttributeID, "
        " targetAttributeID, "
        " calculationTypeID, "
		" description, "
        " reverseCalculationTypeID, "
        " targetGroupIDs, "
        " appliedPerLevel, "
        " affectingType, "
        " affectedType "
        " FROM dgmShipBonusModifiers "
        " WHERE shipID = '%u' ",
        shipID))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
    }
}
