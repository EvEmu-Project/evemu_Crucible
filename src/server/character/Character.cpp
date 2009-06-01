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
	uint64 _corpRole,
	uint64 _rolesAtAll,
	uint64 _rolesAtBase,
	uint64 _rolesAtHQ,
	uint64 _rolesAtOther)
: corpHQ(_corpHQ),
  corpRole(_corpRole),
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
  m_corpRole(_corpData.corpRole),
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

bool Character::RemoveSkillsFromSkillQueue(uint32 itemID, uint32 typeID) {
	// remove the character skills from the skill queue.
	return m_factory.db().RemoveSkillsFromSkillQueue(itemID, typeID);
}

PyRepList *Character::GetSkillsFromSkillQueue(uint32 itemID) {
	PyRepList *result = new PyRepList;
	m_factory.db().GetSkillsFromSkillQueue(itemID, result);
	return(result);
}

bool Character::AddSkillToSkillQueue(uint32 itemID, uint32 typeID, uint8 level) {
	// add the skill to the db.
	return m_factory.db().AddSkillToSkillQueue(itemID, typeID, level);
}

void Character::TrainSkill(InventoryItem *skill) { // rename is StartSkillTraining
	if(m_flag != flagPilot) {
		codelog(ITEM__ERROR, "%s (%u): Tried to train skill %u on non-pilot object.", m_itemName.c_str(), m_itemID, skill->itemID());
		return;
	}
	
	if(skill->flag() == flagSkillInTraining) {
		_log(ITEM__TRACE, "%s (%u): Requested to train skill %u item %u but it is already in training. Doing nothing.", m_itemName.c_str(), m_itemID, skill->typeID(), skill->itemID());
		return;
	}

	// TODO: based on config options later, check to see if another character, owned by this characters account,
	// is training a skill.  If so, return. (flagID=61).

	if(!skill->SkillPrereqsComplete(skill)){
		// TODO: need to send back a response to the client.  need packet specs.
		_log(ITEM__TRACE, "%s (%u): Requested to train skill %u item %u but prereq not complete.", m_itemName.c_str(), m_itemID, skill->typeID(), skill->itemID());
		return;
	}


	// checks done, let's do what official does when you train a skill that may
	// or may not be in the skillqueue (does it remove everything but the
	// skill wanting to be trained?)

	Client *c = m_factory.entity_list.FindCharacter(m_itemID);
	if(c == NULL){return;}
 	
	//stop training our old skill...
	InventoryItem *TrainingSkill = GetSkillInTraining();
	if(TrainingSkill != NULL){
		CharStopTrainingSkill(c, TrainingSkill);
 	}

	if(!Contains(skill)) {
 		//this must be a skill in another container...
 		_log(ITEM__ERROR, "%s (%u): Tried to train skill item %u which has the skill flag but is not contained within this item.", m_itemName.c_str(), m_itemID, skill->itemID());
 		return;
 	} else {
 		//skill is within this item, change its flag.
 		_log(ITEM__TRACE, "%s (%u): Starting training of skill %u item %u", m_itemName.c_str(), m_itemID, skill->typeID(), skill->itemID());
 	}
		double SPM = (GetSPM(c, skill->primaryAttribute(),skill->secondaryAttribute()));
		uint64 TimeTraining = Win32TimeNow() + ((GetSPForLevel(skill, skill->skillLevel()+1) - skill->skillPoints())/SPM)* Win32Time_Minute;
	if(c != NULL) {
		OnSkillStartTraining osst;
		osst.itemID = skill->itemID();
		osst.endOfTraining = TimeTraining; //hack hack hack
		PyRepTuple *tmp = osst.FastEncode();	//this is consumed below
 		skill->ChangeFlag(flagSkillInTraining, true);
		skill->Set_expiryTime(TimeTraining);
		c->SetTrainStatus(true, TimeTraining);
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
	FindByFlag(flagSkillInTraining, skills, false);

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

bool Character::SkillAlreadyInjected(InventoryItem *skill) {
	/* Check to see if the passed skill is already injected into
	   the character. */
	std::vector<InventoryItem *> skills;
	FindByFlag(flagSkill, skills, false);
	FindByFlag(flagSkillInTraining, skills, false);

	std::vector<InventoryItem *>::iterator cur, end;
	cur = skills.begin();
	end = skills.end();
	for(; cur != end; cur++) {						// could check by flag here instead of location and owner.
		if((skill->typeID() == (*cur)->typeID()) && (skill->ownerID() == (*cur)->locationID())) {
			skills.clear();
			return true;
		}
	}
	skills.clear();
	return false;
}

bool Character::InjectSkillIntoBrain(InventoryItem *skill) {
	if(skill->flag() != flagSkill) {
		
		// are we injecting from a stack of skills?
		if(skill->quantity() > 1) {
			// drop the quantity of the stack and create a new skill to inject.
			uint32 qty;
			qty = skill->quantity();
			skill->SetQuantity( --qty, true);

			ItemData idata(
				skill->typeID(),
				m_itemID,
				0, // do not assign to character yet.
				flagSkill,
				1
			);
			InventoryItem *i = m_factory.SpawnItem(idata);
			if(i == NULL) {
				codelog(ITEM__ERROR, "%s (%u): Unable to spawn new item from stack to inject into character.", m_itemName.c_str(), skill->itemID());
				return false;
			}
			i->ChangeSingleton(true, false);
 			i->Set_skillLevel(0);
 			i->Set_skillPoints(0);
			i->MoveInto(this, flagSkill);
			i->ChangeFlag(flagSkill, true);
			i->DecRef(); // no longer needed.
			return true;
		}else{
			// inject this skill into the character
			skill->ChangeSingleton(true,false);
 			skill->Set_skillLevel(0);
 			skill->Set_skillPoints(0);
			skill->MoveInto(this, flagSkill);
			skill->ChangeFlag(flagSkill, true);	
			return true;
		}
	}
	return false;
 }

void Character::StopTrainingSkill() {
	if(m_flag != flagPilot) {return;}
	Client *c = m_factory.entity_list.FindCharacter(m_itemID);
	std::vector<InventoryItem *> skills;
	FindByFlag(flagSkillInTraining, skills, false);
	std::vector<InventoryItem *>::iterator cur, end;
	cur = skills.begin();
	end = skills.end();
	for(; cur != end; cur++) {
		InventoryItem *skill = *cur;
		CharStopTrainingSkill(c, skill);
	}}

bool Character::CharStopTrainingSkill(Client *c, InventoryItem *skill) { 
	double currentPoints;
	uint64 TimeEndTrain = skill->expiryTime();
	if(TimeEndTrain != 0){
		double secs = TimeEndTrain - Win32TimeNow(); 
		double SPM = (GetSPM(c, skill->primaryAttribute(), skill->secondaryAttribute()));//*(1+(1E-24*skill->skillLevel()));
		currentPoints =  GetSPForLevel(skill, skill->skillLevel()+1) - ((secs/Win32Time_Minute)*SPM);
	}else{
		currentPoints = skill->skillPoints();
	}

	skill->Set_skillPoints(currentPoints);
	skill->ChangeFlag(flagSkill, true);
	if(c != NULL) {
		OnSkillTrainingStopped osst;
		osst.itemID = skill->itemID();
		osst.endOfTraining = 0;
		PyRepTuple *tmp = osst.FastEncode();	//this is consumed below
		c->SendNotification("OnSkillTrainingStopped", "charid", &tmp);
		c->SetTrainStatus(false, 0);
		return true;
 	}
	return false;
}

double Character::GetSPM(Client *c, uint32 PrimeryAttr, uint32 SeconderyAttr){
	double LearningLv = 0;
	//3374 - Skill Learning
	InventoryItem *skill2 = m_factory.GetInvforType(3374, flagSkill, c->Char()->itemID(), false);
	if(skill2 != NULL){
		LearningLv = skill2->skillLevel();
	}
	double Primery = (c->GetCharAttrib(PrimeryAttr)/**(1+(0.02 * LearningLv))*/)*2;
	double Secondery = (c->GetCharAttrib(SeconderyAttr)/**(1+(0.02 * LearningLv))*/)*2;
	return(Primery+(Secondery/2));
}

uint32 Character::GetSPForLevel(InventoryItem *skill, uint8 Level){
	return(250 * skill->skillTimeConstant() * pow(32, (Level-1) / 2.0));
}

InventoryItem *Character::GetSkillInTraining(){
	// Find a better way to get this, since only 1 skill can be trained.  seems wastefull
	// but keeping FindByFlag generic so it can find multiples is needed.

	std::vector<InventoryItem *> skills;
	FindByFlag(flagSkillInTraining, skills, false);
	std::vector<InventoryItem *>::iterator cur, end;
	cur = skills.begin();
	end = skills.end();
	for(; cur != end; cur++) {
		InventoryItem *skill = *cur;
		return(skill);
	}
	return NULL;
}

void Character::CharEndTrainSkill(){
	Client *c = m_factory.entity_list.FindCharacter(m_itemID);
	InventoryItem *skill = GetSkillInTraining();
	skill->Set_skillPoints(GetSPForLevel(skill, skill->skillLevel()+1));
	skill->Set_skillLevel(skill->skillLevel()+1);
	skill->ChangeFlag(flagSkill, true);
		if(c != NULL) {
			OnSkillTrainingStopped osst;
			osst.itemID = skill->itemID();
			osst.endOfTraining = 0; //hack hack hack
			PyRepTuple *tmp = osst.FastEncode();	//this is consumed below
			c->SendNotification("OnSkillTrainingStopped", "charid", &tmp);
			c->SendNotifyMsg("Training of the skill has been completed.", skill->itemName());
			c->SetTrainStatus(false, 0);

				}
}
