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
    Author:     caytchen, Zhur
*/

#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(CharUnboundMgrService)

CharUnboundMgrService::CharUnboundMgrService(PyServiceMgr* mgr)
: PyService(mgr, "charUnboundMgr"),
  m_dispatch(new Dispatcher(this)) 
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(CharUnboundMgrService, SelectCharacterID)
	PyCallable_REG_CALL(CharUnboundMgrService, GetCharacterToSelect)
	PyCallable_REG_CALL(CharUnboundMgrService, GetCharactersToSelect)
	PyCallable_REG_CALL(CharUnboundMgrService, GetCharacterInfo)
	PyCallable_REG_CALL(CharUnboundMgrService, IsUserReceivingCharacter)
	PyCallable_REG_CALL(CharUnboundMgrService, DeleteCharacter)
	PyCallable_REG_CALL(CharUnboundMgrService, PrepareCharacterForDelete)
	PyCallable_REG_CALL(CharUnboundMgrService, CancelCharacterDeletePrepare)
	PyCallable_REG_CALL(CharUnboundMgrService, ValidateNameEx)
	PyCallable_REG_CALL(CharUnboundMgrService, GetCharCreationInfo)
	PyCallable_REG_CALL(CharUnboundMgrService, GetCharNewExtraCreationInfo)
	PyCallable_REG_CALL(CharUnboundMgrService, CreateCharacterWithDoll)
}

CharUnboundMgrService::~CharUnboundMgrService() {
	delete m_dispatch;
}

PyResult CharUnboundMgrService::Handle_IsUserReceivingCharacter(PyCallArgs &call) {
	return new PyBool(false);
}

PyResult CharUnboundMgrService::Handle_ValidateNameEx(PyCallArgs &call)
{
	Call_SingleWStringArg arg;
	if (!arg.Decode(&call.tuple))
	{
		codelog(CLIENT__ERROR, "Failed to decode args for ValidateNameEx call");
		return NULL;
	}

	return new PyBool(m_db.ValidateCharName(arg.arg.c_str()));
}

PyResult CharUnboundMgrService::Handle_SelectCharacterID(PyCallArgs &call) {
	CallSelectCharacterID arg;
	if (!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Failed to decode args for SelectCharacterID call");
		return NULL;
	}

	call.client->SelectCharacter(arg.charID);
	return NULL;
}

PyResult CharUnboundMgrService::Handle_GetCharactersToSelect(PyCallArgs &call) {
	return(m_db.GetCharacterList(call.client->GetAccountID()));
}

PyResult CharUnboundMgrService::Handle_GetCharacterToSelect(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments");
		return NULL;
	}

	PyRep *result = m_db.GetCharSelectInfo(args.arg);
	if(result == NULL) {
		_log(CLIENT__ERROR, "Failed to load character %d", args.arg);
		return NULL;
	}

	return result;
}

PyResult CharUnboundMgrService::Handle_DeleteCharacter(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if (!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments for DeleteCharacter call");
		return NULL;
	}

	return m_db.DeleteCharacter(call.client->GetAccountID(), args.arg);
}

PyResult CharUnboundMgrService::Handle_PrepareCharacterForDelete(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if (!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments for PrepareCharacterForDelete call");
		return NULL;
	}

	return new PyLong((int64)m_db.PrepareCharacterForDelete(call.client->GetAccountID(), args.arg));
}

PyResult CharUnboundMgrService::Handle_CancelCharacterDeletePrepare(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if (!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "Invalid arguments for CancelCharacterDeletePrepare call");
		return NULL;
	}

	m_db.CancelCharacterDeletePrepare(call.client->GetAccountID(), args.arg);

	// the client doesn't care what we return here
	return NULL;
}

PyResult CharUnboundMgrService::Handle_GetCharacterInfo(PyCallArgs &call) {
	return NULL;
}

PyResult CharUnboundMgrService::Handle_GetCharCreationInfo(PyCallArgs &call) {
	PyDict *result = new PyDict();

	//send all the cache hints needed for char creation.
	m_manager->cache_service->InsertCacheHints(
		ObjCacheService::hCharCreateCachables,
		result);
	_log(CLIENT__MESSAGE, "Sending char creation info reply");

	return result;
}

PyResult CharUnboundMgrService::Handle_GetCharNewExtraCreationInfo(PyCallArgs &call) {
	PyDict *result = new PyDict();
	m_manager->cache_service->InsertCacheHints(ObjCacheService::hCharCreateNewExtraCachables, result);
	_log(CLIENT__MESSAGE, "Sending char new extra creation info reply");
	return result;
}

PyResult CharUnboundMgrService::Handle_CreateCharacterWithDoll(PyCallArgs &call) {
	CallCreateCharacterWithDoll arg;
	if (!arg.Decode(call.tuple))
	{
		codelog(CLIENT__ERROR, "Failed to decode args for CreateCharacterWithDoll call");
		return NULL;
	}

	_log(CLIENT__MESSAGE, "CreateCharacterWithDoll called for '%s'", arg.name.c_str());
    _log(CLIENT__MESSAGE, "  bloodlineID=%u genderID=%u ancestryID=%u",
            arg.bloodlineID, arg.genderID, arg.ancestryID);

    // obtain character type
    m_manager->item_factory.SetUsingClient( call.client );
    const CharacterType *char_type = m_manager->item_factory.GetCharacterTypeByBloodline(arg.bloodlineID);
    if(char_type == NULL)
        return NULL;

    // we need to fill these to successfully create character item
    ItemData idata;
    CharacterData cdata;
    CharacterAppearance capp;
    CorpMemberInfo corpData;

    idata.typeID = char_type->id();
    idata.name = arg.name;
    idata.ownerID = 1; // EVE System
    idata.quantity = 1;
    idata.singleton = true;

    cdata.accountID = call.client->GetAccountID();
    cdata.gender = arg.genderID;
    cdata.ancestryID = arg.ancestryID;
	cdata.schoolID = arg.schoolID;

	//Set the character's career based on the school they picked.
	if( m_db.GetCareerBySchool( cdata.schoolID, cdata.careerID ) ) {
		// Right now we don't know what causes the specialization switch, so just make both values the same
		cdata.careerSpecialityID = cdata.careerID;
	} else {
		codelog(SERVICE__WARNING, "Could not find default School ID %u. Using Caldari Military.", cdata.schoolID);
		cdata.careerID = 11;
		cdata.careerSpecialityID = 11;
	}

    corpData.corpRole = 0;
    corpData.rolesAtAll = 0;
    corpData.rolesAtBase = 0;
    corpData.rolesAtHQ = 0;
    corpData.rolesAtOther = 0;

    // Variables for storing attribute bonuses
    uint8 intelligence = char_type->intelligence();
    uint8 charisma = char_type->charisma();
    uint8 perception = char_type->perception();
    uint8 memory = char_type->memory();
    uint8 willpower = char_type->willpower();

    // Setting character's starting position, and getting it's location...
    // Also query attribute bonuses from ancestry
    if(    !m_db.GetLocationCorporationByCareer(cdata)
        || !m_db.GetAttributesFromAncestry(cdata.ancestryID, intelligence, charisma, perception, memory, willpower)
    ) {
        codelog(CLIENT__ERROR, "Failed to load char create details. Bloodline %u, ancestry %u.",
            char_type->bloodlineID(), cdata.ancestryID);
        return NULL;
    }

	// Added ability to set starting station in xml config by Pyrii
	if( sConfig.character.startStation ) { // Skip if 0
		if( !m_db.GetLocationByStation(sConfig.character.startStation, cdata) ) {
			codelog(SERVICE__WARNING, "Could not find default station ID %u. Using Career Defaults instead.", sConfig.character.startStation);
		}
	}

	idata.locationID = cdata.stationID; // Just so our starting items end up in the same place.

	// Change starting corperation based on value in XML file.
	if( sConfig.character.startCorporation ) { // Skip if 0
		if( m_db.DoesCorporationExist( sConfig.character.startCorporation ) ) {
			cdata.corporationID = sConfig.character.startCorporation;
		} else {
			codelog(SERVICE__WARNING, "Could not find default Corporation ID %u. Using Career Defaults instead.", sConfig.character.startCorporation);
		}
	}

    cdata.bounty = 0;
    cdata.balance = sConfig.character.startBalance;
    cdata.securityRating = sConfig.character.startSecRating;
    cdata.logonMinutes = 0;
    cdata.title = "No Title";

    cdata.startDateTime = Win32TimeNow();
    cdata.createDateTime = cdata.startDateTime;
    cdata.corporationDateTime = cdata.startDateTime;

    //this builds appearance data from strdict --- no longer relevant with the next gen stuff
    //capp.Build(arg.appearance);

    typedef std::map<uint32, uint32>        CharSkillMap;
    typedef CharSkillMap::iterator          CharSkillMapItr;

    //load skills
    CharSkillMap startingSkills;
    if( !m_db.GetSkillsByRace(char_type->race(), startingSkills) )
    {
        codelog(CLIENT__ERROR, "Failed to load char create skills. Bloodline %u, Ancestry %u.",
            char_type->bloodlineID(), cdata.ancestryID);
        return NULL;
    }

    //now we have all the data we need, stick it in the DB
    //create char item
    CharacterRef char_item = m_manager->item_factory.SpawnCharacter(idata, cdata, capp, corpData);
    if( !char_item ) {
        //a return to the client of 0 seems to be the only means of marking failure
        codelog(CLIENT__ERROR, "Failed to create character '%s'", idata.name.c_str());
        return NULL;
    }

    // add attribute bonuses
    // (use Set_##_persist to properly persist attributes into DB)
    /*char_item->Set_intelligence_persist(intelligence);
    char_item->Set_charisma_persist(charisma);
    char_item->Set_perception_persist(perception);
    char_item->Set_memory_persist(memory);
    char_item->Set_willpower_persist(willpower);*/

    char_item->SetAttribute(AttrIntelligence, intelligence);
    char_item->SetAttribute(AttrCharisma, charisma);
    char_item->SetAttribute(AttrPerception, perception);
    char_item->SetAttribute(AttrMemory, memory);
    char_item->SetAttribute(AttrWillpower, willpower);

    // register name
    m_db.add_name_validation_set(char_item->itemName().c_str(), char_item->itemID());

    //spawn all the skills
    uint32 skillLevel;
    EvilNumber skillPoints;
    CharSkillMapItr cur, end;
    cur = startingSkills.begin();
    end = startingSkills.end();
    for(; cur != end; cur++)
    {
        ItemData skillItem( cur->first, char_item->itemID(), char_item->itemID(), flagSkill );
        SkillRef i = m_manager->item_factory.SpawnSkill( skillItem );
        if( !i ) {
            _log(CLIENT__ERROR, "Failed to add skill %u to char %s (%u) during char create.", cur->first, char_item->itemName().c_str(), char_item->itemID());
            continue;
        }

        //_log(CLIENT__MESSAGE, "Training skill %u to level %d (%d points)", i->typeID(), i->skillLevel(), i->skillPoints());
        //i->Set_skillLevel( cur->second );
        skillLevel = cur->second;
        i->SetAttribute(AttrSkillLevel, skillLevel );
        //i->Set_skillPoints( i->GetSPForLevel( cur->second ) );
        skillPoints = i->GetSPForLevel( EvilNumber((uint64)cur->second) );
        skillPoints.to_float();
        //if( !(skillPoints.to_int()) )
        //    sLog.Error( "CharacterService::Handle_CreateCharacter2()", "skillPoints.to_int() failed, resulting in a floating point value larger than a 64-bit signed integer... o.O !!" );
        i->SetAttribute(AttrSkillPoints, skillPoints );
        i->SaveAttributes();
        //i->mAttributeMap.Save();
    }

    //now set up some initial inventory:
    InventoryItemRef initInvItem;

    // add "Damage Control I"
    ItemData itemDamageControl( 2046, char_item->itemID(), char_item->locationID(), flagHangar, 1 );
    initInvItem = m_manager->item_factory.SpawnItem( itemDamageControl );

    if( !initInvItem )
        codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", char_item->itemName().c_str());

    // add 1 unit of "Tritanium"
    ItemData itemTritanium( 34, char_item->itemID(), char_item->locationID(), flagHangar, 1 );
    initInvItem = m_manager->item_factory.SpawnItem( itemTritanium );

    // add 1 unit of "Clone Grade Alpha"
    ItemData itemCloneAlpha( 164, char_item->itemID(), char_item->locationID(), flagClone, 1 );
	itemCloneAlpha.customInfo="active";
    initInvItem = m_manager->item_factory.SpawnItem( itemCloneAlpha );

    if( !initInvItem )
        codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", char_item->itemName().c_str());

    // give the player its ship.
    std::string ship_name = char_item->itemName() + "'s Ship";

    ItemData shipItem( char_type->shipTypeID(), char_item->itemID(), char_item->locationID(), flagHangar, ship_name.c_str() );
    ShipRef ship_item = m_manager->item_factory.SpawnShip( shipItem );

    if( !ship_item ) {
        codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", char_item->itemName().c_str());
    }
    else
        //welcome on board your starting ship
        char_item->MoveInto( *ship_item, flagPilot, false );

    _log( CLIENT__MESSAGE, "Sending char create ID %u as reply", char_item->itemID() );

	// we need to report the charID to the ImageServer so it can correctly assign a previously received image
	ImageServer::get().ReportNewCharacter(call.client->GetAccountID(), char_item->itemID());

    return new PyInt( char_item->itemID() );
}