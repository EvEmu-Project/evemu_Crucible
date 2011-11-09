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
	Author:		Aknor Jaden
*/


#include "EVEServerPCH.h"


APICharacterManager::APICharacterManager(const PyServiceMgr &services)
: APIServiceManager(services)
{
}

std::tr1::shared_ptr<std::string> APICharacterManager::ProcessCall(const APICommandCall * pAPICommandCall)
{
    sLog.Debug("APIAdminManager::ProcessCall()", "EVEmu API - Character Service Manager");

    if( pAPICommandCall->find( "servicehandler" ) == pAPICommandCall->end() )
    {
        sLog.Error( "APICharacterManager::ProcessCall()", "Cannot find 'servicehandler' specifier in pAPICommandCall packet" );
        return std::tr1::shared_ptr<std::string>(new std::string(""));
    }

    if( pAPICommandCall->find( "servicehandler" )->second == "CharacterSheet.xml.aspx" )
        return _CharacterSheet(pAPICommandCall);
    //else if( pAPICommandCall->find( "servicehandler" )->second == "TODO.xml.aspx" )
    //    return _TODO(pAPICommandCall);
    else
    {
        sLog.Error("APIAdminManager::ProcessCall()", "EVEmu API - Admin Service Manager - ERROR: Cannot resolve '%s' as a valid service query for Admin Service Manager",
            pAPICommandCall->find("servicehandler")->second.c_str() );
        return std::tr1::shared_ptr<std::string>(new std::string(""));
    }
    sLog.Debug("APICharacterManager::ProcessCall()", "EVEmu API - Character Service Manager");

    return std::tr1::shared_ptr<std::string>(new std::string(""));
}

std::tr1::shared_ptr<std::string> APICharacterManager::_CharacterSheet(const APICommandCall * pAPICommandCall)
{

    sLog.Debug("APICharacterManager::_CharacterSheet()", "EVEmu API - Character Service Manager - CALL: CharacterSheet.xml.aspx");

    if( pAPICommandCall->find( "userid" ) == pAPICommandCall->end() )
    {
        sLog.Error( "APICharacterManager::_CharacterSheet()", "ERROR: No 'userID' parameter found in call argument list - exiting with error and sending back NOTHING" );
		return BuildErrorXMLResponse( "106", "Must provide userID parameter for authentication." );
    }

	if( pAPICommandCall->find( "apikey" ) == pAPICommandCall->end() )
    {
        sLog.Error( "APICharacterManager::_CharacterSheet()", "ERROR: No 'apiKey' parameter found in call argument list - exiting with error and sending back NOTHING" );
		return BuildErrorXMLResponse( "203", "Authentication failure." );
    }

	if( pAPICommandCall->find( "characterid" ) == pAPICommandCall->end() )
    {
        sLog.Error( "APICharacterManager::_CharacterSheet()", "ERROR: No 'characterID' parameter found in call argument list - exiting with error and sending back NOTHING" );
		return BuildErrorXMLResponse( "105", "Invalid characterID." );
    }

    // TODO: Implement calls in APICharacterDB class to grab all data for this call and populate the xml structure with that data
    uint32 characterID = atoi( pAPICommandCall->find( "characterid" )->second.c_str() );
    std::vector<std::string> skillTypeIDList;
    std::vector<std::string> skillPointsList;
    std::vector<std::string> skillLevelList;
    std::vector<std::string> skillPublishedList;
    m_charDB.GetCharacterSkillsTrained( characterID, skillTypeIDList, skillPointsList, skillLevelList, skillPublishedList );

    std::vector<std::string> charInfoList;
    m_charDB.GetCharacterInfo( characterID, charInfoList );

	//return BuildErrorXMLResponse( "9999", "EVEmu API Server: Character Manager - CharacterSheet.xml.aspx STUB" );

	// EXAMPLE:
    std::vector<std::string> rowset;
    _BuildXMLHeader();
    {
        _BuildXMLTag( "result" );
        {
            _BuildSingleXMLTag( "characterID", std::string(itoa(characterID)) );
            _BuildSingleXMLTag( "name", charInfoList.at(12) );
            _BuildSingleXMLTag( "DoB", Win32TimeToString(atol(charInfoList.at(8).c_str()) );
            _BuildSingleXMLTag( "race", charInfoList.at(11) );
            _BuildSingleXMLTag( "bloodLine", charInfoList.at(10) );
            _BuildSingleXMLTag( "ancestry", charInfoList.at(9) );
            _BuildSingleXMLTag( "gender", (charInfoList.at(14).compare("0") ? "Female" : "Male") );
            _BuildSingleXMLTag( "corporationName", charInfoList.at(13) );
            _BuildSingleXMLTag( "corporationID", charInfoList.at(2) );
            _BuildSingleXMLTag( "allianceName", "" );
            _BuildSingleXMLTag( "allianceID", "0" );
            _BuildSingleXMLTag( "cloneName", "Clone Grade Pi" );
            _BuildSingleXMLTag( "cloneSkillPoints", "54600000" );
            _BuildSingleXMLTag( "balance", charInfoList.at(0) );

            // Attribute Enhancers (implants)
            _BuildXMLTag( "attributeEnhancers" );
            {
                _BuildXMLTag( "memoryBonus" );
                {
                    _BuildSingleXMLTag( "augmentatorName", "Memory Augmentation - Basic" );
                    _BuildSingleXMLTag( "augmentatorValue", "3" );
                }
                _CloseXMLTag(); // close tag "memoryBonus"
                _BuildXMLTag( "perceptionBonus" );
                {
                    _BuildSingleXMLTag( "augmentatorName", "Ocular Filter - Basic" );
                    _BuildSingleXMLTag( "augmentatorValue", "3" );
                }
                _CloseXMLTag(); // close tag "perceptionBonus"
                _BuildXMLTag( "willpowerBonus" );
                {
                    _BuildSingleXMLTag( "augmentatorName", "Neural Boost - Basic" );
                    _BuildSingleXMLTag( "augmentatorValue", "3" );
                }
                _CloseXMLTag(); // close tag "willpowerBonus"
                _BuildXMLTag( "intelligenceBonus" );
                {
                    _BuildSingleXMLTag( "augmentatorName", "Snake Delta" );
                    _BuildSingleXMLTag( "augmentatorValue", "3" );
                }
                _CloseXMLTag(); // close tag "intelligenceBonus"
                _BuildXMLTag( "charismaBonus" );
                {
                    _BuildSingleXMLTag( "augmentatorName", "Limited Social Adaptation Chip" );
                    _BuildSingleXMLTag( "augmentatorValue", "3" );
                }
                _CloseXMLTag(); // close tag "charismaBonus"
            }
            _CloseXMLTag(); // close tag "attributeEnhancers"

            // Attributes
            _BuildXMLTag( "attributes" );
            {
                _BuildSingleXMLTag( "intelligence", "6" );
                _BuildSingleXMLTag( "memory", "4" );
                _BuildSingleXMLTag( "charisma", "7" );
                _BuildSingleXMLTag( "perception", "12" );
                _BuildSingleXMLTag( "willpower", "10" );
            }
            _CloseXMLTag(); // close tag "attributes"

            // Skills
            rowset.push_back("typeID");
            rowset.push_back("skillpoints");
            rowset.push_back("level");
            rowset.push_back("published");
            _BuildXMLRowSet( "skills", "typeID", &rowset );
            {
                for(int i=0; i<skillTypeIDList.size(); i++)
                {
                    rowset.clear();
                    rowset.push_back(skillTypeIDList.at(i));
                    rowset.push_back(skillPointsList.at(i));
                    rowset.push_back(skillLevelList.at(i));
                    rowset.push_back(skillPublishedList.at(i));
                    _BuildXMLRow( &rowset );
                }
            }
            _CloseXMLRowSet();  // close rowset "skills"

            // Certificates:
            rowset.push_back("certificateID");
            _BuildXMLRowSet( "certificates", "certificateID", &rowset );
            {
            }
            _CloseXMLRowSet();  // close rowset "certificates"

            // Corporation Roles:
            // TODO
        }
        _CloseXMLTag(); // close tag "result"
    }
    _CloseXMLHeader( EVEAPI::CacheStyles::Long );

    return _GetXMLDocumentString();
}
