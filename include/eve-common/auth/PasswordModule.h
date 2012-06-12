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

#ifndef _PASSWORD_MODULE_H
#define _PASSWORD_MODULE_H

/**
 * \class PasswordModule
 *
 * @brief simple class to handle password hash generation
 *
 * this class mainly acts like a shell for the password hash stuff.
 *
 * @author Captnoord
 * @date January 2009
 * @note these function's aren't MT compatible ATM.
 */

/* small hack, should be easy to fix */
#define SHA_DIGEST_SIZE 20
extern uint8 mDigest[SHA_DIGEST_SIZE];

class PasswordModule
{
public:
    /**
     *
     */
    #ifndef WIN32
    static uint8 * wchar_tToUtf16(uint8 * chars, int charLen);
    #endif

    /**
     * @brief this functions converts a username and password into a Sha1 hash.
     *
     *
     *
     * @param[in] userName Unicode username string representation.
     * @param[in] passWord Unicode password string representation.
     * @param[out] passWordHash return parameter, returning the Sha1 hash of the userName and passWord
     * @note this is a relative slow function, so I recommend not to use it to calculate the hashes on the fly.
     */
    static bool GeneratePassHash(/*in*/std::wstring &userName, /*in*/std::wstring &passWord, /*out*/std::string &passWordHash);

    /**
     * @brief this functions converts a username and password into a Sha1 hash.
     *
     *
     *
     * @param[in] userName Unicode username wchar_t string representation.
     * @param[in] passWord Unicode password wchar_t string representation.
     * @param[out] passWordHash return parameter, returning the Sha1 hash of the userName and passWord
     * @note this is a relative slow function, so I recommend not to use it to calculate the hashes on the fly.
     */
    static bool GeneratePassHash(/*in*/const wchar_t *userName, /*in*/const wchar_t *passWord, /*out*/std::string &passWordHash);

    /**
     * @brief GenerateHexString generates a hex string from a normal string.
     *
     * GenerateHexString generates a hex string from a normal string, which
     * makes reading non strings easier to read.
     *
     * @param[in] str is the object that needs to be "hexified".
     * @return the hexadecimal representation of str.
     */
    static std::string GenerateHexString(const std::string & str);
};

#endif//_PASSWORD_MODULE_H
