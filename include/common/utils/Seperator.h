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
	Author:		Zhur, Bloody.Rabbit
*/

#ifndef __SEPERATOR_H__INCL__
#define __SEPERATOR_H__INCL__

#include "utils/utils_string.h"

/**
 * @brief Separates string to arguments.
 *
 * @author Zhur, Bloody.Rabbit
 */
class Seperator
{
public:
	Seperator( const char* str,
               const char* divs = " \t",
               const char* quotes = "\"\'" );

    const std::string& arg( size_t index ) const { return mArgs[ index ]; }
    size_t argCount() const { return mArgs.size(); }

    bool isNumber( size_t index ) const { return ::IsNumber( arg( index ) ); }
    bool isHexNumber( size_t index ) const { return ::IsHexNumber( arg( index ) ); }

protected:
    std::vector<std::string> mArgs;
};

#endif /* !__SEPERATOR_H__INCL__ */
