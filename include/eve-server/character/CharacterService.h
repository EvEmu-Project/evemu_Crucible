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
	Author:		Zhur
*/

#ifndef __CHARACTERSERVICE_H_INCL__
#define __CHARACTERSERVICE_H_INCL__

#include "character/CharacterDB.h"
#include "PyService.h"

class CharacterService : public PyService {
public:
	CharacterService(PyServiceMgr *mgr);
	virtual ~CharacterService();
	
	void GetCharacterData(uint32 characterID, std::map<std::string, uint32> &characterDataMap);
protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	CharacterDB m_db;
	
	PyCallable_DECL_CALL(GetCharacterToSelect)
	PyCallable_DECL_CALL(GetCharactersToSelect)
	PyCallable_DECL_CALL(SelectCharacterID)
	PyCallable_DECL_CALL(GetOwnerNoteLabels)
	PyCallable_DECL_CALL(GetCharCreationInfo)
	PyCallable_DECL_CALL(GetCharNewExtraCreationInfo)
	PyCallable_DECL_CALL(GetAppearanceInfo)
	PyCallable_DECL_CALL(ValidateName)
	PyCallable_DECL_CALL(ValidateNameEx)
	PyCallable_DECL_CALL(CreateCharacter2)
	PyCallable_DECL_CALL(Ping)
	PyCallable_DECL_CALL(PrepareCharacterForDelete)
	PyCallable_DECL_CALL(CancelCharacterDeletePrepare)
	PyCallable_DECL_CALL(AddOwnerNote)
	PyCallable_DECL_CALL(EditOwnerNote)
	PyCallable_DECL_CALL(GetOwnerNote)
	PyCallable_DECL_CALL(GetHomeStation)
	PyCallable_DECL_CALL(GetCloneTypeID)
	PyCallable_DECL_CALL(GetRecentShipKillsAndLosses)

	PyCallable_DECL_CALL(GetCharacterDescription)
	PyCallable_DECL_CALL(SetCharacterDescription)

	PyCallable_DECL_CALL(GetNote)
	PyCallable_DECL_CALL(SetNote)

	/**
	 * \brief client message to tell the server to log the char creation (I think).
	 *
	 * no long description.
	 *
	 * @param[in] call containing the clients argument info regarding the call.
	 * @return PyResult containing the server's response.
	 */
	PyCallable_DECL_CALL(LogStartOfCharacterCreation)
};

#endif // __CHARACTERSERVICE_H_INCL__
