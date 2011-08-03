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

PyCallable_Make_InnerDispatcher(PhotoUploadService)

PhotoUploadService::PhotoUploadService(PyServiceMgr* mgr)
	: PyService(mgr, "photoUploadSvc"), m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(PhotoUploadService, Upload)
}

PhotoUploadService::~PhotoUploadService()
{
	delete m_dispatch;
}

PyResult PhotoUploadService::Handle_Upload(PyCallArgs &call)
{
	Call_SingleStringArg arg;
	if (!arg.Decode(&call.tuple))
	{
		codelog(CLIENT__ERROR, "Failed to decode args for Upload call");
		return NULL;
	}

	sLog.Log("photo upload", "Received image from account %i, size: %i", call.client->GetAccountID(), arg.arg.size());

	std::tr1::shared_ptr<std::vector<char> > data(new std::vector<char>(arg.arg.begin(), arg.arg.end()));
	ImageServer::get().ReportNewImage(call.client->GetAccountID(), data);

	return new PyBool(true);
}
