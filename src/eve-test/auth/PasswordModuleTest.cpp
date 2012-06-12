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
const std::string USERNAME = " Hello World ";
// password
const std::string PASSWORD = " Secret Password ";
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
              USERNAME.c_str(), USERNAME.length(),
              PASSWORD.c_str(), PASSWORD.length() );

    // Generate the hash
    std::string hash;
    if( !PasswordModule::GeneratePassHash(
            USERNAME, PASSWORD, hash ) )
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
