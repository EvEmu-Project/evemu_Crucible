/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef __CHARACTERDB_H_INCL__
#define __CHARACTERDB_H_INCL__

#include <set>

#include "../ServiceDB.h"

class PyRepObject;
class PyRepString;
class CharacterAppearance;
class ItemFactory;
class InventoryItem;
class Client;

class CharacterDB : public ServiceDB {
public:
	CharacterDB(DBcore *db);
	virtual ~CharacterDB();

	bool GetCharItems(uint32 characterID, std::set<uint32> &into);
	bool DeleteCharacter(uint32 characterID);

	PyRepObject *GetCharacterList(uint32 accountID);
	PyRepObject *GetCharSelectInfo(uint32 characterID);
	PyRepObject *GetCharPublicInfo(uint32 characterID);
	PyRepObject *GetCharPublicInfo3(uint32 characterID);
	//PyRepObject *GetAgentPublicInfo(uint32 agentID);
	PyRepObject *GetOwnerNoteLabels(uint32 charID);
	PyRepObject *GetOwnerNote(uint32 charID, uint32 noteID);
	bool ValidateCharName(const char *name);
	InventoryItem *CreateCharacter(uint32 acct, ItemFactory *fact, const CharacterData &data, CharacterAppearance &app);
	bool LoadCharacterAppearance(uint32 characterID, CharacterAppearance &into);
	
	PyRepObject *GetCharacterAppearance(uint32 characterID);
	
	PyRepObject *GetCharDesc(uint32 characterID);//mandela
	bool SetCharDesc(uint32 characterID, const char *str);//mandela
	
	uint8 GetRaceByBloodline(uint32 bloodlineID);

	bool GetLocationCorporationByCareer(CharacterData & cdata, double & x, double & y, double & z);
	bool GetShipTypeByBloodline(uint32 bloodlineID, uint32 &shipTypeID);

	bool GetAttributesFromBloodline(CharacterData & cdata);
	bool GetAttributesFromAncestry(CharacterData & cdata);

	bool GetSkillsByRace(uint32 raceID, std::map<uint32, uint32> &into);
	bool GetSkillsByCareer(uint32 careerID, std::map<uint32, uint32> &into);
	bool GetSkillsByCareerSpeciality(uint32 careerSpecialityID, std::map<uint32, uint32> &into);

	PyRepString *GetNote(uint32 ownerID, uint32 itemID); // LSMoura
	bool SetNote(uint32 ownerID, uint32 itemID, const char *str); // LSMoura
	
	//hack, needs to move and be cached:
	//uint32 GetRaceFromBloodline(uint32 bloodline);

	uint32 AddOwnerNote(uint32 charID, const std::string & label, const std::string & content);
	bool EditOwnerNote(uint32 charID, uint32 noteID, const std::string & label, const std::string & content);
	
protected:
	void AddNewSkill(const CharacterData & cdata, uint32 skillID);
};





#endif


