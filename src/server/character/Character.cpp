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
	return Type::Load<CharacterType>(factory, characterTypeID);
}

CharacterType *CharacterType::_Load(ItemFactory &factory, uint32 typeID
) {
	return Type::_Load<CharacterType>(
		factory, typeID
	);
}

CharacterType *CharacterType::_Load(ItemFactory &factory, uint32 typeID,
	// Type stuff:
	const Group &group, const TypeData &data
) {
	return CharacterType::_Load<CharacterType>(
		factory, typeID, group, data
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
 * CharacterData
 */
CharacterData::CharacterData(
	uint32 _accountID,
	const char *_title,
	const char *_desc,
	bool _gender,
	double _bounty,
	double _balance,
	double _securityRating,
	uint32 _logonMinutes,
	uint32 _corporationID,
	uint32 _allianceID,
	uint32 _stationID,
	uint32 _solarSystemID,
	uint32 _constellationID,
	uint32 _regionID,
	uint32 _ancestryID,
	uint32 _careerID,
	uint32 _schoolID,
	uint32 _careerSpecialityID,
	uint64 _startDateTime,
	uint64 _createDateTime,
	uint64 _corporationDateTime)
: accountID(_accountID),
  title(_title),
  description(_desc),
  gender(_gender),
  bounty(_bounty),
  balance(_balance),
  securityRating(_securityRating),
  logonMinutes(_logonMinutes),
  corporationID(_corporationID),
  allianceID(_allianceID),
  stationID(_stationID),
  solarSystemID(_solarSystemID),
  constellationID(_constellationID),
  regionID(_regionID),
  ancestryID(_ancestryID),
  careerID(_careerID),
  schoolID(_schoolID),
  careerSpecialityID(_careerSpecialityID),
  startDateTime(_startDateTime),
  createDateTime(_createDateTime),
  corporationDateTime(_corporationDateTime)
{
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

/*
 * CorpMemberInfo
 */
CorpMemberInfo::CorpMemberInfo(
	uint32 _corpHQ,
	uint64 _corprole,
	uint64 _rolesAtAll,
	uint64 _rolesAtBase,
	uint64 _rolesAtHQ,
	uint64 _rolesAtOther)
: corpHQ(_corpHQ),
  corprole(_corprole),
  rolesAtAll(_rolesAtAll),
  rolesAtBase(_rolesAtBase),
  rolesAtHQ(_rolesAtHQ),
  rolesAtOther(_rolesAtOther)
{
}

/*
 * Character
 */
Character::Character(
	ItemFactory &_factory,
	uint32 _characterID,
	// InventoryItem stuff:
	const CharacterType &_charType,
	const ItemData &_data,
	// Character stuff:
	const CharacterData &_charData,
	const CorpMemberInfo &_corpData)
: InventoryItem(_factory, _characterID, _charType, _data),
  m_accountID(_charData.accountID),
  m_title(_charData.title),
  m_description(_charData.description),
  m_gender(_charData.gender),
  m_bounty(_charData.bounty),
  m_balance(_charData.balance),
  m_securityRating(_charData.securityRating),
  m_logonMinutes(_charData.logonMinutes),
  m_corporationID(_charData.corporationID),
  m_corpHQ(_corpData.corpHQ),
  m_allianceID(_charData.allianceID),
  m_corpRole(_corpData.corprole),
  m_rolesAtAll(_corpData.rolesAtAll),
  m_rolesAtBase(_corpData.rolesAtBase),
  m_rolesAtHQ(_corpData.rolesAtHQ),
  m_rolesAtOther(_corpData.rolesAtOther),
  m_stationID(_charData.stationID),
  m_solarSystemID(_charData.solarSystemID),
  m_constellationID(_charData.constellationID),
  m_regionID(_charData.regionID),
  m_ancestryID(_charData.ancestryID),
  m_careerID(_charData.careerID),
  m_schoolID(_charData.schoolID),
  m_careerSpecialityID(_charData.careerSpecialityID),
  m_startDateTime(_charData.startDateTime),
  m_createDateTime(_charData.createDateTime),
  m_corporationDateTime(_charData.corporationDateTime)
{
	// allow characters to be only singletons
	assert(singleton() && quantity() == 1);
}

Character *Character::Load(ItemFactory &factory, uint32 characterID, bool recurse) {
	return InventoryItem::Load<Character>(factory, characterID, recurse);
}

Character *Character::_Load(ItemFactory &factory, uint32 characterID
) {
	return InventoryItem::_Load<Character>(
		factory, characterID
	);
}

Character *Character::_Load(ItemFactory &factory, uint32 characterID,
	// InventoryItem stuff:
	const Type &type, const ItemData &data
) {
	return Character::_Load<Character>(
		factory, characterID, type, data
	);
}

Character *Character::_Load(ItemFactory &factory, uint32 characterID,
	// InventoryItem stuff:
	const CharacterType &charType, const ItemData &data,
	// Character stuff:
	const CharacterData &charData, const CorpMemberInfo &corpData
) {
	// construct the item
	return(new Character(
		factory, characterID,
		charType, data,
		charData, corpData
	));
}

Character *Character::Spawn(ItemFactory &factory,
	// InventoryItem stuff:
	ItemData &data,
	// Character stuff:
	CharacterData &charData, CharacterAppearance &appData, CorpMemberInfo &corpData
) {
	uint32 characterID = Character::_Spawn(factory, data, charData, appData, corpData);
	if(characterID == 0)
		return NULL;
	// item cannot contain anything yet - don't recurse
	return Character::Load(factory, characterID, false);
}

uint32 Character::_Spawn(ItemFactory &factory,
	// InventoryItem stuff:
	ItemData &data,
	// Character stuff:
	CharacterData &charData, CharacterAppearance &appData, CorpMemberInfo &corpData
) {
	// make sure it's a character
	const CharacterType *ct = factory.GetCharacterType(data.typeID);
	if(ct == NULL)
		return 0;

	// make sure it's a singleton with qty 1
	if(!data.singleton || data.quantity != 1) {
		_log(ITEM__ERROR, "Tried to create non-singleton character %s.", data.name.c_str());
		return 0;
	}

	// first the item
	uint32 characterID = InventoryItem::_Spawn(factory, data);
	if(characterID == 0)
		return 0;

	// then character
	if(!factory.db().NewCharacter(characterID, charData, appData, corpData)) {
		// delete the item
		factory.db().DeleteItem(characterID);

		return 0;
	}

	return characterID;
}

void Character::Save(bool recursive, bool saveAttributes) const {
	// character data
	m_factory.db().SaveCharacter(
		itemID(),
		CharacterData(
			accountID(),
			title().c_str(),
			description().c_str(),
			gender(),
			bounty(),
			balance(),
			securityRating(),
			logonMinutes(),
			corporationID(),
			allianceID(),
			stationID(),
			solarSystemID(),
			constellationID(),
			regionID(),
			ancestryID(),
			careerID(),
			schoolID(),
			careerSpecialityID(),
			startDateTime(),
			createDateTime(),
			corporationDateTime()
		)
	);

	// corporation data
	m_factory.db().SaveCorpMemberInfo(
		itemID(),
		CorpMemberInfo(
			corporationHQ(),
			corpRole(),
			rolesAtAll(),
			rolesAtBase(),
			rolesAtHQ(),
			rolesAtOther()
		)
	);

	// our parent cares about the rest
	return InventoryItem::Save(recursive, saveAttributes);
}

void Character::Delete() {
	// delete character record
	m_factory.db().DeleteCharacter(itemID());

	// let the parent care about the rest
	InventoryItem::Delete();
}

bool Character::AlterBalance(double balanceChange) {
	if(balanceChange == 0)
		return true;

	double result = balance() + balanceChange;

	//remember, this can take a negative amount...
	if(result < 0)
		return false;

	m_balance = result;

	//TODO: save some info to journal.
	Save(false, false);

	return true;
}

void Character::SetLocation(uint32 stationID, uint32 solarSystemID, uint32 constellationID, uint32 regionID) {
	m_stationID = stationID;
	m_solarSystemID = solarSystemID;
	m_constellationID = constellationID;
	m_regionID = regionID;

	Save(false, false);
}

void Character::JoinCorporation(uint32 corporationID) {
	m_corporationID = corporationID;

	//TODO: load new roles
	m_corpRole = 0;
	m_rolesAtAll = 0;
	m_rolesAtBase = 0;
	m_rolesAtHQ = 0;
	m_rolesAtOther = 0;

	//TODO: recursively change corp on all our items.

	Save(false, false);
}

void Character::SetDescription(const char *newDescription) {
	m_description = newDescription;

	Save(false, false);
}

void Character::TrainSkill(InventoryItem *skill) {
	if(m_flag != flagPilot) {
		codelog(ITEM__ERROR, "%s (%u): Tried to train skill %u on non-pilot object.", m_itemName.c_str(), m_itemID, skill->itemID());
		return;
	}
	
	if(skill->flag() == flagSkillInTraining) {
		_log(ITEM__TRACE, "%s (%u): Requested to train skill %u item %u but it is already in training. Doing nothing.", m_itemName.c_str(), m_itemID, skill->typeID(), skill->itemID());
		return;
	}
	
	Client *c = m_factory.entity_list.FindCharacter(m_ownerID);
	
	//stop training our old skill...
	//search for all, just in case we screwed up
	std::vector<InventoryItem *> skills;
	//find all the skills contained within ourself.
	FindByFlag(flagSkillInTraining, skills, false);
	//encode an entry for each one.
	std::vector<InventoryItem *>::iterator cur, end;
	cur = skills.begin();
	end = skills.end();
	for(; cur != end; cur++) {
		InventoryItem *i = *cur;
		i->ChangeFlag(flagSkill);
		
		if(c != NULL) {
			OnSkillTrainingStopped osst;
			osst.itemID = skill->itemID();
			osst.endOfTraining = 0;
			PyRepTuple *tmp = osst.FastEncode();	//this is consumed below
			c->SendNotification("OnSkillTrainingStopped", "charid", &tmp);
		}
	}

	if(skill->flag() != flagSkill) {
		//this is a skill book being trained for the first time.
		_log(ITEM__TRACE, "%s (%u): Initial training of skill %u item %u", m_itemName.c_str(), m_itemID, skill->typeID(), skill->itemID());
		skill->MoveInto(this, flagSkillInTraining);
		//set the initial attributes.
		skill->Set_skillLevel(0);
		skill->Set_skillPoints(0);
	} else if(!Contains(skill)) {
		//this must be a skill in another container...
		_log(ITEM__ERROR, "%s (%u): Tried to train skill item %u which has the skill flag but is not contained within this item.", m_itemName.c_str(), m_itemID, skill->itemID());
		return;
	} else {
		//skill is within this item, change its flag.
		_log(ITEM__TRACE, "%s (%u): Starting training of skill %u item %u", m_itemName.c_str(), m_itemID, skill->typeID(), skill->itemID());
		skill->ChangeFlag(flagSkillInTraining, true);
	}
	
	if(c != NULL) {
		OnSkillStartTraining osst;
		osst.itemID = skill->itemID();
		osst.endOfTraining = Win32TimeNow() + Win32Time_Month; //hack hack hack
		
		PyRepTuple *tmp = osst.FastEncode();	//this is consumed below
		c->SendNotification("OnSkillStartTraining", "charid", &tmp);
	}
}

PyRepObject *Character::CharGetInfo() {
	//TODO: verify that we are a char?
	
	if(!LoadContents(true)) {
		codelog(ITEM__ERROR, "%s (%u): Failed to load contents for CharGetInfo", m_itemName.c_str(), m_itemID);
		return NULL;
	}

	Rsp_CommonGetInfo result;
	Rsp_CommonGetInfo_Entry entry;

	//first encode self.
	if(!Populate(entry))
		return NULL;	//print already done.
	
	result.items[m_itemID] = entry.FastEncode();

	//now encode skills...
	std::vector<InventoryItem *> skills;
	//find all the skills contained within ourself.
	FindByFlag(flagSkill, skills, false);
	//encode an entry for each one.
	std::vector<InventoryItem *>::iterator cur, end;
	cur = skills.begin();
	end = skills.end();
	for(; cur != end; cur++) {
		if(!(*cur)->Populate(entry)) {
			codelog(ITEM__ERROR, "%s (%u): Failed to load skill item %u for CharGetInfo", m_itemName.c_str(), itemID(), (*cur)->itemID());
		} else {
			result.items[(*cur)->itemID()] = entry.FastEncode();
		}
	}
	
	return(result.FastEncode());
}

PyRepObject *Character::GetDescription() const {
	util_Row row;

	row.header.push_back("description");
	row.line.add(new PyRepString(description()));

	return row.FastEncode();
}


