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
	Author:		Bloody.Rabbit
*/

#ifndef __COMMANDS_H__INCL__
#define __COMMANDS_H__INCL__

/** Struct describing one eve-tool command. */
struct EVEToolCommand
{
    /** Name of the command; for invocation. */
    const char* name;
    /** Callback; what to call. */
    void ( *callback )( const Seperator& cmd );
    /** Description of command. */
    const char* description;
};

/** Array of all available commands in eve-tool. */
extern const EVEToolCommand EVETOOL_COMMANDS[];
/** Number of available commands. */
extern const size_t EVETOOL_COMMAND_COUNT;

/**
 * @brief Finds a command.
 *
 * @param[in] name Name of command.
 *
 * @return Found command; NULL if not found.
 */
const EVEToolCommand* FindCommand( const char* name );
/**
 * @brief Finds a command.
 *
 * @param[in] name Name of command.
 *
 * @return Found command; NULL if not found.
 */
const EVEToolCommand* FindCommand( const std::string& name );

/**
 * @brief Processed given command.
 *
 * @param[in] cmd Command to be processed.
 */
void ProcessCommand( const Seperator& cmd );

#endif /* !__COMMANDS_H__INCL__ */

