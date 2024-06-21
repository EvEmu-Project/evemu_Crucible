/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        Zhur
    Updates:        Allan
*/

#ifndef __CHARACTERDB_H_INCL__
#define __CHARACTERDB_H_INCL__

#include "ServiceDB.h"

// struct for portraitInfo
struct PortraitInfo {
    uint32 backgroundID;
    uint32 lightID;
    uint32 lightColorID;
    float cameraX;
    float cameraY;
    float cameraZ;
    float cameraFieldOfView;
    float cameraPoiX;
    float cameraPoiY;
    float cameraPoiZ;
    float headLookTargetX;
    float headLookTargetY;
    float headLookTargetZ;
    float lightIntensity;
    float portraitPoseNumber;
    float headTilt;
    float orientChar;
    float browLeftCurl;
    float browLeftTighten;
    float browLeftUpDown;
    float browRightCurl;
    float browRightTighten;
    float browRightUpDown;
    float eyeClose;
    float eyesLookVertical;
    float eyesLookHorizontal;
    float squintLeft;
    float squintRight;
    float jawSideways;
    float jawUp;
    float puckerLips;
    float frownLeft;
    float frownRight;
    float smileLeft;
    float smileRight;
};

/* POD structure for skills in queue */
struct QueuedSkill {
    uint8 level;
    uint16 typeID;
    int64 endTime;
    int64 startTime;
};
typedef std::vector<QueuedSkill> SkillQueue;

//class PyObject;
//class PyString;
//class PyObjectEx;
//class CharacterAppearance;
//class ItemFactory;
//class InventoryItem;
//class Client;

struct CharacterTypeData;
struct CharacterData;
struct KillData;
struct ItemData;
struct CorpData;

class CharacterDB : public ServiceDB
{
public:
    static uint32 NewCharacter(const CharacterData& data, const CorpData& corpData);
    static bool SaveCharacter(uint32 charID, const CharacterData &data);
    static bool SaveCorpData(uint32 charID, const CorpData &data);
    static void DeleteCharacter(uint32 charID);
    // this changes corp member counts, adds employment history, and updates char's corp and start date
    static void AddEmployment(uint32 charID, uint32 corpID, uint32 oldCorpID=0);
    static bool GetCharacterData(uint32 characterID, CharacterData &into);
    static void GetCharacterDataMap(uint32 charID, std::map<std::string, int64> &characterDataMap);
    static bool GetCharHomeStation(uint32 charID, uint32 &stationID);
    //if you want to get the typeID of the clone use GetActiveCloneType
    static bool GetActiveCloneID(uint32 charID, uint32 &itemID);
    static PyRep *GetInfoWindowDataForChar(uint32 charID);
    static uint32 GetStartingStationByCareer(uint32 careerID);

    static void SetCharacterOnlineStatus(uint32 char_id, bool online=false);

    static PyRep* List(uint32 ownerID);
    static PyRep* ListStations(uint32 ownerID, std::ostringstream& flagIDs, bool forCorp=false, bool bpOnly=false);
    static PyRep* ListStationItems(uint32 ownerID, uint32 stationID);
    static PyRep* ListStationBlueprintItems(uint32 ownerID, uint32 stationID, bool forCorp=false);

    static PyRep *GetCharacterList(uint32 accountID);
    static PyRep *GetCharSelectInfo(uint32 charID);
    void SetAvatar(uint32 charID, PyRep* hairDarkness);
    void SetAvatarColors(uint32 charID, uint32 colorID, uint32 colorNameA, uint32 colorNameBC, double weight, double gloss);
    void SetAvatarModifiers(uint32 charID, PyRep* modifierLocationID,  PyRep* paperdollResourceID, PyRep* paperdollResourceVariation);
    void SetAvatarSculpts(uint32 charID, PyRep* sculptLocationID, PyRep* weightUpDown, PyRep* weightLeftRight, PyRep* weightForwardBack);
    void SetPortraitInfo(uint32 charID, PortraitInfo &data);
    PyRep *GetCharPublicInfo(uint32 charID);
    PyRep *GetCharPublicInfo3(uint32 charID);
    PyRep *GetCharPrivateInfo(uint32 charID);

    //PyObject *GetAgentPublicInfo(uint32 agentID);
    PyRep *GetOwnerNoteLabels(uint32 charID);
    PyRep *GetOwnerNote(uint32 charID, uint32 noteID);
    uint32 PickAlternateShip(uint32 charID, uint32 locationID);
    void SetCurrentShip(uint32 charID, uint32 shipID);
    void SetCurrentPod(uint32 charID, uint32 podID);

    bool ChangeCloneType(uint32 charID, uint32 typeID);
    static bool ChangeCloneLocation(uint32 charID, uint32 locationID);
    bool GetCharClones(uint32 charID, std::vector<uint32> &into);
    bool GetActiveCloneType(uint32 charID, uint32 &typeID);
    std::string GetCharName(uint32 charID);

    PyRep* GetContacts(uint32 charID, bool blocked);
    void AddContact(uint32 ownerID, uint32 charID, int32 standing, bool inWatchlist);
    void UpdateContact(int32 standing, uint32 charID, uint32 ownerID);
    void RemoveContact(uint32 charID, uint32 ownerID);
    void SetBlockContact(uint32 charID, uint32 ownerID, bool blocked);

    bool GetCharItems(uint32 charID, std::vector<uint32> &into);
    static bool GetCareerBySchool(uint32 schoolID, uint8 &raceID, uint32 &careerID);

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


    /* OUTDATED, new below
     * static bool        GetAttributesFromAncestry(uint32 ancestryID, uint8 &intelligence, uint8 &charisma, uint8 &perception, uint8 &memory, uint8 &willpower); 
     */
    static bool        GetAttributesFromAttributes(uint8 &intelligence, uint8 &charisma, uint8 &perception, uint8 &memory, uint8 &willpower);
   
    static bool        GetBaseSkills(std::map< uint32, uint8 >& into);
    static bool        GetSkillsByRace(uint32 raceID, std::map< uint32, uint8 >& into);
    static bool        GetSkillsByCareer(uint32 careerID, std::map< uint32, uint8 >& into);

    /**
     * Retrieves the character note from the database as a PyString pointer.
     *
     * @author LSMoura
     */
    PyString*   GetNote(uint32 ownerID, uint32 itemID);

    /**
     * Stores the character note on the database, given the ownerID and itemID and the string itself.
     *
     * If the String is null or size zero, the entry is removed from the database.
     *
     * @return boolean true if success.
     *
     * @author LSMoura
     */
    bool        SetNote(uint32 ownerID, uint32 itemID, const char *str);

    uint32      AddOwnerNote(uint32 charID, const std::string &label, const std::string &content);
    bool        EditOwnerNote(uint32 charID, uint32 noteID, const std::string &label, const std::string &content);

    static int64       PrepareCharacterForDelete(uint32 accountID, uint32 charID);
    static void        CancelCharacterDeletePrepare(uint32 accountID, uint32 charID);

    bool        ReportRespec(uint32 characterId);
    PyRep*      GetRespecInfo(uint32 characterId);

    /**
     * Loads skill queue.
     *
     * @param[in] charID ID of character whose queue should be loaded.
     * @param[in] into SkillQueue into which loaded data should be stored.
     * @return True if load succeeds, false if fails.
     */
    bool        LoadSkillQueue(uint32 charID, SkillQueue &into);
    bool        LoadPausedSkillQueue(uint32 charID, SkillQueue &into);
    /**
     * Saves skill queue.
     *
     * @param[in] charID ID of character whose skill queue is saved.
     * @param[in] queue Queue to save.
     * @return True if save succeeds, false if fails.
     */
    bool        SaveSkillQueue(uint32 charID, SkillQueue &queue);
    bool        SavePausedSkillQueue(uint32 charID, SkillQueue &queue);
    void        SaveSkillHistory(uint16 eventID, double logDate, uint32 characterID, uint32 skillTypeID, uint8 skillLevel, uint32 absolutePoints);
    PyRep*      GetSkillHistory(uint32 charID);
    void        UpdateSkillQueueEndTime(int64 endtime, uint32 charID);

    void        SetLogInTime(uint32 charID);
    void        SetLogOffTime(uint32 charID);

    static void        AddOwnerCache(uint32 ownerID, std::string ownerName, uint32 typeID);

    PyRep*      GetBounty(uint32 charID, uint32 ownerID);
    PyRep*      GetTopBounties();
    void        AddBounty(uint32 charID, uint32 ownerID, uint32 amount);

    PyRep*      GetKillOrLoss(uint32 charID);

    static void SetCorpRole(uint32 charID, int64 role);
    static int64 GetCorpRole(uint32 charID);
    static uint32 GetCorpID(uint32 charID);
    static float GetCorpTaxRate(uint32 charID);
    static PyRep* GetMyCorpMates(uint32 corpID);
    static bool GetCharCorpData(uint32 characterID, CorpData &into);

    // get skill level for given skill for offline character (used by market tax)
    static uint8 GetSkillLevel(uint32 charID, uint16 skillTypeID);

    PyRep*      GetLabels(uint32 charID);
    void        SetLabel(uint32 charID, uint32 color, std::string name);
    void        EditLabel(uint32 charID, uint32 labelID, uint32 color, std::string name);
    void        DeleteLabel(uint32 charID, uint32 labelID);

    void        VisitSystem(uint32 solarSystemID, uint32 charID);

    /*  name validation shit */
    // called on CreateCharacterWithDoll() and will throw on error
    static void        ValidateCharName(std::string name);
    // called by "Check Name" button in char creation.  returns integer
    static PyRep*      ValidateCharNameRep(std::string name);

    /**
     * Loads character type data.
     *
     * @param[in] bloodlineID Bloodline to be loaded.
     * @param[out] into Where loaded data should be stored.
     * @return True on success, false on failure.
     */
    static bool GetCharacterType(uint8 bloodlineID, CharacterTypeData &into);
    /**
     * Obtains bloodline and loads character type data.
     *
     * @param[in] characterTypeID ID of character type to be loaded.
     * @param[out] bloodlineID Resulting bloodline.
     * @param[out] into Where character type data should be stored.
     * @return True on success, false on failure.
     */
    static bool GetCharacterType(uint16 characterTypeID, uint8 &bloodlineID, CharacterTypeData &into);
    /**
     * Obtains ID of character type based on bloodline.
     *
     * @param[in] bloodlineID ID of bloodline.
     * @param[out] characterTypeID Resulting ID of character type.
     * @return True on success, false on failure.
     */
    static bool GetCharacterTypeByBloodline(uint8 bloodlineID, uint16 &characterTypeID);
    /**
     * Obtains ID of character type and loads it.
     *
     * @param[in] bloodlineID ID of bloodline to be loaded.
     * @param[out] characterTypeID Resulting character type.
     * @param[out] into Where loaded character type data should be stored.
     * @return True on success, false on failure.
     */
    static bool GetCharacterTypeByBloodline(uint8 bloodlineID, uint16 &characterTypeID, CharacterTypeData &into);
    /**
     * Obtains ID of bloodline based on character type.
     *
     * @param[in] characterTypeID ID of character type.
     * @param[out] bloodlineID Resulting ID of bloodline.
     * @return True on success, false on failure.
     */
    static bool GetBloodlineByCharacterType(uint16 characterTypeID, uint8& bloodlineID);
};

#endif
