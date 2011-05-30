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

#ifndef __SCENARIO_SERVICE_H_INCL__
#define __SCENARIO_SERVICE_H_INCL__

#include "system/SystemDB.h"
#include "PyService.h"

class ScenarioService
: public PyService
{
public:
	ScenarioService(PyServiceMgr *mgr);
	virtual ~ScenarioService();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	SystemDB m_db;

	PyCallable_DECL_CALL(ResetD)
	//PlayDungeon(dungeonVID, selectedRoom)
	//EditRoom(dungeonVID, selectedRoom)
	//GotoRoom(selectedRoom)
	//GetDunObjects() (returns list of slim items)
	//GetSelObjects() (returns list of slim items)
	//IsSelectedByObjID(dunObjectID)
	//DuplicateSelection(amount, X, Y, Z)
	//SetSelectionByID(ids)
	//SetSelectedRadius(minRadius, maxRadius)
	//SetRotate(y, p, r)
	//RotateSelected(yaw, pitch, roll)
	//JitterSelection(X, Y, Z)
	//ArrangeSelection(X, Y, Z)
	//DeleteSelected()
	//RefreshSelection()

	//overloaded in order to support bound objects:
	//virtual PyBoundObject *_CreateBoundObject(Client *c, const PyRep *bind_args);
};





#endif


