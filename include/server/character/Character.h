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
	Author:		Zhur, Bloody.Rabbit
*/

#ifndef __CHARACTER__H__INCL__
#define __CHARACTER__H__INCL__

class CharacterData {
public:
	uint32 charid;
	
	std::string name;
	std::string title;
	std::string description;
	bool gender;

	double bounty;
	double balance;
	double securityRating;
	uint32 logonMinutes;

	uint32 corporationID;
	uint64 corporationDateTime;
	uint32 allianceID;
	
	uint32 stationID;
	uint32 solarSystemID;
	uint32 constellationID;
	uint32 regionID;

	uint32 typeID;
	uint32 bloodlineID;
	EVERace raceID;	//must correspond to our bloodlineID!
	uint32 ancestryID;

	uint32 careerID;
	uint32 schoolID;
	uint32 careerSpecialityID;
	
	uint8 intelligence;
	uint8 charisma;
	uint8 perception;
	uint8 memory;
	uint8 willpower;

	uint64 startDateTime;
	uint64 createDateTime;
};

class CharacterAppearance {
public:
	CharacterAppearance();
	CharacterAppearance(const CharacterAppearance &from);
	~CharacterAppearance();

#define INT(v) \
	uint32 v;
#define INT_DYN(v) \
	bool IsNull_##v() const { \
		return(v == NULL); \
	} \
	uint32 Get_##v() const { \
		return(IsNull_##v() ? NULL : *v); \
	} \
	void Set_##v(uint32 val) { \
		Clear_##v(); \
		v = new uint32(val); \
	} \
	void Clear_##v() { \
		if(!IsNull_##v()) \
			delete v; \
		v = NULL; \
	}
#define REAL(v) \
	double v;
#define REAL_DYN(v) \
	bool IsNull_##v() const { \
		return(v == NULL); \
	} \
	double Get_##v() const { \
		return(IsNull_##v() ? NULL : *v); \
	} \
	void Set_##v(double val) { \
		Clear_##v(); \
		v = new double(val); \
	} \
	void Clear_##v() { \
		if(!IsNull_##v()) \
			delete v; \
		v = NULL; \
	}
#include "character/CharacterAppearance_fields.h"

	void Build(const std::map<std::string, PyRep *> &from);
	void operator=(const CharacterAppearance &from);

protected:
#define INT_DYN(v) \
	uint32 *v;
#define REAL_DYN(v) \
	double *v;
#include "character/CharacterAppearance_fields.h"
};

class CorpMemberInfo {
public:
	CorpMemberInfo() : corpHQ(0), corprole(0), rolesAtAll(0), rolesAtBase(0), rolesAtHQ(0), rolesAtOther(0) {}

	uint32 corpHQ;	//this really doesn't belong here...
	uint64 corprole;
	uint64 rolesAtAll;
	uint64 rolesAtBase;
	uint64 rolesAtHQ;
	uint64 rolesAtOther;
};

#endif /* !__CHARACTER__H__INCL__ */

