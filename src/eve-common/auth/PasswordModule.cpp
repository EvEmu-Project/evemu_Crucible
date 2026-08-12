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
    Author:     Captnoord
*/

#include "eve-common.h"

#include <array>
#include <fstream>
#include <string_view>

#ifdef _WIN32
#include <bcrypt.h>
#endif

#include <argon2.h>

#include "auth/PasswordModule.h"
#include "auth/ShaModule.h"

namespace {

constexpr std::size_t ARGON2ID_MEMORY_KIB = 65536u;
constexpr std::uint32_t ARGON2ID_TIME_COST = 3u;
constexpr std::uint32_t ARGON2ID_PARALLELISM = 1u;
constexpr std::size_t ARGON2ID_SALT_BYTES = 16u;
constexpr std::size_t ARGON2ID_HASH_BYTES = 32u;
constexpr std::size_t MAX_CREDENTIAL_BYTES = 256u;
constexpr std::size_t MAX_VERIFIER_BYTES = 128u;
constexpr std::size_t MAX_RANDOM_HEX_BYTES = 64u;
constexpr char ARGON2ID_VERIFIER_PREFIX[] =
    "$argon2id$v=19$m=65536,t=3,p=1$";

bool FillSecureRandom( uint8* buffer, std::size_t length )
{
#ifdef _WIN32
    return BCryptGenRandom(
        nullptr,
        buffer,
        static_cast<ULONG>( length ),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG ) == 0;
#elif defined( __unix__ ) || defined( __APPLE__ )
    std::ifstream randomDevice(
        "/dev/urandom",
        std::ios::in | std::ios::binary );
    if ( !randomDevice )
        return false;

    randomDevice.read(
        reinterpret_cast<char*>( buffer ),
        static_cast<std::streamsize>( length ) );
    return randomDevice.gcount() == static_cast<std::streamsize>( length );
#else
    (void)buffer;
    (void)length;
    return false;
#endif
}

bool HasSupportedVerifierFormat( const std::string& verifier )
{
    constexpr std::string_view prefix( ARGON2ID_VERIFIER_PREFIX );
    return verifier.size() <= MAX_VERIFIER_BYTES
        && verifier.find( '\0' ) == std::string::npos
        && verifier.size() > prefix.size()
        && verifier.compare( 0, prefix.size(), prefix ) == 0;
}

bool ConstantTimeEqual(
    const std::string& left,
    const std::string& right )
{
    if ( left.size() != right.size() )
        return false;

    unsigned char difference = 0;
    for ( std::size_t index = 0; index < left.size(); ++index )
        difference |= static_cast<unsigned char>(
            left[index] ^ right[index] );

    return difference == 0;
}

}

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
            [](int c) { return !std::isspace(c); })
        - username.begin();
    // Find reverse index of last non-space
    const size_t backIndex  =
        std::find_if( username.rbegin(), username.rend(),
            [](int c) { return !std::isspace(c); })
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
    if( user.empty() || pass.empty() )
    {
        hash.clear();
        return false;
    }

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

bool PasswordModule::GenerateArgon2idVerifier(
    const std::string& credential,
    std::string& verifier )
{
    verifier.clear();
    if ( credential.empty() || credential.size() > MAX_CREDENTIAL_BYTES )
        return false;

    std::array<uint8, ARGON2ID_SALT_BYTES> salt{};
    if ( !FillSecureRandom( salt.data(), salt.size() ) )
        return false;

    const std::size_t encodedLength = argon2_encodedlen(
        ARGON2ID_TIME_COST,
        static_cast<std::uint32_t>( ARGON2ID_MEMORY_KIB ),
        ARGON2ID_PARALLELISM,
        static_cast<std::uint32_t>( salt.size() ),
        static_cast<std::uint32_t>( ARGON2ID_HASH_BYTES ),
        Argon2_id );
    if ( encodedLength == 0 || encodedLength > MAX_VERIFIER_BYTES )
        return false;

    std::vector<char> encoded( encodedLength, '\0' );
    const int result = argon2id_hash_encoded(
        ARGON2ID_TIME_COST,
        static_cast<std::uint32_t>( ARGON2ID_MEMORY_KIB ),
        ARGON2ID_PARALLELISM,
        credential.data(),
        credential.size(),
        salt.data(),
        salt.size(),
        ARGON2ID_HASH_BYTES,
        encoded.data(),
        encoded.size() );
    if ( result != ARGON2_OK )
        return false;

    verifier.assign( encoded.data() );
    return HasSupportedVerifierFormat( verifier );
}

bool PasswordModule::VerifyArgon2idVerifier(
    const std::string& credential,
    const std::string& verifier )
{
    if ( credential.empty() || credential.size() > MAX_CREDENTIAL_BYTES )
        return false;
    if ( !HasSupportedVerifierFormat( verifier ) )
        return false;

    return argon2id_verify(
        verifier.c_str(),
        credential.data(),
        credential.size() ) == ARGON2_OK;
}

bool PasswordModule::VerifyCredential(
    const std::string& credential,
    const std::string& legacyHash,
    const std::string& verifier )
{
    if ( credential.empty() )
        return false;

    if ( !verifier.empty() )
        return VerifyArgon2idVerifier( credential, verifier );

    return ConstantTimeEqual( credential, legacyHash );
}

bool PasswordModule::GenerateSecureRandomHex(
    std::size_t byteCount,
    std::string& value )
{
    value.clear();
    if ( byteCount == 0 || byteCount > MAX_RANDOM_HEX_BYTES )
        return false;

    std::vector<uint8> randomBytes( byteCount );
    if ( !FillSecureRandom( randomBytes.data(), randomBytes.size() ) )
        return false;

    static constexpr char HEX[] = "0123456789ABCDEF";
    value.reserve( byteCount * 2 );
    for ( const uint8 byte : randomBytes ) {
        value.push_back( HEX[byte >> 4] );
        value.push_back( HEX[byte & 0x0F] );
    }

    return true;
}
