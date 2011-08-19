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
	Author:		Zhur
*/

#ifndef __CHARACTERDB_H_INCL__
#define __CHARACTERDB_H_INCL__

#include "ServiceDB.h"

class PyObject;
class PyString;
class CharacterAppearance;
class ItemFactory;
class InventoryItem;
class Client;

class CharacterDB : public ServiceDB
{
public:
	CharacterDB();

	PyRep *GetCharacterList(uint32 accountID);
	PyRep *GetCharSelectInfo(uint32 characterID);
	PyObject *GetCharPublicInfo(uint32 characterID);
	PyObject *GetCharPublicInfo3(uint32 characterID);
	//PyObject *GetAgentPublicInfo(uint32 agentID);
	PyObject *GetOwnerNoteLabels(uint32 charID);
	PyObject *GetOwnerNote(uint32 charID, uint32 noteID);

	bool GetCharClones(uint32 characterID, std::vector<uint32> &into); 
	bool GetActiveClone(uint32 characterID, uint32 &itemID); 
	bool GetActiveCloneType(uint32 characterID, uint32 &typeID); 
	void GetCharacterData(uint32 characterID, std::map<std::string, uint32> &characterDataMap);

	bool ValidateCharName(const char *name);
	/**
	 * add_name_validation_set
	 *
	 * This method will add a character name and ID to the name validation set
	 * for use in checking character names at creation and login.
	 * 
	 * @param[in] name
	 * @param[in] characterID
	 * @return true if adding is successful and false if it was not.
	 * @author Captnoord, Firefoxpdm
	 */
	bool add_name_validation_set(const char* name, uint32 characterID);
	/**
	 * del_name_validation_set
	 *
	 * This method will remove a entry from the name validation set based
	 * on the passed characterID
	 *
	 * @param[in] characterID
	 * @return true if the deletion was successful and false if a error occurred.
	 * @author Captnoord, Firefoxpdm
	 */
	bool del_name_validation_set(uint32 characterID);

	bool GetCharItems(uint32 characterID, std::vector<uint32> &into);

	bool GetLocationByStation(uint32 staID, CharacterData &cdata);

	bool GetCareerBySchool(uint32 schoolID, uint32 &careerID);

	bool GetLocationCorporationByCareer(CharacterData &cdata);

	bool DoesCorporationExist(uint32 corpID);

	/**
	 * Obtains attribute bonuses for given ancestry.
	 *
	 * @param[in] ancestryID ID of ancestry.
	 * @param[out] intelligence Bonus to intelligence.
	 * @param[out] charisma Bonus to charisma.
	 * @param[out] perception Bonus to perception.
	 * @param[out] memory Bonus to memory.
	 * @param[out] willpower Bonus to willpower.
	 * @return True if operation succeeded, false if failed.
	 */
	bool GetAttributesFromAncestry(uint32 ancestryID, uint8 &intelligence, uint8 &charisma, uint8 &perception, uint8 &memory, uint8 &willpower);

	bool GetSkillsByRace(uint32 raceID, std::map<uint32, uint32> &into);
	bool GetSkillsByCareer(uint32 careerID, std::map<uint32, uint32> &into);
	bool GetSkillsByCareerSpeciality(uint32 careerSpecialityID, std::map<uint32, uint32> &into);

	/**
	 * Retrieves the character note from the database as a PyString pointer.
	 *
	 * @author LSMoura
	 */
	PyString *GetNote(uint32 ownerID, uint32 itemID);

	/**
	 * Stores the character note on the database, given the ownerID and itemID and the string itself.
	 *
	 * If the String is null or size zero, the entry is removed from the database.
	 *
	 * @return boolean true if success.
	 *
	 * @author LSMoura
	 */
	bool SetNote(uint32 ownerID, uint32 itemID, const char *str);
	
	uint32 AddOwnerNote(uint32 charID, const std::string &label, const std::string &content);
	bool EditOwnerNote(uint32 charID, uint32 noteID, const std::string &label, const std::string &content);

	uint64 PrepareCharacterForDelete(uint32 accountID, uint32 charID);
	void CancelCharacterDeletePrepare(uint32 accountID, uint32 charID);
	PyRep* DeleteCharacter(uint32 accountID, uint32 charID);

	bool ReportRespec(uint32 characterId); 
	bool GetRespecInfo(uint32 characterId, uint32& out_freeRespecs, uint64& out_nextRespec);

private:
	/**
	 * djb2 algorithm taken from http://www.cse.yorku.ca/~oz/hash.html slightly modified
	 *
	 * @param[in] str string that needs to be hashed.
	 * @return djb2 hash of the string.
	 */
	uint32 djb2_hash(const char* str);

	/**
	 * load_name_validation_set
	 * This method will load up all character names into a set for validating
	 * character names.
	 *
	 * @author Captnoord, Firefoxpdm
	 */
	void load_name_validation_set();

	/* set only for validation */
	typedef std::set<uint32>			CharValidationSet;
	typedef CharValidationSet::iterator	CharValidationSetItr;
	CharValidationSet mNameValidation;

	/* helper object for deleting ( waisting mem here ) */
	typedef std::map<uint32, std::string>	CharIdNameMap;
	typedef CharIdNameMap::iterator			CharIdNameMapItr;
	CharIdNameMap mIdNameContainer;
};

#endif
