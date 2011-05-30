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



#include "EVEServerPCH.h"

PyBoundObject::PyBoundObject(PyServiceMgr *mgr)
: m_manager(mgr),
  m_nodeID(0),
  m_bindID(0)
{
    m_strBoundObjectName = "PyBoundObject";
}

PyBoundObject::~PyBoundObject() {
}

PyResult PyBoundObject::Call(const std::string &method, PyCallArgs &args) {
    sLog.Debug("Bound Object","NodeID: %u BindID: %u calling %s in service manager '%s'", nodeID(), bindID(), method.c_str(), GetBoundObjectClassStr().c_str()); 
	args.Dump(SERVICE__CALL_TRACE);

	return(PyCallable::Call(method, args));
}

std::string PyBoundObject::GetBindStr() const {
	//generate a nice bind string:
	char bind_str[128];
	snprintf(bind_str, sizeof(bind_str), "N=%u:%u", nodeID(), bindID());

	return(std::string(bind_str));
}

















