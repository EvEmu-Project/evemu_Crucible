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
    Author:        Luck
*/

#ifndef MODULE_DEFS_H
#define MODULE_DEFS_H

#include "inventory/AttributeEnum.h"

//more will go here
//this is to avoid include complications and multiple dependancies etc..
enum ModuleCommand
{
	CMD_ERROR,
	ONLINE,
	OFFLINE,
	ACTIVATE,
	DEACTIVATE,
	OVERLOAD,       //idk if this is used yet - or what it's called :)
	DEOVERLOAD,     //idk if this is used
    LOAD_CHARGE,
    RELOAD_CHARGE,
    UNLOAD_CHARGE
};

enum ModuleStates
{
    MOD_UNFITTED,
    MOD_OFFLINE,
    MOD_ONLINE,
    MOD_ACTIVATED,
    MOD_DEACTIVATING,
    MOD_OVERLOADED
};

enum ChargeStates
{
    MOD_UNLOADED,
    MOD_LOADED,
    MOD_LOADING,
    MOD_RELOADING
};

// These are the module states where an effect will, ahem, take 'effect':
// *** use these values to decode the 'effectAppliedInState' field of the 'dgmEffectsInfo' database table
enum ModuleEffectTriggers
{
    EFFECT_ONLINE,              // means the effect takes effect on the target (see below) upon entering the ONLINE state
    EFFECT_ACTIVE,              // used only for ACTIVE modules operating in non-Overloaded mode
    EFFECT_OVERLOAD             // used only for ACTIVE modules operating in Overloaded mode
};

// These are the targets to which module effects are applied when activated:
// *** use these values to decode the 'effectAppliedTo' field of the 'dgmEffectsInfo' database table
enum ModuleEffectTargets
{
    SELF,           // means the target of the effect is the module's own attribute(s)
    SHIP,           // means the target of the effect is the attribute(s) of the ship to which the module is fitted
    TARGET          // means the target of the effect is the attribute(s) of the current target of the ship to which the module is fitted
};

// These are the methods by which module effects are applied to the designated target:
// *** use these values to decode the 'effectApplicationType' field of the 'dgmEffectsInfo' database table
enum ModuleApplicationTypes
{
    EFFECT_ONLINE_MAINTAIN,     // applied by PASSIVE or ACTIVE modules where an effect is maintained; means the effect takes effect on the
                                // target (see below) upon entering the ONLINE state, then reversed when going out of ONLINE state
    EFFECT_ACTIVE_MAINTAIN,     // applied by ACTIVE modules where an effect is maintained; means the effect takes effect on the
                                // target (see below) upon entering the ACTIVATE state, then reversed when going out of ACTIVATE state
    EFFECT_ACTIVE_CONSUME       // applied by ACTIVE modules where an effect is applied cumulatively on each cycle; means the effect takes
};                              // effect on the target (see below) one extra time when in ACTIVATE state after each CYCLE duration expires

// These are the methods by which module effects are applied to the designated target:
// *** use these values to decode the 'stackingPenaltyApplied' field of the 'dgmEffectsInfo' database table
enum ModuleStackingPenaltyState
{
    NO_PENALTY,
    PENALTY
};

//this may or may not be redundant...idk
enum ModulePowerLevel
{
	HIGH_POWER,
	MEDIUM_POWER,
	LOW_POWER,
	RIG,
	SUBSYSTEM
};

//calculation types
// *** use these values to decode the 'calculationTypeID' and the 'reverseCalculationTypeID' fields of the 'dgmEffectsInfo' database table
enum EVECalculationType
{
    NONE,
    AUTO,
	ADD,
	SUBTRACT,
	DIVIDE,
	MULTIPLY,
	ADD_PERCENT,
    REV_ADD_PERCENT,
	SUBTRACT_PERCENT,
    REV_SUBTRACT_PERCENT,
	ADD_AS_PERCENT,
	SUBTRACT_AS_PERCENT
	//more will show up, im sure
};


//TODO - check mem usage
static EvilNumber Add(EvilNumber &val1, EvilNumber &val2)
{
	return val1 + val2;
}

static EvilNumber Subtract(EvilNumber &val1, EvilNumber &val2)
{
	return val1 - val2;
}

static EvilNumber Divide(EvilNumber &val1, EvilNumber &val2)
{
	return ( val1 / val2 );
}

static EvilNumber Multiply(EvilNumber &val1, EvilNumber &val2)
{
	return val1 * val2;
}

static EvilNumber AddPercent(EvilNumber &val1, EvilNumber &val2)
{
	return val1 + ( val1 * val2	);
}

static EvilNumber ReverseAddPercent(EvilNumber &val1, EvilNumber &val2)
{
    EvilNumber val3 = 1;
	return val1 / (val3 + val2);
}

static EvilNumber SubtractPercent(EvilNumber &val1, EvilNumber &val2)
{
	return val1 - ( val1 * val2 );
}

static EvilNumber ReverseSubtractPercent(EvilNumber &val1, EvilNumber &val2)
{
    EvilNumber val3 = 1;
	return val1 / ( val3 - val2 );
}

static EvilNumber AddAsPercent(EvilNumber &val1, EvilNumber &val2)
{
	EvilNumber val3 = 100;
	return val1 + ( val1 * (val2 / val3) );
}

static EvilNumber SubtractAsPercent(EvilNumber &val1, EvilNumber &val2)
{
	EvilNumber val3 = 1;
	EvilNumber val4 = 100;

	return val1 / ( val3 + ( val2 / val4 ));
}

static EvilNumber CalculateNewAttributeValue(EvilNumber attrVal, EvilNumber attrMod, EVECalculationType type)
{
	switch(type)
	{
        case NONE :                     return attrVal;
        case AUTO :                     return attrVal;                             // AUTO NOT SUPPORTED AT THIS TIME !!!
	    case ADD :					    return Add(attrVal, attrMod);
	    case SUBTRACT :				    return Subtract(attrVal, attrMod);
	    case DIVIDE :				    return Divide(attrVal, attrMod);
	    case MULTIPLY :				    return Multiply(attrVal, attrMod);
	    case ADD_PERCENT :			    return AddPercent(attrVal, attrMod);
        case REV_ADD_PERCENT :          return ReverseAddPercent(attrVal, attrMod);
	    case SUBTRACT_PERCENT :		    return SubtractPercent(attrVal, attrMod);
        case REV_SUBTRACT_PERCENT :     return ReverseSubtractPercent(attrVal, attrMod);
	    case ADD_AS_PERCENT :		    return AddAsPercent(attrVal, attrMod);
	    case SUBTRACT_AS_PERCENT :      return SubtractAsPercent(attrVal, attrMod);
	}

	sLog.Error("CalculateNewAttributeValue", "Unknown EveCalculationType used");
	assert(false);
    return NULL;
}

#endif
