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
    Author:     Zhur
*/

#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(CharacterService)

CharacterService::CharacterService(PyServiceMgr *mgr)
: PyService(mgr, "character"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CharacterService, GetCharactersToSelect)
    PyCallable_REG_CALL(CharacterService, GetCharacterToSelect)
    PyCallable_REG_CALL(CharacterService, SelectCharacterID)
    PyCallable_REG_CALL(CharacterService, GetOwnerNoteLabels)
    PyCallable_REG_CALL(CharacterService, GetCharCreationInfo)
    PyCallable_REG_CALL(CharacterService, GetCharNewExtraCreationInfo)
    PyCallable_REG_CALL(CharacterService, GetAppearanceInfo)
    PyCallable_REG_CALL(CharacterService, ValidateName)
    PyCallable_REG_CALL(CharacterService, ValidateNameEx)
    PyCallable_REG_CALL(CharacterService, CreateCharacter2)
    PyCallable_REG_CALL(CharacterService, Ping)
    PyCallable_REG_CALL(CharacterService, PrepareCharacterForDelete)
    PyCallable_REG_CALL(CharacterService, CancelCharacterDeletePrepare)
    PyCallable_REG_CALL(CharacterService, AddOwnerNote)
    PyCallable_REG_CALL(CharacterService, EditOwnerNote)
    PyCallable_REG_CALL(CharacterService, GetOwnerNote)
    PyCallable_REG_CALL(CharacterService, GetHomeStation)
    PyCallable_REG_CALL(CharacterService, GetCloneTypeID)
    PyCallable_REG_CALL(CharacterService, GetCharacterAppearanceList)
    PyCallable_REG_CALL(CharacterService, GetRecentShipKillsAndLosses)

    PyCallable_REG_CALL(CharacterService, GetCharacterDescription)
    PyCallable_REG_CALL(CharacterService, SetCharacterDescription)

    PyCallable_REG_CALL(CharacterService, GetNote)
    PyCallable_REG_CALL(CharacterService, SetNote)

    PyCallable_REG_CALL(CharacterService, LogStartOfCharacterCreation)
}

CharacterService::~CharacterService() {
    delete m_dispatch;
}

//void CharacterService::GetCharacterData(uint32 characterID, std::vector<uint32> &characterDataVector)
void CharacterService::GetCharacterData(uint32 characterID, std::map<std::string, uint32> &characterDataMap) {
    m_db.GetCharacterData( characterID, characterDataMap );
}

PyResult CharacterService::Handle_GetCharactersToSelect(PyCallArgs &call) {
    return(m_db.GetCharacterList(call.client->GetAccountID()));
}

PyResult CharacterService::Handle_GetCharacterToSelect(PyCallArgs &call) {
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

PyResult CharacterService::Handle_SelectCharacterID(PyCallArgs &call) {
    CallSelectCharacterID args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Failed to parse args from account %u", call.client->GetAccountID());
        //TODO: throw exception
        return NULL;
    }

    //we don't care about tutorial dungeon right now
    call.client->SelectCharacter(args.charID);

    return NULL;
}

PyResult CharacterService::Handle_GetOwnerNoteLabels(PyCallArgs &call) {
    return m_db.GetOwnerNoteLabels(call.client->GetCharacterID());
}

PyResult CharacterService::Handle_GetCharCreationInfo(PyCallArgs &call) {
    PyDict *result = new PyDict();

    //send all the cache hints needed for char creation.
    m_manager->cache_service->InsertCacheHints(
        ObjCacheService::hCharCreateCachables,
        result);
    _log(CLIENT__MESSAGE, "Sending char creation info reply");

    return result;
}

PyResult CharacterService::Handle_GetCharNewExtraCreationInfo(PyCallArgs &call) {
    _log(CLIENT__MESSAGE, "Sending empty char new extra creation info reply");

    return new PyDict();
}

PyResult CharacterService::Handle_GetAppearanceInfo(PyCallArgs &call) {
    PyDict *result = new PyDict();

    //send all the cache hints needed for char creation.
    m_manager->cache_service->InsertCacheHints(
        ObjCacheService::hAppearanceCachables,
        result );

    _log(CLIENT__MESSAGE, "Sending appearance info reply");

    return result;
}

PyResult CharacterService::Handle_ValidateName( PyCallArgs& call )
{
    Call_SingleWStringArg arg;
    if( !arg.Decode( &call.tuple ) )
    {
        _log( SERVICE__ERROR, "Failed to decode args." );
        return NULL;
    }

    // perform checks on the "name" that is passed.  we may want to impliment something
    // to limit the kind of names allowed.

    return new PyBool( m_db.ValidateCharName( arg.arg.c_str() ) );
}

PyResult CharacterService::Handle_ValidateNameEx( PyCallArgs& call )
{
    //just redirect it now
    return Handle_ValidateName( call );
}

PyResult CharacterService::Handle_CreateCharacter2(PyCallArgs &call) {
    CallCreateCharacter2 arg;
    if(!arg.Decode(&call.tuple)) {
        _log(CLIENT__ERROR, "Failed to decode CreateCharacter2 arg.");
        return NULL;
    }

    _log(CLIENT__MESSAGE, "CreateCharacter2 called for '%s'", arg.name.c_str());
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

    // just hack something
    cdata.careerID = 11;
    cdata.careerSpecialityID = 11;

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

    //NOTE: these are currently hard coded to Todaki because other things are
    //also hard coded to only work in Todaki. Once these various things get fixed,
    //just take this code out and the above calls should have cdata populated correctly.
    cdata.stationID = idata.locationID = 60004420;
    cdata.solarSystemID = 30001407;
    cdata.constellationID = 20000206;
    cdata.regionID = 10000016;

    cdata.bounty = 0;
    cdata.balance = sConfig.character.startBalance;
    cdata.securityRating = 0;
    cdata.logonMinutes = 0;
    cdata.title = "No Title";

    cdata.startDateTime = Win32TimeNow();
    cdata.createDateTime = cdata.startDateTime;
    cdata.corporationDateTime = cdata.startDateTime;

    //this builds appearance data from strdict
    capp.Build(arg.appearance);

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

    return new PyInt( char_item->itemID() );
}

PyResult CharacterService::Handle_Ping(PyCallArgs &call)
{
    return(new PyInt(call.client->GetAccountID()));
}

PyResult CharacterService::Handle_PrepareCharacterForDelete(PyCallArgs &call) {
    /*
     * Deletion is apparently a timed process, and is related to the
     * deletePrepareDateTime column in the char select result.
     *
     * For now, we will just implement an immediate delete as its better than
     * nothing, since clearing out all the tables by hand is a nightmare.
     */

    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
        return NULL;
    }

    //TODO: make sure this person actually owns this char...

    sLog.Debug( "CharacterService", "Called PrepareCharacterForDelete stub: deleting immediately." );

    { // character scope to make sure char_item is deleted immediately
        m_manager->item_factory.SetUsingClient( call.client );
        CharacterRef char_item = m_manager->item_factory.GetCharacter( args.arg );
        if( !char_item ) {
            codelog(CLIENT__ERROR, "Failed to load char item %u.", args.arg);
            return NULL;
        }
        //unregister name
        m_db.del_name_validation_set( char_item->itemID() );
        //does the recursive delete of all contained items
        char_item->Delete();
    }

    //now, clean up all items which werent deleted
    std::vector<uint32> items;
    if(!m_db.GetCharItems(args.arg, items)) {
        codelog(CLIENT__ERROR, "Unable to get items of char %u.", args.arg);
        return NULL;
    }

    std::vector<uint32>::iterator cur, end;
    cur = items.begin();
    end = items.end();
    for(; cur != end; cur++) {
        InventoryItemRef i = m_manager->item_factory.GetItem( *cur );
        if( !i ) {
            codelog(CLIENT__ERROR, "Failed to load item %u to delete. Skipping.", *cur);
            continue;
        }

        i->Delete();
    }

    //we return deletePrepareDateTime, in eve time format.
    return(new PyLong(Win32TimeNow() + Win32Time_Second*5));
}

PyResult CharacterService::Handle_CancelCharacterDeletePrepare( PyCallArgs& call )
{
    Call_SingleIntegerArg args;
    if( !args.Decode( &call.tuple ) )
    {
        codelog( CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName() );
        return NULL;
    }

    sLog.Debug( "CharacterService", "Called CancelCharacterDeletePrepare stub." );

    //returns nothing.
    return NULL;
}

PyResult CharacterService::Handle_AddOwnerNote(PyCallArgs &call) {
    Call_AddOwnerNote args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return(new PyInt(0));
    }

    uint32 noteID = m_db.AddOwnerNote(call.client->GetCharacterID(), args.label, args.content);
    if (noteID == 0) {
        codelog(SERVICE__ERROR, "%s: Failed to set Owner note.", call.client->GetName());
        return (new PyNone());
    }

    return(new PyInt(noteID));
}

PyResult CharacterService::Handle_EditOwnerNote(PyCallArgs &call) {
    Call_EditOwnerNote args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return(new PyInt(0));
    }

    if (!m_db.EditOwnerNote(call.client->GetCharacterID(), args.noteID, args.label, args.content)) {
        codelog(SERVICE__ERROR, "%s: Failed to set Owner note.", call.client->GetName());
        return (new PyNone());
    }

    return(new PyInt(args.noteID));
}

PyResult CharacterService::Handle_GetOwnerNote(PyCallArgs &call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return(new PyInt(0));
    }
    return m_db.GetOwnerNote(call.client->GetCharacterID(), arg.arg);
}


PyResult CharacterService::Handle_GetHomeStation( PyCallArgs& call )
{
    //returns the station ID of the station where this player's Clone is

	uint32 clone;
	m_db.GetActiveClone(call.client->GetCharacterID(),clone);

	return new PyInt( m_manager->item_factory.GetItem(clone)->locationID() );
}

PyResult CharacterService::Handle_GetCloneTypeID( PyCallArgs& call )
{
	uint32 clone;
	//we get the typeID of the active clone.
	//this works without restarting the server and recaching
	//everything after we upgrade the clone
	m_db.GetActiveCloneType(call.client->GetCharacterID(),clone);
	return new PyInt(clone);
}

PyResult CharacterService::Handle_GetRecentShipKillsAndLosses( PyCallArgs& call )
{
    sLog.Debug( "CharacterService", "Called GetRecentShipKillsAndLosses stub." );

    util_Rowset rs;

    rs.header.push_back("killID");
    rs.header.push_back("solarSystemID");
    rs.header.push_back("victimCharacterID");
    rs.header.push_back("victimCorporationID");
    rs.header.push_back("victimAllianceID");
    rs.header.push_back("victimFactionID");
    rs.header.push_back("victimShipTypeID");
    rs.header.push_back("finalCharacterID");
    rs.header.push_back("finalCorporationID");
    rs.header.push_back("finalAllianceID");
    rs.header.push_back("finalFactionID");
    rs.header.push_back("finalShipTypeID");
    rs.header.push_back("finalWeaponTypeID");
    rs.header.push_back("killBlob");    //string
    rs.header.push_back("killTime");    //uint64
    rs.header.push_back("victimDamageTaken");
    rs.header.push_back("finalSecurityStatus"); //real
    rs.header.push_back("finalDamageDone");
    rs.header.push_back("moonID");

    return(rs.Encode());
}

/////////////////////////
PyResult CharacterService::Handle_GetCharacterDescription(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments");
        return NULL;
    }

    m_manager->item_factory.SetUsingClient( call.client );
    CharacterRef c = m_manager->item_factory.GetCharacter(args.arg);
    if( !c ) {
        _log(CLIENT__ERROR, "Failed to load character %u.", args.arg);
        return NULL;
    }

    return c->GetDescription();
}

//////////////////////////////////
PyResult CharacterService::Handle_SetCharacterDescription(PyCallArgs &call) {
    Call_SingleWStringSoftArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments");
        return NULL;
    }

    CharacterRef c = call.client->GetChar();
    if( !c ) {
        _log(CLIENT__ERROR, "SetCharacterDescription called with no char!");
        return NULL;
    }

    c->SetDescription( args.arg.c_str() );

    return NULL;
}

PyResult CharacterService::Handle_GetCharacterAppearanceList(PyCallArgs &call) {
    Call_SingleIntList args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: Invalid arguments", call.client->GetName());
        return NULL;
    }

    PyList *l = new PyList();

    std::vector<int32>::iterator cur, end;
    cur = args.ints.begin();
    end = args.ints.end();
    for(; cur != end; cur++) {
        PyRep* r = m_db.GetCharacterAppearance(*cur);
        if(r == NULL)
            r = new PyNone();

        l->AddItem(r);
    }

    return(l);
}

/** Retrieves a Character note.
*
* Checks the database for a given character note and returns it.
*
* @return PySubStream pointer with the string with the note or with PyNone if no note is stored.
*
*  **LSMoura
*/
PyResult CharacterService::Handle_GetNote(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments");
        return NULL;
    }

    return(m_db.GetNote(call.client->GetCharacterID(), args.arg));
}

/** Stores a Character note.
*
*  Stores the given character note in the database.
*
* @return PySubStream pointer with a PyNone() value.
*
*  **LSMoura
*/
PyResult CharacterService::Handle_SetNote(PyCallArgs &call) {
    Call_SetNote args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments");
        return NULL;
    }

    if (!m_db.SetNote(call.client->GetCharacterID(), args.itemID, args.note.c_str()))
        codelog(CLIENT__ERROR, "Error while storing the note on the database.");

    return NULL;
}

PyResult CharacterService::Handle_LogStartOfCharacterCreation(PyCallArgs &call)
{
    /* we seem to send nothing back. */
    return NULL;
}
