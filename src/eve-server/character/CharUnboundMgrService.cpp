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
    Author:     caytchen, Zhur
    Updates:        Allan
*/

#include "eve-server.h"
//#include "../../eve-common/EVE_Skills.h"

#include "EntityList.h"

#include "account/AccountService.h"
#include "cache/ObjCacheService.h"
#include "character/CharUnboundMgrService.h"
#include "imageserver/ImageServer.h"
#include "station/StationDataMgr.h"

CharUnboundMgrService::CharUnboundMgrService(EVEServiceManager& mgr) :
    Service("charUnboundMgr")
{
    this->m_cache = mgr.Lookup <ObjCacheService>("objectCaching");

    this->Add("SelectCharacterID", static_cast <PyResult (CharUnboundMgrService::*) (PyCallArgs&, PyInt*, std::optional <PyInt*>, std::optional <PyInt*>)> (&CharUnboundMgrService::SelectCharacterID));
    this->Add("SelectCharacterID", static_cast <PyResult (CharUnboundMgrService::*) (PyCallArgs&, PyInt*, std::optional <PyBool*>, std::optional <PyInt*>)> (&CharUnboundMgrService::SelectCharacterID));
    this->Add("GetCharacterToSelect", &CharUnboundMgrService::GetCharacterToSelect);
    this->Add("GetCharactersToSelect", &CharUnboundMgrService::GetCharactersToSelect);
    this->Add("GetCharacterInfo", &CharUnboundMgrService::GetCharacterInfo);
    this->Add("IsUserReceivingCharacter", &CharUnboundMgrService::IsUserReceivingCharacter);
    this->Add("DeleteCharacter", &CharUnboundMgrService::DeleteCharacter);
    this->Add("PrepareCharacterForDelete", &CharUnboundMgrService::PrepareCharacterForDelete);
    this->Add("CancelCharacterDeletePrepare", &CharUnboundMgrService::CancelCharacterDeletePrepare);
    this->Add("ValidateNameEx", &CharUnboundMgrService::ValidateNameEx);
    this->Add("GetCharCreationInfo", &CharUnboundMgrService::GetCharCreationInfo);
    this->Add("GetCharNewExtraCreationInfo", &CharUnboundMgrService::GetCharNewExtraCreationInfo);
    this->Add("CreateCharacterWithDoll", &CharUnboundMgrService::CreateCharacterWithDoll);
}

PyResult CharUnboundMgrService::ValidateNameEx(PyCallArgs &call, PyRep* name)
{
    return CharacterDB::ValidateCharNameRep(PyRep::StringContent(name));
}

PyResult CharUnboundMgrService::GetCharacterToSelect(PyCallArgs &call, PyInt* characterID)
{
    return CharacterDB::GetCharSelectInfo(characterID->value());
}

PyResult CharUnboundMgrService::GetCharactersToSelect(PyCallArgs &call)
{
    return CharacterDB::GetCharacterList(call.client->GetUserID());
}

PyResult CharUnboundMgrService::DeleteCharacter(PyCallArgs &call, PyInt* characterID)
{
    CharacterDB::DeleteCharacter(characterID->value());
    return nullptr;
}

PyResult CharUnboundMgrService::PrepareCharacterForDelete(PyCallArgs &call, PyInt* characterID)
{
    return new PyLong(CharacterDB::PrepareCharacterForDelete(call.client->GetUserID(), characterID->value()));
}

PyResult CharUnboundMgrService::CancelCharacterDeletePrepare(PyCallArgs &call, PyInt* characterID)
{
    CharacterDB::CancelCharacterDeletePrepare(call.client->GetUserID(), characterID->value());
    return nullptr;
}

PyResult CharUnboundMgrService::IsUserReceivingCharacter(PyCallArgs &call) {
    /*  this is called when selecting the 3ed slot when there are 2 chars on account already.
     * returning true will disable creating a 3ed character.
     * returning false will allow creating a 3ed character.
     */
    if (sConfig.character.allow3edChar)
        return PyStatic.NewFalse();
    return PyStatic.NewTrue();
}

//  called from petitioner (but only when session.characterID is None)
PyResult CharUnboundMgrService::GetCharacterInfo(PyCallArgs &call) {
    // chars = sm.RemoteSvc('charUnboundMgr').GetCharacterInfo()
    _log(CLIENT__ERROR, "Called GetCharacterInfo");
    //   characterID, characterName

    return nullptr;
}

PyResult CharUnboundMgrService::GetCharCreationInfo(PyCallArgs &call) {
    PyDict *result = new PyDict();
    //send all the cache hints needed for char creation.
    this->m_cache->InsertCacheHints(ObjCacheService::hCharCreateCachables, result);
    _log(CLIENT__MESSAGE, "Sending char creation info reply");
    return result;
}

PyResult CharUnboundMgrService::GetCharNewExtraCreationInfo(PyCallArgs &call) {
    PyDict *result = new PyDict();
    this->m_cache->InsertCacheHints(ObjCacheService::hCharCreateNewExtraCachables, result);
    _log(CLIENT__MESSAGE, "Sending char new extra creation info reply");
    return result;
}

PyResult CharUnboundMgrService::SelectCharacterID(PyCallArgs& call, PyInt* characterID, std::optional <PyBool*> loadDungeon, std::optional <PyInt*> secondChoiceID) {
    PyInt* loadDungeons = new PyInt (loadDungeon.has_value () ? loadDungeon.value()->value() : 0);

    return this->SelectCharacterID (call, characterID, loadDungeons, secondChoiceID);
}

PyResult CharUnboundMgrService::SelectCharacterID(PyCallArgs &call, PyInt* characterID, std::optional <PyInt*> loadDungeon, std::optional <PyInt*> secondChoiceID)
{
    if (!IsCharacterID(characterID->value())) {
        sLog.Error("Client::SelectCharacter()", "CharacterID %u invalid.", characterID->value());
        call.client->SendErrorMsg("Character ID %u invalid.  Ref: ServerError 00522", characterID->value());
        return nullptr;
    }

    call.client->SelectCharacter(characterID->value());
    return nullptr;
}

PyResult CharUnboundMgrService::CreateCharacterWithDoll(PyCallArgs &call, PyRep* characterName, PyInt* bloodlineID, PyInt* genderID, PyInt* ancestryID, PyObject* characterInfo, PyObject* portraitInfo, PyInt* schoolID) {
    // charID = sm.RemoteSvc('charUnboundMgr').CreateCharacterWithDoll(charactername, bloodlineID, genderID, ancestryID, charInfo, portraitInfo, schoolID)
    // ensure the PyObject* is an util.KeyVal, this might benefit from some helper methods instead of directly using the PyObject
    if (characterInfo->type()->content() != "util.KeyVal" || characterInfo->arguments()->IsDict() == false) {
        codelog(SERVICE__ERROR, "%s: Failed to decode characterInfo", GetName());
        return PyStatic.NewZero();
    }

    // ensure the PyObject* is an util.KeyVal, this might benefit from some helper methods instead of directly using the PyObject
    if (portraitInfo->type()->content() != "util.KeyVal" || portraitInfo->arguments()->IsDict() == false) {
        codelog(SERVICE__ERROR, "%s: Failed to decode portraitInfo", GetName());
        return PyStatic.NewZero();
    }

    PyDict* charInfoData = characterInfo->arguments()->AsDict();
    PyDict* portraitInfoData = portraitInfo->arguments()->AsDict();
    // check name and throw on failure before we get too far in this
    CharacterDB::ValidateCharName(PyRep::StringContent(characterName));

    Client* pClient = call.client;
    pClient->CreateChar(true);

    if (!pClient->RecPic())
        pClient->SendInfoModalMsg("The Portrait for this character was not received.  Your character will still be created, but the server will not have their picture.");

    _log(CLIENT__MESSAGE, "Calling CreateCharacterWithDoll with schoolID: %i bloodlineID: %i gender: %s ancestryID: %i", \
                        schoolID->value(), bloodlineID->value(), genderID->value() == 1 ? "male" : "female", ancestryID->value());

    // obtain character type
    const CharacterType *char_type = sItemFactory.GetCharacterTypeByBloodline(bloodlineID->value());
    if (char_type == nullptr)
        return PyStatic.NewZero();

    // we need to fill these to successfully create character item
    CharacterData cdata = CharacterData();
        cdata.accountID = pClient->GetUserID();
        cdata.gender = genderID->value();
        cdata.ancestryID = ancestryID->value();
        cdata.bloodlineID = bloodlineID->value();
        cdata.schoolID = schoolID->value();
        cdata.description = "Character Created on ";
        cdata.description += currentDateTime();
        cdata.securityRating = sConfig.character.startSecRating;
        cdata.title = "No Title";
        cdata.createDateTime = (int64)GetFileTimeNow();

    //Set the character's career and race based on the school they picked.
    if (CharacterDB::GetCareerBySchool(cdata.schoolID, cdata.raceID, cdata.careerID)) {
        //  The Specialization has been taken out in Crucible.  set to same as Career (default)
        cdata.careerSpecialityID = cdata.careerID;
    } else {
        _log(CLIENT__MESSAGE, "Could not find default School ID %u. Using Caldari Military.", cdata.schoolID);
        cdata.raceID = 1;
        cdata.careerID = 11;
        cdata.careerSpecialityID = 11;
    }

    // Variables for storing attribute bonuses
    uint8 intelligence = char_type->intelligence();
    uint8 charisma = char_type->charisma();
    uint8 perception = char_type->perception();
    uint8 memory = char_type->memory();
    uint8 willpower = char_type->willpower();

    CorpData corpData = CorpData();
        corpData.startDateTime = cdata.createDateTime;
        corpData.corpRole = Corp::Role::Member;
        corpData.corpAccountKey = Account::KeyType::Cash;
        corpData.rolesAtAll = Corp::Role::Member;
        corpData.rolesAtBase = Corp::Role::Member;
        corpData.rolesAtHQ = Corp::Role::Member;
        corpData.rolesAtOther = Corp::Role::Member;
        corpData.grantableRoles = Corp::Role::Member;
        corpData.grantableRolesAtBase = Corp::Role::Member;
        corpData.grantableRolesAtHQ = Corp::Role::Member;
        corpData.grantableRolesAtOther = Corp::Role::Member;

    bool defCorp = true;
    if (sConfig.character.startCorporation) { // Skip if 0
        if ( CorporationDB::DoesCorporationExist( sConfig.character.startCorporation ) ) {
            corpData.corporationID = sConfig.character.startCorporation;
            defCorp = false;
        } else {
            _log(CLIENT__MESSAGE, "Could not find default Corporation ID %u. Using Career Defaults instead.", sConfig.character.startCorporation);
        }
    }
    if (defCorp) {
        if (!CorporationDB::GetCorporationBySchool(cdata.schoolID, corpData.corporationID))
            _log(CLIENT__MESSAGE, "Could not place character in default corporation for school.");
    }

    // Setting character's default starting position, and getting the location...
    // this also sets schoolID and corporationID based on career
    CorporationDB::GetLocationCorporationByCareer(cdata, corpData.corporationID);

    if (sDataMgr.IsStation(sConfig.character.startStation)) { // Skip if 0
        cdata.stationID = sConfig.character.startStation;
        StationData sData = StationData();
        stDataMgr.GetStationData(cdata.stationID, sData);
        cdata.solarSystemID = sData.systemID;
        cdata.constellationID = sData.constellationID;
        cdata.regionID = sData.regionID;
    }

    corpData.baseID = cdata.stationID;

    cdata.typeID = char_type->id();
    cdata.name = PyRep::StringContent(characterName);
    cdata.locationID = cdata.stationID;
    cdata.logonMinutes = 2;

    sItemFactory.SetUsingClient( pClient );

    CharacterRef charRef = sItemFactory.SpawnCharacter(cdata, corpData);
    if (charRef.get() == nullptr) {
        //a return to the client of 0 seems to be the only means of marking failure
        _log(CLIENT__ERROR, "Failed to create character '%s'", cdata.name.c_str());
        sItemFactory.UnsetUsingClient();
        return PyStatic.NewZero();
    }
    charRef->SetClient(pClient);      // set client in char

    // add call to JoinCorp here, and remove corp shit from charDB

    //this builds and saves appearance data from charInfo dict
    CharacterAppearance capp;
        capp.Build(charRef->itemID(), charInfoData);

    //this builds and saves portrait data from portraitInfo dict
    CharacterPortrait cpor;
        cpor.Build(charRef->itemID(), portraitInfoData);

    // query attribute bonuses from ancestry
    if (!CharacterDB::GetAttributesFromAncestry(cdata.ancestryID, intelligence, charisma, perception, memory, willpower)) {
        _log(CLIENT__ERROR, "Failed to load char create details. Bloodline %u, ancestry %u.", char_type->bloodlineID(), cdata.ancestryID);
        sItemFactory.UnsetUsingClient();
        return PyStatic.NewZero();
    }
    // triple attributes and save
    uint8 multiplier = sConfig.character.statMultiplier;
    charRef->SetAttribute(AttrIntelligence, intelligence * multiplier, false);
    charRef->SetAttribute(AttrCharisma, charisma * multiplier, false);
    charRef->SetAttribute(AttrPerception, perception * multiplier, false);
    charRef->SetAttribute(AttrMemory, memory * multiplier, false);
    charRef->SetAttribute(AttrWillpower, willpower * multiplier, false);

    //load skills
    std::map<uint32, uint8> startingSkills;
    startingSkills.clear();
	//  Base Skills
    if (!CharacterDB::GetBaseSkills(startingSkills)) {
        _log(CLIENT__ERROR, "Failed to load char Base skills. Bloodline %u, Ancestry %u.",
             char_type->bloodlineID(), cdata.ancestryID);
        // dont really care if this fails.  not enough to deny creation ...maybe make error?
    }
	//  Race Skills
    if (!CharacterDB::GetSkillsByRace(char_type->race(), startingSkills)) {
        _log(CLIENT__ERROR, "Failed to load char Race skills. Bloodline %u, Ancestry %u.",
             char_type->bloodlineID(), cdata.ancestryID);
        // dont really care if this fails.  not enough to deny creation ...maybe make error?
    }
	//  Career Skills
    if (!CharacterDB::GetSkillsByCareer(cdata.careerID, startingSkills)) {
        _log(CLIENT__ERROR, "Failed to load char Career skills for %u.", cdata.careerSpecialityID);
        // dont really care if this fails.  not enough to deny creation ...maybe make error?
    }

    //spawn all the skills
    uint8 skillLevel = 0;
    uint32 skillPoints = 0;
    for (auto cur : startingSkills) {
        ItemData skillItem( cur.first, charRef->itemID(), charRef->itemID(), flagSkill );
        SkillRef skill = sItemFactory.SpawnSkill( skillItem );
        if (skill.get() == nullptr) {
            _log(CLIENT__ERROR, "Failed to add skill %u to char %s(%u) during create.",
                 cur.first, charRef->name(), charRef->itemID());
            // missed a skill...whatever.
            continue;
        }

        skillLevel = cur.second;
        skill->SetAttribute(AttrSkillLevel, skillLevel, false);
        skillPoints = skill->GetSPForLevel(skillLevel);
        skill->SetAttribute(AttrSkillPoints, skillPoints, false);
        skill->SaveItem();
        cdata.skillPoints += skillPoints;
        charRef->SaveSkillHistory(EvESkill::Event::SkillPointsApplied/*EvESkill::Event::CharCreation*/, // 'CharCreation' shows as "Unknown" in PD>Skill>History
                                  GetFileTimeNow(),
                                    charRef->itemID(),
                                    cur.first,
                                    skillLevel,
                                    skillPoints);
    }

    //now set up some initial inventory:
    /** @todo update this to reflect char career */

    // add client to EntityList for subsequent calls that need Client* (AttributeMap changes)
    pClient->SetChar(charRef);        // AddPlayer() needs charRef
    sEntityList.AddPlayer(pClient);

    // need system loaded for proper ship creation/loading and subsquent character login
    sEntityList.FindOrBootSystem(cdata.solarSystemID);

    // create alpha-level clone
    ItemData iData( itemCloneAlpha, charRef->itemID(), cdata.locationID, flagClone, 1 );
        iData.customInfo="Active: ";
        iData.customInfo += charRef->itemName();
        iData.customInfo += "(";
        iData.customInfo += std::to_string(charRef->itemID());
        iData.customInfo += ")";
    sItemFactory.SpawnItem( iData )->SaveItem();

    // give the player their pod and set in system (NOT hangar)
    pClient->CreateNewPod();
    pClient->GetPod()->Move(cdata.solarSystemID, flagCapsule, false);

    ShipItemRef sRef = pClient->SpawnNewRookieShip(cdata.locationID);
    // set shipID in client and char objects and save (shipID error fix)
    pClient->SetShip(sRef);
    charRef->SaveFullCharacter();

    // Release the item factory now that the character is finished being accessed:
    sItemFactory.UnsetUsingClient();

    // we need to report the charID to the ImageServer so it can correctly assign a previously received image
    sImageServer.ReportNewCharacter(pClient->GetUserID(), charRef->itemID());

    //  add charID to staticOwners
    CharacterDB::AddOwnerCache(charRef->itemID(), charRef->itemName(), char_type->id());

    std::string reason = "DESC: Inheritance Payment to ";
    reason += charRef->itemName();
    AccountService::TranserFunds(corpSCC, charRef->itemID(), sConfig.character.startBalance, reason, Journal::EntryType::Inheritance);
    AccountService::TranserFunds(corpSCC, charRef->itemID(), sConfig.character.startAurBalance, reason, \
                            Journal::EntryType::Inheritance, 0, Account::KeyType::AUR, Account::KeyType::AUR);

    charRef->LogOut();
    sRef->LogOut();

    sEntityList.RemovePlayer(pClient);

    pClient->CreateChar(false);

    _log( CLIENT__MESSAGE, "Created New Character - Sending charID %u as reply", charRef->itemID() );

    return new PyInt(charRef->itemID());
}
