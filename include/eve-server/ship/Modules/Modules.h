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


#ifndef MODULES_H
#define MODULES_H

//generic module base class - possibly should inherit from RefPtr...

class GenericModule
{
public:
	GenericModule() { }
	virtual ~GenericModule() { }

	virtual void Process() = 0;
	virtual void Offline() = 0;
	virtual void Online() = 0;
	virtual void Deactivate() = 0;
	virtual void Load() = 0;
	virtual void Unload() = 0;
	virtual void Repair() = 0;
	virtual void Overload() = 0;
	virtual void DeOverload() = 0;
	virtual void DestroyRig() = 0;

	virtual void SetAttribute(uint32 attrID, EvilNumber val) = 0;
	virtual EvilNumber GetAttribute(uint32 attrID) = 0;

	//access functions
	virtual uint32 itemID() = 0;
	virtual EVEItemFlags flag() = 0;

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

//calculation functions
#pragma region EveEffectsCalculations

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

#pragma endregion


#pragma endregion

#endif /* MODULES_H */