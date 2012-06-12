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

#ifndef __AUTH__PASSWORD_MODULE_H__INCL__
#define __AUTH__PASSWORD_MODULE_H__INCL__

/**
 * \class PasswordModule
 *
 * @brief simple class to handle password hash generation
 *
 * this class mainly acts like a shell for the password hash stuff.
 *
 * @author Captnoord
 * @date January 2009
 */
class PasswordModule
{
public:
    /**
     * @brief Generates a SHA-1 hash from a username and a password.
     *
     * @param[in]  user The username.
     * @param[in]  pass The password.
     * @param[out] hash Where to place the resultant SHA-1 hash.
     *
     * @note This is a relatively slow function, so I recommend
     *       not to use it to calculate hashes on the fly.
     */
    static bool GeneratePassHash(
        const std::string& user,
        const std::string& pass,
        std::string& hash );
    /**
     * @brief Generates a SHA-1 hash from a username and a password.
     *
     * @param[in]  user    The username.
     * @param[in]  userLen Length of the username.
     * @param[in]  pass    The password.
     * @param[in]  passLen Length of the password.
     * @param[out] hash    Where to place the resultant SHA-1 hash.
     *
     * @note This is a relatively slow function, so I recommend
     *       not to use it to calculate hashes on the fly.
     */
    static bool GeneratePassHash(
        const char* user, size_t userLen,
        const char* pass, size_t passLen,
        std::string& hash );

    /**
     * @brief Generates a SHA-1 hash from a username and a password.
     *
     * The input username and password must be encoded in UTF-16 and
     * the username must be lowercased and trimmed to give expected
     * results.
     *
     * @param[in]  user The username.
     * @param[in]  pass The password.
     * @param[out] hash Where to place the resultant SHA-1 hash.
     *
     * @note This is a relatively slow function, so I recommend
     *       not to use it to calculate hashes on the fly.
     */
    static bool GeneratePassHash(
        const std::vector< uint16 >& user,
        const std::vector< uint16 >& pass,
        std::string& hash );
    /**
     * @brief Generates a SHA-1 hash from a username and a password.
     *
     * The input username and password must be encoded in UTF-16 and
     * the username must be lowercased and trimmed to give expected
     * results.
     *
     * @param[in]  user    The username.
     * @param[in]  userLen Length of the username (in UTF-16 characters).
     * @param[in]  pass    The password.
     * @param[in]  passLen Length of the password (in UTF-16 characters).
     * @param[out] hash    Where to place the resultant SHA-1 hash.
     *
     * @note This is a relatively slow function, so I recommend
     *       not to use it to calculate hashes on the fly.
     */
    static bool GeneratePassHash(
        const uint16* user, size_t userLen,
        const uint16* pass, size_t passLen,
        std::string& hash );
};

#endif /* !__AUTH__PASSWORD_MODULE_H__INCL__ */
