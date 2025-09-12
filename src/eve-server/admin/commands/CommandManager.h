#ifndef __COMMANDMANAGER_H_INCL__
#define __COMMANDMANAGER_H_INCL__

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "admin/commands/ICommand.h"

class Client;
class EVEServiceManager;
class PyResult;

namespace Commands {

    // Command registrar interface
    class ICommandRegistrar {
    public:
        virtual ~ICommandRegistrar() = default;
        virtual void RegisterCommands(class CommandManager& manager) = 0;
    };

    // Command manager
    class CommandManager {
    public:
        CommandManager(EVEServiceManager& services);
        ~CommandManager();

        // Register command
        void RegisterCommand(std::unique_ptr<ICommand> command);
        
        // Register command registrar
        void RegisterCommandRegistrar(std::unique_ptr<ICommandRegistrar> registrar);
        
        // Execute command
        PyResult* Execute(Client* client, const std::string& commandLine);
        
        // Get command list
        std::vector<std::string> GetCommandList(Client* client = nullptr) const;
        
        // Get command help
        std::string GetCommandHelp(const std::string& commandName) const;
        
        // Get commands by category
        std::map<std::string, std::vector<std::string>> GetCommandsByCategory(Client* client = nullptr) const;
        
        // Find command
        ICommand* FindCommand(const std::string& name) const;
        
        // Initialize all commands
        void Initialize();
        
        // Cleanup resources
        void Shutdown();

    private:
        EVEServiceManager& m_services;
        std::map<std::string, std::unique_ptr<ICommand>> m_commands;
        std::vector<std::unique_ptr<ICommandRegistrar>> m_registrars;
        
        // Parse command line
        std::vector<std::string> ParseCommandLine(const std::string& commandLine) const;
        
        // Check permission
        bool CheckPermission(Client* client, ICommand* command) const;
        
        // Create error response
        PyResult* CreateErrorResponse(const std::string& message) const;
        
        // Create success response
        PyResult* CreateSuccessResponse(const std::string& message) const;
        
        // Create command list response
        PyResult* CreateCommandListResponse(const std::vector<std::string>& commands) const;
    };

    // Command registration macro
    #define REGISTER_COMMAND(manager, commandClass) \
        manager.RegisterCommand(std::make_unique<commandClass>())

    // Command registrar base class
    template<typename TDerived>
    class CommandRegistrarBase : public ICommandRegistrar {
    public:
        void RegisterCommands(CommandManager& manager) override {
            static_cast<TDerived*>(this)->DoRegisterCommands(manager);
        }
    };

} // namespace Commands

#endif // __COMMANDMANAGER_H_INCL__