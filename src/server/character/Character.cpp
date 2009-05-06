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

#include "EvemuPCH.h"

CharacterAppearance::CharacterAppearance() {
	//NULL all dynamic fields
#define NULL_FIELD(v) \
	v = NULL;

#define INT_DYN(v) NULL_FIELD(v)
#define REAL_DYN(v) NULL_FIELD(v)
#include "character/CharacterAppearance_fields.h"

#undef NULL_FIELD
}

CharacterAppearance::CharacterAppearance(const CharacterAppearance &from) {
	//just do deep copy
	*this = from;
}

CharacterAppearance::~CharacterAppearance() {
	//delete all dynamic fields
#define CLEAR_FIELD(v) \
	Clear_##v();

#define INT_DYN(v) CLEAR_FIELD(v)
#define REAL_DYN(v) CLEAR_FIELD(v)
#include "character/CharacterAppearance_fields.h"

#undef CLEAR_FIELD
}

void CharacterAppearance::Build(const std::map<std::string, PyRep *> &from) {
	//builds our contents from strdict
	std::map<std::string, PyRep *>::const_iterator itr;

	_log(CLIENT__MESSAGE, "  Appearance Data:");

#define INT(v) \
	itr = from.find(#v); \
	if(itr != from.end()) { \
		if(!itr->second->IsInteger()) \
			_log(CLIENT__ERROR, "Invalid type for " #v ": expected integer, got %s.", itr->second->TypeString()); \
		else { \
			v = ((PyRepInteger *)itr->second)->value; \
			_log(CLIENT__MESSAGE, "     %s: %u", itr->first.c_str(), v); \
		} \
	}
#define INT_DYN(v) \
	itr = from.find(#v); \
	if(itr != from.end()) { \
		if(!itr->second->IsInteger()) \
			_log(CLIENT__ERROR, "Invalid type for " #v ": expected integer, got %s.", itr->second->TypeString()); \
		else { \
			Set_##v(((PyRepInteger *)itr->second)->value); \
			_log(CLIENT__MESSAGE, "     %s: %u", itr->first.c_str(), Get_##v()); \
		} \
	}
#define REAL(v) \
	itr = from.find(#v); \
	if(itr != from.end()) { \
		if(!itr->second->IsReal()) \
			_log(CLIENT__ERROR, "Invalid type for " #v ": expected real, got %s.", itr->second->TypeString()); \
		else { \
			v = ((PyRepReal *)itr->second)->value; \
			_log(CLIENT__MESSAGE, "     %s: %f", itr->first.c_str(), v); \
		} \
	}
#define REAL_DYN(v) \
	itr = from.find(#v); \
	if(itr != from.end()) { \
		if(!itr->second->IsReal()) \
			_log(CLIENT__ERROR, "Invalid type for " #v ": expected real, got %s.", itr->second->TypeString()); \
		else { \
			Set_##v(((PyRepReal *)itr->second)->value); \
			_log(CLIENT__MESSAGE, "     %s: %f", itr->first.c_str(), Get_##v()); \
		} \
	}
#include "character/CharacterAppearance_fields.h"
}

void CharacterAppearance::operator=(const CharacterAppearance &from) {
#define COPY(v) \
	v = from.v;
#define COPY_DYN(v) \
	if(!from.IsNull_##v()) \
		Set_##v(from.Get_##v());

#define INT(v) COPY(v)
#define INT_DYN(v) COPY_DYN(v)
#define REAL(v) COPY(v)
#define REAL_DYN(v) COPY_DYN(v)
#include "character/CharacterAppearance_fields.h"

#undef COPY
#undef COPY_DYN
}


