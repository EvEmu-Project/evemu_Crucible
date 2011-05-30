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

#ifndef __PYBOUNDOBJECT_H_INCL__
#define __PYBOUNDOBJECT_H_INCL__

#include "PyCallable.h"

class PyBoundObject
: public PyCallable {
public:
	PyBoundObject(PyServiceMgr *mgr);
	virtual ~PyBoundObject();

	virtual void Release() = 0;

	uint32 nodeID() const { return(m_nodeID); }
	uint32 bindID() const { return(m_bindID); }

	//returns string "N=(nodeID):(bindID)"
	std::string GetBindStr() const;
    std::string GetBoundObjectClassStr() const { return m_strBoundObjectName; };

	//just to say who we are:
	virtual PyResult Call(const std::string &method, PyCallArgs &args);

protected:
	friend class PyServiceMgr;	//for access to _SetNodeBindID only.
	void _SetNodeBindID(uint32 nodeID, uint32 bindID) { m_nodeID = nodeID; m_bindID = bindID; }

	PyServiceMgr *const m_manager;
    std::string m_strBoundObjectName;

private:
	uint32 m_nodeID;
	uint32 m_bindID;
};

#endif
