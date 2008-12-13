/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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

#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "EVECollectDisp.h"

extern int EVE_NIDS_main(EVECollectDispatcher *disp, int argc, char *argv[]);

int main(int argc, char *argv[]) {

	if(!load_log_settings("log.ini"))
		_log(COLLECT__MESSAGE, "Warning: Unable to read %s", "log.ini");
	else
		_log(COLLECT__MESSAGE, "Log settings loaded from %s", "log.ini");

	if(!PyRepString::LoadStringFile("strings.txt"))
		_log(COLLECT__ERROR, "Unable to open ./strings.txt, I need it to decode string table elements!");

	EVECollectDispatcher dispatch;
	dispatch.lookResolver.LoadIntFile("intres.txt");
	dispatch.lookResolver.LoadStringFile("strres.txt");
	
	return(EVE_NIDS_main(&dispatch, argc, argv));
}




