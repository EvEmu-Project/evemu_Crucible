#ifndef __COMMANDDISPATCHER_H_INCL__
#define __COMMANDDISPATCHER_H_INCL__

#include "../common/types.h"
#include <string>
#include <map>

class Client;
class Seperator;
class CommandDB;
class PyResult;
class PyServiceMgr;

class CommandDispatcher {
public:
	//this is the prototype for a command function:
	typedef PyResult (*CommandFunc)(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args);
	
	class CommandRecord {
	public:
		std::string command;
		std::string description;
		uint32 required_role;
		CommandFunc function;
	};

	CommandDispatcher(CommandDB *db, PyServiceMgr *services);
	virtual ~CommandDispatcher();

	PyResult Execute(Client *from, const char *msg) const;

	void AddCommand(const char *cmd, const char *desc, uint32 required_role, CommandFunc function);
	
protected:
	CommandDB *const m_db;	//we own this
	PyServiceMgr *const m_services;	//we do not own this
	
	std::map<std::string, CommandRecord *> m_commands;	//we own these pointers
};







#endif


