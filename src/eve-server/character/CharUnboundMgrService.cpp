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
    Author:     caytchen
*/

#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(CharUnboundMgrService)

CharUnboundMgrService::CharUnboundMgrService(PyServiceMgr* mgr)
: PyService(mgr, "charUnboundMgr"),
  m_dispatch(new Dispatcher(this)) 
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(CharUnboundMgrService, SelectCharacterID)
	PyCallable_REG_CALL(CharUnboundMgrService, GetCharacterToSelect)
	PyCallable_REG_CALL(CharUnboundMgrService, GetCharactersToSelect)
	PyCallable_REG_CALL(CharUnboundMgrService, GetCharacterInfo)
	PyCallable_REG_CALL(CharUnboundMgrService, IsUserReceivingCharacter)
	PyCallable_REG_CALL(CharUnboundMgrService, DeleteCharacter)
	PyCallable_REG_CALL(CharUnboundMgrService, PrepareCharacterForDelete)
	PyCallable_REG_CALL(CharUnboundMgrService, CancelCharacterDeletePrepare)
	PyCallable_REG_CALL(CharUnboundMgrService, ValidateNameEx)
	PyCallable_REG_CALL(CharUnboundMgrService, GetCharCreationInfo)
	PyCallable_REG_CALL(CharUnboundMgrService, GetCharNewExtraCreationInfo)
	PyCallable_REG_CALL(CharUnboundMgrService, CreateCharacterWithDoll)
}

CharUnboundMgrService::~CharUnboundMgrService() {
	delete m_dispatch;
}

PyResult CharUnboundMgrService::Handle_IsUserReceivingCharacter(PyCallArgs &call) {
	return new PyBool(false);
}

PyResult CharUnboundMgrService::Handle_ValidateNameEx(PyCallArgs &call)
{
	Call_SingleWStringArg arg;
	if (!arg.Decode(&call.tuple))
	{
		codelog(CLIENT__ERROR, "Failed to decode args for ValidateNameEx call");
		return NULL;
	}

	return new PyBool(m_db.ValidateCharName(arg.arg.c_str()));
}

PyResult CharUnboundMgrService::Handle_SelectCharacterID(PyCallArgs &call) {
	return NULL;
}

PyResult CharUnboundMgrService::Handle_GetCharactersToSelect(PyCallArgs &call) {
	return NULL;
}

PyResult CharUnboundMgrService::Handle_GetCharacterToSelect(PyCallArgs &call) {
	return NULL;
}

PyResult CharUnboundMgrService::Handle_DeleteCharacter(PyCallArgs &call) {
	return NULL;
}

PyResult CharUnboundMgrService::Handle_PrepareCharacterForDelete(PyCallArgs &call) {
	return NULL;
}

PyResult CharUnboundMgrService::Handle_CancelCharacterDeletePrepare(PyCallArgs &call) {
	return NULL;
}

PyResult CharUnboundMgrService::Handle_GetCharacterInfo(PyCallArgs &call) {
	return NULL;
}

PyResult CharUnboundMgrService::Handle_GetCharCreationInfo(PyCallArgs &call) {
	PyDict *result = new PyDict();

	//send all the cache hints needed for char creation.
	m_manager->cache_service->InsertCacheHints(
		ObjCacheService::hCharCreateCachables,
		result);
	_log(CLIENT__MESSAGE, "Sending char creation info reply");

	return result;
}

PyResult CharUnboundMgrService::Handle_GetCharNewExtraCreationInfo(PyCallArgs &call) {
	_log(CLIENT__MESSAGE, "Sending empty char new extra creation info");
	return new PyDict();
}

PyResult CharUnboundMgrService::Handle_CreateCharacterWithDoll(PyCallArgs &call) {
	return NULL;
}