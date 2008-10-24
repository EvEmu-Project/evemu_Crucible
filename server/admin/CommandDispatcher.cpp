/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "EvemuPCH.h"

CommandDispatcher::CommandDispatcher(CommandDB *db, PyServiceMgr *services)
: m_db(db),
  m_services(services)
{
}

CommandDispatcher::~CommandDispatcher() {
	std::map<std::string, CommandRecord *>::iterator cur, end;
	cur = m_commands.begin();
	end = m_commands.end();
	for(; cur != end; cur++) {
		delete cur->second;
	}

	delete m_db;
}

PyResult CommandDispatcher::Execute(Client *from, const char *msg) const {
	//might want to check for # or / at the begining of this crap.
	Seperator sep(msg+1);

	if(sep.argnum == 0) {
		//empty command, return list of commands
		PyRepString *reason = new PyRepString("Commands: ");

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
		_log(COMMAND__ERROR, "Unable to find command '%s' for %s", sep.arg[0], from->GetName());
		throw(PyException(MakeCustomError("Unknown command '%s'", sep.arg[0])));
	}
	
	CommandRecord *rec = res->second;

	if((from->GetRole() & rec->required_role) != rec->required_role) {
		_log(COMMAND__ERROR, "Access denied to %s for command '%s', had role 0x%x, need role 0x%x", from->GetName(), rec->command.c_str(), from->GetRole(), rec->required_role);
		throw(PyException(MakeCustomError("Access denied to command '%s'", sep.arg[0])));
	}

	return(rec->function(from, m_db, m_services, sep));
}

void CommandDispatcher::AddCommand(const char *cmd, const char *desc, uint32 required_role, CommandFunc function) {
	std::map<std::string, CommandRecord *>::iterator res;
	res = m_commands.find(cmd);
	
	if(res != m_commands.end())
		delete res->second;	//watch for command overwrite
	
	CommandRecord *rec = new CommandRecord;
	rec->command = cmd;
	rec->description = desc;
	rec->required_role = required_role;
	rec->function = function;

	m_commands[cmd] = rec;
}



















