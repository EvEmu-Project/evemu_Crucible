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
    Author:        Zhur, Bloody.Rabbit
    Updates:        Allan
*/

#ifndef __CHARACTER__H__INCL__
#define __CHARACTER__H__INCL__

#include "StaticDataMgr.h"
#include "character/CertificateMgrDB.h"
#include "character/CharacterDB.h"
#include "character/Skill.h"
#include "corporation/CorporationDB.h"
#include "inventory/ItemType.h"
#include "inventory/InventoryDB.h"
#include "inventory/InventoryItem.h"
#include "standing/StandingDB.h"

/**
 * Simple container for raw character type data.
 */
class CharacterTypeData {
public:
    CharacterTypeData(
        const char* _bloodlineName = "",
        uint8 _race = 0,
        const char* _desc = "",
        const char* _maleDesc = "",
        const char* _femaleDesc = "",
        uint32 _corporationID = 0,
        uint8 _perception = 0,
        uint8 _willpower = 0,
        uint8 _charisma = 0,
        uint8 _memory = 0,
        uint8 _intelligence = 0,
        const char* _shortDesc = "",
        const char* _shortMaleDesc = "",
        const char* _shortFemaleDesc = ""
    );

    // Content:
    std::string bloodlineName;
    uint8 race;
    std::string description;
    std::string maleDescription;
    std::string femaleDescription;
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
: public ItemType
{
    friend class ItemType; // to let it construct us
public:
    static CharacterType* Load(uint16 typeID);

    /*
     * Access functions:
     */
    uint8 bloodlineID() const                           { return m_bloodlineID; }

    const std::string& bloodlineName() const            { return m_bloodlineName; }
    const std::string& description() const              { return m_description; }
    const std::string& maleDescription() const          { return m_maleDescription; }
    const std::string& femaleDescription() const        { return m_femaleDescription; }
    uint32 corporationID() const                        { return m_corporationID; }

    uint8 perception() const                            { return m_perception; }
    uint8 willpower() const                             { return m_willpower; }
    uint8 charisma() const                              { return m_charisma; }
    uint8 memory() const                                { return m_memory; }
    uint8 intelligence() const                          { return m_intelligence; }

    const std::string& shortDescription() const         { return m_shortDescription; }
    const std::string& shortMaleDescription() const     { return m_shortMaleDescription; }
    const std::string& shortFemaleDescription() const   { return m_shortFemaleDescription; }

protected:
    CharacterType(
        uint16 _id,
        uint8 _bloodlineID,
        // ItemType stuff:
        const Inv::TypeData& _data,
        // CharacterType stuff:
        const CharacterTypeData& _charData
    );

    /**
     * Member functions
     */
    using ItemType::_Load;

    // Template loader:
    template<class _Ty>
    static _Ty* _LoadType(uint16 typeID, const Inv::TypeData& data)
    {
        // check we are really loading a character type
        if (data.groupID != EVEDB::invGroups::Character) {
            _log( ITEM__ERROR, "Trying to load %s as CharacterType.", sDataMgr.GetGroupName(data.groupID));
            return nullptr;
        }

        // query character type data
        uint8 bloodlineID(0);
        CharacterTypeData charData;
        if (!CharacterDB::GetCharacterType(typeID, bloodlineID, charData) )
            return nullptr;

        return new CharacterType( typeID, bloodlineID, data, charData );
    }

    /*
     * Data members
     */
    uint8 m_bloodlineID;

    std::string m_bloodlineName;
    std::string m_description;
    std::string m_maleDescription;
    std::string m_femaleDescription;
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
 * Container for character appearance stuff.
 */
class CharacterAppearance {
    // use default c'tor et. al.
public:
    /*  these arent used...
   uint32 colorID;
   uint32 colorNameA;
   uint32 colorNameBC;
   double weight;
   double gloss;

   uint32 modifierLocationID;
   uint32 paperdollResourceID;
   uint32 paperdollResourceVariation;

   uint32 sculptID;
   double weightUpDown;
   double weightLeftRight;
   double weightForwardBack;
*/
   void Build(uint32 ownerID, PyDict* data);

private:
	CharacterDB m_db;
};

/**
 * Container for character portrait stuff.
 */
class CharacterPortrait {
    // use default c'tor et. al.

public:
    void Build(uint32 charID, PyDict* data);

private:
    CharacterDB m_db;
};

/**
 * Class representing character.
 */
class Character
: public InventoryItem
{
    friend class InventoryItem;    // to let it construct us

public:
    virtual void Delete();

    /**
     * Primary public interface:
     */
    bool            AlterBalance(float amount, uint8 type);
    void            SetLocation(uint32 stationID, SystemData& data);
    void            JoinCorporation(const CorpData& data);
    void            SetDescription(const char *newDescription);
    void            SetAccountKey(int32 accountKey);
    void            SetBaseID(uint32 baseID);
    void            SetFleetData(CharFleetData& fleet);
    uint32          PickAlternateShip(uint32 locationID);

    void            SetClient(Client* pClient)          { m_pClient = pClient; }
    Client*         GetClient()                         { return m_pClient; }

    void            AddItem(InventoryItemRef item);

    // skills
    bool            HasSkill(uint16 skillTypeID) const;
    bool            HasSkillTrainedToLevel(uint16 skillTypeID, uint8 skillLevel) const;
    SkillRef        GetCharSkillRef(uint16 skillTypeID) const;
    int8            GetSkillLevel(uint16 skillTypeID, bool zeroForNotInjected = true) const;
    PyRep*          GetRAMSkills();
    Skill*          GetSkillInTraining() const          { return m_inTraining; }
    void            GetSkillsList(std::vector<InventoryItemRef>& skills) const;
    void            VerifySP();
    uint32          GetTotalSPTrained()                 { return m_charData.skillPoints; };
    uint8           GetSPPerMin(Skill* skill);
    int64           GetEndOfTraining();
    uint8           InjectSkillIntoBrain(SkillRef skill);
    void            AddToSkillQueue(uint16 typeID, uint8 level);
    void            ClearSkillQueue(bool update=false);
    void            PauseSkillQueue();
    // if there is currently a skill in training, this will check completion, set and save attribs, and remove from queue
    void            CancelSkillInTraining(bool update=false);
    void            LoadPausedSkillQueue(uint16 typeID);
    void            SkillQueueLoop(bool update=true);
    void            UpdateSkillQueue();
    void            UpdateSkillQueueEndTime();
    void            RemoveFromQueue(SkillRef sRef);
    void            ClearSkillFlags();

    PyRep*          GetSkillHistory();
    uint32          GetTotalSP();

    /* Certificates */
    void            GrantCertificate( uint32 certificateID );
    void            UpdateCertificate( uint32 certificateID, bool pub );
    bool            HasCertificate( uint32 certificateID ) const;
    void            GetCertificates( CertMap& crt );

    /* Primary public packet builders */
    PyDict*         GetCharInfo();
    PyObject*       GetDescription() const;
    PyTuple*        SendSkillQueue();

    /* Public fields */
    const CharacterType&    type() const                        { return static_cast<const CharacterType& >(InventoryItem::type()); }
    uint32                  bloodlineID() const                 { return type().bloodlineID(); }
    uint8                   race() const                        { return type().race(); }

    // Account:
    uint32                  accountID() const                   { return m_charData.accountID; }

    const std::string&      title() const                       { return m_charData.title; }
    const std::string&      description() const                 { return m_charData.description; }
    bool                    gender() const                      { return m_charData.gender; }

    float                   bounty() const                      { return m_charData.bounty; }
    float                   balance(uint8 type);
    float                   GetSecurityRating() const           { return m_charData.securityRating; }
    uint32					loginTime() const                   { return m_loginTime; }
    uint32                  logonMinutes() const                { return m_charData.logonMinutes; }
    uint16                  OnlineTime();

    void                    secStatusChange( float amount )    { m_charData.securityRating += amount; }

    // Corporation:
    void                    UpdateCorpData(CorpData& data);
    CorpData                GetCorpData()                       { return m_corpData; }
    std::string             corpTicker() const                  { return m_corpData.ticker; }
    uint32                  corporationID() const               { return m_corpData.corporationID; }
    uint32                  corporationHQ() const               { return m_corpData.corpHQ; }
    int32                   allianceID() const                  { return m_corpData.allianceID; }
    int32                   warFactionID() const                { return m_corpData.warFactionID; }
    int32                   corpAccountKey() const              { return m_corpData.corpAccountKey; }
    float                   corpTaxRate() const                 { return m_corpData.taxRate; }
    float                   corpSecRating() const               { return m_corpData.secRating; }
    void                    SetCorpHQ(uint32 stationID)         { m_corpData.corpHQ = stationID; UpdateCorpData(m_corpData);}

    // Corporation role:
    int64                   corpRole() const                    { return m_corpData.corpRole; }
    int64                   rolesAtAll() const                  { return m_corpData.rolesAtAll; }
    int64                   rolesAtBase() const                 { return m_corpData.rolesAtBase; }
    int64                   rolesAtHQ() const                   { return m_corpData.rolesAtHQ; }
    int64                   rolesAtOther() const                { return m_corpData.rolesAtOther; }

    // Fleet:
    int64                   fleetJoinTime()                     { return m_fleetData.joinTime; }
    int32                   fleetID() const                     { return m_fleetData.fleetID; }
    int32                   wingID() const                      { return m_fleetData.wingID; }
    int32                   squadID() const                     { return m_fleetData.squadID; }
    int8                    fleetRole() const                   { return m_fleetData.role; }
    int8                    fleetBooster() const                { return m_fleetData.booster; }
    int8                    fleetJob() const                    { return m_fleetData.job; }

    // Current location:
    uint32                  stationID() const                   { return m_charData.stationID; }
    uint32                  solarSystemID() const               { return m_charData.solarSystemID; }
    uint32                  constellationID() const             { return m_charData.constellationID; }
    uint32                  regionID() const                    { return m_charData.regionID; }

    // Ancestry, career:
    uint32                  ancestryID() const                  { return m_charData.ancestryID; }
    uint32                  careerID() const                    { return m_charData.careerID; }
    uint32                  schoolID() const                    { return m_charData.schoolID; }
    uint32                  careerSpecialityID() const          { return m_charData.careerSpecialityID; }

    // Some important dates:
    int64                   startDateTime() const               { return m_corpData.startDateTime; }
    int64                   createDateTime() const              { return m_charData.createDateTime; }

    uint32                  shipID() const                      { return m_charData.shipID; }
    uint32                  capsuleID() const                   { return m_charData.capsuleID; }

    void                    SetActiveShip(uint32 shipID);
    void                    SetActivePod(uint32 podID);
    void                    ResetClone();

    void                    PayBounty(CharacterRef cRef);
    void                    LogKill(KillData data)          { ServiceDB::SaveKillOrLoss(data); }

    //  saves
    void                    LogOut();
    void                    SaveBookMarks();
    void                    SaveCharacter();
    void                    SaveFullCharacter();
    void                    SaveSkillQueue();
    void                    SaveCertificates();
    void                    SaveSkillHistory(uint16 eventID, double logDate, uint32 characterID, uint16 skillTypeID, uint8 skillLevel, uint32 absolutePoints);

    void                    SetLoaded(bool set=false)   { m_loaded = set; }

    void                    SetLoginTime();
    void                    SetLogonMinutes();

    //  Standings functions
    //  fromID = char|agent|corp|faction|alliance   toID = me|myCorp|myAlliance.
    float                   GetStandingModified(uint32 fromID, uint32 toID=0);    // this IS adjusted for skills
    //  fromID = char|agent|corp|faction|alliance   toID = me|myCorp|myAlliance.
    float                   GetNPCCorpStanding(uint32 fromID, uint32 toID=0);
    void                    SetStanding(uint32 fromID, uint32 toID, float standing);
    void                    FleetShareMissionRewards();
    void                    FleetShareMissionStandings(float newStanding);

    //  Dynamic Data
    void                    VisitSystem(uint32 solarSystemID);

    // character skill, implant and booster effects.  parsed and applied on undock and ship init in space (with all other ship-related effects)
    // NOTE:  implants and boosters not implemented yet
    void                    ProcessEffects(ShipItem* pShip);
    void                    ResetModifiers();   // this will reset ALL char and skill attribs and modifier maps to default

protected:
    Character(
        uint32 _characterID,
        // InventoryItem stuff:
        const CharacterType& _charType,
        const ItemData& _data,
        // Character stuff:
        const CharacterData& _charData,
        const CorpData& _corpData
    );
    virtual ~Character();

    void LoadBookmarks();

    /*
     * templated loading system
     */
public:
    static CharacterRef Load( uint32 characterID);
    static CharacterRef Spawn( CharacterData& charData, CorpData& corpData);
    virtual bool _Load();

protected:
    using InventoryItem::_Load;

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 charID, const ItemType& type, const ItemData& data) {
        if (type.groupID() != EVEDB::invGroups::Character) {
            _log(ITEM__ERROR, "Trying to load %s as Character.", sDataMgr.GetCategoryName(type.categoryID()));
            if (sConfig.debug.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>(nullptr);
        }
        CharacterData charData = CharacterData();
        if (!CharacterDB::GetCharacterData(charID, charData))
            return RefPtr<_Ty>(nullptr);

        CorpData corpData = CorpData();
        if (!CharacterDB::GetCharCorpData(charID, corpData))
            return RefPtr<_Ty>(nullptr);

        // cast the type
        const CharacterType& charType = static_cast<const CharacterType& >(type);

        // construct the character item
        return CharacterRef(new Character(charID, charType, data, charData, corpData));
    }


private:
    CertificateMgrDB m_cdb;
    CharacterDB m_db;
    StandingDB s_db;

    Client* m_pClient;

    CorpData m_corpData;
    CharacterData m_charData;
    CharFleetData m_fleetData;

    // Skill queue:
    Skill* m_inTraining;      // holder for training skill, as inventory maps wont work right.
    SkillQueue m_skillQueue;    //std::vector<QueuedSkill>
    uint32 m_freePoints;

    CertMap m_certificates;     // std::map<uint16, CharCerts>

    bool m_loaded :1;      /* to avoid multiple load calls (_Load is called ~4x) */

    uint32 m_loginTime;

    std::map<uint8, InventoryItemRef>  m_implantMap;    // slotID/itemRef
};

#endif /* !__CHARACTER__H__INCL__ */

/*{'FullPath': u'UI/Messages', 'messageID': 259712, 'label': u'CharTerminationSecM02Body'}(u'They would occasionally scare the odd passer by.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259713, 'label': u'CharTerminationSecM03Body'}(u'Regarded as almost being a dangerous character.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259714, 'label': u'CharTerminationSecM04Body'}(u'Regarded as being a dangerous character.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259715, 'label': u'CharTerminationSecM05Body'}(u'Regarded as being quite a dangerous character.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259716, 'label': u'CharTerminationSecM06Body'}(u'An effective dispatcher of foes, and those that got in the way.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259717, 'label': u'CharTerminationSecM07Body'}(u'An fairly effective dispatcher of foes, and those that got in the way.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259718, 'label': u'CharTerminationSecM08Body'}(u'A highly effective dispatcher of foes, and those that got in the way.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259719, 'label': u'CharTerminationSecM09Body'}(u'An accomplished expeditor of their adversaries, and those that got in the way.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259720, 'label': u'CharTerminationSecP01Body'}(u'Liked by some, and trusted by close friends occasionally.', None, None)
 */