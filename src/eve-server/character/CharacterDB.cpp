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

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "character/Character.h"
#include "character/CharacterDB.h"

uint32 CharacterDB::NewCharacter(const CharacterData& data, const CorpData& corpData) {
    DBerror err;
    std::string nameEsc, titleEsc, descriptionEsc;
    sDatabase.DoEscapeString(nameEsc, data.name);
    sDatabase.DoEscapeString(titleEsc, data.title);
    sDatabase.DoEscapeString(descriptionEsc, data.description);

    uint32 charID = 0;
    if (!sDatabase.RunQueryLID(err, charID,
        "INSERT INTO chrCharacters"
        "  (accountID, characterName, typeID, locationID, description, balance, aurBalance,"
        "   logonDateTime, baseID, corpAccountKey, createDateTime, title,"
        "   ancestryID, bloodlineID, raceID, careerID, schoolID, careerSpecialityID, gender,"
        "   stationID, solarSystemID, constellationID, regionID, freeRespecs)"
        " VALUES"
        "  (%u,'%s', %u, %u, '%s', %f, %f,"
        "   %f, %u, %u, %f, '%s',"
        "   %u, %u, %u, %u, %u, %u, %u,"
        "   %u, %u, %u, %u, 2)",
        data.accountID, nameEsc.c_str(), data.typeID, data.locationID, descriptionEsc.c_str(), data.balance, data.aurBalance,
        GetFileTimeNow(), corpData.baseID, corpData.corpAccountKey, GetFileTimeNow(), titleEsc.c_str(),
        data.ancestryID, data.bloodlineID, data.raceID, data.careerID, data.schoolID, data.careerSpecialityID, data.gender,
        data.stationID, data.solarSystemID, data.constellationID, data.regionID))
    {
        codelog(DATABASE__ERROR, "Failed to insert character %s.", err.c_str());
        return 0;
    }

    AddEmployment(charID, corpData.corporationID);

    return charID;
}

bool CharacterDB::SaveCharacter(uint32 characterID, const CharacterData &data) {
    DBerror err;

    std::string titleEsc, descriptionEsc;
    sDatabase.DoEscapeString(titleEsc, data.title);
    sDatabase.DoEscapeString(descriptionEsc, data.description);

    if (!sDatabase.RunQuery(err,
        "UPDATE chrCharacters"
        " SET"
        "  title = '%s',"
        "  description = '%s',"
        "  bounty = %f,"
        "  balance = %f,"
        "  aurBalance = %f,"
        "  securityRating = %f,"
        "  logonMinutes = %u,"
        "  skillPoints = %u,"
        "  locationID = %u,"
        "  stationID = %u,"
        "  solarSystemID = %u,"
        "  constellationID = %u,"
        "  regionID = %u"
        " WHERE characterID = %u",
            titleEsc.c_str(), descriptionEsc.c_str(), data.bounty, data.balance, data.aurBalance, data.securityRating, data.logonMinutes,
            data.skillPoints, data.locationID, data.stationID, data.solarSystemID, data.constellationID, data.regionID, characterID))
    {
        codelog(DATABASE__ERROR, "Failed to save character %u: %s.", characterID, err.c_str());
        return false;
    }

    return true;
}

bool CharacterDB::SaveCorpData(uint32 characterID, const CorpData &data) {
    DBerror err;

    if (!sDatabase.RunQuery(err,
        "UPDATE chrCharacters"
        " SET"
        "  corporationID = %u, "
        "  baseID = %u,"
        "  corpRole = %lli,"
        "  corpAccountKey = %i,"
        "  rolesAtAll = %lli,"
        "  rolesAtBase = %lli,"
        "  rolesAtHQ = %lli,"
        "  rolesAtOther = %lli,"
        "  grantableRoles = %lli,"
        "  grantableRolesAtBase = %lli,"
        "  grantableRolesAtHQ = %lli,"
        "  grantableRolesAtOther = %" PRIi64
        " WHERE characterID = %u",
        data.corporationID, data.baseID, data.corpRole, data.corpAccountKey, data.rolesAtAll, data.rolesAtBase, data.rolesAtHQ, data.rolesAtOther,
        data.grantableRoles, data.grantableRolesAtBase, data.grantableRolesAtHQ, data.grantableRolesAtOther, characterID))
    {
        codelog(DATABASE__ERROR, "Failed to update corp member info of character %u: %s.", characterID, err.c_str());
        return false;
    }

    return true;
}

void CharacterDB::DeleteCharacter(uint32 characterID) {
    /**
     *        0 matches in bounties
     *        4 matches in channelChars
     *        0 matches in chrOwnerNote
     *        0 matches in ramJobs
     */

    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM eveMailDetails"
                            " USING eveMail, eveMailDetails"
                            " WHERE eveMail.messageID = eveMailDetails.messageID"
                            " AND (senderID = %u OR channelID = %u)", characterID, characterID);
    sDatabase.RunQuery(err, "DELETE FROM eveMail WHERE (senderID = %u OR channelID = %u)", characterID, characterID);
    sDatabase.RunQuery(err, "DELETE FROM bookmarks WHERE ownerID = %u",  characterID);
    sDatabase.RunQuery(err, "DELETE FROM bookmarkFolders WHERE ownerID = %u",  characterID);
    //sDatabase.RunQuery(err, "DELETE FROM bookmarkVouchers WHERE ownerID = %u",  characterID);
    sDatabase.RunQuery(err, "DELETE FROM mktOrders WHERE ownerID = %u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM mktTransactions WHERE clientID = %u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM repStandings WHERE (fromID = %u OR toID = %u)", characterID, characterID);
    sDatabase.RunQuery(err, "DELETE FROM repStandingChanges WHERE (fromID = %u OR toID = %u)", characterID, characterID);
    sDatabase.RunQuery(err, "DELETE FROM chrCertificates WHERE characterID=%u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM chrCharacters WHERE characterID=%u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM chrEmployment WHERE characterID=%u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM jnlCharacters WHERE ownerID=%u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM crpShares WHERE shareholderID=%u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM chrSkillHistory WHERE characterID=%u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM chrSkillQueue WHERE characterID=%u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM crpApplications WHERE characterID=%u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM chrCharacterAttributes WHERE charID = %u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM chrPausedSkillQueue WHERE characterID = %u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM entity_attributes"
                            " WHERE itemID IN (SELECT itemID FROM entity WHERE ownerID = %u)", characterID);
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE ownerID = %u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM avatar_colors WHERE charID = %u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM avatar_modifiers WHERE charID = %u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM avatar_sculpts WHERE charID = %u", characterID);
    sDatabase.RunQuery(err, "DELETE FROM avatars WHERE charID = %u", characterID);
}

bool CharacterDB::ReportRespec(uint32 characterId)
{
    DBerror error;
    if (!sDatabase.RunQuery(error, "UPDATE chrCharacters SET freeRespecs = freeRespecs - 1, lastRespecDateTime = %f, nextRespecDateTime = %lli WHERE characterId = %u",
        GetFileTimeNow(), (GetFileTimeNow() + EvE::Time::Month *3), characterId))
        return false;
    return true;
}

PyRep* CharacterDB::GetRespecInfo(uint32 characterId)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT freeRespecs, lastRespecDateTime, nextRespecDateTime FROM chrCharacters WHERE characterID = %u", characterId))
        return nullptr;
    DBResultRow row;
    if (!res.GetRow(row))
        return nullptr;

    PyDict* result = new PyDict();
    result->SetItemString( "freeRespecs", new PyInt( row.GetInt(0) ) );
    result->SetItemString( "lastRespecDate", new PyLong( row.GetInt64(1) ) );
    result->SetItemString( "nextTimedRespec", new PyLong( row.GetInt64(2) ) );

    return result;
}

int64 CharacterDB::PrepareCharacterForDelete(uint32 accountID, uint32 charID)
{
    // calculate the point in time from which this character may be deleted
    int64 deleteTime = GetFileTimeNow() + (EvE::Time::Second * sConfig.character.terminationDelay);

    // note: the queries relating to character deletion have been specifically designed to avoid wreaking havoc when used by a malicious client
    // the client can't lie to us about accountID, only charID

    DBerror error;
    uint32 affectedRows;
    sDatabase.RunQuery(error, affectedRows, "UPDATE chrCharacters SET deletePrepareDateTime = %lli WHERE accountID = %u AND characterID = %u", deleteTime, accountID, charID);
    if (affectedRows != 1)
        return 0;

    return deleteTime;
}

void CharacterDB::CancelCharacterDeletePrepare(uint32 accountID, uint32 charID)
{
    DBerror error;
    uint32 affectedRows;
    sDatabase.RunQuery(error, affectedRows, "UPDATE chrCharacters SET deletePrepareDateTime = 0 WHERE accountID = %u AND characterID = %u", accountID, charID);
    if (affectedRows != 1)
        _log(CLIENT__ERROR, "Failed to cancel character deletion, affected rows: %u", affectedRows);
}

PyRep *CharacterDB::GetCharacterList(uint32 accountID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  characterID,"
        "  characterName,"
        "  deletePrepareDateTime,"
        "  gender,"
        "  typeID"
        " FROM chrCharacters"
        " WHERE accountID=%u", accountID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyRep* CharacterDB::ValidateCharNameRep(std::string name)
{
    /** @todo
            validStates = {-1: localization.GetByLabel('UI/CharacterCreation/InvalidName/TooShort'),
             -2: localization.GetByLabel('UI/CharacterCreation/InvalidName/TooLong'),
             -5: localization.GetByLabel('UI/CharacterCreation/InvalidName/IllegalCharacter'),
             -6: localization.GetByLabel('UI/CharacterCreation/InvalidName/TooManySpaces'),
             -7: localization.GetByLabel('UI/CharacterCreation/InvalidName/ConsecutiveSpaces'),
             -101: localization.GetByLabel('UI/CharacterCreation/InvalidName/Unavailable'),
             -102: localization.GetByLabel('UI/CharacterCreation/InvalidName/Unavailable')}
             */
    // *name  is sent from client WITHOUT leading space, if there is one, and will not allow more than one space.

    if (name.empty())
        return new PyInt(-1);
    if (name.length() < 3)
        return new PyInt(-1);
    if (name.length() > 37)    //client caps at 24
        return new PyInt(-2);

    if (!sDatabase.IsSafeString(name.c_str()))
        return new PyInt(-5);

    for (const auto cur : badWords)
        if (EvE::icontains(name, cur))
            return new PyInt(-5);

    for (const auto cur : badChars)
        if (EvE::icontains(name, cur))
            return new PyInt(-5);

    // check for consecutive spaces
    if (EvE::icontains(name, "  "))
        return new PyInt(-7);

    // check for multiple spaces
    int found = name.find(" ");
    if (found != name.npos) {
        found = name.find(" ", found + 1, 1);
        if (found != name.npos)
            return new PyInt(-6);
    }

    std::string eName;
    sDatabase.DoEscapeString(eName, name.c_str());
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT characterID FROM chrCharacters WHERE characterName LIKE '%s' ", eName.c_str() );
    if (res.GetRowCount() > 0)  // name exists
        return new PyInt(-101);

    /* if we got here the name is "new" */
    return PyStatic.NewOne();
}

/** @todo this should throw on error */
void CharacterDB::ValidateCharName(std::string name)
{
    /* {'messageKey': 'CharNameInvalid', 'dataID': 17885077, 'suppressable': False, 'bodyID': 260107, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 471}
     * {'messageKey': 'CharNameInvalidBannedWord', 'dataID': 17884963, 'suppressable': False, 'bodyID': 260067, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 472}
     * {'messageKey': 'CharNameInvalidFirstChar', 'dataID': 17884966, 'suppressable': False, 'bodyID': 260068, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 473}
     * {'messageKey': 'CharNameInvalidLastChar', 'dataID': 17884969, 'suppressable': False, 'bodyID': 260069, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 474}
     * {'messageKey': 'CharNameInvalidMaxLength', 'dataID': 17885083, 'suppressable': False, 'bodyID': 260109, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 475}
     * {'messageKey': 'CharNameInvalidMaxSpaces', 'dataID': 17884972, 'suppressable': False, 'bodyID': 260070, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 476}
     * {'messageKey': 'CharNameInvalidMinLength', 'dataID': 17884975, 'suppressable': False, 'bodyID': 260071, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 477}
     * {'messageKey': 'CharNameInvalidSomeChar', 'dataID': 17884978, 'suppressable': False, 'bodyID': 260072, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 478}
     * {'messageKey': 'CharNameInvalidTaken', 'dataID': 17884981, 'suppressable': False, 'bodyID': 260073, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 479}
     * {'messageKey': 'CharNameTooShort', 'dataID': 17884986, 'suppressable': False, 'bodyID': 260075, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 260074, 'messageID': 482}
     */
    /* {'FullPath': u'UI/Messages', 'messageID': 260067, 'label': u'CharNameInvalidBannedWordBody'}(u'Character name contains a banned word.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 260068, 'label': u'CharNameInvalidFirstCharBody'}(u'First character in character name is illegal.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 260069, 'label': u'CharNameInvalidLastCharBody'}(u'Last character in character name is illegal.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 260070, 'label': u'CharNameInvalidMaxSpacesBody'}(u'Character name can only include 1 space.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 260071, 'label': u'CharNameInvalidMinLengthBody'}(u'Minimum length for a character name is 4 characters.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 260072, 'label': u'CharNameInvalidSomeCharBody'}(u'Character name contains an illegal character.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 260073, 'label': u'CharNameInvalidTakenBody'}(u'Character name is already taken.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 260074, 'label': u'CharNameTooShortTitle'}(u'Name Too Short', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 260075, 'label': u'CharNameTooShortBody'}(u'The name must be at least 3 characters long.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 260107, 'label': u'CharNameInvalidBody'}(u'The name you have selected for your character is not valid', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 260109, 'label': u'CharNameInvalidMaxLengthBody'}(u'Maximum length for a character name is 37 characters. The first and second name must be less than 24 character combined, and the third name must be less than 12 characters.', None, None)
     * {'FullPath': u'UI/CharacterCreation/InvalidName', 'messageID': 233748, 'label': u'IllegalCharacter'}(u'Name contains illegal characters.', None, None)
     * {'FullPath': u'UI/CharacterCreation/InvalidName', 'messageID': 233749, 'label': u'TooShort'}(u'Name is too short.', None, None)
     * {'FullPath': u'UI/CharacterCreation/InvalidName', 'messageID': 233750, 'label': u'Unavailable'}(u'Name is unavailable.', None, None)
     *
     * {'FullPath': u'UI/Messages', 'messageID': 258506, 'label': u'CharCreationNameError1Title'}(u'Information', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 258507, 'label': u'CharCreationNameError1Body'}(u'You have to type some name to be able to proceed.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 258508, 'label': u'CharCreationNameError3Title'}(u'Information', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 258509, 'label': u'CharCreationNameError3Body'}(u'The name contains either illegal characters, is already taken or is not allowed.', None, None)
     *
     */

    // *name  is sent from client WITHOUT leading space, if there is one, and will not allow more than one space.

    if (name.empty())
        throw UserError ("CharNameInvalid");
    if (name.length() < 3)
        throw UserError ("CharNameTooShort");
    //if (name.length() < 4)
    //    throw UserError ("CharNameInvalidMinLength");
    if (name.length() > 37)    //client caps at 24
        throw UserError ("CharNameInvalidMaxLength");

    //if (!sDatabase.IsSafeString(name.c_str()))
    //    throw UserError ("CharNameInvalidSomeChar");

    //if (name.at(0) == "+")
    //    throw UserError ("CharNameInvalidFirstChar");

    // check for banned words in char name
    for (const auto cur : badWords)
        if (EvE::icontains(name, cur))
            throw UserError ("CharNameInvalidBannedWord");
    for (const auto cur : badChars)
        if (EvE::icontains(name, cur))
            throw UserError ("CharNameInvalidSomeChar");

    // check for multiple spaces
    int found = name.find(" ");
    if (found != name.npos) {
        found = name.find(" ", found + 1, 1);
        if (found != name.npos)
            throw UserError ("CharNameInvalidMaxSpaces");
    }

    std::string eName;
    sDatabase.DoEscapeString(eName, name.c_str());
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT characterID FROM chrCharacters WHERE characterName LIKE '%s' ", eName.c_str() );
    if (res.GetRowCount() > 0)  // name exists
        throw UserError ("CharNameInvalidTaken");

    /* if we got here, the name is good */
}

void CharacterDB::AddEmployment(uint32 charID, uint32 corpID, uint32 oldCorpID/*0*/) {
    // Add new employment history record and update character's corp and start date   -allan  25Mar14
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO chrEmployment"
        "  (characterID, corporationID, startDate, deleted)"
        " VALUES (%u, %u, %f, 0)", charID, corpID, GetFileTimeNow()))
    {
        codelog(DATABASE__ERROR, "Error in employment insert query: %s", err.c_str());
    }

    if (!sDatabase.RunQuery(err, "UPDATE chrCharacters SET startDateTime = %f, corporationID = %u WHERE characterID = %u", GetFileTimeNow(), corpID, charID))
        codelog(DATABASE__ERROR, "Error in character insert query: %s", err.c_str());

    // Decrease previous corp's member count
    if (IsCorp(oldCorpID))
        if (!sDatabase.RunQuery(err, "UPDATE crpCorporation SET memberCount = memberCount-1 WHERE corporationID = %u", oldCorpID))
            codelog(CORP__DB_ERROR, "Error in prev corp member decrease query: %s", err.c_str());

    // Increase new corp's member number...
    if (!sDatabase.RunQuery(err, "UPDATE crpCorporation SET memberCount = memberCount+1 WHERE corporationID = %u", corpID))
        codelog(CORP__DB_ERROR, "Error in new corp member increase query: %s", err.c_str());
}

PyRep *CharacterDB::GetCharSelectInfo(uint32 characterID) {
    //  this shows char on select screen....fixed/updated  -allan 20Jan15
    std::string shipName = "My Ship";
    uint32 shipTypeID = 606;  //arbitrary default.

    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT itemName, typeID FROM entity WHERE itemID = (SELECT shipID FROM chrCharacters WHERE characterID = %u)", characterID)) {
        _log(CHARACTER__WARNING, "Unable to get current ship: %s", res.error.c_str());
    } else {
        DBResultRow row;
        /** @todo  need to make proper error here. */
        // this causes blanks on char sel screen if there is no ship, or shipID is wrong.
        if (!res.GetRow(row))
            return PyStatic.NewNone();

        sDatabase.DoEscapeString(shipName, row.GetText(0));
        shipTypeID = row.GetUInt(1);
    }

    /** @todo  not sure how to implement these yet... */
    // these show on char portrait at top left on sel screen
    uint32 unreadMailCount = 0;
    uint32 upcomingEventCount = 0;
    uint32 unprocessedNotifications = 0;

    //res.Reset();
    if (!sDatabase.RunQuery(res,
        "SELECT " // fixed DB query per client code -allan 18Jan15
        "  %u AS unreadMailCount,"
        "  %u AS upcomingEventCount, "
        "  %u AS unprocessedNotifications, "
        "  characterID,"
        "  petitionMessage, "
        "  gender, "
        "  bloodlineID, "
        "  createDateTime, "    //this is char create date
        "  startDateTime, "     //this is char joined corp date
        "  corporationID, "
        "  0 AS worldSpaceID, " /* this gives "walking around in [station xxxx]" msgs on login screen when !=0 */
        "  stationID, "
        "  solarSystemID, "
        "  constellationID, "
        "  regionID, "
        "  0 AS allianceID, "       /** @todo (allan) reset this once alliances are implemented (already in corpDB) */
        "  0 AS allianceMemberStartDate,"
        "  'none' AS shortName, "   /** @todo (allan) this is alliance tickerName. reset this once alliances are implemented */
        "  bounty, "
        "  skillQueueEndTime, "
        "  skillPoints, "
        "  %u AS shipTypeID, "
        "  '%s' AS shipName, "
        "  securityRating, "
        "  title, "
        "  balance, "
        "  aurBalance, "
        "  15 AS daysLeft, "    /* this calls a subscription renewal warning on char select screen (see pic in gallery) when <= 10 */
        "  30 AS userType,"     /* 23 is trial acct.  30 is normal */
        "  paperDollState"      // used for re-customization.  see paperDollState:: in packet_types.h
        " FROM chrCharacters"
        " WHERE characterID=%u",
        unreadMailCount, upcomingEventCount, unprocessedNotifications,
        shipTypeID, shipName.c_str(), characterID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep *CharacterDB::GetCharPublicInfo(uint32 characterID) {
    if (IsAgent(characterID)) {
        sLog.Error("CharacterDB::GetCharPublicInfo()", "Character %u is NPC.", characterID);
        return nullptr;
    }
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT "       // fixed DB Query   -allan 11Jan14   -update 20April16
        "  ch.typeID,"
        "  ch.characterName,"
        "  ch.corporationID,"
        "  ch.raceID,"
        "  ch.bloodlineID,"
        "  ch.ancestryID,"
        "  ch.careerID,"
        "  ch.schoolID,"
        "  cs.schoolNameID, "
        "  ch.careerSpecialityID,"
        "  ch.age,"
        "  ch.createDateTime,"
        "  ch.gender,"
        "  ch.characterID,"
        "  ch.description,"
        "  ch.startDateTime"
        " FROM chrCharacters AS ch"
        "  LEFT JOIN chrSchools AS cs USING (schoolID) "
        " WHERE ch.characterID=%u", characterID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Error in GetCharPublicInfo query: no data for char %d", characterID);
        return nullptr;
    }

    return(DBRowToKeyVal(row));
}

bool CharacterDB::GetCharacterData(uint32 characterID, CharacterData &into) {
    DBQueryResult res;

    if (IsAgent(characterID)) {
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "   0 as accountID,"
            "   title,"
            "   description,"
            "   gender,"
            "   bounty,"
            "   0 as balance,"
            "   0 as aurBalance,"
            "   securityRating,"
            "   0 as logonMinutes,"
            "   stationID,"
            "   solarSystemID,"
            "   constellationID,"
            "   regionID,"
            "   ancestryID,"
            "   0 AS bloodlineID,"      /** @todo fix these */
            "   0 AS raceID,"
            "   careerID,"
            "   schoolID,"
            "   careerSpecialityID,"
            "   createDateTime,"
            "   0 as shipID,"       // update this when agents are in space
            "   0 as capsuleID,"
            "   flag,"
            "   name,"
            "   0 AS skillPoints,"
            "   typeID"
            " FROM chrNPCCharacters AS chr"
            " WHERE characterID = %u", characterID)) {
            codelog(DATABASE__ERROR, "Error in GetCharacter query: %s", res.error.c_str());
            return false;
            }
    } else {
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "   accountID,"
            "   title,"
            "   description,"
            "   gender,"
            "   bounty,"
            "   balance,"
            "   aurBalance,"
            "   securityRating,"
            "   logonMinutes,"
            "   stationID,"
            "   solarSystemID,"
            "   constellationID,"
            "   regionID,"
            "   ancestryID,"
            "   bloodlineID,"
            "   raceID,"
            "   careerID,"
            "   schoolID,"
            "   careerSpecialityID,"
            "   createDateTime,"
            "   shipID,"
            "   capsuleID,"
            "   flag,"
            "   characterName,"
            "   skillPoints,"
            "   typeID"
            " FROM chrCharacters"
            " WHERE characterID = %u", characterID))
        {
            codelog(DATABASE__ERROR, "Error in GetCharacter query: %s", res.error.c_str());
            return false;
        }
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No data found for character %u.", characterID);
        return false;
    }

    into.accountID = row.IsNull( 0 ) ? 0 : row.GetUInt( 0 );
    into.title = row.GetText( 1 );
    into.description = row.GetText( 2 );
    into.gender = row.GetInt( 3 ) ? true : false;
    into.bounty = row.GetDouble( 4 );
    into.balance = row.GetDouble( 5 );
    into.aurBalance = row.GetDouble( 6 );
    into.securityRating = row.GetDouble( 7 );
    into.logonMinutes = row.GetUInt( 8 );
    into.stationID = row.GetUInt( 9 );
    into.solarSystemID = row.GetUInt( 10 );
    into.locationID = (into.stationID == 0 ? into.solarSystemID : into.stationID);
    into.constellationID = row.GetUInt( 11 );
    into.regionID = row.GetUInt( 12 );
    into.ancestryID = row.GetUInt( 13 );
    into.bloodlineID =  row.GetUInt( 14 );
    into.raceID = row.GetUInt( 15 );
    into.careerID =row.GetUInt( 16 );
    into.schoolID = row.GetUInt( 17 );
    into.careerSpecialityID = row.GetUInt( 18 );
    into.createDateTime = row.GetInt64( 19 );
    into.shipID = row.GetUInt( 20 );
    into.capsuleID = row.GetUInt( 21 );
    into.flag = row.GetUInt(22);
    into.name = row.GetText(23);
    into.skillPoints = row.GetUInt(24);
    into.typeID = row.GetUInt(25);

    return true;
}

bool CharacterDB::GetCharCorpData(uint32 characterID, CorpData &into) {
    DBQueryResult res;
    DBResultRow row;

    if (IsAgent(characterID)) {
        into.corpAccountKey = 1001;

        if (!sDatabase.RunQuery(res,
            "SELECT corporationID, locationID"
            " FROM agtAgents"
            " WHERE agentID = %u",
            characterID))
        {
            codelog(DATABASE__ERROR, "Failed to query corp member info of character %u: %s.", characterID, res.error.c_str());
            return false;
        }

        if (!res.GetRow(row)) {
            _log(DATABASE__MESSAGE, "No corp member info found for character %u.", characterID);
            return false;
        }
        into.corporationID = row.GetInt(0);
        into.baseID = row.GetInt(1);
    } else {
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  startDateTime,"
            "  corporationID,"
            "  corpAccountKey,"
            "  corpRole,"
            "  rolesAtAll,"
            "  rolesAtBase,"
            "  rolesAtHQ,"
            "  rolesAtOther,"
            "  grantableRoles,"
            "  grantableRolesAtBase,"
            "  grantableRolesAtHQ,"
            "  grantableRolesAtOther,"
            "  baseID"
            " FROM chrCharacters"
            " WHERE characterID = %u",
            characterID))
        {
            codelog(DATABASE__ERROR, "Failed to query corp member info of character %u: %s.", characterID, res.error.c_str());
            return false;
        }
        if (!res.GetRow(row)) {
            _log(DATABASE__MESSAGE, "No corp member info found for character %u.", characterID);
            return false;
        }

        into.startDateTime = row.GetInt64(0);
        into.corporationID = row.GetInt(1);
        into.corpAccountKey = row.GetInt(2);
        into.corpRole = row.GetInt64(3);
        into.rolesAtAll = row.GetInt64(4);
        into.rolesAtBase = row.GetInt64(5);
        into.rolesAtHQ = row.GetInt64(6);
        into.rolesAtOther = row.GetInt64(7);
        into.grantableRoles = row.GetInt64(8);
        into.grantableRolesAtBase = row.GetInt64(9);
        into.grantableRolesAtHQ = row.GetInt64(10);
        into.grantableRolesAtOther = row.GetInt64(11);
        into.baseID = row.GetInt(12);
    }

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  taxRate,"
        "  stationID,"
        "  allianceID,"
        "  warFactionID,"
        "  corporationName,"
        "  tickerName"
        " FROM crpCorporation"
        " WHERE corporationID = %u", into.corporationID))
    {
        codelog(DATABASE__ERROR, "Failed to query HQ of character's %u corporation %u: %s.", characterID, into.corporationID, res.error.c_str());
        return false;
    }

    if (!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No HQ found for character's %u corporation.", characterID);
        return false;
    }

    into.taxRate = row.GetDouble(0);
    into.corpHQ = (row.IsNull(1) ? 0 : row.GetUInt(1));
    into.allianceID = (row.IsNull(2) ? 0 : row.GetUInt(2));
    into.warFactionID = (row.IsNull(3) ? 0 : row.GetUInt(3));
    into.name = row.GetText(4);
    into.ticker = row.GetText(5);

    return true;
}

void CharacterDB::GetCharacterDataMap(uint32 charID, std::map<std::string, int64> &characterDataMap) {
    DBQueryResult res;
    DBResultRow row;

    if (!sDatabase.RunQuery(res,
        "SELECT "       // fixed DB Query   -allan 11Jan14      UD: 04Dec17
        "  ch.corporationID, "
        "  ch.stationID, "
        "  ch.solarSystemID, "
        "  ch.constellationID, "
        "  ch.regionID, "
        "  co.stationID, "  //5
        "  ch.corpRole, "
        "  ch.corpAccountKey, "
        "  ch.rolesAtAll, "
        "  ch.rolesAtBase, "
        "  ch.rolesAtHQ, "  //10
        "  ch.rolesAtOther, "
        "  ch.shipID, "
        "  ch.gender, "
        "  ch.bloodlineID, "
        "  ch.raceID, "     //15
        "  ch.locationID, "
        "  ch.baseID,"
        "  co.allianceID,"
        "  co.warFactionID,"
        "  ch.characterName"         //20
        " FROM chrCharacters AS ch"
        "    LEFT JOIN crpCorporation AS co USING (corporationID) "
        " WHERE characterID = %u", charID))
    {
        sLog.Error("CharacterDB::GetCharacterDataMap()", "Failed to query HQ of character's %u corporation: %s.", charID, res.error.c_str());
    }

    if (!res.GetRow(row))
    {
        sLog.Error("CharacterDB::GetCharacterDataMap()", "No valid rows were returned by the database query.");
        return;
    }

    characterDataMap["corporationID"] = row.GetUInt(0);
    characterDataMap["stationID"] = row.GetUInt(1);
    characterDataMap["solarSystemID"] = row.GetUInt(2);
    characterDataMap["constellationID"] = row.GetUInt(3);
    characterDataMap["regionID"] = row.GetUInt(4);
    characterDataMap["corporationHQ"] = row.GetUInt(5);
    characterDataMap["corpRole"] = row.GetInt64(6);
    characterDataMap["corpAccountKey"] = row.GetInt(7);
    characterDataMap["rolesAtAll"] = row.GetInt64(8);
    characterDataMap["rolesAtBase"] = row.GetInt64(9);
    characterDataMap["rolesAtHQ"] = row.GetInt64(10);
    characterDataMap["rolesAtOther"] = row.GetInt64(11);
    characterDataMap["shipID"] = row.GetUInt(12);
    characterDataMap["gender"] = row.GetInt(13);
    characterDataMap["bloodlineID"] = row.GetUInt(14);
    characterDataMap["raceID"] = row.GetUInt(15);
    characterDataMap["locationID"] = row.GetUInt(16);
    characterDataMap["baseID"] = row.GetInt(17);
    characterDataMap["allianceID"] = row.GetInt(18);
    characterDataMap["warFactionID"] = row.GetInt(19);

    uint32 stationID = row.GetInt(17);
    if (!CharacterDB::GetCharHomeStation(charID, stationID)) {
        ItemData iData( itemCloneAlpha, charID, stationID, flagClone, 1 );
        iData.customInfo="Active: ";
        iData.customInfo += row.GetText(20);
        iData.customInfo += "(";
        iData.customInfo += std::to_string(charID);
        iData.customInfo += ") {ud}";
        sItemFactory.SpawnItem( iData )->SaveItem();
    }
    characterDataMap["cloneStationID"] = stationID;
}

PyRep* CharacterDB::GetCharPublicInfo3(uint32 charID) {
    // bounty, title, startDateTime, description, corporationID
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT "
        "  bounty,"
        "  title,"
        "  startDateTime,"
        "  description,"
        "  corporationID"
        " FROM chrCharacters "
        " WHERE characterID=%u", charID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyRep* CharacterDB::GetCharPrivateInfo(uint32 charID) {
    // characterID, gender, raceID, bloodlineID, createDateTime
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  gender,"
        "  createDateTime,"
        "  raceID,"
        "  bloodlineID"
        " FROM chrCharacters "
        " WHERE characterID=%u", charID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return nullptr;
    return DBRowToPackedRow(row);
}

PyRep *CharacterDB::GetInfoWindowDataForChar(uint32 characterID) {
    //corpID, allianceID, title
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT "
        "  ch.corporationID AS corpID,"
        "  co.allianceID,"
        "  ch.title"
        " FROM chrCharacters AS ch"
        "    LEFT JOIN crpCorporation AS co USING (corporationID)"
        " WHERE characterID=%u", characterID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Expected at least one row when getting character corp info\n");
        return nullptr;
    }

    return DBRowToKeyVal(row);
}

std::string CharacterDB::GetCharName(uint32 characterID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT characterName FROM chrCharacters WHERE characterID=%u", characterID)) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return "";
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "Name not found for CharacterID %u.", characterID);
        return "";
    }

    return row.GetText(0);
}

void CharacterDB::SetCharacterOnlineStatus(uint32 char_id, bool online) {
    _log(CLIENT__TRACE, "CharacterDB:  Setting character %u %s.", char_id, online ? "Online" : "Offline");
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE chrCharacters SET online = %u WHERE characterID = %u", (online?1:0), char_id);

    if ( online )
        sDatabase.RunQuery(err, "UPDATE srvStatus SET Connections = Connections + 1");
}

PyRep* CharacterDB::GetContacts(uint32 charID, bool blocked)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery( res,
        "SELECT contactID, inWatchlist, relationshipID, labelMask"
        " FROM chrContacts WHERE ownerID = %u and blocked = %u", charID, blocked))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

void CharacterDB::AddContact(uint32 ownerID, uint32 charID, int32 standing, bool inWatchlist)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO chrContacts (ownerID, contactID, relationshipID, "
        " inWatchlist, labelMask, blocked) VALUES "
        " (%u, %u, %i, %i, 0, 0) ",
        ownerID, charID, standing, inWatchlist);
}

void CharacterDB::UpdateContact(int32 standing, uint32 charID, uint32 ownerID)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE chrContacts SET relationshipID=%i "
        " WHERE contactID=%u AND ownerID=%u ",
         standing, charID, ownerID);
}

void CharacterDB::SetBlockContact(uint32 charID, uint32 ownerID, bool blocked)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE chrContacts SET blocked=%u "
        " WHERE contactID=%u AND ownerID=%u ",
        blocked, charID, ownerID);
}

void CharacterDB::RemoveContact(uint32 charID, uint32 ownerID)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "DELETE from chrContacts "
        " WHERE contactID=%u AND ownerID=%u ",
         charID, ownerID);
}

//just return all itemIDs which has ownerID set to characterID
bool CharacterDB::GetCharItems(uint32 characterID, std::vector<uint32> &into) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  itemID"
        " FROM entity"
        " WHERE ownerID = %u",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to query items of char %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while(res.GetRow(row))
        into.push_back(row.GetUInt(0));

    return true;
}

uint32 CharacterDB::PickAlternateShip(uint32 charID, uint32 locationID)
{   // this picks first ship that db finds belonging to charID in locationID
    DBQueryResult res;
    sDatabase.RunQuery(res,
        "SELECT e.itemID"
        " FROM entity AS e"
        "  LEFT JOIN invTypes USING (typeID)"
        "  LEFT JOIN invGroups USING (groupID)"
        " WHERE e.ownerID = %u"
        "  AND locationID = %u"
        "  AND categoryID = %u"
        "  LIMIT 1", charID, locationID, EVEDB::invCategories::Ship);

    DBResultRow row;
    if (res.GetRow(row)) {
        return row.GetUInt(0);
    } else {
        return 0;
    }
}

void CharacterDB::SetCurrentShip(uint32 charID, uint32 shipID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE chrCharacters SET shipID = %u WHERE characterID = %u", shipID, charID))
        _log(DATABASE__ERROR, "Failed to save ship %u for character %u: %s.", shipID, charID, err.c_str());
}

void CharacterDB::SetCurrentPod(uint32 charID, uint32 podID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE chrCharacters SET capsuleID = %u WHERE characterID = %u", podID, charID))
        _log(DATABASE__ERROR, "Failed to save pod %u for character %u: %s.", podID, charID, err.c_str());
}

//returns a list of the itemID for all the clones belonging to the character
bool CharacterDB::GetCharClones(uint32 characterID, std::vector<uint32> &into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT itemID FROM entity WHERE ownerID = %u AND flag='400'", characterID)) {
        _log(DATABASE__ERROR, "Failed to query clones of char %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while(res.GetRow(row))
        into.push_back(row.GetUInt(0));

    return true;
}

//returns the itemID of the active clone
bool CharacterDB::GetActiveCloneID(uint32 characterID, uint32 &itemID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT itemID"
        " FROM entity"
        " WHERE ownerID = %u"
        "  AND flag=400",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to query active clone of char %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (res.GetRow(row)) {
        itemID=row.GetUInt(0);
    } else {
        return false;
    }
    return true;
}

//we use this function because, when we change the clone type,
//the cached item type doesn't change, so we need to read it
//directly from the db
bool CharacterDB::GetActiveCloneType(uint32 characterID, uint32 &typeID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT typeID"
        " FROM entity"
        " WHERE ownerID = %u"
        "  AND flag=400",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to query active clone of char %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (res.GetRow(row)) {
        typeID=row.GetUInt(0);
        return true;
    }

    typeID = 0;
    return false;
}

// Return the Home station of the char based on the active clone
bool CharacterDB::GetCharHomeStation(uint32 characterID, uint32 &stationID) {
	DBQueryResult res;
	if ( !sDatabase.RunQuery(res,
        "SELECT locationID "
        " FROM entity"
        " WHERE ownerID = %u"
        "  AND flag=400",
        characterID ))
	{
        _log(CHARACTER__ERROR, "Could't get clone location for char %u", characterID );
		return false;
	}

	DBResultRow row;
    if (res.GetRow(row))
        stationID = row.GetUInt(0);
	return true;
}

//replace all the typeID of the character's clones
bool CharacterDB::ChangeCloneType(uint32 characterID, uint32 typeID)
{
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT "
        " typeName "
        "FROM invTypes "
        "WHERE typeID = %u",
        typeID))
    {
        _log(DATABASE__ERROR, "Failed to change clone type of char %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row))
    {
        sLog.Error( "CharacterDB::ChangeCloneType()", "Could not find Clone typeID = %u in invTypes table.", typeID );
        return false;
    }
    std::string typeNameString = row.GetText(0);

    if (!sDatabase.RunQuery(res.error,
        "UPDATE "
        "entity "
        "SET typeID=%u, itemName='%s' "
        "WHERE ownerID=%u "
        "AND flag=400",
        typeID,
        typeNameString.c_str(),
                           characterID))
    {
        _log(DATABASE__ERROR, "Failed to change clone type of char %u: %s.", characterID, res.error.c_str());
        return false;
    }
    sLog.Debug( "CharacterDB", "Clone upgrade successful" );
    return true;
}

bool CharacterDB::ChangeCloneLocation(uint32 characterID, uint32 locationID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res.error, "UPDATE entity SET locationID=%u WHERE ownerID=%u AND flag=400", locationID, characterID)) {
        _log(DATABASE__ERROR, "Failed to change location of clone %u: %s.", characterID, res.error.c_str());
        return false;
    }

    return true;
}

bool CharacterDB::GetAttributesFromAttributes(uint8 &intelligence, uint8 &charisma, uint8 &perception, uint8 &memory, uint8 &willpower) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT "
        "  attributeID, baseAttribute "
        " FROM chrAttributes "))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return (false);
    }
        
    for(int i = 0; i < res.GetRowCount(); i++){
        DBResultRow row;
        uint32 attributeID = 0;

        if (!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Failed to find attribute information for attribute %d", attributeID);
            return false;
        }

        attributeID = row.GetUInt(0);

        switch (attributeID)
        {
        case (uint32)1:
            intelligence += row.GetUInt(1);
            break;
        case (uint32)2:
            charisma += row.GetUInt(1);
            break;
        case (uint32)3:
            perception += row.GetUInt(1);
            break;
        case (uint32)4:
            memory += row.GetUInt(1);
            break;
        case (uint32)5:
            willpower += row.GetUInt(1);
            break;
        default:
            codelog(DATABASE__ERROR, "Unknown attribute %d", attributeID); 
            break;
        }
    }
    
    return (true);
}

bool CharacterDB::GetCareerBySchool(uint32 schoolID, uint8 &raceID, uint32 &careerID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
     "SELECT raceID, careerID FROM careers WHERE schoolID = %u", schoolID))  {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Failed to find matching career for school %u", schoolID);
        return false;
    }
    raceID = row.GetInt(0);
    careerID = row.GetInt(1);
    return true;
}

uint32 CharacterDB::GetStartingStationByCareer(uint32 careerID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                "SELECT "
                "  co.stationID"
                " FROM careers AS c"
                "    LEFT JOIN chrSchools AS cs USING (schoolID)"
                "    LEFT JOIN crpCorporation AS co ON cs.corporationID = co.corporationID"
                " WHERE c.careerID = %u", careerID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Failed to find station for career %u", careerID);
        return 0;
    }

    return row.GetUInt(0);
}

void CharacterDB::SetAvatar(uint32 charID, PyRep* hairDarkness) {
	//populate the DB with avatar information
	DBerror err;
	if (!sDatabase.RunQuery(err,
		"INSERT INTO avatars (charID, hairDarkness)"
		" VALUES (%u, %f)",
		charID, hairDarkness->AsFloat()->value()))
	{
		codelog(DATABASE__ERROR, "Error in query: %s", err.c_str());
	}
}

void CharacterDB::SetAvatarColors(uint32 charID, uint32 colorID, uint32 colorNameA, uint32 colorNameBC, double weight, double gloss) {
	//add avatar colors to the DB
	DBerror err;
	if (!sDatabase.RunQuery(err,
		"INSERT INTO avatar_colors (charID, colorID, colorNameA, colorNameBC, weight, gloss)"
		" VALUES (%u, %u, %u, %u, %f, %f)",
		charID, colorID, colorNameA, colorNameBC, weight, gloss))
	{
		codelog(DATABASE__ERROR, "Error in query: %s", err.c_str());
	}
}

void CharacterDB::SetAvatarModifiers(uint32 charID, PyRep* modifierLocationID,  PyRep* paperdollResourceID, PyRep* paperdollResourceVariation) {
	//add avatar modifiers to the DB
	DBerror err;
	if (!sDatabase.RunQuery(err,
		"INSERT INTO avatar_modifiers (charID, modifierLocationID, paperdollResourceID, paperdollResourceVariation)"
		" VALUES (%u, %u, %u, %u)",
		charID,
		modifierLocationID->AsInt()->value(),
		paperdollResourceID->AsInt()->value(),
		paperdollResourceVariation->IsInt() ? paperdollResourceVariation->AsInt()->value() : 0 ))
	{
		codelog(DATABASE__ERROR, "Error in query: %s", err.c_str());
	}
}

void CharacterDB::SetAvatarSculpts(uint32 charID, PyRep* sculptLocationID, PyRep* weightUpDown, PyRep* weightLeftRight, PyRep* weightForwardBack) {
	//add avatar sculpts to the DB
	DBerror err;
	if (!sDatabase.RunQuery(err,
		"INSERT INTO avatar_sculpts (charID, sculptLocationID, weightUpDown, weightLeftRight, weightForwardBack)"
		" VALUES (%u, %u, %f, %f, %f)",
		charID,
		sculptLocationID->AsInt()->value(),
		weightUpDown->IsFloat() ? weightUpDown->AsFloat()->value() : 0.0,
		weightLeftRight->IsFloat() ? weightLeftRight->AsFloat()->value() : 0.0,
		weightForwardBack->IsFloat() ? weightForwardBack->AsFloat()->value() : 0.0))
	{
		codelog(DATABASE__ERROR, "Error in query: %s", err.c_str());
	}
}

void CharacterDB::SetPortraitInfo(uint32 charID, PortraitInfo& data)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO chrPortraitData "
        " (charID, backgroundID, lightID, lightColorID, cameraX, cameraY, cameraZ, cameraPoiX, cameraPoiY, cameraPoiZ,"
        " headLookTargetX, headLookTargetY, headLookTargetZ, lightIntensity, headTilt, orientChar, browLeftCurl, browLeftTighten,"
        " browLeftUpDown, browRightCurl, browRightTighten, browRightUpDown, eyeClose, eyesLookVertical, eyesLookHorizontal,"
        " squintLeft, squintRight, jawSideways, jawUp, puckerLips, frownLeft, frownRight, smileLeft, smileRight, "
        " cameraFieldOfView, portraitPoseNumber) VALUES"
        " (%u,%u,%u,%u,%f,%f,%f,%f,%f,%f,"
        " %f,%f,%f,%f,%f,%f,%f,%f,"
        " %f,%f,%f,%f,%f,%f,%f,"
        " %f,%f,%f,%f,%f,%f,%f,%f,%f,"
        " %f,%f)",
        charID, data.backgroundID, data.lightID, data.lightColorID, data.cameraX, data.cameraY, data.cameraZ, data.cameraPoiX, data.cameraPoiY, data.cameraPoiZ,
        data.headLookTargetX, data.headLookTargetY, data.headLookTargetZ, data.lightIntensity, data.headTilt, data.orientChar, data.browLeftCurl, data.browLeftTighten,
        data.browLeftUpDown, data.browRightCurl, data.browRightTighten, data.browRightUpDown, data.eyeClose, data.eyesLookVertical, data.eyesLookHorizontal,
        data.squintLeft, data.squintRight, data.jawSideways, data.jawUp, data.puckerLips, data.frownLeft, data.frownRight, data.smileLeft, data.smileRight,
        data.cameraFieldOfView, data.portraitPoseNumber))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", err.c_str());
    }
}

bool CharacterDB::GetBaseSkills(std::map<uint32, uint8> &into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT skillTypeID, level FROM sklBaseSkills"))  {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    while (res.GetRow(row))
        into[row.GetUInt(0)] = row.GetUInt(1);

    return true;
}

bool CharacterDB::GetSkillsByRace(uint32 raceID, std::map<uint32, uint8> &into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT skillTypeID, level FROM sklRaceSkills WHERE raceID = %u ", raceID)) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    while (res.GetRow(row)) {
        if (into.find(row.GetUInt(0)) == into.end()) {
            into[row.GetUInt(0)] = row.GetUInt(1);
        } else {
            into[row.GetUInt(0)] += row.GetUInt(1);
        }
        //check to avoid more than 5 levels of a skill
        if (into[row.GetUInt(0)] > 5)
            into[row.GetUInt(0)] = 5;
    }

    return true;
}

bool CharacterDB::GetSkillsByCareer(uint32 careerID, std::map<uint32, uint8> &into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT "
        "  skillTypeID, level"
        " FROM sklCareerSkills"
        " WHERE careerID = %u", careerID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    while (res.GetRow(row)) {
        if (into.find(row.GetUInt(0)) == into.end()) {
            into[row.GetUInt(0)] = row.GetUInt(1);
        } else {
            into[row.GetUInt(0)] += row.GetUInt(1);
        }
        //check to avoid more than 5 levels of a skill
        if (into[row.GetUInt(0)] > 5)
            into[row.GetUInt(0)] = 5;
    }

    return true;
}

PyString *CharacterDB::GetNote(uint32 ownerID, uint32 itemID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT note FROM chrNotes WHERE ownerID = %u AND itemID = %u",  ownerID, itemID))  {
        codelog(DATABASE__ERROR, "Error on query: %s", res.error.c_str());
        return nullptr;
    }
    DBResultRow row;
    if (!res.GetRow(row))
        return nullptr;

    return (new PyString(row.GetText(0)));
}

bool CharacterDB::SetNote(uint32 ownerID, uint32 itemID, const char *str) {
    DBerror err;
    if (str[0] == '\0') {
        // str is empty
        if (!sDatabase.RunQuery(err,
            "DELETE FROM chrNotes "
            " WHERE itemID = %u AND ownerID = %u LIMIT 1",
             itemID, ownerID)
            )
        {
            codelog(DATABASE__ERROR, "Error on query: %s", err.c_str());
            return false;
        }
    } else {
        // escape it before insertion
        std::string escaped;
        sDatabase.DoEscapeString(escaped, str);

        if (!sDatabase.RunQuery(err,
            "REPLACE INTO chrNotes ( ownerID, itemID, note)"
            " VALUES (%u, %u, '%s')",
            ownerID, itemID, escaped.c_str())
            )
        {
            codelog(DATABASE__ERROR, "Error on query: %s", err.c_str());
            return false;
        }
    }

    return true;
}

uint32 CharacterDB::AddOwnerNote(uint32 charID, const std::string & label, const std::string & content) {
    DBerror err;
    uint32 id;
    std::string lblS;
    sDatabase.DoEscapeString(lblS, label);

    std::string contS;
    sDatabase.DoEscapeString(contS, content);

    if (!sDatabase.RunQueryLID(err, id,
        "INSERT INTO chrOwnerNote (ownerID, label, note) VALUES (%u, '%s', '%s');",
        charID, lblS.c_str(), contS.c_str()))
    {
        codelog(DATABASE__ERROR, "Error on query: %s", err.c_str());
        return 0;
    }

    return id;
}

bool CharacterDB::EditOwnerNote(uint32 charID, uint32 noteID, const std::string & label, const std::string & content) {
    DBerror err;
    std::string contS;
    sDatabase.DoEscapeString(contS, content);

    if (!sDatabase.RunQuery(err,
        "UPDATE chrOwnerNote SET note = '%s' WHERE ownerID = %u AND noteID = %u;",
        contS.c_str(), charID, noteID))
    {
        codelog(DATABASE__ERROR, "Error on query: %s", err.c_str());
        return false;
    }

    return true;
}

PyRep *CharacterDB::GetOwnerNoteLabels(uint32 charID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT noteID, label FROM chrOwnerNote WHERE ownerID = %u", charID))
    {
        codelog(DATABASE__ERROR, "Error on query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyRep *CharacterDB::GetOwnerNote(uint32 charID, uint32 noteID) {
    /*
                    [PyTuple 6 items]
                      [PyTuple 2 items]
                        [PyString "noteDate"]
                        [PyInt 64]
                      [PyTuple 2 items]
                        [PyString "typeID"]
                        [PyInt 2]
                      [PyTuple 2 items]
                        [PyString "referenceID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "note"]
                        [PyInt 130]
                      [PyTuple 2 items]
                        [PyString "userID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "label"]
                        [PyInt 130]
          [PyPackedRow 19 bytes]
            ["noteDate" => <129041092800000000> [FileTime]]
            ["typeID" => <1> [I2]]
            ["referenceID" => <1661059544> [I4]]
            ["note" => <1::F::0::Main|> [WStr]]
            ["userID" => <0> [I4]]
            ["label" => <S:Folders> [WStr]]
            */
    DBQueryResult res;

    if (!sDatabase.RunQuery(res, "SELECT note FROM chrOwnerNote WHERE ownerID = %u AND noteID = %u", charID, noteID))
    {
        codelog(DATABASE__ERROR, "Error on query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

void CharacterDB::EditLabel(uint32 charID, uint32 labelID, uint32 color, std::string name)
{
    std::string eName;
    sDatabase.DoEscapeString(eName, name);

    DBQueryResult res;
    sDatabase.RunQuery(res, "UPDATE chrLabels SET color = %u, name = '%s' WHERE ownerID = %u AND labelID = %u", color, eName.c_str(), charID, labelID);
}

PyRep* CharacterDB::GetLabels(uint32 charID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT labelID, color, name FROM chrLabels WHERE ownerID = %u", charID)) {
        codelog(DATABASE__ERROR, "Error on query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCIndexedRowset(res, "labelID");
}

void CharacterDB::SetLabel(uint32 charID, uint32 color, std::string name)
{
    std::string eName;
    sDatabase.DoEscapeString(eName, name);

    DBQueryResult res;
    sDatabase.RunQuery(res, "INSERT INTO chrLabels (color, name, ownerID) VALUES (%u, '%s', %u)", color, eName.c_str(), charID);
}

void CharacterDB::DeleteLabel(uint32 charID, uint32 labelID)
{
    DBerror err;
    sDatabase.RunQuery(err,"DELETE FROM chrLabels WHERE ownerID = %u AND labelID = %u", charID, labelID);
}

bool CharacterDB::LoadSkillQueue(uint32 characterID, SkillQueue &into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery( res, "SELECT typeID, level, startTime, endTime FROM chrSkillQueue WHERE characterID = %u ORDER BY orderIndex ASC", characterID)) {
        _log(DATABASE__ERROR, "Failed to query skill queue of character %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while (res.GetRow(row)) {
        QueuedSkill qs = QueuedSkill();
            qs.typeID = row.GetUInt(0);
            qs.level = row.GetUInt(1);
            qs.startTime = row.GetInt64(2);
            qs.endTime = row.GetInt64(3);
        into.push_back( qs );
    }

    return true;
}

bool CharacterDB::LoadPausedSkillQueue(uint32 characterID, SkillQueue &into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery( res, "SELECT typeID, level FROM chrPausedSkillQueue WHERE characterID = %u ORDER BY orderIndex ASC", characterID)) {
        _log(DATABASE__ERROR, "Failed to query paused skill queue of character %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while (res.GetRow(row)) {
        QueuedSkill qs = QueuedSkill();
            qs.typeID = row.GetUInt( 0 );
            qs.level = row.GetUInt( 1 );
        into.push_back( qs );
    }

    // delete paused queue, as subsequent presses of 'apply' button will add paused queue again, and again, etc...
    DBerror err;
    if (!sDatabase.RunQuery(err,"DELETE FROM chrPausedSkillQueue WHERE characterID = %u", characterID)) {
        _log(DATABASE__ERROR, "Failed to delete skill queue of character %u: %s.", characterID, err.c_str());
        return false;
    }

    return true;
}

bool CharacterDB::SaveSkillQueue(uint32 characterID, SkillQueue &data) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "DELETE FROM chrSkillQueue WHERE characterID = %u", characterID)) {
        _log(DATABASE__ERROR, "Failed to delete skill queue of character %u: %s.", characterID, err.c_str());
        return false;
    }

    if (data.empty())
        return true;

    std::string query;

    for (uint8 i = 0; i < data.size(); ++i) {
        const QueuedSkill &qs = data[i];
        char buf[80];
        snprintf(buf, sizeof(buf), "(%u, %u, %u, %u, %lli, %lli)", characterID, i, qs.typeID, qs.level, qs.startTime, qs.endTime );

        if (i > 0)
            query += ',';
        query += buf;
    }

    if (!sDatabase.RunQuery( err,
        "INSERT INTO chrSkillQueue (characterID, orderIndex, typeID, level, startTime, endTime)"
        " VALUES %s",query.c_str()))
    {
        _log(DATABASE__ERROR, "SaveSkillQueue - unable to save data - %s", err.c_str());
        return false;
    }
    return true;
}

bool CharacterDB::SavePausedSkillQueue(uint32 characterID, SkillQueue &data) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "DELETE FROM chrPausedSkillQueue WHERE characterID = %u", characterID)) {
        _log(DATABASE__ERROR, "Failed to delete paused skill queue of character %u: %s.", characterID, err.c_str());
        return false;
    }

    if (data.empty())
        return true;

    std::string query;

    for (uint8 i = 0; i < data.size(); ++i) {
        const QueuedSkill &qs = data[i];
        char buf[30];
        snprintf(buf, sizeof(buf), "(%u, %u, %u, %u)", characterID, i, qs.typeID, qs.level);

        if (i > 0)
            query += ',';
        query += buf;
    }

    if ( !sDatabase.RunQuery( err,
        "INSERT INTO chrPausedSkillQueue (characterID, orderIndex, typeID, level)"
        " VALUES %s",query.c_str()))
    {
        _log(DATABASE__ERROR, "SavePausedSkillQueue - unable to save data - %s", err.c_str());
        return false;
    }

    return true;
}

void CharacterDB::SaveSkillHistory(uint16 eventID, double logDate, uint32 characterID, uint32 skillTypeID, uint8 skillLevel, uint32 absolutePoints) {
    DBerror err;
    if ( !sDatabase.RunQuery( err,
        "INSERT INTO chrSkillHistory (eventTypeID, logDate, characterID, skillTypeID, skillLevel, absolutePoints)"
        " VALUES (%u, %f, %u, %u, %u, %u)", eventID, logDate, characterID, skillTypeID, skillLevel, absolutePoints ))
            _log(DATABASE__ERROR, "Failed to set chrSkillHistory for character %u: %s", characterID, err.c_str());
}

PyRep* CharacterDB::GetSkillHistory(uint32 characterID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT logDate, eventTypeID, skillTypeID, absolutePoints"
        " FROM chrSkillHistory"
        " WHERE characterID = %u"
        " ORDER BY logDate DESC"
        " LIMIT 50",
        characterID )) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

void CharacterDB::UpdateSkillQueueEndTime(int64 endtime, uint32 charID) {
    DBerror err;
    sDatabase.RunQuery( err, "UPDATE chrCharacters SET skillQueueEndTime = %lli WHERE characterID = %u ", endtime, charID );
}

void CharacterDB::SetLogInTime(uint32 charID)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE chrCharacters SET logonDateTime = %f WHERE characterID = %u", GetFileTimeNow(), charID );
}

void CharacterDB::SetLogOffTime(uint32 charID)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE chrCharacters SET logoffDateTime = %f WHERE characterID = %u", GetFileTimeNow(), charID );
}

void CharacterDB::AddOwnerCache(uint32 ownerID, std::string ownerName, uint32 typeID) {
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO cacheOwners(ownerID, ownerName, typeID)"
        " VALUES (%u, '%s', %u)",
        ownerID, ownerName.c_str(), typeID);
}

PyRep* CharacterDB::GetBounty(uint32 charID, uint32 ownerID) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT characterID, bounty FROM webBounties WHERE characterID = %u OR ownerID = %u", charID, ownerID);
    return DBResultToRowset(res);
}

PyRep* CharacterDB::GetTopBounties() {
    DBQueryResult res;
    sDatabase.RunQuery(res,
                       "SELECT c.characterID, c.bounty, c.online, o.characterName AS ownerName"
                       " FROM webBounties AS b"
                       " LEFT JOIN chrCharacters AS c ON c.characterID = b.characterID"
                       " LEFT JOIN chrCharacters AS o ON o.characterID = b.ownerID"
                       " ORDER BY c.bounty DESC"
                       " LIMIT 10");    /** @todo config variable? */
    return DBResultToRowset(res);
}

void CharacterDB::AddBounty(uint32 charID, uint32 ownerID, uint32 amount) {
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE chrCharacters SET bounty = bounty + %u WHERE characterID = %u", amount, charID);
    sDatabase.RunQuery(err,
        "INSERT INTO webBounties(characterID, ownerID, bounty, timePlaced)"
        " VALUES (%u, %u, %u, UNIX_TIMESTAMP(CURRENT_TIMESTAMP) )",
        charID, ownerID, amount );
}

PyRep* CharacterDB::GetKillOrLoss(uint32 charID) {
    /*
     *    def GetKillsRecentKills(self, num, startIndex):
     *        shipKills = sm.RemoteSvc('charMgr').GetRecentShipKillsAndLosses(num, startIndex)
     *        return [ k for k in shipKills if k.finalCharacterID == eve.session.charid ]
     *
     *    def GetKillsRecentLosses(self, num, startIndex):
     *        shipKills = sm.RemoteSvc('charMgr').GetRecentShipKillsAndLosses(num, startIndex)
     *        return [ k for k in shipKills if k.victimCharacterID == eve.session.charid ]
     */
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  killID,"
        "  solarSystemID,"
        "  victimCharacterID,"
        "  victimCorporationID,"
        "  victimAllianceID,"
        "  victimFactionID,"
        "  victimShipTypeID,"
        "  finalCharacterID,"
        "  finalCorporationID,"
        "  finalAllianceID,"
        "  finalFactionID,"
        "  finalShipTypeID,"
        "  finalWeaponTypeID,"
        "  killBlob,"
        "  killTime,"
        "  victimDamageTaken,"
        "  finalSecurityStatus,"
        "  finalDamageDone,"
        "  moonID"
        " FROM chrKillTable"
        " WHERE ((victimCharacterID = %u) OR (finalCharacterID = %u))", charID, charID))
        /* should we limit this? */
    {
        codelog(DATABASE__ERROR, "Error on query: %s", res.error.c_str());
        return nullptr;
    }

    _log(DATABASE__RESULTS, "GetKillOrLoss for %u returned %lu items", charID, res.GetRowCount());

    return DBResultToCRowset(res);
}

void CharacterDB::SetCorpRole(uint32 charID, int64 role)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE chrCharacters SET corpRole = %lli WHERE characterID = %u", role, charID);
}

int64 CharacterDB::GetCorpRole(uint32 charID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT corpRole FROM chrCharacters WHERE characterID = %u", charID)) {
        sLog.Error("CharacterDB::GetCorpRole()", "Failed to query role of character %u: %s.", charID, res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        sLog.Error("CharacterDB::GetCorpRole()", "No valid rows were returned by the database query.");
        return 0;
    }

    return row.GetUInt(0);
}

uint32 CharacterDB::GetCorpID(uint32 charID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT corporationID FROM chrCharacters WHERE characterID = %u", charID)) {
        sLog.Error("CharacterDB::GetCorpID()", "Failed to query corpID of character %u: %s.", charID, res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        sLog.Error("CharacterDB::GetCorpID()", "No valid rows were returned by the database query.");
        return 0;
    }

    return row.GetUInt(0);
}

float CharacterDB::GetCorpTaxRate(uint32 charID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT taxRate FROM crpCorporation WHERE corporationID = (SELECT corporationID FROM chrCharacters WHERE characterID = %u)", charID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return 0.0f;

    return row.GetFloat(0);
}

PyRep* CharacterDB::GetMyCorpMates(uint32 corpID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT solarSystemID AS locationID, characterID FROM chrCharacters WHERE corporationID = %u", corpID)) {
        sLog.Error("CharacterDB::GetMyCorpMates()", "Failed to query corpMates for corpID %u: %s.", corpID, res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

void CharacterDB::VisitSystem(uint32 solarSystemID, uint32 charID) {
    DBerror err;
    sDatabase.RunQuery(err,
            "INSERT INTO chrVisitedSystems (characterID, solarSystemID, visits, lastDateTime)"
            "VALUES (%u, %u, 1, %f)"
            " ON DUPLICATE KEY UPDATE"
            " visits = visits +1,"
            " lastDateTime = %f", charID, solarSystemID, GetFileTimeNow(), GetFileTimeNow());
}

PyRep* CharacterDB::List(uint32 ownerID)
{
    // maybe get all items owned by calling character?
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT "
        "  e.itemID, "
        "  e.itemName, "
        "  e.typeID, "
        "  e.ownerID, "
        "  e.locationID, "
        "  e.flag AS flagID, "
        "  e.quantity AS stacksize, "
        "  e.customInfo, "
        "  e.singleton, "
        "  g.categoryID, "
        "  g.groupID "
        "FROM entity AS e"
        "  LEFT JOIN invTypes USING (typeID)"
        "  LEFT JOIN invGroups AS g USING (groupID)"
        "WHERE e.ownerID=%u AND e.locationID < %u", ownerID, maxStation))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyRep* CharacterDB::ListStations(uint32 ownerID, std::ostringstream& flagIDs, bool forCorp/*false*/, bool bpOnly/*false*/)
{
    /** @todo check into this to see if we're querying POS modules(uk) also.
     * if so, we'll need to revise location checks and somehow fix it so customs offices (and anything else using flagHangar)
     *  doesnt show up, which leads to elusive "AttributeError: 'NoneType' object has no attribute 'solarSystemID'"  in client
     *  when locationID is NOT station
     */
    DBQueryResult res;
    /** @todo these queries are wrong....  (location and maybe owner)*/
    if (bpOnly) {
        if (forCorp) {
            // this is some funky shit to get correct stationID for corp bps in hangar, as their location is officeID, but we need stationID for this call
            if (!sDatabase.RunQuery(res,
                "SELECT o.stationID, COUNT(e.itemID) as blueprintCount"
                " FROM entity AS e"
                "  LEFT JOIN invTypes USING (typeID)"
                "  LEFT JOIN invGroups AS g USING (groupID)"
                "  LEFT JOIN staOffices as o ON o.itemID = e.locationID"
                " WHERE e.ownerID=%u AND e.flag IN (%s) AND g.categoryID = %u AND (e.locationID >= %u AND e.locationID <= %u)"
                " GROUP BY locationID", ownerID, flagIDs.str().c_str(), EVEDB::invCategories::Blueprint, minOffice, maxOffice))
            {
                codelog(SERVICE__ERROR, "Error in ListStations query: %s", res.error.c_str());
                return nullptr;
            }
        } else {
            if (!sDatabase.RunQuery(res,
                "SELECT e.locationID AS stationID, COUNT(e.itemID) as blueprintCount"
                " FROM entity AS e"
                "  LEFT JOIN invTypes USING (typeID)"
                "  LEFT JOIN invGroups AS g USING (groupID)"
                " WHERE e.ownerID=%u AND e.flag IN (%s) AND g.categoryID = %u AND e.locationID <= %u"
                " GROUP BY locationID", ownerID, flagIDs.str().c_str(), EVEDB::invCategories::Blueprint, maxStation))
            {
                codelog(SERVICE__ERROR, "Error in ListStations query: %s", res.error.c_str());
                return nullptr;
            }
        }
    } else {
        if (forCorp) {
            // this is some funky shit to get correct stationID for corp bps in hangar, as their location is officeID, but we need stationID for this call
            if (!sDatabase.RunQuery(res,
                "SELECT o.stationID, COUNT(e.itemID) as itemCount"
                " FROM entity AS e"
                "  LEFT JOIN staOffices as o ON o.itemID = e.locationID"
                " WHERE e.ownerID=%u AND e.flag IN (%s) AND (e.locationID >= %u AND e.locationID <= %u)"
                " GROUP BY locationID", ownerID, flagIDs.str().c_str(), minOffice, maxOffice))
            {
                codelog(SERVICE__ERROR, "Error in ListStations query: %s", res.error.c_str());
                return nullptr;
            }
        } else {
            if (!sDatabase.RunQuery(res,
                "SELECT locationID AS stationID, COUNT(itemID) as itemCount"
                " FROM entity WHERE ownerID=%u AND flag IN (%s) AND locationID <= %u"
                " GROUP BY locationID", ownerID, flagIDs.str().c_str(), maxStation))
            {
                codelog(SERVICE__ERROR, "Error in ListStations query: %s", res.error.c_str());
                return nullptr;
            }
        }
    }
    PyRep* rsp = DBResultToCRowset(res);
    if (is_log_enabled(CLIENT__RSP_DUMP))
        rsp->Dump(CLIENT__RSP_DUMP, "    ");
    return rsp;
}

PyRep* CharacterDB::ListStationItems(uint32 ownerID, uint32 stationID)
{
    /** @todo check into this to see if we're querying POS modules also */
    // some code shows 'copy' field here (for corp bp)
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT "
        "  e.itemID, "
        "  e.itemName, "
        "  e.typeID, "
        "  e.ownerID, "
        "  e.locationID, "
        "  e.flag AS flagID, "
        "  e.quantity AS stacksize, "
        "  e.customInfo, "
        "  e.singleton, "
        "  g.categoryID, "
        "  g.groupID "
        "FROM entity AS e"
        "  LEFT JOIN invTypes USING (typeID)"
        "  LEFT JOIN invGroups AS g USING (groupID)"
        "WHERE e.ownerID=%u AND e.locationID=%u AND e.flag=4",
        ownerID, stationID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    PyRep* rsp = DBResultToCRowset(res);
    if (is_log_enabled(CLIENT__RSP_DUMP))
        rsp->Dump(CLIENT__RSP_DUMP, "    ");
    return rsp;
}

PyRep* CharacterDB::ListStationBlueprintItems(uint32 ownerID, uint32 stationID, bool forCorp/*false*/)
{
    /** @todo check into this to see if we're querying POS modules also(uk) */
    DBQueryResult res;
    if (forCorp) {
        // do crazy shit here to get actual stationID/locationID of bp items in corp hangar
        if (!sDatabase.RunQuery(res,
            "SELECT "
            "  e.itemID, "
            "  e.itemName, "
            "  e.typeID, "
            "  e.ownerID, "
            "  o.stationID AS locationID, "
            "  e.flag AS flagID, "
            "  e.quantity AS stacksize, "
            //"  e.customInfo, "
            "  e.singleton, "
            //"  g.groupID, "
            //"  g.categoryID, "
            "  b.pLevel AS productivityLevel,"
            "  b.mLevel AS materialLevel,"
            "  b.copy,"
            "  b.runs AS licensedProductionRunsRemaining"
            " FROM entity AS e"
            "  LEFT JOIN invTypes USING (typeID)"
            "  LEFT JOIN invGroups AS g USING (groupID)"
            "  LEFT JOIN invBlueprints AS b USING (itemID)"
            "  LEFT JOIN staOffices as o ON e.locationID = o.itemID"
            " WHERE e.ownerID=%u AND o.stationID=%u AND g.categoryID = %u",
            ownerID, stationID, EVEDB::invCategories::Blueprint))
        {
            codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
            return nullptr;
        }
    } else {
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  e.itemID,"
            "  e.itemName,"
            "  e.typeID,"
            "  e.ownerID,"
            "  e.locationID,"
            "  e.flag AS flagID,"
            "  e.quantity AS stacksize,"
            "  e.customInfo,"
            "  e.singleton,"
            "  g.groupID,"
            "  g.categoryID,"
            "  b.pLevel AS productivityLevel,"
            "  b.mLevel AS materialLevel,"
            "  b.copy,"
            "  b.runs AS licensedProductionRunsRemaining"
            " FROM entity AS e"
            "  LEFT JOIN invTypes USING (typeID)"
            "  LEFT JOIN invGroups AS g USING (groupID)"
            "  LEFT JOIN invBlueprints AS b USING (itemID)"
            " WHERE e.ownerID=%u AND e.locationID=%u AND e.flag=4 AND g.categoryID = %u",
            ownerID, stationID, EVEDB::invCategories::Blueprint))
        {
            codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
            return nullptr;
        }
    }

    PyRep* rsp = DBResultToCRowset(res);
    if (is_log_enabled(CLIENT__RSP_DUMP))
        rsp->Dump(CLIENT__RSP_DUMP, "    ");
    return rsp;
}

uint8 CharacterDB::GetSkillLevel(uint32 charID, uint16 skillTypeID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT valueInt FROM entity_attributes WHERE attributeID= %u AND itemID= "
        "(SELECT itemID FROM entity WHERE locationID = %u AND typeID = %u)",
        AttrSkillLevel, charID, skillTypeID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (res.GetRow(row))
        return row.GetUInt(0);
    return 0;
}

bool CharacterDB::GetCharacterType(uint8 bloodlineID, CharacterTypeData &into) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  bloodlineName,"
        "  raceID,"
        "  description,"
        "  maleDescription,"
        "  femaleDescription,"
        "  corporationID,"
        "  shortDescription,"
        "  shortMaleDescription,"
        "  shortFemaleDescription "
        " FROM chrBloodlines "
        " WHERE bloodlineID = %u",
        bloodlineID))
    {
        codelog(DATABASE__ERROR, "Failed to query bloodline %u: %s.", bloodlineID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No data found for bloodline %u.", bloodlineID);
        return false;
    }

    into.bloodlineName = row.GetText(0);
    into.race = row.GetUInt(1);
    into.description = row.GetText(2);
    into.maleDescription = row.GetText(3);
    into.femaleDescription = row.GetText(4);
    into.corporationID = row.GetUInt(5);
    into.shortDescription = row.GetText(6);
    into.shortMaleDescription = row.GetText(7);
    into.shortFemaleDescription = row.GetText(8);
    return true;
}

bool CharacterDB::GetCharacterType(uint16 characterTypeID, uint8 &bloodlineID, CharacterTypeData &into) {
    if (!GetBloodlineByCharacterType(characterTypeID, bloodlineID))
        return false;
    return GetCharacterType(bloodlineID, into);
}

bool CharacterDB::GetCharacterTypeByBloodline(uint8 bloodlineID, uint16 &characterTypeID, CharacterTypeData &into) {
    if (!GetCharacterTypeByBloodline(bloodlineID, characterTypeID))
        return false;
    return GetCharacterType(bloodlineID, into);
}

bool CharacterDB::GetCharacterTypeByBloodline(uint8 bloodlineID, uint16& characterTypeID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT typeID FROM bloodlineTypes WHERE bloodlineID = %u", bloodlineID)) {
        codelog(DATABASE__ERROR, "Failed to query bloodline %u: %s.", bloodlineID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No data for bloodline %u.", bloodlineID);
        return false;
    }

    characterTypeID = row.GetUInt(0);

    return true;
}

bool CharacterDB::GetBloodlineByCharacterType(uint16 characterTypeID, uint8 &bloodlineID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT bloodlineID FROM bloodlineTypes WHERE typeID = %u", characterTypeID)) {
        codelog(DATABASE__ERROR, "Failed to query character type %u: %s.", characterTypeID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No data for character type %u.", characterTypeID);
        return false;
    }

    bloodlineID = row.GetUInt(0);

    return true;
}

PyRep *CharacterDB::GetCharacterShipFittings(uint32 charID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                            "SELECT id as fittingID, name, description, shipID FROM chrShipFittings WHERE characterID = %u", charID)) {
        _log(DATABASE__ERROR, "Error in GetFittings query: %s", res.error.c_str());
        return new PyDict();
    }

    PyDict *ret = new PyDict();
    DBResultRow row;
    while (res.GetRow(row)) {
        uint32 fittingID = row.GetInt(0);
        std::string name = row.GetText(1);
        std::string description = row.GetText(2);
        uint32 shipID = row.GetInt(3);

        auto *fitting = new PyDict();
        fitting->SetItemString("fittingID", new PyInt(fittingID));
        fitting->SetItemString("ownerID", new PyInt(charID));
        fitting->SetItemString("name", new PyWString(name));
        fitting->SetItemString("description", new PyWString(description));
        fitting->SetItemString("shipTypeID", new PyInt(shipID));

        auto *fitData = new PyList();
        if (DBQueryResult fitRes; sDatabase.RunQuery(fitRes,
                                                     "SELECT itemID, flagID, qty FROM shipFittings WHERE fittingID = %u", fittingID)) {
            DBResultRow fitRow;
            while (fitRes.GetRow(fitRow)) {
                fitData->AddItem(new_tuple(new PyInt(fitRow.GetInt(0)), new PyInt(fitRow.GetInt(1)), new PyInt(fitRow.GetInt(2))));
            }
        }

        fitting->SetItemString("fitData", fitData);
        ret->SetItem(new PyInt(fittingID), new PyObject("util.KeyVal", fitting));
    }
    return ret;
}

uint32_t CharacterDB::SaveCharShipFitting(PyDict &fitting, uint32_t ownerID) {
    uint32_t fittingID = 0;
    DBerror err;
    auto name = PyRep::StringContent(fitting.GetItemString("name"));
    auto description = PyRep::StringContent(fitting.GetItemString("description"));
    auto shipTypeID = fitting.GetItemString("shipTypeID")->AsInt()->value();
    if (!sDatabase.RunQueryLID(err, fittingID,
                               "INSERT INTO chrShipFittings (characterID, name, description, shipID) VALUES (%u, '%s', '%s', %u)",
                               ownerID,
                               name.c_str(),
                               description.c_str(),
                               shipTypeID)) {
        codelog(DATABASE__ERROR, "Error in SaveCharShipFitting query: %s", err.c_str());
        return 0;
    }
    auto fitData = fitting.GetItemString("fitData")->AsList();
    for (auto fitItem : *fitData) {
        auto item = fitItem->AsTuple()->GetItem(0)->AsInt()->value();
        auto flagID = fitItem->AsTuple()->GetItem(1)->AsInt()->value();
        auto qty = fitItem->AsTuple()->GetItem(2)->AsInt()->value();
        if (!sDatabase.RunQuery(err,
                                "INSERT INTO shipFittings (fittingID, itemID, flagID, qty) VALUES (%u, %u, %u, %u)",
                                fittingID, item, flagID, qty)) {
            codelog(DATABASE__ERROR, "Error in SaveCharShipFitting fitData query: %s", err.c_str());
            return 0;
        }
    }
    return fittingID;
}
