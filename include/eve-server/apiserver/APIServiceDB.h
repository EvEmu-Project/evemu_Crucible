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
    Author:        Aknor Jaden
*/

#ifndef __APISERVICEDB_H_INCL__
#define __APISERVICEDB_H_INCL__

class APIServiceDB
{
public:
    APIServiceDB();

    /**
     * @brief ?
     *
     * ?
     *
     * @param[in] ?
     * @param[in] ?
     *
     * @retval ?
     */
    bool GetAccountIdFromUsername(std::string username, std::string * accountID);

    /**
     * @brief ?
     *
     * ?
     *
     * @param[in] ?
     * @param[in] ?
     *
     * @retval ?
     */
    bool GetAccountIdFromUserID(std::string userID, uint32 * accountID);

    /**
     * @brief ?
     *
     * ?
     *
     * @param[in] ?
     * @param[in] ?
     *
     * @retval ?
     */
    bool GetApiAccountInfoUsingAccountID(std::string accountID, uint32 * userID, std::string * apiFullKey,
        std::string * apiLimitedKey, uint32 * apiRole);

    /**
     * @brief ?
     *
     * ?
     *
     * @param[in] ?
     * @param[in] ?
     *
     * @retval ?
     */
    bool GetApiAccountInfoUsingUserID(std::string userID, std::string * apiFullKey, std::string * apiLimitedKey, uint32 * apiRole);

    /**
     * @brief ?
     *
     * ?
     *
     * @param[in] ?
     * @param[in] ?
     *
     * @retval ?
     */
    bool UpdateUserIdApiKeyDatabaseRow(uint32 userID, std::string apiFullKey, std::string apiLimitedKey);

    /**
     * @brief ?
     *
     * ?
     *
     * @param[in] ?
     * @param[in] ?
     *
     * @retval ?
     */
    bool InsertNewUserIdApiKeyInfoToDatabase(uint32 accountID, std::string apiFullKey, std::string apiLimitedKey, uint32 apiRole);

    /**
     * @brief ?
     *
     * ?
     *
     * @param[in] ?
     * @param[in] ?
     *
     * @retval ?
     */
    bool UpdateUserIdApiRole(uint32 userID, uint32 apiRole);
};

#endif    //__APISERVICEDB_H_INCL__
