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

PyCallable_Make_InnerDispatcher(PaperDollService)

PaperDollService::PaperDollService(PyServiceMgr* mgr)
: PyService(mgr, "paperDollServer"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(PaperDollService, GetPaperDollData)
	PyCallable_REG_CALL(PaperDollService, ConvertAndSavePaperDoll)
	PyCallable_REG_CALL(PaperDollService, UpdateExistingCharacterFull)
	PyCallable_REG_CALL(PaperDollService, UpdateExistingCharacterLimited)
	PyCallable_REG_CALL(PaperDollService, GetPaperDollPortraitDataFor)
	PyCallable_REG_CALL(PaperDollService, GetMyPaperDollData)
}

PaperDollService::~PaperDollService() {
	delete m_dispatch;
}

PyResult PaperDollService::Handle_GetPaperDollData(PyCallArgs &call) {
	return NULL;
}

PyResult PaperDollService::Handle_ConvertAndSavePaperDoll(PyCallArgs &call) {
	return NULL;
}

PyResult PaperDollService::Handle_UpdateExistingCharacterFull(PyCallArgs &call) {
	return NULL;
}

PyResult PaperDollService::Handle_UpdateExistingCharacterLimited(PyCallArgs &call) {
	return NULL;
}

PyResult PaperDollService::Handle_GetPaperDollPortraitDataFor(PyCallArgs &call) {
	return NULL;
}

PyResult PaperDollService::Handle_GetMyPaperDollData(PyCallArgs &call) 
{
	PyDict* args = new PyDict;

	args->SetItemString( "colors", new PyDict );
	args->SetItemString( "modifiers", new PyDict );
	args->SetItemString( "sculpts", new PyDict );

	return new PyObject("util.KeyVal", args);
}