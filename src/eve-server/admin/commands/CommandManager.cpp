#include "eve-server.h"
#include "admin/commands/CommandManager.h"
#include "Client.h"
#include "services/ServiceManager.h"
#include "python/PyRep.h"
#include "utils/Seperator.h"

namespace Commands {

    CommandManager::CommandManager(EVEServiceManager& services)
        : m_services(services) {
        m_commands.clear();
        m_registrars.clear();
    }

    CommandManager::~CommandManager() {
        Shutdown();
    }

    void CommandManager::RegisterCommand(std::unique_ptr<ICommand> command) {
        if (!command) {
            sLog.Error("CommandManager", "Attempted to register null command");
            return;
        }
        
        std::string name = command->GetName();
        if (name.empty()) {
            sLog.Error("CommandManager", "Attempted to register command with empty name");
            return;
        }
        
        // Check if command with same name already exists
        if (m_commands.find(name) != m_commands.end()) {
            sLog.Warning("CommandManager", "Command '%s' already exists, replacing...", name.c_str());
        }
        
        sLog.Debug("CommandManager", "Registering command: %s (Category: %s, Role: %lld)", 
                   name.c_str(), command->GetCategory().c_str(), command->GetRequiredRole());
        
        m_commands[name] = std::move(command);
    }

    void CommandManager::RegisterCommandRegistrar(std::unique_ptr<ICommandRegistrar> registrar) {
        if (!registrar) {
            sLog.Error("CommandManager", "Attempted to register null command registrar");
            return;
        }
        
        m_registrars.push_back(std::move(registrar));
    }

    PyResult* CommandManager::Execute(Client* client, const std::string& commandLine) {
        if (!client) {
            sLog.Error("CommandManager", "Execute called with null client");
            return CreateErrorResponse("Internal error: null client");
        }
        
        if (commandLine.empty() || commandLine[0] != '/') {
            return CreateErrorResponse("Invalid command format");
        }
        
        // Parse command line
        auto args = ParseCommandLine(commandLine.substr(1)); // Remove leading '/'
        
        if (args.empty()) {
            // Return command list
            auto commands = GetCommandList(client);
            return CreateCommandListResponse(commands);
        }
        
        std::string commandName = args[0];
        args.erase(args.begin()); // Remove command name, keep parameters
        
        // Find command
        ICommand* command = FindCommand(commandName);
        if (!command) {
            return CreateErrorResponse("Unknown command: " + commandName);
        }
        
        // Check permissions
        if (!CheckPermission(client, command)) {
            sLog.Warning("CommandManager", "Access denied to command '%s' for user '%s' (role: %lld, required: %lld)",
                        commandName.c_str(), client->GetName(), client->GetAccountRole(), command->GetRequiredRole());
            return CreateErrorResponse("Access denied to command: " + commandName);
        }
        
        // Validate parameters
        std::string errorMsg;
        if (!command->ValidateParameters(args, errorMsg)) {
            return CreateErrorResponse("Parameter error: " + errorMsg);
        }
        
        try {
            // Create execution context
            CommandContext context(client, &m_services, args);
            
            // Execute command
            CommandResult result = command->Execute(context);
            
            if (result.success) {
                if (result.data) {
                    return result.data.get();
                } else {
                    return CreateSuccessResponse(result.message.empty() ? "Command executed successfully" : result.message);
                }
            } else {
                return CreateErrorResponse(result.message.empty() ? "Command execution failed" : result.message);
            }
        } catch (const std::exception& e) {
            sLog.Error("CommandManager", "Exception during command execution: %s", e.what());
            return CreateErrorResponse("Command execution failed: " + std::string(e.what()));
        } catch (...) {
            sLog.Error("CommandManager", "Unknown exception during command execution");
            return CreateErrorResponse("Command execution failed: unknown error");
        }
    }

    std::vector<std::string> CommandManager::GetCommandList(Client* client) const {
        std::vector<std::string> commands;
        
        for (const auto& pair : m_commands) {
            if (!client || CheckPermission(client, pair.second.get())) {
                commands.push_back(pair.first);
            }
        }
        
        std::sort(commands.begin(), commands.end());
        return commands;
    }

    std::string CommandManager::GetCommandHelp(const std::string& commandName) const {
        ICommand* command = FindCommand(commandName);
        if (!command) {
            return "Unknown command: " + commandName;
        }
        
        return command->GenerateHelp();
    }

    std::map<std::string, std::vector<std::string>> CommandManager::GetCommandsByCategory(Client* client) const {
        std::map<std::string, std::vector<std::string>> categories;
        
        for (const auto& pair : m_commands) {
            if (!client || CheckPermission(client, pair.second.get())) {
                std::string category = pair.second->GetCategory();
                categories[category].push_back(pair.first);
            }
        }
        
        // Sort commands in each category
        for (auto& pair : categories) {
            std::sort(pair.second.begin(), pair.second.end());
        }
        
        return categories;
    }

    ICommand* CommandManager::FindCommand(const std::string& name) const {
        auto it = m_commands.find(name);
        return (it != m_commands.end()) ? it->second.get() : nullptr;
    }

    void CommandManager::Initialize() {
        sLog.Blue("CommandManager", "Initializing command system...");
        
        // Register all commands
        for (auto& registrar : m_registrars) {
            registrar->RegisterCommands(*this);
        }
        
        sLog.Green("CommandManager", "Command system initialized with %lu commands", m_commands.size());
        
        // Display command statistics by category
        auto categories = GetCommandsByCategory();
        for (const auto& pair : categories) {
            sLog.Cyan("CommandManager", "  %s: %lu commands", pair.first.c_str(), pair.second.size());
        }
    }

    void CommandManager::Shutdown() {
        sLog.Blue("CommandManager", "Shutting down command system...");
        m_commands.clear();
        m_registrars.clear();
    }

    std::vector<std::string> CommandManager::ParseCommandLine(const std::string& commandLine) const {
        Seperator sep(commandLine.c_str());
        std::vector<std::string> args;
        
        for (uint32 i = 0; i < sep.argCount(); ++i) {
            args.push_back(sep.arg(i));
        }
        
        return args;
    }

    bool CommandManager::CheckPermission(Client* client, ICommand* command) const {
        if (!client || !command) {
            return false;
        }
        
        int64 requiredRole = command->GetRequiredRole();
        int64 clientRole = client->GetAccountRole();
        
        return (clientRole & requiredRole) == requiredRole;
    }

    PyResult* CommandManager::CreateErrorResponse(const std::string& message) const {
        return new PyResult(new PyString("Error: " + message));
    }

    PyResult* CommandManager::CreateSuccessResponse(const std::string& message) const {
        return new PyResult(new PyString(message));
    }

    PyResult* CommandManager::CreateCommandListResponse(const std::vector<std::string>& commands) const {
        std::string response = "Available commands: ";
        
        if (commands.empty()) {
            response += "None";
        } else {
            response += "[";
            for (size_t i = 0; i < commands.size(); ++i) {
                if (i > 0) response += ", ";
                response += "'" + commands[i] + "'";
            }
            response += "]";
        }
        
        return new PyResult(new PyString(response));
    }

} // namespace Commands