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
	OVERLOAD,   //idk if this is used yet - or what it's called :)
	DEOVERLOAD  //idk if this is used
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
enum EVECalculationType
{
	ADD,
	SUBTRACT,
	DIVIDE,
	MULTIPLY,
	ADD_PERCENT,
	SUBTRACT_PERCENT,
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

static EvilNumber AddAsPercent(EvilNumber &val1, EvilNumber &val2)
{
	EvilNumber val3 = 100;
	return val1 + ( val1 * val2 / val3 );
}

static EvilNumber SubtractPercent(EvilNumber &val1, EvilNumber &val2)
{
	return val1 - ( val1 * val2 );
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
	case ADD :					return Add(attrVal, attrMod);
	case SUBTRACT :				return Subtract(attrVal, attrMod);
	case DIVIDE :				return Divide(attrVal, attrMod);
	case MULTIPLY :				return Multiply(attrVal, attrMod);
	case ADD_PERCENT :			return AddPercent(attrVal, attrMod);
	case ADD_AS_PERCENT :		return AddAsPercent(attrVal, attrMod);
	case SUBTRACT_PERCENT :		return SubtractPercent(attrVal, attrMod);
	case SUBTRACT_AS_PERCENT :  return SubtractAsPercent(attrVal, attrMod);
	}

	sLog.Error("CalculateNewAttributeValue", "Unknown EveCalculationType used");
	assert(false);
    return NULL;
}

#endif
