#ifndef __COMMANDDISPATCHER_H_INCL__
#define __COMMANDDISPATCHER_H_INCL__

#include "admin/CommandDB.h"

class Client;
class Seperator;
class PyResult;
class EVEServiceManager;

class CommandDispatcher {
public:
    //this is the prototype for a command function:
    typedef PyResult (*CommandFunc)(Client *who, CommandDB *db, EVEServiceManager& services, const Seperator &args);

    class CommandRecord {
    public:
        CommandRecord( const char * cmd, const char * desc, int64 req_role,
                       CommandFunc& callback, int64 alternate_role = 0 )
        : command(cmd), description(desc), required_role(req_role),
          alternative_role(alternate_role), function(callback) {}

        std::string command;
        std::string description;
        int64 required_role;
        int64 alternative_role;  // optional independently sufficient role
        CommandFunc function;
    };

    CommandDispatcher(EVEServiceManager &services);
    virtual ~CommandDispatcher();

    void Close();

    PyResult Execute(Client *from, const char *msg);

    void AddCommand(const char *cmd, const char *desc, int64 required_role,
                    CommandFunc function, int64 alternative_role = 0);
    void ListCommands();

    const std::map<std::string, CommandRecord*>& GetCommandList() const { return m_commands; } // ---commandlist update; gives external read-only access to the command list.

protected:
    EVEServiceManager &m_services;
    CommandDB m_db;

    std::map<std::string, CommandRecord *> m_commands;    //we own these pointers
};

extern CommandDispatcher* g_dispatcher; // ---commandlist update

#endif
