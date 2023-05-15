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
    Author:        caytchen, Zhur
*/

#ifndef __CHARUNBOUNDMGRSERVICE__H__INCL__
#define __CHARUNBOUNDMGRSERVICE__H__INCL__

#include "services/Service.h"
#include "cache/ObjCacheService.h"

/**
 * \class CharUnboundMgrService
 *
 * @brief Handles everything related to creating, deleting and selecting a character
 *
 * This has officially replaced the old CharacterService; as of Incursion, there is no longer any reference to a character remote service.
 *
 * @author caytchen
 * @date April 2011
 */
class CharUnboundMgrService : public Service <CharUnboundMgrService> {
public:
    CharUnboundMgrService(EVEServiceManager& mgr);

private:
    PyResult SelectCharacterID(PyCallArgs& call, PyInt* characterID, std::optional <PyInt*> loadDungeon, std::optional <PyInt*> secondChoiceID);
    PyResult SelectCharacterID(PyCallArgs& call, PyInt* characterID, std::optional <PyBool*> loadDungeon, std::optional <PyInt*> secondChoiceID);

    /**
     * \brief Get details on a character id
     *
     * Get details on a character ud
     *
     * @param[in] call character id
     * @return PyResult character details
     */
    PyResult GetCharacterToSelect(PyCallArgs& call, PyInt* characterID);

    /**
     * \brief Get a list of characters on this account
     *
     * This is the real deal and cached in the central 'cc' service in the game.
     *
     * @param[in] call empty
     * @return PyResult list of characters
     */
    PyResult GetCharactersToSelect(PyCallArgs& call);

    /**
     * \brief Get a lightweight list of characters on this account
     *
     * Only used when creating a petition and not ingame. This is basically a lightweight GetCharactersToSelect in that it only sends characterID and characterName.
     *
     * @param[in] call empty
     * @return PyResult list of characters with characterID and characterName
     */
    PyResult GetCharacterInfo(PyCallArgs& call);

    /**
     * \brief Client check if this account is currently receiving a character from an character transfer
     *
     * Since we have no infrastructure for character transfer, this is not implemented and will always return false
     *
     * @param[in] call empty
     * @return PyResult true if there is a character transfer queued for this account, false if not
     */
    PyResult IsUserReceivingCharacter(PyCallArgs& call);

    PyResult DeleteCharacter(PyCallArgs& call, PyInt* characterID);
    PyResult PrepareCharacterForDelete(PyCallArgs& call, PyInt* characterID);
    PyResult CancelCharacterDeletePrepare(PyCallArgs& call, PyInt* characterID);

    /**
     * \brief Client check to see if a name may be used for a new character
     *
     * Validates a name by checking if it confirms to naming standards and if it doesn't exist. No idea as to how this differs from ValidateName.
     *
     * @param[in] call name to check
     * @return PyResult true if the name may be used
     */
    PyResult ValidateNameEx(PyCallArgs& call, PyRep* name);

    PyResult GetCharCreationInfo(PyCallArgs& call);
    PyResult GetCharNewExtraCreationInfo(PyCallArgs& call);
    PyResult CreateCharacterWithDoll(PyCallArgs& call, PyRep* characterName, PyInt* bloodlineID, PyInt* genderID, PyInt* ancestryID, PyObject* characterInfo, PyObject* portraitInfo, PyInt* schoolID);

private:
    ObjCacheService* m_cache;
};

#endif // __CHARUNBOUNDMGRSERVICE__H__INCL__