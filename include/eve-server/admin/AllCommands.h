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
#ifndef __ALLCOMMANDS_H_INCL__
#define __ALLCOMMANDS_H_INCL__

//these are not absolutely essential to be included, we could 'class' them all,
//doing this to make the implementation files need less includes
#include "admin/CommandDB.h"
#include "mining/Asteroid.h"
#include "PyServiceMgr.h"
#include "PyCallable.h"
#include "Client.h"
#include "EntityList.h"

class CommandDispatcher;

/* this file includes all of the macro headers for each class of commands and
 * actually prototypes them
 *
 * if you add a new command header/file, you need to add it to the
 * AllCommandsList.h file.
 */

#define COMMAND(name, role, description) \
	PyResult Command_##name(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args);
#include "admin/AllCommandsList.h"

extern void RegisterAllCommands(CommandDispatcher &into);

#endif




