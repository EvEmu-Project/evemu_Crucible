
#ifndef MODULE_DEFS_H
#define MODULE_DEFS_H

#include "inventory/AttributeEnum.h"

//more will go here
//this is to avoid include complications and multiple dependancies etc..
typedef enum ModuleCommand
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
typedef enum ModulePowerLevel
{
	HIGH_POWER,
	MEDIUM_POWER,
	LOW_POWER,
	RIG,
	SUBSYSTEM
};

//calculation types
typedef enum EVECalculationType
{
	ADD,
	SUBTRACT,
	DIVIDE,
	MULTIPLY,
	ADD_PERCENT,
	ADD_AS_PERCENT,
	SUBTRACT_PERCENT,
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
	EvilNumber *val3 = new EvilNumber(100);
	return val1 + ( val1 * val2 / *val3 );
}

static EvilNumber SubtractPercent(EvilNumber &val1, EvilNumber &val2)
{
	return val1 - ( val1 * val2 );
}

static EvilNumber SubtractAsPercent(EvilNumber &val1, EvilNumber &val2)
{
	EvilNumber *val3 = new EvilNumber(1);
	EvilNumber *val4 = new EvilNumber(100);

	return val1 / ( *val3 + ( val2 / *val4 ));
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

}

#endif