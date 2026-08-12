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

    std::string invalidHash = "unexpected";
    if( PasswordModule::GeneratePassHash( "", PASSWORD, invalidHash ) ||
        !invalidHash.empty() )
    {
        ::fprintf( stderr, "Empty usernames must be rejected" );
        return EXIT_FAILURE;
    }

    invalidHash = "unexpected";
    if( PasswordModule::GeneratePassHash( USERNAME, "", invalidHash ) ||
        !invalidHash.empty() )
    {
        ::fprintf( stderr, "Empty passwords must be rejected" );
        return EXIT_FAILURE;
    }

    const std::string credential = "client credential";
    std::string verifier;
    if (!PasswordModule::GenerateArgon2idVerifier(credential, verifier)) {
        ::fprintf(stderr, "Argon2id verifier generation failed");
        return EXIT_FAILURE;
    }

    if (verifier.compare(
            0,
            sizeof("$argon2id$v=19$m=65536,t=3,p=1$") - 1,
            "$argon2id$v=19$m=65536,t=3,p=1$") != 0) {
        ::fprintf(stderr, "Argon2id verifier format is invalid");
        return EXIT_FAILURE;
    }

    if (!PasswordModule::VerifyArgon2idVerifier(credential, verifier) ||
        PasswordModule::VerifyArgon2idVerifier("wrong credential", verifier)) {
        ::fprintf(stderr, "Argon2id verifier comparison failed");
        return EXIT_FAILURE;
    }

    if (!PasswordModule::VerifyCredential(credential, credential, verifier) ||
        PasswordModule::VerifyCredential("wrong credential", credential,
                                         verifier)) {
        ::fprintf(stderr, "KDF-first credential verification failed");
        return EXIT_FAILURE;
    }

    std::string conflictingVerifier;
    if (!PasswordModule::GenerateArgon2idVerifier(
            "different credential", conflictingVerifier) ||
        PasswordModule::VerifyCredential(
            credential, credential, conflictingVerifier) ||
        !PasswordModule::VerifyCredential(credential, credential, "") ||
        PasswordModule::VerifyCredential("wrong credential", credential, "")) {
        ::fprintf(stderr, "Legacy credential fallback failed");
        return EXIT_FAILURE;
    }

    std::string randomHex;
    if (!PasswordModule::GenerateSecureRandomHex(32, randomHex) ||
        randomHex.size() != 64 ||
        PasswordModule::GenerateSecureRandomHex(0, randomHex) ||
        !randomHex.empty()) {
        ::fprintf(stderr, "Secure random hexadecimal generation failed");
        return EXIT_FAILURE;
    }

    std::string invalidVerifier = "unexpected";
    if (PasswordModule::GenerateArgon2idVerifier("", invalidVerifier) ||
        !invalidVerifier.empty() ||
        PasswordModule::VerifyArgon2idVerifier(credential, "unexpected")) {
        ::fprintf(stderr, "Invalid Argon2id inputs were accepted");
        return EXIT_FAILURE;
    }

    ::puts( "Hash and Argon2id computation OK" );
    return EXIT_SUCCESS;
}
