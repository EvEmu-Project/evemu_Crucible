
#include "CommandDispatcher.h"
#include "../common/seperator.h"
#include "CommandDB.h"
#include "../common/logsys.h"
#include "../Client.h"

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

bool CommandDispatcher::Execute(Client *from, const char *msg) const {
	//might want to check for # or / at the begining of this crap.
	Seperator sep(msg+1);
	
	std::map<std::string, CommandRecord *>::const_iterator res;
	res = m_commands.find(sep.arg[0]);
	if(res == m_commands.end()) {
		_log(COMMAND__ERROR, "Unable to find command '%s' for %s", sep.arg[0], from->GetName());
		from->SendErrorMsg("Unknown command '%s'", sep.arg[0]);
		return(true);
	}
	
	CommandRecord *rec = res->second;

	if((from->GetRole() & rec->required_role) != rec->required_role) {
		_log(COMMAND__ERROR, "Access denied to %s for command '%s', had role 0x%x, need role 0x%x", from->GetName(), rec->command.c_str(), from->GetRole(), rec->required_role);
		from->SendErrorMsg("Access denied to command '%s'", sep.arg[0]);
		return(true);
	}

	CommandFunc func = rec->function;
	func(from, m_db, m_services, sep);
	
	return(true);
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



















