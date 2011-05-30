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

#ifndef __SYSTEMDB_H_INCL__
#define __SYSTEMDB_H_INCL__

#include "ServiceDB.h"

class DBSystemEntity {
public:
	uint32 itemID;
	uint32 typeID;
	uint32 groupID;
	uint32 orbitID;
	GPoint position;
	double radius;
	double security;
	std::string itemName;
};

class DBSystemDynamicEntity {
public:
	uint32 itemID;
    std::string itemName;
	uint32 typeID;
	uint32 groupID;
    uint32 ownerID;
    uint32 locationID;
    uint32 flag;
	uint32 categoryID;
    uint32 corporationID;
    uint32 allianceID;
    double x;
    double y;
    double z;
};

class SystemDB
: public ServiceDB
{
public:
	bool LoadSystemEntities(uint32 systemID, std::vector<DBSystemEntity> &into);
	bool LoadSystemDynamicEntities(uint32 systemID, std::vector<DBSystemDynamicEntity> &into);
	static uint32 GetObjectLocationID( uint32 itemID );

	PyObject *ListFactions();
	PyObject *ListJumps(uint32 stargateID);
	
protected:
};





#endif


