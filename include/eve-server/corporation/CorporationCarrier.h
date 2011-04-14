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


#ifndef __CORPORATIONCARRIER_H_INCL__
#define __CORPORATIONCARRIER_H_INCL__

class OfficeInfo {
public:
	uint32 officeID;
	uint32 officeFolderID;
	uint32 typeID;
	uint32 stationID;
	uint32 corporationID;

	OfficeInfo(uint32 corpID, uint32 statID) {
		corporationID = corpID;
		stationID = statID;						// starts with 60...
		officeFolderID = stationID + 6000000;	// starts with 66...
		typeID = 1932;							// No idea what this should be or if it should be different for each of them...
		officeID = 0;
	}

	OfficeInfo(uint32 corpID, uint32 statID, uint32 offID) {
		corporationID = corpID;
		stationID = statID;						// starts with 60...
		officeFolderID = stationID + 6000000;	// starts with 66...
		typeID = 1932;							// No idea what this should be or if it should be different for each of them...
		officeID = offID;
	}
};



class ApplicationInfo {
public:
	uint32 corpID;
	uint32 charID;
	std::string appText;
	uint64 appTime;
	uint64 role;
	uint64 grantRole;
	uint32 status;
	uint32 deleted;
	uint32 lastCID;
	bool valid;
	
	ApplicationInfo(bool valid_flag=true) : corpID(0), charID(0), appTime(Win32TimeNow()), role(0), grantRole(0), status(0), deleted(0), lastCID(0), valid(valid_flag) { }
};


#endif


