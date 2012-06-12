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
    Author:     Bloody.Rabbit
*/

#include "eve-test.h"

// username
const char   USERNAME[]   = " Hello World ";
const size_t USERNAME_LEN = sizeof( USERNAME ) - 1;

// password
const char   PASSWORD[]   = " Secret Password ";
const size_t PASSWORD_LEN = sizeof( PASSWORD ) - 1;

// hash
const uint8 HASH[] =
{
    0x1b, 0xe6, 0xdf, 0x86, 0x42, 0x1d, 0xcf, 0x2c,
    0xfa, 0x71, 0x90, 0x0f, 0x34, 0x27, 0x75, 0xb0,
    0x31, 0x98, 0xdc, 0xe5
};
const size_t HASH_LEN = sizeof( HASH );

int auth_PasswordModuleTest( int argc, char* argv[] )
{
    // Print input data
    ::printf( "username='%s' (len=%lu)\n"
              "password='%s' (len=%lu)\n",
              USERNAME, USERNAME_LEN,
              PASSWORD, PASSWORD_LEN );

    std::wstring wUsername, wPassword;

    // Convert username
    wUsername.resize( USERNAME_LEN );
    if( USERNAME_LEN != ::mbstowcs(
            &wUsername[0],
            USERNAME, USERNAME_LEN ) )
    {
        ::perror( "Cannot convert username to wide string" );
        return EXIT_FAILURE;
    }

    // Convert password
    wPassword.resize( PASSWORD_LEN );
    if( PASSWORD_LEN != ::mbstowcs(
            &wPassword[0],
            PASSWORD, PASSWORD_LEN ) )
    {
        ::perror( "Cannot convert password to wide string" );
        return EXIT_FAILURE;
    }

    // Generate the hash
    std::string hash;
    if( !PasswordModule::GeneratePassHash(
            wUsername, wPassword,
            hash ) )
    {
        ::fprintf( stderr, "Cannot generate the hash" );
        return EXIT_FAILURE;
    }

    // Compare the lengths
    if( HASH_LEN != hash.length()
        // Compare the hashes
        || 0 != ::memcmp( hash.c_str(), HASH, HASH_LEN ) )
    {
        ::fprintf( stderr, "The computed and the reference"
                           " hashes do not match" );
        return EXIT_FAILURE;
    }

    ::puts( "Hash computation OK" );
    return EXIT_SUCCESS;
}
