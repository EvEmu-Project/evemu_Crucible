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

#ifndef __APICHARACTERDB_H_INCL__
#define __APICHARACTERDB_H_INCL__



class APICharacterDB
{
public:
    APICharacterDB();

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
    bool GetCharacterSkillsTrained(uint32 characterID, std::vector<std::string> & skillTypeIDList, std::vector<std::string> & skillPointsList,
        std::vector<std::string> & skillLevelList, std::vector<std::string> & skillPublishedList);

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
    bool GetCharacterInfo(uint32 characterID, std::vector<std::string> & charInfoList);

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
    bool GetCharacterAttributes(uint32 characterID, std::map<std::string, std::string> & attribList);

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
    bool GetCharacterSkillQueue(uint32 characterID, std::vector<std::string> & orderList, std::vector<std::string> & typeIdList,
        std::vector<std::string> & levelList, std::vector<std::string> & rankList, std::vector<std::string> & skillIdList,
        std::vector<std::string> & primaryAttrList, std::vector<std::string> & secondaryAttrList, std::vector<std::string> & skillPointsTrainedList);

    /**
     * @brief NOT DEFINED YET
     *
     * ?
     *
     * @param[in] ?
     * @param[in] ?
     *
     * @retval ?
     */
    bool GetCharacterImplants(uint32 characterID, std::map<std::string, std::string> & implantList);

    /**
     * @brief NOT DEFINED YET
     *
     * ?
     *
     * @param[in] ?
     * @param[in] ?
     *
     * @retval ?
     */
    bool GetCharacterCertificates(uint32 characterID, std::vector<std::string> & certList);

    /**
     * @brief NOT DEFINED YET
     *
     * ?
     *
     * @param[in] ?
     * @param[in] ?
     *
     * @retval ?
     */
    bool GetCharacterCorporationRoles(uint32 characterID, std::string roleType, std::map<std::string, std::string> & roleList);

protected:

};

#endif    //__APICHARACTERDB_H_INCL__
