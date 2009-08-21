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

CommandDispatcher::CommandDispatcher(PyServiceMgr &services, DBcore &db) : m_services(services), m_db(&db) {}

CommandDispatcher::~CommandDispatcher() {
	std::map<std::string, CommandRecord *>::iterator cur, end;
	cur = m_commands.begin();
	end = m_commands.end();
	for(; cur != end; cur++) {
		delete cur->second;
	}
}

PyResult CommandDispatcher::Execute(Client *from, const char *msg) {
	//might want to check for # or / at the beginning of this crap.
	Seperator sep(msg+1);

	if(sep.argnum == 0) {
		//empty command, return list of commands
		PyString *reason = new PyString("Commands: ");

		std::map<std::string, CommandRecord *>::const_iterator cur, end;
		cur = m_commands.begin();
		end = m_commands.end();
		reason->value += "[";
		for(; cur != end; cur++)
			reason->value += "'" + cur->second->command + "',";
		reason->value += "]";

		std::map<std::string, PyRep *> args;
		args["reason"] = reason;
		throw(PyException(MakeUserError("", args)));
	}
	
	std::map<std::string, CommandRecord *>::const_iterator res;
	res = m_commands.find(sep.arg[0]);
	if(res == m_commands.end()) {
        sLog.Error("CMD Dispatcher", "Unable to find command '%s' for %s", sep.arg[0], from->GetName());
		throw(PyException(MakeCustomError("Unknown command '%s'", sep.arg[0])));
	}
	
	CommandRecord *rec = res->second;

	if((from->GetAccountRole() & rec->required_role) != rec->required_role) {
        sLog.Error("CMD Dispatcher", "Access denied to %s for command '%s', had role 0x%x, need role 0x%x", from->GetName(), rec->command.c_str(), from->GetAccountRole(), rec->required_role);
		throw(PyException(MakeCustomError("Access denied to command '%s'", sep.arg[0])));
	}

	return rec->function(from, &m_db, &m_services, sep);
}

void CommandDispatcher::AddCommand(const char *cmd, const char *desc, uint32 required_role, CommandFunc function) {
	std::map<std::string, CommandRecord *>::iterator res;
	res = m_commands.find(cmd);
	
	if(res != m_commands.end())
		delete res->second;	//watch for command overwrite
	
	CommandRecord *rec = new CommandRecord(cmd, desc, required_role, function);
	m_commands[cmd] = rec;
}
