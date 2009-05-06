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
	Author:		Zhur, Bloody.Rabbit
*/

#include "EvemuPCH.h"

/*
 * CharacterTypeData
 */
CharacterTypeData::CharacterTypeData(
	const char *_bloodlineName,
	EVERace _race,
	const char *_desc,
	const char *_maleDesc,
	const char *_femaleDesc,
	uint32 _shipTypeID,
	uint32 _corporationID,
	uint8 _perception,
	uint8 _willpower,
	uint8 _charisma,
	uint8 _memory,
	uint8 _intelligence,
	const char *_shortDesc,
	const char *_shortMaleDesc,
	const char *_shortFemaleDesc)
: bloodlineName(_bloodlineName),
  race(_race),
  description(_desc),
  maleDescription(_maleDesc),
  femaleDescription(_femaleDesc),
  shipTypeID(_shipTypeID),
  corporationID(_corporationID),
  perception(_perception),
  willpower(_willpower),
  charisma(_charisma),
  memory(_memory),
  intelligence(_intelligence),
  shortDescription(_shortDesc),
  shortMaleDescription(_shortMaleDesc),
  shortFemaleDescription(_shortFemaleDesc)
{
}

/*
 * CharacterType
 */
CharacterType::CharacterType(
	uint32 _id,
	uint8 _bloodlineID,
	// Type stuff:
	const Group &_group,
	const TypeData &_data,
	// CharacterType stuff:
	const Type &_shipType,
	const CharacterTypeData &_charData)
: Type(_id, _group, _data),
  m_bloodlineID(_bloodlineID),
  m_bloodlineName(_charData.bloodlineName),
  m_description(_charData.description),
  m_maleDescription(_charData.maleDescription),
  m_femaleDescription(_charData.femaleDescription),
  m_shipType(_shipType),
  m_corporationID(_charData.corporationID),
  m_perception(_charData.perception),
  m_willpower(_charData.willpower),
  m_charisma(_charData.charisma),
  m_memory(_charData.memory),
  m_intelligence(_charData.intelligence),
  m_shortDescription(_charData.shortDescription),
  m_shortMaleDescription(_charData.shortMaleDescription),
  m_shortFemaleDescription(_charData.shortFemaleDescription)
{
	// check for consistency
	assert(_data.race == _charData.race);
	assert(_charData.shipTypeID == _shipType.id());
}

CharacterType *CharacterType::Load(ItemFactory &factory, uint32 characterTypeID) {
	// static _Load
	CharacterType *ct = CharacterType::_Load(factory, characterTypeID);
	if(ct == NULL)
		return NULL;

	// dynamic _Load
	if(!ct->_Load(factory)) {
		delete ct;
		return NULL;
	}

	// successfully loaded, return
	return ct;
}

CharacterType *CharacterType::_Load(ItemFactory &factory, uint32 typeID
) {
	// pull data
	TypeData data;
	if(!factory.db().GetType(typeID, data))
		return NULL;

	// get group
	const Group *g = factory.GetGroup(data.groupID);
	if(g == NULL)
		return NULL;

	// check we are really loading a character type
	if(g->id() != EVEDB::invGroups::Character) {
		_log(ITEM__ERROR, "Load of character type %lu requested, but it's %s.", typeID, g->name().c_str());
		return NULL;
	}

	return(
		CharacterType::_Load(factory, typeID, *g, data)
	);
}

CharacterType *CharacterType::_Load(ItemFactory &factory, uint32 typeID,
	// Type stuff:
	const Group &group, const TypeData &data
) {
	// query character type data
	uint32 bloodlineID;
	CharacterTypeData charData;
	if(!factory.db().GetCharacterType(typeID, bloodlineID, charData))
		return NULL;

	// load ship type
	const Type *shipType = factory.GetType(charData.shipTypeID);
	if(shipType == NULL)
		return NULL;

	return(
		CharacterType::_Load(factory, typeID, bloodlineID, group, data, *shipType, charData)
	);
}

CharacterType *CharacterType::_Load(ItemFactory &factory, uint32 typeID, uint8 bloodlineID,
	// Type stuff:
	const Group &group, const TypeData &data,
	// CharacterType stuff:
	const Type &shipType, const CharacterTypeData &charData
) {
	// enough data for construction
	return(new CharacterType(typeID, bloodlineID,
		group, data,
		shipType, charData
	));
}

/*
 * CharacterAppearance
 */
CharacterAppearance::CharacterAppearance() {
	//NULL all dynamic fields
#define NULL_FIELD(v) \
	v = NULL;

#define INT_DYN(v) NULL_FIELD(v)
#define REAL_DYN(v) NULL_FIELD(v)
#include "character/CharacterAppearance_fields.h"

#undef NULL_FIELD
}

CharacterAppearance::CharacterAppearance(const CharacterAppearance &from) {
	//just do deep copy
	*this = from;
}

CharacterAppearance::~CharacterAppearance() {
	//delete all dynamic fields
#define CLEAR_FIELD(v) \
	Clear_##v();

#define INT_DYN(v) CLEAR_FIELD(v)
#define REAL_DYN(v) CLEAR_FIELD(v)
#include "character/CharacterAppearance_fields.h"

#undef CLEAR_FIELD
}

void CharacterAppearance::Build(const std::map<std::string, PyRep *> &from) {
	//builds our contents from strdict
	std::map<std::string, PyRep *>::const_iterator itr;

	_log(CLIENT__MESSAGE, "  Appearance Data:");

#define INT(v) \
	itr = from.find(#v); \
	if(itr != from.end()) { \
		if(!itr->second->IsInteger()) \
			_log(CLIENT__ERROR, "Invalid type for " #v ": expected integer, got %s.", itr->second->TypeString()); \
		else { \
			v = ((PyRepInteger *)itr->second)->value; \
			_log(CLIENT__MESSAGE, "     %s: %u", itr->first.c_str(), v); \
		} \
	}
#define INT_DYN(v) \
	itr = from.find(#v); \
	if(itr != from.end()) { \
		if(!itr->second->IsInteger()) \
			_log(CLIENT__ERROR, "Invalid type for " #v ": expected integer, got %s.", itr->second->TypeString()); \
		else { \
			Set_##v(((PyRepInteger *)itr->second)->value); \
			_log(CLIENT__MESSAGE, "     %s: %u", itr->first.c_str(), Get_##v()); \
		} \
	}
#define REAL(v) \
	itr = from.find(#v); \
	if(itr != from.end()) { \
		if(!itr->second->IsReal()) \
			_log(CLIENT__ERROR, "Invalid type for " #v ": expected real, got %s.", itr->second->TypeString()); \
		else { \
			v = ((PyRepReal *)itr->second)->value; \
			_log(CLIENT__MESSAGE, "     %s: %f", itr->first.c_str(), v); \
		} \
	}
#define REAL_DYN(v) \
	itr = from.find(#v); \
	if(itr != from.end()) { \
		if(!itr->second->IsReal()) \
			_log(CLIENT__ERROR, "Invalid type for " #v ": expected real, got %s.", itr->second->TypeString()); \
		else { \
			Set_##v(((PyRepReal *)itr->second)->value); \
			_log(CLIENT__MESSAGE, "     %s: %f", itr->first.c_str(), Get_##v()); \
		} \
	}
#include "character/CharacterAppearance_fields.h"
}

void CharacterAppearance::operator=(const CharacterAppearance &from) {
#define COPY(v) \
	v = from.v;
#define COPY_DYN(v) \
	if(!from.IsNull_##v()) \
		Set_##v(from.Get_##v());

#define INT(v) COPY(v)
#define INT_DYN(v) COPY_DYN(v)
#define REAL(v) COPY(v)
#define REAL_DYN(v) COPY_DYN(v)
#include "character/CharacterAppearance_fields.h"

#undef COPY
#undef COPY_DYN
}


