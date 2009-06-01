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

#ifndef __CHARACTER__H__INCL__
#define __CHARACTER__H__INCL__

#include "inventory/Type.h"
#include "inventory/InventoryItem.h"

/**
 * Simple container for raw character type data.
 */
class CharacterTypeData {
public:
	CharacterTypeData(
		const char *_bloodlineName = "",
		EVERace _race = (EVERace)0,
		const char *_desc = "",
		const char *_maleDesc = "",
		const char *_femaleDesc = "",
		uint32 _shipTypeID = 0,
		uint32 _corporationID = 0,
		uint8 _perception = 0,
		uint8 _willpower = 0,
		uint8 _charisma = 0,
		uint8 _memory = 0,
		uint8 _intelligence = 0,
		const char *_shortDesc = "",
		const char *_shortMaleDesc = "",
		const char *_shortFemaleDesc = ""
	);

	// Content:
	std::string bloodlineName;
	EVERace race;
	std::string description;
	std::string maleDescription;
	std::string femaleDescription;
	uint32 shipTypeID;
	uint32 corporationID;

	uint8 perception;
	uint8 willpower;
	uint8 charisma;
	uint8 memory;
	uint8 intelligence;

	std::string shortDescription;
	std::string shortMaleDescription;
	std::string shortFemaleDescription;
};

/**
 * Class which maintains character type data.
 */
class CharacterType
: public Type
{
	friend class Type; // to let it construct us
public:
	/**
	 * Loads and returns new CharacterType.
	 *
	 * @param[in] factory
	 * @param[in] characterTypeID ID of character type to load.
	 * @return Pointer to new object, NULL if failed.
	 */
	static CharacterType *Load(ItemFactory &factory, uint32 characterTypeID);

	/*
	 * Access functions:
	 */
	uint32 bloodlineID() const { return m_bloodlineID; }

	const std::string &bloodlineName() const { return m_bloodlineName; }
	const std::string &description() const { return m_description; }
	const std::string &maleDescription() const { return m_maleDescription; }
	const std::string &femaleDescription() const { return m_femaleDescription; }
	const Type &shipType() const { return m_shipType; }
	uint32 shipTypeID() const { return shipType().id(); }
	uint32 corporationID() const { return m_corporationID; }

	uint8 perception() const { return m_perception; }
	uint8 willpower() const { return m_willpower; }
	uint8 charisma() const { return m_charisma; }
	uint8 memory() const { return m_memory; }
	uint8 intelligence() const { return m_intelligence; }

	const std::string &shortDescription() const { return m_shortDescription; }
	const std::string &shortMaleDescription() const { return m_shortMaleDescription; }
	const std::string &shortFemaleDescription() const { return m_shortFemaleDescription; }

protected:
	CharacterType(
		uint32 _id,
		uint8 _bloodlineID,
		// Type stuff:
		const Group &_group,
		const TypeData &_data,
		// CharacterType stuff:
		const Type &_shipType,
		const CharacterTypeData &_charData
	);

	/*
	 * Member functions
	 */
	// Template loader:
	template<class _Ty>
	static _Ty *_Load(ItemFactory &factory, uint32 typeID,
		// Type stuff:
		const Group &group, const TypeData &data
	) {
		// check we are really loading a character type
		if(group.id() != EVEDB::invGroups::Character) {
			_log(ITEM__ERROR, "Load of character type %u requested, but it's %s.", typeID, group.name().c_str());
			return NULL;
		}

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
			_Ty::_Load(factory, typeID, bloodlineID, group, data, *shipType, charData)
		);
	}

	// Actual loading stuff:
	static CharacterType *_Load(ItemFactory &factory, uint32 typeID
	);
	static CharacterType *_Load(ItemFactory &factory, uint32 typeID,
		// Type stuff:
		const Group &group, const TypeData &data
	);
	static CharacterType *_Load(ItemFactory &factory, uint32 typeID, uint8 bloodlineID,
		// Type stuff:
		const Group &group, const TypeData &data,
		// CharacterType stuff:
		const Type &shipType, const CharacterTypeData &charData
	);

	virtual bool _Load(ItemFactory &factory) { return Type::_Load(factory); }

	/*
	 * Data members
	 */
	uint8 m_bloodlineID;

	std::string m_bloodlineName;
	std::string m_description;
	std::string m_maleDescription;
	std::string m_femaleDescription;
	const Type &m_shipType;
	uint32 m_corporationID;

	uint8 m_perception;
	uint8 m_willpower;
	uint8 m_charisma;
	uint8 m_memory;
	uint8 m_intelligence;

	std::string m_shortDescription;
	std::string m_shortMaleDescription;
	std::string m_shortFemaleDescription;
};

/**
 * Container for raw character data.
 */
class CharacterData {
public:
	CharacterData(
		uint32 _accountID = 0,
		const char *_title = "",
		const char *_desc = "",
		bool _gender = false,
		double _bounty = 0.0,
		double _balance = 0.0,
		double _securityRating = 0.0,
		uint32 _logonMinutes = 0,
		uint32 _corporationID = 0,
		uint32 _allianceID = 0,
		uint32 _stationID = 0,
		uint32 _solarSystemID = 0,
		uint32 _constellationID = 0,
		uint32 _regionID = 0,
		uint32 _ancestryID = 0,
		uint32 _careerID = 0,
		uint32 _schoolID = 0,
		uint32 _careerSpecialityID = 0,
		uint64 _startDateTime = 0,
		uint64 _createDateTime = 0,
		uint64 _corporationDateTime = 0);

	uint32 accountID;

	std::string title;
	std::string description;
	bool gender;

	double bounty;
	double balance;
	double securityRating;
	uint32 logonMinutes;

	uint32 corporationID;
	uint32 allianceID;

	uint32 stationID;
	uint32 solarSystemID;
	uint32 constellationID;
	uint32 regionID;

	uint32 ancestryID;
	uint32 careerID;
	uint32 schoolID;
	uint32 careerSpecialityID;

	uint64 startDateTime;
	uint64 createDateTime;
	uint64 corporationDateTime;
};

/**
 * Container for character appearance stuff.
 */
class CharacterAppearance {
public:
	CharacterAppearance();
	CharacterAppearance(const CharacterAppearance &from);
	~CharacterAppearance();

#define INT(v) \
	uint32 v;
#define INT_DYN(v) \
	bool IsNull_##v() const { \
		return(v == NULL); \
	} \
	uint32 Get_##v() const { \
		return(IsNull_##v() ? NULL : *v); \
	} \
	void Set_##v(uint32 val) { \
		Clear_##v(); \
		v = new uint32(val); \
	} \
	void Clear_##v() { \
		if(!IsNull_##v()) \
			delete v; \
		v = NULL; \
	}
#define REAL(v) \
	double v;
#define REAL_DYN(v) \
	bool IsNull_##v() const { \
		return(v == NULL); \
	} \
	double Get_##v() const { \
		return(IsNull_##v() ? NULL : *v); \
	} \
	void Set_##v(double val) { \
		Clear_##v(); \
		v = new double(val); \
	} \
	void Clear_##v() { \
		if(!IsNull_##v()) \
			delete v; \
		v = NULL; \
	}
#include "character/CharacterAppearance_fields.h"

	void Build(const std::map<std::string, PyRep *> &from);
	void operator=(const CharacterAppearance &from);

protected:
#define INT_DYN(v) \
	uint32 *v;
#define REAL_DYN(v) \
	double *v;
#include "character/CharacterAppearance_fields.h"
};

/**
 * Container for some corporation-membership related stuff.
 */
class CorpMemberInfo {
public:
	CorpMemberInfo(
		uint32 _corpHQ = 0,
		uint64 _corpRole = 0,
		uint64 _rolesAtAll = 0,
		uint64 _rolesAtBase = 0,
		uint64 _rolesAtHQ = 0,
		uint64 _rolesAtOther = 0
	);

	uint32 corpHQ;	//this really doesn't belong here...

	uint64 corpRole;
	uint64 rolesAtAll;
	uint64 rolesAtBase;
	uint64 rolesAtHQ;
	uint64 rolesAtOther;
};

/**
 * Class representing character.
 */
class Character
: public InventoryItem
{
	friend class InventoryItem;	// to let it construct us
public:
	/**
	 * Loads character.
	 *
	 * @param[in] factory
	 * @param[in] characterID ID of character to load.
	 * @param[in] recurse Whether load should be recursive - load all contained items as well.
	 * @return Pointer to new Character object; NULL if failed.
	 */
	static Character *Load(ItemFactory &factory, uint32 characterID, bool recurse=false);
	/**
	 * Spawns new character.
	 *
	 * @param[in] factory
	 * @param[in] data ItemData (data for entity table) for new character.
	 * @param[in] charData Character data for new character.
	 * @param[in] appData Appearance data for new character.
	 * @param[in] corpData Corporation membership data for new character.
	 * @return Pointer to new Character object; NULL if failed.
	 */
	static Character *Spawn(ItemFactory &factory, ItemData &data, CharacterData &charData, CharacterAppearance &appData, CorpMemberInfo &corpData);

	/*
	 * Primary public interface:
	 */
	Character *IncRef() { return static_cast<Character *>(InventoryItem::IncRef()); }

	void Save(bool recursive=false, bool saveAttributes=true) const;
	void Delete();

	bool AlterBalance(double balanceChange);
	void SetLocation(uint32 stationID, uint32 solarSystemID, uint32 constellationID, uint32 regionID);
	void JoinCorporation(uint32 corporationID);
	void SetDescription(const char *newDescription);

	/* SkillAlreadyInjected(InventoryItem *skill)
	 * 
	 * Perform a check to see if the passed skill is
	 * already injected into the character.
	 * @author xanarox
	 * @param InventoryItem
	*/
	bool SkillAlreadyInjected(InventoryItem *skill);

	/* InjectSkillIntoBrain(InventoryItem *skill)
	 * 
	 * Perform injection of passed skill into the character.
	 * @author xanarox
	 * @param InventoryItem
	*/
	bool InjectSkillIntoBrain(InventoryItem *skill);

	/* RemoveSkillsFromSkillQueue()
	 * 
	 * This will remove all skills from the characters skill queue.
	 * @author xanarox
	*/
	bool RemoveSkillsFromSkillQueue(uint32 itemID, uint32 typeID);

	/* GetSkillsFromSkillQueue()
	 * 
	 * This will get the skills from the skill queue for a character.
	 * @author xanarox
	 * @param InventoryItem
	*/
	PyRepList *GetSkillsFromSkillQueue(uint32 itemID);

	/* AddSkillToSkillQueue()
	 * 
	 * This will add a skill into the skill queue.
	 * @author xanarox
	 * @param InventoryItem
	*/
	bool AddSkillToSkillQueue(uint32 itemID, uint32 typeID, uint8 level);


	void TrainSkill(InventoryItem *skill);
	double GetSPM(Client *c, uint32 PrimeryAttr, uint32 SeconderyAttr);
	uint32 GetSPForLevel(InventoryItem *skill, uint8 Level);
	void StopTrainingSkill();
	bool CharStopTrainingSkill(Client *c, InventoryItem *skill);
	void EndSkillTrain();
	InventoryItem *GetSkillInTraining();
	void CharEndTrainSkill();




	// NOTE: We do not handle Split/Merge logic since singleton-restricted construction does this for us.

	/*
	 * Primary public packet builders:
	 */
	PyRepObject *CharGetInfo();
	PyRepObject *GetDescription() const;

	/*
	 * Public fields:
	 */
	const CharacterType &   type() const { return static_cast<const CharacterType &>(InventoryItem::type()); }
	uint32                  bloodlineID() const { return type().bloodlineID(); }
	EVERace                 race() const { return type().race(); }

	// Account:
	uint32                  accountID() const { return m_accountID; }

	const std::string &     title() const { return m_title; }
	const std::string &     description() const { return m_description; }
	bool                    gender() const { return m_gender; }

	double                  bounty() const { return m_bounty; }
	double                  balance() const { return m_balance; }
	double                  securityRating() const { return m_securityRating; }
	uint32                  logonMinutes() const { return m_logonMinutes; }

	// Corporation:
	uint32                  corporationID() const { return m_corporationID; }
	uint32                  corporationHQ() const { return m_corpHQ; }
	uint32                  allianceID() const { return m_allianceID; }

	// Corporation role:
	uint64                  corpRole() const { return m_corpRole; }
	uint64                  rolesAtAll() const { return m_rolesAtAll; }
	uint64                  rolesAtBase() const { return m_rolesAtBase; }
	uint64                  rolesAtHQ() const { return m_rolesAtHQ; }
	uint64                  rolesAtOther() const { return m_rolesAtOther; }

	// Current location:
	uint32                  stationID() const { return m_stationID; }
	uint32                  solarSystemID() const { return m_solarSystemID; }
	uint32                  constellationID() const { return m_constellationID; }
	uint32                  regionID() const { return m_regionID; }

	// Ancestry, career:
	uint32                  ancestryID() const { return m_ancestryID; }
	uint32                  careerID() const { return m_careerID; }
	uint32                  schoolID() const { return m_schoolID; }
	uint32                  careerSpecialityID() const { return m_careerSpecialityID; }

	// Some importand dates:
	uint64                  startDateTime() const { return m_startDateTime; }
	uint64                  createDateTime() const { return m_createDateTime; }
	uint64                  corporationDateTime() const { return m_corporationDateTime; }

protected:
	Character(
		ItemFactory &_factory,
		uint32 _characterID,
		// InventoryItem stuff:
		const CharacterType &_charType,
		const ItemData &_data,
		// Character stuff:
		const CharacterData &_charData,
		const CorpMemberInfo &_corpData
	);

	/*
	 * Member functions:
	 */
	// Template loader:
	template<class _Ty>
	static _Ty *_Load(ItemFactory &factory, uint32 characterID,
		// InventoryItem stuff:
		const Type &type, const ItemData &data
	) {
		// check it's a character
		if(type.groupID() != EVEDB::invGroups::Character) {
			_log(ITEM__ERROR, "Trying to load %s as Character.", type.group().name().c_str());
			return NULL;
		}
		// cast the type
		const CharacterType &charType = static_cast<const CharacterType &>(type);

		CharacterData charData;
		if(!factory.db().GetCharacter(characterID, charData))
			return NULL;

		CorpMemberInfo corpData;
		if(!factory.db().GetCorpMemberInfo(characterID, corpData))
			return NULL;

		return(
			_Ty::_Load(factory, characterID, charType, data, charData, corpData)
		);
	}

	// Actual loading stuff:
	static Character *_Load(ItemFactory &factory, uint32 characterID
	);
	static Character *_Load(ItemFactory &factory, uint32 characterID,
		// InventoryItem stuff:
		const Type &type, const ItemData &data
	);
	static Character *_Load(ItemFactory &factory, uint32 characterID,
		// InventoryItem stuff:
		const CharacterType &charType, const ItemData &data,
		// Character stuff:
		const CharacterData &charData, const CorpMemberInfo &corpData
	);
	virtual bool _Load(bool recurse=false) { return InventoryItem::_Load(recurse); }

	static uint32 _Spawn(ItemFactory &factory,
		// InventoryItem stuff:
		ItemData &data,
		// Character stuff:
		CharacterData &charData, CharacterAppearance &appData, CorpMemberInfo &corpData
	);

	/*
	 * Data members
	 */
	uint32 m_accountID;

	std::string m_title;
	std::string m_description;
	bool m_gender;

	double m_bounty;
	double m_balance;
	double m_securityRating;
	uint32 m_logonMinutes;

	uint32 m_corporationID;
	uint32 m_corpHQ;
	uint32 m_allianceID;

	uint64 m_corpRole;
	uint64 m_rolesAtAll;
	uint64 m_rolesAtBase;
	uint64 m_rolesAtHQ;
	uint64 m_rolesAtOther;

	uint32 m_stationID;
	uint32 m_solarSystemID;
	uint32 m_constellationID;
	uint32 m_regionID;

	uint32 m_ancestryID;
	uint32 m_careerID;
	uint32 m_schoolID;
	uint32 m_careerSpecialityID;

	uint64 m_startDateTime;
	uint64 m_createDateTime;
	uint64 m_corporationDateTime;
};

#endif /* !__CHARACTER__H__INCL__ */

