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
    Author:     Captnoord
*/

#include "eve-common.h"

#include "auth/PasswordModule.h"
#include "auth/ShaModule.h"

bool PasswordModule::GeneratePassHash(
    const std::string& user,
    const std::string& pass,
    std::string& hash )
{
    // Pass it to the next function
    return GeneratePassHash(
        user.c_str(), user.length(),
        pass.c_str(), pass.length(),
        hash );
}

bool PasswordModule::GeneratePassHash(
    const char* user, size_t userLen,
    const char* pass, size_t passLen,
    std::string& hash )
{
    // Convert username and password to UTF-16
    std::vector< uint16 > username, password;
    utf8::utf8to16( user, user + userLen,
                    std::back_inserter( username ) );
    utf8::utf8to16( pass, pass + passLen,
                    std::back_inserter( password ) );

    // Lowercase the username
    std::transform( username.begin(), username.end(),
                    username.begin(), ::tolower );

    // Find index of first non-space
    const size_t frontIndex =
        std::find_if( username.begin(), username.end(),
                      std::not1( std::ptr_fun( ::isspace ) ) )
        - username.begin();
    // Find reverse index of last non-space
    const size_t backIndex  =
        std::find_if( username.rbegin(), username.rend(),
                      std::not1( std::ptr_fun( ::isspace ) ) )
        - username.rbegin();

    // Trim the username
    username.erase( username.begin(),
                    username.begin() + frontIndex );
    username.erase( username.end() - backIndex,
                    username.end() );

    // Pass it to the next function
    return GeneratePassHash( username, password, hash );
}

bool PasswordModule::GeneratePassHash(
    const std::vector< uint16 >& user,
    const std::vector< uint16 >& pass,
    std::string& hash )
{
    // Pass it to the next function
    return GeneratePassHash(
        &user[0], user.size(),
        &pass[0], pass.size(),
        hash );
}

bool PasswordModule::GeneratePassHash(
    const uint16* user, size_t userLen,
    const uint16* pass, size_t passLen,
    std::string& hash )
{
    // Define byte pointers (due to readability)
    const uint8* _user    = reinterpret_cast< const uint8* >( user );
    size_t       _userLen = userLen * sizeof( uint16 );
    const uint8* _pass    = reinterpret_cast< const uint8* >( pass );
    size_t       _passLen = passLen * sizeof( uint16 );

    // Helper digest buffer
    uint8 digest[ SHA_DIGESTSIZE ];

    // Initialize the hash
    ShaModule::SHAobject shaObj;
    ShaModule::sha_init( &shaObj );
    ShaModule::sha_update( &shaObj, _pass, _passLen );
    ShaModule::sha_update( &shaObj, _user, _userLen );

    // The hashing loop
    for( size_t i = 0; i < 1000; ++i )
    {
        // Store the digest
        ShaModule::sha_digest( &shaObj, digest );

        // Rehash the whole stuff
        ShaModule::sha_init( &shaObj );
        ShaModule::sha_update( &shaObj, digest, SHA_DIGESTSIZE );
        ShaModule::sha_update( &shaObj, _user, _userLen );
    }

    // Obtain the resulting hash
    ShaModule::sha_final( digest, &shaObj );
    hash.assign(
        reinterpret_cast< char* >( digest ),
        SHA_DIGESTSIZE );

    return true;
}
