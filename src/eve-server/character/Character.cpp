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
    Author:     Zhur, Bloody.Rabbit
    Updates:        Allan
*/

#include "eve-server.h"
//#include "../../eve-common/EVE_Skills.h"

#include "Client.h"
#include "ConsoleCommands.h"
#include "EntityList.h"
#include "StaticDataMgr.h"
#include "StatisticMgr.h"
#include "account/AccountService.h"
#include "character/Character.h"
#include "effects/EffectsProcessor.h"
#include "fleet/FleetService.h"
#include "inventory/AttributeEnum.h"
#include "inventory/Inventory.h"
#include "ship/Ship.h"

/*
 * CharacterTypeData
 */
CharacterTypeData::CharacterTypeData(
    const char* _bloodlineName, uint8 _race, const char* _desc, const char* _maleDesc, const char* _femaleDesc, uint32 _corporationID, uint8 _perception, uint8 _willpower, uint8 _charisma, uint8 _memory, uint8 _intelligence, const char* _shortDesc, const char* _shortMaleDesc, const char* _shortFemaleDesc)
: bloodlineName(_bloodlineName),
  race(_race),
  description(_desc),
  maleDescription(_maleDesc),
  femaleDescription(_femaleDesc),
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
CharacterType::CharacterType(uint16 _id, uint8 _bloodlineID, const Inv::TypeData& _data, const CharacterTypeData& _charData)
: ItemType(_id, _data),
  m_bloodlineID(_bloodlineID),
  m_bloodlineName(_charData.bloodlineName),
  m_description(_charData.description),
  m_maleDescription(_charData.maleDescription),
  m_femaleDescription(_charData.femaleDescription),
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
}

CharacterType *CharacterType::Load(uint16 typeID) {
    return ItemType::Load<CharacterType>(typeID );
}

/*
 * CharacterAppearance
 */
void CharacterAppearance::Build(uint32 ownerID, PyDict* data)
{
    PyList* colors = data->GetItemString("colors")->AsList();
    PyList::const_iterator color_cur = colors->begin();
    for (; color_cur != colors->end(); ++color_cur) {
        if ((*color_cur)->IsObjectEx()) {
            PyObjectEx_Type2* color_obj = (PyObjectEx_Type2*)(*color_cur)->AsObjectEx();
            PyTuple* color_tuple = color_obj->GetArgs()->AsTuple();
            //color tuple data structure
            //[0] PyToken
            //[1] colorID
            //[2] colorNameA
            //[3] colorNameBC
            //[4] weight
            //[5] gloss

            m_db.SetAvatarColors(ownerID,
                                 PyRep::IntegerValue(color_tuple->GetItem(1)),
                                 PyRep::IntegerValue(color_tuple->GetItem(2)),
                                 PyRep::IntegerValue(color_tuple->GetItem(3)),
                                 color_tuple->GetItem(4)->AsFloat()->value(),
                                 color_tuple->GetItem(5)->AsFloat()->value());
        }
    }

    PyObjectEx* appearance = data->GetItemString("appearance")->AsObjectEx();
    PyObjectEx_Type2* app_obj = (PyObjectEx_Type2*)appearance;
    PyTuple* app_tuple = app_obj->GetArgs()->AsTuple();

    m_db.SetAvatar(ownerID, app_tuple->GetItem(1));

    PyList* modifiers = data->GetItemString("modifiers")->AsList();
    PyList::const_iterator modif_cur = modifiers->begin();
    for (; modif_cur != modifiers->end(); ++modif_cur) {
        if ((*modif_cur)->IsObjectEx()) {
            PyObjectEx_Type2* modif_obj = (PyObjectEx_Type2*)(*modif_cur)->AsObjectEx();
            PyTuple* modif_tuple = modif_obj->GetArgs()->AsTuple();

            //color tuple data structure
            //[0] PyToken
            //[1] modifierLocationID
            //[2] paperdollResourceID
            //[3] paperdollResourceVariation
            m_db.SetAvatarModifiers(ownerID,
                                    modif_tuple->GetItem(1),
                                    modif_tuple->GetItem(2),
                                    modif_tuple->GetItem(3));
        }
    }

    PyList* sculpts = data->GetItemString("sculpts")->AsList();
    PyList::const_iterator sculpt_cur = sculpts->begin();
    for (; sculpt_cur != sculpts->end(); sculpt_cur++) {
        if ((*sculpt_cur)->IsObjectEx()) {
            PyObjectEx_Type2* sculpt_obj = (PyObjectEx_Type2*)(*sculpt_cur)->AsObjectEx();
            PyTuple* sculpt_tuple = sculpt_obj->GetArgs()->AsTuple();

            //sculpts tuple data structure
            //[0] PyToken
            //[1] sculptLocationID
            //[2] weightUpDown
            //[3] weightLeftRight
            //[4] weightForwardBack
            m_db.SetAvatarSculpts(ownerID,
                                sculpt_tuple->GetItem(1),
                                sculpt_tuple->GetItem(2),
                                sculpt_tuple->GetItem(3),
                                sculpt_tuple->GetItem(4));
        }
    }
}


/*
 * CharacterPortrait
 */

void CharacterPortrait::Build(uint32 charID, PyDict* data)
{
    PortraitInfo info = PortraitInfo();

    info.backgroundID = PyRep::IntegerValue(data->GetItemString("backgroundID"));
    info.lightColorID = PyRep::IntegerValue(data->GetItemString("lightColorID"));
    info.lightID = PyRep::IntegerValue(data->GetItemString("lightID"));

    info.cameraFieldOfView = data->GetItemString("cameraFieldOfView")->AsFloat()->value();
    info.lightIntensity = data->GetItemString("lightIntensity")->AsFloat()->value();

    PyTuple* cameraPoi = data->GetItemString("cameraPoi")->AsTuple();
    info.cameraPoiX = cameraPoi->GetItem(0)->AsFloat()->value();
    info.cameraPoiY = cameraPoi->GetItem(1)->AsFloat()->value();
    info.cameraPoiZ = cameraPoi->GetItem(2)->AsFloat()->value();

    PyTuple* cameraPosition = data->GetItemString("cameraPosition")->AsTuple();
    info.cameraX = cameraPosition->GetItem(0)->AsFloat()->value();
    info.cameraY = cameraPosition->GetItem(1)->AsFloat()->value();
    info.cameraZ = cameraPosition->GetItem(2)->AsFloat()->value();

    PyDict* poseData = data->GetItemString("poseData")->AsDict();
    PyTuple* HeadLookTarget = poseData->GetItemString("HeadLookTarget")->AsTuple();
    info.headLookTargetX = HeadLookTarget->GetItem(0)->AsFloat()->value();
    info.headLookTargetY = HeadLookTarget->GetItem(1)->AsFloat()->value();
    info.headLookTargetZ = HeadLookTarget->GetItem(2)->AsFloat()->value();

    info.browLeftCurl = poseData->GetItemString("BrowLeftCurl")->AsFloat()->value();
    info.browLeftUpDown = poseData->GetItemString("BrowLeftUpDown")->AsFloat()->value();
    info.browLeftTighten = poseData->GetItemString("BrowLeftTighten")->AsFloat()->value();
    info.browRightCurl = poseData->GetItemString("BrowRightCurl")->AsFloat()->value();
    info.browRightUpDown = poseData->GetItemString("BrowRightUpDown")->AsFloat()->value();
    info.browRightTighten = poseData->GetItemString("BrowRightTighten")->AsFloat()->value();

    info.squintLeft = poseData->GetItemString("SquintLeft")->AsFloat()->value();
    info.smileLeft = poseData->GetItemString("SmileLeft")->AsFloat()->value();
    info.frownLeft = poseData->GetItemString("FrownLeft")->AsFloat()->value();
    info.squintRight = poseData->GetItemString("SquintRight")->AsFloat()->value();
    info.smileRight = poseData->GetItemString("SmileRight")->AsFloat()->value();
    info.frownRight = poseData->GetItemString("FrownRight")->AsFloat()->value();

    info.jawUp = poseData->GetItemString("JawUp")->AsFloat()->value();
    info.jawSideways = poseData->GetItemString("JawSideways")->AsFloat()->value();
    info.headTilt = poseData->GetItemString("HeadTilt")->AsFloat()->value();
    info.eyeClose = poseData->GetItemString("EyeClose")->AsFloat()->value();
    info.eyesLookHorizontal = poseData->GetItemString("EyesLookHorizontal")->AsFloat()->value();
    info.eyesLookVertical = poseData->GetItemString("EyesLookVertical")->AsFloat()->value();
    info.orientChar = poseData->GetItemString("OrientChar")->AsFloat()->value();
    info.portraitPoseNumber = poseData->GetItemString("PortraitPoseNumber")->AsFloat()->value();
    info.puckerLips = poseData->GetItemString("PuckerLips")->AsFloat()->value();

    m_db.SetPortraitInfo(charID, info);
}


/*
 * Character
 */
Character::Character(
    uint32 _characterID,
    // InventoryItem stuff:
    const CharacterType &_charType,
    const ItemData &_data,
    // Character stuff:
    const CharacterData &_charData,
    const CorpData &_corpData)
: InventoryItem(_characterID, _charType, _data),
  m_charData(_charData),
  m_corpData(_corpData),
  m_pClient(nullptr),
  m_inTraining(nullptr),
  m_loaded(false),
  m_fleetData(CharFleetData()),
  m_freePoints(0),
  m_loginTime(0)
{
    // enforce characters to be singletons
    assert(isSingleton());

    if (!IsAgent(m_itemID)) {
        m_loginTime = sEntityList.GetStamp();
        pInventory = new Inventory(InventoryItemRef(this));
    }
}

Character::~Character()
{
    SaveBookMarks();
    SaveFullCharacter();
    SaveCertificates();
    SafeDelete(pInventory);
}

CharacterRef Character::Load( uint32 characterID) {
    return InventoryItem::Load<Character>( characterID );
}

bool Character::_Load() {
    if (m_loaded)
        return true;
    if (IsAgent(m_itemID))
        return true;

    if (!pInventory->LoadContents()) {
        sLog.Error("Character::_Load","LoadContents returned false for char %u", m_itemID);
        return (m_loaded = false);
    }
    if (!m_db.LoadSkillQueue(m_itemID, m_skillQueue)) {
        sLog.Error("Character::_Load","LoadSkillQueue returned false for char %u", m_itemID);
        return (m_loaded = false);
    }
    if (!m_skillQueue.empty()) {
        SkillRef sRef = GetCharSkillRef(m_skillQueue.front().typeID);
        if (sRef.get() != nullptr) {
            sRef->SetFlag(flagSkillInTraining, false);
            m_inTraining = sRef.get();
        }
    } else {
        ClearSkillFlags();
    }

    m_loaded = InventoryItem::_Load();

    if (m_loaded) {
        m_certificates.clear();
        if (!m_cdb.LoadCertificates(m_itemID, m_certificates)) {
            sLog.Warning("Character::_Load","LoadCertificates returned false for char %u", m_itemID);
            return (m_loaded = false);
        }
    }

    // load char personal bookmarks and folders ... corp shit will be done ???
    LoadBookmarks();

    m_charData.loginTime = GetFileTimeNow();

    return m_loaded;
}

void Character::VerifySP()
{
    std::vector<InventoryItemRef> skills;
    pInventory->GetItemsByFlag(flagSkill, skills);
    for (auto cur : skills) {
        SkillRef::StaticCast(cur)->VerifyAttribs();
        SkillRef::StaticCast(cur)->VerifySP();
    }
}

CharacterRef Character::Spawn( CharacterData& charData, CorpData& corpData) {
    // make sure it's a character
    const CharacterType *ct = sItemFactory.GetCharacterType(charData.typeID);
    if (ct == nullptr)
        return CharacterRef(nullptr);

    uint32 characterID(CharacterDB::NewCharacter(charData, corpData));
    if (!IsCharacterID(characterID)) {
        _log(CHARACTER__ERROR, "Failed to get itemID for new character.");
        return CharacterRef(nullptr);
    }

    return Character::Load(characterID);
}

void Character::LogOut()
{
    SaveFullCharacter();
    m_db.SetLogOffTime(m_itemID);
    if (!sConsole.IsShutdown())
        if (IsFleetID(m_fleetData.fleetID))
            sFltSvc.LeaveFleet(m_pClient);

    pInventory->Unload();

    if (!m_pClient->IsCharCreation()) {
        sItemFactory.RemoveItem(m_itemID);
        if (sDataMgr.IsStation(m_charData.locationID))
            ;   // do we need to do anything here?
    }
}

void Character::Delete() {
    // delete contents
    pInventory->DeleteContents();
    // delete character record
    m_db.DeleteCharacter(m_itemID);
    // let the parent care about the rest
    InventoryItem::Delete();
}

float Character::balance(uint8 type)
{
    if (type == Account::CreditType::ISK) {
        return m_charData.balance;
    } else if (type == Account::CreditType::AURUM) {
        return m_charData.aurBalance;
    } else {
        _log(ACCOUNT__ERROR, "Character::balance() - invalid type %u", type);
    }
    return 0.0f;
}

bool Character::AlterBalance(float amount, uint8 type) {
    if (amount == 0)
        return true;

    // amount can be negative.  check for funds to remove, if applicable
    if ((balance(type) + amount) < 0) {
        std::map<std::string, PyRep *> args;
        args["amount"] = new PyFloat(-amount);
        args["balance"] = new PyFloat(balance(type));
        throw UserError ("NotEnoughMoney")
                .AddISK ("amount", -amount)
                .AddISK ("balance", balance (type));
    }

    //adjust balance and send notification of change
    OnAccountChange ac;
    ac.ownerid = m_itemID;
    if (type == Account::CreditType::ISK) {
        m_charData.balance += amount;
        // cap isk balance at one trillion
        if (m_charData.balance > 1000000000000)
            m_charData.balance = 1000000000000;
        ac.balance = m_charData.balance;
        ac.accountKey = "cash";
    } else if (type == Account::CreditType::AURUM) {
        m_charData.aurBalance += amount;
        ac.balance = m_charData.aurBalance;
        ac.accountKey = "AURUM";
    }

    PyTuple *answer = ac.Encode();
    m_pClient->SendNotification("OnAccountChange", "cash", &answer, false);

    SaveCharacter();
    return true;
}

void Character::SetLocation(uint32 stationID, SystemData& data) {
    m_charData.locationID = (stationID == 0 ? data.systemID : stationID);
    m_charData.stationID = stationID;
    m_charData.solarSystemID = data.systemID;
    m_charData.constellationID = data.constellationID;
    m_charData.regionID = data.regionID;
    SaveCharacter();
}

void Character::SetDescription(const char *newDescription) {
    m_charData.description = newDescription;
    SaveCharacter();
}

void Character::JoinCorporation(const CorpData &data) {
    // Add new employment history record    -allan  25Mar14   update 20Jan15   update again 23May19
    CharacterDB::AddEmployment(m_itemID, data.corporationID, m_corpData.corporationID);
    m_corpData = data;
    m_pClient->UpdateCorpSession(m_corpData);
    /** @todo remove this in favor of updating data when changed */
    m_db.SaveCorpData(m_itemID, m_corpData);
}

void Character::SetAccountKey(int32 accountKey)
{
    m_corpData.corpAccountKey = accountKey;
    /** @todo remove this in favor of updating data when changed */
    m_db.SaveCorpData(m_itemID, m_corpData);
    m_pClient->UpdateCorpSession(m_corpData);
}

void Character::SetBaseID(uint32 baseID)
{
    m_corpData.baseID = baseID;
    /** @todo remove this in favor of updating data when changed */
    m_db.SaveCorpData(m_itemID, m_corpData);
    m_pClient->UpdateCorpSession(m_corpData);
}

void Character::UpdateCorpData(CorpData& data)
{
    m_corpData = data;
    m_pClient->UpdateCorpSession(m_corpData);
}

uint32 Character::PickAlternateShip(uint32 locationID)
{
    return m_db.PickAlternateShip(m_itemID, locationID);
}

void Character::SetFleetData(CharFleetData& fleet)
{
    m_fleetData = fleet;
    //if ((fleet.joinTime) and (m_fleetJoinTime != fleet.joinTime))
        //  m_fleetJoinTime = fleet.joinTime;

    m_pClient->UpdateFleetSession(m_fleetData);
}

void Character::FleetShareMissionRewards()
{
    // not used yet
}

void Character::FleetShareMissionStandings(float newStanding)
{   // not used yet.
    if (m_fleetData.fleetID == 0)
        return;
    // negative standings are NOT shared with fleet
    if (newStanding < 0)
        return;
}

void Character::ResetModifiers()
{
    _log(EFFECTS__TRACE, "Character::ResetModifiers()");
    ClearModifiers();
    ResetAttributes();
    std::vector<InventoryItemRef> allSkills;
    pInventory->GetItemsByFlag(flagSkill, allSkills);
    for (auto curSkill : allSkills) {
        curSkill->ClearModifiers();
        curSkill->ResetAttributes();
    }
}

void Character::ProcessEffects(ShipItem* pShip)
{
    _log(EFFECTS__TRACE, "Character::ProcessEffects()");
    //ResetModifiers();

    //  427 total skills.  this should be fairly fast...it is.
    std::vector<InventoryItemRef> allSkills;
    pInventory->GetItemsByFlag(flagSkill, allSkills);

    Effect curEffect = Effect();
    std::vector<TypeEffects> typeFx;
    for (auto curSkill : allSkills) {
        typeFx.clear();
        sFxDataMgr.GetTypeEffect(curSkill->typeID(), typeFx);
        for (auto curFx : typeFx) {
            curEffect = sFxDataMgr.GetEffect(curFx.effectID);
            fxData data = fxData();
            data.action = FX::Action::Invalid;
            data.srcRef = curSkill;
            sFxProc.ParseExpression(this, sFxDataMgr.GetExpression(curEffect.preExpression), data);
        }
    }
    // apply processed char effects
    sFxProc.ApplyEffects(this, this, pShip);
}

/*
 * SKILL__ERROR
 * SKILL__WARNING
 * SKILL__MESSAGE
 * SKILL__INFO
 * SKILL__DEBUG
 * SKILL__TRACE
 * SKILL__QUEUE
 */

void Character::GetSkillsList(std::vector<InventoryItemRef> &skills) const {
    pInventory->GetItemsByFlag(flagSkill, skills);
}

bool Character::HasSkill(uint16 skillTypeID) const {
    return (GetCharSkillRef(skillTypeID).get() != nullptr);
}

PyRep* Character::GetSkillHistory() {
    return m_db.GetSkillHistory(m_itemID);
}

void Character::SaveSkillQueue() {
    _log(SKILL__QUEUE, "%s(%u):  Saving skill queue.", name(), m_itemID );
    m_db.SaveSkillQueue( m_itemID, m_skillQueue );
}

void Character::ClearSkillFlags()
{
    std::vector<InventoryItemRef> skills;
    pInventory->GetItemsByFlag(flagSkill, skills);
    for (auto cur : skills)
        cur->SetFlag(flagSkill, false);
}

uint8 Character::GetSPPerMin(Skill* skill)
{
    uint8 primary = GetAttribute(skill->GetAttribute(AttrPrimaryAttribute).get_uint32()).get_uint32();
    uint8 secondary = GetAttribute(skill->GetAttribute(AttrSecondaryAttribute).get_uint32()).get_uint32();
    return EvEMath::Skill::PointsPerMinute(primary, secondary);
}

SkillRef Character::GetCharSkillRef(uint16 skillTypeID) const
{
    InventoryItemRef skill = pInventory->GetByTypeFlag( skillTypeID, flagSkill );
    if (skill.get() == nullptr)
        skill = pInventory->GetByTypeFlag( skillTypeID, flagSkillInTraining );

    return SkillRef::StaticCast( skill );
}

int8 Character::GetSkillLevel(uint16 skillTypeID, bool zeroForNotInjected /*true*/) const {
    SkillRef requiredSkill = GetCharSkillRef( skillTypeID );
    // First, check for existence of skill trained or in training:
    if (requiredSkill.get() == nullptr)
        return (zeroForNotInjected ? 0 : -1);

    return (int8)requiredSkill->GetAttribute(AttrSkillLevel).get_uint32() ;
}

bool Character::HasSkillTrainedToLevel(uint16 skillTypeID, uint8 skillLevel) const {
    SkillRef requiredSkill = GetCharSkillRef( skillTypeID );
    // First, check for existence of skill
    if (requiredSkill.get() == nullptr)
        return false;
    // Second, check for required minimum level of skill, note it must already be trained to this level:
    if (requiredSkill->GetAttribute(AttrSkillLevel) < skillLevel)
        return false;
    return true;
}

PyRep* Character::GetRAMSkills()
{
    /*  this queries RAM skills and is used to display blueprints tab (S&I -> Blueprints)
     *      called by RamProxy::GetRelevantCharSkills()
     *
     *            skillLevels, attributeValues = sm.GetService('manufacturing').GetRelevantCharSkills()
     *            maxManufacturingJobCount = int(attributeValues[const.attributeManufactureSlotLimit])    -AttrManufactureSlotLimit = 196,
     *            maxResearchJobCount = int(attributeValues[const.attributeMaxLaborotorySlots])           -AttrMaxLaborotorySlots = 467,
     *
     *            skillLevels <<  this is a dict of max remote ram jobs
     *            attributeValues  << this is a dict of max ram jobs
     */

    PyDict* skillLevels = new PyDict();
        skillLevels->SetItem(new PyInt(EVEDB::invTypes::ScientificNetworking), new PyInt(GetSkillLevel(EvESkill::ScientificNetworking)));
        skillLevels->SetItem(new PyInt(EVEDB::invTypes::SupplyChainManagement), new PyInt(GetSkillLevel(EvESkill::SupplyChainManagement)));

    uint8 mLab = 1 + GetSkillLevel(EvESkill::LaboratoryOperation) + GetSkillLevel(EvESkill::AdvancedLaboratoryOperation);
    uint8 mSlot = 1 + GetSkillLevel(EvESkill::MassProduction) + GetSkillLevel(EvESkill::AdvancedMassProduction);
    PyDict* attributeValues = new PyDict();
        attributeValues->SetItem(new PyInt(AttrMaxLaborotorySlots), new PyInt(mLab));
        attributeValues->SetItem(new PyInt(AttrManufactureSlotLimit), new PyInt(mSlot));

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, skillLevels);
        tuple->SetItem(1, attributeValues);
    return tuple;
}

int64 Character::GetEndOfTraining() {
    if (m_inTraining == nullptr)
        return 0;

    // sanity check for skill training
    if (m_skillQueue.empty()) {
        sLog.Error("SkillQueue", "%s(%u) flagged as training but queue empty", \
                m_inTraining->name(), m_inTraining->itemID());
        m_inTraining->SetFlag(flagSkill, true);
        m_inTraining = nullptr;
        m_pClient->SetTrainingEndTime(0);
        return 0;
    }

    m_pClient->SetTrainingEndTime(m_skillQueue.front().endTime);
    return m_skillQueue.front().endTime;
}

void Character::RemoveFromQueue(SkillRef sRef)
{
    SkillQueue::iterator itr = m_skillQueue.begin();
    while (itr != m_skillQueue.end()) {
        if (sRef->typeID() == itr->typeID)
            if (sRef->GetAttribute(AttrSkillLevel).get_uint32() >= itr->level)
                itr = m_skillQueue.erase(itr);
    }
    SkillQueueLoop();
}

void Character::ClearSkillQueue(bool update/*false*/)
{
    CancelSkillInTraining(update);
    m_skillQueue.clear();
    _log(SKILL__QUEUE, "%s(%u) Skill Queue Cleared", name(), m_itemID);
}

PyTuple *Character::SendSkillQueue() {
    // get current skill queue
    PyList *list = new PyList();
    for (auto cur : m_skillQueue) {
        SkillQueue_Element el;
        el.typeID = cur.typeID;
        el.level = cur.level;
        list->AddItem( el.Encode() );
    }

    // and encapsulate it in a tuple with the free points
    PyTuple *tuple = new PyTuple(2);
        tuple->SetItem(0, list);
        tuple->SetItem(1, new PyInt(m_freePoints));
    return tuple;
}

uint32 Character::GetTotalSP() {
    // Loop through all skills trained and calculate total SP this character has trained
    //  this will also update charData for current SP
    m_charData.skillPoints = 0;
    std::vector<InventoryItemRef> skills;
    pInventory->GetItemsByFlag(flagSkill, skills);
    for (auto cur : skills)
        m_charData.skillPoints += cur->GetAttribute( AttrSkillPoints ).get_uint32();    // much cleaner and more accurate    -allan

    return m_charData.skillPoints;
}

void Character::SaveSkillHistory(uint16 eventID, double logDate, uint32 characterID, uint16 skillTypeID, uint8 skillLevel, uint32 absolutePoints)
{
    if (absolutePoints < 1)
        return;
    if (logDate < 1)
        return;
    if (!sDataMgr.IsSkillTypeID(skillTypeID))
        return;
    m_db.SaveSkillHistory(eventID, logDate, characterID, skillTypeID, skillLevel, absolutePoints);
}

uint8 Character::InjectSkillIntoBrain(SkillRef skill) {
    if (m_pClient == nullptr)
        return 0;

    // returns
    // 1=success, 2=prereqs, 3=already known, 4=split fail, 5=load fail

    SkillRef oldSkill(GetCharSkillRef(skill->typeID()));
    if (oldSkill.get() != nullptr) {
        /** @todo: build and send proper UserError for CharacterAlreadyKnowsSkill. */
        m_pClient->SendNotifyMsg("You already know this skill.");
        return 3;
    }

    /** @todo config option (later) to check if another character on this account is training a skill.
     *  If so, send error, cancel inject and return. (flagID=61).
     */

    if (!skill->SkillPrereqsComplete(*this)) {
        /** @todo need to send back a response to the client.  need packet specs. */
        _log(SKILL__DEBUG, "%s(%u): Requested to inject %s (%u/%u) but prereq not complete.", \
                name(), m_itemID, skill->name(), skill->typeID(), skill->itemID());
        m_pClient->SendNotifyMsg("Injection failed.  Skill prerequisites incomplete.");
        return 2;
    }

    // are we injecting from a stack of skills?
    if (skill->quantity() > 1) {
        // split the stack to obtain single item
        skill = SkillRef::StaticCast(skill->Split(1));
        if (skill.get() == nullptr ) {
            _log(ITEM__ERROR, "%s (%u): Unable to split stack of %s (%u).", name(), m_itemID, skill->name(), skill->itemID());
            return 4;
        }
    }

    skill->ChangeSingleton(true);
    skill->Move(m_itemID, flagSkill, true);
    skill->SetAttribute(AttrSkillPoints, EvilZero);
    skill->SetAttribute(AttrSkillLevel, EvilZero);

    skill->VerifyAttribs();

    // 'EvESkill::Event::SkillInjected' shows as "Unknown" in PD>Skill>History
    SaveSkillHistory(EvESkill::Event::SkillInjected, GetFileTimeNow(), m_itemID, skill->typeID(), 0, 0);
    _log(SKILL__MESSAGE, "%s(%u) has injected %s(%u)", name(), m_itemID, skill->name(), skill->itemID());

    return 1;
}

void Character::PauseSkillQueue() {
    m_db.SavePausedSkillQueue(m_itemID, m_skillQueue);

    CancelSkillInTraining(true);

    m_skillQueue.clear();
    UpdateSkillQueueEndTime();
    _log(SKILL__QUEUE, "%s(%u)  Skill Queue Paused", name(), m_itemID);
}

void Character::LoadPausedSkillQueue(uint16 typeID)
{
    m_db.LoadPausedSkillQueue(m_itemID, m_skillQueue);

    if (m_skillQueue.empty())
        return;

    Skill* skill(GetCharSkillRef(typeID).get());
    if (typeID != m_skillQueue.front().typeID) {
        // skill to start != first skill in queue...do we just start removing skills till we find this typeID?
        _log(SKILL__WARNING, "LoadPausedSkillQueue() - type sent (%u) does not match first in queue (%u)",
             typeID, m_skillQueue.front().typeID);
    }

    // queue was paused.  all endTimes are off, so reset using now as start time for first skill.
    uint8 nextLvl(0);
    uint32 currentSP(0), nextSP(0);
    int64 startTime(GetFileTimeNow());
    for (SkillQueue::iterator itr = m_skillQueue.begin(); itr != m_skillQueue.end(); ++itr) {
        skill = GetCharSkillRef(itr->typeID).get();
        if (skill == nullptr)
            continue;
        nextLvl = skill->GetAttribute(AttrSkillLevel).get_uint32() + 1;
        if (nextLvl > EvESkill::MAXSKILLLEVEL)
            nextLvl = EvESkill::MAXSKILLLEVEL;

        if (itr->level == nextLvl) {
            currentSP = skill->GetCurrentSP(this, startTime);
        } else {
            currentSP = skill->GetSPForLevel(itr->level - 1);
        }

        nextSP = skill->GetSPForLevel(itr->level);
        itr->startTime = startTime;
        itr->endTime = EvEMath::Skill::EndTime(currentSP, nextSP, GetSPPerMin(skill), startTime);
        skill->SaveItem();
        startTime = itr->endTime;
    }

    // get first skill, add start history and send begin training packet
    skill = GetCharSkillRef(m_skillQueue.front().typeID).get();
    skill->SetFlag(flagSkillInTraining, true);
    skill->SaveItem();

    OnSkillStartTraining osst;
        osst.itemID = skill->itemID();
        osst.endOfTraining = m_skillQueue.front().endTime;
    PyTuple* tmp = osst.Encode();
    m_pClient->QueueDestinyEvent(&tmp);

    //  do we save/show cancel/restart training for pause/resume?
    SaveSkillHistory(EvESkill::Event::TrainingStarted, GetFileTimeNow(), m_itemID, skill->itemID(), m_skillQueue.front().level, skill->GetSPForLevel(m_skillQueue.front().level));

    UpdateSkillQueueEndTime();

    _log(SKILL__QUEUE, "%s(%u)  Paused Skill Queue Loaded and restarted", name(), m_itemID);
}

void Character::CancelSkillInTraining(bool update/*false*/)
{
    if (m_inTraining == nullptr)
        return; //nothing to do...

    if (m_skillQueue.empty())
        return;

    QueuedSkill qs = m_skillQueue.front();
    uint8 nextLvl(m_inTraining->GetAttribute(AttrSkillLevel).get_uint32() + 1);
    if (nextLvl > EvESkill::MAXSKILLLEVEL)
        nextLvl = EvESkill::MAXSKILLLEVEL;

    int64 curTime(GetFileTimeNow());
    uint32 currentSP(0);

    // is skill in training first in queue?
    if (qs.typeID != m_inTraining->typeID()) {
        // nope
        // not sure how to do this correctly...
        currentSP = m_inTraining->GetCurrentSP(this);
        SaveSkillHistory(EvESkill::Event::TrainingCanceled, curTime, m_itemID, m_inTraining->typeID(), nextLvl, currentSP);
        // send training stopped packet
        if (update) {
            OnSkillTrainingStopped osst;
                osst.itemID = m_inTraining->itemID();
                osst.silent = true;    // silent means 'disable neocom blink event'
            PyTuple* tmp = osst.Encode();
            m_pClient->QueueDestinyEvent(&tmp);
        }

        if (is_log_enabled(SKILL__INFO))
            _log(SKILL__INFO, "%s:%s(%u/%u) Training canceled from skill not in queue.", \
                    name(), m_inTraining->name(), m_inTraining->typeID(), m_inTraining->itemID());

        m_inTraining->SetAttribute(AttrSkillPoints, currentSP, update);
        m_inTraining->SetFlag(flagSkill, update);
        m_inTraining->SaveItem();
        m_inTraining = nullptr;
        // either way, training canceled.
        return;
    }

    // at this point, skill in training is first in queue
    currentSP = m_inTraining->GetCurrentSP(this, qs.startTime);
    if ((qs.endTime < curTime) or (currentSP >= m_inTraining->GetSPForLevel(nextLvl))) {
        // this level has completed
        // send training complete packet
        if (update) {
            OnSkillTrained ost;
                ost.itemID = m_inTraining->itemID();
            PyTuple* tmp = ost.Encode();
            m_pClient->QueueDestinyEvent(&tmp);
        }

        // update attribs and save
        m_inTraining->SetAttribute(AttrSkillLevel, nextLvl, false);
        currentSP = m_inTraining->GetSPForLevel(nextLvl);
        SaveSkillHistory(EvESkill::Event::TrainingComplete, qs.endTime, m_itemID, m_inTraining->typeID(), nextLvl, currentSP);

        if (is_log_enabled(SKILL__INFO))
            _log(SKILL__INFO, "%s:%s(%u/%u) - CancelSkillInTraining - Training to level %u completed.", \
                    name(), m_inTraining->name(), m_inTraining->typeID(), m_inTraining->itemID(), nextLvl);
    } else {
        SaveSkillHistory(EvESkill::Event::TrainingCanceled, curTime, m_itemID, m_inTraining->typeID(), nextLvl, currentSP);
        // send training stopped packet
        if (update) {
            OnSkillTrainingStopped osst;
                osst.itemID = m_inTraining->itemID();
                osst.silent = true;    // silent means 'disable neocom blink event'
            PyTuple* tmp = osst.Encode();
            m_pClient->QueueDestinyEvent(&tmp);
        }

        if (is_log_enabled(SKILL__INFO))
            _log(SKILL__INFO, "%s:%s(%u/%u) - Training to level %u canceled.  CurrentSP: %u, nextSP: %u", \
                    name(), m_inTraining->name(), m_inTraining->typeID(), m_inTraining->itemID(), nextLvl, currentSP, m_inTraining->GetSPForLevel(nextLvl));
    }

    m_inTraining->SetAttribute(AttrSkillPoints, currentSP, update);
    m_inTraining->SetFlag(flagSkill, update);
    m_inTraining->SaveItem();

    // remove from queue, if applicable
    if (!m_skillQueue.empty())
        if (m_inTraining->typeID() == m_skillQueue.front().typeID)
            m_skillQueue.erase( m_skillQueue.begin() );

    m_inTraining = nullptr;
}

void Character::AddToSkillQueue(uint16 typeID, uint8 level) {
    Skill* skill(GetCharSkillRef(typeID).get());
    if (skill == nullptr) {
        //  skill not found.  cancel and return
        _log(SKILL__QUEUE, "Cannot find Skill %u.", typeID);
        m_pClient->SendErrorMsg("Cannot find skill to train.  Ref: ServerError 60723.");
        return;
    }

    _log(SKILL__INFO, "Starting checks to add %s to training queue.", skill->name());

    uint8 nextLvl(skill->GetAttribute(AttrSkillLevel).get_uint32() + 1);
    if (nextLvl > EvESkill::MAXSKILLLEVEL)
        nextLvl = EvESkill::MAXSKILLLEVEL;

    int64 curTime(GetFileTimeNow());
    uint32 currentSP(skill->GetCurrentSP(this)), nextSP(skill->GetSPForLevel(level));
    if (level < nextLvl) {
        // level to train is below current level. update client data and return.
        SaveSkillHistory(EvESkill::Event::TaskMaster, curTime, m_itemID, typeID, level, currentSP);
        skill->SetFlag(flagSkill, true);
        skill->SaveItem();

        OnSkillTrained ost;
            ost.itemID = skill->itemID();
        PyTuple* tmp = ost.Encode();
        m_pClient->QueueDestinyEvent(&tmp);

        _log(SKILL__WARNING, "Trying to add level %u but current level is %u.", level, nextLvl -1);
        return;
    }

    if (level > nextLvl)
        currentSP = skill->GetSPForLevel(level -1);

    // verify sp is below next level
    if (currentSP >= nextSP) {
        // it's not.  update client data and return.
        SaveSkillHistory(EvESkill::Event::TaskMaster, curTime, m_itemID, typeID, level, currentSP);
        skill->SetFlag(flagSkill, true);
        skill->SaveItem();

        OnSkillTrained ost;
            ost.itemID = skill->itemID();
        PyTuple* tmp = ost.Encode();
        m_pClient->QueueDestinyEvent(&tmp);

        _log(SKILL__WARNING, "Trying to add level %u at %u sp but current sp is %u.", level, nextSP, currentSP);
        return;
    }

    // current level trainable
    QueuedSkill qs = QueuedSkill();
        qs.typeID = typeID;
        qs.level = level;

    if (m_skillQueue.empty()) {
        // nothing in queue.  begin training this skill
        skill->SetFlag(flagSkillInTraining, true);
        m_inTraining = skill;
        qs.startTime = curTime;
        qs.endTime = EvEMath::Skill::EndTime(currentSP, nextSP, GetSPPerMin(skill), curTime);
        SaveSkillHistory(EvESkill::Event::TrainingStarted, curTime, m_itemID, typeID, level, nextSP);

        OnSkillStartTraining osst;
            osst.itemID = skill->itemID();
            osst.endOfTraining = qs.endTime;
        PyTuple* tmp = osst.Encode();
        m_pClient->QueueDestinyEvent(&tmp);
    } else {
        qs.startTime = m_skillQueue.back().endTime +EvE::Time::Second;
        qs.endTime = EvEMath::Skill::EndTime(currentSP, nextSP, GetSPPerMin(skill), qs.startTime);
    }

    // add to queue and save
    m_skillQueue.push_back( qs );
    skill->SaveItem();

    float timeLeft = (qs.endTime - qs.startTime) / EvE::Time::Second;
    const char* formatedTime = EvE::FormatTime(timeLeft);
    _log(SKILL__QUEUE, "Added %s Level %u to queue with %s(%.1f) to train %uSP.", \
            skill->name(), level, formatedTime, timeLeft, nextSP - currentSP);
}

void Character::UpdateSkillQueue() {
    /* cleaned up code and reworked logic  -allan 28Apr16   -- revisited 23Mar17  --updated code, logic and timers 16Nov17  -again 9jan18*/
    // finally fixed.  22Jan18  --it wasnt 24Jan18     still not right 3Dec18    -again 4jan19      -another 6Aug19
    // rewrote and FINALLY fixed.  10Aug19   --still wrong 23Aug20
    // complete revamp and rename, this kept for notes  -allan 12Oct20
}

void Character::SkillQueueLoop(bool update/*true*/)
{
    double begin(GetTimeMSeconds());
    _log(SKILL__QUEUE, "%s(%u) calling SkillQueueLoop() - update %s", name(), m_itemID, update?"true":"false");

    // anything to do here?
    if (m_skillQueue.empty())
        if (m_inTraining == nullptr)
            return;

    if (m_inTraining->typeID() != m_skillQueue.front().typeID)
        CancelSkillInTraining(update);

    // at this point, there is a skill in training, and it is front of queue
    int64 curTime(GetFileTimeNow());
    if (m_skillQueue.front().endTime > curTime) {
        float timeLeft = (m_skillQueue.front().endTime - curTime) / EvE::Time::Second;
        const char* formatedTime = EvE::FormatTime(timeLeft);
        _log(SKILL__INFO, "%s still training.  %s remaining.", m_inTraining->name(), formatedTime);
        UpdateSkillQueueEndTime();
        return;
    }

    PyList* list = new PyList();
    bool sent(false), multiple(false);
    Skill* skill(nullptr);
    while (!m_skillQueue.empty()) {
        QueuedSkill qs = m_skillQueue.front();
        skill = GetCharSkillRef( qs.typeID ).get();
        if ((qs.typeID == 0) or (skill == nullptr)) {
            _log( SKILL__WARNING, "SkillID %u to train was not found.  Erase and continue.", qs.typeID);
            m_skillQueue.erase( m_skillQueue.begin() );
            continue;
        }

        _log( SKILL__INFO, "Starting checks for %s.", skill->name());

        if (qs.endTime == 0) {
            // this should not hit at this point.
            _log(SKILL__ERROR, "endTime wasnt set.  Erase from queue and continue.");
            skill->SetFlag(flagSkill, true);
            skill->SaveItem();
            m_skillQueue.erase( m_skillQueue.begin() );
            skill = nullptr;
            m_inTraining = nullptr;
            continue;
        }

        if (qs.endTime < curTime) {
            //  skill training has completed.
            uint32 currentSP = skill->GetSPForLevel(qs.level);
            SaveSkillHistory(EvESkill::Event::QueueTrainingCompleted, qs.endTime, m_itemID, skill->typeID(), qs.level, currentSP);

            if (is_log_enabled(SKILL__INFO))
                _log(SKILL__INFO, "Queued Training completed for level: %u", qs.level);

            // update attribs and save
            skill->SetAttribute(AttrSkillLevel, qs.level, update);
            skill->SetAttribute(AttrSkillPoints, currentSP, update);
            skill->SetFlag(flagSkill, update);
            skill->SaveItem();

            // remove completed skill level from queue
            m_skillQueue.erase( m_skillQueue.begin() );

            // notify client
            if (update) {
                PyTuple* tmp(nullptr);
                if (m_skillQueue.empty()) {
                    // only one skill to update
                    OnSkillTrained ost;
                        ost.itemID = skill->itemID();
                    tmp = ost.Encode();
                } else {
                    // another skill in the works.  send combined update
                    SkillRef sref = GetCharSkillRef(m_skillQueue.front().typeID);
                    if (sref.get() == nullptr) {
                        // that shit didnt work...revert to multiple packets
                        OnSkillTrained ost;
                            ost.itemID = skill->itemID();
                        tmp = ost.Encode();
                    } else {
                        multiple = true;
                        //  queue skill switching uses the following to reduce data sent
                        //def OnSkillSwitched(self, oldSkillID, newSkillID, ETA):
                        OnSkillSwitched oss;
                            oss.oldSkillID = skill->itemID();
                            oss.newSkillID = sref->itemID();
                            oss.ETA = m_skillQueue.front().endTime;
                        tmp = oss.Encode();
                    }
                }
                m_pClient->QueueDestinyEvent(&tmp);
            } else if (m_pClient->IsLogin())
                // for login, use OnMultipleSkillsTrained[]
                list->AddItemInt(skill->itemID());

            if (m_pClient->IsInSpace() and update) {
                switch (skill->groupID()) {
                    case EVEDB::invGroups::Trade:
                    case EVEDB::invGroups::Social:
                    case EVEDB::invGroups::Planet_Management:
                    case EVEDB::invGroups::Corporation_Management: {
                        ;   // do nothing for these.
                    } break;
                    default: {
                        if (!sent) {
                            sent = true;
                            m_pClient->SendInfoModalMsg("Completed Skill Training.<br>Your ship will update to the new skill level the next time you undock.");
                        }
                    } break;
                }
            }

            // clear variables and continue
            skill = nullptr;
            m_inTraining = nullptr;
            continue;
        }

        // at this point, there is at least one skill queued, and nothing currently training.
        uint32 nextSP(skill->GetSPForLevel(qs.level));
        SaveSkillHistory(EvESkill::Event::TrainingStarted, qs.startTime, m_itemID, qs.typeID, qs.level, nextSP);
        skill->SetFlag(flagSkillInTraining, true);
        skill->SaveItem();
        m_inTraining = skill;

        if (is_log_enabled(SKILL__INFO)) {
            float timeLeft = (qs.endTime - curTime) / EvE::Time::Second;
            const char* formatedTime = EvE::FormatTime(timeLeft);
            _log(SKILL__INFO, "Training started.  %s to train %u sp for level %u", \
                    formatedTime, nextSP - skill->GetCurrentSP(this, qs.startTime), qs.level);
        }

        if (!multiple) {
            OnSkillStartTraining osst;
                osst.itemID = skill->itemID();
                osst.endOfTraining = qs.endTime;
            PyTuple* tmp = osst.Encode();
            m_pClient->QueueDestinyEvent(&tmp);
        }

        break;
    }

    UpdateSkillQueueEndTime();
    SaveCharacter();

    if (m_pClient->IsLogin()) {
        PyTuple* tmp(nullptr);
        if (!list->empty()) {
            if (list->size() > 1) {
                OnMultipleSkillsTrained omst;
                    omst.skillList = list;
                tmp = omst.Encode();
            }
        } else if (skill != nullptr) {
            OnSkillTrained ost;
                ost.itemID = skill->itemID();
            tmp = ost.Encode();
        }

        m_pClient->QueueDestinyEvent(&tmp);
    }

    _log(SKILL__QUEUE, "SkillQueueLoop() completed in %.4fms", (GetTimeMSeconds() - begin));
}

void Character::UpdateSkillQueueEndTime()
{
    if (m_skillQueue.empty()) {
        m_db.UpdateSkillQueueEndTime(0, m_itemID);
        m_pClient->SetTrainingEndTime(0);
        if (is_log_enabled(SKILL__TRACE))
            _log(SKILL__QUEUE, "%s(%u):  UpdateSkillQueueEndTime() - Queue is empty.", name(), m_itemID);
        return;
    }

    // update client timer check for skill in training
    m_pClient->SetTrainingEndTime(m_skillQueue.front().endTime);
    m_db.UpdateSkillQueueEndTime(m_skillQueue.back().endTime, m_itemID);

    SaveSkillQueue();
}

PyDict *Character::GetCharInfo() {
    // this is char, skills, implants, boosters.
    if (!pInventory->ContentsLoaded())
        if (!pInventory->LoadContents()) {
            codelog(CHARACTER__ERROR, "%s (%u): Failed to load contents for GetCharInfo", name(), m_itemID);
            return nullptr;
        }

    Rsp_CommonGetInfo_Entry entry1;
    if (!Populate(entry1))
        return nullptr;

    PyDict *result = new PyDict();
    result->SetItem(new PyInt(m_itemID), new PyObject("util.KeyVal", entry1.Encode()));

    //now encode skills...
    std::vector<InventoryItemRef> skills;
    skills.clear();
    //find all the skills contained within ourself.
    pInventory->GetItemsByFlag(flagSkill, skills );
    pInventory->GetItemsByFlag(flagSkillInTraining, skills );

    /** @todo  get implants and boosters here once implemented */

    //encode an entry for each one.
    for (auto cur : skills) {
        Rsp_CommonGetInfo_Entry entry;
        if (cur->Populate(entry)) {
            result->SetItem(new PyInt(cur->itemID()), new PyObject("util.KeyVal", entry.Encode()));
        } else {
            codelog(CHARACTER__ERROR, "%s (%u): Failed to load character item %u for GetCharInfo", name(), m_itemID, cur->itemID());
        }
    }

    /** @todo i dont know how boosters and implants work yet, so may have to set item different for them.  */

    return result;
}

PyObject *Character::GetDescription() const {
    util_Row row;
        row.header.push_back("description");
        row.line = new PyList();
        row.line->AddItemString( description().c_str() );
    return row.Encode();
}

void Character::AddItem(InventoryItemRef iRef)
{
    if (iRef.get() == nullptr)
        return;

    InventoryItem::AddItem(iRef);

    _log( CHARACTER__INFO, "%s(%u) has been added to %s with flag %i.", iRef->name(), iRef->itemID(), name(), (uint8)iRef->flag() );
}

void Character::SetActiveShip(uint32 shipID)
{
    m_charData.shipID = shipID;
    m_db.SetCurrentShip(m_itemID, shipID);
}

void Character::SetActivePod(uint32 podID)
{
    m_charData.capsuleID = podID;
    m_db.SetCurrentPod(m_itemID, podID);
}

void Character::ResetClone()
{
    m_db.ChangeCloneType(m_itemID, itemCloneAlpha);
}

void Character::SaveCharacter() {
    _log( CHARACTER__INFO, "Saving character info for %u.", m_itemID );

    // update skill points before save
    GetTotalSP();
    SetLogonMinutes();
    m_db.SaveCharacter(m_itemID, m_charData);
}

void Character::SaveFullCharacter() {
    _log( CHARACTER__INFO, "Saving full character info for %u.", m_itemID );
    //GetTotalSP();
    SaveCharacter();
    m_db.SaveCorpData(m_itemID, m_corpData);
    SaveAttributes();

    if (m_inTraining != nullptr) {
        m_inTraining->SetAttribute(AttrSkillPoints, m_inTraining->GetCurrentSP(this, m_skillQueue.front().startTime), false);
        m_inTraining->SaveItem();
    }

    SaveSkillQueue();
}

void Character::PayBounty(CharacterRef cRef) {
    std::string reason = "Bounty for the killing of ";
    reason += cRef->itemName();
    AccountService::TransferFunds(corpCONCORD, m_itemID, cRef->bounty(), reason, Journal::EntryType::Bounty, cRef->itemID());
    // add data to StatisticMgr
    sStatMgr.Add(Stat::pcBounties, cRef->bounty());
}

void Character::SetLoginTime() {
    m_loginTime = sEntityList.GetStamp();
    m_db.SetLogInTime(m_itemID);
}

uint16 Character::OnlineTime()
{
    double onlineTime = m_charData.loginTime - GetFileTimeNow();
    onlineTime /= 10000000;
    onlineTime -= 11644473600;
    onlineTime /= 60;
    return (uint16)onlineTime;
}

// called on 10m timer from client
void Character::SetLogonMinutes() {
    //  get login time and set _logonMinutes       -allan
    uint16 loginMinutes = (sEntityList.GetStamp() - m_loginTime) /60;

    // some checks are done < 1m, so if this check has no minutes, keep original time and exit
    if (loginMinutes > 0) {
        m_charData.logonMinutes += loginMinutes;
        m_loginTime = sEntityList.GetStamp();
    }
}

// certificate system
bool Character::HasCertificate( uint32 certID ) const {
    CertMap::const_iterator itr = m_certificates.find(certID);
    return (itr != m_certificates.end());
}

void Character::GetCertificates( CertMap &crt ) {
    crt = m_certificates;
}

void Character::GrantCertificate( uint32 certID )
{
    CharCerts cert = CharCerts();
        cert.certificateID = certID;
        cert.grantDate = GetFileTimeNow();
        cert.visibilityFlags = 0;
    m_certificates.emplace(certID, cert);
    m_cdb.AddCertificate(m_itemID, cert);
}

void Character::UpdateCertificate( uint32 certID, bool pub ) {
    m_cdb.UpdateCertificate(m_itemID, certID, pub);
}

void Character::SaveCertificates() {
    _log( CHARACTER__INFO, "Saving Certificates of character %u", m_itemID );
    m_cdb.SaveCertificates( m_itemID, m_certificates );
}

// functions and methods for bookmark system (char mem maps)
/** @todo this will need more thought/work   */
void Character::LoadBookmarks()
{
    // not used yet
}

void Character::SaveBookMarks()
{
    // not used yet
}


// functions and methods for standings system
/** @todo  this needs to be moved to common standings code */
/** @todo  these need to use common standings methods for formulas  */
/** @todo  these need secStatus tests and calcs for negative secStatus */
float Character::GetStandingModified(uint32 fromID, uint32 toID)
{
    if (toID == 0)
        toID = m_itemID;
    float res = StandingDB::GetStanding(fromID, toID);
    if (res < 0.0f) {
        res += ((10.0f + res) * (0.04f * GetSkillLevel(EvESkill::Diplomacy)));
    } else {
        res += ((10.0f - res) * (0.04f * GetSkillLevel(EvESkill::Connections)));
    }
    return res;
}

float Character::GetNPCCorpStanding(uint32 fromID, uint32 toID) {
    if (toID == 0)
        toID = m_itemID;
    float res = StandingDB::GetStanding(fromID, toID);
    if (res < 0.0f) {
        res += ((10.0f + res) * (0.04f * GetSkillLevel(EvESkill::Diplomacy)));
    } else {
        res += ((10.0f - res) * (0.04f * GetSkillLevel(EvESkill::Connections)));
    }
    return res;
}

void Character::SetStanding(uint32 fromID, uint32 toID, float standing) {
    StandingDB::SetStanding(fromID, toID, standing);
    PyTuple* payload = new PyTuple(0);
    m_pClient->SendNotification("OnStandingSet", "charid", payload, false);
}

// for map system
void Character::VisitSystem(uint32 solarSystemID) {
	m_db.VisitSystem(solarSystemID, m_itemID);
}
